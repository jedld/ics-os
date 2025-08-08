/*
  Arrow Key Code Test
  Test what codes arrow keys actually produce in ICS-OS
*/

#include "../../sdk/dexsdk.h"

int main() {
    printf("Arrow Key Code Test\n");
    printf("Press arrow keys to see their codes\n");
    printf("ESC to exit\n\n");
    
    while (1) {
        char c = getch();
        
        if (c == 27) {  // ESC
            break;
        }
        
        printf("Key pressed: %d (0x%02x)", (int)c, (unsigned char)c);
        
        // Check against our expected arrow key codes
        switch ((unsigned char)c) {
            case 0x96: printf(" = LEFT arrow (expected)"); break;
            case 0x97: printf(" = UP arrow (expected)"); break;
            case 0x98: printf(" = DOWN arrow (expected)"); break;
            case 0x99: printf(" = RIGHT arrow (expected)"); break;
            default:
                if (c >= 32 && c <= 126) {
                    printf(" = '%c' (printable)", c);
                } else {
                    printf(" = (special/control)");
                }
                break;
        }
        printf("\n");
    }
    
    printf("\nArrow key test completed.\n");
    return 0;
}
