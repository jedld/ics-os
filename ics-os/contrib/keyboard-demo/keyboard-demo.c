/*
  Name: Keyboard Demo - Enhanced Keyboard API Demonstration
  Description: Shows how to use the enhanced keyboard functions in ICS-OS SDK
  Author: Enhanced by GitHub Copilot for ICS-OS
  Date: 2025
*/

#include "../../sdk/dexsdk.h"

void print_key_info(key_event_t *event) {
    printf("Key pressed: ");
    
    if (event->ctrl) printf("Ctrl+");
    if (event->alt) printf("Alt+");
    if (event->shift) printf("Shift+");
    
    if (event->is_special) {
        switch (event->ascii) {
            case KEY_F1: printf("F1"); break;
            case KEY_F2: printf("F2"); break;
            case KEY_F3: printf("F3"); break;
            case KEY_F4: printf("F4"); break;
            case KEY_F5: printf("F5"); break;
            case KEY_F6: printf("F6"); break;
            case KEY_F7: printf("F7"); break;
            case KEY_F8: printf("F8"); break;
            case KEY_F9: printf("F9"); break;
            case KEY_F10: printf("F10"); break;
            case KEY_F11: printf("F11"); break;
            case KEY_F12: printf("F12"); break;
            case KEY_UP: printf("Up Arrow"); break;
            case KEY_DOWN: printf("Down Arrow"); break;
            case KEY_LEFT: printf("Left Arrow"); break;
            case KEY_RIGHT: printf("Right Arrow"); break;
            case KEY_HOME: printf("Home"); break;
            case KEY_END: printf("End"); break;
            case KEY_PGUP: printf("Page Up"); break;
            case KEY_PGDN: printf("Page Down"); break;
            case KEY_INS: printf("Insert"); break;
            case KEY_DEL: printf("Delete"); break;
            default: printf("Special Key (0x%02X)", event->ascii); break;
        }
    } else if (event->ctrl) {
        /* Show the control key combination */
        if (event->ascii >= 'a' && event->ascii <= 'z') {
            printf("%c", event->ascii - 'a' + 'A'); /* Show uppercase */
        } else {
            printf("%c", event->ascii);
        }
    } else if (event->is_printable) {
        printf("'%c'", event->ascii);
    } else {
        printf("Non-printable (0x%02X)", event->ascii);
    }
    
    printf(" (Raw code: 0x%04X)\n", event->code);
}

int main() {
    key_event_t event;
    
    clrscr();
    printf("Enhanced Keyboard API Demo\n");
    printf("==========================\n\n");
    printf("Press any key to see detailed information.\n");
    printf("Press Ctrl+Q to quit.\n\n");
    
    while (1) {
        if (get_key_event(&event) == 0) {
            print_key_info(&event);
            
            /* Check for Ctrl+Q to quit */
            if (event.ctrl && (event.ascii == 'q' || event.ascii == 'Q')) {
                printf("\nGoodbye!\n");
                break;
            }
            
            /* Demonstrate simple key handling */
            if (event.ctrl) {
                switch (event.ascii) {
                    case 's':
                    case 'S':
                        printf("  -> This would save a file!\n");
                        break;
                    case 'o':
                    case 'O':
                        printf("  -> This would open a file!\n");
                        break;
                    case 'n':
                    case 'N':
                        printf("  -> This would create a new file!\n");
                        break;
                }
            }
            
            printf("\n");
        }
    }
    
    return 0;
}
