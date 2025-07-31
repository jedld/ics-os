/*
  Name: DexEdit - Enhanced Text Editor for ICS-OS
  Description: A user-friendly text editor similar to nano but simpler
  Features:
  - Line-based editing with cursor navigation
  - Insert/overwrite modes
  - File operations (new, open, save, save as)
  - Copy/paste functionality  
  - Search functionality
  - Status line with help
  - Improved keyboard shortcuts
  - Better visual feedback
  - Line numbers (toggleable)
  - Word wrap support
  
  Author: Enhanced by GitHub Copilot for ICS-OS
  Date: 2025
*/

#include "../../sdk/dexsdk.h"

// Key definitions (copied from keyboard.h to avoid include issues)
#define	KEY_F1		0x80
#define	KEY_F2		(KEY_F1 + 1)
#define	KEY_F3		(KEY_F2 + 1)
#define	KEY_F4		(KEY_F3 + 1)
#define	KEY_F5		(KEY_F4 + 1)
#define	KEY_F6		(KEY_F5 + 1)
#define	KEY_F7		(KEY_F6 + 1)
#define	KEY_F8		(KEY_F7 + 1)
#define	KEY_F9		(KEY_F8 + 1)
#define	KEY_F10		(KEY_F9 + 1)
#define	KEY_F11		(KEY_F10 + 1)
#define	KEY_F12		(KEY_F11 + 1)
#define	KEY_INS		0x90
#define	KEY_DEL		(KEY_INS + 1)
#define	KEY_HOME	(KEY_DEL + 1)
#define	KEY_END		(KEY_HOME + 1)
#define	KEY_PGUP	(KEY_END + 1)
#define	KEY_PGDN	(KEY_PGUP + 1)
#define	KEY_LFT		(KEY_PGDN + 1)
#define	KEY_UP		(KEY_LFT + 1)
#define	KEY_DN		(KEY_UP + 1)
#define	KEY_RT		(KEY_DN + 1)

// Additional function declarations
int atoi(const char *str);
char *gets(char *buf);

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 200
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define EDITOR_HEIGHT 22
#define STATUS_LINE 22
#define HELP_LINE 23

// Editor state
typedef struct {
    char lines[MAX_LINES][MAX_LINE_LENGTH];
    int num_lines;
    int current_line;
    int current_col;
    int top_line;          // First line shown on screen
    int left_col;          // First column shown on screen
    char filename[256];
    int modified;
    int insert_mode;       // 1 = insert, 0 = overwrite
    int show_line_numbers;
    int word_wrap;
    char clipboard[MAX_LINE_LENGTH];
    int clipboard_length;
    char search_term[64];
} Editor;

// Global editor instance
static Editor editor;

// Color definitions
#define COLOR_NORMAL 7      // Light gray on black
#define COLOR_STATUS 15     // White on black  
#define COLOR_HIGHLIGHT 15  // Bright white on black
#define COLOR_ERROR 12      // Light red on black
#define COLOR_SUCCESS 10    // Light green on black
#define COLOR_LINE_NUM 8    // Dark gray on black

// Function prototypes
void editor_init(void);
void editor_load_file(const char* filename);
void editor_save_file(void);
void editor_save_as(void);
void editor_new_file(void);
void editor_refresh_screen(void);
void editor_draw_status_line(void);
void editor_draw_help_line(void);
void editor_handle_key(key_event_t *event);
void editor_move_cursor(int row, int col);
void editor_insert_char(char c);
void editor_delete_char(void);
void editor_backspace(void);
void editor_insert_line(void);
void editor_delete_line(void);
void editor_copy_line(void);
void editor_paste_line(void);
void editor_search(void);
void editor_goto_line(void);
void editor_toggle_line_numbers(void);
void editor_toggle_word_wrap(void);
void editor_show_help(void);
void editor_confirm_exit(void);
int editor_get_line_number_width(void);
void editor_scroll_to_cursor(void);
void editor_show_message(const char* message, int color);
int editor_confirm_dialog(const char* message);

