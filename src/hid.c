#include "uf2.h"

#if USE_HID

typedef struct {
    PacketBuffer pbuf;
    uint16_t size;
    uint8_t serial;
    uint8_t padding;
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
        logval("recv", sz);
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

void send_hid_response(const void *data, int size) {
    logval("sendresp", size);
    memcpy(hidbuf.buf32 + 2, data, size);
    hidbuf.buf32[1] = 0;
    send_hid(hidbuf.buf, 8 + size);
}

static void checksum_pages(int start, int num) {
    for (int i = 0; i < num; ++i) {
        uint8_t *data = (uint8_t *)start + i * FLASH_ROW_SIZE;
        uint16_t crc = 0;
        for (int j = 0; j < FLASH_ROW_SIZE; ++j) {
            crc = add_crc(*data++, crc);
        }
        hidbuf.buf16[i + 4] = crc;
    }
    hidbuf.buf32[1] = 0;
    send_hid_response(hidbuf.buf32 + 2, num * 2);
}

void process_hid(void) {
    int sz = recv_hid(&hidbuf);

    if (!sz)
        return;

    if (hidbuf.serial) {
        // echo back serial input - just for testing
        hidbuf.pbuf.buf[1] ^= 'a' - 'A';
        hidbuf.pbuf.buf[2] ^= 'a' - 'A';
        hidbuf.pbuf.buf[3] ^= 'a' - 'A';
        USB_WriteCore(hidbuf.pbuf.buf, 64, USB_EP_HID, true);
        return;
    }

    logval("HID", hidbuf.buf32[0]);
    uint32_t resp[] = {hidbuf.buf32[0], HF2_STATUS_OK};

    switch (hidbuf.buf16[0]) {
    case HF2_CMD_INFO:
        send_hid_response(infoUf2File, strlen(infoUf2File));
        return;

    case HF2_CMD_BININFO:
        hidbuf.buf32[2] = HF2_MODE_BOOTLOADER; // bootloader
        hidbuf.buf32[3] = FLASH_ROW_SIZE;
        send_hid_response(hidbuf.buf32 + 2, 8);
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
        send_hid(resp, 8); // first send ACK and then start writing, while getting the next packet
        if (hidbuf.buf32[1] >= APP_START_ADDRESS) {
            flash_write_row((void *)hidbuf.buf32[1], hidbuf.buf32 + 2);
        }
        return;
    case HF2_CMD_MEM_WRITE_PAGE:
        assert(sz == 8 + FLASH_ROW_SIZE);
        memcpy((void *)hidbuf.buf32[1], hidbuf.buf + 8, FLASH_ROW_SIZE);
        break;
    case HF2_CMD_MEM_READ_PAGE:
        assert(sz == 8);
        send_hid_response((void *)hidbuf.buf32[1], FLASH_ROW_SIZE);
        return;
    case HF2_CMD_CHKSUM_PAGES:
        checksum_pages(hidbuf.buf32[1], hidbuf.buf32[2]);
        return;

    default:
        resp[1] = HF2_STATUS_INVALID_CMD;
        break;
    }

    send_hid(resp, 8);
}

#endif