# syscall.S, defines the syscall entrance and the syscall jumptables
# vim:ts=4 sw=4 noexpandtab
#define ASM 1
#include "x86_desc.h"
#include "syscall.h"

.globl enter_syscall
enter_syscall:
	cld
	pushl	%eax
	push	%es
	push	%ds
	pushl	%eax
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	movl	$USER_DS, %edx
	movl	%edx, %es
	movl	%edx, %ds

	cmpl	$MAX_SYSCALL, %eax
	ja		exit_syscall

	call	*syscall_table(,%eax,4)
	movl	%eax, %ebx
	jmp		exit_syscall

exit_syscall:
	popl	%ebx
	popl	%ecx
	popl	%edx
	popl	%esi
	popl	%edi
	popl	%ebp
	popl	%eax
	pop		%ds
	pop		%es
	addl	$4, %esp
	iret

syscall_table:
	.long	0 # Unused

	.extern	sys_open
	.long	sys_open

	.extern	sys_read
	.long	sys_read

	.extern	sys_write
	.long	sys_write

	.extern	sys_close
	.long	sys_close

	.extern	sys_exec
	.long	sys_exec

	.extern	sys_halt
	.long	sys_halt

