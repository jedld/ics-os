/*
  Name: usb_core.c
  Description: Minimal USB device enumeration for EHCI
*/

#include "usb.h"
#include "../../stdlib/stdlib.h"
#include "../../stdlib/time.h"

// EHCI interface
int ehci_init_controller();
int ehci_get_port_count();
int ehci_reset_port(int port);
int ehci_port_connected(int port);
int ehci_control_transfer(int devaddr, int max_packet, usb_setup_packet *setup, BYTE *data, int length, int dir_in);
int ehci_bulk_transfer(int devaddr, int ep, int max_packet, int dir_in, BYTE *data, int length);
int ehci_ready();
void usb_msc_rescan();

static usb_device usb_devices[USB_MAX_DEVICES];
static int usb_device_count = 0;
static int usb_next_address = 1;

static int usb_control(usb_device *dev, BYTE request, WORD value, WORD index, BYTE *data, WORD length, int dir_in)
{
    usb_setup_packet setup;
    setup.bmRequestType = (dir_in ? 0x80 : 0x00);
    setup.bRequest = request;
    setup.wValue = value;
    setup.wIndex = index;
    setup.wLength = length;

    return ehci_control_transfer(dev->addr, dev->max_packet0, &setup, data, length, dir_in);
}

static int usb_get_descriptor(usb_device *dev, BYTE dtype, BYTE index, BYTE *buf, WORD len)
{
    return usb_control(dev, USB_REQ_GET_DESCRIPTOR, (dtype<<8) | index, 0, buf, len, USB_DIR_IN);
}

static int usb_set_address(usb_device *dev, int addr)
{
    int res = usb_control(dev, USB_REQ_SET_ADDRESS, addr, 0, 0, 0, USB_DIR_OUT);
    if (res) dev->addr = addr;
    delay(10);
    return res;
}

static int usb_set_configuration(usb_device *dev, int cfg)
{
    int res = usb_control(dev, USB_REQ_SET_CONFIGURATION, cfg, 0, 0, 0, USB_DIR_OUT);
    if (res) dev->configuration = cfg;
    return res;
}

static int usb_parse_config(usb_device *dev, BYTE *cfg, int len)
{
    int i = 0;
    while (i < len)
    {
        BYTE dlen = cfg[i];
        BYTE dtype = cfg[i+1];
        if (dlen == 0) break;

        if (dtype == USB_DT_INTERFACE)
        {
            BYTE class = cfg[i+5];
            BYTE subclass = cfg[i+6];
            BYTE proto = cfg[i+7];
            if (class == USB_CLASS_MASS_STORAGE && subclass == USB_SUBCLASS_SCSI && proto == USB_PROTOCOL_BULKONLY)
            {
                // find endpoints after this interface
                int j = i + dlen;
                while (j < len)
                {
                    BYTE elen = cfg[j];
                    BYTE etype = cfg[j+1];
                    if (elen == 0) break;
                    if (etype == USB_DT_ENDPOINT)
                    {
                        BYTE addr = cfg[j+2];
                        BYTE attr = cfg[j+3];
                        WORD mps = cfg[j+4] | (cfg[j+5] << 8);
                        if ((attr & 0x03) == 0x02) // bulk
                        {
                            if (addr & 0x80)
                            {
                                dev->bulk_in.address = addr & 0x0F;
                                dev->bulk_in.attributes = attr;
                                dev->bulk_in.max_packet = mps;
                            }
                            else
                            {
                                dev->bulk_out.address = addr & 0x0F;
                                dev->bulk_out.attributes = attr;
                                dev->bulk_out.max_packet = mps;
                            }
                        }
                    }
                    j += elen;
                    if (etype == USB_DT_INTERFACE) break;
                }
                return 1;
            }
        }
        i += dlen;
    }
    return 0;
}

