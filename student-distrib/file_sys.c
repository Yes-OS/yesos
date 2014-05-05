/* file_sys.c - Functions to initialize and interact
 *              with the file system.
 *
 * vim:ts=4 sw=4 noexpandtab
 *
 * TO DO:
 *		-Add functionality:
 *			copy a program image from the disk blocks into continuous physical memory
 *			set up the stack properly and return to user-level
 */

#include "lib.h"
#include "file_sys.h"

/* File operations jump table */
fops_t file_fops = {
	.read  = &file_read,
	.write = &file_write,
	.open  = &file_open,
	.close = &file_close,
};

/* Directory operations jump table */
fops_t dir_fops = {
	.read  = &dir_read,
	.write = &dir_write,
	.open  = &dir_open,
	.close = &dir_close,
};

/*Variables for File_sys functions*/
static index_node_t* node_head;
static data_block_t* data_head;
static boot_block_t* boot_block;


/* 
 * Sets up the head pointer for the file system
 * Sets up relevant structures and variables
 *
 * Inputs: boot_val -	pointer to fs head
 * Outputs:	none
 */
void fs_init(boot_block_t* boot_val)
{
	boot_block = boot_val;

	node_head = (index_node_t*)boot_block + 1;
	data_head = (data_block_t*)node_head + boot_block->num_inodes;
}

/* 
 * Read system call for normal file types. Reads data from a file
 * and stores it in a buffer.
 *
 * Inputs: fd - file descriptor,
 *         buf - buffer 
 *         nbytes - number of bytes to read
 * Outputs:Number of bytes read
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes)
{
	file_t *file;
	int32_t ret;

	file = get_file_from_fd(fd);

	/* ensure file is open */
	if (!file || !(file->flags & FILE_OPEN)) {
		return -1;
	}

	ret = read_data(file->inode_ptr, file->file_pos, (uint8_t *)buf, nbytes);
	file->file_pos += ret;

	return ret;
}

/*  
 *  Write system call for regular file types. 
 *  Read-only file system - Not implemented;
 *
 *  Return -1
 */
int32_t file_write(int32_t fd, const void* buf, int nbytes)
{
	/* silence warnings about unused variables */
	(void)fd; (void)buf; (void)nbytes;
	return -1;
}

/*  
 *  Open system call for regular file types. Sets up a file descriptor
 *  for a specific file for a specific process.
 *
 *  Inputs: filename - name of the file
 *  Outputs: file descriptor
 */
int32_t file_open(const uint8_t *filename)
{
	dentry_t dentry;
	int32_t fd;
	file_t *file;
	int32_t status;

	status = read_dentry_by_name(filename, &dentry);
	if (status) {
		return -1;
	}

	/* find unused descriptor */
	fd = get_unused_fd();
	if (fd < 0) {
		return -1;
	}

	/* grab associated file, should never fail since we were just allocated a FD */
	file = get_file_from_fd(fd);

	/* set up the file */
	file->flags |= FILE_OPEN;
	file->inode_ptr = dentry.inode;
	file->file_pos = 0;
	file->file_op = &file_fops;

	return fd;
}

/*  Close system call for regular file types. Releases a file
 *  descriptor from the process that calls it.
 *
 *  Input: fd -File descriptor
 *  Return 0
 */
int32_t file_close(int32_t fd)
{
	release_fd(fd);
	return 0;
}


/*	
 *	Read Directory Entry by Name
 *	Parameters:	fname 	- the name of the file.
 *				dentry	- pointer to directory entry of file to set name.
 *
 *  Fill 'dentry_t' block passed in with:
 *		-file name, file type, and inode number
 *
 *	Return 0 on success, -1 on failure.
 *
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
{
	dentry_t* temp;
	uint8_t i;
	uint32_t entries = boot_block->num_dentries;
	uint32_t len_filename;

	for(i = 0; i <= entries ; i++) {

		temp = &boot_block->entries[i];

		/* length of the filename in the filesystem */
		len_filename = strlen((int8_t*)temp->file_name);

		/* there are some null entries in the fs; needed or they'll match any string passed */
		if (!len_filename) {
			continue;
		}

		/* we need to compare on the max length of a file, or else we get weird matching results */
		if(!strncmp((int8_t*)fname, (int8_t*)temp->file_name, FILE_NAME_SIZE)) {
			strncpy((int8_t *)dentry->file_name, (int8_t*)temp->file_name, len_filename);
			/* just in case the filename doesn't end in a null character */
			dentry->file_name[len_filename] = '\0';
			dentry->file_type = temp->file_type;
			dentry->inode = temp->inode;

			return 0;
		}
	}

	return -1;
}

