![Alt Text](./ics-os.gif)

## About

Modern real-world operating systems are too complex to be taught to undergraduates and other instructional operating systems are not complete and usable and do not work on real hardware. By providing students with a _not so complex_ working operating system to play with, they will be able to appreciate and understand deeper the concepts underlying an operating system.

Thus, this project aims to develop a simple yet operational instructional operating system for teaching undergraduate operating systems courses. ICS-OS is a fork of <a href='http://sourceforge.net/projects/dex-os'>DEX-OS</a> by Joseph Dayo.

## Downloads

Latest floppy image: <a href='https://github.com/srg-ics-uplb/ics-os/raw/master/ics-os/ics-os-floppy.img'>ics-os-floppy.img</a>

Test the floppy image in qemu.
```
$qemu-system-i386 -fda ics-os-floppy.img
```

## USB Boot (BIOS)

You can build a bootable USB image and test it on real hardware or in QEMU.

### Build a USB image

From the ics-os directory:

```
$ make usb
```

This produces ics-os-usb.img. The helper uses scripts/create-usb-image.sh and requires root to set up a loop device and install GRUB.

### Write to a thumb drive (Linux)

**Warning:** This will erase the target drive.

```
$ sudo dd if=ics-os-usb.img of=/dev/sdX bs=4M status=progress conv=fsync
```

Replace /dev/sdX with your USB device (not a partition like /dev/sdX1).

### Boot in QEMU

```
$ make boot-usb
```

### Boot on real hardware

Enable legacy BIOS/CSM boot and select the USB device from the BIOS boot menu.

### Notes

- The USB image is FAT32 and uses GRUB in BIOS mode.
- USB Mass Storage hot-plug is supported; inserted drives appear as usbX devices and can be mounted via `mount fat usbX /usbX`.

## Zork (Z-Machine) Port

We can run Zork by porting a GPL-compatible Z-Machine interpreter (e.g., Frotz) and using the ICS-OS SDK stdlib.

1) Place interpreter sources in contrib/zork/interpreter.
2) Build and install:

```
$ make -C ics-os/contrib/zork
$ make -C ics-os/contrib/zork install
```

3) Copy your legally obtained Zork story file (e.g., ZORK1.DAT) into the boot image and run:

```
zork.exe /ZORK1.DAT
```

See [ics-os/contrib/zork/README.md](ics-os/contrib/zork/README.md) for details and licensing notes.
## Build Environment

Ubuntu 16.04 64-bit is the last tested working build environment. Virtualbox can be used 
to run this version of Ubuntu.  However, docker is the recommended build environment since new versions 
of Ubuntu do not work anymore.

### Using Docker to build

ICS-OS is a 32-bit operating system and requires a 32-bit build environment. You need to install 
[docker](https://docs.docker.com/engine/install/ubuntu/) and [docker-compose](https://docs.docker.com/compose/install/) 
to build the ICS-OS kernel and user applications.

We have provided a helper script to simplify the build process using Docker.

To build the floppy image:
```
$ ./build.sh
```

To clean the build:
```
$ ./build.sh clean
```

Alternatively, you can run the docker commands directly (see `docker-compose.yml` for details), but using `build.sh` is recommended.

Make the floppy image then boot.

```
$ ./build.sh
```

You can boot the floppy image directly using qemu.
```
$ qemu-system-i386 -fda ics-os/ics-os-floppy.img
```

See [Lab 01](https://github.com/srg-ics-uplb/ics-os/blob/master/labs/lab01/ICSOS_Lab01.pdf) for a more complete discussion of how 
to set up the build environment.

## Development and Support
This project is used at the <a href='http://www.ics.uplb.edu.ph'>Institute of Computer Science</a>, <a href='http://www.uplb.edu.ph'>University of the Philippines Los Banos</a> for <a href='http://ics.uplb.edu.ph/courses/ugrad/cmsc/125'>CMSC 125</a>. It is maintained by the <a href='https://sites.google.com/up.edu.ph/systems-research'>Systems Research Group</a>.

Get started by reading the <a href="https://github.com/srg-ics-uplb/ics-os/wiki/Kernel-Developer's-Guide">Kernel Developer's Guide</a>.

Don't forget to check the <a href="http://github.com/srg-ics-uplb/ics-os/wiki">Wiki</a>.

You can ask questions by submitting an issue.

## Citation

If you find his resource useful in your research or teaching, please cite our [paper](https://jachermocilla.org/publications/hermocilla-pitj2009-ics-os.pdf).

---

J. A. C. Hermocilla. Ics-os: A kernel programming approach to teaching operating system concepts. Philippine Information Technology Journal, 2(2):25--30, 2009.

---

You can also use the following bibtex entry.

```
@article{hermocilla-ics-os-pitj2009,
  author = {Hermocilla, J. A. C.},
  title = {ICS-OS: A Kernel Programming Approach to Teaching Operating System Concepts},
  journal = {Philippine Information Technology Journal},
  volume = {2},
  number = {2},
  year = {2009},
  issn = {2012-0761},
  pages = {25--30},
  publisher = {Philippine Society of Information Technology Educators and Computing Society of the Philippines },
  address = {Philippines},
  pdf = {https://jachermocilla.org/publications/hermocilla-pitj2009-ics-os.pdf}
}
```
