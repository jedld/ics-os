/*
  Name: exFAT filesystem driver
  Copyright: 
  Author: Generated for ICS-OS exFAT support
  Date: 31/07/25
  Description: This is the implementation of the exFAT filesystem based on the
  "Microsoft exFAT File System Specification" version 1.00 released by 
  Microsoft Corporation.
  
    ICS-OS Educational Operating System
    Copyright (C) 2025

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
*/

#include "../dextypes.h"
#include "exfat.h"
#include "../devmgr/dex32_devmgr.h"
#include "../iomgr/iosched.h"
#include "../vfs/vfs_core.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

//#define DEBUG_EXFAT

int exfat_deviceid;

// Helper function to read boot sector
int exfat_read_boot_sector(exfat_boot_sector *boot_sector, int device_id)
{
    DWORD handle;
    
    #ifdef DEBUG_EXFAT
    printf("exFAT: Reading boot sector...\n");
    #endif
    
    handle = dex32_requestIO(device_id, IO_READ, 0, 1, boot_sector);
    while (!dex32_IOcomplete(handle));
    dex32_closeIO(handle);
    
    return 1;
}

// Helper function to validate exFAT boot sector
int exfat_validate_boot_sector(exfat_boot_sector *boot_sector)
{
    // Check filesystem name
    if (memcmp(boot_sector->fs_name, "EXFAT   ", 8) != 0) {
        return 0; // Not exFAT
    }
    
    // Check boot signature
    if (boot_sector->boot_signature != 0xAA55) {
        return 0;
    }
    
    // Check must-be-zero fields
    int i;
    for (i = 0; i < 53; i++) {
        if (boot_sector->must_be_zero[i] != 0) {
            return 0;
        }
    }
    
    return 1; // Valid exFAT
}

// Helper function to calculate cluster size
DWORD exfat_get_cluster_size(exfat_boot_sector *boot_sector)
{
    DWORD bytes_per_sector = 1 << boot_sector->bytes_per_sector_shift;
    DWORD sectors_per_cluster = 1 << boot_sector->sectors_per_cluster_shift;
    return bytes_per_sector * sectors_per_cluster;
}

// Helper function to get cluster sector
DWORD exfat_cluster_to_sector(exfat_boot_sector *boot_sector, DWORD cluster)
{
    if (cluster < 2) return 0; // Invalid cluster
    
    return boot_sector->cluster_heap_offset + 
           ((cluster - 2) << boot_sector->sectors_per_cluster_shift);
}

// Convert UTF-16 filename to ASCII (simplified)
void exfat_utf16_to_ascii(uint16_t *utf16_name, char *ascii_name, int length)
{
    int i;
    for (i = 0; i < length && utf16_name[i] != 0; i++) {
        // Simple conversion - just take lower byte for ASCII characters
        if (utf16_name[i] < 128) {
            ascii_name[i] = (char)utf16_name[i];
        } else {
            ascii_name[i] = '?'; // Non-ASCII character
        }
    }
    ascii_name[i] = 0; // Null terminate
}

// Read directory entries from a cluster
int exfat_read_directory_cluster(DWORD cluster, void **buffer, 
                                 exfat_boot_sector *boot_sector, int device_id)
{
    DWORD sector = exfat_cluster_to_sector(boot_sector, cluster);
    DWORD cluster_size = exfat_get_cluster_size(boot_sector);
    DWORD sectors_per_cluster = 1 << boot_sector->sectors_per_cluster_shift;
    DWORD handle;
    
    *buffer = malloc(cluster_size);
    if (*buffer == NULL) return 0;
    
    #ifdef DEBUG_EXFAT
    printf("exFAT: Reading directory cluster %d at sector %d\n", cluster, sector);
    #endif
    
    handle = dex32_requestIO(device_id, IO_READ, sector, sectors_per_cluster, *buffer);
    while (!dex32_IOcomplete(handle));
    dex32_closeIO(handle);
    
    return cluster_size;
}

