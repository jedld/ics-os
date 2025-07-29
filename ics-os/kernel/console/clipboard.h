/*
  Name: clipboard.h
  Description: Simple clipboard functionality for ICS-OS console
  Author: GitHub Copilot
  Date: 2025
*/

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#define CLIPBOARD_MAX_SIZE 1024

// Clipboard operations
void clipboard_init(void);
int clipboard_copy(const char *text, int length);
int clipboard_paste(char *buffer, int max_length);
void clipboard_clear(void);
int clipboard_get_length(void);
int clipboard_is_empty(void);

// Console integration functions
void console_copy_selection(void);
void console_paste_clipboard(void);
void console_select_all(void);

#endif
