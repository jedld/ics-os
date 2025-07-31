/*
 * Simple kernel logging to file implementation
 * Provides basic file logging capability for ICS-OS
 */

#include "../dextypes.h"
#include "../stdlib/dexstdlib.h"

#define KERNEL_LOG_FILE "kernel.log"
#define LOG_BUFFER_SIZE 8192

static char kernel_log_buffer[LOG_BUFFER_SIZE];
static int log_buffer_pos = 0;

// Add a log entry to the kernel log buffer
void kernel_log_add(const char* subsystem, const char* level, const char* message) {
    int msg_len = sprintf(&kernel_log_buffer[log_buffer_pos], 
                         "[%s] %s: %s\n", subsystem, level, message);
    
    log_buffer_pos += msg_len;
    
    // Simple wrap-around if buffer is full
    if (log_buffer_pos >= LOG_BUFFER_SIZE - 256) {
        log_buffer_pos = 0;
    }
}

// Get the current log buffer contents
const char* kernel_log_get_buffer(void) {
    return kernel_log_buffer;
}

// Dump the log buffer to console (for debugging)
void kernel_log_dump(void) {
    printf("=== KERNEL LOG DUMP ===\n");
    printf("%s", kernel_log_buffer);
    printf("=== END LOG DUMP ===\n");
}

// Clear the log buffer
void kernel_log_clear(void) {
    memset(kernel_log_buffer, 0, LOG_BUFFER_SIZE);
    log_buffer_pos = 0;
}
