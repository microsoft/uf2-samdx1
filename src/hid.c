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
        HF2_Command cmd;
        HF2_Response resp;
    };
} HID_InBuffer;

int recv_hid(HID_InBuffer *pkt) {
    if (!USB_ReadCore(NULL, 64, pkt->ep, &pkt->pbuf))
        return 0; // no data

    assert(pkt->pbuf.ptr == 0 && pkt->pbuf.size > 0);
    pkt->pbuf.size = 0;
    uint8_t tag = pkt->pbuf.buf[0];
    // serial packets not allowed when in middle of command packet
    assert(pkt->size == 0 || !(tag & HF2_FLAG_SERIAL_OUT));
    memcpy(pkt->buf + pkt->size, pkt->pbuf.buf + 1, tag & HF2_SIZE_MASK);
    pkt->size += tag & HF2_SIZE_MASK;
    assert(pkt->size <= sizeof(pkt->buf));
    tag &= HF2_FLAG_MASK;
    if (tag != HF2_FLAG_CMDPKT_BODY) {
        pkt->serial = tag - 0x40;
        int sz = pkt->size;
        pkt->size = 0;
        return sz;
    }
    return 0;
}

void send_hid(const void *data, int size, int ep, int flag) {
    uint8_t buf[64];
    const uint8_t *ptr = data;

    for (;;) {
        int s = 63;
        if (size <= 63) {
            s = size;
            if (flag == HF2_FLAG_CMDPKT_BODY)
                flag = HF2_FLAG_CMDPKT_LAST;
        }
        buf[0] = flag | s;
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
        memcpy(pkt->resp.data8, data, size);
    send_hid(pkt->buf, 4 + size, pkt->ep, HF2_FLAG_CMDPKT_BODY);
}

static void checksum_pages(HID_InBuffer *pkt, int start, int num) {
    for (int i = 0; i < num; ++i) {
        uint8_t *data = (uint8_t *)start + i * FLASH_ROW_SIZE;
        uint16_t crc = 0;
        for (int j = 0; j < FLASH_ROW_SIZE; ++j) {
            crc = add_crc(*data++, crc);
        }
        pkt->resp.data16[i] = crc;
    }
    send_hid_response(pkt, 0, num * 2);
}

void process_core(HID_InBuffer *pkt) {
    int sz = recv_hid(pkt);

    if (!sz)
        return;

    if (pkt->serial) {
#if USE_LOGS
        if (pkt->buf[0] == 'L') {
            send_hid(logStoreUF2.buffer, logStoreUF2.ptr, pkt->ep, HF2_FLAG_SERIAL_OUT);
        } else
#endif
        {
            send_hid("OK\n", 3, pkt->ep, HF2_FLAG_SERIAL_ERR);
        }
        return;
    }

    logwrite("HID sz=");
    logwritenum(sz);
    logval(" CMD", pkt->buf32[0]);

    // one has to be careful dealing with these, as they share memory
    HF2_Command *cmd = &pkt->cmd;
    HF2_Response *resp = &pkt->resp;

    uint32_t cmdId = cmd->command_id;
    uint32_t tmp;
    resp->tag = cmd->tag;
    resp->status16 = HF2_STATUS_OK;

#define checkDataSize(str, add) assert(sz == 8 + sizeof(cmd->str) + (add))

    switch (cmdId) {
    case HF2_CMD_INFO:
        send_hid_response(pkt, infoUf2File, strlen(infoUf2File));
        return;

    case HF2_CMD_BININFO:
        resp->bininfo.mode = HF2_MODE_BOOTLOADER;
        resp->bininfo.flash_page_size = FLASH_ROW_SIZE;
        resp->bininfo.flash_num_pages = FLASH_SIZE / FLASH_ROW_SIZE;
        resp->bininfo.max_message_size = sizeof(pkt->buf);
        send_hid_response(pkt, 0, sizeof(resp->bininfo));
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
        checkDataSize(write_flash_page, FLASH_ROW_SIZE);
        // first send ACK and then start writing, while getting the next packet
        send_hid_response(pkt, 0, 0);
        if (cmd->write_flash_page.target_addr >= APP_START_ADDRESS) {
            flash_write_row((void *)cmd->write_flash_page.target_addr, cmd->write_flash_page.data);
        }
        return;
    case HF2_CMD_WRITE_WORDS:
        checkDataSize(write_words, cmd->write_words.num_words << 2);
        copy_words((void *)cmd->write_words.target_addr, cmd->write_words.words,
                   cmd->write_words.num_words);
        break;
    case HF2_CMD_READ_WORDS:
        checkDataSize(read_words, 0);
        tmp = cmd->read_words.num_words;
        copy_words(resp->data32, (void *)cmd->read_words.target_addr, tmp);
        send_hid_response(pkt, 0, tmp << 2);
        return;
    case HF2_CMD_CHKSUM_PAGES:
        checkDataSize(chksum_pages, 0);
        checksum_pages(pkt, cmd->chksum_pages.target_addr, cmd->chksum_pages.num_pages);
        return;

    default:
        // command not understood
        resp->status16 = HF2_STATUS_INVALID_CMD;
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