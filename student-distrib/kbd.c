/* kbd.c, ps/2 keyboard driver implementation 
 * vim:ts=4 noexpandtab
 */
#include "types.h"
#include "lib.h"
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
static uint32_t ps2_read_data()
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
#define SCAN_ENTRIES 256
const static char scancodes[SCAN_ENTRIES] = {
	#include "scancodes.h"
};

/* sent when a key is sent */
#define PS2_KEY_RELEASED			0xf0


/* Defines a circular FIFO command queue for the PS/2 keyboard */
#define CMD_QUEUE_LEN 64
/* increment the pointer, wrap if necessary */
#define INC(ptr) (((ptr - kbd_cmd_queue) + 1) % CMD_QUEUE_LEN + kbd_cmd_queue)
/* find the difference between the head and tail pointers, wrapping around the end of 
 * of the queue if necessary */
#define DIFF(head, tail) (((tail) - (head) + CMD_QUEUE_LEN) % CMD_QUEUE_LEN)

/* define the actual queue */
uint8_t kbd_cmd_queue[CMD_QUEUE_LEN] = {0};
uint8_t *kcq_head, *kcq_tail;

/* queue related function definitions */
static void kbd_queue_init();
static int kbd_queue_push(uint8_t cmd);
static int kbd_queue_pop(uint8_t *cmd);
//static int kbd_queue_peek(uint8_t *cmd);


/* Initialize the keyboard command queue, clearing the head and tail pointers */
static void kbd_queue_init()
{
	kcq_head = kcq_tail = kbd_cmd_queue;
}

/* Push a command to the end of the queue, returns 0 on success and -1 on failure */
static int kbd_queue_push(uint8_t cmd)
{
	if (DIFF(kcq_head, kcq_tail) == CMD_QUEUE_LEN) {
		/* queue is full, quit */
		return -1;
	}

	/* push command to tail */
	*kcq_tail = cmd;
	kcq_tail = INC(kcq_tail);

	return 0;
}

/* Pop a command from the head of hte queue, returns 0 on success and -1 on failure */
static int kbd_queue_pop(uint8_t *cmd)
{
	if (DIFF(kcq_head, kcq_tail) == 0) {
		/* queue is empty, quit */
		return -1;
	}

	/* pop command from head */
	*cmd = *kcq_head;
	kcq_head = INC(kcq_head);

	return 0;
}

#if 0 /* not currently used */
/* Peek at the head of the queue, returns 0 on success and -1 on failure */
static int kbd_queue_peek(uint8_t *cmd)
{
	if (DIFF(kcq_head, kcq_tail) == 0) {
		/* queue is empty, quit */
		return -1;
	}

	/* copy command from head */
	*cmd = *kcq_head;

	return 0;
}
#endif

void kbd_init()
{
	uint32_t read;

	kbd_queue_init();

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
		printf("Welp, we're fucked! [0x%x]\n", read);
		//return;
	}
#endif

	/* test first ps/2 port */
	ps2_write_command(PS2_CMD_TEST_PORT1);
	read = ps2_read_data();
	if (read != 0) {
		printf("Welp, we're fucked for other reasons! [0x%x]\n", read);
		//return;
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
	kbd_queue_push(PS2_CMD_RESET);
	ps2_write_data(PS2_CMD_RESET);
}


/* handle interrupt */
void kbd_handle_interrupt()
{
	uint32_t value;

	/* read from the port */
	value = ps2_read_data();
	if (value == PS2_KEY_RELEASED) {
		/* if released, read garbage */
		ps2_read_data();
	}
	else {
		putc(scancodes[value]);
	}


	kbd_queue_pop((uint8_t*)&value);
}


