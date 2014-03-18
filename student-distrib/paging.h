/* paging.h - Defines constants and functions for paging  
 * vim:ts=4 sw=4 noexpandtab
 */

#ifndef _PAGING_H
#define _PAGING_H

#define PAGE_SIZE	4096
#define	NUM_ENTRIES	1024
#define EMPTY		0x00
#define SET_RW		0x02
#define SET_RW_P	0x03
#define SET_4MB		0x80

/*Initialize paging*/
void paging_init(void);


#endif /* _PAGING_H */

