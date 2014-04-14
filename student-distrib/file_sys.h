/* file_sys.h - Defines members, functions, and constants
 *              used for the file system.
 * vim:ts=4 sw=4 noexpandtab
 */
 
#ifndef _FILE_SYS_H
#define _FILE_SYS_H

#include "types.h"
#include "lib.h"
#include "x86_desc.h"
#include "multiboot.h"

#ifndef ASM

#define FILE_NAME_SIZE 		32

#define FILE_TYPE_RTC		0
#define FILE_TYPE_DIR		1
#define FILE_TYPE_REG		2

#define BLOCK_SIZE			4096
#define ADDRESSES_PER_BLOCK	1024

/* User Space virtual addressing values */
#define USER_SPACE			0x08000000
#define MB_4_OFFSET			0x00400000
#define EXEC_OFFSET			0x00048000



/* ________Data Structures________ */

/* File descriptor structure
 * 16-bytes
 */
typedef struct file {
    void* file_op;
    uint32_t inode_ptr;
    uint32_t file_pos;
    uint32_t flags;
} __attribute__((packed)) file_t;

/*	Index Node Struct
 *	4096-bytes
 */
typedef struct index_node{
	//	4 bytes - length in bytes
	//	4 bytes per data block #

	uint32_t byte_length;
	uint32_t data_blocks[ADDRESSES_PER_BLOCK - 1];

} __attribute__((packed)) index_node_t;

/*	Data Block Struct
 *	4096-bytes
 */
typedef struct data_block{
	//	4096 bytes of data

	uint8_t data[BLOCK_SIZE];	//data in data block

} __attribute__((packed)) data_block_t;

/*	Directory Entry Struct
 *	64-bytes
 */
typedef struct dentry{
	//	32 bytes - file name
	//	4 bytes	 - files type	(0, 1, or 2)
	//	4 bytes	 - inode number (ignored for type 0 and 1)
	//	24 bytes - reserved 

	uint8_t file_name[32];
	uint32_t file_type;
	uint32_t inode_num;
	uint8_t reserved[24];

} __attribute__((packed)) dentry_t;


/*	Boot Block
 *	4096 bytes
 */
typedef struct boot_block{
	//	4 bytes - number of directory entries
	//	4 bytes - number of index nodes (inodes)
	//	4 bytes - number of data blocks
	//	52 bytes - reserved
	//	64-byte directory entries

	uint32_t num_entries;
	uint32_t num_nodes;
	uint32_t num_blocks;
	uint8_t reserved[52];
	dentry_t entries[63];
	
		
} __attribute__((packed)) boot_block_t;

extern boot_block_t* mbi_val;


/* ________Function prototypes________ */

void fs_init(void);

uint32_t fs_read(file_t* file, uint8_t* buf, int count);
uint32_t fs_write(file_t* file, uint8_t* buf, int count);
uint32_t fs_open(void);
uint32_t fs_close(void);


int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

uint32_t dir_read(file_t* file, uint8_t* buf, int count);
uint32_t dir_write(file_t* file, uint8_t* buf, int count);
uint32_t dir_open(void);
uint32_t dir_close(void);

// ** NEW ** //
uint32_t file_loader(file_t* file, uint32_t* EIP);


#endif /* ASM           */
#endif /* _FILE_SYS_H   */

