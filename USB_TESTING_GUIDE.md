# USB Testing Guide for ICS-OS

## Current Implementation Status

The ICS-OS USB implementation has been enhanced with real USB communication capabilities using UHCI Transfer Descriptors. The kernel now includes a comprehensive logging system that can output debug information to both console and serial port for better debugging with QEMU.

## Enhanced Logging with Serial Output

ICS-OS now supports kernel logging to serial port (COM1) which can be redirected by QEMU for better debugging:

```bash
# Run with serial logging to file
qemu-system-i386 -serial file:usb_debug.log [other options]

# Run with serial logging to terminal
qemu-system-i386 -serial stdio [other options]
```

The USB subsystem now uses structured logging with different levels:
- `usb_debug()` - Detailed USB operation logs
- `usb_info()` - General USB information  
- `usb_warn()` - USB warnings
- `usb_err()` - USB errors

## Testing Steps

### 1. Check USB Initialization Messages

When the OS boots, look for these enhanced USB messages:
```
[timestamp] [USB:INFO] Initializing USB subsystem
[timestamp] [USB:INFO] Found USB controller: Type=0, Base=0xXXXX
[timestamp] [USB:INFO] Initializing USB controller 0
[timestamp] [USB:DEBUG] UHCI controller reset complete  
[timestamp] [USB:DEBUG] Detecting devices on controller (Type=0, Base=0xXXXX)
```

### 2. Enhanced Serial Logging for Debugging

To get detailed USB logs for analysis:

```bash
# Run QEMU with USB and serial logging
qemu-system-i386 \
  -device piix3-usb-uhci \
  -device usb-storage,drive=usb-drive \
  -drive id=usb-drive,file=usb-thumb.img \
  -serial file:usb_debug.log \
  -drive file=ics-os-uefi.iso,index=0,media=cdrom

# Monitor logs in real-time  
tail -f usb_debug.log
```

### 3. Look for Real vs Simulated Device Detection

**Real USB Detection Indicators:**
- Messages like: `[USB:INFO] Device detected on port X, status=0xXXXX`
- Control transfer messages: `[USB:DEBUG] Real control transfer - req_type=0xXX, req=0xXX`
- Real VID/PID from actual device (not 0x1234/0x5678)

**Simulation Mode Indicators:**
- Messages like: `[USB:INFO] Creating simulated device for emulation environment`
- VID=0x1234, PID=0x5678 (hardcoded simulation values)
- `[USB:INFO] Simulated mass storage device created (64MB)`

### 4. Test USB Device Commands

Once booted, try these commands:

```bash
# List USB controllers
usb controllers

# List USB devices
usb devices

# Check if USB device is mounted
devmgr

# Try to mount USB device
mount /dev/usb0 /mnt/usb fat32

# Check directory listing
dir /dev/

# Try reading from USB
hexdump /dev/usb0 0 1
```

### 4. Verify Real Hardware Communication

**Key Indicators of Real USB Communication:**

1. **Port Status Reading**: Look for messages showing actual port status values
2. **Transfer Descriptor Creation**: Messages about creating TDs and QHs
3. **USB Address Assignment**: Real SET_ADDRESS requests
4. **Descriptor Reading**: Actual device descriptor values from QEMU virtual device

**Expected vs Simulation Behavior:**

| Feature | Real USB | Simulation |
|---------|----------|------------|
| VID/PID | From virtual device | 0x1234/0x5678 |
| Port Detection | Actual port registers | Fallback creation |
| Transfer Messages | UHCI TD creation | Direct simulation |
| Device Data | From usb-thumb.img | Hardcoded FAT32 |

### 5. Test Data Reading

To verify if actual data is being read from the QEMU virtual USB device:

1. Check the contents of `/extras/usb-thumb.img` on the host
2. Try reading the same data from `/dev/usb0` in ICS-OS
3. Compare to see if real data matches

### 6. Advanced Testing

**Check UHCI Register Programming:**
- Look for messages about frame list setup
- Transfer descriptor linking
- USB command register writes

**Verify Transfer Completion:**
- Watch for "USB: Transfer completed successfully" vs timeouts
- Check for USB error status messages

## Kernel Logging System

ICS-OS now includes a comprehensive logging system that greatly improves debugging capabilities:

### Log Levels Available:
- `EMERG`, `ALERT`, `CRIT` - Critical system issues
- `ERR` - Recoverable errors  
- `WARN` - Warnings and potential issues
- `INFO` - General informational messages
- `DEBUG` - Detailed debugging information

### Adjusting Log Levels:
You can change the log level at runtime to see more or fewer messages. The USB subsystem uses all log levels appropriately.

### Serial Port Benefits for USB Debugging:
1. **Persistent Logs**: Messages are saved to file and don't scroll off screen
2. **Real-time Monitoring**: Use `tail -f` to watch logs as they happen
3. **No Screen Clutter**: Console remains clean for user interaction
4. **Detailed Debug Info**: Debug level logs show internal USB operations
5. **Timestamped Messages**: See exact timing of USB operations

## Troubleshooting

### If you see VID=0x1234, PID=0x5678:
- This indicates simulation mode is active
- Real hardware detection may have failed
- QEMU virtual USB device might not be properly attached

### If USB mount fails:
- Check if device was detected: `usb devices`
- Verify device manager registration: `devmgr`
- Try different mount points or filesystem types

### Debug Mode:
The current implementation has extensive debugging output. Watch the boot messages carefully to see which code path is being taken.

## Expected QEMU Virtual Device Behavior

With the command:
```bash
qemu-system-i386 -device piix3-usb-uhci -device usb-storage,drive=usb-drive -drive id=usb-drive,file=usb-thumb.img
```

You should see:
1. UHCI controller detected at base address
2. USB device detected on one of the ports
3. Real USB enumeration process
4. Actual device descriptor reading
5. Mass storage device initialization

## Key Questions to Answer

1. **Are real USB transfers happening?**
   - Look for UHCI transfer descriptor messages
   - Check for real port status values (not just simulation)

2. **Is actual QEMU device data being read?**
   - Compare mounted data with usb-thumb.img contents
   - Verify it's not just the hardcoded simulation data

3. **Is the UHCI controller properly programmed?**
   - Watch for frame list base address setup
   - Transfer completion detection

This will help determine if the real USB implementation is working or if it's falling back to simulation mode.
