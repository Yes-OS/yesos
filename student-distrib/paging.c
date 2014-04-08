/* paging.c - Sets up paging and virtual memory
 * vim:ts=4 sw=4 noexpandtab
 */
 
#include "paging.h"
#include "lib.h"
#include "vga.h"
#include "x86_desc.h"

pde_t page_directory[NUM_ENTRIES]	__attribute__((aligned(PAGE_SIZE)));
pte_t page_table[NUM_ENTRIES]	__attribute__((aligned(PAGE_SIZE)));

/* define some actions to set/clear status bits */

/* sets the 31 bit of CR0 */
/* enables memory paging */
#define set_pg_flag()                               \
	do {                                            \
		asm volatile (                              \
				"movl    %%cr0, %%eax\n             \
				 orl     $0x80000000, %%eax\n       \
				 movl    %%eax, %%cr0"              \
				: : : "eax", "cc", "memory"  \
			);                                      \
	} while (0)

/* sets the 4 bit of CR4 */
/* enables page size extensions, allowing 4 MB pages */
#define set_pse_flag()                         \
	do {                                       \
		asm (                                  \
				"movl    %%cr4, %%eax\n        \
				 orl     $0x00000010, %%eax\n  \
				 movl    %%eax, %%cr4"         \
				: : : "eax", "cc"       \
			);                                 \
	} while (0)

/* clears the 5 bit of CR4 */
/* disables physical address extension */
#define clr_pae_flag()                         \
	do {                                       \
		asm (                                  \
				"movl    %%cr4, %%eax\n        \
				 andl    $0xFFFFFFDF, %%eax\n  \
				 movl    %%eax, %%cr4"         \
				: : : "eax", "cc"       \
			);                                 \
	} while (0)

/* sets page directory base register (cr3) */
#define set_pdbr(base)               \
	do {                             \
		asm volatile (               \
				"movl    %0, %%cr3"  \
				: : "r" ((base))     \
				: "memory"    \
			);                       \
	} while (0)


/* helper functions */
static void clear_page_dir();
static void clear_page_table();
static void install_pages();

/* define some empty values, useful for initialization */
static const pte_t empty_page_entry = {{.val = 0UL}};
static const pde_t empty_dir_entry = {{.val = 0UL}};


/* initializes paging */
void paging_init(void)
{
	install_pages();
}

/* clears the page directory for the kernel */
static void clear_page_dir(pde_t directory[NUM_ENTRIES])
{
	int i;

	/* clear page directory */
	for (i = 0; i < NUM_ENTRIES; i++) {
		/* install an empty entry */
		directory[i] = empty_dir_entry;
	}
}

/* clears the first page table */
static void clear_page_table(pte_t table[NUM_ENTRIES])
{
	int i;

	/* clear first page table */
	for (i = 0; i < NUM_ENTRIES; i++) {
		/* install an empty entry */
		table[i] = empty_page_entry;
	}
}

/* installs the 4MB page for the kernel, and maps 64k of video memory */
static void install_pages()
{
	int i;

	clear_page_dir(page_directory);
	clear_page_table(page_table);

	{
		/* setup first page directory */
		pde_t first_page_dir = empty_dir_entry;

		first_page_dir.present = 1;
		first_page_dir.read_write = 0;
		first_page_dir.user_supervisor = 1;
		first_page_dir.pt_base_addr = PAGE_BASE_ADDR((uint32_t)page_table);

		page_directory[0] = first_page_dir;

		/* setup video memory */
		pte_t video_mem_temp = empty_page_entry;
		for (i = 0; i < 16; i++) {
			video_mem_temp.present = 1;
			video_mem_temp.read_write = 1;
			video_mem_temp.user_supervisor = 1;
			video_mem_temp.page_base_addr = PAGE_BASE_ADDR(VIDEO + i * 0x1000);
			page_table[PAGE_TABLE_IDX(VIDEO + i * 0x1000)] = video_mem_temp;
		}
	}
	
	{
		/* setup kernel page */
		pde_t kernel_mem = empty_dir_entry;

		kernel_mem.present = 1;
		kernel_mem.read_write = 1;
		kernel_mem.user_supervisor = 1;
		kernel_mem.page_size = 1;
		kernel_mem.page_base_addr_4mb = PAGE_BASE_ADDR_4MB(KERNEL_MEM);

		page_directory[PAGE_DIR_IDX(KERNEL_MEM)] = kernel_mem;
	}

	/* set up registers */
	clr_pae_flag();
	set_pse_flag();
	set_pdbr(page_directory);

	/* enable paging */
	set_pg_flag();
}
