CC=gcc -m32

all: fish

# Note that you must be superuser to run the emulated version of the
# program.

fish_emulated: fish.o blink.o ece391emulate.o ece391support.o
	$(CC) -nostdlib -lc -g -o fish_emulated fish.o blink.o ece391emulate.o ece391support.o

fish: fish.exe
	../elfconvert fish.exe
	mv fish.exe.converted fish

fish.exe: fish.o blink.o ece391support.o ece391syscall.o
	$(CC) -nostdlib -g -o fish.exe fish.o blink.o ece391syscall.o ece391support.o

%.o: %.S
	$(CC) -nostdlib -c -Wall -g -D_USERLAND -D_ASM -o $@ $<

%.o: %.c
	$(CC) -nostdlib -Wall -c -g -o $@ $<

clean::
	rm -f *.o *~
clear: clean
	rm fish fish.exe fish_emulated
