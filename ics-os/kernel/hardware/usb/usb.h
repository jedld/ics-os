/*
  Name: usb.h
  Copyright: 
  Author: ICS-OS Development Team
  Date: 29/07/25
  Description: USB driver framework header file for ICS-OS
  
    ICS educational extensible operating system
    Copyright (C) 2025  ICS-OS Development Team

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#ifndef USB_H
#define USB_H

/* USB Driver Configuration Constants */
#define MAX_USB_CONTROLLERS 8
#define MAX_USB_DEVICES 64

#include "../../dextypes.h"
#include "../../devmgr/dex32_devmgr.h"

// USB specification constants
#define USB_ENDPOINT_IN         0x80
#define USB_ENDPOINT_OUT        0x00

// USB request types
#define USB_REQUEST_TYPE_STANDARD   0x00
#define USB_REQUEST_TYPE_CLASS      0x20
#define USB_REQUEST_TYPE_VENDOR     0x40

// USB standard requests
#define USB_REQUEST_GET_STATUS      0x00
#define USB_REQUEST_CLEAR_FEATURE   0x01
#define USB_REQUEST_SET_FEATURE     0x03
#define USB_REQUEST_SET_ADDRESS     0x05
#define USB_REQUEST_GET_DESCRIPTOR  0x06
#define USB_REQUEST_SET_DESCRIPTOR  0x07
#define USB_REQUEST_GET_CONFIG      0x08
#define USB_REQUEST_SET_CONFIG      0x09

// USB descriptor types
#define USB_DESCRIPTOR_DEVICE       0x01
#define USB_DESCRIPTOR_CONFIG       0x02
#define USB_DESCRIPTOR_STRING       0x03
#define USB_DESCRIPTOR_INTERFACE    0x04
#define USB_DESCRIPTOR_ENDPOINT     0x05

// USB device classes
#define USB_CLASS_HUB               0x09
#define USB_CLASS_MASS_STORAGE      0x08

// USB Mass Storage subclasses
#define USB_SUBCLASS_SCSI           0x06

// USB Mass Storage protocols
#define USB_PROTOCOL_BULK_ONLY      0x50

// UHCI/OHCI controller constants
#define UHCI_USBCMD         0x00
#define UHCI_USBSTS         0x02
#define UHCI_USBINTR        0x04
#define UHCI_FRNUM          0x06
#define UHCI_FLBASEADD      0x08
#define UHCI_SOFMOD         0x0C
#define UHCI_PORTSC1        0x10
#define UHCI_PORTSC2        0x12

// USB device speeds
#define USB_SPEED_LOW       0
#define USB_SPEED_FULL      1
#define USB_SPEED_HIGH      2

// USB transfer types
#define USB_TRANSFER_CONTROL    0
#define USB_TRANSFER_BULK       1
#define USB_TRANSFER_INTERRUPT  2
#define USB_TRANSFER_ISOCHRONOUS 3

// USB Mass Storage SCSI commands
#define SCSI_INQUIRY            0x12
#define SCSI_READ_CAPACITY      0x25
#define SCSI_READ_CAPACITY_10   0x25
#define SCSI_READ_10            0x28
#define SCSI_WRITE_10           0x2A
#define SCSI_TEST_UNIT_READY    0x00

// USB Mass Storage signatures
#define CBW_SIGNATURE           0x43425355  // "USBC"
#define CSW_SIGNATURE           0x53425355  // "USBS"

// Command Block Wrapper (CBW) for USB Mass Storage
typedef struct __attribute__((packed)) {
    DWORD signature;        // 0x43425355 "USBC"
    DWORD tag;
    DWORD data_transfer_length;
    BYTE flags;
    BYTE lun;
    BYTE cb_length;
    BYTE cb[16];
} usb_cbw_t;

// Command Status Wrapper (CSW) for USB Mass Storage  
typedef struct __attribute__((packed)) {
    DWORD signature;        // 0x53425355 "USBS"
    DWORD tag;
    DWORD data_residue;
    BYTE status;
} usb_csw_t;

// USB Mass Storage device information
typedef struct {
    BYTE bulk_in_endpoint;
    BYTE bulk_out_endpoint;
    DWORD tag_counter;
    DWORD num_blocks;
    DWORD block_size;
} usb_mass_storage_info_t;

