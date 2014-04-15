/* kbd.c, ps/2 keyboard driver implementation
 * vim:ts=4 sw=4 noexpandtab
 */
#include "types.h"
#include "lib.h"
#include "vga.h"
#include "queue.h"
#include "term.h"
#include "kbd.h"

/* the PS/2 ports */
#define PS2_PORT_DATA					0x60
#define PS2_PORT_STATUS					0x64
#define PS2_PORT_COMMAND				0x64

/* the PS/2 keyboard commands */
#define PS2_CMD_RESET					0xFF

/* the PS/2 controller commands */
#define PS2_CMD_DISABLE_PORT1			0xAD
#define PS2_CMD_ENABLE_PORT1			0xAE
#define PS2_CMD_DISABLE_PORT2			0xA7
#define PS2_CMD_ENABLE_PORT2			0xA8
#define PS2_CMD_CONTROLLER_TEST			0xAA
#define PS2_CMD_READ_CONFIGURATION		0x20
#define PS2_CMD_WRITE_CONFIGURATION		0x60
#define PS2_CMD_TEST_PORT1				0xAB

/* the PS/2 response codes */
#define PS2_RET_NAK						0xFE
#define PS2_RET_ATTACH					0xAA

/* the PS/2 controller configuration */
#define PS2_CFG_INT_PORT1				(1 << 0)
#define PS2_CFG_INT_PORT2				(1 << 1)
#define PS2_CFG_SYSTEM					(1 << 2)
#define PS2_CFG_UNUSED1					(1 << 3)
#define PS2_CFG_CLK_PORT1				(1 << 4)
#define PS2_CFG_CLK_PORT2				(1 << 5)
#define PS2_CFG_TRANS_PORT1				(1 << 6)
#define PS2_CFG_UNUSED2					(1 << 7)

/* the PS/2 controller status bits */
#define PS2_STATUS_OBUFFER_FULL			(1 << 0)
#define PS2_STATUS_IBUFFER_FULL			(1 << 1)
#define PS2_STATUS_SYSTEM_FLAG			(1 << 2)
#define PS2_STATUS_COMMAND				(1 << 3)
#define PS2_STATUS_UKN1					(1 << 4)
#define PS2_STATUS_UKN2					(1 << 5)
#define PS2_STATUS_TIME_OUT				(1 << 6)
#define PS2_STATUS_PARITY_ERR			(1 << 7)


/* define functions to read and write safely */

/* writes a command to the PS/2 controller */
#define ps2_write_command(cmd)                         \
	do {                                               \
		uint32_t status;                               \
		                                               \
		/* wait until we can write */                  \
		do {                                           \
			status = inb(PS2_PORT_STATUS);             \
		} while (status & PS2_STATUS_IBUFFER_FULL);    \
                                                       \
		/* write command */                            \
		outb((cmd), PS2_PORT_COMMAND);                 \
	} while (0)

/* writes data to the data port */
#define ps2_write_data(data)                           \
	do {                                               \
		uint32_t status;                               \
		                                               \
		/* wait until we can write */                  \
		do {                                           \
			status = inb(PS2_PORT_STATUS);             \
		} while (status & PS2_STATUS_IBUFFER_FULL);    \
                                                       \
		/* write command */                            \
		outb((data), PS2_PORT_DATA);                   \
	} while (0)

/* block while reading a byte from the data port */
static inline uint32_t ps2_read_data()
{
	uint32_t data;
	uint32_t status;

	/* wait until data available */
	do {
		status = inb(PS2_PORT_STATUS);
	} while (!(status & PS2_STATUS_OBUFFER_FULL));

	data = inb(PS2_PORT_DATA);
	return data;
}

/* the scancode to ascii table */
#define SCAN_ENTRIES 512
const static uint16_t scancodes[SCAN_ENTRIES] = {
	#include "scancodes.h"
};

/* sent when a key is sent */
#define KBD_KEY_RELEASED			0xf0
#define KBD_KEY_EMUL0				0xe0
#define KBD_KEY_EMUL1				0xe1


/* Defines a circular FIFO command queue for the PS/2 keyboard */
#define CMD_QUEUE_LEN 64
DECLARE_CIRC_BUF(uint8_t, kbd_cmd_queue, CMD_QUEUE_LEN);
uint8_t emul = 0;
uint8_t released = 0;

static int16_t kbd_combine_key(int16_t value);


