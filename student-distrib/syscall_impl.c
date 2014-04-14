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

typedef int32_t open_t(const uint8_t *filename);
typedef int32_t read_t(int32_t fd, void *buf, int32_t nbytes);
typedef int32_t write_t(int32_t fd, const void *buf, int32_t nbytes);
typedef int32_t close_t(int32_t fd);

/* Gets the process's PCB */
static inline pcb_t *get_proc_pcb()
{
	uint32_t pcb;
	asm (	"movl	$0xFFFFE000, %0		\n\
			 andl	%%esp, %0"
			: "=r"(pcb)
			: : );
	return (pcb_t *)pcb;
}

int32_t sys_open(const uint8_t *filename)
{
	return 0;
}

int32_t sys_read(int32_t fd, void *buf, int32_t nbytes)
{
	return 0;
}

int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes)
{
	return 0;
}

int32_t sys_close(int32_t fd)
{
	return 0;
}

int32_t sys_exec(const uint8_t *command)
{
	uint8_t file[MAX_CMD_LEN];
	int i;
	const uint8_t *c;

	for (i = 0, c = command; i < MAX_CMD_LEN && *c == (uint8_t)' '; i++, c++) {
		/* skip beginning spaces */
	}


	return 0;
}

int32_t sys_halt(uint8_t status)
{
	return 0;
}

