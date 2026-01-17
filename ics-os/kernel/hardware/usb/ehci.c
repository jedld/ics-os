/*
  Name: ehci.c
  Description: Minimal EHCI host controller driver (USB 2.0)
*/

#include "usb.h"
#include "../../stdlib/stdlib.h"
#include "../../stdlib/time.h"

// Simple PCI config access via ports 0xCF8/0xCFC
static inline void pci_outl(WORD port, DWORD val)
{
    asm volatile("outl %%eax,%%dx" : : "d"(port), "a"(val));
}

static inline DWORD pci_inl(WORD port)
{
    DWORD ret;
    asm volatile("inl %%dx,%%eax" : "=a"(ret) : "d"(port));
    return ret;
}

static DWORD pci_config_read_dword(BYTE bus, BYTE dev, BYTE func, BYTE offset)
{
    DWORD address = 0x80000000 | ((DWORD)bus << 16) | ((DWORD)(dev & 0x1F) << 11) | ((DWORD)(func & 0x07) << 8) | (offset & 0xFC);
    pci_outl(0xCF8, address);
    return pci_inl(0xCFC);
}

static void pci_config_write_dword(BYTE bus, BYTE dev, BYTE func, BYTE offset, DWORD value)
{
    DWORD address = 0x80000000 | ((DWORD)bus << 16) | ((DWORD)(dev & 0x1F) << 11) | ((DWORD)(func & 0x07) << 8) | (offset & 0xFC);
    pci_outl(0xCF8, address);
    pci_outl(0xCFC, value);
}

static BYTE pci_config_read_byte(BYTE bus, BYTE dev, BYTE func, BYTE offset)
{
    DWORD v = pci_config_read_dword(bus, dev, func, offset & 0xFC);
    return (v >> ((offset & 3) * 8)) & 0xFF;
}

#define EHCI_USBCMD_RUN       0x00000001
#define EHCI_USBCMD_RESET     0x00000002
#define EHCI_USBCMD_ASYNC     0x00000020
#define EHCI_USBSTS_HALT      0x00001000
#define EHCI_USBSTS_INT       0x00000001
#define EHCI_PORTSC_CCS       0x00000001
#define EHCI_PORTSC_PED       0x00000004
#define EHCI_PORTSC_PR        0x00000100
#define EHCI_PORTSC_OWNER     0x00002000
#define EHCI_CONFIGFLAG       0x00000001

#define QTD_TOKEN_ACTIVE      (1<<7)
#define QTD_TOKEN_PID_OUT     (0<<8)
#define QTD_TOKEN_PID_IN      (1<<8)
#define QTD_TOKEN_PID_SETUP   (2<<8)
#define QTD_TOKEN_CERR        (3<<10)
#define QTD_TOKEN_DT          (1<<31)

#define QH_TYPE_QH            (2<<1)
#define QH_HEAD               (1<<15)

#define QTD_TERMINATE         0x00000001

typedef struct __attribute__((packed)) _ehci_qtd {
    DWORD next_qtd;
    DWORD alt_next_qtd;
    DWORD token;
    DWORD buffer[5];
    DWORD buffer_hi[5];
} ehci_qtd;

typedef struct __attribute__((packed)) _ehci_qh {
    DWORD horiz_link;
    DWORD ep_char;
    DWORD ep_cap;
    DWORD current_qtd;
    DWORD next_qtd;
    DWORD alt_next_qtd;
    DWORD token;
    DWORD buffer[5];
    DWORD buffer_hi[5];
} ehci_qh;

typedef struct _ehci_controller {
    volatile DWORD *cap_base;
    volatile DWORD *op_base;
    DWORD cap_length;
    DWORD hcs_params;
    DWORD hcc_params;
    DWORD port_count;
    ehci_qh *async_qh;
} ehci_controller;

static ehci_controller ehci;

static void *usb_alloc_aligned(int size, int align)
{
    BYTE *raw = (BYTE*)malloc(size + align);
    DWORD addr = (DWORD)raw;
    addr = (addr + (align-1)) & ~(align-1);
    return (void*)addr;
}

static void ehci_qtd_set_buf(ehci_qtd *qtd, BYTE *buf, DWORD len)
{
    DWORD addr = (DWORD)buf;
    int i;
    for (i=0;i<5;i++)
    {
        qtd->buffer[i] = addr;
        qtd->buffer_hi[i] = 0;
        addr = (addr & ~0xFFF) + 0x1000;
    }
}

