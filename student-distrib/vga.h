/* vga.h - defines functions for manipulating VGA memory and registers
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _VGA_H_
#define _VGA_H_

#include "types.h"

/* General constants related to video memory */
#define NUM_COLS 80
#define NUM_ROWS 25
#define ATTRIB 0xF0

#define VIDEO 0xB8000

/* Values related to VGA CRTC registers */
#define VGA_CRTC_ADDRESS	0x3D4
#define VGA_CRTC_DATA		0x3D5

#define VGA_CURSOR_LOC_LOW	0x0F
#define VGA_CURSOR_LOC_HIGH	0x0E

/* c structs go here */
#ifndef ASM

typedef struct screen {
	uint8_t data[NUM_ROWS * NUM_COLS * 2];
	uint8_t x;
	uint8_t y;
} screen_t;

/* location of soft cursor on the screen */
extern int screen_x, screen_y;
/* location of video memory in a flat segment */
extern char *video_mem;

/* Color Support */

extern uint8_t foreground_color;
extern uint8_t background_color;

#define COLOR_BLACK 	0x0
#define COLOR_BLUE 		0x1
#define COLOR_GREEN 	0x2
#define COLOR_CYAN		0x3
#define COLOR_RED		0x4
#define COLOR_PINK		0x5
#define COLOR_ORANGE	0x6
#define COLOR_LT_GRAY	0x7
#define COLOR_DK_GRAY	0x8
#define COLOR_PURPLE	0x9
#define COLOR_LT_GREEN	0xA
#define COLOR_LT_BLUE	0xB
#define COLOR_LT_RED	0xC
#define COLOR_LT_PINK	0xD
#define COLOR_YELLOW	0xE
#define COLOR_WHITE		0xF

#define FG_DEFAULT		COLOR_WHITE
#define BG_DEFAULT		COLOR_BLACK

void set_foreground_color(uint8_t color);
void set_background_color(uint8_t color);
void set_colors(uint8_t fg, uint8_t bg);
void set_color_palette(uint8_t palette);
void set_default_colors(void);

/*******************************/

void vga_cursor_set_location(uint8_t row, uint8_t col);
void update_cursor(void);

void screen_clear(screen_t *screen);
void screen_save(screen_t *screen);
void screen_restore(screen_t *screen);

#endif /* ASM */
#endif /* _VGA_H_ */
