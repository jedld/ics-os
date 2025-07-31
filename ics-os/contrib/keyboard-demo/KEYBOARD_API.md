# Enhanced Keyboard API for ICS-OS

## Overview

The enhanced keyboard API provides high-level, easy-to-use functions for handling keyboard input in ICS-OS applications. This API abstracts away the complexity of dealing with raw keyboard codes and meta bits, making it much easier to develop keyboard-intensive applications like text editors, games, and interactive programs.

## Key Features

- **Control Key Detection**: Easy detection of Ctrl+key combinations (handles both legacy and modern formats)
- **Special Key Handling**: Simple handling of function keys, arrow keys, etc.
- **Meta Key Support**: Support for Alt, Shift, and Ctrl modifiers
- **Key Event Structure**: Structured information about each key press
- **Legacy Compatibility**: Automatically detects and converts legacy control codes (ICS-OS uses 0-based: Ctrl+A = 0, Ctrl+Q = 16, etc.)
- **Backward Compatibility**: Works alongside existing `getch()` and `getchar()` functions

## API Functions

### Core Functions

#### `int get_key_event(key_event_t *event)`
Fills a key event structure with detailed information about the pressed key.

**Parameters:**
- `event`: Pointer to a `key_event_t` structure to fill

**Returns:**
- `0` on success
- `-1` on error

**Note:** This function automatically handles both modern meta bit format and legacy control codes. ICS-OS uses 0-based legacy control codes (0-25 for Ctrl+A through Ctrl+Z) which are automatically converted to the modern format with appropriate `ctrl` flag and `ascii` character set.

#### `int getch_enhanced()`
Enhanced version of `getch()` that returns the full key code including meta bits.

**Returns:**
- Full key code with meta bits

### Utility Functions

#### `int is_ctrl_key(int key)`
Checks if a key is a control key combination.

#### `int is_special_key(int key)`
Checks if a key is a special key (function keys, arrows, etc.).

#### `char get_ascii_from_key(int key)`
Extracts the ASCII character from a key code.

## Data Structures

### `key_event_t`
```c
typedef struct {
    int code;           /* Raw key code */
    char ascii;         /* ASCII character (if printable) */
    int ctrl;           /* 1 if Ctrl pressed */
    int alt;            /* 1 if Alt pressed */
    int shift;          /* 1 if Shift pressed */
    int is_special;     /* 1 if special key (arrows, function keys, etc.) */
    int is_printable;   /* 1 if printable ASCII character */
} key_event_t;
```

## Constants

### Control Key Combinations
```c
#define CTRL_A  (KBD_META_CTRL | 'a')
#define CTRL_B  (KBD_META_CTRL | 'b')
// ... (all letters A-Z available)
```

### Special Keys
```c
#define KEY_F1      0x80    // Function keys F1-F12
#define KEY_UP      0x97    // Arrow keys
#define KEY_DOWN    0x98
#define KEY_LEFT    0x96
#define KEY_RIGHT   0x99
#define KEY_HOME    0x92    // Navigation keys
#define KEY_END     0x93
#define KEY_PGUP    0x94
#define KEY_PGDN    0x95
#define KEY_INS     0x90    // Edit keys
#define KEY_DEL     0x91
```

### Meta Bits
```c
#define KBD_META_ALT    0x0200  /* Alt is pressed */
#define KBD_META_CTRL   0x0400  /* Ctrl is pressed */
#define KBD_META_SHIFT  0x0800  /* Shift is pressed */
```

## Example Usage

### Simple Key Handling
```c
#include "../../sdk/dexsdk.h"

int main() {
    key_event_t event;
    
    printf("Press keys (Ctrl+Q to quit):\\n");
    
    while (1) {
        if (get_key_event(&event) == 0) {
            // Handle control keys
            if (event.ctrl) {
                if (event.ascii == 'q' || event.ascii == 'Q') {
                    break; // Ctrl+Q to quit
                }
                printf("Ctrl+%c pressed\\n", event.ascii);
            }
            // Handle printable characters
            else if (event.is_printable) {
                printf("Character: %c\\n", event.ascii);
            }
            // Handle special keys
            else if (event.is_special) {
                switch (event.ascii) {
                    case KEY_UP: printf("Up arrow\\n"); break;
                    case KEY_F1: printf("F1 key\\n"); break;
                    // ... handle other special keys
                }
            }
        }
    }
    
    return 0;
}
```

### Text Editor Pattern
```c
void handle_input(key_event_t *event) {
    // Handle control key shortcuts
    if (event->ctrl) {
        switch (event->ascii) {
            case 's': save_file(); break;
            case 'o': open_file(); break;
            case 'q': quit_editor(); break;
        }
        return;
    }
    
    // Handle special keys
    if (event->is_special) {
        switch (event->ascii) {
            case KEY_UP: move_cursor_up(); break;
            case KEY_DOWN: move_cursor_down(); break;
            case KEY_LEFT: move_cursor_left(); break;
            case KEY_RIGHT: move_cursor_right(); break;
            case KEY_F1: show_help(); break;
        }
        return;
    }
    
    // Handle regular character input
    if (event->is_printable) {
        insert_character(event->ascii);
    }
}
```

## Migration from Old API

### Before (Raw keyboard handling)
```c
int key = getch();
if (key == 19) { // Ctrl+S
    save_file();
}
```

### After (Enhanced API)
```c
key_event_t event;
get_key_event(&event);
if (event.ctrl && (event.ascii == 's' || event.ascii == 'S')) {
    save_file();
}
```

## Benefits

1. **Readability**: Code is much more readable and self-documenting
2. **Maintainability**: Easier to modify and extend keyboard handling
3. **Portability**: Abstracts platform-specific keyboard details
4. **Consistency**: Provides consistent behavior across applications
5. **Development Speed**: Faster development of keyboard-intensive applications

## Applications Using Enhanced API

- **DexEdit**: Enhanced text editor with nano-like functionality
- **Keyboard Demo**: Demonstration program showing all API features

## Future Enhancements

- Support for key repeat detection
- Keyboard macro recording/playback
- Custom key mapping support
- International keyboard layout support
