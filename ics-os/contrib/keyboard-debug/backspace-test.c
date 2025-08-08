/*
  Backspace Key Test
  Test what code backspace actually produces in ICS-OS
*/

#include "../../sdk/dexsdk.h"

int main() {
    printf("Backspace Key Test\n");
    printf("Press backspace to see its code\n");
    printf("Also test other keys for comparison\n");
    printf("ESC to exit\n\n");
    
    while (1) {
        char c = getch();
        
        if (c == 27) {  // ESC
            break;
        }
        
        printf("Key pressed: %d (0x%02x)", (int)c, (unsigned char)c);
        
        // Check for backspace codes
        if (c == 8) {
            printf(" = Backspace (ASCII 8)");
        } else if (c == 127) {
            printf(" = Delete/Backspace (ASCII 127)");
        } else if ((unsigned char)c == 145) {
            printf(" = Backspace (145)");
        } else if (c >= 32 && c <= 126) {
            printf(" = '%c' (printable)", c);
        } else {
            printf(" = (control/special)");
        }
        printf("\n");
    }
    
    printf("\nBackspace test completed.\n");
    return 0;
}
