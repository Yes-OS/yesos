/* file_sys.h - Defines members, functions, and constants
 *              used for the file system.
 * vim:ts=4 sw=4 noexpandtab
 */
 
#ifndef _FILE_SYS_H
#define _FILE_SYS_H

#include "types.h"
#include "lib.h"
#include "x86_desc.h"

#ifndef ASM

#define FILE_NAME_SIZE 		32;

#define FILE_TYPE_RTC		0;
#define FILE_TYPE_DIR		1;
#define FILE_TYPE_REG		2;

/* beginning of the file system */
extern uint32_t* fs_head;


/* ________Data Structures________ */

/* File descriptor structure
 * 16-bytes
 */
typedef struct file {
    uint32_t file_op;
    uint32_t inode_ptr;
    uint32_t file_pos;
    uint32_t flags;
} file_t;

/*	Index Node Struct
 *	4096-bytes
 */
typedef struct index_node
{
	//	4 bytes - length in bytes
	//	4 bytes per data block #

	uint32_t block_length;
	uint32_t * data_block;	//used to traverse blocks

} index_node_t;


/*	Directory Entry Struct
 *	64-bytes
 */
typedef struct dentry
{
	//	32 bytes - file name
	//	4 bytes	 - files type	(0, 1, or 2)
	//	4 bytes	 - inode number (ignored for type 0 and 1)
	//	24 bytes - reserved 

	uint8_t * file_name;
	uint32_t file_type;
	uint32_t inode_num;

} dentry_t;


/*	Boot Block
 *	64 + (num_entries * 64) bytes
 */
typedef struct boot_block
{
	//	4 bytes - number of directory entries
	//	4 bytes - number of index nodes (inodes)
	//	4 bytes - number of data blocks
	//	52 bytes - reserved
	//	64-byte directory entries

	uint32_t num_entries;
	uint32_t num_nodes;
	uint32_t num_blocks;
	dentry_t * entries; //used to traverse dir. entries
	//	array size stored in 'num_entries'
		
} boot_block_t;


/* ________Function prototypes________ */

/*
 *
 */
void fs_init(void);

/* 
 *
 */
uint32_t fs_read(uint8_t* buf, int count);

/* 
 *
 */
uint32_t fs_write(/**/);

/* 
 *
 */
uint32_t fs_open(/**/);

/* 
 *
 */
uint32_t fs_close(/**/);

/*	Read Directory Entry by Name
 *	Parameters:	fname 	- the name of the file.
 *				dentry	- pointer to directory entry of file to set name.
 *
 *  Fill 'dentry_t' block passed in with:
 *		-file name, file type, and inode number
 *
 *	Return 0 on success, -1 on failure.
 *	
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);


/*	Read Directory Entry by Name
 *	Parameters:	fname 	- the name of the file.
 *				dentry	- pointer to directory entry of file to set name.
 *
 *  Fill 'dentry_t' block passed in with:
 *		-file name, file type, and inode number
 *
 *	Return 0 on success, -1 on failure.
 *	
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

/*	Read Data
 *	Parameters:	inode	- the index node of the data.
 *				offset	- how many bytes to begin reading at?
 *				buf		- data array to read.
 *				length  - size of array.
 *	Return 0 on success, -1 on failure.
 *
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);


#endif /* ASM           */
#endif /* _FILE_SYS_H   */

