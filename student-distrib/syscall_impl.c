/* syscall_impl, implement syscalls
 * vim:ts=4 sw=4 noexpandtab
 */
#include "types.h"
#include "lib.h"
#include "proc.h"
#include "term.h"
#include "file_sys.h"
#include "syscall.h"
#include "x86_desc.h"
#include "paging.h"
#include "rtc.h"
#include "sched.h"

/* IF is bit 9 in EFLAGS */
#define FLAG_INT (1<<9)

#define MAX_CMD_LEN 33

#define enter_userland(_ss, _esp, _flags, _cs, _eip) asm volatile(            \
		"movl     %0, %%eax\n"                                                \
		"movl     %%eax, %%ds\n"                                              \
		"movl     %%eax, %%es\n"                                              \
		"movl     %%eax, %%fs\n"                                              \
		"pushl    %0\n"                                                       \
		"pushl    %1\n"                                                       \
		"pushl    %2\n"                                                       \
		"pushl    %3\n"                                                       \
		"pushl    %4\n"                                                       \
		"iret"                                                                \
		: : "g"((_ss)), "g"((_esp)), "g"((_flags)), "g"((_cs)), "g"((_eip))   \
		: "memory", "cc", "eax")

#define push_context() asm volatile ( \
		"pushl    %%ss\n"             \
		"pushl    %%esp\n"            \
		"pushfl\n"                    \
		"pushl    %%cs\n"             \
		"pushl    %%eip\n"            \
		"PUSH_ALL")

uint8_t nprocs = 0;
uint32_t proc_bitmap = 0;

int32_t sys_open(const uint8_t *filename)
{
	dentry_t dentry;
	int32_t status;

	/* don't try to open null file */
	if (!filename) {
		return -1;
	}

	status = read_dentry_by_name(filename, &dentry);
	if (status) {
		return -1;
	}

	switch (dentry.file_type) {
		case FILE_TYPE_REG:
			return file_fops.open(filename);
		case FILE_TYPE_DIR:
			return dir_fops.open(filename);
		case FILE_TYPE_RTC:
			return rtc_fops.open(filename);
		default:
			/* unknown type */
			return -1;
	}

	return -1;
}

int32_t sys_read(int32_t fd, void *buf, int32_t nbytes)
{
	/* don't try to fill null buffer */
	if (!buf) {
		return -1;
	}

	file_t *file = get_file_from_fd(fd);

	/* get_file_from_fd validates the fd for us */
	if (!file || !(file->flags & FILE_PRESENT)) {
		return -1;
	}

	return file->file_op->read(fd, buf, nbytes);
}

int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes)
{
	/* no point writing if it's a null buffer */
	if (!buf) {
		return -1;
	}

	file_t *file = get_file_from_fd(fd);

	/* get_file_from_fd validates the fd for us */
	if (!file || !(file->flags & FILE_PRESENT)) {
		return -1;
	}

	return file->file_op->write(fd, buf, nbytes);
}

int32_t sys_close(int32_t fd)
{
	file_t *file = get_file_from_fd(fd);

	/* get_file_from_fd validates the fd for us */
	if (!file || !(file->flags & FILE_PRESENT)) {
		return -1;
	}

	return file->file_op->close(fd);
}

int32_t sys_exec(const uint8_t *command)
{
	int32_t ret;
	/* since command was actually passed in as eax, and since eax is the top of
	 * the registers_t structure, the address of command is also a pointer to the
	 * top of the hardware context of the syscall */
	ret = sys_exec_internal(command, (registers_t *)&command);

	/* if ret > 0, we started a new process, else we've failed */
	return (ret > 0) ? 0 : -1;
}

