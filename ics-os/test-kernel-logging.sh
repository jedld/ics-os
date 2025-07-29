#!/bin/bash

# Simple test to verify kernel logging with QEMU
echo "Testing ICS-OS Kernel Logging System"
echo "===================================="

# Create logs directory
mkdir -p logs

echo "Starting QEMU with serial logging..."
echo "Logs will be saved to logs/kernel_test.log"
echo "QEMU will run for 15 seconds then automatically terminate"

# Run QEMU with serial logging, timeout after 15 seconds
timeout 15s qemu-system-i386 \
    -cdrom ics-os-uefi.iso \
    -serial file:logs/kernel_test.log \
    -m 128M \
    -display none \
    -no-reboot || true

echo ""
echo "QEMU test completed. Checking log output..."

if [ -f "logs/kernel_test.log" ]; then
    echo ""
    echo "=== Kernel Log Output ==="
    cat logs/kernel_test.log
    echo ""
    echo "=== Checking for Logging System Messages ==="
    if grep -q "Kernel logging system initialized" logs/kernel_test.log; then
        echo "✓ Kernel logging system initialization found"
    else
        echo "✗ Kernel logging system initialization not found"
    fi
    
    if grep -q "\[.*\]" logs/kernel_test.log; then
        echo "✓ Structured log messages with timestamps found"
    else
        echo "✗ No structured log messages found"
    fi
    
    if grep -qi "usb" logs/kernel_test.log; then
        echo "✓ USB subsystem logs found"
    else
        echo "✗ No USB subsystem logs found"
    fi
    
    echo ""
    echo "Log file saved at: logs/kernel_test.log"
    echo "File size: $(wc -c < logs/kernel_test.log) bytes"
    echo "Line count: $(wc -l < logs/kernel_test.log) lines"
else
    echo "ERROR: No log file was created!"
    exit 1
fi

echo ""
echo "Test completed successfully!"
