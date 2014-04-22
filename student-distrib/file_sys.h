/* file_sys.h - Defines members, functions, and constants
 *              used for the file system.
 * vim:ts=4 sw=4 noexpandtab
 */

#ifndef _FILE_SYS_H
#define _FILE_SYS_H

#include "types.h"
#include "proc.h"

/****************************************
 *            Global Defines            *
 ****************************************/

#define FILE_NAME_SIZE 32

#define FILE_TYPE_RTC 0
#define FILE_TYPE_DIR 1
#define FILE_TYPE_REG 2

#define BLOCK_SIZE 4096

#define ELF_EIP_OFFSET 24

#ifndef ASM

/****************************************
 *              Data Types              *
 ****************************************/

/*
 * Index Node Struct
 * Size: 4096 bytes
 * Members:
 *   - 4 byte length
 *   - 4 byte index of data block #1
 *   - 4 byte index of data block #2
 *   - ...
 *   - 4 byte index of data block #1023
 */
typedef struct index_node{
	uint32_t byte_length;
	uint32_t data_blocks[(BLOCK_SIZE / sizeof(uint32_t)) - 1];
} __attribute__((packed)) index_node_t;

/*
 * Data Block Struct
 * Size: 4096 bytes
 * Members:
 *   - 4096 byte data
 */
typedef struct data_block{
	uint8_t data[BLOCK_SIZE];
} __attribute__((packed)) data_block_t;

/*
 * Directory Entry Struct
 * Size: 64 bytes
 * Members:
 *   - 32 byte file name
 *   -  4 byte file type
 *   -  4 byte inode index
 *   - 24 byte reserved
 */
typedef struct dentry{
	uint8_t file_name[32];
	uint32_t file_type;
	uint32_t inode;
	uint8_t reserved[24];
} __attribute__((packed)) dentry_t;


/*
 * Boot Block Struct
 * Size: 4096 bytes
 * Members:
 *   -  4 byte number of directory entries
 *   -  4 byte number of inodes
 *   -  4 byte number of data blocks
 *   - 52 byte reserved
 *   - 63 directory entries
 */
typedef struct boot_block{
	uint32_t num_dentries;
	uint32_t num_inodes;
	uint32_t num_dblocks;
	uint8_t reserved[52];
	dentry_t entries[63];
} __attribute__((packed)) boot_block_t;



/****************************************
 *         Function Declarations        *
 ****************************************/

void fs_init(boot_block_t* boot_val);

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t dir_open(const uint8_t *filename);
int32_t dir_close(int32_t fd);

int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t file_open(const uint8_t* filename);
int32_t file_close(int32_t fd);

uint32_t file_loader(dentry_t* file, uint32_t* eip);


/****************************************
 *           Global Variables           *
 ****************************************/

/* file operations table for files and directories */
extern fops_t file_fops;
extern fops_t dir_fops;

#endif /* ASM           */
#endif /* _FILE_SYS_H   */

