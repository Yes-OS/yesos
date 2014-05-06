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

uint8_t nprocs = 0;
uint32_t proc_bitmap = 0;

/* Sys Open:
 *
 * INPUT: filename - file that is being opened
 * Returns 0 on success, -1 on fail
 */
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
			return file_fops.open(get_proc_pcb(), filename);
		case FILE_TYPE_DIR:
			return dir_fops.open(get_proc_pcb(), filename);
		case FILE_TYPE_RTC:
			return rtc_fops.open(get_proc_pcb(), filename);
		default:
			/* unknown type */
			return -1;
	}

	return -1;
}

/* Sys Read:
 *
 * INPUT: filename - file that is being read
 *        buf - buffer read into
 *        nbytes - number of bytes to read
 * Returns 0 on success, -1 on fail
 */
int32_t sys_read(int32_t fd, void *buf, int32_t nbytes)
{
	pcb_t *pcb;

	/* don't try to fill null buffer */
	if (!buf) {
		return -1;
	}

	pcb = get_proc_pcb();
	file_t *file = get_file_from_fd(pcb, fd);

	/* get_file_from_fd validates the fd for us */
	if (!file || !(file->flags & FILE_PRESENT)) {
		return -1;
	}

	return file->file_op->read(pcb, fd, buf, nbytes);
}

/* Sys Write:
 *
 * INPUT: fd - file descriptor of file that is being written to
 *        buf - buffer written from
 *        nbytes - number of bytes to write
 * Returns 0 on success, -1 on fail
 */
int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes)
{
	pcb_t *pcb;

	/* no point writing if it's a null buffer */
	if (!buf) {
		return -1;
	}

	pcb = get_proc_pcb();
	file_t *file = get_file_from_fd(pcb, fd);

	/* get_file_from_fd validates the fd for us */
	if (!file || !(file->flags & FILE_PRESENT)) {
		return -1;
	}

	return file->file_op->write(pcb, fd, buf, nbytes);
}

/* Sys Close:
 *
 * INPUT: fd - file descriptor of file that is being Closed
 * Returns 0 on success, -1 on fail
 */
int32_t sys_close(int32_t fd)
{
	pcb_t *pcb;

	pcb = get_proc_pcb();
	file_t *file = get_file_from_fd(pcb, fd);

	/* get_file_from_fd validates the fd for us */
	if (!file || !(file->flags & FILE_PRESENT)) {
		return -1;
	}

	return file->file_op->close(pcb, fd);
}

/* Sys Exec:
 *
 * INPUT: Command - Command to be executed
 * Returns 0 on success, -1 on fail
 */
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

/* Sys Halt:
 *  Closes all relevant files
 *  Sets scheduling flags for a terminating process
 *  Situationally switches our of a program to user/kernel land
 *
 * INPUT: status - Status to put in parent eax
 * Returns 0 on success, -1 on fail
 */
int32_t sys_halt(uint8_t status)
{
	pcb_t *pcb;

	pcb = get_proc_pcb();

	return sys_halt_internal(pcb->pid, (int32_t)status);
}

/* Sys GetArgs:
 *
 * INPUT: buf - buffer to put args into
 *        nbytes - number of bytes to copy to the buffer
 * Returns 0 on success, -1 on fail
 */
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

/* Sys VidMap:
 *
 * INPUT: screen_start - Pointer to the screen's pointer to map to
 * Returns 0 on success, -1 on fail
 */
