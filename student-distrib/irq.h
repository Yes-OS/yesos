#ifndef _IRQ_H_
#define _IRQ_H_

#ifndef ASM

typedef struct registers {
	unsigned long ds, es, fs, gs;
	unsigned long edi, esi, ebp, esp, ebx, edx, ecx, eax;
	unsigned long ir_no, errno;
	unsigned long eip, cs, eflags, user_esp, ss;
} __attribute__((packed)) registers_t;

#endif

#endif // _IRQ_H_
