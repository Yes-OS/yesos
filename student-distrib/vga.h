/* vga.h - defines functions for manipulating VGA memory and registers
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _VGA_H_
#define _VGA_H_

#include "types.h"

/* General constants related to video memory */
#define NUM_COLS 80
#define NUM_ROWS 25

#define VIDEO 0xB8000

/* Values related to VGA CRTC registers */
#define VGA_CRTC_ADDRESS	0x3D4
#define VGA_CRTC_DATA		0x3D5

#define VGA_CURSOR_LOC_LOW	0x0F
#define VGA_CURSOR_LOC_HIGH	0x0E

/* c structs go here */
#ifndef ASM
extern void vga_cursor_set_location(uint8_t row, uint8_t col);
#endif /* ASM */

#endif /* _VGA_H_ */
