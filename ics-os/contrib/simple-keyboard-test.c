/*
 * Simple keyboard test for dexedit issues
 * This will test basic getch() functionality
 */

#include "../sdk/dexsdk.h"

int main() {
    char c;
    
    printf("Simple Keyboard Test\n");
    printf("Press keys to see their ASCII codes (q to quit)\n");
    printf("Testing basic getch() function:\n\n");
    
    while (1) {
        c = getch();
        
        printf("Key pressed: ASCII %d (0x%02X)", c, (unsigned char)c);
        
        if (c >= 32 && c <= 126) {
            printf(" '%c'", c);
        }
        
        if (c == '\r') {
            printf(" (Enter/Carriage Return)");
        } else if (c == '\n') {
            printf(" (Line Feed)");
        } else if (c == '\b') {
            printf(" (Backspace)");
        } else if ((unsigned char)c == 145) {
            printf(" (ICS-OS Backspace)");
        }
        
        printf("\n");
        
        if (c == 'q' || c == 'Q') {
            break;
        }
    }
    
    printf("Keyboard test completed.\n");
    return 0;
}
