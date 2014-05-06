/* graphics.c - essential graphics for the operating system.
 * vim:ts=4 sw=4 noexpandtab
 */

#include "vga.h"
#include "lib.h"
#include "graphics.h"

/* Makes a rectangle based on the size and position given.
 * Shows our ability to make dynamic objects on screen.
 */
 void draw_rectangle(int x, int y, int width, int height, int color)
 {
	int i,j;
	int bg = background_color;
	background_color = color;

 	for(i = 0; i < width; i++)
 	{
 		for(j = 0; j < height; j++)
 		{
 			set_cursor(x+i,y+j);
 			putc(' ');
 		}
 	}

 	background_color = bg;
 }


/* Makes a frame based on the size and position given.
 * Shows our ability to make dynamic objects on screen.
 */
 void draw_frame(int x, int y, int width, int height, int color)
 {
	int i;
	int fg = foreground_color;
	foreground_color = color;

	for(i = 1; i < width-1; i++)
	{
		set_cursor(i+x,y);
		putc(CHAR_HORIZONTAL_LINE);
		set_cursor(i+x,y+height-1);
		putc(CHAR_HORIZONTAL_LINE);
	}
	for(i = 1; i < height-1; i++)
	{
		set_cursor(x,y+i);
		putc(CHAR_VERTICAL_LINE);
		set_cursor(x+width-1,y+i);
		putc(CHAR_VERTICAL_LINE);
	}
	set_cursor(x,y);
	putc(CHAR_NW_CORNER);
	set_cursor(x+width-1,y);
	putc(CHAR_NE_CORNER);
	set_cursor(x,y+height-1);
	putc(CHAR_SW_CORNER);
	set_cursor(x+width-1,y+height-1);
	putc(CHAR_SE_CORNER);


 	foreground_color = fg;
 }

/* Makes an animated singing man.
 * One of the most essential functions in the OS.
 */
 void make_the_man(int frame_duration)
 {
	int i,j;
	int MESSAGE_X = 5, MESSAGE_Y = 3;
	set_colors(COLOR_WHITE,COLOR_RED);




	/* Animate the man. */
	for (i = 4; i < NUM_COLS-1; i++)
	{
		clear();

		/* Display Message */
		set_cursor(MESSAGE_X, MESSAGE_Y);
		puts("Rebooting...");
		set_cursor(MESSAGE_X, MESSAGE_Y+1);
		puts("YesOS - \"You're a Winner.\"");


		/* Draw the man. */

		set_cursor(i%NUM_COLS,NUM_ROWS*3/4);

		if(i%2)
			putc(153);
		else putc(148);

		set_cursor(i%NUM_COLS,NUM_ROWS*3/4+1);
		putc(CHAR_FULL_BLOCK);
		set_cursor(i%NUM_COLS,NUM_ROWS*3/4+2);
		putc(CHAR_UP_T_BAR);
		set_cursor(i%NUM_COLS-1,NUM_ROWS*3/4+1);
		if(i%4 == 0) putc(CHAR_SW_CORNER);
		else if (i%4 == 2) putc(CHAR_NW_CORNER);
		else putc(CHAR_HORIZONTAL_LINE);
		set_cursor(1+i%NUM_COLS,NUM_ROWS*3/4+1);
		if(i%4 == 0) putc(CHAR_NE_CORNER);
		else if (i%4 == 2) putc(CHAR_SE_CORNER);
		else putc(CHAR_HORIZONTAL_LINE);
		
		for(j = 0; j < 3; j++)
		{
			if(i < NUM_COLS/2)
			{
				set_cursor(i%NUM_COLS-2-j,NUM_ROWS*3/4-j%2);

				if(j == i%3) putc(CHAR_MUSIC_NOTE);
				else putc(' ');
			}
			else
			{
				set_cursor(i%NUM_COLS-4,NUM_ROWS*3/4-1);

				puts("YES");
				break;
			}

		}
		
		hide_cursor();

		sleep(frame_duration);

	}

	set_cursor(i%NUM_COLS-4,NUM_ROWS*3/4);
	puts("OS");
	hide_cursor();
	sleep(500);

	set_cursor(i%NUM_COLS-4,NUM_ROWS*3/4);
	puts("  ");
	hide_cursor();
	set_cursor(NUM_COLS-6,NUM_ROWS*3/4-1);
	puts("BYE");
	hide_cursor();
	sleep(1000);

 }

 /*	 Draws YesOS splash screen
  *		Called on bootup.
  */
 void yes_os_splash(void)
 {
	clear();

	int i;
	uint8_t logo_height = 13;

	/* Center align the YesOS logo. */
	for (i = 0; i < (NUM_ROWS-logo_height)/2; ++i)
	{
		puts("\n");
	}

	/* Draw the logo and hide the cursor. */
	puts(" _________                                                            _________ ");	
	puts("[_________]__________________________________________________________[_________]");
	puts(" \\_______/____________________________________________________________\\_______/ ");
	puts("  |     |         ___   ___                    ______                  |     |  ");
	puts("  |     |        |   | |   | _____   ____     /  __  \\    ____         |     |  ");
	puts("  |     |         \\  \\ /  / |  ___| /  __|   /  /  \\  \\  /  __|        |     |  ");
	puts("  |     |          \\  V  /  | |_   |  /_    |  |    |  ||  /_          |     |  ");
	puts("  |     |           |   |   |  _|   \\_  \\   |  |    |  | \\_  \\         |     |  ");
	puts("  |     |           |   |   | |___  __/  |   \\  \\__/  /  __/  |        |     |  ");
	puts("  |     |          |_____|  |_____||____/     \\______/  |____/         |     |  ");
	puts("  |_____|______________________________________________________________|_____|  ");
	puts(" /_______\\____________________________________________________________/_______\\ ");
	puts("[_________]                                                          [_________]");

	hide_cursor();

 }

/*	Initializes status bar.
 */
 void statusbar_init()
 {
 	draw_rectangle(0,NUM_ROWS,NUM_COLS,1, COLOR_GREEN);

 }