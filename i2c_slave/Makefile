#
# Makefile - debug
#
# Author: Rick Kimball -- Modified by Huan Truong
# email: rick@kimballsoftware.com
# email: htruong@tnhh.net
# Version: 1.00 Initial version 05-12-2011

CC=msp430-gcc
CXX=msp430-g++
MCU=msp430fr5739

CFLAGS=-mmcu=$(MCU) -O2 -g -Wall

APP=main
APP2=master

all: $(APP).elf

$(APP).elf: $(APP).o $(APP2).o
	$(CC) $(CFLAGS) -o $(APP).elf $(APP).o 
	$(CC) $(CFLAGS) -o $(APP2).elf $(APP2).o 
	msp430-objdump -DS $(APP).elf >$(APP).lst
	msp430-size $(APP).elf
	
%.o : %.c
	$(CC) $(CFLAGS) -c $<

$(APP).o:	$(APP).c
	$(CC) $(CFLAGS) -c -o $(APP).o $(APP).c
	
install:
	mspdebug -q --force-reset rf2500 "load $(APP).elf"

install2:
	mspdebug -q --force-reset rf2500 "load $(APP2).elf"

clean:
	rm -f $(APP).o $(DEBUG).o $(APP).elf $(APP).lst
