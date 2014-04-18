/* vga.c - defines functions for manipulating VGA memory and registers
 * vim:ts=4 sw=4 noexpandtab
 */
#include "lib.h"
#include "vga.h"

int screen_x;
int screen_y;
char* video_mem = (char *)VIDEO;

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

void clear_screen(screen_t *screen)
{
	int32_t i;

	/* clear video memory */
	for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
		screen->data[(i << 1)] = ' ';
		screen->data[(i << 1) + 1] = ATTRIB;
	}

	/* reset cursor position */
	screen->x = screen->y = 0;
}

void save_screen(screen_t *screen)
{
	int32_t i;

	/* copy from video memory into our buffer */
	for (i = 0; i < NUM_ROWS * NUM_COLS * 2; i++) {
		screen->data[i] = *((uint8_t *)video_mem + i);
	}

	/* save cursor positions */
	screen->x = screen_x;
	screen->y = screen_y;
}

void restore_screen(screen_t *screen)
{
	int32_t i;

	/* copy from our buffer to video memory */
	for (i = 0; i < NUM_ROWS * NUM_COLS * 2; i++) {
		*((uint8_t *)video_mem + i) = screen->data[i];
	}

	/* restore cursor position */
	screen_x = screen->x;
	screen_y = screen->y;

	/* refresh cursor position */
	update_cursor();
}

