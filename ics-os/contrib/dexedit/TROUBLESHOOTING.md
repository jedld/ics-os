# DexEdit Troubleshooting Guide

## Common Issues and Solutions

### Build Issues

**Problem**: `undefined reference to 'fprintf'` or similar function errors
**Solution**: The editor has been updated to use only ICS-OS SDK functions. Ensure you're using the latest version.

**Problem**: `identifier "KEY_UP" is undefined`
**Solution**: Key constants are now defined directly in the source code to avoid header dependency issues.

### Runtime Issues

**Problem**: Editor crashes on startup
**Solution**: 
- Ensure sufficient memory is available
- Check that the file system is properly mounted
- Verify the executable is not corrupted

**Problem**: Cannot save files
**Solution**:
- Check file permissions
- Ensure the target directory exists and is writable
- Verify the file system is mounted read-write

**Problem**: Keyboard input not working properly
**Solution**:
- Check if another process is capturing keyboard input
- Ensure the editor has focus in the foreground manager
- Try using the basic movement keys (arrow keys)

### Editor Behavior

**Problem**: Text appears garbled or screen is corrupted
**Solution**:
- Press Ctrl+L twice to refresh the screen
- Check that the terminal supports the expected character set
- Restart the editor

**Problem**: Cannot find text with search function
**Solution**:
- Search is case-sensitive
- Ensure the text exists in the file
- Try searching from the beginning of the file

**Problem**: Line numbers not showing
**Solution**:
- Press Ctrl+L to toggle line numbers on/off
- Check that the terminal width is sufficient

## Performance Considerations

### Memory Usage
- Maximum 1000 lines per file
- Maximum 200 characters per line
- Total memory usage approximately 200KB for text buffer

### File Size Limits
- Recommended maximum file size: 50KB
- Larger files may cause performance issues
- Consider using external tools for very large files

## Bug Reporting

If you encounter bugs not covered here:

1. **Document the issue**:
   - What were you doing when the problem occurred?
   - What did you expect to happen?
   - What actually happened?

2. **Gather information**:
   - ICS-OS version
   - File being edited (if applicable)
   - Sequence of key presses that triggered the issue

3. **Try to reproduce**:
   - Can you make the problem happen again?
   - Does it happen with different files?
   - Does it happen immediately or after extended use?

## Known Limitations

### Current Version Limitations
- No undo/redo functionality
- No syntax highlighting
- No block selection
- No find-and-replace
- Single file editing only
- No configuration file support

### ICS-OS Specific Limitations
- Limited to text mode display (80x25)
- No mouse support
- Keyboard handling depends on ICS-OS key mapping
- File operations limited by ICS-OS VFS capabilities

## Debug Mode

For debugging issues, you can add debug prints to the source code:

```c
// Add this function for debugging
void debug_print(const char* msg) {
    setx(0); sety(24);
    textcolor(15);
    printf("DEBUG: %s", msg);
    getch(); // Wait for key press
}

// Use it in problem areas
debug_print("Before file save");
```

## Recovery Procedures

### If editor hangs:
1. Try Ctrl+Q to quit
2. If unresponsive, use Ctrl+Alt+F4 to terminate
3. Check console for error messages

### If file is corrupted:
1. Exit editor without saving
2. Check if backup exists (editor.filename.bak)
3. Use hex dump utility to examine file contents

### If display is corrupted:
1. Press Ctrl+L to refresh
2. Restart the editor
3. Check terminal settings

## Testing New Features

When modifying the editor:

1. **Test basic functionality**:
   - Creating new files
   - Opening existing files
   - Saving files
   - Basic editing operations

2. **Test edge cases**:
   - Very long lines
   - Maximum number of lines
   - Empty files
   - Read-only files

3. **Test error conditions**:
   - Invalid file names
   - Insufficient disk space
   - File permission errors

## Performance Optimization Tips

### For better performance:
- Keep files under 500 lines when possible
- Avoid very long lines (>100 characters)
- Close and reopen for very large edits
- Use line numbers sparingly for large files

### Memory management:
- The editor pre-allocates memory for maximum file size
- Memory is not freed until editor exits
- Consider editor restart for memory-intensive editing sessions
