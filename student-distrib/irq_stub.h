/* irq_stub.h, defines interrupt handler stubs
 * vim:ts=4 noexpandtab
 */
#ifndef _IRQ_STUB_H_
#define _IRQ_STUB_H_

#ifndef ASM

void divide_error();
void debug();
void nmi();
void breakpoint();
void overflow();
void invalid_opcode();
void device_not_avilable();
void double_fault();
void coprocessor_segment_overrun();
void invalid_tss();
void segment_not_present();
void stack_fault();
void general_protection();
void coprocessor_error();
void alignment_check();
void machine_check();
void simd_coprocessor_error();

#endif

#endif
