/* graphics.c - essential graphics for the operating system.
 * vim:ts=4 sw=4 noexpandtab
 */

#include "vga.h"
#include "lib.h"
#include "graphics.h"


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

 void make_the_man(int frame_duration)
 {
	int x = screen_x;
	int y = screen_y;
	int i,j;
	for (i = 0; i < NUM_COLS/2-1; i++)
	{
		clear();
		set_cursor(NUM_COLS/2+i%NUM_COLS,NUM_ROWS*3/4);

		if(i%2)
			putc(153);
		else putc(148);

		set_cursor(NUM_COLS/2+i%NUM_COLS,NUM_ROWS*3/4+1);
		putc(219);
		set_cursor(NUM_COLS/2+i%NUM_COLS,NUM_ROWS*3/4+2);
		putc(202);
		set_cursor(NUM_COLS/2-1+i%NUM_COLS,NUM_ROWS*3/4+1);
		if(i%4 == 0) putc(200);
		else if (i%4 == 2) putc(201);
		else putc(205);
		set_cursor(NUM_COLS/2+1+i%NUM_COLS,NUM_ROWS*3/4+1);
		if(i%4 == 0) putc(187);
		else if (i%4 == 2) putc(188);
		else putc(205);
		
		for(j = 0; j < 3; j++)
		{
			if(i < 10)
			{
			set_cursor(NUM_COLS/2-2-j+i%NUM_COLS,NUM_ROWS*3/4-j%2);

				if(j == i%3) putc(14);
				else putc(' ');
			}
			else
			{
				set_cursor(NUM_COLS/2-4+i%NUM_COLS,NUM_ROWS*3/4-1);

				puts("YES");
				break;
			}

		}
		
		hide_cursor();

		sleep(frame_duration);
	}

	sleep(1000);
	set_cursor(NUM_COLS/2-6+NUM_COLS/2,NUM_ROWS*3/4-1);
	puts("BYE");
	hide_cursor();
	sleep(500);

	set_cursor(x,y);
 }

 void yes_os_splash(void)
 {
	clear();

	int i;
	uint8_t logo_height = 13;

	for (i = 0; i < (NUM_ROWS-logo_height)/2; ++i)
	{
		puts("\n");
	}

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