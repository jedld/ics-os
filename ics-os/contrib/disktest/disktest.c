/*
  Name: ICS-OS Disk I/O Performance Test
  Author: Generated for ICS-OS floppy drive optimization testing
  Description: A simple disk read/write performance testing utility
  that measures floppy disk I/O speeds and reports results.
*/

#include "../../sdk/dexsdk.h"
#include "../../sdk/time.h"

#define TEST_FILE "testfile.dat"
#define BUFFER_SIZE 512       // 512 bytes (1 sector)
#define LARGE_BUFFER_SIZE 2048 // 2KB for multi-sector tests
#define NUM_ITERATIONS 10     // Number of test iterations

// Test data patterns
const char pattern1[BUFFER_SIZE] = {0}; // Will be filled with 0xAA
const char pattern2[BUFFER_SIZE] = {0}; // Will be filled with 0x55

void fill_pattern(char *buffer, int size, char pattern) {
    int i;
    for (i = 0; i < size; i++) {
        buffer[i] = pattern;
    }
}

void print_separator() {
    printf("========================================\n");
}

void print_header() {
    print_separator();
    printf("    ICS-OS Disk I/O Performance Test\n");
    print_separator();
    printf("Testing floppy drive read/write speeds\n");
    printf("Buffer size: %d bytes\n", BUFFER_SIZE);
    printf("Iterations: %d\n", NUM_ITERATIONS);
    print_separator();
}

int test_sequential_write() {
    FILE *file;
    char buffer[BUFFER_SIZE];
    int start_time, end_time, elapsed;
    int i, bytes_written = 0;
    
    printf("Sequential Write Test:\n");
    
    // Fill buffer with test pattern
    fill_pattern(buffer, BUFFER_SIZE, 0xAA);
    
    // Start timing
    start_time = time();
    
    // Open file for writing
    file = fopen(TEST_FILE, "wb");
    if (!file) {
        printf("Error: Cannot create test file!\n");
        return 0;
    }
    
    // Write data multiple times
    for (i = 0; i < NUM_ITERATIONS; i++) {
        int written = fwrite(buffer, 1, BUFFER_SIZE, file);
        bytes_written += written;
        if (written != BUFFER_SIZE) {
            printf("Warning: Write %d only wrote %d bytes\n", i, written);
        }
    }
    
    fclose(file);
    
    // End timing
    end_time = time();
    elapsed = end_time - start_time;
    
    if (elapsed > 0) {
        printf("  Wrote %d bytes in %d ticks\n", bytes_written, elapsed);
        printf("  Throughput: %d bytes/tick\n", bytes_written / elapsed);
    } else {
        printf("  Wrote %d bytes (too fast to measure)\n", bytes_written);
    }
    
    return bytes_written;
}

int test_sequential_read() {
    FILE *file;
    char buffer[BUFFER_SIZE];
    int start_time, end_time, elapsed;
    int i, bytes_read = 0;
    
    printf("Sequential Read Test:\n");
    
    // Start timing
    start_time = time();
    
    // Open file for reading
    file = fopen(TEST_FILE, "rb");
    if (!file) {
        printf("Error: Cannot open test file for reading!\n");
        return 0;
    }
    
    // Read data multiple times
    for (i = 0; i < NUM_ITERATIONS; i++) {
        int read_count = fread(buffer, 1, BUFFER_SIZE, file);
        bytes_read += read_count;
        if (read_count != BUFFER_SIZE) {
            if (feof(file)) {
                // Seek back to beginning
                fseek(file, 0, 0);
                read_count = fread(buffer, 1, BUFFER_SIZE, file);
                bytes_read += read_count;
            } else {
                printf("Warning: Read %d only read %d bytes\n", i, read_count);
            }
        }
    }
    
    fclose(file);
    
    // End timing
    end_time = time();
    elapsed = end_time - start_time;
    
    if (elapsed > 0) {
        printf("  Read %d bytes in %d ticks\n", bytes_read, elapsed);
        printf("  Throughput: %d bytes/tick\n", bytes_read / elapsed);
    } else {
        printf("  Read %d bytes (too fast to measure)\n", bytes_read);
    }
    
    return bytes_read;
}

int test_random_access() {
    FILE *file;
    char buffer[64]; // Smaller buffer for random access
    int start_time, end_time, elapsed;
    int i, operations = 0;
    
    printf("Random Access Test:\n");
    
    // Start timing
    start_time = time();
    
    // Open file for read/write
    file = fopen(TEST_FILE, "r+b");
    if (!file) {
        printf("Error: Cannot open test file for random access!\n");
        return 0;
    }
    
    // Perform random seek and read operations
    for (i = 0; i < NUM_ITERATIONS; i++) {
        // Seek to different positions
        int position = (i * 127) % (BUFFER_SIZE * NUM_ITERATIONS);
        fseek(file, position, 0);
        
        // Read some data
        int read_count = fread(buffer, 1, 64, file);
        if (read_count > 0) operations++;
        
        // Seek to another position and write
        position = ((i + 5) * 89) % (BUFFER_SIZE * NUM_ITERATIONS);
        fseek(file, position, 0);
        fill_pattern(buffer, 64, 0x55);
        int written = fwrite(buffer, 1, 64, file);
        if (written > 0) operations++;
    }
    
    fclose(file);
    
    // End timing
    end_time = time();
    elapsed = end_time - start_time;
    
    if (elapsed > 0) {
        printf("  Performed %d operations in %d ticks\n", operations, elapsed);
        printf("  Operations/tick: %d\n", operations / elapsed);
    } else {
        printf("  Performed %d operations (too fast to measure)\n", operations);
    }
    
    return operations;
}

void test_file_operations() {
    int start_time, end_time, elapsed;
    int i;
    
    printf("File Create/Delete Test:\n");
    
    start_time = time();
    
    // Test file creation and deletion
    for (i = 0; i < 5; i++) {
        char filename[32];
        FILE *file;
        
        // Create filename
        sprintf(filename, "temp%d.tmp", i);
        
        // Create file
        file = fopen(filename, "wb");
        if (file) {
            fwrite("test", 1, 4, file);
            fclose(file);
            
            // Delete file
            remove(filename);
        }
    }
    
    end_time = time();
    elapsed = end_time - start_time;
    
    if (elapsed > 0) {
        printf("  Created/deleted 5 files in %d ticks\n", elapsed);
    } else {
        printf("  Created/deleted 5 files (too fast to measure)\n");
    }
}

void cleanup_test_files() {
    printf("Cleaning up test files...\n");
    remove(TEST_FILE);
}

int main() {
    clrscr();
    print_header();
    
    printf("Starting disk I/O performance tests...\n\n");
    
    // Run write test
    if (test_sequential_write() > 0) {
        printf("  Write test completed successfully.\n\n");
    } else {
        printf("  Write test failed!\n\n");
        return 1;
    }
    
    // Run read test
    if (test_sequential_read() > 0) {
        printf("  Read test completed successfully.\n\n");
    } else {
        printf("  Read test failed!\n\n");
    }
    
    // Run random access test
    test_random_access();
    printf("  Random access test completed.\n\n");
    
    // Run file operations test
    test_file_operations();
    printf("  File operations test completed.\n\n");
    
    // Cleanup
    cleanup_test_files();
    
    print_separator();
    printf("All tests completed!\n");
    printf("Lower times indicate better performance.\n");
    print_separator();
    
    printf("\nPress any key to exit...");
    getch();
    
    return 0;
}
