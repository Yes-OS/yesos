# x86_desc.S - Set up x86 segment descriptors, descriptor tables 
# vim:ts=4 noexpandtab

#define ASM     1
#include "x86_desc.h"

.text

.globl  ldt_size, tss_size, gdt_size
.globl  gdt_desc, ldt_desc, tss_desc
.globl  tss, tss_desc_ptr, ldt, ldt_desc_ptr
.globl  gdt_ptr, gdtr
.globl  idt_desc_ptr, idt

.extern isr_impl

.align 4


tss_size:
	.long tss_bottom - tss - 1

ldt_size:
	.long ldt_bottom - ldt - 1

/* GDT size is one fewer than the end pointer */
#define GDT_SIZE gdt_bottom - gdt - 1
gdt_size:
	.word GDT_SIZE

/* limit is the GDT size */
gdtr:
	.word GDT_SIZE 
	.long gdt

	.align 4

	.word 0 # Padding
ldt_desc:
	.word KERNEL_LDT
	.long ldt

	.align 4
tss:
_tss:
	.rept 104
	.byte 0
	.endr
tss_bottom:

	.align  16
gdt:
_gdt:

	# First GDT entry cannot be used
	.quad 0
	
	# NULL entry
	.quad 0
	
	# Segmentation will not be used
	# CS and DS both are 0-4GB r/w segments
	#
	# The layout is (from Intel IA-32 reference manual):
	#  31        24 23  22  21  20  19   16 15  14 13 12  11   8 7          0
	# |----------------------------------------------------------------------|
	# |            |   | D |   | A |  Seg  |   |  D  |   |      |            |
	# | Base 31:24 | G | / | 0 | V | Limit | P |  P  | S | Type | Base 23:16 |
	# |            |   | B |   | L | 19:16 |   |  L  |   |      |            |
	# |----------------------------------------------------------------------|
	#
	# |----------------------------------------------------------------------|
	# |                                    |                                 |
	# | Base 15:0                          | Segment Limit 15:0              |
	# |                                    |                                 |
	# |----------------------------------------------------------------------|
	
gdt_ptr:
	# Set up an entry for kernel CS
	.quad 0x00CF9A000000FFFF

	# Set up an entry for kernel DS
	.quad 0x00CF92000000FFFF

	# Set up an entry for user CS
	.quad 0x00CFFA000000FFFF

	# Set up an entry for user DS
	.quad 0x00CFF2000000FFFF

	# Set up an entry for TSS
tss_desc_ptr:
	.quad 0

	# Set up one LDT
ldt_desc_ptr:
	.quad 0

gdt_bottom:

	.align 16
ldt:
	.rept 4
	.quad 0
	.endr
ldt_bottom:

.align 4
	.word 0 # Padding
idt_desc_ptr:
	.word idt_bottom - idt - 1
	.long idt
	

	.align  16
idt:
_idt:
	.rept NUM_VEC
	.quad 0
	.endr

idt_bottom:

# Interrupt handlers

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


device_not_avilable:
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
	pushw	%gs
	pushw	%fs
	pushw	%es
	pushw	%ds

	movw	$KERNEL_DS, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs

	call	isr_impl

	# restore segments
	popw	%ds
	popw	%es
	popw	%fs
	popw	%gs

	popal
	addl	$8, %esp	# clean up error code and ISR number
	sti
	iret

