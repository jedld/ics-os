# ICS-OS UEFI and USB Boot Support

## Overview

ICS-OS now supports modern boot methods including:
- **UEFI Boot**: Compatible with modern computers that use UEFI firmware
- **USB Boot**: Create bootable USB drives for physical hardware testing
- **Hybrid Images**: Boot on both BIOS and UEFI systems from the same image

## Quick Start

### Build UEFI-Compatible ISO
```bash
cd ics-os
make uefi-iso
```

### Build USB Bootable Image
```bash
cd ics-os
make usb-image
```

### Build Both ISO and USB Images
```bash
cd ics-os
make uefi-all
```

## Testing with QEMU

### Test UEFI Boot
```bash
# UEFI ISO testing
make boot-uefi-iso

# USB UEFI testing  
make boot-usb-uefi

# Fallback BIOS testing
make boot-uefi-iso-fallback
```

### Manual QEMU Commands
```bash
# UEFI ISO
qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom ics-os-uefi.iso -m 256M

# BIOS compatibility
qemu-system-x86_64 -cdrom ics-os-uefi.iso -m 256M

# USB image
qemu-system-x86_64 -drive format=raw,file=ics-os-usb.img -m 256M
```

## Creating Bootable USB Drives

### Method 1: Direct Write (Linux/macOS)
```bash
# Build the USB image
make usb-image

# Find your USB device (be very careful!)
lsblk

# Write to USB drive (replace /dev/sdX with your USB device)
sudo dd if=ics-os-usb.img of=/dev/sdX bs=1M status=progress
sudo sync
```

### Method 2: Using the Build Script on Host
```bash
# Run on the host system (not in Docker) for loop device access
cd ics-os
./scripts/build-uefi.sh usb

# The script will create ics-os-usb.img which can then be written to USB
```

### Method 3: Manual USB Setup
```bash
# Format USB drive with FAT32
sudo mkfs.fat -F32 /dev/sdX1

# Mount USB drive
sudo mount /dev/sdX1 /mnt/usb

# Copy ICS-OS files
sudo cp -r ics-os/tmp-uefi/* /mnt/usb/

# Install GRUB
sudo grub-install --target=i386-pc --boot-directory=/mnt/usb/boot /dev/sdX

# Unmount
sudo umount /mnt/usb
```

## UEFI Implementation Details

### Boot Process
1. **UEFI Detection**: GRUB automatically detects UEFI vs BIOS mode
2. **Platform-Specific Menus**: Different boot options for UEFI and BIOS
3. **Multiboot Support**: ICS-OS kernel loads via multiboot protocol
4. **Hybrid Compatibility**: Same image works on both UEFI and BIOS systems

### GRUB Configuration Features
- Platform detection (`$grub_platform`)
- Automatic root device detection
- Video driver loading for UEFI graphics
- Safe mode options
- System information display

### File Structure
```
ics-os-uefi.iso/
├── boot/grub/grub.cfg          # Main GRUB configuration
├── EFI/BOOT/grub.cfg           # UEFI-specific boot config
├── vmdex                       # ICS-OS kernel
├── apps/                       # Applications
├── tcc1/                       # TinyC compiler
└── lib1/                       # Libraries
```

## Troubleshooting

### UEFI Boot Issues
```bash
# Enable UEFI verbose output
qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom ics-os-uefi.iso -m 256M -serial stdio

# Check GRUB modules
grub-mkrescue --help | grep -i modules
```

### USB Boot Issues
```bash
# Verify USB image structure
file ics-os-usb.img
fdisk -l ics-os-usb.img

# Test USB image in QEMU first
qemu-system-x86_64 -drive format=raw,file=ics-os-usb.img -m 256M -serial stdio
```

### Build Dependencies
Ensure all required packages are installed:
```bash
# Ubuntu/Debian
sudo apt-get install grub-pc-bin grub-efi-amd64-bin grub-efi-ia32-bin xorriso mtools dosfstools parted ovmf

# Update Docker image
docker-compose build ics-os-build
```

## Hardware Compatibility

### Tested Configurations
- ✅ QEMU/KVM (BIOS and UEFI)
- ✅ VirtualBox (with UEFI enabled)
- ✅ VMware Workstation
- ✅ Modern laptops with UEFI
- ✅ Legacy BIOS systems

### Known Limitations
- 32-bit kernel only (runs on 64-bit hardware in compatibility mode)
- Basic video drivers (no modern GPU acceleration)
- Limited hardware driver support
- No Secure Boot support

## Development Notes

### Adding Secure Boot (Future)
To add Secure Boot support in the future:
1. Sign the kernel with a valid certificate
2. Use shim bootloader
3. Update GRUB configuration for signed boot

### 64-bit Kernel Support
For full 64-bit support:
1. Convert kernel to x86_64 architecture
2. Update memory management
3. Modify multiboot headers
4. Update device drivers

## Files Modified/Added

### New Files
- `scripts/build-uefi.sh` - UEFI/USB image builder
- `boot/grub/grub-uefi.cfg` - UEFI-specific GRUB config
- `UEFI-BOOT.md` - This documentation

### Modified Files
- `Dockerfile` - Added UEFI build tools
- `Makefile` - Added UEFI/USB targets
- `.gitignore` - Added UEFI image exclusions

### Build Output
- `ics-os-uefi.iso` - Hybrid BIOS/UEFI bootable ISO
- `ics-os-usb.img` - USB bootable image
- `tmp-uefi/` - Temporary build directory

This implementation provides modern boot support while maintaining compatibility with the educational nature of ICS-OS.
