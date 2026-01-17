/*
  Name: usb.h
  Description: Minimal USB 2.0 (EHCI) core + Mass Storage interface
*/

#ifndef USB_H
#define USB_H

#include "../../dextypes.h"

#define USB_MAX_DEVICES 8
#define USB_MAX_MSC_DEVICES 8

#define USB_DIR_OUT 0
#define USB_DIR_IN  1

#define USB_REQ_GET_STATUS        0x00
#define USB_REQ_CLEAR_FEATURE     0x01
#define USB_REQ_SET_FEATURE       0x03
#define USB_REQ_SET_ADDRESS       0x05
#define USB_REQ_GET_DESCRIPTOR    0x06
#define USB_REQ_SET_DESCRIPTOR    0x07
#define USB_REQ_GET_CONFIGURATION 0x08
#define USB_REQ_SET_CONFIGURATION 0x09

#define USB_DT_DEVICE        0x01
#define USB_DT_CONFIG        0x02
#define USB_DT_STRING        0x03
#define USB_DT_INTERFACE     0x04
#define USB_DT_ENDPOINT      0x05

#define USB_CLASS_MASS_STORAGE 0x08
#define USB_SUBCLASS_SCSI      0x06
#define USB_PROTOCOL_BULKONLY  0x50

#define USB_SPEED_HIGH 2
#define USB_SPEED_FULL 1
#define USB_SPEED_LOW  0

typedef struct __attribute__((packed)) _usb_setup_packet {
    BYTE bmRequestType;
    BYTE bRequest;
    WORD wValue;
    WORD wIndex;
    WORD wLength;
} usb_setup_packet;

typedef struct _usb_endpoint {
    BYTE address;
    BYTE attributes;
    WORD max_packet;
    BYTE interval;
} usb_endpoint;

typedef struct _usb_device {
    int present;
    int addr;
    int port;
    int speed;
    int max_packet0;
    int msc_registered;
    usb_endpoint bulk_in;
    usb_endpoint bulk_out;
    BYTE configuration;
} usb_device;

int usb_init();
int usb_poll();
DWORD usb_poll_thread();
int usb_msc_init();
void usb_msc_rescan();
int usb_msc_get_block_count();
const char *usb_msc_get_block_name(int index);
int usb_msc_get_block_deviceid(int index);
usb_device *usb_get_device(int index);
usb_device *usb_get_device_slot(int index);
int usb_bulk_transfer(usb_device *dev, int ep, int max_packet, int dir_in, BYTE *data, int length);

#endif
