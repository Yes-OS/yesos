/* vga.c - defines functions for manipulating VGA memory and registers
 * vim:ts=4 sw=4 noexpandtab
 */
#include "lib.h"
#include "vga.h"
#include "term.h"

int32_t screen_x;
int32_t screen_y;
uint8_t* video_mem = (uint8_t *)VIDEO;
uint8_t foreground_color = COLOR_WHITE;
uint8_t background_color = COLOR_BLACK;

vid_mem_t *fake_video_mem;

/* Sets the vga cursor to the specified position 
 * INPUTS: row - row to set the cursor to
 *         col - column to set the cursor to
 */
void vga_cursor_set_location(uint8_t row, uint8_t col)
{
	uint16_t address = row * NUM_COLS + col;
	outb(VGA_CURSOR_LOC_LOW, VGA_CRTC_ADDRESS);
	outb(address & 0x0FF, VGA_CRTC_DATA);
	outb(VGA_CURSOR_LOC_HIGH, VGA_CRTC_ADDRESS);
	outb((address >> 8) & 0x0FF, VGA_CRTC_DATA);
}

/* Sets the vga cursor to the current soft location of video memory 
 */
void update_cursor(void)
{
	vga_cursor_set_location(screen_y, screen_x);
}

/* Clear the video memory of a passed screen
 * INPUTS: screen - pointer to the screen to clear
 */
void screen_clear(screen_t *screen)
{
	int32_t i;

	/* clear video memory */
	for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
		screen->video->data[(i << 1)] = ' ';
		screen->video->data[(i << 1) + 1] = ((background_color << 4) + foreground_color);
	}

	/* reset cursor position */
	screen->x = screen->y = 0;
}

/* Save the video memory of a passed screen
 * INPUTS: screen - pointer to the screen to save
 */
void screen_save(screen_t *screen)
{
	/* copy from video memory into our buffer */
	memcpy((void *)screen->video->data, (const void *)VIDEO, sizeof screen->video->data);
}

/* Restore the video memory of a screen to the active video memory
 * INPUTS: screen - pointer to the screen to restore
 */
void screen_restore(screen_t *screen)
{
	/* copy from our buffer to video memory */
	memcpy((void *)VIDEO, (void *)screen->video->data, sizeof screen->video->data);

	/* refresh cursor position */
	screen_update_cursor(screen);
}

/* Update the cursor of a passed screen to its own global data
 * INPUTS: screen - pointer to the screen that we want to update its cursor
 */
void screen_update_cursor(screen_t *screen)
{
	vga_cursor_set_location(screen->y, screen->x);
}

/* Set foreground color to desired color.
 * INPUTS: color - the color you want to use.
 */
void set_foreground_color(uint8_t color)
{
	foreground_color = color;
}

/* Set background color to desired color.
 * INPUTS: color - the color you want to use.
 */
void set_background_color(uint8_t color)
{
	background_color = color;
}

/* Sets foreground and background colors to desired colors.
 * INPUTS: fg - the color you want to use for foreground.
 		   bg - the color you want to use for background.
 */
void set_colors(uint8_t fg, uint8_t bg)
{
	foreground_color = fg;
	background_color = bg;
}

/* Set colors to their defaults.
 */
void set_default_colors()
{
	foreground_color = FG_DEFAULT;
	background_color = BG_DEFAULT;
}

/* Set location of the cursor.
 * INPUTS: x - the x position.
 *		   y - the y position.
 */
void set_cursor(uint8_t x, uint8_t y)
{
	screen_x = x;
	screen_y = y;
	update_cursor();
}

/* Set location of the cursor to a hidden region.
 */
void hide_cursor()
{
	screen_x = -1;
	screen_y = -1;
	update_cursor();
}

