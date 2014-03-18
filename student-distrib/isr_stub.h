/* isr_stub.h, defines interrupt handler stubs
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _IRQ_STUB_H_
#define _IRQ_STUB_H_

#ifndef ASM

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

// IRQs
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