void kbd_init()
{
	uint32_t read;

	CIRC_BUF_INIT(kbd_cmd_queue);
	/* awful hack to handle first 0xAA sent */
	CIRC_BUF_PUSH(kbd_cmd_queue, PS2_CMD_RESET, read);

	/* disable all PS/2 devices */
	ps2_write_command(PS2_CMD_DISABLE_PORT1);
	ps2_write_command(PS2_CMD_DISABLE_PORT2);

	/* flush output buffer? */
	read = inb(PS2_PORT_DATA);

	/* get controller configuration byte */
	ps2_write_command(PS2_CMD_READ_CONFIGURATION);
	read = ps2_read_data();

	/* disable interrupts and keycode translation */
	read &= ~(PS2_CFG_INT_PORT1 | PS2_CFG_INT_PORT2 | PS2_CFG_TRANS_PORT1);

	/* write back to the controller */
	ps2_write_command(PS2_CMD_WRITE_CONFIGURATION);
	ps2_write_data(read);

	/* test the controller */
	/* or not since qemu doesn't seem to have a ps/2 controller */
#if 0
	ps2_write_data(PS2_CMD_CONTROLLER_TEST);
	read = ps2_read_data();
	if (read != 0x55) {
		printf("Welp, we're borked! [0x%x]\n", read);
		//return;
	}
#endif

	/* test first ps/2 port */
	ps2_write_command(PS2_CMD_TEST_PORT1);
	read = ps2_read_data();
	if (read != 0) {
		printf("Welp, we're borked for other reasons! [0x%x]\n", read);
		return;
	}

	/* enable ps/2 port 1 */
	ps2_write_command(PS2_CMD_ENABLE_PORT1);
	ps2_write_command(PS2_CMD_READ_CONFIGURATION);
	read = ps2_read_data();
	read |= PS2_CFG_INT_PORT1;
	ps2_write_command(PS2_CMD_WRITE_CONFIGURATION);
	ps2_write_data(read);

	/* reset device on port 1 */
	kbd_reset();
}

/* Sends the reset command to the keyboard */
void kbd_reset()
{
	int ok;
	emul = 0;
	released = 0;
	CIRC_BUF_PUSH(kbd_cmd_queue, PS2_CMD_RESET, ok);
	ps2_write_data(PS2_CMD_RESET);
	(void)ok;
}


/* handle interrupt */
void kbd_handle_interrupt()
{
	uint32_t value;
	uint8_t cmd;
	int ok;

	/* read data from line */
	value = ps2_read_data();

	/* decide what to do with it */
	switch (value) {
		case PS2_RET_ATTACH:
			printf("PS/2 Keyboard Attached\n");
			/* if this was the result of a reset, clear it from the buffer */
			CIRC_BUF_PEEK(kbd_cmd_queue, cmd, ok);
			if (ok && cmd == PS2_CMD_RESET) {
				CIRC_BUF_POP(kbd_cmd_queue, cmd, ok);
			}
			break;
		case KBD_KEY_EMUL1:
			emul = 2;
			break;
		case KBD_KEY_EMUL0:
			emul = 1;
			break;
		case PS2_RET_NAK:
			CIRC_BUF_PEEK(kbd_cmd_queue, cmd, ok);
			if (ok) {
				/* resend value since the keyboard/controller did not receive what we sent */
				ps2_write_command(cmd);
			}
			else {
				printf("WARN: keyboard nak'd and we didn't send a command\n");
			}
			break;
		case KBD_KEY_RELEASED:
			released = 1;
			break;
		default:
			/* if there's a command, respond to it here */
			CIRC_BUF_PEEK(kbd_cmd_queue, cmd, ok);
			if (ok) {
				switch (cmd) {
					default:
						/* not handling specific commands right now */
						break;
				}
			}

			/* otherwise, it's a key, so we handle it */

			/* translate multibyte into single byte */
			value = kbd_combine_key(value);
			if (emul && --emul) {
				return;
			}
			value = scancodes[value];

			/* handle specific keys */
			switch (value) {
				case KBD_KEY_NULL:
					break;
				default:
					if (released) {
						released = 0;
						term_handle_keypress(value, 0);
						break;
					}
					term_handle_keypress(value, 1);
			}
			break;
	}
}

/* combines multibyte keys into a single byte */
int16_t kbd_combine_key(int16_t value)
{
	/* combine multibyte */
	value = (value & 0x7f) | ((value & 0x80) << 1);
	if (emul == 1) {
		value |= 0x80;
	}

	return value;
}
