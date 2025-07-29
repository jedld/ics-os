#!/bin/bash

# ICS-OS Kernel Logging Test Script
# This script builds and tests the kernel logging system with QEMU

set -e

echo "=================================================="
echo "ICS-OS Kernel Logging System Test"
echo "=================================================="

# Change to the ics-os kernel directory
cd ics-os/kernel

echo "Building kernel with logging support..."
make clean
make

if [ $? -ne 0 ]; then
    echo "ERROR: Kernel build failed!"
    exit 1
fi

echo "Kernel built successfully!"

# Create test log directory
mkdir -p ../../logs

echo "Starting QEMU with USB devices and serial logging..."
echo "USB devices: UHCI controller + USB mass storage"
echo "Logs will be saved to ../../logs/kernel.log"
echo "Press Ctrl+C to stop QEMU"

# Run QEMU with serial logging to file and USB device
# Note: Using floppy image and adding USB devices for testing
qemu-system-i386 \
    -fda ../../ics-os/ics-os-floppy.img \
    -device piix3-usb-uhci \
    -device usb-storage,drive=usb-drive \
    -drive id=usb-drive,file=../../extras/usb-thumb.img,format=raw \
    -serial file:logs/kernel.log \
    -m 128M \
    -enable-kvm 2>/dev/null || qemu-system-i386 \
    -fda ../../ics-os/ics-os-floppy.img \
    -device piix3-usb-uhci \
    -device usb-storage,drive=usb-drive \
    -drive id=usb-drive,file=../../extras/usb-thumb.img,format=raw \
    -serial file:logs/kernel.log \
    -m 128M &

QEMU_PID=$!

# Wait a moment for QEMU to start
sleep 3

echo ""
echo "QEMU started with PID: $QEMU_PID"
echo "Monitor the log file with: tail -f ../../logs/kernel.log"
echo ""
echo "Waiting 15 seconds for USB initialization and boot messages..."
sleep 15

if [ -f "../../logs/kernel.log" ]; then
    echo "=== Initial Kernel Log Messages ==="
    head -20 ../../logs/kernel.log
    echo ""
    echo "=== USB-related Log Messages ==="
    grep -i usb ../../logs/kernel.log || echo "No USB messages found yet"
    echo ""
    echo "=== Full log available at: logs/kernel.log ==="
else
    echo "WARNING: Log file not created yet"
fi

# Keep QEMU running for user interaction
echo "QEMU is running. Press Enter to stop it..."
read -r

echo "Stopping QEMU..."
kill $QEMU_PID 2>/dev/null || true
wait $QEMU_PID 2>/dev/null || true

echo "Test completed. Check logs/kernel.log for detailed output."