// Mount the root directory
int exfat_mount_root(vfs_node *mountpoint, int device_id)
{
    exfat_boot_sector boot_sector;
    DWORD cluster_size;
    void *root_dir_data;
    int dir_size;
    
    if (mountpoint->files != 0) return -1;
    
    #ifdef DEBUG_EXFAT
    printf("exFAT: Mounting root directory...\n");
    #endif
    
    // Read and validate boot sector
    if (!exfat_read_boot_sector(&boot_sector, device_id)) {
        printf("exFAT: Failed to read boot sector\n");
        return -1;
    }
    
    if (!exfat_validate_boot_sector(&boot_sector)) {
        printf("exFAT: Invalid boot sector\n");
        return -1;
    }
    
    printf("exFAT: Valid exFAT filesystem detected\n");
    printf("exFAT: Volume length: %lld sectors\n", boot_sector.volume_length);
    printf("exFAT: Cluster count: %d\n", boot_sector.cluster_count);
    
    cluster_size = exfat_get_cluster_size(&boot_sector);
    printf("exFAT: Cluster size: %d bytes\n", cluster_size);
    
    // Store boot sector in mountpoint
    mountpoint->misc = malloc(sizeof(exfat_boot_sector));
    memcpy(mountpoint->misc, &boot_sector, sizeof(exfat_boot_sector));
    mountpoint->miscsize = sizeof(exfat_boot_sector);
    
    // Read root directory
    dir_size = exfat_read_directory_cluster(boot_sector.first_cluster_of_root_directory,
                                           &root_dir_data, &boot_sector, device_id);
    if (dir_size == 0) {
        printf("exFAT: Failed to read root directory\n");
        free(mountpoint->misc);
        return -1;
    }
    
    mountpoint->misc2 = root_dir_data;
    mountpoint->miscsize2 = dir_size;
    mountpoint->attb = FILE_DIRECTORY | FILE_OREAD | FILE_OWRITE;
    
    // Mount the directory entries
    exfat_mount_directory(mountpoint, root_dir_data, dir_size, device_id);
    
    return 1;
}

// Forward declaration
int exfat_mount_directory(vfs_node *mountpoint, void *dir_data, 
                         int dir_size, int device_id);

// Mount directory entries from directory data
int exfat_mount_directory(vfs_node *mountpoint, void *dir_data, 
                         int dir_size, int device_id)
{
    char *dir_ptr = (char*)dir_data;
    int offset = 0;
    
    #ifdef DEBUG_EXFAT
    printf("exFAT: Mounting directory entries...\n");
    #endif
    
    while (offset < dir_size) {
        exfat_directory_entry *entry = (exfat_directory_entry*)(dir_ptr + offset);
        
        // Check for end of directory
        if (entry->entry_type == EXFAT_TYPE_END_OF_DIRECTORY ||
            entry->entry_type == EXFAT_TYPE_UNUSED) {
            break;
        }
        
        // Process file entries
        if (entry->entry_type == EXFAT_TYPE_FILE) {
            // File entry found, process it with its secondary entries
            if (offset + 32 < dir_size) { // Make sure we have space for stream extension
                exfat_stream_extension_entry *stream = 
                    (exfat_stream_extension_entry*)(dir_ptr + offset + 32);
                
                if (stream->entry_type == EXFAT_TYPE_STREAM_EXTENSION) {
                    // Create VFS node for this file
                    vfs_node *node = (vfs_node*)malloc(sizeof(vfs_node));
                    memset(node, 0, sizeof(vfs_node));
                    vfs_createnode(node, mountpoint);
                    
                    // Set basic file information
                    node->fsid = exfat_deviceid;
                    node->memid = device_id;
                    node->size = stream->data_length;
                    node->start_sector = stream->first_cluster;
                    
                    // Set attributes
                    node->attb = FILE_OREAD | FILE_OWRITE;
                    if (entry->file_attributes & EXFAT_ATTR_DIRECTORY) {
                        node->attb |= FILE_DIRECTORY;
                        node->files = VFS_NOT_MOUNTED;
                    }
                    if (entry->file_attributes & EXFAT_ATTR_READ_ONLY) {
                        node->attb &= ~FILE_OWRITE;
                    }
                    
                    // Extract filename from file name entries
                    char filename[256] = {0};
                    int name_entries = entry->secondary_count - 1; // Minus stream extension
                    int name_pos = 0;
                    int i;
                    
                    for (i = 0; i < name_entries && i < 17; i++) { // Max 17 name entries
                        if (offset + 64 + i * 32 < dir_size) {
                            exfat_file_name_entry *name_entry = 
                                (exfat_file_name_entry*)(dir_ptr + offset + 64 + i * 32);
                            
                            if (name_entry->entry_type == EXFAT_TYPE_FILE_NAME) {
                                char name_part[31];
                                exfat_utf16_to_ascii(name_entry->file_name, name_part, 15);
                                strcat(filename, name_part);
                            }
                        }
                    }
                    
                    // Set filename (truncate if too long)
                    if (strlen(filename) > 0) {
                        strncpy(node->name, filename, 255);
                        node->name[255] = 0;
                    } else {
                        strcpy(node->name, "unnamed");
                    }
                    
                    #ifdef DEBUG_EXFAT
                    printf("exFAT: Added file: %s (size: %lld)\n", node->name, node->size);
                    #endif
                    
                    // Skip all secondary entries
                    offset += 32 * (1 + entry->secondary_count);
                    continue;
                }
            }
        }
        
        // Skip this entry
        offset += 32;
    }
    
    return 1;
}

