CC = msp430-gcc
CFLAGS =-Os -Wall -g -mmcu=msp430fr5739
PATCHED_MSPDEBUG=mspdebug

all : $(OBJECTS)
	$(CC) $(CFLAGS) main.c -o main.elf

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -fr $(OBJECTS) main.elf

install: all
	$(PATCHED_MSPDEBUG) rf2500 "load main.elf"

size: all
	msp430-size main.elf
