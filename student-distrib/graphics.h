/* graphics.h - essential graphics for the operating system.
 * vim:ts=4 sw=4 noexpandtab
 */

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_



#include "vga.h"
#include "lib.h"

/* ASCII Char Defines */
#define CHAR_FULL_BLOCK			219
#define CHAR_VERTICAL_LINE		186
#define CHAR_HORIZONTAL_LINE	205
#define CHAR_NW_CORNER			201
#define CHAR_NE_CORNER			187
#define CHAR_SW_CORNER			200
#define CHAR_SE_CORNER			188


/* Graphics functions */
void make_the_man(int frame_duration);
void yes_os_splash(void);
void draw_rectangle(int x, int y, int width, int height, int color);
void draw_frame(int x, int y, int width, int height, int color);



#endif