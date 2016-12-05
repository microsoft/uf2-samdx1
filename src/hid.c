#include "uf2.h"

#if USE_HID

#define HF2_FLAG_PKT_LAST 0xC0
#define HF2_FLAG_PKT_BODY 0x80
#define HF2_FLAG_SERIAL 0x40
#define HF2_FLAG_MASK 0xC0
#define HF2_FLAG_RESERVED 0x00
#define HF2_SIZE_MASK 63

#define HF2_CMD_INFO 0x0001
#define HF2_CMD_RESET_INTO_APP 0x0002
#define HF2_CMD_RESET_INTO_BOOTLOADER 0x0003
#define HF2_CMD_WRITE_FLASH_PAGE 0x0004
#define HF2_CMD_MEM_WRITE_PAGE 0x0005
#define HF2_CMD_MEM_READ_PAGE 0x0006

#define HF2_STATUS_OK 0x00000000
#define HF2_STATUS_INVALID_CMD 0x00000001

typedef struct {
    PacketBuffer pbuf;
    COMPILER_WORD_ALIGNED
    uint16_t size;
    uint8_t serial;
    union {
        uint8_t buf[FLASH_ROW_SIZE + 64];
        uint32_t buf32[(FLASH_ROW_SIZE + 64) / 4];
        uint16_t buf16[(FLASH_ROW_SIZE + 64) / 2];
    };
} HID_InBuffer;

int recv_hid(HID_InBuffer *pkt) {
    if (!USB_ReadCore(NULL, 64, USB_EP_HID, &pkt->pbuf))
        return 0; // no data

    assert(pkt->pbuf.ptr == 0 && pkt->pbuf.size > 0);
    pkt->pbuf.size = 0;
    uint8_t tag = pkt->pbuf.buf[0];
    assert(pkt->size == 0 || (tag & 0x80));
    memcpy(pkt->buf + pkt->size, pkt->pbuf.buf + 1, tag & HF2_SIZE_MASK);
    pkt->size += tag & HF2_SIZE_MASK;
    assert(pkt->size <= sizeof(pkt->buf));
    if (tag & 0x40) {
        int sz = pkt->size;
        pkt->serial = (tag & HF2_FLAG_MASK) == HF2_FLAG_SERIAL;
        pkt->size = 0;
        return sz;
    }
    return 0;
}

void send_hid(const void *data, int size) {
    uint8_t buf[64];
    const uint8_t *ptr = data;

    for (;;) {
        int s;
        if (size <= 63) {
            s = size;
            buf[0] = HF2_FLAG_PKT_LAST | size;
        } else {
            s = 63;
            buf[0] = HF2_FLAG_PKT_BODY | 63;
        }
        memcpy(buf + 1, ptr, s);
        USB_WriteCore(buf, sizeof(buf), USB_EP_HID, true);
        ptr += s;
        size -= s;
        if (!size)
            break;
    }
}

HID_InBuffer hidbuf;

void process_hid(void) {
    int sz = recv_hid(&hidbuf);

    if (!sz)
        return;
    if (hidbuf.serial)
        return; // serial
    if (!recv_hid(&hidbuf))
        return;

    logval("HID", hidbuf.buf32[0]);
    uint32_t resp[] = { hidbuf.buf32[0], HF2_STATUS_OK };

    switch (hidbuf.buf16[0]) {
    case HF2_CMD_INFO:
        send_hid(infoUf2File, strlen(infoUf2File));
        break;
    case HF2_CMD_RESET_INTO_APP:
        resetIntoApp();
        break;
    case HF2_CMD_RESET_INTO_BOOTLOADER:
        resetIntoBootloader();
        break;
    case HF2_CMD_WRITE_FLASH_PAGE:
        assert(sz == 8 + FLASH_PAGE_SIZE);
        flash_write_row((void*)hidbuf.buf32[1], hidbuf.buf32 + 2);
        break;
    case HF2_CMD_MEM_WRITE_PAGE:
        assert(sz == 8 + FLASH_PAGE_SIZE);
        memcpy((void*)hidbuf.buf32[1], hidbuf.buf + 8, FLASH_PAGE_SIZE);
        break;
    case HF2_CMD_MEM_READ_PAGE:
        assert(sz == 8);
        memcpy(hidbuf.buf32 + 2, (void*)hidbuf.buf32[1], FLASH_PAGE_SIZE);
        hidbuf.buf32[1] = 0;
        send_hid(hidbuf.buf, 8 + FLASH_PAGE_SIZE);
        return;

    default:
        resp[1] = HF2_STATUS_INVALID_CMD;
        break;
    }

    send_hid(resp, 8);
}

#endif