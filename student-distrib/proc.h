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
} __attribute__((packed)) file_t;

/*
 * Process Control Block
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

	/*Process arguments*/
	uint8_t cmd_args[MAX_ARGS_LEN + 1];

	/*Page table*/
	pd_t *page_directory;

	/*Process Parent*/
	struct pcb *parent;

	/*Parent State*/
	registers_t *parent_regs;

	/* stores video memory state */
	screen_t screen;

	/* flag denotes whether process has mapped video memory */
	int8_t has_video_mapped;

	/* terminal keyboard buffer */
	term_t term_ctx;
} pcb_t;


/****************************************
 *           Global Variables           *
 ****************************************/

extern uint8_t nprocs;
extern uint32_t proc_bitmap;


/****************************************
 *         Function Declarations        *
 ****************************************/

/* Gets the process's PCB */
static inline pcb_t *get_proc_pcb()
{
	uint32_t pcb;
	asm (	"movl	$0xFFFFE000, %0\n"
			"andl	%%esp, %0"
			: "=r"(pcb)
			:
			: "memory");

	/* check if we're running as the kernel pre-execute first shell */
	if (KERNEL_MEM + MB_4_OFFSET - USER_STACK_SIZE < pcb && pcb < KERNEL_MEM + MB_4_OFFSET) {
		return NULL;
	}

	return (pcb_t *)pcb;
}

static inline file_t *get_file_from_fd(int32_t fd)
{
	pcb_t *pcb = get_proc_pcb();
	if (pcb && 0 <= fd && fd < MAX_FILES) {
		return &pcb->file_array[fd];
	}
	return NULL;
}

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

static inline int32_t get_first_free_pid()
{
	int32_t index;
	uint32_t bitmap = proc_bitmap >> 1;

	for(index = 1; index < MAX_PROCESSES + 1; index++, bitmap>>=1) {

		if((bitmap % 2) == 0) {
			/* Mark that bit as active, and return bit. */
			proc_bitmap |= (1<<index);
			return index;
		}
	}

	return -1;
}

static inline void free_pid(int32_t pid)
{
	proc_bitmap &= ~(1<<pid);
}

static inline vid_mem_t *get_proc_fake_vid_mem()
{
	pcb_t *pcb;

	pcb = get_proc_pcb();
	if (!pcb) {
		return NULL;
	}

	return fake_video_mem + pcb->pid - 1;
}

#endif

#endif