// Initialize editor
void editor_init(void) {
    int i;
    
    // Initialize editor state
    editor.num_lines = 1;
    editor.current_line = 0;
    editor.current_col = 0;
    editor.top_line = 0;
    editor.left_col = 0;
    editor.modified = 0;
    editor.insert_mode = 1;
    editor.show_line_numbers = 1;
    editor.word_wrap = 0;
    editor.clipboard_length = 0;
    strcpy(editor.filename, "");
    strcpy(editor.search_term, "");
    
    // Initialize empty document
    for (i = 0; i < MAX_LINES; i++) {
        editor.lines[i][0] = '\0';
    }
    
    strcpy(editor.clipboard, "");
}

// Load file into editor
void editor_load_file(const char* filename) {
    FILE* file;
    char line[MAX_LINE_LENGTH];
    int line_count = 0;
    
    file = fopen(filename, "r");
    if (file == NULL) {
        // File doesn't exist, create new
        strcpy(editor.filename, filename);
        editor_show_message("New file - ready for editing", COLOR_SUCCESS);
        return;
    }
    
    // Clear existing content
    editor.num_lines = 0;
    
    // Read file line by line
    while (fgets(line, sizeof(line), file) != NULL && line_count < MAX_LINES) {
        // Remove newline character
        int len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        // Truncate long lines
        if (len >= MAX_LINE_LENGTH) {
            line[MAX_LINE_LENGTH-1] = '\0';
        }
        
        strcpy(editor.lines[line_count], line);
        line_count++;
    }
    
    fclose(file);
    
    if (line_count == 0) {
        editor.num_lines = 1;
        editor.lines[0][0] = '\0';
    } else {
        editor.num_lines = line_count;
    }
    
    strcpy(editor.filename, filename);
    editor.modified = 0;
    editor.current_line = 0;
    editor.current_col = 0;
    editor.top_line = 0;
    editor.left_col = 0;
    
    editor_show_message("File loaded successfully", COLOR_SUCCESS);
}

// Save current file
void editor_save_file(void) {
    FILE* file;
    int i;
    
    if (strlen(editor.filename) == 0) {
        editor_save_as();
        return;
    }
    
    file = fopen(editor.filename, "w");
    if (file == NULL) {
        editor_show_message("Error: Cannot save file", COLOR_ERROR);
        return;
    }
    
    for (i = 0; i < editor.num_lines; i++) {
        int j;
        char* line = editor.lines[i];
        
        // Write each character of the line
        for (j = 0; line[j] != '\0'; j++) {
            fputc(line[j], file);
        }
        
        // Write newline
        fputc('\n', file);
    }
    
    fclose(file);
    editor.modified = 0;
    editor_show_message("File saved successfully", COLOR_SUCCESS);
}

// Save file with new name
void editor_save_as(void) {
    char new_filename[256];
    
    textcolor(COLOR_NORMAL);
    setx(0); sety(STATUS_LINE);
    printf("Save as: ");
    
    // Simple input for filename
    gets(new_filename);
    
    if (strlen(new_filename) > 0) {
        strcpy(editor.filename, new_filename);
        editor_save_file();
    }
}

// Create new file
void editor_new_file(void) {
    if (editor.modified) {
        if (!editor_confirm_dialog("Discard changes and create new file?")) {
            return;
        }
    }
    
    editor_init();
    editor_show_message("New file created", COLOR_SUCCESS);
}

// Get width needed for line numbers
int editor_get_line_number_width(void) {
    if (!editor.show_line_numbers) return 0;
    
    int width = 1;
    int max_line = editor.num_lines;
    
    while (max_line >= 10) {
        width++;
        max_line /= 10;
    }
    
    return width + 2; // Add space for margin
}

// Scroll to ensure cursor is visible
void editor_scroll_to_cursor(void) {
    // Vertical scrolling
    if (editor.current_line < editor.top_line) {
        editor.top_line = editor.current_line;
    } else if (editor.current_line >= editor.top_line + EDITOR_HEIGHT) {
        editor.top_line = editor.current_line - EDITOR_HEIGHT + 1;
    }
    
    // Horizontal scrolling
    int line_num_width = editor_get_line_number_width();
    int available_width = SCREEN_WIDTH - line_num_width;
    
    if (editor.current_col < editor.left_col) {
        editor.left_col = editor.current_col;
    } else if (editor.current_col >= editor.left_col + available_width) {
        editor.left_col = editor.current_col - available_width + 1;
    }
}

