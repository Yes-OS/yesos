/* irq.h, implement interrupt request handling and related functions
 * vim:ts=4 noexpandtab
 */
#ifndef _IRQ_H_
#define _IRQ_H_

#ifndef ASM

#include "types.h"

typedef struct registers {
	uint32_t ds, es, fs, gs;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t irqno, errno;
	uint32_t eip, cs, eflags, user_esp, ss;
} __attribute__((packed)) registers_t;

extern void isr_impl(registers_t);

extern void set_intr_gate(uint8_t,uint32_t);
extern void set_system_gate(uint8_t,uint32_t);
extern void set_system_intr_gate(uint8_t,uint32_t);
extern void set_trap_gate(uint8_t,uint32_t);

#endif

#endif // _IRQ_H_
