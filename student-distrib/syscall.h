/* syscall.h, defines the syscall numbers
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include "types.h"
#include "isr.h"

/****************************************
 *            Global Defines            *
 ****************************************/

/* syscall numbers */
#define SYS_HALT    1
#define SYS_EXECUTE 2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETARGS 7
#define SYS_VIDMAP  8
#define SYS_SCHED   9

#define MIN_SYSCALL 1
#define MAX_SYSCALL 9

#ifndef ASM

/****************************************
 *         Function Declarations        *
 ****************************************/

/* Enters a system call */
void enter_syscall();

/* Opens a new system call */
int32_t sys_open(const uint8_t *filename);

/* Does a syscall read */
int32_t sys_read(int32_t fd, void *buf, int32_t nbytes);

/* Does a syscall write */
int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes);

/* Closes a syscall */
int32_t sys_close(int32_t fd);

/* Executes a syscall; adds to scheduler */
int32_t sys_exec(const uint8_t *command);

/* Halts the syscall; removes from scheduler */
int32_t sys_halt(uint8_t status);

/* Gets the arguments of a syscall */
int32_t sys_getargs(uint8_t *buf, int32_t nbytes);

/* Maps to video memory of a specified screen */
int32_t sys_vidmap(uint8_t **screen_start);

/* Relinquish remainder of scheduled time to another process */
int32_t sys_sched(int32_t unused);

/* for internal use to spawn parentless processes */
int32_t sys_exec_internal(const uint8_t *command, registers_t *parent_ctx);
int32_t sys_halt_internal(int32_t pid, int32_t status);

/* used by the kernel to relinquish sheduling time */
#define sched() asm volatile ( \
		"movl    %0, %%eax\n"  \
		"int $0x80"            \
		: : "g"(SYS_SCHED)     \
		: "eax", "cc", "memory")

/* used to exit as syscall */
#define exit_syscall(_context) asm volatile ( \
				"movl %0, %%esp\n"            \
				"jmp exit_syscall"            \
				: : "g"(_context)             \
				: "cc", "memory")

#endif /* ASM */
#endif /* _SYSCALL_H_ */
