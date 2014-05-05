# syscall.S, defines the syscall entrance and the syscall jumptables
# vim:ts=4 sw=4 noexpandtab
#define ASM 1
#include "isr_stub.h"
#include "x86_desc.h"
#include "syscall.h"

# Follow standard syscall table implementation
.globl enter_syscall
enter_syscall:
	pushl	$0
	pushl	%eax
	PUSH_ALL

	cmpl	$MIN_SYSCALL, %eax
	jb		syscall_oob
	cmpl	$MAX_SYSCALL, %eax
	ja		syscall_oob

	call	*syscall_table(,%eax,4)

	# Set the return value by overwriting the first eax we pushed
	movl	%eax, 24(%esp)
	jmp		exit_syscall

# syscall was out of bounds, return -1
syscall_oob:
	movl	$-1, 24(%esp)

.globl exit_syscall
exit_syscall:
	POP_ALL
	addl	$8, %esp
	iret

syscall_table:
	.long	0 # Unused

	.long	sys_halt
	.long	sys_exec
	.long	sys_read
	.long	sys_write
	.long	sys_open
	.long	sys_close
	.long	sys_getargs
	.long	sys_vidmap
