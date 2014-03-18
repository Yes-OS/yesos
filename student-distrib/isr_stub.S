# isr_stub.S, defines stubs for all the necessary interrupts and forwards them
#    to one great dispatcher
# vim:ts=4 noexpandtab

# Interrupt handlers
#define ASM 1
#include "isr_stub.h"
#include "x86_desc.h"

.text

.globl divide_error, debug, nmi, breakpoint, overflow, bound
.globl invalid_opcode, device_not_available, double_fault, coprocessor_segment_overrun
.globl invalid_tss, segment_not_present, stack_fault, general_protection, page_fault
.globl coprocessor_error, alignment_check, machine_check, simd_coprocessor_error

.align 4

divide_error:
	cli
	pushl	$0	# dummy error code
	pushl	$0	# isr number
	jmp		isr_stub


debug:
	cli
	pushl	$0	# dummy error code
	pushl	$1	# isr number
	jmp		isr_stub


nmi:
	cli
	pushl	$0	# dummy error code
	pushl	$2	# isr number
	jmp		isr_stub


breakpoint:
	cli
	pushl	$0	# dummy error code
	pushl	$3	# isr number
	jmp		isr_stub


overflow:
	cli
	pushl	$0	# dummy error code
	pushl	$4	# isr number
	jmp		isr_stub


bound:
	cli
	pushl	$0	# dummy error code
	pushl	$5	# isr number
	jmp		isr_stub


invalid_opcode:
	cli
	pushl	$0	# dummy error code
	pushl	$6	# isr number
	jmp		isr_stub


device_not_available:
	cli
	pushl	$0	# dummy error code
	pushl	$7	# isr number
	jmp		isr_stub


double_fault:
	cli
	pushl	$0	# dummy error code
	pushl	$8	# isr number
	jmp		isr_stub


coprocessor_segment_overrun:
	cli
	pushl	$0	# dummy error code
	pushl	$9	# isr number
	jmp		isr_stub


invalid_tss:
	cli
	pushl	$10	# isr number
	jmp		isr_stub


segment_not_present:
	cli
	pushl	$11	# isr number
	jmp		isr_stub


stack_fault:
	cli
	pushl	$12	# isr number
	jmp		isr_stub


general_protection:
	cli
	pushl	$13	# isr number
	jmp		isr_stub


page_fault:
	cli
	pushl	$14	# isr number
	jmp		isr_stub


coprocessor_error:
	cli
	pushl	$0	# dummy error code
	pushl	$16	# isr number
	jmp		isr_stub


alignment_check:
	cli
	pushl	$17	# isr number
	jmp		isr_stub


machine_check:
	cli
	pushl	$0	# dummy error code
	pushl	$18	# isr number
	jmp		isr_stub


simd_coprocessor_error:
	cli
	pushl	$0	# dummy error code
	pushl	$19	# isr number
	jmp		isr_stub


isr_stub:
	pushal

	# push segments
	xorl	%eax, %eax
	movw	%gs, %ax
	pushl	%eax
	movw	%fs, %ax
	pushl	%eax
	movw	%es, %ax
	pushl	%eax
	movw	%ds, %ax
	pushl	%eax

	movw	$KERNEL_DS, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs

	call	isr_impl

	# restore segments
	popl	%eax
	movw	%ax, %ds
	popl	%eax
	movw	%ax, %es
	popl	%eax
	movw	%ax, %fs
	popl	%eax
	movw	%ax, %gs

	popal
	addl	$8, %esp	# clean up error code and ISR number
	sti
	iret
