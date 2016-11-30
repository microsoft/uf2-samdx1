#include "uf2.h"

#define BOOTLOADER_K 8

extern const uint8_t bootloader[];
extern const uint16_t bootloader_crcs[];

uint8_t pageBuf[FLASH_ROW_SIZE];

int main(void) {
    led_init();

    logmsg("Start");

    assert(8 << NVMCTRL->PARAM.bit.PSZ == FLASH_PAGE_SIZE);
    // assert(FLASH_PAGE_SIZE * NVMCTRL->PARAM.bit.NVMP == FLASH_SIZE);

    /* We have determined we should stay in the monitor. */
    /* System initialization */
    system_init();
    cpu_irq_enable();

    logmsg("Before main loop");

    const uint8_t *ptr = bootloader;
    int i;

    for (i = 0; i < BOOTLOADER_K; ++i) {
        int crc = 0;
        for (int j = 0; j < 1024; ++j) {
            crc = add_crc(*ptr++, crc);
        }
        if (bootloader_crcs[i] != crc) {
            logmsg("Invalid checksum. Aborting.");
            panic();
        }
    }

    for (i = 0; i < BOOTLOADER_K * 1024; i += FLASH_ROW_SIZE) {
        memcpy(pageBuf, &bootloader[i], FLASH_ROW_SIZE);
        flash_write_row((void *)i, (void*)pageBuf);
    }

    logmsg("Update successful!");

    // erase first row of this updater app, so the bootloader doesn't start us again
    flash_erase_row((void*)i);

    for (i = 0; i < 20; ++i) {
        LED_MSC_TGL();
        delay(200);
    }

    LED_MSC_OFF();

    resetIntoBootloader();

    while (1) {
    }
}
