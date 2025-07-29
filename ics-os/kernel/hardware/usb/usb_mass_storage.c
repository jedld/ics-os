/*
  Name: usb_mass_storage.c
  Copyright: 
  Author: ICS-OS Development Team
  Date: 29/07/25
  Description: USB Mass Storage driver for ICS-OS using Bulk-Only Transport
  
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
#include "../../stdlib/dexstdlib.h"

#ifndef NULL
#define NULL 0
#endif

// Global mass storage device array
extern usb_device_t *mass_storage_devices[MAX_USB_DEVICES];
extern int num_mass_storage;

// Initialize USB mass storage device
int usb_mass_storage_init(usb_device_t *device) {
    if (!device) return -1;
    
    printf("Initializing USB Mass Storage device...\n");
    
    // Allocate mass storage info
    device->mass_storage_info = malloc(sizeof(usb_mass_storage_info_t));
    if (!device->mass_storage_info) {
        return -1;
    }
    
    usb_mass_storage_info_t *ms = device->mass_storage_info;
    memset(ms, 0, sizeof(usb_mass_storage_info_t));
    
    // Set default endpoints (would normally be read from descriptors)
    ms->bulk_in_endpoint = 0x81;   // IN endpoint 1
    ms->bulk_out_endpoint = 0x02;  // OUT endpoint 2
    ms->tag_counter = 1;
    
    // Try to reset the mass storage device
    if (usb_mass_storage_reset(device) != 0) {
        printf("Failed to reset mass storage device\n");
        free(device->mass_storage_info);
        device->mass_storage_info = NULL;
        return -1;
    }
    
    // Test basic SCSI commands
    if (usb_mass_storage_test_unit_ready(device) == 0) {
        printf("Mass storage device is ready\n");
        
        // Try to read capacity
        if (usb_mass_storage_read_capacity(device) == 0) {
            printf("Device capacity: %u blocks, %u bytes per block\n",
                   ms->num_blocks, ms->block_size);
        }
    }
    
    return 0;
}

// Register mass storage device in global array
int usb_register_mass_storage_device(usb_device_t *device) {
    if (!device || num_mass_storage >= 8) {
        return -1;
    }
    
    mass_storage_devices[num_mass_storage] = device;
    device->mass_storage_index = num_mass_storage;
    num_mass_storage++;
    
    printf("Registered USB mass storage device #%d\n", device->mass_storage_index);
    return 0;
}

// USB Mass Storage Reset
int usb_mass_storage_reset(usb_device_t *device) {
    if (!device || !device->mass_storage_info) return -1;
    
    // Send Bulk-Only Mass Storage Reset
    return usb_control_transfer(device, 0x21, 0xFF, 0, 0, NULL, 0);
}

// Test Unit Ready command
int usb_mass_storage_test_unit_ready(usb_device_t *device) {
    if (!device || !device->mass_storage_info) return -1;
    
    usb_mass_storage_info_t *ms = device->mass_storage_info;
    
    // Create Command Block Wrapper
    usb_cbw_t cbw;
    memset(&cbw, 0, sizeof(cbw));
    cbw.signature = CBW_SIGNATURE;
    cbw.tag = ms->tag_counter++;
    cbw.data_transfer_length = 0;
    cbw.flags = 0;  // No data transfer
    cbw.lun = 0;
    cbw.cb_length = 6;
    
    // SCSI TEST UNIT READY command
    cbw.cb[0] = SCSI_TEST_UNIT_READY;
    
    // Send CBW
    if (usb_bulk_transfer(device, ms->bulk_out_endpoint, &cbw, sizeof(cbw)) != 0) {
        return -1;
    }
    
    // Receive CSW
    usb_csw_t csw;
    if (usb_bulk_transfer(device, ms->bulk_in_endpoint, &csw, sizeof(csw)) != 0) {
        return -1;
    }
    
    if (csw.signature != CSW_SIGNATURE || csw.tag != cbw.tag) {
        return -1;
    }
    
    return (csw.status == 0) ? 0 : -1;
}

// Read Capacity command
int usb_mass_storage_read_capacity(usb_device_t *device) {
    if (!device || !device->mass_storage_info) return -1;
    
    usb_mass_storage_info_t *ms = device->mass_storage_info;
    
    // Create Command Block Wrapper
    usb_cbw_t cbw;
    memset(&cbw, 0, sizeof(cbw));
    cbw.signature = CBW_SIGNATURE;
    cbw.tag = ms->tag_counter++;
    cbw.data_transfer_length = 8;  // Read Capacity returns 8 bytes
    cbw.flags = 0x80;  // Data from device to host
    cbw.lun = 0;
    cbw.cb_length = 10;
    
    // SCSI READ CAPACITY(10) command
    cbw.cb[0] = SCSI_READ_CAPACITY_10;
    
    // Send CBW
    if (usb_bulk_transfer(device, ms->bulk_out_endpoint, &cbw, sizeof(cbw)) != 0) {
        return -1;
    }
    
    // Receive data
    BYTE capacity_data[8];
    if (usb_bulk_transfer(device, ms->bulk_in_endpoint, capacity_data, 8) != 0) {
        return -1;
    }
    
    // Parse capacity data (big-endian)
    ms->num_blocks = (capacity_data[0] << 24) | (capacity_data[1] << 16) | 
                     (capacity_data[2] << 8) | capacity_data[3];
    ms->block_size = (capacity_data[4] << 24) | (capacity_data[5] << 16) | 
                     (capacity_data[6] << 8) | capacity_data[7];
    
    // Receive CSW
    usb_csw_t csw;
    if (usb_bulk_transfer(device, ms->bulk_in_endpoint, &csw, sizeof(csw)) != 0) {
        return -1;
    }
    
    if (csw.signature != CSW_SIGNATURE || csw.tag != cbw.tag) {
        return -1;
    }
    
    return (csw.status == 0) ? 0 : -1;
}

// Read sectors from USB mass storage device
int usb_mass_storage_read_sectors(usb_device_t *device, DWORD lba, WORD sectors, void *buffer) {
    if (!device || !device->mass_storage_info || !buffer || sectors == 0) {
        return -1;
    }
    
    usb_mass_storage_info_t *ms = device->mass_storage_info;
    DWORD transfer_length = sectors * ms->block_size;
    
    // Create Command Block Wrapper
    usb_cbw_t cbw;
    memset(&cbw, 0, sizeof(cbw));
    cbw.signature = CBW_SIGNATURE;
    cbw.tag = ms->tag_counter++;
    cbw.data_transfer_length = transfer_length;
    cbw.flags = 0x80;  // Data from device to host
    cbw.lun = 0;
    cbw.cb_length = 10;
    
    // SCSI READ(10) command
    cbw.cb[0] = SCSI_READ_10;
    cbw.cb[2] = (lba >> 24) & 0xFF;
    cbw.cb[3] = (lba >> 16) & 0xFF;
    cbw.cb[4] = (lba >> 8) & 0xFF;
    cbw.cb[5] = lba & 0xFF;
    cbw.cb[7] = (sectors >> 8) & 0xFF;
    cbw.cb[8] = sectors & 0xFF;
    
    // Send CBW
    if (usb_bulk_transfer(device, ms->bulk_out_endpoint, &cbw, sizeof(cbw)) != 0) {
        return -1;
    }
    
    // Receive data
    if (usb_bulk_transfer(device, ms->bulk_in_endpoint, buffer, transfer_length) != 0) {
        return -1;
    }
    
    // Receive CSW
    usb_csw_t csw;
    if (usb_bulk_transfer(device, ms->bulk_in_endpoint, &csw, sizeof(csw)) != 0) {
        return -1;
    }
    
    if (csw.signature != CSW_SIGNATURE || csw.tag != cbw.tag) {
        return -1;
    }
    
    return (csw.status == 0) ? 0 : -1;
}

// Write sectors to USB mass storage device
int usb_mass_storage_write_sectors(usb_device_t *device, DWORD lba, WORD sectors, void *buffer) {
    if (!device || !device->mass_storage_info || !buffer || sectors == 0) {
        return -1;
    }
    
    usb_mass_storage_info_t *ms = device->mass_storage_info;
    DWORD transfer_length = sectors * ms->block_size;
    
    // Create Command Block Wrapper
    usb_cbw_t cbw;
    memset(&cbw, 0, sizeof(cbw));
    cbw.signature = CBW_SIGNATURE;
    cbw.tag = ms->tag_counter++;
    cbw.data_transfer_length = transfer_length;
    cbw.flags = 0x00;  // Data from host to device
    cbw.lun = 0;
    cbw.cb_length = 10;
    
    // SCSI WRITE(10) command
    cbw.cb[0] = SCSI_WRITE_10;
    cbw.cb[2] = (lba >> 24) & 0xFF;
    cbw.cb[3] = (lba >> 16) & 0xFF;
    cbw.cb[4] = (lba >> 8) & 0xFF;
    cbw.cb[5] = lba & 0xFF;
    cbw.cb[7] = (sectors >> 8) & 0xFF;
    cbw.cb[8] = sectors & 0xFF;
    
    // Send CBW
    if (usb_bulk_transfer(device, ms->bulk_out_endpoint, &cbw, sizeof(cbw)) != 0) {
        return -1;
    }
    
    // Send data
    if (usb_bulk_transfer(device, ms->bulk_out_endpoint, buffer, transfer_length) != 0) {
        return -1;
    }
    
    // Receive CSW
    usb_csw_t csw;
    if (usb_bulk_transfer(device, ms->bulk_in_endpoint, &csw, sizeof(csw)) != 0) {
        return -1;
    }
    
    if (csw.signature != CSW_SIGNATURE || csw.tag != cbw.tag) {
        return -1;
    }
    
    return (csw.status == 0) ? 0 : -1;
}

// Block device interface functions for VFS integration
int usb_block_read(int device_id, DWORD sector, WORD count, void *buffer) {
    if (device_id >= num_mass_storage || !mass_storage_devices[device_id]) {
        return -1;
    }
    
    return usb_mass_storage_read_sectors(mass_storage_devices[device_id], sector, count, buffer);
}

int usb_block_write(int device_id, DWORD sector, WORD count, void *buffer) {
    if (device_id >= num_mass_storage || !mass_storage_devices[device_id]) {
        return -1;
    }
    
    return usb_mass_storage_write_sectors(mass_storage_devices[device_id], sector, count, buffer);
}

int usb_block_get_info(int device_id, DWORD *total_sectors, DWORD *sector_size) {
    if (device_id >= num_mass_storage || !mass_storage_devices[device_id]) {
        return -1;
    }
    
    usb_device_t *device = mass_storage_devices[device_id];
    if (!device->mass_storage_info) return -1;
    
    usb_mass_storage_info_t *ms = (usb_mass_storage_info_t*)device->mass_storage_info;
    *total_sectors = ms->num_blocks;
    *sector_size = ms->block_size;
    
    return 0;
}
