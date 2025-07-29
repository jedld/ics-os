# Kernel Logging System for ICS-OS

## Overview

ICS-OS now includes a comprehensive kernel logging system that provides structured logging with multiple output targets and log levels. This system can output kernel logs to both the console and to a serial port, which is especially useful for debugging and development with QEMU.

## Features

- **Multiple Log Levels**: EMERG, ALERT, CRIT, ERR, WARNING, NOTICE, INFO, DEBUG
- **Multiple Output Targets**: Console, Serial Port, or Both
- **Serial Port Support**: Uses COM1 (0x3F8) for QEMU serial redirection
- **Subsystem Tagging**: Organize logs by kernel subsystem (USB, MEMORY, PROCESS, etc.)
- **Timestamps**: Optional timestamp support
- **Color Support**: Optional color coding for different log levels
- **Format String Support**: Full printf-style formatting

## Serial Port Integration with QEMU

### QEMU Serial Redirection

The kernel logging system can output to COM1 serial port, which QEMU can redirect to various targets:

#### 1. Output to Terminal (stdio)
```bash
qemu-system-i386 -serial stdio [other options]
```

#### 2. Output to File
```bash
qemu-system-i386 -serial file:kernel.log [other options]
```

#### 3. Output to TCP Socket (for remote debugging)
```bash
qemu-system-i386 -serial tcp:localhost:4444,server,nowait [other options]
```

#### 4. Output to Named Pipe
```bash
qemu-system-i386 -serial pipe:kernel_pipe [other options]
```

### Example QEMU Command for Kernel Logging

```bash
# Basic serial logging to file
qemu-system-i386 \
  -device piix3-usb-uhci \
  -device usb-storage,drive=usb-drive \
  -drive id=usb-drive,file=usb-thumb.img \
  -serial file:kernel.log \
  -drive file=ics-os-uefi.iso,index=0,media=cdrom

# Serial logging to both file and stdio
qemu-system-i386 \
  -device piix3-usb-uhci \
  -device usb-storage,drive=usb-drive \
  -drive id=usb-drive,file=usb-thumb.img \
  -serial stdio \
  -serial file:debug.log \
  -drive file=ics-os-uefi.iso,index=0,media=cdrom
```

## Usage Examples

### Basic Logging Macros

```c
#include "debug/klog.h"

// Simple kernel messages (no subsystem specified)
kinfo("System initialized successfully");
kwarn("Memory low: %d KB remaining", available_kb);
kerr("Failed to mount filesystem: error code %d", error);
kdebug("Debug value: x=%d, y=%d", x, y);

// Subsystem-specific logging
usb_info("USB controller initialized at base 0x%04X", base_addr);
usb_debug("Port %d status: 0x%04X", port, status);
mem_warn("Page allocation failed for process %d", pid);
```

### Advanced Logging

```c
// Direct logging with custom subsystem
klog_printf(KLOG_INFO, "MYDRV", "Driver loaded: version %s", version);

// Change log level at runtime
klog_set_level(KLOG_DEBUG);  // Show all messages including debug

// Change output target
klog_set_target(KLOG_TARGET_SERIAL);  // Serial only
klog_set_target(KLOG_TARGET_BOTH);    // Both console and serial
```

### Log Configuration

```c
// Check current configuration
klog_show_config();

// Enable/disable features
klog_enable_timestamps(1);  // Enable timestamps
klog_enable_colors(1);      // Enable color coding
klog_enable_subsystem_tags(1); // Enable subsystem tags
```

## Log Levels

| Level   | Value | Description               | Usage |
|---------|-------|---------------------------|-------|
| EMERG   | 0     | System is unusable        | System panic, critical hardware failure |
| ALERT   | 1     | Action must be taken      | Hardware error requiring immediate attention |
| CRIT    | 2     | Critical conditions       | Critical system errors |
| ERR     | 3     | Error conditions          | Recoverable errors |
| WARNING | 4     | Warning conditions        | Potential problems |
| NOTICE  | 5     | Normal but significant    | Important normal operations |
| INFO    | 6     | Informational messages    | General information |
| DEBUG   | 7     | Debug-level messages      | Detailed debugging information |

## Subsystem Tags

The logging system supports the following predefined subsystem tags:

- `KERN` - General kernel messages
- `MEM` - Memory management
- `PROC` - Process management
- `VFS` - Virtual File System
- `USB` - USB subsystem
- `NET` - Network subsystem
- `SER` - Serial port driver
- `CON` - Console system
- `DEV` - Device management

## Output Format

### Console Output
```
[12.345] [USB:INFO] Device detected on port 1
[12.346] [USB:DEBUG] Port status: 0x0101
[12.347] [KERN:WARN] Low memory condition detected
```

### Serial Output (for QEMU logging)
```
[12.345] [USB:INFO] Device detected on port 1
[12.346] [USB:DEBUG] Port status: 0x0101  
[12.347] [KERN:WARN] Low memory condition detected
```

## Integration with USB Testing

The new logging system significantly improves USB debugging capabilities mentioned in the USB Testing Guide:

### Enhanced USB Debugging

```c
// Replace old printf statements with structured logging
usb_info("Initializing USB subsystem");
usb_debug("Found USB controller: Type=%d, Base=0x%X", type, base);
usb_debug("UHCI controller reset complete");
usb_info("Device detected on port %d, status=0x%04X", port, status);
```

### QEMU USB + Serial Logging Example

```bash
# Run ICS-OS with USB device and kernel logging
qemu-system-i386 \
  -device piix3-usb-uhci \
  -device usb-storage,drive=usb-drive \
  -drive id=usb-drive,file=usb-thumb.img \
  -serial file:usb_debug.log \
  -drive file=ics-os-uefi.iso,index=0,media=cdrom

# Monitor the log file in real-time
tail -f usb_debug.log
```

This will show detailed USB initialization and device detection logs that can help determine whether real USB communication is happening vs. simulation mode.

## Building with Logging Support

The logging system is automatically included in the kernel build process. The following files are added:

- `kernel/hardware/serial/serial.h` - Serial port driver interface
- `kernel/hardware/serial/serial.c` - Serial port driver implementation  
- `kernel/debug/klog.h` - Kernel logging interface
- `kernel/debug/klog.c` - Kernel logging implementation

These files are included in `kernel32.c` and will be compiled automatically with the existing build system.

## Performance Considerations

- Serial output is slower than console output
- Debug level logging can be disabled in production builds
- The logging system uses a fixed-size buffer (512 bytes) for each message
- No logging occurs if the message level is below the configured minimum level

## Future Enhancements

- Network logging support (syslog protocol)
- Log rotation and buffering
- Binary log format for better performance
- Remote debugging protocol support
- Integration with kernel debugger
