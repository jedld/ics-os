CC=gcc
# Updatobj: scheduler.o fat.o iso9660.o devfs.o iomgr.o devmgr_error.o kernel32.o \
		startup.o asmlib.o irqwrap.o 
	#strip --strip-debug *.oLAGS for modern GCC with 32-bit target support
CFLAGS=-fno-stack-protector -std=gnu99 -m32 -w -nostdlib -fno-builtin -ffreestanding -c -g -fno-pie -fno-pic
ASM=nasm
ASMFLAGS=-f elf32

bzImage: all
	objcopy --only-keep-debug Kernel32.bin Kernel32.sym
	objcopy --strip-debug Kernel32.bin
	gzip -c -9 Kernel32.bin >  vmdex
	cp vmdex ..

all: obj Kernel32.bin

obj: scheduler.o fat.o iso9660.o devfs.o exfat.o iomgr.o devmgr_error.o kernel32.o \
		startup.o asmlib.o irqwrap.o usb.o usb_mass_storage.o usb_uhci.o usb_enum.o
	#strip --strip-debug *.o
		
kernel32.o: kernel32.c build.h
	$(CC) $(CFLAGS) -o kernel32.o kernel32.c 
	
scheduler.o:
	$(CC) $(CFLAGS) -o scheduler.o process/scheduler.c

fat.o:	
	$(CC) $(CFLAGS) -o fat.o filesystem/fat12.c
	
iso9660.o:
	$(CC) $(CFLAGS) -o iso9660.o filesystem/iso9660.c
	
devfs.o:
	$(CC) $(CFLAGS) -o devfs.o filesystem/devfs.c

exfat.o:
	$(CC) $(CFLAGS) -o exfat.o filesystem/exfat.c
	
iomgr.o:
	$(CC) $(CFLAGS) -o iomgr.o iomgr/iosched.c
	
devmgr_error.o:
	$(CC) $(CFLAGS) -o devmgr_error.o devmgr/devmgr_error.c

startup.o:
	$(ASM) $(ASMFLAGS) -o startup.o startup/startup.asm
	
asmlib.o:
	$(ASM) $(ASMFLAGS) -o asmlib.o startup/asmlib.asm 

irqwrap.o:
	$(ASM) $(ASMFLAGS) -o irqwrap.o irqwrap.asm

usb.o:
	$(CC) $(CFLAGS) -o usb.o hardware/usb/usb.c

usb_mass_storage.o:
	$(CC) $(CFLAGS) -o usb_mass_storage.o hardware/usb/usb_mass_storage.c

usb_uhci.o:
	$(CC) $(CFLAGS) -o usb_uhci.o hardware/usb/usb_uhci.c

usb_enum.o:
	$(CC) $(CFLAGS) -o usb_enum.o hardware/usb/usb_enum.c
	
Kernel32.bin:
	ld -melf_i386 -T lscript.ld -Map mapfile.txt --no-pie

clean:
	rm -f *.o
	rm -f Kernel32.bin
	rm -f Kernel32.sym
	rm -f vmdex
	rm -f Kernel32.sym
	rm -f vmdex
	$(MAKE) -C hardware/usb clean
