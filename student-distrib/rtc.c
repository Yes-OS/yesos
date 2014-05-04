/* rtc.c - Functions to interact with the RTC controller
 * vim:ts=4 sw=4 noexpandtab
 */

#include "lib.h"
#include "proc.h"
#include "rtc.h"

/* File operations jump table */
fops_t rtc_fops = {
  .read  = rtc_read,
  .write = rtc_write,
  .open  = rtc_open,
  .close = rtc_close,
};

static file_t *open_rtcs = NULL;

/* frequency stored in bytes 24-27 of the flags variable */
#define rtc_virt_get_freq(_rtc) (((_rtc)->flags & 0x0F000000UL) >> 24)

/* frequency stored in bytes 24-27 of the flags variable */
#define rtc_virt_set_freq(_rtc, freq) do {    \
	(_rtc)->flags &= 0xF0FFFFFFUL;            \
	(_rtc)->flags |= ((freq) & 0x0FUL) << 24; \
} while (0)

/* true if virtual rtc has ticked */
#define rtc_virt_has_ticked(_rtc) ((_rtc)->flags & (1 << 31))
#define rtc_virt_clr_ticked(_rtc) do { \
	(_rtc)->flags &= ~(1 << 31);       \
} while (0)
#define rtc_virt_set_ticked(_rtc) do { \
	(_rtc)->flags |= (1 << 31);        \
} while (0)

/* operations on number of virtual ticks */
#define rtc_virt_ticks(_rtc) (((_rtc)->file_pos & 0xFFFF0000UL) >> 16)
#define rtc_virt_set_ticks(_rtc, ticks) do {        \
	(_rtc)->file_pos &= 0x0000FFFFUL;               \
	(_rtc)->file_pos |= ((ticks) & 0xFFFFUL) << 16; \
} while (0)
#define rtc_virt_clr_ticks(_rtc) rtc_virt_set_ticks(_rtc,0)

static inline uint16_t rtc_virt_incr_ticks(file_t *rtc)
{
	uint16_t ticks;

	ticks = rtc_virt_ticks(rtc);
	rtc_virt_set_ticks(rtc, ++ticks);

	return ticks;
}

static inline uint16_t rtc_virt_decr_ticks(file_t *rtc)
{
	uint16_t ticks;

	ticks = rtc_virt_ticks(rtc);
	if (ticks > 0) {
		rtc_virt_set_ticks(rtc, --ticks);
	}

	return ticks;
}

/* operations on number of remaining real ticks counter */
#define rtc_virt_get_ctr(_rtc) ((_rtc)->file_pos & 0xFFFFUL)
#define rtc_virt_set_ctr(_rtc, ctr) do { \
	(_rtc)->file_pos &= 0xFFFF0000UL;    \
	(_rtc)->file_pos |= ctr & 0xFFFFUL;  \
} while (0)
#define rtc_virt_clr_ctr(_rtc) rtc_virt_set_ctr(_rtc, 0)
#define rtc_virt_rst_ctr(_rtc) do {                  \
	rtc_virt_set_ctr(_rtc,                           \
			MAX_FREQ_HZ >> rtc_virt_get_freq(_rtc)); \
} while (0)


static inline uint16_t rtc_virt_incr_ctr(file_t *rtc)
{
	uint16_t ctr;

	ctr = rtc_virt_get_ctr(rtc);
	rtc_virt_set_ctr(rtc, ++ctr);

	return ctr;
}

static inline uint16_t rtc_virt_decr_ctr(file_t *rtc)
{
	uint16_t ctr;

	ctr = rtc_virt_get_ctr(rtc);
	if (ctr > 0) {
		rtc_virt_set_ctr(rtc, --ctr);
	}

	return ctr;
}


/* initialize the RTC */
void rtc_init(void)
{
    uint8_t regB;

    /*Select Register B to set up periodic interrupts*/
    outb((DISABLE_NMI | REG_B), NMI_RTC_PORT);

	/*Extract current value from register B*/
	regB = inb(RTC_RAM_PORT);

	/*Enable bit 6 of register B to enable Periodic Interrupts (PIE)*/
	regB = regB | 0x40;

	/*Set the same index, because reading from the port sets the index to register D*/
	outb((DISABLE_NMI | REG_B), NMI_RTC_PORT);

	/*Set Register B with enabled PIE*/
	outb(regB, RTC_RAM_PORT);

	/*Re-enable NMI*/
	uint8_t enable = inb(NMI_RTC_PORT);
	enable = enable & ENABLE_NMI;
	outb(enable, NMI_RTC_PORT);

	/* clear chain */
	open_rtcs = NULL;

	/* set defualt frequency */
	rtc_modify_freq(RTC_FREQ);
}