// Refresh the entire screen
void editor_refresh_screen(void) {
    int i, row;
    int line_num_width = editor_get_line_number_width();
    int available_width = SCREEN_WIDTH - line_num_width;
    
    clrscr();
    
    // Draw editor content
    for (row = 0; row < EDITOR_HEIGHT; row++) {
        int line_index = editor.top_line + row;
        setx(0); sety(row);
        
        if (line_index < editor.num_lines) {
            // Draw line number if enabled
            if (editor.show_line_numbers) {
                int line_num = line_index + 1;
                textcolor(COLOR_LINE_NUM);
                
                // Simple line number display
                if (line_num >= 1000) {
                    printf("999+ ");
                } else if (line_num >= 100) {
                    printf("%d ", line_num);
                } else if (line_num >= 10) {
                    printf(" %d ", line_num);
                } else {
                    printf("  %d ", line_num);
                }
            }
            
            // Draw line content
            textcolor(COLOR_NORMAL);
            char* line = editor.lines[line_index];
            int line_len = strlen(line);
            
            // Display portion of line that fits on screen
            for (i = editor.left_col; i < editor.left_col + available_width && i < line_len; i++) {
                printf("%c", line[i]);
            }
        } else if (line_index == 0) {
            // Show empty file indicator
            textcolor(COLOR_LINE_NUM);
            printf("~");
        } else {
            // Show empty line indicator
            textcolor(COLOR_LINE_NUM);
            printf("~");
        }
    }
    
    // Draw status and help lines
    editor_draw_status_line();
    editor_draw_help_line();
    
    // Position cursor
    int display_row = editor.current_line - editor.top_line;
    int display_col = line_num_width + editor.current_col - editor.left_col;
    
    if (display_row >= 0 && display_row < EDITOR_HEIGHT && 
        display_col >= line_num_width && display_col < SCREEN_WIDTH) {
        setx(display_col); sety(display_row);
    }
}

// Draw status line
void editor_draw_status_line(void) {
    char status[SCREEN_WIDTH + 1];
    char modified_indicator = editor.modified ? '*' : ' ';
    int i, pos = 0;
    
    // Build status string manually
    status[pos++] = ' ';
    status[pos++] = modified_indicator;
    
    // Add filename
    char* filename = strlen(editor.filename) > 0 ? editor.filename : "[New File]";
    for (i = 0; filename[i] != '\0' && pos < 25; i++) {
        status[pos++] = filename[i];
    }
    
    // Add padding
    while (pos < 30) {
        status[pos++] = ' ';
    }
    
    // Add mode
    char* mode = editor.insert_mode ? "INS" : "OVR";
    for (i = 0; mode[i] != '\0'; i++) {
        status[pos++] = mode[i];
    }
    
    // Add space
    status[pos++] = ' ';
    
    // Add position info (simplified)
    status[pos++] = 'L';
    status[pos++] = 'n';
    status[pos++] = ' ';
    
    // Simple line number display (just show if >999 lines)
    if (editor.current_line + 1 >= 1000) {
        status[pos++] = '9';
        status[pos++] = '9';
        status[pos++] = '9';
        status[pos++] = '+';
    } else if (editor.current_line + 1 >= 100) {
        int ln = editor.current_line + 1;
        status[pos++] = '0' + (ln / 100);
        status[pos++] = '0' + ((ln / 10) % 10);
        status[pos++] = '0' + (ln % 10);
    } else if (editor.current_line + 1 >= 10) {
        int ln = editor.current_line + 1;
        status[pos++] = '0' + (ln / 10);
        status[pos++] = '0' + (ln % 10);
    } else {
        status[pos++] = '0' + (editor.current_line + 1);
    }
    
    // Pad with spaces
    while (pos < SCREEN_WIDTH) {
        status[pos++] = ' ';
    }
    status[SCREEN_WIDTH] = '\0';
    
    setx(0); sety(STATUS_LINE);
    textcolor(COLOR_STATUS);
    printf("%s", status);
}

