#include "uf2.h"

#if USE_HID

void process_hid(void) {
    uint8_t buf[64] = {0};
    if (!USB_Read(NULL, 64, USB_EP_HID))
        return; // no data

    USB_Read(buf, 64, USB_EP_HID);
    logval("buf0", buf[0]);
    logval("buf1", buf[1]);
    logval("buf2", buf[2]);
}

#endif