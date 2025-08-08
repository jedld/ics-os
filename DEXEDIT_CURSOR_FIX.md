# DexEdit Cursor Positioning Fix

## Issue Description
The dexedit text editor had a cursor positioning problem where the cursor would appear at the bottom of the screen instead of at the actual text editing position where the user was typing.

## Root Cause
The editor was calling `setx()` and `sety()` to set the logical cursor position, but was missing the critical `update_cursor()` call needed to actually move the hardware cursor to the correct screen position.

## Solution
Added the missing `update_cursor(display_row, display_col)` call in the `editor_refresh_screen()` function after setting the logical cursor position.

### Code Change
**File:** `ics-os/contrib/dexedit/dexedit.c`

**Before:**
```c
if (display_row >= 0 && display_row < EDITOR_HEIGHT && 
    display_col >= line_num_width && display_col < SCREEN_WIDTH) {
    setx(display_col); sety(display_row);
}
```

**After:**
```c
if (display_row >= 0 && display_row < EDITOR_HEIGHT && 
    display_col >= line_num_width && display_col < SCREEN_WIDTH) {
    setx(display_col); sety(display_row);
    update_cursor(display_row, display_col);  // y=row, x=col
}
```

## Technical Details

### Function Signature
```c
void update_cursor(int y, int x);  // First parameter is row (y), second is column (x)
```

### Cursor Position Calculation
- `display_row = editor.current_line - editor.top_line` (relative to visible area)
- `display_col = line_num_width + editor.current_col - editor.left_col` (accounts for line numbers and horizontal scrolling)

### ICS-OS Cursor System
The ICS-OS display system has two levels of cursor positioning:

1. **Logical Position**: Set by `setx()` and `sety()` - tracks where text output will appear
2. **Hardware Cursor**: Set by `update_cursor()` - controls the visible cursor on screen

Both must be synchronized for proper cursor display.

## Testing
After the fix:
- ✅ Cursor appears at the actual text editing position
- ✅ Cursor follows arrow key movements correctly
- ✅ Cursor stays positioned during text insertion/deletion
- ✅ Cursor positioning works with line numbers enabled/disabled
- ✅ Cursor positioning works with horizontal/vertical scrolling

## Benefits
- **Improved User Experience**: Users can see exactly where they are typing
- **Better Text Editing**: Visual feedback matches actual editing position
- **Reduced Confusion**: No more guessing where text will be inserted
- **Professional Feel**: Editor now behaves like modern text editors

## Related Functions
- `setx(int x)`: Set logical X position
- `sety(int y)`: Set logical Y position  
- `update_cursor(int y, int x)`: Update hardware cursor position
- `editor_refresh_screen()`: Main screen drawing function
- `editor_move_cursor()`: Logical cursor movement function
