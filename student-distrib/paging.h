/* paging.h - Defines constants and functions for paging
 * vim:ts=4 sw=4 noexpandtab
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

/****************************************
 *            Global Defines            *
 ****************************************/

#define PAGE_SIZE   4096
#define NUM_ENTRIES 1024

#define KERNEL_MEM 0x400000
#define USER_MEM   0x08000000
#define USER_VID   0x088B8000

/* Tests if a given directory entry is for a 4MB page */
#define PDE_IS_4MB(entry) ((entry).page_size == 1)

/* Selects bits 31-22 in an address */
#define PAGE_DIR_MASK       0xFFC00000
/* Selects bits 21-12 in an address */
#define PAGE_TABLE_MASK     0x003FF000
/* Selects bits 11-0 in an address */
#define PAGE_OFFSET_MASK    0x00000FFF
/* Selects the upper 20 bits of an address */
#define PAGE_BASE_MASK      0xFFFFF000

/* computes the index and base addresses for different entries */
#define PAGE_TABLE_IDX(addr)        (((addr) & PAGE_TABLE_MASK) >> 12)
#define PAGE_DIR_IDX(addr)          (((addr) & PAGE_DIR_MASK) >> 22)
#define PAGE_BASE_ADDR(addr)        (((addr) & PAGE_BASE_MASK) >> 12)
#define PAGE_BASE_ADDR_4MB(addr)    (((addr) & PAGE_BASE_MASK) >> 22)

/* bitfield defines for paging flags */
#define PG_PRESENT     (1 << 0)
#define PG_WRITE       (1 << 1)
#define PG_USER        (1 << 2)
#define PG_WRITETHRU   (1 << 3)
#define PG_NOCACHE     (1 << 4)
#define PG_ACCESSED    (1 << 5)
#define PG_DIRTY       (1 << 6)
#define PG_SIZE_4MB    (1 << 7)
#define PG_GLOBAL      (1 << 8)
#define PG_PT_ATTR_IDX (1 << 12)


#ifndef ASM

/****************************************
 *              Data Types              *
 ****************************************/

/* Page Directory Entry */
typedef struct pde {
	union {
		uint32_t val;
		struct {
			/* common bits */
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

		/* used for 4MB entries, assumes page_size = 1 */
		struct {
			uint32_t : 6;       /* same as bits above */
			uint32_t dirty : 1; /* should be aligned with the reserved field above */
			uint32_t : 5;       /* same as bits above */
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

/* Page directory */
typedef struct pd {
	pde_t entry[NUM_ENTRIES];
} pd_t;

/* Page table */
typedef struct pt {
	pte_t entry[NUM_ENTRIES];
} pt_t;


/****************************************
 *           Global Variables           *
 ****************************************/

/* Global page Directories */
extern pd_t page_directories[];

/* Global video memory loc array*/
extern pt_t user_video_mems[];


/****************************************
 *           Macro Definitions          *
 ****************************************/

/* sets page directory base register (cr3) */
#define set_pdbr(base) asm volatile ( \
				"movl    %0, %%cr3"   \
				: : "r" ((base))      \
				: "memory"            \
			)

/* gets page directory base register (cr3) */
#define get_pdbr(addr) asm volatile ( \
				"movl    %%cr3, %0"   \
				: "=r" ((addr))       \
				: : "memory"          \
			)


/****************************************
 *         Function Declarations        *
 ****************************************/

/* Initialize paging */
void paging_init(void);

/* Installation of user vid mem for executables */
void install_user_vid_mem(pd_t *page_directory, pt_t *user_vid_mem_table);

/* Used for switching (fake)video-memory for tasks running in the background */
int32_t switch_to_fake_video_memory();
int32_t switch_from_fake_video_memory();

#endif /* _ASM_ */
#endif /* _PAGING_H */

