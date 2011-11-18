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

DEBUG=debug

all: $(APP).elf

$(APP).elf: $(APP).o
	$(CC) $(CFLAGS) -o $(APP).elf $(APP).o $(DEBUG).o
	msp430-objdump -DS $(APP).elf >$(APP).lst
	msp430-size $(APP).elf
	
%.o : %.c
	$(CC) $(CFLAGS) -c $<

$(APP).o:	$(DEBUG).o $(APP).c
	$(CC) $(CFLAGS) -c -o $(APP).o $(APP).c
	
install:
	mspdebug -q --force-reset rf2500 "load $(APP).elf"

clean:
	rm -f $(APP).o $(DEBUG).o $(APP).elf $(APP).lst