// Mount a directory (called by VFS when accessing subdirectories)
int exfat_mountdirectory(vfs_node *directory, int device_id)
{
    exfat_boot_sector *boot_sector;
    void *dir_data;
    int dir_size;
    
    if (directory->path == NULL || directory->path->misc == NULL) {
        return 0;
    }
    
    boot_sector = (exfat_boot_sector*)directory->path->misc;
    
    #ifdef DEBUG_EXFAT
    printf("exFAT: Mounting subdirectory: %s\n", directory->name);
    #endif
    
    // Read directory cluster
    dir_size = exfat_read_directory_cluster(directory->start_sector, &dir_data, 
                                           boot_sector, device_id);
    if (dir_size == 0) {
        return 0;
    }
    
    directory->misc2 = dir_data;
    directory->miscsize2 = dir_size;
    
    // Mount the directory entries
    exfat_mount_directory(directory, dir_data, dir_size, device_id);
    
    return 1;
}

// Read file data
int exfat_openfileEX(vfs_node *f, char *buf, DWORD start, DWORD end, int device_id)
{
    exfat_boot_sector *boot_sector;
    DWORD cluster;
    DWORD cluster_size;
    DWORD bytes_to_read = end - start + 1;
    DWORD bytes_read = 0;
    DWORD file_offset = start;
    
    if (f->path == NULL || f->path->misc == NULL) {
        return 0;
    }
    
    boot_sector = (exfat_boot_sector*)f->path->misc;
    cluster = f->start_sector; // First cluster of file
    cluster_size = exfat_get_cluster_size(boot_sector);
    
    #ifdef DEBUG_EXFAT
    printf("exFAT: Reading file %s from offset %d, length %d\n", 
           f->name, start, bytes_to_read);
    #endif
    
    // Simple implementation: read from first cluster only
    // A full implementation would need to follow the FAT chain
    if (cluster >= 2) {
        DWORD sector = exfat_cluster_to_sector(boot_sector, cluster);
        DWORD sectors_per_cluster = 1 << boot_sector->sectors_per_cluster_shift;
        void *cluster_data = malloc(cluster_size);
        DWORD handle;
        
        if (cluster_data == NULL) return 0;
        
        handle = dex32_requestIO(device_id, IO_READ, sector, sectors_per_cluster, cluster_data);
        while (!dex32_IOcomplete(handle));
        dex32_closeIO(handle);
        
        // Copy requested data
        DWORD copy_offset = file_offset % cluster_size;
        DWORD copy_size = (bytes_to_read < cluster_size - copy_offset) ? 
                         bytes_to_read : cluster_size - copy_offset;
        
        if (copy_offset < cluster_size) {
            memcpy(buf, (char*)cluster_data + copy_offset, copy_size);
            bytes_read = copy_size;
        }
        
        free(cluster_data);
    }
    
    return bytes_read;
}

