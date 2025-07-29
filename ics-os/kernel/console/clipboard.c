/*
  Name: clipboard.c
  Description: Simple clipboard functionality for ICS-OS console
  Author: GitHub Copilot
  Date: 2025
*/

#include "clipboard.h"
#include "../stdlib/dexstdlib.h"
#include "console.h"

// Clipboard buffer
static char clipboard_buffer[CLIPBOARD_MAX_SIZE];
static int clipboard_length = 0;
static int clipboard_initialized = 0;

// Initialize clipboard system
void clipboard_init(void) {
    memset(clipboard_buffer, 0, CLIPBOARD_MAX_SIZE);
    clipboard_length = 0;
    clipboard_initialized = 1;
}

// Copy text to clipboard
int clipboard_copy(const char *text, int length) {
    if (!clipboard_initialized) clipboard_init();
    
    if (!text || length <= 0) return -1;
    
    // Limit to buffer size
    if (length >= CLIPBOARD_MAX_SIZE) {
        length = CLIPBOARD_MAX_SIZE - 1;
    }
    
    memcpy(clipboard_buffer, text, length);
    clipboard_buffer[length] = '\0';
    clipboard_length = length;
    
    printf("Copied %d characters to clipboard\n", length);
    return length;
}

// Paste text from clipboard
int clipboard_paste(char *buffer, int max_length) {
    if (!clipboard_initialized) clipboard_init();
    
    if (!buffer || max_length <= 0 || clipboard_length == 0) return 0;
    
    int copy_length = clipboard_length;
    if (copy_length >= max_length) {
        copy_length = max_length - 1;
    }
    
    memcpy(buffer, clipboard_buffer, copy_length);
    buffer[copy_length] = '\0';
    
    return copy_length;
}

// Clear clipboard
void clipboard_clear(void) {
    if (!clipboard_initialized) clipboard_init();
    
    memset(clipboard_buffer, 0, CLIPBOARD_MAX_SIZE);
    clipboard_length = 0;
    printf("Clipboard cleared\n");
}

// Get clipboard length
int clipboard_get_length(void) {
    if (!clipboard_initialized) clipboard_init();
    return clipboard_length;
}

// Check if clipboard is empty
int clipboard_is_empty(void) {
    if (!clipboard_initialized) clipboard_init();
    return clipboard_length == 0;
}

// Console integration - copy text to clipboard (simple version)
void console_copy_selection(void) {
    printf("Use: copy <text> - to copy text to clipboard\n");
    printf("Or use clipboard commands to manage clipboard content\n");
}

// Console integration - paste from clipboard (enhanced)
void console_paste_clipboard(void) {
    if (clipboard_is_empty()) {
        printf("Clipboard is empty\n");
        return;
    }
    
    char paste_buffer[256];
    int pasted_length = clipboard_paste(paste_buffer, sizeof(paste_buffer));
    
    if (pasted_length > 0) {
        printf("Clipboard contents: %s\n", paste_buffer);
        printf("Tip: You can copy this text using your terminal's copy function\n");
    }
}

// Console integration - select all text (simple version)  
void console_select_all(void) {
    printf("Use your terminal's select-all function (usually Ctrl+A)\n");
    printf("Then copy with Ctrl+C and paste in ICS-OS using 'paste' command\n");
}
