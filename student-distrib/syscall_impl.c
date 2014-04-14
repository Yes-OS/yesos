/* syscall_impl, implement syscalls
 * vim:ts=4 sw=4 noexpandtab
 */
#include "types.h"
#include "lib.h"
#include "proc.h"
#include "term.h"
#include "file_sys.h"
#include "syscall.h"

#define MAX_CMD_LEN 33

static uint8_t nprocs = 0;

typedef int32_t open_t(const uint8_t *filename);
typedef int32_t read_t(int32_t fd, void *buf, int32_t nbytes);
typedef int32_t write_t(int32_t fd, const void *buf, int32_t nbytes);
typedef int32_t close_t(int32_t fd);

/* Gets the process's PCB */
static inline pcb_t *get_proc_pcb()
{
	uint32_t pcb;
	asm (	"movl	$0xFFFFE000, %0		\n"
			"andl	%%esp, %0"
			: "=r"(pcb)
			: 
			: "memory"
			);
	return (pcb_t *)pcb;
}

int32_t sys_open(const uint8_t *filename)
{
	return 0;
}

int32_t sys_read(int32_t fd, void *buf, int32_t nbytes)
{
	pcb_t* PCB = get_proc_pcb();
	file_t file = PCB->file_array[fd];
	return ((read_t*)file.file_op + 1)(fd, buf, nbytes);
}

int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes)
{
	pcb_t* PCB = get_proc_pcb();
	file_t file = PCB->file_array[fd];
	return ((write_t*)file.file_op + 2)(fd, buf, nbytes);
}

int32_t sys_close(int32_t fd)
{
	pcb_t* PCB = get_proc_pcb();
	file_t file = PCB->file_array[fd];
	return ((close_t*)file.file_op + 3)(fd);
}

int32_t sys_exec(const uint8_t *command)
{
	uint8_t file_name[MAX_CMD_LEN];
	int i, fcnt = 0;
	const uint8_t *c;
	dentry_t dentry;
	file_t file;
	int32_t status;
	uint32_t eip;
	pcb_t *pcb;

	for (i = 0, c = command; i < MAX_CMD_LEN && *c == (uint8_t)' '; i++, c++) {
		/* skip beginning spaces */
	}

	/* copy command name */
	for (; i < MAX_CMD_LEN && *c != ' '; i++, c++) {
		file_name[fcnt++] = *c;
	}
	file_name[fcnt] = '\0';

	status = read_dentry_by_name(file_name, &dentry);
	if (!status) {
		return status;
	}

	file.flags = 0;
	file.file_op = 0;
	file.file_pos = 0;
	file.inode_ptr = dentry.inode_num;

	status = file_loader(&file, &eip);
	if (!status) {
		return status;
	}

	pcb = get_proc_pcb();

	{
		/* set up fops */
		file_t file;
		file.flags = 0;
		file.file_op = term_fops;
		file.file_pos = 0;
		file.inode_ptr = 0;
		pcb
	}

	return 0;
}

int32_t sys_halt(uint8_t status)
{
	return 0;
}