// Write file data (basic implementation)
int exfat_writefileEX(vfs_node *f, char *buf, int start, int end, int device_id)
{
    // For now, return 0 to indicate write not supported
    // A full implementation would need FAT manipulation
    #ifdef DEBUG_EXFAT
    printf("exFAT: Write operation not yet implemented\n");
    #endif
    return 0;
}

// Create file (basic implementation)
int exfat_createfileEX(vfs_node *node, int device_id)
{
    // For now, return -1 to indicate create not supported
    // A full implementation would need directory entry creation
    #ifdef DEBUG_EXFAT
    printf("exFAT: Create file operation not yet implemented\n");
    #endif
    return -1;
}

// Delete file (basic implementation)
int exfat_deletefile(vfs_node *node, int device_id)
{
    // For now, return -1 to indicate delete not supported
    #ifdef DEBUG_EXFAT
    printf("exFAT: Delete file operation not yet implemented\n");
    #endif
    return -1;
}

// Get bytes per block
DWORD exfat_getbytesperblock(int device_id)
{
    return EXFAT_SECTOR_SIZE;
}

// Validate filename
int exfat_validatefilename(const char *filename)
{
    // Basic validation - exFAT supports long filenames
    if (filename == NULL || strlen(filename) == 0 || strlen(filename) > 255) {
        return 0;
    }
    
    // Check for invalid characters (simplified)
    const char *invalid_chars = "\\/:*?\"<>|";
    int i;
    for (i = 0; filename[i]; i++) {
        if (strchr(invalid_chars, filename[i]) != NULL) {
            return 0;
        }
    }
    
    return 1;
}

// Rewrite file (for VFS compatibility)
void exfat_rewritefileEX(vfs_node *f, int device_id)
{
    // Do nothing for now
}

// Modify file attributes
int exfat_modifyattb(vfs_node *f, const int attb, int device_id)
{
    // For now, return 0 to indicate not supported
    return 0;
}

// Register exFAT filesystem with device manager
int exfat_register(const char *name)
{
    devmgr_fs_desc exfat_fs_desc;
    memset(&exfat_fs_desc, 0, sizeof(devmgr_fs_desc));
    
    // Fill up the filesystem descriptor
    strcpy(exfat_fs_desc.hdr.name, name);
    strcpy(exfat_fs_desc.hdr.description, "Microsoft exFAT filesystem driver");
    exfat_fs_desc.hdr.type = DEVMGR_FS;
    exfat_fs_desc.hdr.size = sizeof(exfat_fs_desc);
    
    // Set function pointers
    exfat_fs_desc.mountroot = exfat_mount_root;
    exfat_fs_desc.rewritefile = exfat_rewritefileEX;
    exfat_fs_desc.readfile = exfat_openfileEX;
    exfat_fs_desc.chattb = exfat_modifyattb;
    exfat_fs_desc.getsectorsize = NULL; // Use default
    exfat_fs_desc.deletefile = exfat_deletefile;
    exfat_fs_desc.addsectors = NULL; // Not supported yet
    exfat_fs_desc.writefile = exfat_writefileEX;
    exfat_fs_desc.createfile = exfat_createfileEX;
    exfat_fs_desc.getbytesperblock = exfat_getbytesperblock;
    exfat_fs_desc.mountdirectory = exfat_mountdirectory;
    exfat_fs_desc.validate_filename = exfat_validatefilename;
    
    // Register with device manager
    exfat_deviceid = devmgr_register((devmgr_generic*)&exfat_fs_desc);
    
    printf("exFAT: Filesystem driver registered (ID: %d)\n", exfat_deviceid);
    
    return exfat_deviceid;
}
