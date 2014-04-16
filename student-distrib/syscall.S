# syscall.S, defines the syscall entrance and the syscall jumptables
# vim:ts=4 sw=4 noexpandtab
#define ASM 1
#include "isr_stub.h"
#include "x86_desc.h"
#include "syscall.h"

.globl enter_syscall
enter_syscall:
	pushl	$0
	pushl	%eax
	PUSH_ALL

	cmpl	$MAX_SYSCALL, %eax
	ja		exit_syscall

	call	*syscall_table(,%eax,4)

	# Set the return value by overwriting the first eax we pushed
	movl	%eax, 40(%esp)
	jmp		exit_syscall

.globl exit_syscall
exit_syscall:
	POP_ALL
	popl	%eax
	addl	$4, %esp
	iret

syscall_table:
	.long	0 # Unused

	.extern	sys_halt
	.long	sys_halt

	.extern	sys_exec
	.long	sys_exec

	.extern	sys_read
	.long	sys_read

	.extern	sys_write
	.long	sys_write

	.extern	sys_open
	.long	sys_open

	.extern	sys_close
	.long	sys_close
