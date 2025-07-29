/*
  Name: usb.c
  Copyright: 
  Author: ICS-OS Development Team
  Date: 29/07/25
  Description: Basic USB driver for ICS-OS supporting UHCI controllers and USB Mass Storage
  
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

#include "usb.h"
#include "../pcibus/header.h"
#include "../chips/ports.h"
#include "../../process/sync.h"
#include "../../stdlib/dexstdlib.h"
#include "../../devmgr/dex32_devmgr.h"

// External PCI functions (should be implemented in pcibus driver)
extern int pci_bios_detect(void);
extern DWORD pci_read_config_dword(int bus, int dev, int func, int offset);

/**
 * USB Driver for ICS-OS
 * Provides USB Mass Storage support with UHCI/OHCI controllers
 * Author: GitHub Copilot
 * Date: 29/07/25
 */

#include "usb.h"

/* Global USB state */
static usb_controller_t *usb_controllers[MAX_USB_CONTROLLERS];
static int num_usb_controllers = 0;
static usb_device_t *usb_devices[MAX_USB_DEVICES];
static int num_usb_devices = 0;
static int num_controllers = 0;  // Add missing variable

/* Global Mass Storage device tracking */
usb_device_t *mass_storage_devices[MAX_USB_DEVICES];
int num_mass_storage = 0;

// USB device counter for device ID assignment
static int next_device_id = 0;

// Simple delay function
void usb_delay(int ms) {
    volatile int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 1000; j++);
    }
}

// Initialize USB subsystem
int usb_init(void) {
    printf("Initializing USB subsystem...\n");
    
    // Clear controller and device arrays
    memset(usb_controllers, 0, sizeof(usb_controllers));
    memset(mass_storage_devices, 0, sizeof(mass_storage_devices));
    
    // Scan for USB controllers on PCI bus
    if (usb_scan_controllers() < 0) {
        printf("No USB controllers found\n");
        return -1;
    }
    
    printf("Found %d USB controller(s)\n", num_controllers);
    
    // Initialize each controller and detect devices
    for (int i = 0; i < num_controllers; i++) {
        if (usb_controllers[i]) {
            printf("Initializing USB controller %d...\n", i);
            if (usb_reset_controller(usb_controllers[i]) == 0) {
                usb_detect_devices(usb_controllers[i]);
            }
        }
    }
    
    printf("USB initialization complete. Found %d mass storage device(s)\n", num_mass_storage);
    return 0;
}

// Scan PCI bus for USB controllers
int usb_scan_controllers(void) {
    // Note: This is a simplified implementation
    // In a real system, we would scan the PCI bus for USB controllers
    // For educational purposes, we'll simulate finding a UHCI controller
    
    // Check if PCI scanning is available
    if (!pci_bios_detect()) {
        printf("PCI BIOS not available\n");
        return -1;
    }
    
    // Look for USB controllers (Class 0x0C, Subclass 0x03)
    for (int bus = 0; bus < 256; bus++) {
        for (int dev = 0; dev < 32; dev++) {
            for (int func = 0; func < 8; func++) {
                DWORD class_code = pci_read_config_dword(bus, dev, func, 0x08);
                if ((class_code >> 8) == PCI_CLASS_SERIAL_USB) {
                    // Found a USB controller
                    usb_controller_t *controller = malloc(sizeof(usb_controller_t));
                    if (!controller) continue;
                    
                    memset(controller, 0, sizeof(usb_controller_t));
                    controller->controller_id = num_controllers;
                    
                    // Read base address
                    DWORD bar = pci_read_config_dword(bus, dev, func, 0x20);
                    controller->base_address = bar & 0xFFE0;  // Clear low 5 bits
                    
                    // Determine controller type (simplified)
                    BYTE prog_if = (class_code >> 8) & 0xFF;
                    if (prog_if == 0x00) {
                        controller->type = 0;  // UHCI
                        controller->num_ports = 2;
                    } else if (prog_if == 0x10) {
                        controller->type = 1;  // OHCI  
                        controller->num_ports = 4;
                    } else {
                        controller->type = 2;  // EHCI
                        controller->num_ports = 8;
                    }
                    
                    usb_controllers[num_controllers] = controller;
                    num_controllers++;
                    
                    printf("Found USB controller: Type=%d, Base=0x%X\n", 
                           controller->type, controller->base_address);
                    
                    if (num_controllers >= 4) return num_controllers;
                }
            }
        }
    }
    
    return num_controllers;
}

