# USB Drive Mounting Guide for ICS-OS

## 📋 Prerequisites

1. USB mass storage device must be detected first
2. Use `usb` command to verify device detection

## 🔍 Step 1: Check USB Device Status

```bash
# Check if USB controllers are detected
usb controllers

# Check if USB mass storage devices are detected  
usb devices

# Show all USB information
usb
```

## 📂 Step 2: List Available Block Devices

```bash
# List all registered block devices
lsdev
```

Look for USB devices named like `usb0`, `usb1`, etc.

## 🗂️ Step 3: Mount the USB Drive

### Basic Mount Command:
```bash
mount <filesystem> <device> <mount_point>
```

### Examples:

#### Mount USB drive with FAT filesystem:
```bash
mount fat usb0 /mnt/usb
```

#### Mount USB drive with FAT32:
```bash
mount fat usb0 /tmp/usb
```

## 📁 Step 4: Access Mounted Files

```bash
# Change to mounted directory
cd /mnt/usb

# List files on USB drive
ls

# Display file contents
type filename.txt
```

## 🔧 Step 5: Unmount When Done

```bash
# Unmount the USB drive
umount /mnt/usb
```

## ⚠️ Troubleshooting

### If USB device not detected:
1. Check if USB controller is found: `usb controllers`
2. Try different USB port
3. Ensure USB drive is FAT/FAT32 formatted
4. Check if device appears in `lsdev`

### If mount fails:
1. Verify device name with `lsdev`
2. Create mount point first: `mkdir /mnt/usb`
3. Try different filesystem type (fat, fat32)
4. Check if already mounted: try `umount` first

## 💡 Supported Filesystems

- **fat** - FAT16/FAT32 filesystems
- **cdfs** - CD-ROM/ISO9660 filesystems (for CD/DVD drives)

## 📋 Common Device Names

- `usb0` - First USB mass storage device
- `usb1` - Second USB mass storage device  
- `fd0` - Floppy drive
- `hd0a` - IDE hard drive first partition

## 🎯 Real Hardware vs QEMU

### On Real Hardware:
- USB drives will be detected automatically if supported
- May require specific USB controller drivers
- Detection depends on hardware compatibility

### In QEMU Simulation:
- Virtual USB drives created via QEMU parameters
- Simulated devices for testing purposes
- Use `boot-usb-test` targets for testing

## 📖 Example Session

```bash
ICS-OS> usb
USB: Found 1 controller(s)
USB: Found 1 mass storage device(s)

ICS-OS> lsdev
Device List:
usb0: USB Mass Storage Device #0

ICS-OS> mkdir /mnt/usb
ICS-OS> mount fat usb0 /mnt/usb
mount successful.

ICS-OS> cd /mnt/usb
ICS-OS> ls
readme.txt
test.txt

ICS-OS> type readme.txt
Hello from USB thumb drive!

ICS-OS> cd /
ICS-OS> umount /mnt/usb
```

This guide covers both simulation (QEMU) and real hardware scenarios!
