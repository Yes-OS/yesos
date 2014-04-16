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

#define MAX_CMD_LEN 33

#define enter_userland(_ss, _esp, _flags, _cs, _eip) asm volatile (           \
		"pushl    %0\n"                                                       \
		"pushl    %1\n"                                                       \
		"pushl    %2\n"                                                       \
		"pushl    %3\n"                                                       \
		"pushl    %4\n"                                                       \
		"iret"                                                                \
		: : "g"((_ss)), "g"((_esp)), "g"((_flags)), "g"((_cs)), "g"((_eip))   \
		: "memory", "cc")

static uint8_t nprocs = 0;

int32_t sys_open(const uint8_t *filename)
{
	return 0;
}

int32_t sys_read(int32_t fd, void *buf, int32_t nbytes)
{
	pcb_t* pcb = get_proc_pcb();
	file_t file = pcb->file_array[fd];
	return file.file_op->read(fd, buf, nbytes);
}

int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes)
{
	pcb_t* pcb = get_proc_pcb();
	file_t file = pcb->file_array[fd];
	return file.file_op->write(fd, buf, nbytes);
}

int32_t sys_close(int32_t fd)
{
	pcb_t* pcb = get_proc_pcb();
	file_t file = pcb->file_array[fd];
	return file.file_op->close(fd);
}

int32_t sys_exec(const uint8_t *command)
{
	uint8_t file_name[MAX_CMD_LEN];
	int i, fcnt = 0;
	const uint8_t *c;
	int32_t status;
	uint32_t eip;
	uint32_t flags;
	uint32_t old_pdbr;
	uint32_t kern_esp;
	uint32_t user_esp;
	pcb_t *pcb;
	dentry_t dentry;

	for (i = 0, c = command; i < MAX_CMD_LEN && *c == (uint8_t)' '; i++, c++) {
		/* skip beginning spaces */
	}

	/* copy command name */
	for (; i < MAX_CMD_LEN && *c != ' ' && *c != '\0'; i++, c++) {
		file_name[fcnt++] = *c;
	}
	file_name[fcnt] = '\0';

	status = read_dentry_by_name(file_name, &dentry);
	if (status) {
		goto fail;
	}

	if (nprocs < MAX_PROCESSES) {
		cli_and_save(flags);

		/* increase our process counter */
		nprocs++;

		/* save old page directory */
		get_pdbr(old_pdbr);

		/* set new page directory */
		set_pdbr(&page_directories[nprocs]);

		status = file_loader(&dentry, &eip);
		if (status) {
			goto exit_paging;
		}

		/* calculate location of bottom of process's stack */
		kern_esp = (KERNEL_MEM + 0x400000 - 0x2000 * nprocs - 1) & 0xFFFFFFF0;
		user_esp = (0x08000000 + 0x400000 -1) & 0xFFFFFFF0;

		/* obtain and initialize the PCB */
		pcb = (pcb_t *)(kern_esp & 0xFFFFC000);
		memset(pcb, 0, sizeof(*pcb));
		pcb->pid = nprocs;
		pcb->kern_stack = kern_esp;
		pcb->user_stack = user_esp;
		pcb->page_directory = &page_directories[nprocs];
		/* XXX: Save old state */
		pcb->parent_regs = (registers_t *)&command;

		/* store parent pcb if called from a process */
		if (nprocs > 0) {
			pcb->parent = get_proc_pcb();
		}

		{
			/* set up fops */
			file_t file;
			file.flags = FILE_PRESENT | FILE_OPEN;
			file.file_op = &term_fops;
			file.file_pos = 0;
			file.inode_ptr = 0;

			pcb->file_array[0] = file;
			pcb->file_array[1] = file;
		}

		tss.ss0 = KERNEL_DS;
		tss.esp0 = kern_esp;

		/* exec the actual process */
		enter_userland(USER_DS, user_esp, flags, USER_CS, eip);

		/* never reached */
		goto out;
	}
	else {
		goto fail;
	}

exit_paging:
	set_pdbr(old_pdbr);
	restore_flags(flags);
fail:
	return -1;
out:
	return 0;
}

int32_t sys_halt(uint8_t status)
{
	nprocs--;
	pcb_t *pcb = get_proc_pcb();
	if (nprocs > 0) {
		/* We're still runnin a user process */
		pcb->parent_regs->eax = (int32_t)status;
		set_pdbr(pcb->parent->page_directory);
		tss.ss0 = KERNEL_DS;
		tss.esp0 = pcb->parent->kern_stack;
	}
	else {
		/* returning to kernel mode */
		set_pdbr(&page_directories[0]);
		tss.ss0 = KERNEL_DS;
		tss.esp0 = (KERNEL_MEM + 0x400000 -1) & 0xFFFFFFF0;
		asm volatile (
				"movl %0, %%esp\n"
				"addl $-4, %%esp\n"
				"ret"
				: : "g"(pcb->parent_regs)
				: "cc", "memory");
		/* control doesn't pass here */
	}
	asm volatile (
			"movl %0, %%esp\n"
			"jmp exit_syscall"
			: : "g"(pcb->parent_regs)
			: "cc", "memory");
	return 0;
}