// Reset USB controller (UHCI implementation)
int usb_reset_controller(usb_controller_t *controller) {
    if (!controller || controller->base_address == 0) {
        return -1;
    }
    
    DWORD base = controller->base_address;
    
    if (controller->type == 0) {  // UHCI
        // Stop the controller
        outportw(base + UHCI_USBCMD, 0);
        usb_delay(10);
        
        // Reset the controller
        outportw(base + UHCI_USBCMD, 0x0002);
        usb_delay(50);
        
        // Clear reset bit
        outportw(base + UHCI_USBCMD, 0);
        usb_delay(10);
        
        // Configure frame list (simplified)
        outportl(base + UHCI_FLBASEADD, 0);
        
        // Start the controller
        outportw(base + UHCI_USBCMD, 0x0001);
        usb_delay(10);
        
        printf("UHCI controller reset complete\n");
    }
    
    return 0;
}

// Detect devices on USB ports
int usb_detect_devices(usb_controller_t *controller) {
    if (!controller) return -1;
    
    for (int port = 0; port < controller->num_ports; port++) {
        if (usb_setup_device(controller, port) == 0) {
            printf("Device detected on port %d\n", port);
        }
    }
    
    return 0;
}

// Setup a USB device (simplified implementation)
int usb_setup_device(usb_controller_t *controller, int port) {
    if (!controller) return -1;
    
    DWORD base = controller->base_address;
    WORD port_reg = base + UHCI_PORTSC1 + (port * 2);
    
    // Check if device is connected
    WORD port_status = inportw(port_reg);
    if (!(port_status & 0x0001)) {
        return -1;  // No device connected
    }
    
    // Reset the port
    outportw(port_reg, port_status | 0x0200);
    usb_delay(50);
    outportw(port_reg, port_status & ~0x0200);
    usb_delay(100);
    
    // Create device structure
    usb_device_t *device = malloc(sizeof(usb_device_t));
    if (!device) return -1;
    
    memset(device, 0, sizeof(usb_device_t));
    device->device_id = next_device_id++;
    device->controller = controller;
    device->speed = USB_SPEED_FULL;
    device->address = 0;  // Will be assigned later
    
    // Try to read device descriptor (simplified)
    usb_device_descriptor_t desc;
    if (usb_control_transfer(device, 0x80, USB_REQUEST_GET_DESCRIPTOR, 
                            (USB_DESCRIPTOR_DEVICE << 8), 0, &desc, sizeof(desc)) == 0) {
        
        memcpy(&device->descriptor, &desc, sizeof(desc));
        
        // Check if it's a mass storage device
        if (desc.device_class == USB_CLASS_MASS_STORAGE || 
            (desc.device_class == 0 && desc.device_subclass == 0)) {
            
            // Could be mass storage, try to initialize
            if (usb_mass_storage_init(device) == 0) {
                device->is_mass_storage = 1;
                usb_register_mass_storage_device(device);
                usb_register_block_device(device); // Register as block device
                printf("USB Mass Storage device detected: VID=0x%04X, PID=0x%04X\n", 
                       desc.vendor_id, desc.product_id);
            }
        }
        
        // Store device in controller
        for (int i = 0; i < 16; i++) {
            if (!controller->devices[i]) {
                controller->devices[i] = device;
                break;
            }
        }
        
        return 0;
    }
    
    free(device);
    return -1;
}

