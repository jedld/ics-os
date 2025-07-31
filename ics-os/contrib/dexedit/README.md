# DexEdit - Enhanced Text Editor for ICS-OS

## Overview

DexEdit is a modern, user-friendly text editor for ICS-OS that addresses the limitations of the original simple editor. It provides a nano-like interface while remaining lightweight and simple to use.

## Features

### ✅ Improvements over Original Editor

1. **Better User Interface**
   - Status line showing file name, line/column position, and edit mode
   - Help line at bottom with key shortcuts
   - Visual feedback for all operations
   - Color coding for different UI elements

2. **Enhanced Navigation**
   - Arrow keys for cursor movement
   - Home/End for line navigation
   - Page Up/Down for screen navigation
   - Ctrl+G for "Go to line" functionality

3. **Improved Editing**
   - Insert/Overwrite mode toggle (Insert key)
   - Better backspace and delete handling
   - Proper line wrapping and scrolling
   - Tab insertion (4 spaces)

4. **File Operations**
   - Ctrl+N: New file
   - Ctrl+O: Open file (with confirmation if modified)
   - Ctrl+S: Save file
   - Ctrl+A: Save as (new filename)
   - Ctrl+Q: Quit (with save confirmation if modified)

5. **Search and Navigation**
   - Ctrl+F: Find text (with wrap-around search)
   - Ctrl+G: Go to specific line number

6. **Copy/Paste Support**
   - Ctrl+C: Copy current line
   - Ctrl+V: Paste copied line
   - Built-in clipboard functionality

7. **View Options**
   - Ctrl+L: Toggle line numbers on/off
   - Automatic horizontal and vertical scrolling

8. **Help System**
   - F1: Show comprehensive help screen
   - Always-visible help line at bottom

### 🔧 Technical Improvements

1. **Memory Management**
   - Efficient line-based storage
   - Maximum 1000 lines, 200 characters per line
   - Proper bounds checking

2. **Screen Management**
   - Double-buffered screen updates
   - Proper cursor positioning
   - Smooth scrolling

3. **Error Handling**
   - User-friendly error messages
   - Confirmation dialogs for destructive operations
   - Graceful handling of edge cases

## Usage

### Starting the Editor

```bash
# Start with new file
dexedit.exe

# Open existing file
dexedit.exe filename.txt
```

### Key Bindings

#### File Operations
- **Ctrl+N**: Create new file
- **Ctrl+O**: Open file
- **Ctrl+S**: Save current file
- **Ctrl+A**: Save as (new filename)
- **Ctrl+Q**: Quit editor

#### Navigation
- **Arrow Keys**: Move cursor
- **Home**: Beginning of line
- **End**: End of line
- **Page Up/Down**: Move by screen
- **Ctrl+G**: Go to line number

#### Editing
- **Insert**: Toggle insert/overwrite mode
- **Backspace**: Delete character before cursor
- **Delete**: Delete character at cursor
- **Enter**: Insert new line
- **Tab**: Insert 4 spaces

#### Search
- **Ctrl+F**: Find text (searches from cursor with wrap-around)

#### Line Operations
- **Ctrl+K**: Delete entire line
- **Ctrl+C**: Copy line to clipboard
- **Ctrl+V**: Paste line from clipboard

#### View
- **Ctrl+L**: Toggle line numbers
- **F1**: Show help screen

### Status Indicators

- **\***: File has been modified (shown in status line)
- **INS/OVR**: Current editing mode (Insert/Overwrite)
- **Ln X,Col Y**: Current cursor position

## Building

To build the enhanced editor:

```bash
cd contrib/dexedit
make
make install
```

This will compile the editor and copy it to the apps directory.

## Comparison with Original Editor

| Feature | Original ed.exe | DexEdit |
|---------|----------------|---------|
| Key bindings | Ctrl+O,Enter to save<br>Ctrl+X to exit | Intuitive Ctrl+S, Ctrl+Q, etc. |
| Navigation | Limited | Full arrow key support, page up/down |
| Visual feedback | Minimal | Status line, help line, messages |
| Copy/paste | None | Line-based copy/paste |
| Search | None | Ctrl+F with wrap-around |
| Line numbers | None | Toggleable with Ctrl+L |
| File operations | Basic | New, open, save, save-as with confirmations |
| Error handling | Poor | User-friendly messages and confirmations |
| Help system | None | F1 for comprehensive help |
| Editing modes | Insert only | Insert/overwrite toggle |

## Bug Fixes

The enhanced editor addresses several issues from user feedback:

1. **Memory Safety**: Proper bounds checking prevents crashes
2. **File Handling**: Better error messages for file operations
3. **Cursor Management**: Smooth cursor movement without screen corruption
4. **Line Editing**: Proper handling of long lines and edge cases
5. **Exit Handling**: Confirmation dialogs prevent accidental data loss

## Future Enhancements

Potential future improvements could include:
- Syntax highlighting for common file types
- Block selection and editing
- Find and replace functionality
- Undo/redo capability
- Multiple file support (tabs)
- Configuration file support

## Development Notes

The editor is written in C using the ICS-OS SDK and follows these design principles:

1. **Simplicity**: Easy to use for beginners
2. **Reliability**: Robust error handling and memory management  
3. **Performance**: Efficient screen updates and memory usage
4. **Compatibility**: Works within ICS-OS constraints and limitations

The codebase is well-commented and modular, making it easy to extend and maintain.
