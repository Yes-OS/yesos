/* syscall.h, defines the syscall numbers
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include "types.h"

#define SYS_OPEN    1
#define SYS_READ    2
#define SYS_WRITE   3
#define SYS_CLOSE   4
#define SYS_EXEC    5
#define SYS_HALT    6

#define MAX_SYSCALL 6

#ifndef ASM
void enter_syscall();
int32_t sys_open(const uint8_t *filename);
int32_t sys_read(int32_t fd, void *buf, int32_t nbytes);
int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t sys_close(int32_t fd);
int32_t sys_exec(const uint8_t *command);
int32_t sys_halt(uint8_t status);

#define do_syscall(num) \
	asm volatile ( \
		"int		$0x80"	\
		: : "a"(num)	\
		: "cc", "memory");
#endif
#endif
