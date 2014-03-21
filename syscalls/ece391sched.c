#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024

int main ()
{
    int32_t val, cnt;
	uint8_t buf[BUFSIZE];

    ece391_fdputs (1, (uint8_t*)"Enter the Test Number: (0): 10, (1): 10000, (2): 1000000\n");
	if (-1 == (cnt = ece391_read (0, buf, BUFSIZE-1))) {
        ece391_fdputs (1, (uint8_t*)"Can't read the number from keyboard.\n");
		return 3;
	}	

	if ((ece391_strlen(buf) > 2) || ((ece391_strlen(buf) == 1) && (buf[0] < 48) || (buf[0] > 50)))
	{
		ece391_fdputs (1, (uint8_t*)"Wrong Choice!\n");
		return 0;
	}
	else
	{
		switch(buf[0]){		
			case 48:
				cnt = 10;
				break;
			case 49:
				cnt = 10000;
				break;
			case 50:
				cnt = 1000000;
				break;
		}
	}

	for (val = 0; val < cnt; val++)
	{
		itoa(val+1, buf, 10);
		ece391_fdputs (1, (uint8_t*) buf);		
		ece391_fdputs (1, (uint8_t*) "\n");		
	}
    return 0;
}

