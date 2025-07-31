/*
  Test program to figure out the exact control key mapping
*/
#include "../../sdk/dexsdk.h"

int main() {
    printf("Control Key Mapping Test\n");
    printf("========================\n");
    printf("Try pressing different Ctrl combinations:\n");
    printf("Expected standard mapping:\n");
    printf("  Ctrl+A = 1, Ctrl+B = 2, ..., Ctrl+P = 16, Ctrl+Q = 17\n");
    printf("Press ESC to quit\n\n");
    
    while (1) {
        int code = getch_enhanced();
        char base = code & 0xFF;
        
        printf("Raw code: 0x%04X (%d decimal)", code, base);
        
        if (base >= 1 && base <= 26) {
            char standard_letter = (base - 1) + 'a';  // Standard: 1='a', 2='b', etc.
            char offset_letter = base + 'a';          // Offset: 1='b', 2='c', etc.
            printf(" - Standard would be Ctrl+%c", standard_letter - 'a' + 'A');
            printf(" - If offset, would be Ctrl+%c", (offset_letter > 'z') ? '?' : (offset_letter - 'a' + 'A'));
        } else if (base == 27) {
            printf(" - ESC, exiting...\n");
            break;
        } else {
            printf(" - Not a control key or ESC");
        }
        printf("\n");
    }
    
    return 0;
}
