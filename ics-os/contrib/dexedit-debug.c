/*
 * Debug version of dexedit to test keyboard input
 * This will show what key codes are being received
 */

#include "../sdk/dexsdk.h"

int main() {
    key_event_t event;
    
    printf("DexEdit Keyboard Debug Tool\n");
    printf("Press keys to see their codes (Ctrl+Q to exit)\n");
    printf("Format: [code] [ascii] [special] [printable] [ctrl] [alt] [shift]\n\n");
    
    while (1) {
        if (get_key_event(&event) == 0) {
            printf("Enhanced API: ");
        } else {
            // Fallback to simple getch if enhanced API fails
            char c = getch();
            // Convert to simple key event
            event.code = c;
            event.ascii = c;
            event.ctrl = 0;
            event.alt = 0;
            event.shift = 0;
            // Properly detect special keys in fallback mode
            event.is_special = (c >= 0x80 && c <= 0x99) ? 1 : 0;
            event.is_printable = (c >= 32 && c <= 126) ? 1 : 0;
            printf("Fallback API: ");
        }
        
        printf("[%d] [%d] [%d] [%d] [%d] [%d] [%d]", 
               event.code, event.ascii, event.is_special, event.is_printable,
               event.ctrl, event.alt, event.shift);
               
        if (event.ascii >= 32 && event.ascii <= 126) {
            printf(" '%c'", event.ascii);
        }
        
        printf("\n");
        
        // Exit on Ctrl+Q
        if (event.ctrl && (event.ascii == 'q' || event.ascii == 'Q')) {
            break;
        }
    }
    
    return 0;
}