int32_t sys_vidmap(uint8_t **screen_start)
{
	pcb_t *pcb;
	if (screen_start < (uint8_t **)USER_MEM
			|| screen_start >= (uint8_t **)(USER_MEM + OFFSET_4MB)) {
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

/*
 * sys_sched
 *   Relinquishes the remaining scheduled time to another process
 *
 *   Inputs:
 *     - unused: not used.
 */
int32_t sys_sched(int32_t unused)
{
	uint32_t flags;
	cli_and_save(flags);

	if (nprocs > 0 && active_empty() && expired_empty()) {
		/* sleep here since the only running process is yielding control */
		sti();
		asm("hlt");
	}
	else {
		/* unused is actually used internally to get the top of the argument stack */
		scheduler((registers_t *)&unused);
	}

	restore_flags(flags);
	return 0;
}

/* Sys Exec Internal:
 *  Used for executing out of context
 *
 * INPUT: Command - Command to be executed
 *        parent_ctx - context data of the parent process of the to-be spawned process
 * Returns 0 on success, -1 on fail
 */
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
		kern_esp = (KERNEL_MEM + OFFSET_4MB - USER_STACK_SIZE * pid - 1) & ALIGN_4B;
		user_esp = (USER_MEM + OFFSET_4MB - 1) & ALIGN_4B;

		/* obtain and initialize the PCB */
		pcb = (pcb_t *)(kern_esp & ALIGN_8KB);
		memset(pcb, 0, sizeof(*pcb));
		pcb->pid = pid;
		pcb->kern_stack = kern_esp;
		pcb->user_stack = user_esp;
		pcb->sched_ctx = NULL;
		pcb->page_directory = &page_directories[pcb->pid];

		/* Save old state */
		pcb->parent_ctx = parent_ctx;

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

		/* set up the terminal driver */
		term_fops.open(pcb, NULL);

		/* save old page directory */
		get_pdbr(old_pdbr);

		/* set new page directory */
		set_pdbr(pcb->page_directory);

		/* load the executable */
		/* TODO: do this earlier somehow? It's hard, since it needs to be done
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
		pcb->sched_ctx = (registers_t *)kern_esp - 1;
		/* iret context */
		pcb->sched_ctx->ss = USER_DS;
		pcb->sched_ctx->user_esp = user_esp;
		/* ensure interrupt flag is set, as this may be called with the flag cleared */
		pcb->sched_ctx->eflags = FLAG_INT;
		pcb->sched_ctx->cs = USER_CS;
		pcb->sched_ctx->eip = eip;
		/* unused filler */
		pcb->sched_ctx->isrno = 0;
		pcb->sched_ctx->errno = 0;
		/* segments */
		pcb->sched_ctx->fs = USER_DS;
		pcb->sched_ctx->es = USER_DS;
		pcb->sched_ctx->ds = USER_DS;
		/* general purpose registers */
		pcb->sched_ctx->eax = 0;
		pcb->sched_ctx->ebp = 0;
		pcb->sched_ctx->edi = 0;
		pcb->sched_ctx->esi = 0;
		pcb->sched_ctx->edx = 0;
		pcb->sched_ctx->ecx = 0;
		pcb->sched_ctx->ebx = 0;

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

int32_t sys_halt_internal(int32_t pid, int32_t status)
{
	int32_t term_id;
	int32_t i;
	file_t *file;
	uint32_t flags;

	cli_and_save(flags);

	nprocs--;
	pcb_t *pcb = get_pcb_from_pid(pid);
	free_pid(pcb->pid);

	/* close all open files */
	for (i = 0; i < MAX_FILES; i++) {
		file = &pcb->file_array[i];
		if (file->flags & (FILE_OPEN | FILE_PRESENT)) {
			file->file_op->close(pcb, i);
		}
	}

	/* Scheduling: halting child, set for removal */
	pcb->state |= EXIT_DEAD;

	/* Remove the process from the schedule queue */
	if (pcb->parent) {
		/* restore terminal pid */
		term_id = get_term_ctx(pcb) - term_terms;
		term_pids[term_id] = pcb->parent->pid;
		pcb->parent_ctx->eax = status;
	}
	else {
		/* do whatcha want */
		//set_cursor(10,0);
		//printf("EXITING LAST SHELL IN TERMINAL\n");

		/* clear terminal pid so a new thing can spawn */
		term_id = pcb->term_ctx - term_terms;
		term_pids[term_id] = -1;

		switch_to_open_terminal();

		/* otherwise control should go back to the caller */
		return 0;
	}

	if (pcb == get_proc_pcb()) {
		/* if we're killing the process currently running, we return to the parent process */
		set_pdbr(pcb->parent->page_directory);
		tss.ss0 = KERNEL_DS;
		tss.esp0 = pcb->parent->kern_stack;

		/* Restores registers and exits syscalls */
		exit_syscall(pcb->parent_ctx);
	}

	/* otherwise we just want to give control back to the caller, but we want
	 * to give control back to the parent process, so we should schedule it first */
	push_to_expired(pcb->parent->pid);

	/* if the parent has a context, it's out of date, so give it the context from
	 * when exec was called */
	if (pcb->parent_ctx) {
		pcb->parent->sched_ctx = pcb->parent_ctx;
	}

	restore_flags(flags);

	return 0;
}


