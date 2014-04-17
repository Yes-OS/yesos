/* syscall.h, defines the syscall numbers
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include "types.h"

#define SYS_HALT    1
#define SYS_EXECUTE 2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETARGS 7
#define SYS_VIDMAP  8

#define MIN_SYSCALL 1
#define MAX_SYSCALL 8

#ifndef ASM
void enter_syscall();
int32_t sys_open(const uint8_t *filename);
int32_t sys_read(int32_t fd, void *buf, int32_t nbytes);
int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t sys_close(int32_t fd);
int32_t sys_exec(const uint8_t *command);
int32_t sys_halt(uint8_t status);
int32_t sys_getargs(uint8_t *buf, int32_t nbytes);
int32_t sys_vidmap(uint8_t **screen_start);

#define do_syscall(num) \
	asm volatile ( \
		"int		$0x80"	\
		: : "a"(num)	\
		: "cc", "memory");
#endif
#endif