static int ehci_wait_qtd(ehci_qtd *qtd, int timeout_ms)
{
    int i;
    for (i=0;i<timeout_ms;i++)
    {
        if (!(qtd->token & QTD_TOKEN_ACTIVE))
            return 1;
        delay(1);
    }
    return 0;
}

static int ehci_port_reset(int port)
{
    volatile DWORD *portsc = (volatile DWORD*)((DWORD)ehci.op_base + 0x44 + 4*port);
    DWORD v = *portsc;

    if (!(v & EHCI_PORTSC_CCS))
        return 0;

    *portsc = v | EHCI_PORTSC_PR;
    delay(50);
    *portsc = v & ~EHCI_PORTSC_PR;
    delay(50);

    v = *portsc;
    if (v & EHCI_PORTSC_PED)
        return 1;

    return 0;
}

int ehci_port_connected(int port)
{
    volatile DWORD *portsc = (volatile DWORD*)((DWORD)ehci.op_base + 0x44 + 4*port);
    return ((*portsc) & EHCI_PORTSC_CCS) ? 1 : 0;
}

static int ehci_take_ownership(BYTE bus, BYTE dev, BYTE func, DWORD bar)
{
    DWORD hcc = ehci.hcc_params;
    BYTE eecp = (hcc >> 8) & 0xFF;
    if (eecp == 0) return 1;

    DWORD ext = pci_config_read_dword(bus, dev, func, eecp);
    if ((ext & 0xFF) != 0x01)
        return 1;

    // set OS owned semaphore
    ext |= (1<<24);
    pci_config_write_dword(bus, dev, func, eecp, ext);

    int i;
    for (i=0;i<100;i++)
    {
        ext = pci_config_read_dword(bus, dev, func, eecp);
        if (!(ext & (1<<16)))
            return 1;
        delay(1);
    }
    return 0;
}

static int ehci_setup_async()
{
    ehci.async_qh = (ehci_qh*)usb_alloc_aligned(sizeof(ehci_qh), 32);
    memset(ehci.async_qh, 0, sizeof(ehci_qh));

    ehci.async_qh->horiz_link = ((DWORD)ehci.async_qh) | QH_TYPE_QH;
    ehci.async_qh->ep_char = QH_HEAD;
    ehci.async_qh->next_qtd = QTD_TERMINATE;
    ehci.async_qh->alt_next_qtd = QTD_TERMINATE;

    ehci.op_base[0x18/4] = (DWORD)ehci.async_qh; // ASYNCLISTADDR
    return 1;
}

int ehci_control_transfer(int devaddr, int max_packet, usb_setup_packet *setup, BYTE *data, int length, int dir_in)
{
    ehci_qtd *qtd_setup = (ehci_qtd*)usb_alloc_aligned(sizeof(ehci_qtd), 32);
    ehci_qtd *qtd_data = 0;
    ehci_qtd *qtd_status = (ehci_qtd*)usb_alloc_aligned(sizeof(ehci_qtd), 32);

    memset(qtd_setup, 0, sizeof(ehci_qtd));
    memset(qtd_status, 0, sizeof(ehci_qtd));

    qtd_setup->next_qtd = (length > 0) ? 0 : (DWORD)qtd_status;
    qtd_setup->alt_next_qtd = QTD_TERMINATE;
    qtd_setup->token = QTD_TOKEN_ACTIVE | QTD_TOKEN_PID_SETUP | QTD_TOKEN_CERR | (8<<16);
    ehci_qtd_set_buf(qtd_setup, (BYTE*)setup, 8);

    if (length > 0)
    {
        qtd_data = (ehci_qtd*)usb_alloc_aligned(sizeof(ehci_qtd), 32);
        memset(qtd_data, 0, sizeof(ehci_qtd));
        qtd_setup->next_qtd = (DWORD)qtd_data;
        qtd_data->next_qtd = (DWORD)qtd_status;
        qtd_data->alt_next_qtd = QTD_TERMINATE;
        qtd_data->token = QTD_TOKEN_ACTIVE | QTD_TOKEN_CERR | (length<<16) | (dir_in ? QTD_TOKEN_PID_IN : QTD_TOKEN_PID_OUT) | QTD_TOKEN_DT;
        ehci_qtd_set_buf(qtd_data, data, length);
    }

    qtd_status->next_qtd = QTD_TERMINATE;
    qtd_status->alt_next_qtd = QTD_TERMINATE;
    qtd_status->token = QTD_TOKEN_ACTIVE | QTD_TOKEN_CERR | (0<<16) | (dir_in ? QTD_TOKEN_PID_OUT : QTD_TOKEN_PID_IN) | QTD_TOKEN_DT;

    // setup QH overlay
    ehci.async_qh->ep_char = (max_packet & 0x7FF) | (devaddr << 8) | (USB_SPEED_HIGH << 12) | QH_HEAD;
    ehci.async_qh->ep_cap = 0;
    ehci.async_qh->current_qtd = 0;
    ehci.async_qh->next_qtd = (DWORD)qtd_setup;
    ehci.async_qh->alt_next_qtd = QTD_TERMINATE;
    ehci.async_qh->token = 0;

    // run controller
    ehci.op_base[0x00/4] = EHCI_USBCMD_RUN | EHCI_USBCMD_ASYNC;
    ehci.op_base[0x40/4] = EHCI_CONFIGFLAG;

    if (!ehci_wait_qtd(qtd_status, 200))
        return 0;

    return 1;
}

