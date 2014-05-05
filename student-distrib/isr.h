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

/* Registers struct used mainly during interrupt handling */
typedef struct registers {
	uint32_t ebx, ecx, edx, esi, edi, ebp, eax;
	uint32_t ds, es, fs;
	uint32_t isrno, errno;
	uint32_t eip, cs, eflags, user_esp, ss;
} __attribute__((packed)) registers_t;


/****************************************
 *         Function Declarations        *
 ****************************************/

/* Implements the interrupt service request */
void isr_impl(registers_t);

/* Sets up an interrupt gate */
void set_intr_gate(uint8_t,uint32_t);

/* Sets up a system gate */
void set_system_gate(uint8_t,uint32_t);

/* Sets up a system related interrupt gate */
void set_system_intr_gate(uint8_t,uint32_t);

/* Sets up a trap interrupt gate */
void set_trap_gate(uint8_t,uint32_t);

/* Sets up a task interrupt gate */
void set_task_gate(uint8_t,uint16_t);

/* Sets up all of the gates needed for interrupt/exception handling */
void install_interrupts();

#endif

#endif // _IRQ_H_
