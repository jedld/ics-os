/*
 * Simple kernel logging to file header
 * Provides basic file logging capability for ICS-OS
 */

#ifndef __SIMPLE_LOG_H__
#define __SIMPLE_LOG_H__

// Function prototypes
void kernel_log_add(const char* subsystem, const char* level, const char* message);
const char* kernel_log_get_buffer(void);
void kernel_log_dump(void);
void kernel_log_clear(void);

#endif // __SIMPLE_LOG_H__
