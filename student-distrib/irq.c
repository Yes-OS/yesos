/* irq.c, implement interrupt request handling and related functions
 * vim:ts=4 noexpandtab
 */

#include "irq.h"
#include "lib.h"
#include "x86_desc.h"
#include "irq_stub.h"
#include "types.h"

void isr_impl(registers_t regs)
{
	switch (regs.irqno) {
		default:
			printf("Interrupt ocurred: %d\n", regs.irqno);
			break;
	}
}

void set_intr_gate(uint8_t n, uint32_t addr)
{
	idt_desc_t new_idt_entry;
	memset(&new_idt_entry, 0, sizeof(idt_desc_t));

	/* enables the idt entry */
	new_idt_entry.present = 1;

	/* defines 32-bit trap interrupt */
	/* touch this code and I'll end you - adam */
	new_idt_entry.size = 1;
	new_idt_entry.reserved1 = 1;
	new_idt_entry.reserved2 = 1;
	new_idt_entry.reserved3 = 0;

	/* Ring 0 level access only */
	new_idt_entry.dpl = 0;
	new_idt_entry.seg_selector = KERNEL_CS;

	/* initialize the offset */
	SET_IDT_ENTRY(new_idt_entry, addr);

	/* store IDT */
	idt[n] = new_idt_entry;
}

void set_system_gate(uint8_t n, uint32_t addr)
{
	idt_desc_t new_idt_entry;
	memset(&new_idt_entry, 0, sizeof(idt_desc_t));

	/* enables the idt entry */
	new_idt_entry.present = 1;

	/* defines 32-bit trap interrupt */
	/* touch this code and I'll end you - adam */
	new_idt_entry.size = 1;
	new_idt_entry.reserved1 = 1;
	new_idt_entry.reserved2 = 1;
	new_idt_entry.reserved3 = 1;

	/* Can be called by user level code */
	new_idt_entry.dpl = 3;
	new_idt_entry.seg_selector = KERNEL_CS;

	/* initialize the offset */
	SET_IDT_ENTRY(new_idt_entry, addr);

	/* store IDT */
	idt[n] = new_idt_entry;
}

void set_system_intr_gate(uint8_t n, uint32_t addr)
{
	idt_desc_t new_idt_entry;
	memset(&new_idt_entry, 0, sizeof(idt_desc_t));

	/* enables the idt entry */
	new_idt_entry.present = 1;

	/* defines 32-bit trap interrupt */
	/* touch this code and I'll end you - adam */
	new_idt_entry.size = 1;
	new_idt_entry.reserved1 = 1;
	new_idt_entry.reserved2 = 1;
	new_idt_entry.reserved3 = 0;

	/* Can be called by user level code */
	new_idt_entry.dpl = 3;
	new_idt_entry.seg_selector = KERNEL_CS;

	/* initialize the offset */
	SET_IDT_ENTRY(new_idt_entry, addr);

	/* store IDT */
	idt[n] = new_idt_entry;
}

void set_trap_gate(uint8_t n, uint32_t addr)
{
	idt_desc_t new_idt_entry;
	memset(&new_idt_entry, 0, sizeof(idt_desc_t));

	/* enables the idt entry */
	new_idt_entry.present = 1;

	/* defines 32-bit trap interrupt */
	/* touch this code and I'll end you - adam */
	new_idt_entry.size = 1;
	new_idt_entry.reserved1 = 1;
	new_idt_entry.reserved2 = 1;
	new_idt_entry.reserved3 = 0;

	/* Can be called by user level code */
	new_idt_entry.dpl = 0;
	new_idt_entry.seg_selector = KERNEL_CS;

	/* initialize the offset */
	SET_IDT_ENTRY(new_idt_entry, addr);

	/* store IDT */
	idt[n] = new_idt_entry;
}

void set_task_gate(uint8_t n, uint16_t gdt)
{
	/* not yet implemented, we may not even use this */
}

void install_interrupts()
{
	set_trap_gate(0,  (uint32_t)&divide_error);
	set_trap_gate(1,  (uint32_t)&debug);
	set_trap_gate(2,  (uint32_t)&nmi);
	set_trap_gate(3,  (uint32_t)&breakpoint);
	set_trap_gate(4,  (uint32_t)&overflow);
	set_trap_gate(5,  (uint32_t)&bound);
	set_trap_gate(6,  (uint32_t)&invalid_opcode);
	set_trap_gate(7,  (uint32_t)&device_not_available);
	set_trap_gate(8,  (uint32_t)&double_fault);
	set_trap_gate(9,  (uint32_t)&coprocessor_segment_overrun);
	set_trap_gate(10, (uint32_t)&invalid_tss);
	set_trap_gate(11, (uint32_t)&segment_not_present);
	set_trap_gate(12, (uint32_t)&stack_fault);
	set_trap_gate(13, (uint32_t)&general_protection);
	set_trap_gate(14, (uint32_t)&page_fault);
	set_trap_gate(16, (uint32_t)&coprocessor_error);
	set_trap_gate(17, (uint32_t)&alignment_check);
	set_trap_gate(18, (uint32_t)&machine_check);
	set_trap_gate(19, (uint32_t)&simd_coprocessor_error);
}
