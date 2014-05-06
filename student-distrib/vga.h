/* vga.h - defines functions for manipulating VGA memory and registers
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _VGA_H_
#define _VGA_H_

#include "types.h"

/****************************************
 *            Global Defines            *
 ****************************************/

/* General constants related to video memory */
#define NUM_COLS 80
#define NUM_ROWS 25


#define VIDEO 0xB8000

/* Values related to VGA CRTC registers */
#define VGA_CRTC_ADDRESS      0x3D4
#define VGA_CRTC_DATA         0x3D5

#define VGA_CURSOR_LOC_LOW    0x0F
#define VGA_CURSOR_LOC_HIGH   0x0E

#define SIZE_64K              0x10000

#ifndef ASM

/****************************************
 *              Data Types              *
 ****************************************/

typedef struct vid_mem {	
	/* 64 kb video memory */
	uint8_t data[SIZE_64K];
} __attribute__((packed)) vid_mem_t;

typedef struct screen {
	vid_mem_t *video;
	int32_t x;
	int32_t y;
} screen_t;


/****************************************
 *           Global Variables           *
 ****************************************/

/* location of soft cursor on the screen */
extern int32_t screen_x, screen_y;
/* location of video memory in a flat segment */
extern uint8_t *video_mem;

extern vid_mem_t *fake_video_mem;


/****************************************
 *         Function Declarations        *
 ****************************************/

/* Set the location of the VGA's cursor */
void vga_cursor_set_location(uint8_t row, uint8_t col);

/* Check global data and update the cursor */
void update_cursor(void);

/* Clean the screen of all video data */
void screen_clear(screen_t *screen);

/* Save the current video data to a passed screen */
void screen_save(screen_t *screen);

/* Restore a saved screen to the current video data */
void screen_restore(screen_t *screen);

/* Update the cursor of a passed screen */
void screen_update_cursor(screen_t *screen);

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


void set_colors(uint8_t fg, uint8_t bg);
void set_color_palette(uint8_t palette);
void set_default_colors(void);

/*******************************/

void set_cursor(uint8_t x, uint8_t y);
void hide_cursor();


#endif /* ASM */
#endif /* _VGA_H_ */
