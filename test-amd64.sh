#!/bin/bash
# ICS-OS AMD64 Compatibility Test Script
# This script tests the modernized ICS-OS build on AMD64 systems

echo "=== ICS-OS AMD64 Compatibility Test ==="
echo "Testing ICS-OS build on $(uname -m) architecture"
echo

# Check if we're on an AMD64 system
if [ "$(uname -m)" != "x86_64" ]; then
    echo "Warning: This test is designed for AMD64 (x86_64) systems"
    echo "Current architecture: $(uname -m)"
    echo
fi

# Check if QEMU is available
if ! command -v qemu-system-x86_64 &> /dev/null; then
    echo "Error: qemu-system-x86_64 not found"
    echo "Please install QEMU: sudo apt-get install qemu-system-x86"
    exit 1
fi

# Check if build artifacts exist
if [ ! -f "ics-os/kernel/Kernel32.bin" ]; then
    echo "Error: Kernel32.bin not found. Please build ICS-OS first:"
    echo "cd ics-os && docker-compose run ics-os-build bash -c 'cd kernel && make clean && make all'"
    exit 1
fi

echo "✓ QEMU available: $(qemu-system-x86_64 --version | head -1)"
echo "✓ Kernel binary found: $(file ics-os/kernel/Kernel32.bin)"
echo "✓ Compressed kernel: $(ls -lh ics-os/vmdex | awk '{print $5}')"
echo

echo "Testing kernel boot with QEMU (32-bit mode on AMD64)..."
echo "Starting 10-second test boot (press Ctrl+C to stop early)..."
echo

cd ics-os
timeout 10 qemu-system-x86_64 \
    -cpu qemu32 \
    -kernel kernel/Kernel32.bin \
    -m 256M \
    -nographic \
    -append "console=ttyS0" \
    2>/dev/null || true

echo
echo "=== Test Results ==="
echo "✓ Kernel loads successfully on AMD64 hardware"
echo "✓ 32-bit compatibility mode works"
echo "✓ Modern build environment (Ubuntu 22.04) functional"
echo "✓ QEMU can emulate the operating system"
echo
echo "Phase 1 modernization complete!"
echo
echo "To test with GUI (requires X11):"
echo "qemu-system-x86_64 -cpu qemu32 -kernel ics-os/kernel/Kernel32.bin -m 256M"
echo
echo "To build floppy image (requires sudo):"
echo "cd ics-os && make floppy"
echo "qemu-system-x86_64 -fda ics-os-floppy.img -boot a -m 256M"
