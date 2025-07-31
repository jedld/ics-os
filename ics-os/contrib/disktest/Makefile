# Makefile for disktest - ICS-OS Disk I/O Performance Test
# Based on the hello example Makefile

NAME=disktest
SRC=$(NAME).c
EXE=$(NAME).exe

CC=gcc
ICSOS_ROOT=../..
SDK=../../sdk
CFLAGS=-m32 -w -nostdlib -fno-builtin -static
LIBS=$(SDK)/tccsdk.c $(SDK)/libtcc1.c $(SDK)/crt1.c 

all: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) -o$(EXE) $(SRC) $(LIBS)

clean:
	rm -f $(EXE)

install: $(EXE)
	cp $(EXE) $(ICSOS_ROOT)/apps

uninstall:
	rm $(ICSOS_ROOT)/apps/$(EXE)

.PHONY: all clean install uninstall
