/* proc.h, process header file
 * vim:ts=4 sw=4 noexpandtab
 */

#ifndef _PROC_H_
#define _PROC_H_

#include "isr.h"
#include "paging.h"

#define asm __asm

//	PCB STATE DEFINTIONS
#define TASK_RUNNING			0		//	process is executing or waiting to execute.
#define TASK_INTERRUPTIBLE		1		//	process suspended until a condition becomes true.
#define TASK_UNINTERRUPTIBLE	2		//	process suspended until a given event executes uninterrupted.
#define TASK_STOPPED			3		//	process has been stopped.
#define TASK_TRACED				4		//	process stopped by debugger.
#define EXIT_ZOMBIE				5		//	execution terminated, waiting for parent.
#define EXIT_DEAD				6		//	execution terminated, process being removed.

/* File flags */
#define FILE_PRESENT 1
#define FILE_OPEN    2
#define FILE_CLOSED  4

//	FILE ARRAY DEFINTIONS
#define MAX_FILES		8

/* User Space virtual addressing values */
#define MB_4_OFFSET			0x00400000
#define EXEC_OFFSET			0x00048000


#ifndef ASM

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

/* File descriptor structure
 * 16-bytes
 */
typedef struct file {
    fops_t* file_op;
    uint32_t inode_ptr;
    uint32_t file_pos;
    uint32_t flags;
} __attribute__((packed)) file_t;


typedef struct pcb
{
	//	Process State
	uint32_t state;

	//	Process ID
	uint32_t pid;

	//	File Array
	file_t file_array[MAX_FILES];

	// Stacks
	uint32_t kern_stack;
	uint32_t user_stack;

	// Page table
	pd_t * page_directory;

	//	Process Parent
	struct pcb * parent;

	// Parent State
	registers_t *parent_regs;

} pcb_t;

/* Gets the process's PCB */
static inline pcb_t *get_proc_pcb()
{
	uint32_t pcb;
	asm (	"movl	$0xFFFFC000, %0\n"
			"andl	%%esp, %0"
			: "=r"(pcb)
			:
			:
			);
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


#endif

#endif