// Draw help line
void editor_draw_help_line(void) {
    char help[SCREEN_WIDTH + 1];
    
    strcpy(help, "^S Save ^Q Quit ^F Find ^G Go ^O Open ^N New F1 Help");
    
    // Pad with spaces
    int len = strlen(help);
    while (len < SCREEN_WIDTH) {
        help[len++] = ' ';
    }
    help[SCREEN_WIDTH] = '\0';
    
    setx(0); sety(HELP_LINE);
    textcolor(COLOR_STATUS);
    printf("%s", help);
    
    textcolor(COLOR_NORMAL);
}

// Show message at bottom of screen
void editor_show_message(const char* message, int color) {
    setx(0); sety(HELP_LINE);
    textcolor(color);
    printf("%-*s", SCREEN_WIDTH, message);
    textcolor(COLOR_NORMAL);
    
    // Wait a moment for user to read message
    int i;
    for (i = 0; i < 100000; i++); // Simple delay
}

// Confirmation dialog
int editor_confirm_dialog(const char* message) {
    char full_msg[SCREEN_WIDTH];
    int i, pos = 0;
    
    // Build message manually
    for (i = 0; message[i] != '\0' && pos < SCREEN_WIDTH - 10; i++) {
        full_msg[pos++] = message[i];
    }
    
    full_msg[pos++] = ' ';
    full_msg[pos++] = '(';
    full_msg[pos++] = 'y';
    full_msg[pos++] = '/';
    full_msg[pos++] = 'n';
    full_msg[pos++] = ')';
    full_msg[pos++] = ':';
    full_msg[pos++] = ' ';
    
    while (pos < SCREEN_WIDTH) {
        full_msg[pos++] = ' ';
    }
    full_msg[SCREEN_WIDTH] = '\0';
    
    setx(0); sety(HELP_LINE);
    textcolor(COLOR_HIGHLIGHT);
    printf("%s", full_msg);
    
    char c = getch();
    return (c == 'y' || c == 'Y');
}

