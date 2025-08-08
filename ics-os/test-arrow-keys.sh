#!/bin/bash

# Arrow Key Debugging Test Script
echo "=== ICS-OS Arrow Key Debugging ==="
echo

echo "This script will help debug arrow key issues in ICS-OS"
echo "Expected arrow key codes according to KEYBOARD_API.md:"
echo "  LEFT:  0x96 (150)"
echo "  UP:    0x97 (151)" 
echo "  DOWN:  0x98 (152)"
echo "  RIGHT: 0x99 (153)"
echo

echo "Testing programs available:"
echo "  arrow-test.exe   - Shows raw key codes for arrow keys"
echo "  raw-test.exe     - Shows all raw key codes"
echo "  keyboard-debug.exe - Shows enhanced keyboard API events"
echo "  simple-editor.exe  - Basic editor (backspace works, arrows not implemented)"
echo

echo "Test Sequence:"
echo "1. First test: arrow-test.exe to see raw arrow key codes"
echo "2. If arrow keys produce wrong codes, the issue is in keyboard driver"
echo "3. If arrow keys produce correct codes, the issue is in dexedit key handling"
echo

echo "Ready to start testing? (y/n)"
read -r response

if [ "$response" = "y" ] || [ "$response" = "Y" ]; then
    echo
    echo "=== Step 1: Raw Arrow Key Test ==="
    echo "Running arrow-test.exe..."
    echo "Instructions: Press each arrow key and note the codes displayed"
    echo "Press ESC when done"
    echo
    
    if [ -f "apps/arrow-test.exe" ]; then
        ./apps/arrow-test.exe
    else
        echo "ERROR: arrow-test.exe not found in apps/"
        exit 1
    fi
    
    echo
    echo "=== Step 2: Analysis ==="
    echo "Did the arrow keys produce the expected codes?"
    echo "  LEFT should show: 150 (0x96)"
    echo "  UP should show:   151 (0x97)"  
    echo "  DOWN should show: 152 (0x98)"
    echo "  RIGHT should show: 153 (0x99)"
    echo
    echo "If the codes are different, note them down and check the keyboard driver."
    echo "If the codes are correct, the issue is in dexedit's key definition matching."
    echo
    
    echo "Would you like to run the enhanced keyboard API test? (y/n)"
    read -r response2
    
    if [ "$response2" = "y" ] || [ "$response2" = "Y" ]; then
        echo
        echo "=== Step 3: Enhanced Keyboard API Test ==="
        echo "Running keyboard-debug.exe..."
        echo "This will show if the enhanced API correctly interprets the arrow keys"
        echo
        
        if [ -f "apps/keyboard-debug.exe" ]; then
            ./apps/keyboard-debug.exe
        else
            echo "ERROR: keyboard-debug.exe not found in apps/"
        fi
    fi
    
    echo
    echo "=== Summary ==="
    echo "Based on the test results:"
    echo "- If raw codes are wrong: Fix keyboard driver or key mapping"
    echo "- If raw codes are right but enhanced API fails: Fix enhanced API"
    echo "- If both work but dexedit fails: Fix dexedit key definitions"
    echo
    echo "The dexedit key definitions are in:"
    echo "  contrib/dexedit/dexedit.c (lines with #define KEY_LEFT, etc.)"
    echo
else
    echo "Test cancelled."
fi
