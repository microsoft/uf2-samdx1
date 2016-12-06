#include "uf2.h"

#if USE_HID || USE_WEBUSB

typedef struct {
    PacketBuffer pbuf;
    uint16_t size;
    uint8_t serial;
    uint8_t ep;
    union {
        uint8_t buf[FLASH_ROW_SIZE + 64];
        uint32_t buf32[(FLASH_ROW_SIZE + 64) / 4];
        uint16_t buf16[(FLASH_ROW_SIZE + 64) / 2];
    };
} HID_InBuffer;

int recv_hid(HID_InBuffer *pkt) {
    if (!USB_ReadCore(NULL, 64, pkt->ep, &pkt->pbuf))
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

void send_hid(const void *data, int size, int ep) {
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
        USB_WriteCore(buf, sizeof(buf), ep, true);
        ptr += s;
        size -= s;
        if (!size)
            break;
    }
}

void send_hid_response(HID_InBuffer *pkt, const void *data, int size) {
    logval("sendresp", size);
    if (data)
        memcpy(pkt->buf32 + 1, data, size);
    send_hid(pkt->buf, 4 + size, pkt->ep);
}

static void checksum_pages(HID_InBuffer *pkt, int start, int num) {
    for (int i = 0; i < num; ++i) {
        uint8_t *data = (uint8_t *)start + i * FLASH_ROW_SIZE;
        uint16_t crc = 0;
        for (int j = 0; j < FLASH_ROW_SIZE; ++j) {
            crc = add_crc(*data++, crc);
        }
        pkt->buf16[i + 2] = crc;
    }
    send_hid_response(pkt, 0, num * 2);
}

void process_core(HID_InBuffer *pkt) {
    int sz = recv_hid(pkt);

    if (!sz)
        return;

    if (pkt->serial) {
        // echo back serial input - just for testing
        pkt->pbuf.buf[1] ^= 'a' - 'A';
        pkt->pbuf.buf[2] ^= 'a' - 'A';
        pkt->pbuf.buf[3] ^= 'a' - 'A';
        USB_WriteCore(pkt->pbuf.buf, 64, pkt->ep, true);
        return;
    }

    logwrite("HID sz=");
    logwritenum(sz);
    logval(" CMD", pkt->buf32[0]);

    switch (pkt->buf16[0]) {
    case HF2_CMD_INFO:
        send_hid_response(pkt, infoUf2File, strlen(infoUf2File));
        return;

    case HF2_CMD_BININFO:
        pkt->buf32[1] = HF2_MODE_BOOTLOADER; // bootloader
        pkt->buf32[2] = FLASH_ROW_SIZE;
        send_hid_response(pkt, 0, 8);
        return;

    case HF2_CMD_RESET_INTO_APP:
        resetIntoApp();
        break;
    case HF2_CMD_RESET_INTO_BOOTLOADER:
        resetIntoBootloader();
        break;
    case HF2_CMD_START_FLASH:
        // userspace app should reboot into bootloader on this command; we just ignore it
        break;
    case HF2_CMD_WRITE_FLASH_PAGE:
        assert(sz == 8 + FLASH_ROW_SIZE);
        // first send ACK and then start writing, while getting the next packet
        send_hid_response(pkt, 0, 0);
        if (pkt->buf32[1] >= APP_START_ADDRESS) {
            flash_write_row((void *)pkt->buf32[1], pkt->buf32 + 2);
        }
        return;
    case HF2_CMD_MEM_WRITE_WORDS:
        assert(sz == 8 + FLASH_ROW_SIZE);
        copy_words((void *)pkt->buf32[1], pkt->buf32 + 3, pkt->buf32[2]);
        break;
    case HF2_CMD_MEM_READ_WORDS:
        assert(sz == 12);
        sz = pkt->buf32[2] << 2;
        copy_words(pkt->buf32 + 1, (void *)pkt->buf32[1], pkt->buf32[2]);
        send_hid_response(pkt, 0, sz);
        return;
    case HF2_CMD_CHKSUM_PAGES:
        assert(sz == 12);
        checksum_pages(pkt, pkt->buf32[1], pkt->buf32[2]);
        return;

    default:
        // command not understood
        pkt->buf16[0] |= 0x8000;
        break;
    }

    send_hid_response(pkt, 0, 0);
}

static HID_InBuffer hidbufData;
static HID_InBuffer webbufData;

void process_hid() {
#if USE_HID
    hidbufData.ep = USB_EP_HID;
    process_core(&hidbufData);
#endif
#if USE_WEBUSB
    webbufData.ep = USB_EP_WEB;
    process_core(&webbufData);
#endif
}


#endif