/*
 * UHCI Low-Level Support (Initial Experimental Implementation)
 * WARNING: This is a simplified educational implementation. It sets up a
 * minimal frame list and builds TD chains for control transfers using polling.
 * Not production quality; timing/robust error handling omitted.
 */
#include "usb.h"
#include "../chips/ports.h"
#include "../../stdlib/dexstdlib.h"

#ifndef NULL
#define NULL 0
#endif

/* UHCI register offsets (subset) */
#define UHCI_REG_CMD      0x00  /* USBCMD */
#define UHCI_REG_STS      0x02  /* USBSTS */
#define UHCI_REG_FRNUM    0x06
#define UHCI_REG_FLBASE   0x08
#define UHCI_REG_SOFMOD   0x0C

/* TD link pointer bits */
#define UHCI_PTR_T        0x00000001
#define UHCI_PTR_QH       0x00000002

/* TD control/status bits (simplified) */
#define UHCI_TD_CTRL_ACTIVE   (1<<23)
#define UHCI_TD_CTRL_STALLED  (1<<22)
#define UHCI_TD_CTRL_DBUFERR  (1<<21)
#define UHCI_TD_CTRL_BABBLE   (1<<20)
#define UHCI_TD_CTRL_NAK      (1<<19)
#define UHCI_TD_CTRL_CRCTO    (1<<18)
#define UHCI_TD_CTRL_BITSTUFF (1<<17)
#define UHCI_TD_CTRL_MASK_ERR (UHCI_TD_CTRL_STALLED|UHCI_TD_CTRL_DBUFERR|UHCI_TD_CTRL_BABBLE|UHCI_TD_CTRL_CRCTO|UHCI_TD_CTRL_BITSTUFF)

/* PIDs */
#define PID_OUT  0xE1
#define PID_IN   0x69
#define PID_SETUP 0x2D

struct uhci_td {
    unsigned int link;   /* Next TD/QH */
    unsigned int ctrl;   /* Control & status */
    unsigned int token;  /* PID, addr, endpoint, data toggle, length */
    unsigned int buffer; /* Buffer pointer */
} __attribute__((packed, aligned(16)));

/* Frame list + static TDs (single control chain reused) */
static unsigned int frame_list[1024] __attribute__((aligned(4096)));
static struct uhci_td td_setup __attribute__((aligned(16)));
static struct uhci_td td_data  __attribute__((aligned(16)));
static struct uhci_td td_status __attribute__((aligned(16)));

static int uhci_global_inited = 0;

static void uhci_delay(int it){
    volatile int i; for (i=0;i<it*1000;i++); /* crude */
}

int uhci_init_controller(usb_controller_t *c){
    if (!c || c->type != 0) return -1; /* Only UHCI */
    if (uhci_global_inited) return 0;

    /* Initialize frame list to terminate */
    for (int i=0;i<1024;i++) frame_list[i] = UHCI_PTR_T;

    /* Program frame list base (physical == virtual assumption) */
    outportl(c->base_address + UHCI_REG_FLBASE, (unsigned int)frame_list);
    outportw(c->base_address + UHCI_REG_FRNUM, 0);
    outportb(c->base_address + UHCI_REG_SOFMOD, 0x40);

    /* Run controller: Set Run/Stop bit */
    outportw(c->base_address + UHCI_REG_CMD, 0x0001);
    uhci_delay(1);

    uhci_global_inited = 1;
    return 0;
}

/* Helper to build token dword (length encoded as (maxlen-1) in bits 21:27) */
static unsigned int uhci_build_token(unsigned char pid, unsigned char addr, unsigned char ep, unsigned short len, int toggle){
    unsigned int tok = 0;
    unsigned short encoded_len = (len==0)?0x7FF: (len - 1); /* 11 bits */
    tok |= pid;                /* bits 0-7 */
    tok |= ((unsigned int)addr & 0x7F) << 8;      /* bits 8-14 */
    tok |= ((unsigned int)ep & 0x0F) << 15;       /* bits 15-18 */
    tok |= (unsigned int)encoded_len << 21;       /* bits 21-31 */
    if (toggle) tok |= (1<<19); /* Data Toggle bit (bit 19 in UHCI) */
    return tok;
}