/* handle the rtc interrupt */
void rtc_handle_interrupt()
{
	file_t *rtc;
	pcb_t *pcb;
	int32_t i;
	uint32_t flags;

	/* read a byte from reg c to allow interrupts to continue */
	outb(REG_C, NMI_RTC_PORT);
	inb(RTC_RAM_PORT);

	/*Call the test interrupts function to be sure of interrupts occurring */
	//test_interrupts();

	/* XXX: if we have a process running, manage any open virtual rtcs,
	 * this currently only touches the currently running process,
	 * and will need to be modified once scheduling is implemented */
	if (nprocs > 0) {
		cli_and_save(flags);

		pcb = get_proc_pcb();
		if (!pcb) {
			return;
		}
		for (rtc = open_rtcs; rtc; rtc = (file_t *)rtc->reserved) {
			if (rtc_virt_decr_ctr(rtc) == 0) {
				rtc_virt_rst_ctr(rtc);
				rtc_virt_incr_ticks(rtc);
				rtc_virt_set_ticked(rtc);
			}
		}
		restore_flags(flags);
	}
}


/*modify the frequency of the RTC (Min 2Hz - Max 1024 Hz)*/
void rtc_modify_freq(uint32_t freq)
{
	uint32_t flags;

	cli_and_save(flags);

	uint8_t regA;
	uint32_t new_freq;

	/*Disable NMI and select register A of RTC*/
	outb(REG_A, NMI_RTC_PORT);

	/*clear the lower 4 bits of regA to clear out previous frequency*/
	regA = inb(RTC_RAM_PORT);
	regA = regA & 0xF0;

	outb(REG_A, NMI_RTC_PORT);

	/*Set frequency, where the RTC will be 2^freq*/
	new_freq = BASE_FREQ - freq;
	outb((regA | new_freq), RTC_RAM_PORT);

	restore_flags(flags);
}

/*Loops through until an RTC interrupt is generated*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
{
	file_t *rtc;
	uint32_t ticks;
	uint32_t flags;

	rtc = get_file_from_fd(fd);
	if (!rtc || !(rtc->flags & (FILE_OPEN | FILE_RTC))) {
		return -1;
	}

	/* wait until there are ticks to return */
	while (!rtc_virt_has_ticked(rtc)) {
		asm("hlt");
	}

	/* don't get interrupted when messing with rtc counts */
	cli_and_save(flags);
	rtc_virt_clr_ticked(rtc);

	ticks = rtc_virt_ticks(rtc);
	memcpy(buf, &ticks, min(sizeof ticks, (uint32_t)nbytes));
	rtc_virt_clr_ticks(rtc);
	restore_flags(flags);

	return 0;
}

/*Write a new interrupt frequency to the RTC*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)
{
	/*shift counter variable*/
	uint32_t sc = 0;
	uint32_t freq;
	uint32_t req_freq;
	uint32_t flags;
	file_t *rtc;

	if (!buf || nbytes != sizeof req_freq) {
		return -1;
	}

	rtc = get_file_from_fd(fd);
	if (!rtc || !(rtc->flags & (FILE_OPEN | FILE_RTC))) {
		return -1;
	}

	/* get requested frequency */
	memcpy(&req_freq, buf, sizeof req_freq);

	/*find the number of shifts taken to change freq to 0*/
	freq = req_freq;
	while (freq) {
		freq >>= 1;
		sc++;
	}

	/* decrement by one because sc holds the 1-based index of the highest bit
	 * position, which is one more than the power of the two */
	sc--;

	/*check if the requested freq was a power of 2*/
	if (req_freq == (1 << sc)) {
		cli_and_save(flags);
		rtc_virt_set_freq(rtc, sc);
		rtc_virt_clr_ticked(rtc);
		rtc_virt_clr_ticks(rtc);
		rtc_virt_rst_ctr(rtc);
		restore_flags(flags);

		return 4;
	}

	return -1;
}

/*Modify rtc to default freq of 2Hz*/
int32_t rtc_open(const uint8_t* filename)
{
	int32_t fd;
	file_t *file;
	uint32_t flags;

	fd = get_unused_fd();
	if (fd < 0) {
		return -1;
	}

	file = get_file_from_fd(fd);
	file->flags |= FILE_OPEN | FILE_RTC;
	file->file_op = &rtc_fops;
	/* clear */
	file->file_pos = 0;
	file->inode_ptr = -1;

	/*sets the rtc to 2_Hz by default*/
	cli_and_save(flags);
	rtc_virt_set_freq(file, HZ_2);
	rtc_virt_clr_ticked(file);
	rtc_virt_clr_ticks(file);
	rtc_virt_rst_ctr(file);

	/* update rtc chain */
	file->reserved = (int32_t)open_rtcs;
	open_rtcs = file;
	restore_flags(flags);

	return fd;
}

/*RTC Close*/
int32_t rtc_close(int32_t fd)
{
	file_t **last_rtc, *el;
	file_t *rtc;

	rtc = get_file_from_fd(fd);

	/* remove rtc from chain */
	last_rtc = &open_rtcs;
	el = *last_rtc;

	/* find the rtc we're removing, keep track of its parent */
	while (el && el != rtc) {
		last_rtc = (file_t **)&el->reserved;
		el = *last_rtc;
	}

	/* if we found something */
	if (el) {
		/* remove it from list */
		*last_rtc = (file_t *)&el->reserved;
	}

	release_fd(fd);
	return 0;
}

