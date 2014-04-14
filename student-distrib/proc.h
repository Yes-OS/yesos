/* proc.h, process header file
 * vim:ts=4 sw=4 noexpandtab
 */


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

typedef struct
{
	//	Process State
	uint32_t state;

	//	Process ID
	uint32_t pid;

	//	File Array
	file_t file_array[FILE_ARRAY_LENGTH];

	//	Process Parent
	pcb_t * parent;

}	pcb_t;
