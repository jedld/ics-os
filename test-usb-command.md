# USB Console Command Test Guide

## Overview
ICS-OS now includes a USB console command to query and display information about connected USB devices.

## Usage

### Basic USB Command
```
usb
```
Shows both USB controllers and connected devices.

### Specific Queries
```
usb controllers
```
Lists all detected USB controllers with their details:
- Controller type (UHCI/OHCI/EHCI)
- Base address  
- Number of ports
- Connected device count

```
usb devices
```
Lists all USB mass storage devices with details:
- Device address and IDs
- Vendor/Product IDs
- Storage capacity
- Device file path (/dev/usbX)
- Endpoint information

## Implementation Details

### New Functions Added
1. `usb_list_controllers()` - Shows USB controller information
2. `usb_list_devices()` - Shows USB mass storage device details
3. Console command handler for "usb" in console.c

### Files Modified
- `kernel/hardware/usb/usb.h` - Added function prototypes
- `kernel/hardware/usb/usb.c` - Implemented query functions
- `kernel/console/console.c` - Added console command handler
- `base/icsos.hlp` - Updated help documentation

### Expected Output Format

#### USB Controllers:
```
USB Controllers detected: 2
=================================
Controller 0: UHCI
  Base Address: 0x1820
  Ports: 2
  Devices: 0

Controller 1: EHCI  
  Base Address: 0x1840
  Ports: 8
  Devices: 1
```

#### USB Devices:
```
USB Mass Storage devices: 1
===============================
Device 0: USB0
  Address: 2
  Vendor ID: 0x0951
  Product ID: 0x1666
  Device Class: 0x00
  Block Size: 512 bytes
  Total Blocks: 15663104
  Capacity: 7651 MB
  Bulk IN Endpoint: 0x81
  Bulk OUT Endpoint: 0x02
  Device File: /dev/usb0
```

## Testing Instructions

1. Boot ICS-OS with USB support
2. Insert a USB thumb drive (optional)
3. At the console prompt, type: `usb`
4. Verify controller detection
5. If USB device is present, verify device information
6. Test individual commands: `usb controllers`, `usb devices`
7. Check help system: `help` (should show usb command)

## Educational Value

This command provides students with:
- Understanding of USB controller detection
- Insight into USB device enumeration
- PCI bus scanning demonstration  
- Mass storage device properties
- Device manager integration examples
