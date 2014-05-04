/* vga.c - defines functions for manipulating VGA memory and registers
 * vim:ts=4 sw=4 noexpandtab
 */
#include "lib.h"
#include "vga.h"
#include "term.h"

int32_t screen_x;
int32_t screen_y;
uint8_t* video_mem = (uint8_t *)VIDEO;

vid_mem_t *fake_video_mem;

/* Sets the vga cursor to the specified position */
void vga_cursor_set_location(uint8_t row, uint8_t col)
{
	uint16_t address = row * NUM_COLS + col;
	outb(VGA_CURSOR_LOC_LOW, VGA_CRTC_ADDRESS);
	outb(address & 0x0FF, VGA_CRTC_DATA);
	outb(VGA_CURSOR_LOC_HIGH, VGA_CRTC_ADDRESS);
	outb((address >> 8) & 0x0FF, VGA_CRTC_DATA);
}

/* Sets the vga cursor to the current soft location of video memory */
void update_cursor(void)
{
	vga_cursor_set_location(screen_y, screen_x);
}

void screen_clear(screen_t *screen)
{
	int32_t i;

	/* clear video memory */
	for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
		screen->video->data[(i << 1)] = ' ';
		screen->video->data[(i << 1) + 1] = ATTRIB;
	}

	/* reset cursor position */
	screen->x = screen->y = 0;
}

void screen_save(screen_t *screen)
{
	/* copy from video memory into our buffer */
	memcpy((void *)screen->video->data, (const void *)VIDEO, sizeof screen->video->data);
}

void screen_restore(screen_t *screen)
{
	/* copy from our buffer to video memory */
	memcpy((void *)VIDEO, (void *)screen->video->data, sizeof screen->video->data);

	/* refresh cursor position */
	screen_update_cursor(screen);
}

void screen_update_cursor(screen_t *screen)
{
	vga_cursor_set_location(screen->y, screen->x);
}

