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

/* global terminal context, used by the kernel */
term_t term_global_ctx;
static screen_t kern_screen;

/* current terminal number */
int8_t terminal_num = 0;

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
	int16_t i;

    if(c == '\n' || c == '\r') {
        screen->y++;
        screen->x=0;
	} else if (c == '\b') {
		/* handle backspace */
		screen->x--;
		if (screen->x < 0) {
			/* wrap backwards */
			screen->x = NUM_COLS-1;
			screen->y--;
		}
		/* clear the character */
		*(uint8_t *)(screen->video->data + ((NUM_COLS*screen->y + screen->x) << 1)) = ' ';
		*(uint8_t *)(screen->video->data + ((NUM_COLS*screen->y + screen->x) << 1) + 1) = ATTRIB;
	} else {
		if (c == '\0') {
			return;
		}
		*(uint8_t *)(screen->video->data + ((NUM_COLS*screen->y + screen->x) << 1)) = c;
		*(uint8_t *)(screen->video->data + ((NUM_COLS*screen->y + screen->x) << 1) + 1) = ATTRIB;
		screen->x++;
		screen->y = screen->y + (screen->x / NUM_COLS);
		screen->x %= NUM_COLS;
	}
	if (screen->y >= NUM_ROWS) {
		/* shift screen up */
		for (i = 0; i < (NUM_ROWS - 1) * NUM_COLS; i++) {
			*(uint8_t *)(screen->video->data + ((NUM_COLS*(i / NUM_COLS) + (i % NUM_COLS)) << 1)) =
				*(uint8_t *)(screen->video->data + ((NUM_COLS*(i / NUM_COLS + 1) + (i % NUM_COLS)) << 1));
			*(uint8_t *)(screen->video->data + ((NUM_COLS*(i / NUM_COLS) + (i % NUM_COLS)) << 1) + 1) =
				*(uint8_t *)(screen->video->data + ((NUM_COLS*(i / NUM_COLS + 1) + (i % NUM_COLS)) << 1) + 1);
		}
		/* clear last row */
		for (i = (NUM_ROWS - 1) * NUM_COLS; i < NUM_ROWS * NUM_COLS; i++) {
			*(uint8_t *)(screen->video->data + ((NUM_COLS*(i / NUM_COLS) + (i % NUM_COLS)) << 1)) = ' ';
			*(uint8_t *)(screen->video->data + ((NUM_COLS*(i / NUM_COLS) + (i % NUM_COLS)) << 1) + 1) = ATTRIB;
		}
		screen->y = NUM_ROWS - 1;
	}
}

static void init_ctx(term_t *term_ctx)
{
	/* initialize the keyboard buffer */
	CIRC_BUF_INIT(term_ctx->term_key_buf);

	/* clear the status flags */
	term_ctx->lctrl_held = 0;
	term_ctx->rctrl_held = 0;
	term_ctx->lshift_held = 0;
	term_ctx->rshift_held = 0;
	term_ctx->lalt_held = 0;
	term_ctx->ralt_held = 0;
	term_ctx->caps_lock = 0;
}

int32_t term_init_global_ctx()
{
	init_ctx(&term_global_ctx);
	terminal_num = 0;
	return 0;
}