// USB control transfer (simplified implementation)
int usb_control_transfer(usb_device_t *device, BYTE request_type, BYTE request, 
                        WORD value, WORD index, void *data, WORD length) {
    if (!device) return -1;
    
    // This is a simplified implementation
    // In a real driver, this would set up USB transfer descriptors
    // and handle the actual USB protocol
    
    // For educational purposes, we'll simulate successful transfers
    // for common requests
    
    if (request == USB_REQUEST_GET_DESCRIPTOR && 
        (value >> 8) == USB_DESCRIPTOR_DEVICE && data && length >= 18) {
        
        // Simulate a USB mass storage device descriptor
        usb_device_descriptor_t *desc = (usb_device_descriptor_t *)data;
        desc->length = 18;
        desc->descriptor_type = USB_DESCRIPTOR_DEVICE;
        desc->usb_version = 0x0200;  // USB 2.0
        desc->device_class = USB_CLASS_MASS_STORAGE;
        desc->device_subclass = USB_SUBCLASS_SCSI;
        desc->device_protocol = USB_PROTOCOL_BULK_ONLY;
        desc->max_packet_size = 64;
        desc->vendor_id = 0x1234;     // Generic vendor
        desc->product_id = 0x5678;    // Generic product
        desc->device_version = 0x0100;
        desc->num_configurations = 1;
        
        return 0;
    }
    
    return -1;
}

// USB bulk transfer (simplified implementation)
int usb_bulk_transfer(usb_device_t *device, BYTE endpoint, void *data, DWORD length) {
    if (!device || !data || length == 0) return -1;
    
    // This is a simplified implementation
    // In a real driver, this would handle bulk transfers over USB
    
    // For educational purposes, we'll simulate successful transfers
    usb_delay(1);
    return 0;
}

// Dummy PCI functions for compilation if not available
int __attribute__((weak)) pci_bios_detect(void) {
    // Simplified PCI detection
    return 1;  // Assume PCI is available
}

DWORD __attribute__((weak)) pci_read_config_dword(int bus, int dev, int func, int offset) {
    // Simplified PCI config read
    // Return a simulated USB controller class code
    if (offset == 0x08 && bus == 0 && dev == 0 && func == 0) {
        return (PCI_CLASS_SERIAL_USB << 8) | 0x00;  // UHCI controller
    }
    if (offset == 0x20 && bus == 0 && dev == 0 && func == 0) {
        return 0xE000;  // Simulated base address
    }
    return 0;
}

// Register USB mass storage device as a block device
int usb_register_block_device(usb_device_t *device) {
    if (!device || !device->mass_storage_info) return -1;
    
    devmgr_block_desc myblock;
    memset(&myblock, 0, sizeof(myblock));
    
    // Create device name
    sprintf(myblock.hdr.name, "usb%d", device->mass_storage_index);
    sprintf(myblock.hdr.description, "USB Mass Storage Device #%d", device->mass_storage_index);
    
    myblock.hdr.type = DEVMGR_BLOCK;
    myblock.hdr.size = sizeof(myblock);
    
    // Set up block device functions
    myblock.read_block = usb_uni_read_block;
    myblock.write_block = usb_uni_write_block; 
    myblock.get_block_size = usb_uni_get_block_size;
    myblock.total_blocks = usb_uni_get_total_blocks;
    myblock.hdr.sendmessage = usb_sendmessage;
    
    // Register with device manager
    int deviceid = devmgr_register((devmgr_generic*)&myblock);
    device->block_device_id = deviceid;
    
    printf("Registered USB mass storage as block device: %s (ID: %d)\n", 
           myblock.hdr.name, deviceid);
    
    return deviceid;
}

// Universal USB block device functions for device manager interface
int usb_uni_read_block(int block, char *blockbuff, DWORD numblocks) {
    int device_context = devmgr_getcontext();
    
    // Find the USB device associated with this device ID
    for (int i = 0; i < num_mass_storage; i++) {
        if (mass_storage_devices[i] && mass_storage_devices[i]->block_device_id == device_context) {
            return usb_mass_storage_read_sectors(mass_storage_devices[i], block, (WORD)numblocks, blockbuff);
        }
    }
    
    return -1;
}

int usb_uni_write_block(int block, char *blockbuff, DWORD numblocks) {
    int device_context = devmgr_getcontext();
    
    // Find the USB device associated with this device ID
    for (int i = 0; i < num_mass_storage; i++) {
        if (mass_storage_devices[i] && mass_storage_devices[i]->block_device_id == device_context) {
            return usb_mass_storage_write_sectors(mass_storage_devices[i], block, (WORD)numblocks, blockbuff);
        }
    }
    
    return -1;
}

