# USB Driver Makefile for ICS-OS
# Date: 29/07/25

CC=gcc
CFLAGS=-fno-stack-protector -std=gnu99 -m32 -w -nostdlib -fno-builtin -ffreestanding -c -g -fno-pie -fno-pic -I../../.. -I../../../stdlib
AS=nasm
ASFLAGS=-f elf32

OBJECTS=usb.o usb_mass_storage.o

all: usb_driver.o

usb_driver.o: $(OBJECTS)
	ld -r -o usb_driver.o $(OBJECTS)

usb.o: usb.c usb.h
	$(CC) -fno-stack-protector -std=gnu99 -m32 -w -nostdlib -fno-builtin -ffreestanding -g -fno-pie -fno-pic -I../../.. -I../../../stdlib -c usb.c -o usb.o

usb_mass_storage.o: usb_mass_storage.c usb.h
	$(CC) -fno-stack-protector -std=gnu99 -m32 -w -nostdlib -fno-builtin -ffreestanding -g -fno-pie -fno-pic -I../../.. -I../../../stdlib -c usb_mass_storage.c -o usb_mass_storage.o

clean:
	rm -f *.o

.PHONY: all clean