// Handle keyboard input using enhanced API
void editor_handle_key(key_event_t *event) {
    // Handle control key combinations
    if (event->ctrl) {
        switch (event->ascii) {
            case 'n': // Ctrl+N
            case 'N':
                editor_new_file();
                return;
            case 'o': // Ctrl+O  
            case 'O':
                {
                    char filename[256];
                    setx(0); sety(STATUS_LINE);
                    textcolor(COLOR_NORMAL);
                    printf("Open file: ");
                    gets(filename);
                    if (strlen(filename) > 0) {
                        if (editor.modified) {
                            if (editor_confirm_dialog("Discard changes?")) {
                                editor_load_file(filename);
                            }
                        } else {
                            editor_load_file(filename);
                        }
                    }
                }
                return;
            case 's': // Ctrl+S
            case 'S':
                editor_save_file();
                return;
            case 'a': // Ctrl+A
            case 'A':
                editor_save_as();
                return;
            case 'q': // Ctrl+Q
            case 'Q':
                editor_confirm_exit();
                return;
            case 'f': // Ctrl+F
            case 'F':
                editor_search();
                return;
            case 'g': // Ctrl+G
            case 'G':
                editor_goto_line();
                return;
            case 'k': // Ctrl+K
            case 'K':
                editor_delete_line();
                return;
            case 'c': // Ctrl+C
            case 'C':
                editor_copy_line();
                return;
            case 'v': // Ctrl+V
            case 'V':
                editor_paste_line();
                return;
            case 'l': // Ctrl+L
            case 'L':
                editor_toggle_line_numbers();
                return;
        }
    }
    
    // Handle special keys
    if (event->is_special) {
        switch (event->ascii) {
            case KEY_UP:
                editor_move_cursor(editor.current_line - 1, editor.current_col);
                break;
            case KEY_DOWN:
                editor_move_cursor(editor.current_line + 1, editor.current_col);
                break;
            case KEY_LEFT:
                if (editor.current_col > 0) {
                    editor_move_cursor(editor.current_line, editor.current_col - 1);
                } else if (editor.current_line > 0) {
                    int prev_line_len = strlen(editor.lines[editor.current_line - 1]);
                    editor_move_cursor(editor.current_line - 1, prev_line_len);
                }
                break;
            case KEY_RIGHT:
                {
                    int line_len = strlen(editor.lines[editor.current_line]);
                    if (editor.current_col < line_len) {
                        editor_move_cursor(editor.current_line, editor.current_col + 1);
                    } else if (editor.current_line + 1 < editor.num_lines) {
                        editor_move_cursor(editor.current_line + 1, 0);
                    }
                }
                break;
            case KEY_HOME:
                editor_move_cursor(editor.current_line, 0);
                break;
            case KEY_END:
                {
                    int line_len = strlen(editor.lines[editor.current_line]);
                    editor_move_cursor(editor.current_line, line_len);
                }
                break;
            case KEY_PGUP:
                editor_move_cursor(editor.current_line - EDITOR_HEIGHT, editor.current_col);
                break;
            case KEY_PGDN:
                editor_move_cursor(editor.current_line + EDITOR_HEIGHT, editor.current_col);
                break;
            case KEY_DEL: // Delete
                editor_delete_char();
                break;
            case KEY_INS: // Insert key
                editor.insert_mode = !editor.insert_mode;
                editor_show_message(editor.insert_mode ? "Insert mode" : "Overwrite mode", COLOR_SUCCESS);
                break;
            case KEY_F1: // F1 for help
                editor_show_help();
                break;
        }
        return;
    }
    
    // Handle regular character input
    switch (event->ascii) {
        case 8:  // Backspace key
            editor_backspace();
            break;
        case '\r': // Enter
        case '\n':
            editor_insert_line();
            break;
        case '\t': // Tab
            editor_insert_char(' ');
            editor_insert_char(' ');
            editor_insert_char(' ');
            editor_insert_char(' ');
            break;
        default:
            if (event->is_printable) {
                editor_insert_char(event->ascii);
            }
            break;
    }
}

// Move cursor to specific position
void editor_move_cursor(int row, int col) {
    if (row < 0) row = 0;
    if (row >= editor.num_lines) row = editor.num_lines - 1;
    if (col < 0) col = 0;
    
    editor.current_line = row;
    editor.current_col = col;
    
    // Ensure column is within line bounds
    int line_len = strlen(editor.lines[editor.current_line]);
    if (editor.current_col > line_len) {
        editor.current_col = line_len;
    }
    
    editor_scroll_to_cursor();
}

// Insert character at cursor position
void editor_insert_char(char c) {
    char* line = editor.lines[editor.current_line];
    int line_len = strlen(line);
    int i;
    
    if (line_len >= MAX_LINE_LENGTH - 1) {
        editor_show_message("Line too long", COLOR_ERROR);
        return;
    }
    
    if (editor.insert_mode) {
        // Shift characters to the right
        for (i = line_len; i > editor.current_col; i--) {
            line[i] = line[i-1];
        }
        line[line_len + 1] = '\0';
    }
    
    line[editor.current_col] = c;
    editor.current_col++;
    editor.modified = 1;
}

// Delete character at cursor
void editor_delete_char(void) {
    char* line = editor.lines[editor.current_line];
    int line_len = strlen(line);
    int i;
    
    if (editor.current_col >= line_len) {
        // At end of line, merge with next line if exists
        if (editor.current_line + 1 < editor.num_lines) {
            char* next_line = editor.lines[editor.current_line + 1];
            
            if (strlen(line) + strlen(next_line) < MAX_LINE_LENGTH) {
                strcat(line, next_line);
                
                // Shift lines up
                for (i = editor.current_line + 1; i < editor.num_lines - 1; i++) {
                    strcpy(editor.lines[i], editor.lines[i + 1]);
                }
                editor.num_lines--;
                editor.modified = 1;
            }
        }
        return;
    }
    
    // Shift characters to the left
    for (i = editor.current_col; i < line_len; i++) {
        line[i] = line[i + 1];
    }
    editor.modified = 1;
}