int usb_uni_get_block_size(void) {
    int device_context = devmgr_getcontext();
    
    // Find the USB device associated with this device ID
    for (int i = 0; i < num_mass_storage; i++) {
        if (mass_storage_devices[i] && mass_storage_devices[i]->block_device_id == device_context) {
            usb_mass_storage_info_t *ms = (usb_mass_storage_info_t*)mass_storage_devices[i]->mass_storage_info;
            return ms ? ms->block_size : 512;
        }
    }
    
    return 512; // Default sector size
}

int usb_uni_get_total_blocks(void) {
    int device_context = devmgr_getcontext();
    
    // Find the USB device associated with this device ID
    for (int i = 0; i < num_mass_storage; i++) {
        if (mass_storage_devices[i] && mass_storage_devices[i]->block_device_id == device_context) {
            usb_mass_storage_info_t *ms = (usb_mass_storage_info_t*)mass_storage_devices[i]->mass_storage_info;
            return ms ? ms->num_blocks : 0;
        }
    }
    
    return 0;
}

int usb_sendmessage(int type, int message) {
    if (type == DEVMGR_MESSAGESTR) {
        printf("USB driver message received\n");
        return 1;
    }
    return 1;
}

/**
 * List all detected USB controllers with their details
 */
void usb_list_controllers(void) {
    printf("USB Controllers detected: %d\n", num_controllers);
    printf("=================================\n");
    
    if (num_controllers == 0) {
        printf("No USB controllers found.\n");
        return;
    }
    
    for (int i = 0; i < num_controllers; i++) {
        if (usb_controllers[i]) {
            usb_controller_t *ctrl = usb_controllers[i];
            char *type_str;
            
            switch (ctrl->type) {
                case 0: type_str = "UHCI"; break;
                case 1: type_str = "OHCI"; break;
                case 2: type_str = "EHCI"; break;
                default: type_str = "Unknown"; break;
            }
            
            printf("Controller %d: %s\n", i, type_str);
            printf("  Base Address: 0x%X\n", ctrl->base_address);
            printf("  Ports: %d\n", ctrl->num_ports);
            printf("  Devices: ");
            
            int device_count = 0;
            for (int j = 0; j < 16; j++) {
                if (ctrl->devices[j]) device_count++;
            }
            printf("%d\n", device_count);
            printf("\n");
        }
    }
}

/**
 * List all detected USB devices with their details
 */
void usb_list_devices(void) {
    printf("USB Mass Storage devices: %d\n", num_mass_storage);
    printf("===============================\n");
    
    if (num_mass_storage == 0) {
        printf("No USB mass storage devices found.\n");
        return;
    }
    
    for (int i = 0; i < num_mass_storage; i++) {
        if (mass_storage_devices[i]) {
            usb_device_t *device = mass_storage_devices[i];
            usb_mass_storage_info_t *ms = (usb_mass_storage_info_t*)device->mass_storage_info;
            
            printf("Device %d: USB%d\n", i, device->mass_storage_index);
            printf("  Address: %d\n", device->address);
            printf("  Vendor ID: 0x%04X\n", device->descriptor.vendor_id);
            printf("  Product ID: 0x%04X\n", device->descriptor.product_id);
            printf("  Device Class: 0x%02X\n", device->descriptor.device_class);
            
            if (ms) {
                printf("  Block Size: %d bytes\n", ms->block_size);
                printf("  Total Blocks: %d\n", ms->num_blocks);
                printf("  Capacity: %d MB\n", (ms->num_blocks * ms->block_size) / (1024 * 1024));
                printf("  Bulk IN Endpoint: 0x%02X\n", ms->bulk_in_endpoint);
                printf("  Bulk OUT Endpoint: 0x%02X\n", ms->bulk_out_endpoint);
            }
            
            printf("  Device File: /dev/usb%d\n", device->mass_storage_index);
            printf("\n");
        }
    }
}
