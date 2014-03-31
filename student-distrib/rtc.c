/* rtc.c - Functions to interact with the RTC controller
 * vim:ts=4 sw=4 noexpandtab
 */

#include "rtc.h"
#include "lib.h"

/* initialize the RTC */
void rtc_init(void)
{

    uint8_t regB;

    rtc_intf = 0;

    /*Select Register B to set up periodic interrupts*/
    outb((DISABLE_NMI | REG_B), NMI_RTC_PORT);

	/*Extract current value from register B*/
	regB = inb(RTC_RAM_PORT);

	regB = regB | 0x40;		//Enable bit 6 of register B to enable Periodic Interrupts (PIE)
	
	/*Set the same index, because reading from the port sets the index to register D*/
	outb((DISABLE_NMI | REG_B), NMI_RTC_PORT);	
	
	/*Set Register B with enabled PIE*/
	outb(regB, RTC_RAM_PORT);

	/*Re-enable NMI*/
	uint8_t enable = inb(NMI_RTC_PORT);
	enable = enable & ENABLE_NMI;
	outb(enable, NMI_RTC_PORT);
	
}


/* handle the rtc interrupt */
void rtc_handle_interrupt()
{

	/*clear RTC interrupt flag*/
    rtc_intf = 0;

	/* read a byte from reg c to allow interrupts to continue */
	outb(REG_C, NMI_RTC_PORT);
	inb(RTC_RAM_PORT);

	/* do the test interrupts function so we know things are working */
	//test_interrupts(); 

}


/*modify the frequency of the RTC (Min 2Hz - Max 1024 Hz)*/
void rtc_modify_freq(uint32_t freq)
{

  cli();

  uint8_t regA;

  /*Disable NMI and select register A of RTC*/
  outb(REG_A, NMI_RTC_PORT);

  regA = inb(RTC_RAM_PORT);
  regA = regA & 0xF0;       //clear the lower 4 bits of regA to clear out previous frequency

  outb(REG_A, NMI_RTC_PORT);

  /*Set frequency, where it is 2^freq*/
  switch(freq)
  {

    case 1:
      outb((regA | HZ_2), RTC_RAM_PORT);
      break;

    case 2:
      outb((regA | HZ_4), RTC_RAM_PORT);
      break;

    case 3:
      outb((regA | HZ_8), RTC_RAM_PORT);
      break;

    case 4:
      outb((regA | HZ_16), RTC_RAM_PORT);
      break;

    case 5:
      outb((regA | HZ_32), RTC_RAM_PORT);
      break;

    case 6:
      outb((regA | HZ_64), RTC_RAM_PORT);
      break;

    case 7:
      outb((regA | HZ_128), RTC_RAM_PORT);
      break;

    case 8:
      outb((regA | HZ_256), RTC_RAM_PORT);
      break;

    case 9:
      outb((regA | HZ_512), RTC_RAM_PORT);
      break;

    case 10:
      outb((regA | HZ_1024), RTC_RAM_PORT);
      break;

    default:
	
      break;

  }

  sti();

}

/*Loops through until an RTC interrupt is generated*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
{
  
  rtc_intf = 1;

  while(rtc_intf == 1){
  
    continue;
	
  }

  return 0;

}

/*Write a new interrupt frequency to the RTC*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)
{

  uint32_t sc = 0;			//shift counter variable 
  uint32_t freq = (uint32_t)buf;	//temp freq variable used for checking validity of requested freq
  
  /*find the number of shifts taken to change freq to 0*/
  while(freq != 0){
  
	freq = freq >> 1;
	sc++;
	
  }
  
  /*decrement freq by 1 for the sake of rtc_modify_freq implementation*/
  sc--;
  
  /*check if the requested freq was a power of 2*/
  if((uint32_t)buf == (1 << sc)){
  
	rtc_modify_freq(sc);
	return 0;
	
  }
  
  printf("Error: Invlaid Frequency Value\n");
  return -1;
  
}


/*Modify rtc to default freq of 2Hz*/
int32_t rtc_open(const uint8_t* filename)
{
  
  rtc_modify_freq(1); //sets the rtc to 2_Hz by default
  return 0;

}

/*RTC Close*/
int32_t rtc_close(int32_t fd)
{

  /*Nothing to do here yet*/
  return 0;

}

/*Tests rtc_open function*/
void rtc_open_test(void)
{

	uint8_t* rtc_test = 0;
	rtc_open(rtc_test);
	
}

/*Tests rtc_read and rtc_write functions*/
void rtc_rw_test(void)
{

	uint32_t i = 2;
	int32_t fd_test = 0;
	int32_t nbytes_test = 4;
	
	while(i != 2048){
	
		rtc_read(fd_test, (void*)i, nbytes_test);
		rtc_write(fd_test, (void*)i, nbytes_test);
		
		i *= 2;	
	}
	
}


