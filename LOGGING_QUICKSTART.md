# Quick Start Guide: ICS-OS Kernel Logging

## What's New

ICS-OS now has a modern kernel logging system that can output logs to:
1. **Console** - The traditional screen output
2. **Serial Port** - For debugging with QEMU (recommended)
3. **Both** - Console and serial simultaneously

## Why Serial Logging?

Serial port logging with QEMU provides several advantages:
- **Persistent logs**: Saved to files, won't disappear when screen scrolls
- **Real-time monitoring**: Use `tail -f` to watch logs in real-time
- **Better debugging**: Detailed debug messages without cluttering the console
- **QEMU integration**: Easy redirection to files, network, or other outputs

## Quick Test

1. Build the kernel:
```bash
cd ics-os/kernel
make
```

2. Run with serial logging:
```bash
qemu-system-i386 \
  -cdrom ../ics-os-uefi.iso \
  -serial file:kernel.log \
  -m 128M
```

3. Monitor logs in another terminal:
```bash
tail -f kernel.log
```

## Key Changes Made

### New Files Added:
- `kernel/hardware/serial/serial.h` - Serial port driver interface
- `kernel/hardware/serial/serial.c` - Serial port driver implementation
- `kernel/debug/klog.h` - Kernel logging system interface  
- `kernel/debug/klog.c` - Kernel logging system implementation

### USB Driver Enhanced:
The USB driver (`kernel/hardware/usb/usb.c`) now uses structured logging:
- `usb_info()` - General USB information
- `usb_debug()` - Detailed USB debugging
- `usb_warn()` - USB warnings
- `usb_err()` - USB errors

### Kernel Integration:
- Logging system initialized early in `kernel32.c`
- Available throughout the kernel
- Serial port configured for QEMU compatibility

## Usage Examples

### For Kernel Developers:

```c
#include "debug/klog.h"

// Simple logging (uses KERNEL subsystem)
kinfo("System initialized");
kwarn("Low memory: %d KB", available);
kerr("Failed to load module: %s", name);

// Subsystem-specific logging
usb_debug("Port %d status: 0x%04X", port, status);
mem_warn("Memory fragmentation detected");
```

### For System Administrators:

```bash
# Save all kernel output to file
qemu-system-i386 -serial file:full.log [other options]

# Show only USB-related messages
grep USB full.log

# Monitor logs in real-time
tail -f full.log | grep -i "warn\|err\|crit"
```

## Log Message Format

```
[timestamp] [SUBSYSTEM:LEVEL] message
```

Example:
```
[12.345] [USB:INFO] Device detected on port 1
[12.346] [USB:DEBUG] Port status: 0x0101
[12.347] [KERNEL:WARN] Low memory condition
```

## Integration with USB Testing

This logging system significantly improves the USB testing workflow described in `USB_TESTING_GUIDE.md`:

1. **Clear USB Messages**: Structured USB logs make it easy to see what's happening
2. **Debug vs Production**: Can show detailed USB operations or just important events
3. **Historical Analysis**: Save logs to analyze USB behavior over time
4. **QEMU Integration**: Perfect for testing USB device simulation vs real hardware

## Next Steps

1. Try the test script: `./test-logging.sh`
2. Read the full documentation: `KERNEL_LOGGING.md`
3. Update your QEMU commands to include `-serial file:debug.log`
4. Use the new USB logging messages for better debugging

This logging system makes ICS-OS development much more professional and easier to debug!