/* open terminal fd, returns STDIN */
int32_t term_open(const uint8_t *filename)
{
	(void)filename; /* we don't use the filename for terminal */
	pcb_t *pcb;
	screen_t *screen;

	/* XXX: no bueno, see relevant call in sys_exec */
	pcb = (pcb_t *)filename;
	if (!pcb) {
		return -1;
	}

	screen = get_screen_ctx();
	if (!screen) {
		/* edge case where this must be the first shell */
		screen = &pcb->screen;
	}

	init_ctx(&pcb->term_ctx);

	{
		/* set up stdin/stdio fds */
		file_t file;
		file.flags = FILE_PRESENT | FILE_OPEN;
		file.file_op = &term_fops;
		file.file_pos = 0;
		file.inode_ptr = 0;

		pcb->file_array[0] = file;
		pcb->file_array[1] = file;
	}

	/* set cursor location based on the current cursor location */
	screen->x = screen_x;
	screen->y = screen_y;

	/* flush the cursor location just to be safe */
	screen_update_cursor(screen);

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
	pcb_t *pcb;
	term_t *term_ctx;
	screen_t *screen;
	int idx = 0;
	int ok;
	int8_t c = 0;
	int8_t *buffer = (int8_t *)buf;

	if (fd != STDIN) {
		return -1;
	}

	pcb = get_proc_pcb();
	if (!pcb) {
		return -1;
	}

	screen = get_screen_ctx();
	if (!screen) {
		return -1;
	}

	term_ctx = &pcb->term_ctx;

	do {
		/* dequeue a character and test for backspace and screen clear */
		CIRC_BUF_POP(term_ctx->term_key_buf, c, ok);
		if (ok) {
			if (c == '\b') {
				if (idx > 0) {
					idx--;
					term_putc(screen, (int8_t)c);
					screen_update_cursor(screen);
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

	return idx;
}

/* if fd is STDOUT, proceed as normal, otherwise fail */
int32_t term_write(int32_t fd, const void *buf, int32_t nbytes)
{
	int idx;
	screen_t *screen;

	if (fd != STDOUT) {
		return -1;
	}

	screen = get_screen_ctx();
	if (!screen) {
		return -1;
	}

	for (idx = 0; idx < nbytes; idx++) {
		term_putc(screen, ((int8_t *)buf)[idx]);
	}

	/* update based on screen location */
	screen_update_cursor(screen);

	return idx;
}

void term_handle_keypress(uint16_t key, uint8_t status)
{
	pcb_t *pcb;
	screen_t *screen;
	term_t *term_ctx;
	int ok;
	int c;

	pcb = get_proc_pcb();
	if (pcb) {
		/* if we're running a process, grab the process's ctx */
		/* TODO: this should probably be the foreground process so we don't
		 *       experience weird results sending keys to programs that aren't
		 *       visible */
		term_ctx = &pcb->term_ctx;
		screen = get_screen_ctx();
		if (!screen) {
			/* shouldn't really happen */
			return;
		}
	}
	else {
		/* otherwise, use the one given to the kernel */
		term_ctx = &term_global_ctx;
		screen = &kern_screen;
		kern_screen.video = (vid_mem_t *)VIDEO;
		kern_screen.x = screen_x;
		kern_screen.y = screen_y;
	}

	/* just echo the key value for now, we'll handle things specifically later */
	if (status) {
		if ((term_ctx->lctrl_held || term_ctx->rctrl_held) && key == KBD_KEY_L) {
			/* clear the screen, update the cursor, and clear the key buffer */
			screen_clear(screen);
			screen_update_cursor(screen);
			CIRC_BUF_INIT(term_ctx->term_key_buf);
			CIRC_BUF_PUSH(term_ctx->term_key_buf, KBD_KEY_NULL, ok);
			return;
		}
		if ( ((term_ctx->lctrl_held || term_ctx->rctrl_held) && key == KBD_KEY_C) ||
				((term_ctx->lctrl_held || term_ctx->rctrl_held) &&
				(term_ctx->lalt_held || term_ctx->ralt_held) && key == KBD_KEY_DEL) ) {
			/* kill a process, should be replaced later by signals */
			if (nprocs > 0) {
				/* XXX: AWFUL HACK */
				send_eoi(KBD_IRQ_PORT);
				sys_halt(-1);
			}
		}
		if ((term_ctx->lalt_held || term_ctx->ralt_held) && (key >= KBD_KEY_F1 && key <= KBD_KEY_F4)) {
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
				term_ctx->lctrl_held = 1;
				break;
			case KBD_KEY_RCTRL:
				term_ctx->rctrl_held = 1;
				break;
			case KBD_KEY_LSHIFT:
				term_ctx->lshift_held = 1;
				break;
			case KBD_KEY_RSHIFT:
				term_ctx->rshift_held = 1;
				break;
			case KBD_KEY_LALT:
				term_ctx->lalt_held = 1;
				break;
			case KBD_KEY_RALT:
				term_ctx->ralt_held = 1;
				break;
			case KBD_KEY_CAPS:
				term_ctx->caps_lock ^= 1;
				break;
			default:
				if (key < 128) {
					/* printable */
					/* toggle alphanumeric when capslock is on */
					if (term_ctx->caps_lock && (key >= KBD_KEY_Q && key <= KBD_KEY_M)) {
						key = key_values[term_ctx->caps_lock ^
							(term_ctx->lshift_held | term_ctx->rshift_held)][key];
					}
					else {
						/* handle symbols as normal */
						key = key_values[term_ctx->lshift_held | term_ctx->rshift_held][key];
					}
					if (key) {
						if (key == '\b') {
							CIRC_BUF_PEEK_TAIL(term_ctx->term_key_buf, c, ok);
							if (ok) {
								if (c == KBD_KEY_NULL) {
									/* remove the character because it's technically not needed,
									 * but don't emit a backspace character since it's invisible */
									CIRC_BUF_POP_TAIL(term_ctx->term_key_buf, c, ok);
								}
								else if (c != '\n' && c != '\b') {
									CIRC_BUF_POP_TAIL(term_ctx->term_key_buf, c, ok);
									term_putc(screen, (int8_t)key);
								}
							}
							else {
								CIRC_BUF_PUSH(term_ctx->term_key_buf, key, ok);
							}
						}
						else {
							CIRC_BUF_PUSH(term_ctx->term_key_buf, key, ok);
							if (ok) {
								term_putc(screen, (int8_t)key);
							}
						}
						screen_update_cursor(screen);
					}
				}
				break;
		}
	}
	else {
		switch (key) {
			case KBD_KEY_LCTRL:
				term_ctx->lctrl_held = 0;
				break;
			case KBD_KEY_RCTRL:
				term_ctx->rctrl_held = 0;
				break;
			case KBD_KEY_LSHIFT:
				term_ctx->lshift_held = 0;
				break;
			case KBD_KEY_RSHIFT:
				term_ctx->rshift_held = 0;
				break;
			case KBD_KEY_LALT:
				term_ctx->lalt_held = 0;
				break;
			case KBD_KEY_RALT:
				term_ctx->ralt_held = 0;
				break;
			default:
				break;
		}
	}

	/* updates current global cursor, need to make sure we only print to current terminal */
	screen_x = screen->x;
	screen_y = screen->y;
}