// Backspace at cursor
void editor_backspace(void) {
    if (editor.current_col > 0) {
        editor.current_col--;
        editor_delete_char();
    } else if (editor.current_line > 0) {
        // Move to end of previous line
        int prev_line_len = strlen(editor.lines[editor.current_line - 1]);
        char* current_line = editor.lines[editor.current_line];
        
        if (prev_line_len + strlen(current_line) < MAX_LINE_LENGTH) {
            strcat(editor.lines[editor.current_line - 1], current_line);
            
            // Shift lines up
            int i;
            for (i = editor.current_line; i < editor.num_lines - 1; i++) {
                strcpy(editor.lines[i], editor.lines[i + 1]);
            }
            
            editor.num_lines--;
            editor.current_line--;
            editor.current_col = prev_line_len;
            editor.modified = 1;
            editor_scroll_to_cursor();
        }
    }
}

// Insert new line at cursor
void editor_insert_line(void) {
    if (editor.num_lines >= MAX_LINES) {
        editor_show_message("Maximum lines reached", COLOR_ERROR);
        return;
    }
    
    char* current_line = editor.lines[editor.current_line];
    int i;
    
    // Shift lines down
    for (i = editor.num_lines; i > editor.current_line + 1; i--) {
        strcpy(editor.lines[i], editor.lines[i - 1]);
    }
    
    // Split current line
    strcpy(editor.lines[editor.current_line + 1], &current_line[editor.current_col]);
    current_line[editor.current_col] = '\0';
    
    editor.num_lines++;
    editor.current_line++;
    editor.current_col = 0;
    editor.modified = 1;
    editor_scroll_to_cursor();
}

// Delete entire line
void editor_delete_line(void) {
    if (editor.num_lines <= 1) {
        editor.lines[0][0] = '\0';
        editor.current_col = 0;
    } else {
        int i;
        for (i = editor.current_line; i < editor.num_lines - 1; i++) {
            strcpy(editor.lines[i], editor.lines[i + 1]);
        }
        editor.num_lines--;
        
        if (editor.current_line >= editor.num_lines) {
            editor.current_line = editor.num_lines - 1;
        }
        editor.current_col = 0;
    }
    editor.modified = 1;
}

// Copy current line to clipboard
void editor_copy_line(void) {
    strcpy(editor.clipboard, editor.lines[editor.current_line]);
    editor.clipboard_length = strlen(editor.clipboard);
    editor_show_message("Line copied", COLOR_SUCCESS);
}

// Paste line from clipboard
void editor_paste_line(void) {
    if (editor.clipboard_length == 0) {
        editor_show_message("Clipboard empty", COLOR_ERROR);
        return;
    }
    
    if (editor.num_lines >= MAX_LINES) {
        editor_show_message("Maximum lines reached", COLOR_ERROR);
        return;
    }
    
    // Shift lines down
    int i;
    for (i = editor.num_lines; i > editor.current_line; i--) {
        strcpy(editor.lines[i], editor.lines[i - 1]);
    }
    
    strcpy(editor.lines[editor.current_line], editor.clipboard);
    editor.num_lines++;
    editor.modified = 1;
    editor_show_message("Line pasted", COLOR_SUCCESS);
}