int32_t sys_halt(uint8_t status)
{
	int32_t term_id;

	cli();

	nprocs--;
	pcb_t *pcb = get_proc_pcb();
	free_pid(pcb->pid);

	/* Remove the process from the schedule queue */
	if (pcb->parent) {
		/* restore terminal pid */
		term_id = get_term_ctx() - term_terms;
		term_pids[term_id] = pcb->parent->pid;

		/* Halting from child process */

		/* Scheduling: halting child, set for removal */
		pcb->state |= EXIT_DEAD;

		pcb->parent_regs->eax = (int32_t)status;
		set_pdbr(pcb->parent->page_directory);
		tss.ss0 = KERNEL_DS;
		tss.esp0 = pcb->parent->kern_stack;
	}
	else {
		/* Halting from parent */

		/* Scheduling: halting parent shell. Relaunch */
		sched_flags.relaunch = 1;    //currently not designed action

		/* returning to kernel mode */
		set_pdbr(&page_directories[0]);
		tss.ss0 = KERNEL_DS;
		tss.esp0 = (KERNEL_MEM + MB_4_OFFSET -1) & 0xFFFFFFF0;
		/* returns to kernel space */
		asm volatile (
				"movl %0, %%esp\n"
				"addl $-4, %%esp\n"
				"sti\n" /* needed or things break sometimes */
				"ret"
				: : "g"(pcb->parent_regs)
				: "cc", "memory");
		/* control doesn't pass here */
	}
	/* Restores registers and exits syscalls */
	asm volatile (
			"movl %0, %%esp\n"
			"jmp exit_syscall"
			: : "g"(pcb->parent_regs)
			: "cc", "memory");
	return 0;
}

int32_t sys_getargs(uint8_t *buf, int32_t nbytes)
{
	/* can't copy to null buffer */
	if (!buf) {
		return -1;
	}

	pcb_t *pcb = get_proc_pcb();

	strncpy((int8_t *)buf, (int8_t *)pcb->cmd_args, nbytes);
	buf[nbytes-1] = '\0';

	return 0;
}

int32_t sys_vidmap(uint8_t **screen_start)
{
	pcb_t *pcb;
	if (screen_start < (uint8_t **)USER_MEM
			|| screen_start >= (uint8_t **)(USER_MEM + MB_4_OFFSET)) {
		return -1;
	}

	pcb = get_proc_pcb();
	install_user_vid_mem(pcb->page_directory, &user_video_mems[terminal_num]);
	pcb->has_video_mapped = 1;

	/* flush TLB */
	set_pdbr(pcb->page_directory);

	*screen_start = (uint8_t *)USER_VID;

	return 0;
}

