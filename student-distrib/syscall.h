/* syscall.h, defines the syscall numbers
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#define SYS_OPEN    1
#define SYS_READ    2
#define SYS_WRITE   3
#define SYS_CLOSE   4
#define SYS_EXEC    5
#define SYS_HALT    6

#define MAX_SYSCALL 6

#ifndef ASM
void enter_syscall();

#define do_syscall(num) \
	asm volatile ( \
		"int		$0x80"	\
		: : "a"(num)	\
		: "cc", "memory");
#endif
#endif
