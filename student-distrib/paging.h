/* paging.h - Defines constants and functions for paging  
 * vim:ts=4 sw=4 noexpandtab
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

#define PAGE_SIZE	4096
#define	NUM_ENTRIES	1024
#define EMPTY		0x00
#define SET_RW		0x02
#define SET_RW_P	0x03
#define SET_4MB		0x80

#define KERNEL_MEM	0x400000

/* Tests if a given directory entry is for a 4MB page */
#define PDE_IS_4MB(entry)	((entry).page_size == 1)

/* Selects bits 31-22 in an address */
#define PAGE_DIR_MASK		0xFFC00000
/* Selects bits 21-12 in an address */
#define PAGE_TABLE_MASK		0x003FF000
/* Selects bits 11-0 in an address */
#define PAGE_OFFSET_MASK	0x00000FFF
/* Selects the upper 20 bits of an address */
#define PAGE_BASE_MASK		0xFFFFF000

/* computes the index and base addresses for different entries */
#define PAGE_TABLE_IDX(addr)		(((addr) & PAGE_TABLE_MASK) >> 12)
#define PAGE_DIR_IDX(addr)			(((addr) & PAGE_DIR_MASK) >> 22)
#define PAGE_BASE_ADDR(addr)		(((addr) & PAGE_BASE_MASK) >> 12)
#define PAGE_BASE_ADDR_4MB(addr)	(((addr) & PAGE_BASE_MASK) >> 22)

#ifndef ASM

/*Initialize paging*/
void paging_init(void);

/* type definitions */

/* Page Directory Entry */
typedef struct pde {
	union {
		uint32_t val;
		struct {	/* common bits */
			uint32_t present : 1;
			uint32_t read_write : 1;
			uint32_t user_supervisor : 1;
			uint32_t write_through : 1;
			uint32_t cache_disabled : 1;
			uint32_t accessed : 1;
			uint32_t reserved : 1;
			uint32_t page_size : 1;
			uint32_t global_page : 1;
			uint32_t avail : 3;
			uint32_t pt_base_addr : 20;
		} __attribute__((packed));
		struct {	/* used for 4MB entries, assumes page_size = 1 */
			uint32_t : 6;		/* same as bits above */
			uint32_t dirty : 1; /* should be aligned with the reserved field above */
			uint32_t : 5;		/* same as bits above */
			uint32_t pt_attr_idx : 1;
			uint32_t reserved_4mb : 9;
			uint32_t page_base_addr_4mb : 10;
		} __attribute__((packed));
	};
} pde_t;

/* Page Table Entry */
typedef struct pte {
	union {
		uint32_t val;
		struct {
			uint32_t present : 1;
			uint32_t read_write : 1;
			uint32_t user_supervisor : 1;
			uint32_t write_through : 1;
			uint32_t cache_disabled : 1;
			uint32_t accessed : 1;
			uint32_t dirty : 1;
			uint32_t pt_attr_idx : 1;
			uint32_t global_page : 1;
			uint32_t avail : 3;
			uint32_t page_base_addr : 20;
		} __attribute__((packed));
	};
} pte_t;

#endif

#endif /* _PAGING_H */

