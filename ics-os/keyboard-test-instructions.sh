#!/bin/bash
# Test script to diagnose keyboard issues in ICS-OS

echo "Testing keyboard functionality in ICS-OS"
echo "========================================="

# Test 1: Simple keyboard test
echo "1. Running simple keyboard test..."
echo "   This will show raw key codes from getch()"
echo "   Instructions will be shown in the emulator"
echo ""

echo "Starting ICS-OS with floppy image..."
echo "When the system boots:"
echo "1. Run: simple-keyboard-test.exe"
echo "2. Test these keys:"
echo "   - Type some letters (a, b, c, etc.)"
echo "   - Press Enter key"
echo "   - Press Backspace key"
echo "   - Press q to quit the test"
echo "3. Then run: dexedit.exe"
echo "4. Test typing and backspace in dexedit"
echo ""
echo "Press Ctrl+C to stop the emulator when done"
echo ""

# Note: This script is for documentation purposes
# The actual testing is done interactively in the ICS-OS environment