int ehci_bulk_transfer(int devaddr, int ep, int max_packet, int dir_in, BYTE *data, int length)
{
    ehci_qtd *qtd = (ehci_qtd*)usb_alloc_aligned(sizeof(ehci_qtd), 32);
    memset(qtd, 0, sizeof(ehci_qtd));

    qtd->next_qtd = QTD_TERMINATE;
    qtd->alt_next_qtd = QTD_TERMINATE;
    qtd->token = QTD_TOKEN_ACTIVE | QTD_TOKEN_CERR | (length<<16) | (dir_in ? QTD_TOKEN_PID_IN : QTD_TOKEN_PID_OUT) | QTD_TOKEN_DT;
    ehci_qtd_set_buf(qtd, data, length);

    ehci.async_qh->ep_char = (max_packet & 0x7FF) | (devaddr << 8) | (ep << 15) | (USB_SPEED_HIGH << 12) | QH_HEAD;
    ehci.async_qh->ep_cap = 0;
    ehci.async_qh->current_qtd = 0;
    ehci.async_qh->next_qtd = (DWORD)qtd;
    ehci.async_qh->alt_next_qtd = QTD_TERMINATE;
    ehci.async_qh->token = 0;

    ehci.op_base[0x00/4] = EHCI_USBCMD_RUN | EHCI_USBCMD_ASYNC;
    ehci.op_base[0x40/4] = EHCI_CONFIGFLAG;

    if (!ehci_wait_qtd(qtd, 200))
        return 0;

    return 1;
}

int ehci_init_controller()
{
    BYTE bus, dev, func;
    DWORD bar = 0;

    for (bus=0; bus<0xFF; bus++)
    {
        for (dev=0; dev<32; dev++)
        {
            for (func=0; func<8; func++)
            {
                BYTE class = pci_config_read_byte(bus, dev, func, 0x0B);
                BYTE subclass = pci_config_read_byte(bus, dev, func, 0x0A);
                BYTE progif = pci_config_read_byte(bus, dev, func, 0x09);

                if (class == 0x0C && subclass == 0x03 && progif == 0x20)
                {
                    bar = pci_config_read_dword(bus, dev, func, 0x10);
                    bar &= 0xFFFFFFF0;

                    ehci.cap_base = (DWORD*)bar;
                    ehci.cap_length = ((BYTE*)ehci.cap_base)[0];
                    ehci.hcs_params = ehci.cap_base[1];
                    ehci.hcc_params = ehci.cap_base[2];
                    ehci.op_base = (DWORD*)((DWORD)ehci.cap_base + ehci.cap_length);
                    ehci.port_count = ehci.hcs_params & 0x0F;

                    ehci_take_ownership(bus, dev, func, bar);

                    // reset controller
                    ehci.op_base[0x00/4] |= EHCI_USBCMD_RESET;
                    while (ehci.op_base[0x00/4] & EHCI_USBCMD_RESET) { }

                    ehci_setup_async();

                    return 1;
                }
            }
        }
    }

    return 0;
}

int ehci_get_port_count()
{
    return ehci.port_count;
}

int ehci_reset_port(int port)
{
    return ehci_port_reset(port);
}

int ehci_ready()
{
    return (ehci.cap_base != 0);
}