int32_t sys_exec_internal(const uint8_t *command, registers_t *parent_ctx)
{
	uint8_t file_name[MAX_CMD_LEN];
	int32_t i, fn_cnt;
	const uint8_t *c;
	int32_t status;
	uint32_t eip;
	uint32_t flags;
	uint32_t old_pdbr;
	uint32_t kern_esp;
	uint32_t user_esp;
	int32_t pid;
	pcb_t *pcb;
	dentry_t dentry;

	/* can't exec a null command */
	if (!command) {
		return -1;
	}

	for (i = 0, c = command; *c == (uint8_t)' '; i++, c++) {
		/* skip beginning spaces */
	}

	/* copy command name */
	for (fn_cnt = 0; fn_cnt < MAX_CMD_LEN && *c != ' ' && *c != '\0'; i++, c++) {
		file_name[fn_cnt++] = *c;
	}
	file_name[fn_cnt] = '\0';

	/* move command to point just after the filename */
	command += i;

	status = read_dentry_by_name(file_name, &dentry);
	if (status) {
		goto fail;
	}

	if (nprocs < MAX_PROCESSES) {
		cli_and_save(flags);

		/* increase our process counter */
		nprocs++;

		/* Get first free pid, if none are available, call pid_fail */
		pid = get_first_free_pid();
		if(pid == -1) {
			goto pid_fail;
		}

		/* calculate location of bottom of process's stack */
		/* 0xFFFFFFFC aligns to a 4-byte boundary */
		kern_esp = (KERNEL_MEM + MB_4_OFFSET - USER_STACK_SIZE * pid - 1) & 0xFFFFFFFC;
		user_esp = (USER_MEM + MB_4_OFFSET - 1) & 0xFFFFFFFC;

		/* obtain and initialize the PCB, 0xFFFFE000 */
		pcb = (pcb_t *)(kern_esp & 0xFFFFE000);
		memset(pcb, 0, sizeof(*pcb));
		pcb->pid = pid;
		pcb->kern_stack = kern_esp;
		pcb->user_stack = user_esp;
		pcb->context_esp = NULL;
		pcb->page_directory = &page_directories[pcb->pid];

		/* Save old state */
		pcb->parent_regs = parent_ctx;

		/* copy args into pcb, first eat leading spaces */
		for (c = command; *c == (uint8_t)' '; i++, c++) {
			/* skip space */
		}

		/* copy args to buffer in pcb */
		strncpy((int8_t *)pcb->cmd_args, (int8_t *)c, MAX_ARGS_LEN);

		/* strncpy doesn't set last char to NULL if full length is read */
		pcb->cmd_args[MAX_ARGS_LEN] = '\0';

		/* store parent pcb if called from a process, will be null if called
		 * from the kernel */
		if (parent_ctx) {
			pcb->parent = get_proc_pcb();
		}
		else {
			pcb->parent = NULL;
		}

		/* set up the terminal driver. this has to come after we switch stacks,
		 * else it doesn't operate correctly */
		/* XXX: rewrite this so it's not abusing pointers */
		term_fops.open((uint8_t *)pcb);

		/* save old page directory */
		get_pdbr(old_pdbr);

		/* set new page directory */
		set_pdbr(pcb->page_directory);

		/* load the executable */
		/* XXX: do this earlier somehow? It's hard, since it needs to be done
		 *      after swapping page tables, but swapping page tables screws up
		 *      the current process's stack. Otherwise we do all this work and
		 *      may end up failing to a non-executable. */
		status = file_loader(&dentry, &eip);
		if (status) {
			goto exit_paging;
		}

		if (parent_ctx) {
			/* if we're executing on behalf of a userspace program, we'll jump straight
			 * into execution */
			tss.ss0 = KERNEL_DS;
			tss.esp0 = kern_esp;

			/* exec the actual process. this WON'T return */
			enter_userland(USER_DS, user_esp, flags, USER_CS, eip);
		}

		/* set the context in the PCB */
		pcb->context_esp = (registers_t *)kern_esp - 1;
		/* iret context */
		pcb->context_esp->ss = USER_DS;
		pcb->context_esp->user_esp = user_esp;
		/* ensure interrupt flag is set, as this may be called with the flag cleared */
		pcb->context_esp->eflags = FLAG_INT;
		pcb->context_esp->cs = USER_CS;
		pcb->context_esp->eip = eip;
		/* unused filler */
		pcb->context_esp->isrno = 0;
		pcb->context_esp->errno = 0;
		/* segments */
		pcb->context_esp->fs = USER_DS;
		pcb->context_esp->es = USER_DS;
		pcb->context_esp->ds = USER_DS;
		/* general purpose registers */
		pcb->context_esp->eax = 0;
		pcb->context_esp->ebp = 0;
		pcb->context_esp->edi = 0;
		pcb->context_esp->esi = 0;
		pcb->context_esp->edx = 0;
		pcb->context_esp->ecx = 0;
		pcb->context_esp->ebx = 0;

		/* needed so the process will get scheduled eventually */
		push_to_active(pcb->pid);

		/* since we may be returning to the parent process, restore the pdbr */
		set_pdbr(old_pdbr);

		goto out;
	}
	else {
		goto fail;
	}

exit_paging:
	free_pid(pcb->pid);
	set_pdbr(old_pdbr);
	if (pcb->parent) {
		tss.ss0 = KERNEL_DS;
		tss.esp0 = pcb->parent->kern_stack;
	}
pid_fail:
	--nprocs;
	restore_flags(flags);
fail:
	return -1;
out:
	restore_flags(flags);
	return pcb->pid;
}
