/* proc.h, process header file
 * vim:ts=4 sw=4 noexpandtab
 */

#ifndef _PROC_H_
#define _PROC_H_

#include "types.h"
#include "vga.h"
#include "paging.h"
#include "isr.h"
#include "term.h"

#define asm __asm

/****************************************
 *            Global Defines            *
 ****************************************/

/*PCB STATE DEFINTIONS*/

/*process is executing or waiting to execute*/
#define TASK_RUNNING            0
/*process suspended until a condition becomes true*/
#define TASK_INTERRUPTIBLE      1
/*process suspended until a given event executes uninterrupted*/
#define TASK_UNINTERRUPTIBLE    2
/*process has been stopped*/
#define TASK_STOPPED            3
/*process stopped by debugger*/
#define TASK_TRACED             4
/*execution terminated, waiting for parent*/
#define EXIT_ZOMBIE             5
/*execution terminated, process being removed*/
#define EXIT_DEAD               6

/* File flags */
#define FILE_PRESENT 1
#define FILE_OPEN    2
#define FILE_CLOSED  4
#define FILE_RTC     8

/*FILE ARRAY DEFINTIONS*/
#define MAX_FILES       8

/*Maximum length of command line arguments*/
#define MAX_ARGS_LEN    63

/* Maximum number of processes */
#define MAX_PROCESSES 10

/* User Space virtual addressing values */
#define MB_4_OFFSET         0x400000
#define EXEC_OFFSET         0x48000
#define USER_STACK_SIZE     0x2000

#ifndef ASM

/****************************************
 *              Data Types              *
 ****************************************/

typedef int32_t open_t(const uint8_t *filename);
typedef int32_t read_t(int32_t fd, void *buf, int32_t nbytes);
typedef int32_t write_t(int32_t fd, const void *buf, int32_t nbytes);
typedef int32_t close_t(int32_t fd);

/*
 * File Operations Table
 */
typedef struct fops {
	read_t *read;
	write_t *write;
	open_t *open;
	close_t *close;
} fops_t;

/*
 * File descriptor structure
 * 16 bytes
 */
typedef struct file {
    fops_t* file_op;
    uint32_t inode_ptr;
    uint32_t file_pos;
    volatile uint32_t flags;
	int32_t reserved;
} __attribute__((packed)) file_t;

/* Process Control Block:
 *  Contains various members pertaining to a process,
 *  its context, and storage pointers
 */
typedef struct pcb
{
	/*Process State*/
	uint32_t state;

	/*Process ID*/
	uint32_t pid;

	/*File Array*/
	file_t file_array[MAX_FILES];

	/*Stacks*/
	uint32_t kern_stack;
	uint32_t user_stack;

	/*Context switch esp tracker*/
	registers_t* context_esp;

	/*context switch eip tracker*/
	uint32_t context_eip;

	/*Process arguments*/
	uint8_t cmd_args[MAX_ARGS_LEN + 1];

	/*Page table*/
	pd_t *page_directory;

	/*Process Parent*/
	struct pcb *parent;

	/*Parent State*/
	registers_t *parent_regs;

	/* flag denotes whether process has mapped video memory */
	int8_t has_video_mapped;

	/* holds a pointer to the terminal context the process uses */
	term_t *term_ctx;
} pcb_t;


/****************************************
 *           Global Variables           *
 ****************************************/

extern uint8_t nprocs;
extern uint32_t proc_bitmap;


/****************************************
 *         Function Declarations        *
 ****************************************/

/* Gets the process's PCB 
 */
static inline pcb_t *get_proc_pcb()
{
	uint32_t pcb;
	asm (	"movl	$0xFFFFE000, %0\n"
			"andl	%%esp, %0"
			: "=r"(pcb)
			:
			: "memory");

	/* check if we're running as the kernel pre-execute first shell */
	if (pcb == KERNEL_MEM + MB_4_OFFSET - USER_STACK_SIZE) {
		return NULL;
	}

	return (pcb_t *)pcb;
}

/* Gets the file based on a given file descriptor
 * INPUTS: fd - file descriptor to search with
 * OUTPUTS: returns a file_t pointer
 */
static inline file_t *get_file_from_fd(int32_t fd)
{
	pcb_t *pcb = get_proc_pcb();
	if (pcb && 0 <= fd && fd < MAX_FILES) {
		return &pcb->file_array[fd];
	}
	return NULL;
}

/* Gets the next unused file descriptor from a process's pcb
 * OUTPUT: a file descriptor
 */
static inline int32_t get_unused_fd()
{
	int32_t fd;
	pcb_t *pcb;

	pcb = get_proc_pcb();

	/* skip FDs in use until we find an unused one */
	for (fd = 0; pcb->file_array[fd].flags & FILE_PRESENT; ++fd);

	/* if we found one */
	if (fd < MAX_FILES) {
		pcb->file_array[fd].flags |= FILE_PRESENT;
		return fd;
	}

	/* all are in use */
	return -1;
}

/* Clears a passed file descriptor to be used elsewhere
 * INPUTS: fd - file descriptor to clear
 */
static inline void release_fd(int32_t fd)
{
	pcb_t *pcb;

	if (fd < 0 || fd > MAX_FILES) {
		return;
	}

	pcb = get_proc_pcb();
	if (pcb) {
		pcb->file_array[fd].flags = 0;
	}
}

/* Returns the first process ID possible
 * OUTPUT: an integery PID
 */
static inline int32_t get_first_free_pid()
{
	int32_t index;

	for (index = 1; ((1 << index) & proc_bitmap) && index <= MAX_PROCESSES; index++);

	if (index > MAX_PROCESSES) {
		return -1;
	}

	proc_bitmap |= 1 << index;

	return index;
}

/* Frees a process ID for use elsewhere
 * INPUT: PID - process ID to release
 */
static inline void free_pid(int32_t pid)
{
	proc_bitmap &= ~(1<<pid);
}

/* Returns a pointer to the video memory of a passed terminal
 * INPUT: term_id - terminal of which to return its ID
 */
static inline vid_mem_t *get_term_fake_vid_mem(int32_t term_id)
{
	return fake_video_mem + term_id;
}

/* Gets the context for the correct terminal
 * OUTPUTS: term_t pointer from the applicable PCB
 */
static inline term_t *get_term_ctx()
{
	pcb_t *pcb;

	pcb = get_proc_pcb();
	if (!pcb) {
		return NULL;
	}

	while (pcb->parent) {
		pcb = pcb->parent;
	}

	return pcb->term_ctx;
}

/* Gets the current terminal address
 */
static inline term_t *get_current_term()
{
	if (term_pids[terminal_num] < 0) {
		return NULL;
	}

	return &term_terms[terminal_num];
}

/* Gets the context of the currently used screen
 *  Needed because we only want to get the screen for the topmost shell process 
 */
static inline screen_t *get_screen_ctx()
{
	term_t *term;

	term = get_term_ctx();
	if (!term) {
		return NULL;
	}

	return &term->screen;
}

/* Return the pcb associated with a given PID
 */
#define get_pcb_from_pid(_pid) ((pcb_t *)((KERNEL_MEM + MB_4_OFFSET - USER_STACK_SIZE * _pid - 1) & 0xFFFFE000))

#endif /* ASM */
#endif /* _PROC_H_ */

