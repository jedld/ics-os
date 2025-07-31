# ICS-OS Disk I/O Performance Test

## Overview

The `disktest.exe` utility is designed to measure and evaluate the performance of the floppy disk drive in ICS-OS. It performs various I/O operations and reports timing information to help assess the effectiveness of floppy driver optimizations.

## Features

- **Sequential Write Test**: Measures write throughput by writing multiple 512-byte blocks
- **Sequential Read Test**: Measures read throughput by reading the same data back
- **Random Access Test**: Tests random seek and read/write operations
- **File Operations Test**: Measures file creation and deletion performance
- **Simple Results**: Reports operations per time tick for easy comparison

## Usage

1. Boot ICS-OS with the updated floppy image
2. At the command prompt, type: `disktest`
3. The program will automatically run all tests and display results
4. Press any key to exit when tests complete

## Test Details

- **Buffer Size**: 512 bytes (1 floppy sector)
- **Iterations**: 10 operations per test
- **Test File**: Creates `testfile.dat` for testing (automatically cleaned up)
- **Timing**: Uses ICS-OS system timer ticks for measurement

## Interpreting Results

- **Lower tick counts** indicate faster performance
- **Higher bytes/tick** or **operations/tick** indicate better throughput
- Compare results before and after floppy driver optimizations
- "Too fast to measure" indicates the operation completed within one timer tick

## Expected Improvements

With the floppy driver optimizations implemented:
- Reduced motor start/stop delays
- Better timeout handling
- Improved seek operations
- More efficient sector caching

You should see improvements especially in:
- Sequential operations (due to reduced motor cycling)
- File operations (due to better timeout handling)
- Overall responsiveness

## Technical Notes

- Uses standard ICS-OS file I/O APIs (fopen, fread, fwrite, etc.)
- Times operations using the `time()` system call
- Automatically handles file cleanup
- Designed to work on floppy disk (fd0) device
