# isr_stub.S, defines stubs for all the necessary interrupts and forwards them
#    to one great dispatcher
# vim:ts=4 sw=4 noexpandtab

# Interrupt handlers
#define ASM 1
#include "isr_stub.h"
#include "x86_desc.h"

# Macro definition for creating an interrupt stub with no hardware error
#define MKINTSTUB_NOERR(name, number) \
.globl name                          ;\
name:                                ;\
	pushl	$0                       ;\
	pushl	$number                  ;\
	jmp		isr_stub

# Macro definition for creating an interrupt stub with a hardware error
#define MKINTSTUB(name, number) \
.globl name                    ;\
name:                          ;\
	pushl	$number            ;\
	jmp		isr_stub


.text

.align 4

# sets up the initial stubs for the exceptions
MKINTSTUB_NOERR	(divide_error, EXCEPTION_DIVIDE)
MKINTSTUB_NOERR	(debug, EXCEPTION_DEBUG)
MKINTSTUB_NOERR	(nmi, EXCEPTION_NMI)
MKINTSTUB_NOERR	(breakpoint, EXCEPTION_BREAKPOINT)
MKINTSTUB_NOERR	(overflow, EXCEPTION_OVERFLOW)
MKINTSTUB_NOERR	(bound, EXCEPTION_BOUND)
MKINTSTUB_NOERR	(invalid_opcode, EXCEPTION_INVALID_OPCODE)
MKINTSTUB_NOERR	(device_not_available, EXCEPTION_DEV_NOT_AVAIL)
MKINTSTUB_NOERR	(double_fault, EXCEPTION_DOUBLE_FAULT)
MKINTSTUB_NOERR	(coprocessor_segment_overrun, EXCEPTION_COPROC_SEG_OVERRUN)
MKINTSTUB		(invalid_tss, EXCEPTION_INVALID_TSS)
MKINTSTUB		(segment_not_present, EXCEPTION_SEG_NOT_PRES)
MKINTSTUB		(stack_fault, EXCEPTION_STACK_FAULT)
MKINTSTUB		(general_protection, EXCEPTION_GENERAL_PROTECTION)
MKINTSTUB		(page_fault, EXCEPTION_PAGE_FAULT)
MKINTSTUB_NOERR	(coprocessor_error, EXCEPTION_COPROC_ERROR)
MKINTSTUB		(alignment_check, EXCEPTION_ALIGNMENT_CHECK)
MKINTSTUB_NOERR	(machine_check, EXCEPTION_MACHINE_CHECK)
MKINTSTUB_NOERR	(simd_coprocessor_error, EXCEPTION_SIMD_COPROC_ERR)

# null interrupt installed in the interrupts we don't explicitly handle
MKINTSTUB_NOERR	(null_int, EXCEPTION_NULL)

# the interrupt requests
MKINTSTUB_NOERR	(irq0, IRQ_PIT)
MKINTSTUB_NOERR	(irq1, IRQ_KBD)
MKINTSTUB_NOERR	(irq2, IRQ2)
MKINTSTUB_NOERR	(irq3, IRQ3)
MKINTSTUB_NOERR	(irq4, IRQ4)
MKINTSTUB_NOERR	(irq5, IRQ5)
MKINTSTUB_NOERR	(irq6, IRQ6)
MKINTSTUB_NOERR	(irq7, IRQ7)
MKINTSTUB_NOERR	(irq8, IRQ_RTC)
MKINTSTUB_NOERR	(irq9, IRQ9)
MKINTSTUB_NOERR	(irq10, IRQ10)
MKINTSTUB_NOERR	(irq11, IRQ11)
MKINTSTUB_NOERR	(irq12, IRQ12)
MKINTSTUB_NOERR	(irq13, IRQ13)
MKINTSTUB_NOERR	(irq14, IRQ14)
MKINTSTUB_NOERR	(irq15, IRQ15)


# isr_stub sets up the stack for interrupt handlers and calls a general c function to handle the rest
isr_stub:
	PUSH_ALL
	movl	$KERNEL_DS, %eax
	movw	%ax, %es
	movw	%ax, %ds

	call	isr_impl

	# restore segments
	POP_ALL

	addl	$8, %esp	# clean up error code and ISR number
	iret

