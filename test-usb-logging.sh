#!/bin/bash

# Test USB Logging in ICS-OS
# This script tests the new USB logging functionality

echo "Testing USB Logging in ICS-OS"
echo "=============================="

cd ics-os

echo "Building kernel with USB logging..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Kernel built successfully with USB logging"
else
    echo "✗ Kernel build failed"
    exit 1
fi

echo ""
echo "Starting QEMU with USB device and logging capture..."
echo "The USB driver will now log to both console and internal buffer"
echo ""
echo "Expected USB Log Output:"
echo "- USB initialization messages"
echo "- USB controller detection"
echo "- USB device enumeration"
echo "- Mass storage device creation"
echo "- USB log dump at the end"
echo ""
echo "Press Ctrl+C to exit QEMU when testing is complete."
echo ""

# Run QEMU with USB drive and capture output
qemu-system-i386 -m 128 \
    -drive if=floppy,format=raw,file=ics-os-floppy.img \
    -drive file=usb-thumb.img,if=none,id=usb-drive \
    -device usb-storage,drive=usb-drive \
    -boot a \
    -serial stdio 2>&1 | tee usb_test_output.log

echo ""
echo "Test completed. USB output captured in usb_test_output.log"
echo "Check the log for USB initialization and device detection messages."