static usb_device *usb_find_by_port(int port)
{
    int i;
    for (i=0;i<USB_MAX_DEVICES;i++)
        if (usb_devices[i].present && usb_devices[i].port == port)
            return &usb_devices[i];
    return 0;
}

static usb_device *usb_find_free_slot()
{
    int i;
    for (i=0;i<USB_MAX_DEVICES;i++)
        if (!usb_devices[i].present)
            return &usb_devices[i];
    return 0;
}

static int usb_enumerate_port(int port, usb_device *dev, int addr)
{
    BYTE dev_desc[18];
    BYTE cfg_desc[256];
    BYTE cfg_head[9];

    memset(dev, 0, sizeof(usb_device));
    dev->present = 1;
    dev->port = port;
    dev->speed = USB_SPEED_HIGH;
    dev->max_packet0 = 64;
    dev->addr = 0;
    dev->msc_registered = 0;

    if (!usb_get_descriptor(dev, USB_DT_DEVICE, 0, dev_desc, 8))
        return 0;

    dev->max_packet0 = dev_desc[7];
    if (!usb_set_address(dev, addr))
        return 0;

    if (!usb_get_descriptor(dev, USB_DT_DEVICE, 0, dev_desc, 18))
        return 0;

    if (!usb_get_descriptor(dev, USB_DT_CONFIG, 0, cfg_head, sizeof(cfg_head)))
        return 0;

    int total_len = cfg_head[2] | (cfg_head[3] << 8);
    if (total_len > sizeof(cfg_desc)) total_len = sizeof(cfg_desc);
    if (!usb_get_descriptor(dev, USB_DT_CONFIG, 0, cfg_desc, total_len))
        return 0;

    if (!usb_parse_config(dev, cfg_desc, total_len))
        return 0;

    if (!usb_set_configuration(dev, cfg_desc[5]))
        return 0;

    return 1;
}

int usb_init()
{
    int i;
    memset(usb_devices, 0, sizeof(usb_devices));
    usb_device_count = 0;
    usb_next_address = 1;

    if (!ehci_init_controller())
        return 0;

    for (i=0;i<ehci_get_port_count() && usb_device_count < USB_MAX_DEVICES;i++)
    {
        if (ehci_reset_port(i))
        {
            usb_device *slot = usb_find_free_slot();
            if (slot && usb_enumerate_port(i, slot, usb_next_address))
            {
                usb_device_count++;
                usb_next_address++;
            }
        }
    }
    return usb_device_count;
}

usb_device *usb_get_device(int index)
{
    if (index < 0 || index >= USB_MAX_DEVICES) return 0;
    if (!usb_devices[index].present) return 0;
    return &usb_devices[index];
}

usb_device *usb_get_device_slot(int index)
{
    if (index < 0 || index >= USB_MAX_DEVICES) return 0;
    return &usb_devices[index];
}

int usb_bulk_transfer(usb_device *dev, int ep, int max_packet, int dir_in, BYTE *data, int length)
{
    return ehci_bulk_transfer(dev->addr, ep, max_packet, dir_in, data, length);
}

int usb_poll()
{
    int i;
    int newdevs = 0;

    if (!ehci_ready()) return 0;

    for (i=0;i<ehci_get_port_count();i++)
    {
        usb_device *existing = usb_find_by_port(i);
        int connected = ehci_port_connected(i);

        if (!connected && existing)
        {
            existing->present = 0;
            existing->msc_registered = 0;
            if (usb_device_count > 0) usb_device_count--;
            continue;
        }

        if (connected && !existing)
        {
            if (ehci_reset_port(i))
            {
                usb_device *slot = usb_find_free_slot();
                if (slot && usb_enumerate_port(i, slot, usb_next_address))
                {
                    usb_device_count++;
                    usb_next_address++;
                    newdevs++;
                }
            }
        }
    }

    return newdevs;
}

DWORD usb_poll_thread()
{
    while (1)
    {
        usb_poll();
        usb_msc_rescan();
        delay(10);
    }
    return 0;
}

