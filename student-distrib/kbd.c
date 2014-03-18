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

/* the PS/2 commands */
#define PS2_CMD_RESET					0xFE

/* the PS/2 status bits */
#define PS2_STATUS_OBUFFER_FULL			(1 << 0)
#define PS2_STATUS_IBUFFER_FULL			(1 << 1)
#define PS2_STATUS_SYSTEM_FLAG			(1 << 2)
#define PS2_STATUS_COMMAND				(1 << 3)
#define PS2_STATUS_UKN1					(1 << 4)
#define PS2_STATUS_UKN2					(1 << 5)
#define PS2_STATUS_TIME_OUT				(1 << 6)
#define PS2_STATUS_PARITY_ERR			(1 << 7)


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
static int kbd_queue_peek(uint8_t *cmd);


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

/* Sends the reset command to the keyboard */
void kbd_reset()
{
	uint32_t status;

	/* wait until we can write */
	do {
		status = inb(PS2_PORT_STATUS);
	} while (status & PS2_STATUS_IBUFFER_FULL);

	/* do the writing */
	outb(PS2_PORT_DATA, PS2_CMD_RESET);
}

