/* term.c - terminal driver and related functions
 * vim:ts=4 sw=4 noexpandtab
 */

#include "lib.h"
#include "vga.h"
#include "kbd.h"
#include "queue.h"
#include "proc.h"
#include "syscall.h"
#include "term.h"

/* XXX: ENABLING AWFUL HACK BELOW */
#include "i8259.h"

/* File operations jump table */
fops_t term_fops = {
	.read  = &term_read,
	.write = &term_write,
	.open  = &term_open,
	.close = &term_close,
};


/* Defines a circular buffer for keypresses */
#define KBD_BUF_SIZE 129
DECLARE_CIRC_BUF(int8_t, term_key_buf, KBD_BUF_SIZE);

/* handle modifier keys */
int8_t lctrl_held = 0;
int8_t rctrl_held = 0;
int8_t lshift_held = 0;
int8_t rshift_held = 0;
int8_t lalt_held = 0;
int8_t ralt_held = 0;
int8_t caps_lock = 0;

/* current terminal number */
int8_t terminal_num = 0;

uint32_t chars_since_enter = 0;

/* Translate virtual keys to printable values */
static const int8_t key_values[2][128] = {
	{
		/* lowercase codes */
		   0,    0,  '`',  '1',  '2',  '3',  '4',  '5',
		 '6',  '7',  '8',  '9',  '0',  'q',  'w',  'e',
		 'r',  't',  'y',  'u',  'i',  'o',  'p',  'a',
		 's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',
		 'z',  'x',  'c',  'v',  'b',  'n',  'm',  '-',
		 '=',  '[',  ']', '\\',  ';', '\'',  ',',  '.',
		 '/',  ' ',    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		'\n', '\t', '\b',    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
	},
	{
		/* uppercase codes */
		   0,    0,  '~',  '!',  '@',  '#',  '$',  '%',
		 '^',  '&',  '*',  '(',  ')',  'Q',  'W',  'E',
		 'R',  'T',  'Y',  'U',  'I',  'O',  'P',  'A',
		 'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',
		 'Z',  'X',  'C',  'V',  'B',  'N',  'M',  '_',
		 '+',  '{',  '}',  '|',  ':',  '"',  '<',  '>',
		 '?',  ' ',    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		'\n', '\t', '\b',    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
	},
};


/* implementation of putc that uses information from a screen_t, so processes
 * can print to the screen even when their video memory's not active */
static void term_putc(screen_t *screen, uint8_t c)
{
	uint8_t *video_mem;
	uint8_t screen_x, screen_y;
	int16_t i;

	/* set values from the screen struct */
	video_mem = screen->video->data;
	screen_x = screen->x;
	screen_y = screen->y;

    if(c == '\n' || c == '\r') {
        screen_y++;
        screen_x=0;
	} else if (c == '\b') {
		/* handle backspace */
		screen_x--;
		if (screen_x < 0) {
			/* wrap backwards */
			screen_x = NUM_COLS-1;
			screen_y--;
		}
		/* clear the character */
		*(uint8_t *)(video_mem + ((NUM_COLS*screen_y + screen_x) << 1)) = ' ';
		*(uint8_t *)(video_mem + ((NUM_COLS*screen_y + screen_x) << 1) + 1) = ATTRIB;
	} else {
		if (c == '\0') {
			return;
		}
		*(uint8_t *)(video_mem + ((NUM_COLS*screen_y + screen_x) << 1)) = c;
		*(uint8_t *)(video_mem + ((NUM_COLS*screen_y + screen_x) << 1) + 1) = ATTRIB;
		screen_x++;
		screen_y = screen_y + (screen_x / NUM_COLS);
		screen_x %= NUM_COLS;
	}
	if (screen_y >= NUM_ROWS) {
		/* shift screen up */
		for (i = 0; i < (NUM_ROWS - 1) * NUM_COLS; i++) {
			*(uint8_t *)(video_mem + ((NUM_COLS*(i / NUM_COLS) + (i % NUM_COLS)) << 1)) =
				*(uint8_t *)(video_mem + ((NUM_COLS*(i / NUM_COLS + 1) + (i % NUM_COLS)) << 1));
			*(uint8_t *)(video_mem + ((NUM_COLS*(i / NUM_COLS) + (i % NUM_COLS)) << 1) + 1) =
				*(uint8_t *)(video_mem + ((NUM_COLS*(i / NUM_COLS + 1) + (i % NUM_COLS)) << 1) + 1);
		}
		/* clear last row */
		for (i = (NUM_ROWS - 1) * NUM_COLS; i < NUM_ROWS * NUM_COLS; i++) {
			*(uint8_t *)(video_mem + ((NUM_COLS*(i / NUM_COLS) + (i % NUM_COLS)) << 1)) = ' ';
			*(uint8_t *)(video_mem + ((NUM_COLS*(i / NUM_COLS) + (i % NUM_COLS)) << 1) + 1) = ATTRIB;
		}
		screen_y = NUM_ROWS - 1;
	}
}

/* open terminal fd, returns STDIN */
int32_t term_open(const uint8_t *filename)
{
	(void)filename;

	/* initialize the keyboard buffer */
	CIRC_BUF_INIT(term_key_buf);

	/* clear the status flags */
	lctrl_held = 0;
	rctrl_held = 0;
	lshift_held = 0;
	rshift_held = 0;
	lalt_held = 0;
	ralt_held = 0;
	caps_lock = 0;

	/* Set the current terminal number */
	terminal_num = 0;

	chars_since_enter = 0;

	return STDIN;
}

/* closes terminal fd, fails always */
int32_t term_close(int32_t fd)
{
	(void)fd;
	return -1;
}

