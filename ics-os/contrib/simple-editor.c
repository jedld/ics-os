/*
  Name: Simple Text Editor for ICS-OS (Basic getch version)
  Description: A minimal text editor using only basic getch() to avoid segfaults
  This will help us isolate the keyboard input issues
*/

#include "../sdk/dexsdk.h"

#define MAX_LINES 100
#define MAX_LINE_LENGTH 80
#define SCREEN_HEIGHT 25

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LENGTH];
    int num_lines;
    int current_line;
    int current_col;
    char filename[256];
    int modified;
} SimpleEditor;

static SimpleEditor editor;

void editor_init(void) {
    int i;
    editor.num_lines = 1;
    editor.current_line = 0;
    editor.current_col = 0;
    editor.modified = 0;
    strcpy(editor.filename, "untitled.txt");
    
    // Initialize with empty lines
    for (i = 0; i < MAX_LINES; i++) {
        editor.lines[i][0] = '\0';
    }
}

void editor_refresh_screen(void) {
    int i;
    clrscr();
    
    // Display content
    for (i = 0; i < SCREEN_HEIGHT - 2 && i < editor.num_lines; i++) {
        printf("%2d: %s\n", i + 1, editor.lines[i]);
    }
    
    // Status line
    sety(SCREEN_HEIGHT - 2);
    setx(0);
    printf("File: %s %s | Line %d, Col %d | Ctrl+Q=Quit, Ctrl+S=Save",
           editor.filename, 
           editor.modified ? "(modified)" : "",
           editor.current_line + 1, 
           editor.current_col + 1);
    
    // Position cursor
    sety(editor.current_line);
    setx(4 + editor.current_col); // 4 = "xx: " prefix
}

void editor_insert_char(char c) {
    char *line = editor.lines[editor.current_line];
    int len = strlen(line);
    int i;
    
    if (len >= MAX_LINE_LENGTH - 1) return; // Line too long
    
    // Shift characters to the right
    for (i = len; i >= editor.current_col; i--) {
        line[i + 1] = line[i];
    }
    
    line[editor.current_col] = c;
    editor.current_col++;
    editor.modified = 1;
}

void editor_backspace(void) {
    char *line = editor.lines[editor.current_line];
    int len = strlen(line);
    int i;
    
    if (editor.current_col > 0) {
        // Delete character at current position - 1
        editor.current_col--;
        for (i = editor.current_col; i < len; i++) {
            line[i] = line[i + 1];
        }
        editor.modified = 1;
    } else if (editor.current_line > 0) {
        // Join with previous line
        int prev_len = strlen(editor.lines[editor.current_line - 1]);
        if (prev_len + len < MAX_LINE_LENGTH - 1) {
            strcat(editor.lines[editor.current_line - 1], line);
            
            // Remove current line
            for (i = editor.current_line; i < editor.num_lines - 1; i++) {
                strcpy(editor.lines[i], editor.lines[i + 1]);
            }
            
            editor.num_lines--;
            editor.current_line--;
            editor.current_col = prev_len;
            editor.modified = 1;
        }
    }
}

void editor_insert_line(void) {
    char *line = editor.lines[editor.current_line];
    int i;
    
    if (editor.num_lines >= MAX_LINES) return; // Too many lines
    
    // Shift lines down
    for (i = editor.num_lines; i > editor.current_line + 1; i--) {
        strcpy(editor.lines[i], editor.lines[i - 1]);
    }
    
    // Split current line
    strcpy(editor.lines[editor.current_line + 1], &line[editor.current_col]);
    line[editor.current_col] = '\0';
    
    editor.num_lines++;
    editor.current_line++;
    editor.current_col = 0;
    editor.modified = 1;
}

void editor_save_file(void) {
    FILE *f = fopen(editor.filename, "w");
    int i;
    
    if (!f) {
        // Error saving - just ignore for now
        return;
    }
    
    for (i = 0; i < editor.num_lines; i++) {
        fputs(editor.lines[i], f);
        if (i < editor.num_lines - 1) {
            fputs("\n", f);
        }
    }
    
    fclose(f);
    editor.modified = 0;
}

int main(int argc, char* argv[]) {
    char c;
    int quit = 0;
    
    editor_init();
    
    // Load file if specified
    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (f) {
            strcpy(editor.filename, argv[1]);
            editor.num_lines = 0;
            
            while (fgets(editor.lines[editor.num_lines], MAX_LINE_LENGTH, f) && 
                   editor.num_lines < MAX_LINES) {
                // Remove newline
                int len = strlen(editor.lines[editor.num_lines]);
                if (len > 0 && editor.lines[editor.num_lines][len-1] == '\n') {
                    editor.lines[editor.num_lines][len-1] = '\0';
                }
                editor.num_lines++;
            }
            
            fclose(f);
            if (editor.num_lines == 0) {
                editor.num_lines = 1;
            }
        }
    }
    
    while (!quit) {
        editor_refresh_screen();
        c = getch();
        
        // Handle control keys
        if (c == 17) { // Ctrl+Q
            quit = 1;
        } else if (c == 19) { // Ctrl+S
            editor_save_file();
        } else if (c == '\r' || c == '\n') { // Enter
            editor_insert_line();
        } else if (c == '\b' || c == 8 || (unsigned char)c == 145) { // Backspace
            editor_backspace();
        } else if (c >= 32 && c <= 126) { // Printable characters
            editor_insert_char(c);
        }
        // For now, ignore arrow keys and other special keys
    }
    
    return 0;
}
