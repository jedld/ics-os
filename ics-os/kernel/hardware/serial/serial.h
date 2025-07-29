#ifndef __SERIAL_H__
#define __SERIAL_H__

/*
   Serial Port Driver for ICS-OS
   Provides UART communication for kernel logging and debugging
   
   Copyright (C) 2025 ICS-OS Project
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include "../../dextypes.h"

// Standard PC serial port addresses
#define SERIAL_PORT_COM1    0x3F8
#define SERIAL_PORT_COM2    0x2F8
#define SERIAL_PORT_COM3    0x3E8
#define SERIAL_PORT_COM4    0x2E8

// Serial port register offsets
#define SERIAL_DATA_REG         0   // Data register (R/W)
#define SERIAL_INTR_ENABLE_REG  1   // Interrupt enable register
#define SERIAL_INTR_ID_REG      2   // Interrupt identification register
#define SERIAL_LINE_CTRL_REG    3   // Line control register
#define SERIAL_MODEM_CTRL_REG   4   // Modem control register
#define SERIAL_LINE_STATUS_REG  5   // Line status register
#define SERIAL_MODEM_STATUS_REG 6   // Modem status register
#define SERIAL_SCRATCH_REG      7   // Scratch register

// Divisor latch registers (when DLAB bit is set in Line Control Register)
#define SERIAL_DIVISOR_LOW      0   // Divisor latch low byte
#define SERIAL_DIVISOR_HIGH     1   // Divisor latch high byte

// Line Control Register bits
#define SERIAL_LCR_DLAB         0x80  // Divisor latch access bit
#define SERIAL_LCR_BREAK        0x40  // Break enable
#define SERIAL_LCR_PARITY_EVEN  0x18  // Even parity
#define SERIAL_LCR_PARITY_ODD   0x08  // Odd parity
#define SERIAL_LCR_PARITY_NONE  0x00  // No parity
#define SERIAL_LCR_STOP_1       0x00  // 1 stop bit
#define SERIAL_LCR_STOP_2       0x04  // 2 stop bits
#define SERIAL_LCR_8_BITS       0x03  // 8 data bits
#define SERIAL_LCR_7_BITS       0x02  // 7 data bits
#define SERIAL_LCR_6_BITS       0x01  // 6 data bits
#define SERIAL_LCR_5_BITS       0x00  // 5 data bits

// Line Status Register bits
#define SERIAL_LSR_DATA_READY   0x01  // Data ready
#define SERIAL_LSR_OVERRUN_ERR  0x02  // Overrun error
#define SERIAL_LSR_PARITY_ERR   0x04  // Parity error
#define SERIAL_LSR_FRAMING_ERR  0x08  // Framing error
#define SERIAL_LSR_BREAK_INT    0x10  // Break interrupt
#define SERIAL_LSR_TRANS_EMPTY  0x20  // Transmitter holding register empty
#define SERIAL_LSR_TRANS_SHIFT  0x40  // Transmitter empty
#define SERIAL_LSR_FIFO_ERR     0x80  // Impending error

// Modem Control Register bits
#define SERIAL_MCR_DTR          0x01  // Data terminal ready
#define SERIAL_MCR_RTS          0x02  // Request to send
#define SERIAL_MCR_OUT1         0x04  // Output 1
#define SERIAL_MCR_OUT2         0x08  // Output 2
#define SERIAL_MCR_LOOPBACK     0x10  // Loopback mode

// Interrupt Enable Register bits
#define SERIAL_IER_DATA_AVAIL   0x01  // Data available interrupt
#define SERIAL_IER_TRANS_EMPTY  0x02  // Transmitter empty interrupt
#define SERIAL_IER_LINE_STATUS  0x04  // Line status interrupt
#define SERIAL_IER_MODEM_STATUS 0x08  // Modem status interrupt

// Baud rate divisors for 115200 base rate
#define SERIAL_BAUD_115200      1
#define SERIAL_BAUD_57600       2
#define SERIAL_BAUD_38400       3
#define SERIAL_BAUD_19200       6
#define SERIAL_BAUD_9600        12
#define SERIAL_BAUD_4800        24
#define SERIAL_BAUD_2400        48
#define SERIAL_BAUD_1200        96

// Serial port configuration
typedef struct {
    WORD base_port;        // Base port address
    WORD baud_divisor;     // Baud rate divisor
    BYTE data_bits;        // Number of data bits (5-8)
    BYTE stop_bits;        // Number of stop bits (1-2)
    BYTE parity;           // Parity setting
    BYTE initialized;      // Initialization flag
} serial_port_t;

// Function prototypes
int serial_init(WORD port, WORD baud_divisor);
int serial_is_initialized(WORD port);
void serial_putc(WORD port, char c);
void serial_puts(WORD port, const char* str);
int serial_getc(WORD port);
int serial_data_available(WORD port);
void serial_flush(WORD port);

// High-level functions for kernel logging
int serial_log_init(void);
void serial_log_putc(char c);
void serial_log_puts(const char* str);
void serial_log_printf(const char* fmt, ...);

#endif /* __SERIAL_H__ */