/* if fd is STDIN, proceed as normal, otherwise fail */
int32_t term_read(int32_t fd, void *buf, int32_t nbytes)
{
	int idx = 0;
	int ok;
	int8_t c = 0;
	int8_t *buffer = (int8_t *)buf;

	if (fd != STDIN) {
		return -1;
	}

	do {
		/* dequeue a character and test for backspace and screen clear */
		CIRC_BUF_POP(term_key_buf, c, ok);
		if (ok) {
			if (c == '\b') {
				if (idx > 0) {
					idx--;
					putc((int8_t)c);
					update_cursor();
				}
			}
			else if (c == KBD_KEY_NULL) {
				idx = 0;
			}
			else {
				/* push the character into the buffer */
				buffer[idx++] = c;
			}
		}
		else {
			/* no data, sleep */
			asm ("hlt");
		}
	} while (c != '\n' && idx < nbytes);

	/* prevent backspace past last feed */
	chars_since_enter = 0;

	return idx;
}

/* if fd is STDOUT, proceed as normal, otherwise fail */
int32_t term_write(int32_t fd, const void *buf, int32_t nbytes)
{
	int idx;
	pcb_t *pcb;

	if (fd != STDOUT) {
		return -1;
	}

	pcb = get_proc_pcb();
	if (!pcb) {
		return -1;
	}

	for (idx = 0; idx < nbytes; idx++) {
		term_putc(&pcb->screen, ((int8_t *)buf)[idx]);
	}

	update_cursor();

	return idx;
}

void term_handle_keypress(uint16_t key, uint8_t status)
{
	int ok;
	int c;
	/* just echo the key value for now, we'll handle things specifically later */
	if (status) {
		if ((lctrl_held || rctrl_held) && key == KBD_KEY_L) {
			/* clear the screen, update the cursor, and clear the key buffer */
			clear();
			update_cursor();
			chars_since_enter = 0;
			CIRC_BUF_INIT(term_key_buf);
			CIRC_BUF_PUSH(term_key_buf, KBD_KEY_NULL, ok);
			return;
		}
		if ( ((lctrl_held || rctrl_held) && key == KBD_KEY_C) || 
			((lctrl_held || rctrl_held) && (lalt_held || ralt_held) && key == KBD_KEY_DEL) ) {
			/* kill a process, should be replaced later by signals */
			if (nprocs > 0) {
				/* XXX: AWFUL HACK */
				send_eoi(KBD_IRQ_PORT);
				sys_halt(-1);
			}
		}
		if ((lalt_held || ralt_held) && (key >= KBD_KEY_F1 && key <= KBD_KEY_F4)) {
			/* Get the value of the  new terminal */
			int8_t new_terminal_num = key - KBD_KEY_F1;

			/* If the new terminal is the same, do not switch. */
			if (new_terminal_num == terminal_num) {
				return;
			}

			/* Copy current terminal's video memory into its fake video memory. */

			/* Tell the current terminal that it's video memory has been relocated */

			/* Store the input information for that terminal. (Keyboard buffer) */


			/* Check to see if new terminal has video memory to copy. */

			/* If not, create a blank screen for the new terminal. */

			/* Check to see if new terminal has any input information to load. (Keyboard buffer) */


			/* Set the terminal number to the new terminal. */
			terminal_num = new_terminal_num;
			return;
		}
		switch (key) {
			case KBD_KEY_LCTRL:
				lctrl_held = 1;
				break;
			case KBD_KEY_RCTRL:
				rctrl_held = 1;
				break;
			case KBD_KEY_LSHIFT:
				lshift_held = 1;
				break;
			case KBD_KEY_RSHIFT:
				rshift_held = 1;
				break;
			case KBD_KEY_LALT:
				lalt_held = 1;
				break;
			case KBD_KEY_RALT:
				ralt_held = 1;
				break;
			case KBD_KEY_CAPS:
				caps_lock ^= 1;
				break;
			default:
				if (key < 128) {
					/* printable */
					/* toggle alphanumeric when capslock is on */
					if (caps_lock && (key >= KBD_KEY_Q && key <= KBD_KEY_M)) {
						key = key_values[caps_lock ^ (lshift_held | rshift_held)][key];
					}
					else {
						/* handle symbols as normal */
						key = key_values[lshift_held | rshift_held][key];
					}
					if (key) {
						if (key == '\b') {
							CIRC_BUF_PEEK_TAIL(term_key_buf, c, ok);
							if (ok) {
								if (c == KBD_KEY_NULL) {
									/* remove the character because it's technically not needed,
									 * but don't emit a backspace character since it's invisible */
									CIRC_BUF_POP_TAIL(term_key_buf, c, ok);
								}
								else if (c != '\n' && c != '\b') {
									CIRC_BUF_POP_TAIL(term_key_buf, c, ok);
									putc((int8_t)key);
								}
							}
							else {
								CIRC_BUF_PUSH(term_key_buf, key, ok);
							}
						}
						else {
							CIRC_BUF_PUSH(term_key_buf, key, ok);
							if (ok) {
								putc((int8_t)key);
							}
						}
						update_cursor();
					}
				}
				break;
		}
	}
	else {
		switch (key) {
			case KBD_KEY_LCTRL:
				lctrl_held = 0;
				break;
			case KBD_KEY_RCTRL:
				rctrl_held = 0;
				break;
			case KBD_KEY_LSHIFT:
				lshift_held = 0;
				break;
			case KBD_KEY_RSHIFT:
				rshift_held = 0;
				break;
			case KBD_KEY_LALT:
				lalt_held = 0;
				break;
			case KBD_KEY_RALT:
				ralt_held = 0;
				break;
			default:
				break;
		}
	}
}

