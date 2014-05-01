/* isr_stub.h, defines interrupt handler stubs
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _IRQ_STUB_H_
#define _IRQ_STUB_H_

#include "i8259.h"

/****************************************
 *            Global Defines            *
 ****************************************/

#define EXCEPTION_DIVIDE             0
#define EXCEPTION_DEBUG              1
#define EXCEPTION_NMI                2
#define EXCEPTION_BREAKPOINT         3
#define EXCEPTION_OVERFLOW           4
#define EXCEPTION_BOUND              5
#define EXCEPTION_INVALID_OPCODE     6
#define EXCEPTION_DEV_NOT_AVAIL      7
#define EXCEPTION_DOUBLE_FAULT       8
#define EXCEPTION_COPROC_SEG_OVERRUN 9
#define EXCEPTION_INVALID_TSS        10
#define EXCEPTION_SEG_NOT_PRES       11
#define EXCEPTION_STACK_FAULT        12
#define EXCEPTION_GENERAL_PROTECTION 13
#define EXCEPTION_PAGE_FAULT         14
#define EXCEPTION_COPROC_ERROR       16
#define EXCEPTION_ALIGNMENT_CHECK    17
#define EXCEPTION_MACHINE_CHECK      18
#define EXCEPTION_SIMD_COPROC_ERR    19

/* should not occur */
#define EXCEPTION_NULL               -1

#define IRQ_PIT (IRQ_START + PIT_IRQ_PORT)
#define IRQ_KBD (IRQ_START + KBD_IRQ_PORT)
#define IRQ2    (IRQ_START + 2)
#define IRQ3    (IRQ_START + 3)
#define IRQ4    (IRQ_START + 4)
#define IRQ5    (IRQ_START + 5)
#define IRQ6    (IRQ_START + 6)
#define IRQ7    (IRQ_START + 7)
#define IRQ_RTC (IRQ_START + RTC_IRQ_PORT)
#define IRQ9    (IRQ_START + 9)
#define IRQ10   (IRQ_START + 10)
#define IRQ11   (IRQ_START + 11)
#define IRQ12   (IRQ_START + 12)
#define IRQ13   (IRQ_START + 13)
#define IRQ14   (IRQ_START + 14)
#define IRQ15   (IRQ_START + 15)

/* convenience methods for managing stack for syscalls and interrupts */
#define PUSH_ALL \
	cld ;\
	push	%fs ;\
	push	%es ;\
	push	%ds ;\
	pushl	%eax ;\
	pushl	%ebp ;\
	pushl	%edi ;\
	pushl	%esi ;\
	pushl	%edx ;\
	pushl	%ecx ;\
	pushl	%ebx ;\
	movl	$USER_DS, %edx ;\
	movl	%edx, %es ;\
	movl	%edx, %ds

#define POP_ALL \
	popl	%ebx ;\
	popl	%ecx ;\
	popl	%edx ;\
	popl	%esi ;\
	popl	%edi ;\
	popl	%ebp ;\
	popl	%eax ;\
	pop		%ds ;\
	pop		%es ;\
	pop		%fs


#ifndef ASM

/****************************************
 *         Function Declarations        *
 ****************************************/

/* Exceptions */
void divide_error();
void debug();
void nmi();
void breakpoint();
void overflow();
void bound();
void invalid_opcode();
void device_not_available();
void double_fault();
void coprocessor_segment_overrun();
void invalid_tss();
void segment_not_present();
void stack_fault();
void general_protection();
void page_fault();
void coprocessor_error();
void alignment_check();
void machine_check();
void simd_coprocessor_error();
void null_int();

/* Interrupt Requests */
void irq0();
void irq1();
void irq2();
void irq3();
void irq4();
void irq5();
void irq6();
void irq7();
void irq8();
void irq9();
void irq10();
void irq11();
void irq12();
void irq13();
void irq14();
void irq15();

#endif

#endif
