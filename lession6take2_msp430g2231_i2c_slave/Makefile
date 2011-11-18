#
# Makefile - usci_serial
#
# License: Do with this code what you want. However, don't blame
# me if you connect it to a heart pump and it stops.  This source
# is provided as is with no warranties. It probably has bugs!!
# You have been warned!
#
# Author: Rick Kimball
# email: rick@kimballsoftware.com
# Version: 1.00 Initial version 05-12-2011

CC=msp430-gcc
CXX=msp430-g++
MCU=msp430g2231

CFLAGS=-mmcu=$(MCU) -O2 -g -Wall

APP=main

all: $(APP).elf

$(APP).elf: $(APP).o
	$(CC) $(CFLAGS) -o $(APP).elf $(APP).o I2C.o
	msp430-objdump -DS $(APP).elf >$(APP).lst
	msp430-size $(APP).elf
	
%.o : %.c
	$(CC) $(CFLAGS) -c $<

$(APP).o:	I2C.o $(APP).c
	$(CC) $(CFLAGS) -c -o $(APP).o $(APP).c
	
install:
	mspdebug -q --force-reset rf2500 "prog $(APP).elf"

clean:
	rm -f $(APP).o $(APP).elf $(APP).lst
