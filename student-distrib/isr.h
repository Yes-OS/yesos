/* isr.h, implement interrupt request handling and related functions
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _IRQ_H_
#define _IRQ_H_

#ifndef ASM

#include "types.h"

/****************************************
 *              Data Types              *
 ****************************************/

typedef struct registers {
	uint32_t ebx, ecx, edx, esi, edi, ebp, eax;
	uint32_t ds, es, fs;
	uint32_t isrno, errno;
	uint32_t eip, cs, eflags, user_esp, ss;
} __attribute__((packed)) registers_t;


/****************************************
 *         Function Declarations        *
 ****************************************/

void isr_impl(registers_t);

void set_intr_gate(uint8_t,uint32_t);
void set_system_gate(uint8_t,uint32_t);
void set_system_intr_gate(uint8_t,uint32_t);
void set_trap_gate(uint8_t,uint32_t);
void set_task_gate(uint8_t,uint16_t);

void install_interrupts();

#endif

#endif // _IRQ_H_
