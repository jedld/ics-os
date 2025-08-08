/*
  Simplified DexEdit with Raw Keyboard Input
  This version bypasses the enhanced keyboard API to test basic functionality
*/

#include "../../sdk/dexsdk.h"

// Simplified editor structure
typedef struct {
    char lines[100][80];  // Simple fixed-size lines
    int num_lines;
    int current_line;
    int current_col;
    char filename[256];
} simple_editor_t;

simple_editor_t editor;

void simple_editor_init() {
    editor.num_lines = 1;
    editor.current_line = 0;
    editor.current_col = 0;
    strcpy(editor.lines[0], "");
    strcpy(editor.filename, "untitled.txt");
}

void simple_editor_draw() {
    int i;
    clrscr();
    
    // Draw lines
    for (i = 0; i < 20 && i < editor.num_lines; i++) {
        setx(0); sety(i);
        printf("%s", editor.lines[i]);
    }
    
    // Draw status
    setx(0); sety(22);
    textcolor(7);
    printf("File: %s  Line: %d Col: %d  Raw Keyboard Test", 
           editor.filename, editor.current_line + 1, editor.current_col + 1);
    
    setx(0); sety(23);
    printf("ESC=Exit, Enter=New Line, Backspace=Delete, Arrows=Move");
    
    // Position cursor
    setx(editor.current_col); sety(editor.current_line);
    update_cursor(editor.current_line, editor.current_col);
}

void simple_editor_handle_char(char c) {
    char* line = editor.lines[editor.current_line];
    int line_len = strlen(line);
    int i;
    
    if (c >= 32 && c <= 126 && line_len < 79) {  // Printable character
        // Shift characters right
        for (i = line_len; i > editor.current_col; i--) {
            line[i] = line[i - 1];
        }
        line[editor.current_col] = c;
        line[line_len + 1] = '\0';
        editor.current_col++;
    }
}

void simple_editor_backspace() {
    char* line = editor.lines[editor.current_line];
    int line_len = strlen(line);
    int i;
    
    if (editor.current_col > 0) {
        editor.current_col--;
        // Shift characters left
        for (i = editor.current_col; i < line_len; i++) {
            line[i] = line[i + 1];
        }
    }
}

void simple_editor_new_line() {
    if (editor.num_lines < 99) {
        int i;
        // Shift lines down
        for (i = editor.num_lines; i > editor.current_line + 1; i--) {
            strcpy(editor.lines[i], editor.lines[i - 1]);
        }
        
        // Split current line
        char* current_line = editor.lines[editor.current_line];
        strcpy(editor.lines[editor.current_line + 1], &current_line[editor.current_col]);
        current_line[editor.current_col] = '\0';
        
        editor.num_lines++;
        editor.current_line++;
        editor.current_col = 0;
    }
}

int main() {
    simple_editor_init();
    
    while (1) {
        simple_editor_draw();
        
        char c = getch();
        
        if (c == 27) {  // ESC
            break;
        } else if (c == 8 || c == 127) {  // Backspace
            simple_editor_backspace();
        } else if (c == 13 || c == 10) {  // Enter
            simple_editor_new_line();
        } else if (c >= 32 && c <= 126) {  // Printable
            simple_editor_handle_char(c);
        }
        // Note: Arrow keys won't work in this simple version
        // but we can test basic functionality
    }
    
    clrscr();
    printf("Simple editor test completed.\n");
    return 0;
}
