/*
   Serial Port Driver Implementation for ICS-OS
   Provides UART communication for kernel logging and debugging
   
   Copyright (C) 2025 ICS-OS Project
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include "serial.h"
#include "../chips/ports.h"

// Global serial port configurations
static serial_port_t serial_ports[4] = {
    {SERIAL_PORT_COM1, SERIAL_BAUD_9600, 8, 1, SERIAL_LCR_PARITY_NONE, 0},
    {SERIAL_PORT_COM2, SERIAL_BAUD_9600, 8, 1, SERIAL_LCR_PARITY_NONE, 0},
    {SERIAL_PORT_COM3, SERIAL_BAUD_9600, 8, 1, SERIAL_LCR_PARITY_NONE, 0},
    {SERIAL_PORT_COM4, SERIAL_BAUD_9600, 8, 1, SERIAL_LCR_PARITY_NONE, 0}
};

// Default port for kernel logging (COM1)
static WORD default_log_port = SERIAL_PORT_COM1;

/*
 * Get serial port index from port address
 */
static int get_port_index(WORD port) {
    switch (port) {
        case SERIAL_PORT_COM1: return 0;
        case SERIAL_PORT_COM2: return 1;
        case SERIAL_PORT_COM3: return 2;
        case SERIAL_PORT_COM4: return 3;
        default: return -1;
    }
}

/*
 * Check if transmitter is ready for next character
 */
static int serial_transmit_ready(WORD port) {
    return inportb(port + SERIAL_LINE_STATUS_REG) & SERIAL_LSR_TRANS_EMPTY;
}

/*
 * Check if data is available for reading
 */
int serial_data_available(WORD port) {
    return inportb(port + SERIAL_LINE_STATUS_REG) & SERIAL_LSR_DATA_READY;
}

/*
 * Initialize a serial port with specified baud rate
 */
int serial_init(WORD port, WORD baud_divisor) {
    int port_idx = get_port_index(port);
    if (port_idx < 0) {
        return -1; // Invalid port
    }
    
    // Disable all interrupts
    outportb(port + SERIAL_INTR_ENABLE_REG, 0x00);
    
    // Enable DLAB (set baud rate divisor)
    outportb(port + SERIAL_LINE_CTRL_REG, SERIAL_LCR_DLAB);
    
    // Set divisor (lo byte)
    outportb(port + SERIAL_DIVISOR_LOW, baud_divisor & 0xFF);
    
    // Set divisor (hi byte)
    outportb(port + SERIAL_DIVISOR_HIGH, (baud_divisor >> 8) & 0xFF);
    
    // 8 bits, no parity, one stop bit, clear DLAB
    outportb(port + SERIAL_LINE_CTRL_REG, SERIAL_LCR_8_BITS | SERIAL_LCR_PARITY_NONE | SERIAL_LCR_STOP_1);
    
    // Enable FIFO, clear them, with 14-byte threshold
    outportb(port + SERIAL_INTR_ID_REG, 0xC7);
    
    // IRQs enabled, RTS/DSR set
    outportb(port + SERIAL_MODEM_CTRL_REG, SERIAL_MCR_DTR | SERIAL_MCR_RTS | SERIAL_MCR_OUT2);
    
    // Test serial chip (loopback mode)
    outportb(port + SERIAL_MODEM_CTRL_REG, SERIAL_MCR_DTR | SERIAL_MCR_RTS | SERIAL_MCR_OUT1 | SERIAL_MCR_OUT2 | SERIAL_MCR_LOOPBACK);
    
    // Test by sending byte 0xAE and checking if we get it back
    outportb(port + SERIAL_DATA_REG, 0xAE);
    
    // Check if serial is faulty (i.e: not same byte as sent)
    if (inportb(port + SERIAL_DATA_REG) != 0xAE) {
        return -2; // Faulty serial port
    }
    
    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outportb(port + SERIAL_MODEM_CTRL_REG, SERIAL_MCR_DTR | SERIAL_MCR_RTS | SERIAL_MCR_OUT2);
    
    // Mark as initialized
    serial_ports[port_idx].initialized = 1;
    serial_ports[port_idx].baud_divisor = baud_divisor;
    
    return 0; // Success
}

/*
 * Check if a serial port is initialized
 */
int serial_is_initialized(WORD port) {
    int port_idx = get_port_index(port);
    if (port_idx < 0) {
        return 0;
    }
    return serial_ports[port_idx].initialized;
}

/*
 * Send a character to serial port
 */
void serial_putc(WORD port, char c) {
    if (!serial_is_initialized(port)) {
        return; // Port not initialized
    }
    
    // Wait for transmitter to be ready
    while (!serial_transmit_ready(port)) {
        // Busy wait - in a real system, we might want a timeout
    }
    
    // Send the character
    outportb(port + SERIAL_DATA_REG, c);
}

/*
 * Send a string to serial port
 */
void serial_puts(WORD port, const char* str) {
    if (!str) return;
    
    while (*str) {
        // Convert line feeds to CRLF for proper terminal display
        if (*str == '\n') {
            serial_putc(port, '\r');
        }
        serial_putc(port, *str);
        str++;
    }
}

/*
 * Read a character from serial port (non-blocking)
 * Returns -1 if no data available
 */
int serial_getc(WORD port) {
    if (!serial_is_initialized(port)) {
        return -1;
    }
    
    if (!serial_data_available(port)) {
        return -1; // No data available
    }
    
    return inportb(port + SERIAL_DATA_REG);
}

/*
 * Flush serial port output buffer
 */
void serial_flush(WORD port) {
    if (!serial_is_initialized(port)) {
        return;
    }
    
    // Wait for transmitter to be empty
    while (!(inportb(port + SERIAL_LINE_STATUS_REG) & SERIAL_LSR_TRANS_SHIFT)) {
        // Busy wait
    }
}

/*
 * Initialize default serial port for kernel logging
 */
int serial_log_init(void) {
    // Try to initialize COM1 at 9600 baud for QEMU compatibility
    // QEMU can redirect serial output with: -serial stdio or -serial file:log.txt
    return serial_init(default_log_port, SERIAL_BAUD_9600);
}

/*
 * Send character to default log port
 */
void serial_log_putc(char c) {
    serial_putc(default_log_port, c);
}

/*
 * Send string to default log port
 */
void serial_log_puts(const char* str) {
    serial_puts(default_log_port, str);
}

/*
 * Printf-style formatted output to serial log
 */
void serial_log_printf(const char* fmt, ...) {
    // We'll implement this after we create the kernel logger
    // For now, just output the format string
    serial_log_puts(fmt);
}