static int uhci_poll_td(struct uhci_td *td, int timeout_ms){
    while (timeout_ms-- > 0){
        if (!(td->ctrl & UHCI_TD_CTRL_ACTIVE)){
            if (td->ctrl & UHCI_TD_CTRL_MASK_ERR) return -1;
            return 0;
        }
        uhci_delay(1);
    }
    return -1; /* timeout */
}

/* Minimal control transfer (SETUP + optional DATA IN + STATUS) */
int uhci_control_transfer(usb_device_t *dev, unsigned char req_type, unsigned char req,
                          unsigned short val, unsigned short idx, void *data, unsigned short len){
    if (!dev || dev->controller->type != 0) return -2; /* Not UHCI */
    if (!uhci_global_inited) uhci_init_controller(dev->controller);

    /* Build setup packet (8 bytes) */
    unsigned char setup_pkt[8];
    setup_pkt[0] = req_type;
    setup_pkt[1] = req;
    setup_pkt[2] = val & 0xFF; setup_pkt[3] = (val>>8)&0xFF;
    setup_pkt[4] = idx & 0xFF; setup_pkt[5] = (idx>>8)&0xFF;
    setup_pkt[6] = len & 0xFF; setup_pkt[7] = (len>>8)&0xFF;

    /* TD Setup */
    td_setup.link = (len? (unsigned int)&td_data : (unsigned int)&td_status) & ~0xF; /* 16-byte aligned pointer */
    td_setup.ctrl = UHCI_TD_CTRL_ACTIVE | (3<<27); /* 3 retries */
    td_setup.token = uhci_build_token(PID_SETUP, dev->address, 0, 8, 0);
    td_setup.buffer = (unsigned int)setup_pkt;

    /* TD Data (IN if len>0) */
    if (len){
        td_data.link = (unsigned int)&td_status & ~0xF;
        td_data.ctrl = UHCI_TD_CTRL_ACTIVE | (3<<27);
        td_data.token = uhci_build_token(PID_IN, dev->address, 0, len, 1);
        td_data.buffer = (unsigned int)data;
    }

    /* TD Status (OUT with toggle=1 & zero length) */
    td_status.link = UHCI_PTR_T; /* terminate */
    td_status.ctrl = UHCI_TD_CTRL_ACTIVE | (3<<27);
    td_status.token = uhci_build_token(len?PID_OUT:PID_IN, dev->address, 0, 0, 1);
    td_status.buffer = 0;

    /* Point all frames to first TD for simplicity */
    for (int i=0;i<1024;i++) frame_list[i] = ((unsigned int)&td_setup & ~0xF);

    /* Poll TDs sequentially */
    if (uhci_poll_td(&td_setup, 20)) return -1;
    if (len && uhci_poll_td(&td_data, 50)) return -1;
    if (uhci_poll_td(&td_status, 20)) return -1;

    return 0;
}

/* Simplified bulk transfer (single TD) */
int uhci_bulk_transfer(usb_device_t *dev, unsigned char ep, void *buffer, unsigned int len){
    if (!dev || dev->controller->type != 0) return -2;
    if (!buffer || !len) return -1;
    if (!uhci_global_inited) uhci_init_controller(dev->controller);

    static struct uhci_td td_bulk __attribute__((aligned(16)));
    td_bulk.link = UHCI_PTR_T;
    td_bulk.ctrl = UHCI_TD_CTRL_ACTIVE | (3<<27);
    int in = (ep & 0x80)!=0;
    td_bulk.token = uhci_build_token(in?PID_IN:PID_OUT, dev->address, (ep & 0xF), len, 1);
    td_bulk.buffer = (unsigned int)buffer;
    for (int i=0;i<1024;i++) frame_list[i] = ((unsigned int)&td_bulk & ~0xF);
    if (uhci_poll_td(&td_bulk, 200)) return -1;
    return 0;
}
