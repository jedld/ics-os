/*
  Name: Keyboard Diagnostic Tool for DexEdit
  Description: Debug tool to see what keyboard events are being received
*/

#include "../../sdk/dexsdk.h"

int main() {
    key_event_t event;
    
    clrscr();
    printf("DexEdit Keyboard Diagnostic Tool\n");
    printf("================================\n\n");
    printf("Press keys to see what events are detected.\n");
    printf("Press Ctrl+Q to quit.\n\n");
    printf("Legend:\n");
    printf("  ctrl=1 means Ctrl key is pressed\n");
    printf("  is_special=1 means special key (arrows, F-keys, etc.)\n");
    printf("  is_printable=1 means printable character\n\n");
    
    while (1) {
        if (get_key_event(&event) == 0) {
            printf("Key Event:\n");
            printf("  code=0x%04X ascii='%c'(%d) ctrl=%d alt=%d shift=%d\n", 
                   event.code, 
                   (event.ascii >= 32 && event.ascii <= 126) ? event.ascii : '?',
                   event.ascii,
                   event.ctrl, event.alt, event.shift);
            printf("  is_special=%d is_printable=%d\n", 
                   event.is_special, event.is_printable);
                   
            // Check for specific keys we care about
            if (event.is_special) {
                printf("  Special key detected: ");
                switch (event.ascii) {
                    case 0x97: printf("KEY_UP (0x97)\n"); break;
                    case 0x98: printf("KEY_DOWN (0x98)\n"); break;
                    case 0x96: printf("KEY_LEFT (0x96)\n"); break;
                    case 0x99: printf("KEY_RIGHT (0x99)\n"); break;
                    case 0x92: printf("KEY_HOME (0x92)\n"); break;
                    case 0x93: printf("KEY_END (0x93)\n"); break;
                    case 0x91: printf("KEY_DEL (0x91)\n"); break;
                    case 0x90: printf("KEY_INS (0x90)\n"); break;
                    default: printf("Unknown special (0x%02X)\n", event.ascii); break;
                }
            } else if (event.ascii == 8) {
                printf("  BACKSPACE detected (ASCII 8)\n");
            } else if (event.ascii == 13 || event.ascii == 10) {
                printf("  ENTER detected (ASCII %d)\n", event.ascii);
            }
            
            printf("\n");
            
            // Quit on Ctrl+Q
            if (event.ctrl && (event.ascii == 'q' || event.ascii == 'Q')) {
                printf("Ctrl+Q pressed, exiting...\n");
                break;
            }
        }
    }
    
    return 0;
}
