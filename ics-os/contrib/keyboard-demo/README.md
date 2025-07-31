# Keyboard Demo

This program demonstrates the enhanced keyboard API for ICS-OS. It shows how to use the new high-level keyboard functions to handle control keys, special keys, and regular character input.

## Features Demonstrated

- Control key detection (Ctrl+A, Ctrl+S, etc.)
- Special key handling (F1-F12, arrow keys, etc.)
- Meta key detection (Alt, Shift, Ctrl)
- Key event structure usage
- Printable character detection

## Building

```bash
make clean && make
make install
```

## Running

After building and installing, run the program:

```
keyboard-demo.exe
```

## Usage

- Press any key to see detailed information about the key event
- Try control key combinations like Ctrl+S, Ctrl+O, Ctrl+N
- Test function keys F1-F12
- Test arrow keys and navigation keys
- Press Ctrl+Q to quit

## Example Output

```
Enhanced Keyboard API Demo
==========================

Press any key to see detailed information.
Press Ctrl+Q to quit.

Key pressed: Ctrl+S (Raw code: 0x0473)
  -> This would save a file!

Key pressed: F1 (Raw code: 0x0080)

Key pressed: Up Arrow (Raw code: 0x0097)

Key pressed: 'a' (Raw code: 0x0061)
```

This demo is a practical example of how much easier keyboard handling becomes with the enhanced API compared to manually parsing raw key codes and meta bits.