// USB device descriptor
typedef struct __attribute__((packed)) {
    BYTE length;
    BYTE descriptor_type;
    WORD usb_version;
    BYTE device_class;
    BYTE device_subclass;
    BYTE device_protocol;
    BYTE max_packet_size;
    WORD vendor_id;
    WORD product_id;
    WORD device_version;
    BYTE manufacturer_string;
    BYTE product_string;
    BYTE serial_string;
    BYTE num_configurations;
} usb_device_descriptor_t;

// USB configuration descriptor
typedef struct __attribute__((packed)) {
    BYTE length;
    BYTE descriptor_type;
    WORD total_length;
    BYTE num_interfaces;
    BYTE configuration_value;
    BYTE configuration_string;
    BYTE attributes;
    BYTE max_power;
} usb_config_descriptor_t;

// USB interface descriptor
typedef struct __attribute__((packed)) {
    BYTE length;
    BYTE descriptor_type;
    BYTE interface_number;
    BYTE alternate_setting;
    BYTE num_endpoints;
    BYTE interface_class;
    BYTE interface_subclass;
    BYTE interface_protocol;
    BYTE interface_string;
} usb_interface_descriptor_t;

// USB endpoint descriptor
typedef struct __attribute__((packed)) {
    BYTE length;
    BYTE descriptor_type;
    BYTE endpoint_address;
    BYTE attributes;
    WORD max_packet_size;
    BYTE interval;
} usb_endpoint_descriptor_t;

// USB device structure
typedef struct {
    int device_id;
    BYTE address;
    BYTE speed;
    usb_device_descriptor_t descriptor;
    void *controller;
    BYTE in_endpoint;
    BYTE out_endpoint;
    WORD max_packet_size;
    int is_mass_storage;
    void *mass_storage_info;    // Pointer to USB mass storage info
    int mass_storage_index;     // Index in mass storage device array
    int block_device_id;        // Device manager block device ID
} usb_device_t;

// USB controller structure
typedef struct {
    int controller_id;
    int type;               // 0=UHCI, 1=OHCI, 2=EHCI
    DWORD base_address;
    int num_ports;
    usb_device_t *devices[16];  // Max 16 devices per controller
} usb_controller_t;

// Function prototypes
int usb_init(void);
int usb_scan_controllers(void);
int usb_reset_controller(usb_controller_t *controller);
int usb_detect_devices(usb_controller_t *controller);
int usb_setup_device(usb_controller_t *controller, int port);
int usb_control_transfer(usb_device_t *device, BYTE request_type, BYTE request, 
                        WORD value, WORD index, void *data, WORD length);
int usb_bulk_transfer(usb_device_t *device, BYTE endpoint, void *data, DWORD length);

// USB Mass Storage functions
int usb_mass_storage_init(usb_device_t *device);
int usb_mass_storage_read_sectors(usb_device_t *device, DWORD lba, WORD sectors, void *buffer);
int usb_mass_storage_write_sectors(usb_device_t *device, DWORD lba, WORD sectors, void *buffer);
int usb_mass_storage_read(usb_device_t *device, DWORD lba, DWORD count, void *buffer);
int usb_mass_storage_write(usb_device_t *device, DWORD lba, DWORD count, void *buffer);
int usb_mass_storage_get_capacity(usb_device_t *device, DWORD *blocks, DWORD *block_size);
int usb_mass_storage_test_unit_ready(usb_device_t *device);
int usb_mass_storage_read_capacity(usb_device_t *device);
int usb_mass_storage_reset(usb_device_t *device);

// Device registration
int usb_register_mass_storage_device(usb_device_t *device);
int usb_register_block_device(usb_device_t *device);

// Universal block device functions for device manager
int usb_uni_read_block(int block, char *blockbuff, DWORD numblocks);
int usb_uni_write_block(int block, char *blockbuff, DWORD numblocks);
int usb_uni_get_block_size(void);
int usb_uni_get_total_blocks(void);
int usb_sendmessage(int type, int message);

// USB device query functions
void usb_list_controllers(void);
void usb_list_devices(void);

#endif // USB_H
