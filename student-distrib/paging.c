/* paging.c - Sets up paging and virtual memory
 * vim:ts=4 sw=4 noexpandtab
 */

#include "paging.h"
#include "lib.h"
#include "vga.h"
#include "x86_desc.h"
#include "proc.h"

/* +1 is for the kernel's Page Directory */
pd_t page_directories[MAX_PROCESSES + 1] __attribute__((aligned(PAGE_SIZE)));

/* for video memory */
pt_t first_table __attribute__((aligned(PAGE_SIZE)));

/* For Fake Video Memory */
pt_t video_memories[NUM_TERMS] __attribute__((aligned(PAGE_SIZE)));

/* For mapping user video memory */
pt_t user_video_mems[NUM_TERMS] __attribute__((aligned(PAGE_SIZE)));

/* only used by one process at a time for temporary operations, like
 * swapping video memory */
pt_t temp_table __attribute__((aligned(PAGE_SIZE)));

/* define some actions to set/clear status bits */

/* sets the 31 bit of CR0 */
/* enables memory paging */
#define set_pg_flag() asm volatile (  \
		"movl    %%cr0, %%eax\n       \
		 orl     $0x80000000, %%eax\n \
		 movl    %%eax, %%cr0"        \
		: : : "eax", "cc", "memory"   \
		)

/* sets the 4 bit of CR4 */
/* enables page size extensions, allowing 4 MB pages */
#define set_pse_flag() asm (          \
		"movl    %%cr4, %%eax\n       \
		 orl     $0x00000010, %%eax\n \
		 movl    %%eax, %%cr4"        \
		: : : "eax", "cc"             \
		)

/* clears the 5 bit of CR4 */
/* disables physical address extension */
#define clr_pae_flag() asm (          \
		"movl    %%cr4, %%eax\n       \
		 andl    $0xFFFFFFDF, %%eax\n \
		 movl    %%eax, %%cr4"        \
		: : : "eax", "cc"             \
		)

/* Helper Functions */
static void clear_page_dir(pd_t* directory);
static void clear_page_table(pt_t* table);
static void install_pages();
static void install_kernel_page(pd_t *page_directory);
static void install_user_page(uint32_t index, pd_t *page_directory);
static void map_video_mem(const vid_mem_t *vidmem, const void *virt_addr, pd_t *proc_pd, pt_t *page_table, uint32_t flags);

/* Definition of some empty values, useful for initialization */
static const pte_t empty_page_entry = {{.val = 0UL}};
static const pde_t empty_dir_entry = {{.val = 0UL}};

/*
 * Installation of kernel page directory (0)
 * Different from user Page Directories as Kernel is supervisor only.
 * 
 * Input: pd_t - address of page directory 0 passed in by reference
 * Output: none
 */ 
static void install_kernel_page(pd_t *page_directory)
{
		pde_t kernel_mem = empty_dir_entry;

		kernel_mem.present = 1;
		kernel_mem.read_write = 1;
		kernel_mem.user_supervisor = 0;
		kernel_mem.page_size = 1;
		kernel_mem.page_base_addr_4mb = PAGE_BASE_ADDR_4MB(KERNEL_MEM);

		page_directory->entry[PAGE_DIR_IDX(KERNEL_MEM)] = kernel_mem;
}

/*
 * Install user page directories (>= 1)
 * Different from Kernel Page Directory as User is not a supervisor.
 *
 * Inputs: page_directory - address of page directory passed in by reference
 * Outputs: none
 */ 
static void install_user_page(uint32_t index, pd_t *page_directory)
{
	pde_t user_mem = empty_dir_entry;

	user_mem.present = 1;
	user_mem.read_write = 1;
	user_mem.user_supervisor = 1;
	user_mem.page_size = 1;
	user_mem.page_base_addr_4mb = PAGE_BASE_ADDR_4MB(KERNEL_MEM+index*OFFSET_4MB);

	page_directory->entry[PAGE_DIR_IDX(USER_MEM)] = user_mem;
}

/*
 * Wrapper function for mapping executable program to video memory in Page directory 0.
 *
 * Inputs:page_directory - address of page directory passed in by reference
 *        user_vid_mem_table - address of user video memory table passed in by reference
 * Ouputs:none
 *
 */
void install_user_vid_mem(pd_t *page_directory, pt_t *user_vid_mem_table)
{
	map_video_mem((void *)VIDEO, (void *)USER_VID, page_directory, user_vid_mem_table, PG_WRITE | PG_USER);
}


/*
 * Maps the video memory page table in a page directory.
 * Called during paging initialization to map video mem in all page directories
 *
 * Inputs: vidmem - video memory structure
 *         virt_addr - virtual address for video memory to be written to
 *         proc_pd - currently active page directory
 *         page_table - currently active page table (video memory)
 *         flags - extra flags to set in the table/page beyond the present bit
 * Outputs: none
 *
 */
static void map_video_mem(const vid_mem_t *vidmem, const void *virt_addr, pd_t *proc_pd, pt_t *page_table, uint32_t flags)
{
	int32_t i;

	clear_page_table(page_table);
	for (i = 0; i < (int32_t)(sizeof vidmem->data)/PAGE_SIZE; i++) {
		pte_t temp_entry = empty_page_entry;

		temp_entry.present = 1;
		temp_entry.val |= flags;
		temp_entry.page_base_addr = PAGE_BASE_ADDR((uint32_t)vidmem + i * PAGE_SIZE);

		page_table->entry[PAGE_TABLE_IDX((uint32_t)virt_addr + i * PAGE_SIZE)] = temp_entry;
	}

	{
		pde_t temp_entry = empty_dir_entry;

		temp_entry.present = 1;
		temp_entry.val |= flags;
		temp_entry.pt_base_addr = PAGE_BASE_ADDR((uint32_t)page_table);

		proc_pd->entry[PAGE_DIR_IDX((uint32_t)virt_addr)] = temp_entry;
	}
}