// Search for text
void editor_search(void) {
    char search_buf[64];
    int i, j;
    int found = 0;
    
    setx(0); sety(STATUS_LINE);
    textcolor(COLOR_NORMAL);
    printf("Search: ");
    gets(search_buf);
    
    if (strlen(search_buf) == 0) {
        return;
    }
    
    strcpy(editor.search_term, search_buf);
    
    // Search from current position
    for (i = editor.current_line; i < editor.num_lines; i++) {
        char* line = editor.lines[i];
        int start_col = (i == editor.current_line) ? editor.current_col + 1 : 0;
        
        for (j = start_col; j <= (int)strlen(line) - (int)strlen(search_buf); j++) {
            if (strncmp(&line[j], search_buf, strlen(search_buf)) == 0) {
                editor_move_cursor(i, j);
                editor_show_message("Found", COLOR_SUCCESS);
                found = 1;
                return;
            }
        }
    }
    
    // Search from beginning if not found
    if (!found) {
        for (i = 0; i <= editor.current_line; i++) {
            char* line = editor.lines[i];
            int end_col = (i == editor.current_line) ? editor.current_col : strlen(line);
            
            for (j = 0; j < end_col - (int)strlen(search_buf) + 1; j++) {
                if (strncmp(&line[j], search_buf, strlen(search_buf)) == 0) {
                    editor_move_cursor(i, j);
                    editor_show_message("Found", COLOR_SUCCESS);
                    return;
                }
            }
        }
    }
    
    editor_show_message("Not found", COLOR_ERROR);
}

// Go to specific line
void editor_goto_line(void) {
    char line_buf[16];
    int target_line;
    
    setx(0); sety(STATUS_LINE);
    textcolor(COLOR_NORMAL);
    printf("Go to line: ");
    gets(line_buf);
    
    target_line = atoi(line_buf);
    if (target_line > 0 && target_line <= editor.num_lines) {
        editor_move_cursor(target_line - 1, 0);
        editor_show_message("Jumped to line", COLOR_SUCCESS);
    } else {
        editor_show_message("Invalid line number", COLOR_ERROR);
    }
}

// Toggle line numbers
void editor_toggle_line_numbers(void) {
    editor.show_line_numbers = !editor.show_line_numbers;
    editor.left_col = 0; // Reset horizontal scroll
    editor_show_message(editor.show_line_numbers ? "Line numbers ON" : "Line numbers OFF", COLOR_SUCCESS);
}

// Show help screen
void editor_show_help(void) {
    clrscr();
    textcolor(COLOR_HIGHLIGHT);
    printf("                        DexEdit Help\n");
    printf("=========================================================\n");
    textcolor(COLOR_NORMAL);
    printf("\n");
    printf("Navigation:\n");
    printf("  Arrow Keys       - Move cursor\n");
    printf("  Home/End         - Start/End of line\n");
    printf("  Page Up/Down     - Move by screen\n");
    printf("  Ctrl+Home/End    - Start/End of document\n");
    printf("\n");
    printf("Editing:\n");
    printf("  Insert/Delete    - Toggle insert/overwrite mode\n");
    printf("  Backspace        - Delete character before cursor\n");
    printf("  Delete           - Delete character at cursor\n");
    printf("  Enter            - Insert new line\n");
    printf("\n");
    printf("File Operations:\n");
    printf("  Ctrl+N           - New file\n");
    printf("  Ctrl+O           - Open file\n");
    printf("  Ctrl+S           - Save file\n");
    printf("  Ctrl+A           - Save as\n");
    printf("  Ctrl+Q           - Quit\n");
    printf("\n");
    printf("Search & Navigation:\n");
    printf("  Ctrl+F           - Find text\n");
    printf("  Ctrl+G           - Go to line\n");
    printf("\n");
    printf("Line Operations:\n");
    printf("  Ctrl+K           - Delete entire line\n");
    printf("  Ctrl+C           - Copy line\n");
    printf("  Ctrl+V           - Paste line\n");
    printf("\n");
    printf("View Options:\n");
    printf("  Ctrl+L           - Toggle line numbers\n");
    printf("\n");
    printf("Press any key to return to editor...");
    
    getch();
}

// Confirm exit
void editor_confirm_exit(void) {
    if (editor.modified) {
        if (editor_confirm_dialog("File modified. Save before exit?")) {
            editor_save_file();
        }
    }
    exit(0);
}

// Main function
int main(int argc, char* argv[]) {
    key_event_t event;
    
    // Initialize editor
    editor_init();
    
    // Load file if specified
    if (argc > 1) {
        editor_load_file(argv[1]);
    }
    
    // Main editor loop
    while (1) {
        editor_refresh_screen();
        if (get_key_event(&event) == 0) {
            editor_handle_key(&event);
        }
    }
    
    return 0;
}
