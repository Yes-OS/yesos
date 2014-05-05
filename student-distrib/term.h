/* term.h - terminal driver and related functions
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _TERM_H_
#define _TERM_H_

#include "types.h"
#include "queue.h"

/****************************************
 *            Global Defines            *
 ****************************************/

/* constant file descriptors */
#define STDIN   0
#define STDOUT  1

/* buffer size of keyboard + 1 */
#define KBD_BUF_SIZE 129

/* number of terminals */
#define NUM_TERMS 4

/* definitions only available to c code */
#ifndef ASM

/****************************************
 *              Data Types              *
 ****************************************/

/* Terminal struct
 */
typedef struct terminal {
	/* Defines a circular buffer for keypresses */
	DECLARE_CIRC_BUF(int8_t, key_buf, KBD_BUF_SIZE);

	/* handle modifier keys */
	int8_t lctrl_held;
	int8_t rctrl_held;
	int8_t lshift_held;
	int8_t rshift_held;
	int8_t lalt_held;
	int8_t ralt_held;
	int8_t caps_lock;

	struct screen screen;
} term_t;

/****************************************
 *           Global Variables           *
 ****************************************/

/* forward declare to break dependency loop */
struct fops;
struct screen;

extern struct fops term_fops;
extern int32_t terminal_num;
extern struct screen kern_screen;
extern int32_t term_pids[];
extern term_t term_terms[];

/****************************************
 *         Function Declarations        *
 ****************************************/

/* Opens a terminal 
 */
int32_t term_open(const uint8_t *filename);

/* Closes a terminal
 */
int32_t term_close(int32_t fd);

/* Reads from the terminal
 */
int32_t term_read(int32_t fd, void *buf, int32_t nbytes);

/* Writes to the terminal
 */
int32_t term_write(int32_t fd, const void *buf, int32_t nbytes);

/* Handles the keypress of a terminal
 */
void term_handle_keypress(uint16_t key, uint8_t status);

/* Initializes the global terminal context data
 */
int32_t term_init_global_ctx();


#endif /* ASM */
#endif /* _TERM_H_ */