/*	
 *	Read Directory Entry by Name
 *	Parameters:	fname 	- the name of the file.
 *				dentry	- pointer to directory entry of file to set name.
 *
 *  Fill 'dentry_t' block passed in with:
 *		-file name, file type, and inode number
 *
 *	Return 0 on success, -1 on failure.
 *
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
	//Check for non-existant file or invalid index
	if(index <= boot_block->num_dentries) {
		strncpy((int8_t*)dentry->file_name, (int8_t*)boot_block->entries[index].file_name, FILE_NAME_SIZE);
		dentry->file_type = boot_block->entries[index].file_type;
		dentry->inode = boot_block->entries[index].inode;

		return 0;
	}

	return -1;
}

/*	
 *	Read Data
 *	Parameters:	inode	- the index node of the data.
 *				offset	- how many bytes to begin reading at?
 *				buf		- data array to read.
 *				length  - size of array.
 *
 *  Reads up to 'length' bytes from position 'offset'
 *	  in the file with 'inode' number into the given 'buf' buffer.
 *
 *  Return  # of bytes read and placed into buffer
 *			0 when end of file reached.
 *			-1 on failure.
 *
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	int32_t db_idx;
	int32_t b_read;
	int32_t b_rem;
	int32_t round_read;
	data_block_t *p_db;
	index_node_t *p_inode;
	uint8_t *data;

	/* verify bounds */
	if (inode > boot_block->num_inodes) {
		return -1;
	}

	p_inode = node_head + inode;

	if (p_inode->byte_length < offset) {
		return -1;
	}

	/* compute number of bytes to read */
	b_rem = min(p_inode->byte_length - offset, length);

	/* compute first datablock */
	db_idx = offset / BLOCK_SIZE;
	p_db = data_head + p_inode->data_blocks[db_idx];
	data = p_db->data + offset % BLOCK_SIZE;

	b_read = 0;
	while (b_rem > 0) {
		/* compute number of bytes to read this iteration */
		round_read = min(b_rem, p_db->data + BLOCK_SIZE - data);

		/* copy data into buffer */
		memcpy(buf+b_read, data, round_read);
		b_read += round_read;
		b_rem -= round_read;

		/* get next block */
		p_db = data_head + p_inode->data_blocks[++db_idx];
		data = p_db->data;
	}

	return b_read;
}

/* 
 * Read system call for directory file types. Reads off a file
 * name based off the file_pos of the directory.
 *
 * Inputs:fd - file descriptor
 *        buf - buffer to write file name to
 *        nbytes - number of bytes to be read
 * Outputs: number of characters in file name
 *
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes)
{
	file_t *file;
	int32_t ret;
	dentry_t dentry;

	file = get_file_from_fd(fd);

	/* ensure file is open */
	if (!file || !(file->flags & FILE_OPEN)) {
		return -1;
	}

	ret = read_dentry_by_index(file->file_pos, &dentry);
	if (ret) {
		/* at the end of the directory */
		return 0;
	}

	strncpy((int8_t*)buf, (int8_t*)dentry.file_name, nbytes);

	/* just in case */
	((int8_t*)buf)[nbytes-1] = '\0';

	/* increment directory position */
	file->file_pos += 1;

	return strlen((int8_t*)dentry.file_name);
}

/*  
 *  Write system call for directory file types.
 *  Read-only file system.
 *
 *  Return -1
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes)
{
	(void)fd; (void)buf; (void)nbytes;
	return -1;
}

/* 
 * Open system call for directory file types.
 * Similar to open system call for regular file types.
 * Creates a file descriptor for a directory file type.
 *
 * Inputs: filename - name of the file to be opened
 * Outputs: fd- file descriptor
 *
 */
int32_t dir_open(const uint8_t *filename)
{
	dentry_t dentry;
	int32_t fd;
	file_t *file;
	int32_t status;

	status = read_dentry_by_name(filename, &dentry);
	if (status) {
		return -1;
	}

	/* find unused descriptor */
	fd = get_unused_fd();
	if (fd < 0) {
		return -1;
	}

	/* grab associated file, should never fail since we were just allocated a FD */
	file = get_file_from_fd(fd);

	/* set up the file */
	file->flags |= FILE_OPEN;
	file->inode_ptr = dentry.inode;
	file->file_pos = 0;
	file->file_op = &dir_fops;

	return fd;
}

/* 
 *  Close system call for directory file types.
 *  Releases a file descriptor of a directory file type.
 *
 *  Inputs: fd - file descriptor to be released
 *  Outputs: 0
 */
int32_t dir_close(int32_t fd)
{
	release_fd(fd);
	return 0;
}

/*
 * Loads a file to a specific location in memory based on the location
 * specified by the file being executed.
 *
 * Inputs:file - executable file
 *        eip - location in memory to copy the file to
 * Outputs: 0 on success.
 *          returns the eip, as eip should be passed by reference	  
 */
uint32_t file_loader(dentry_t* file, uint32_t* eip)
{
	uint32_t b_rem;
	uint32_t file_eip, b_read;
	uint32_t f_pos;
	uint8_t *dest;

	/* Read the entire file */
	b_rem = node_head[file->inode].byte_length;

	f_pos = 0;

	/* Desitination is user space at the executable load offset */
	dest = (uint8_t *)(USER_MEM + EXEC_OFFSET);
	while(b_rem > 0) {
		b_read = read_data(file->inode, f_pos, dest + f_pos, b_rem);
		f_pos += b_read;
		b_rem -= b_read;
	}

	/* get EIP from bytes 24-27 of executable */
	file_eip = *(uint32_t *)(dest + ELF_EIP_OFFSET);
	if(file_eip < USER_MEM + EXEC_OFFSET || file_eip > USER_MEM + MB_4_OFFSET) {
		return -1;
	}

	*eip = file_eip;

	return 0;
}
