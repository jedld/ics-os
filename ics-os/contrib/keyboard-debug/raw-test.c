/*
  Simple test using raw getch() instead of enhanced API
*/

#include "../../sdk/dexsdk.h"

int main() {
    clrscr();
    printf("Raw Keyboard Test\n");
    printf("=================\n\n");
    printf("Using raw getch() - press keys to see codes\n");
    printf("Press 'q' to quit\n\n");
    
    while (1) {
        char c = getch();
        printf("Raw getch() returned: %d (0x%02X)", (unsigned char)c, (unsigned char)c);
        
        if (c >= 32 && c <= 126) {
            printf(" '%c'", c);
        }
        
        if (c == 8) {
            printf(" BACKSPACE");
        } else if (c == 13 || c == 10) {
            printf(" ENTER");
        } else if (c == 27) {
            printf(" ESC");
        }
        
        printf("\n");
        
        if (c == 'q' || c == 'Q') {
            break;
        }
    }
    
    return 0;
}
