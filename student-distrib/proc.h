/* proc.h, process header file
 * vim:ts=4 sw=4 noexpandtab
 */

#ifndef _PROC_H_
#define _PROC_H_

//	PCB STATE DEFINTIONS
#define TASK_RUNNING			0		//	process is executing or waiting to execute.
#define TASK_INTERRUPTIBLE		1		//	process suspended until a condition becomes true.
#define TASK_UNINTERRUPTIBLE	2		//	process suspended until a given event executes uninterrupted.
#define TASK_STOPPED			3		//	process has been stopped.
#define TASK_TRACED				4		//	process stopped by debugger.
#define EXIT_ZOMBIE				5		//	execution terminated, waiting for parent.
#define EXIT_DEAD				6		//	execution terminated, process being removed.

//	FILE ARRAY DEFINTIONS
#define FILE_ARRAY_LENGTH		8		

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
    struct fops* file_op;
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
	struct file file_array[FILE_ARRAY_LENGTH];

	//	Process Parent
	struct pcb * parent;

} pcb_t;

#endif

#endif