/*
 * Called from the scheduler to swap the running process's video memory
 * into fake video memory.
 *
 * Inputs: pcb - address of pcb passed in by reference
 * Outputs: 0 on success
 *
 */
int32_t switch_to_fake_video_memory(pcb_t *pcb)
{
	pd_t *proc_pd;
	screen_t *screen;
	term_t *term;
	uint32_t flags;
	vid_mem_t *fake;
	int32_t term_id;
	uint32_t old_pdbr;

	if (!pcb) {
		return -1;
	}

	cli_and_save(flags);

	term = get_term_ctx(pcb);
	screen = &term->screen;

	term_id = term - term_terms;
	fake = get_term_fake_vid_mem(term_id);

	/* save old pdbr */
	get_pdbr(old_pdbr);

	/* for each process in the terminal */
	for (; pcb; pcb = pcb->parent) {
		proc_pd = pcb->page_directory;
		/* map in fake video memory */
		map_video_mem(fake, fake, proc_pd, &video_memories[term_id], PG_WRITE | PG_USER);

		/* if we've mapped video memory for the user program, update that too */
		if (pcb->has_video_mapped) {
			map_video_mem(fake, (void *)USER_VID, proc_pd, &user_video_mems[term_id], PG_WRITE | PG_USER);
		}
	}

	/* it doesn't really matter which process in this group we use to save the screen,
	 * so we'll use the last one */
	set_pdbr(proc_pd);

	/* update pointer in terminal's screen context */
	screen->video = fake;

	/* save current screen to new memory */
	screen_save(screen);

	/* restore pd */
	set_pdbr(old_pdbr);

	restore_flags(flags);
	return 0;
}

/*
 * Called from the scheduler to swap the running process's video memory
 * from fake video memory into real video memory
 *
 * Inputs: pcb - address of PCB passed in by reference
 * Outputs: 0 on success
 *
 */
int32_t switch_from_fake_video_memory(pcb_t *pcb)
{
	pd_t *proc_pd;
	screen_t *screen;
	term_t *term;
	uint32_t flags;
	vid_mem_t *fake;
	int32_t term_id;
	uint32_t old_pdbr;

	if (!pcb) {
		return -1;
	}

	cli_and_save(flags);

	/* get term, screen, and term_id */
	term = get_term_ctx(pcb);
	screen = &term->screen;

	term_id = term - term_terms;
	fake = get_term_fake_vid_mem(term_id);

	/* save old pdbr */
	get_pdbr(old_pdbr);

	/* restore the screen */
	set_pdbr(pcb->page_directory);
	screen_restore(screen);

	/* for each process in the terminal */
	for (; pcb; pcb = pcb->parent) {
		proc_pd = pcb->page_directory;

		/* remap fake video memory to real video, things break otherwise */
		map_video_mem((void *)VIDEO, fake, proc_pd, &video_memories[term_id], PG_WRITE | PG_USER);

		/* if we've mapped video memory for the user program, update that too */
		if (pcb->has_video_mapped) {
			map_video_mem((void *)VIDEO, (void *)USER_VID, proc_pd, &user_video_mems[term_id], PG_WRITE | PG_USER);
		}
	}

	/* restore the video memory pointer */
	screen->video = (vid_mem_t *)VIDEO;

	/* restore pd */
	set_pdbr(old_pdbr);

	restore_flags(flags);
	return 0;
}

/* Initializes paging. Wrapper function
 * Installs all of the page tables and page directories needed 
 * for correct system functionality.
 *
 * Inputs: none
 * Outputs: none
 *
 */
void paging_init(void)
{
	install_pages();
}


/*
 * Clears a page directory to make sure no garbage is in any of the entries to start with.
 *
 * Inputs: directory - Address of page directory to be cleared passed in by reference
 * Outputs: none
 *
 */
static void clear_page_dir(pd_t* directory)
{
	int i;

	/* clear page directory */
	for (i = 0; i < NUM_ENTRIES; i++) {
		/* install an empty entry */
		directory->entry[i] = empty_dir_entry;
	}
}

/*
 * Clears a page table to make sure no garbage is in any of the entries to start with.
 *
 * Inputs: table - Address of page table to be cleared passed in by reference
 * Outputs: none
 *
 */
static void clear_page_table(pt_t* table)
{
	int i;

	/* clear first page table */
	for (i = 0; i < NUM_ENTRIES; i++) {
		/* install an empty entry */
		table->entry[i] = empty_page_entry;
	}
}

/* 
 * Initializes every Page table and Page Directory used in this system (statically).
 * Clears all page directories/tables prior to installation.
 * Sets all flags pertaining to Paging initialization.
 *
 * Inputs: none
 * Outputs: none
 *
 */
static void install_pages()
{
	int i;

	/* Initialize per-process things */
	for(i = 0; i < MAX_PROCESSES + 1; i++) {
		clear_page_dir(&page_directories[i]);
		install_kernel_page(&page_directories[i]);
		map_video_mem((void *)VIDEO, (void *)VIDEO, &page_directories[i], &first_table, PG_WRITE);
		if (i > 0) {
			install_user_page(i, &page_directories[i]);
		}
	}

	/* Initialize per-terminal things */
	for (i = 0; i < NUM_TERMS; i++) {
		clear_page_table(&video_memories[i]);
	}

	fake_video_mem = (vid_mem_t *)(KERNEL_MEM + OFFSET_4MB * (MAX_PROCESSES + 1));

	/* set up registers */
	clr_pae_flag();
	set_pse_flag();
	set_pdbr(&page_directories[0]);

	/* enable paging */
	set_pg_flag();
}
