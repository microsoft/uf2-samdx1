#include "uf2.h"

#define BOOTLOADER_K 8

extern const uint8_t bootloader[];
extern const uint16_t bootloader_crcs[];

uint8_t pageBuf[FLASH_ROW_SIZE];

#define exec_cmd(cmd)                                                                              \
    do {                                                                                           \
        NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;                                                \
        NVMCTRL->ADDR.reg = (uint32_t)NVMCTRL_USER / 2;                                         \
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | cmd;                                        \
        while (NVMCTRL->INTFLAG.bit.READY == 0)                                                    \
            ;                                                                                      \
    } while (0)

void setBootProt(int v) {
    uint32_t fuses[2];

    while (!(NVMCTRL->INTFLAG.reg & NVMCTRL_INTFLAG_READY))
        ;

    fuses[0] = *((uint32_t *)NVMCTRL_AUX0_ADDRESS);
    fuses[1] = *(((uint32_t *)NVMCTRL_AUX0_ADDRESS) + 1);

    uint32_t bootprot = (fuses[0] & NVMCTRL_FUSES_BOOTPROT_Msk) >> NVMCTRL_FUSES_BOOTPROT_Pos;

    logval("fuse0", fuses[0]);
    logval("fuse1", fuses[1]);
    logval("bootprot", bootprot);
    logval("needed", v);

    if (bootprot == v)
        return;

    fuses[0] = (fuses[0] & ~NVMCTRL_FUSES_BOOTPROT_Msk) | (v << NVMCTRL_FUSES_BOOTPROT_Pos);

    NVMCTRL->CTRLB.reg = NVMCTRL->CTRLB.reg | NVMCTRL_CTRLB_CACHEDIS | NVMCTRL_CTRLB_MANW;

    exec_cmd(NVMCTRL_CTRLA_CMD_EAR);
    exec_cmd(NVMCTRL_CTRLA_CMD_PBC);

    *((uint32_t *)NVMCTRL_AUX0_ADDRESS) = fuses[0];
    *(((uint32_t *)NVMCTRL_AUX0_ADDRESS) + 1) = fuses[1];

    exec_cmd(NVMCTRL_CTRLA_CMD_WAP);

    resetIntoApp();
}

int main(void) {
    led_init();

    logmsg("Start");

    assert(8 << NVMCTRL->PARAM.bit.PSZ == FLASH_PAGE_SIZE);
    // assert(FLASH_PAGE_SIZE * NVMCTRL->PARAM.bit.NVMP == FLASH_SIZE);

    /* We have determined we should stay in the monitor. */
    /* System initialization */
    system_init();
    cpu_irq_disable();

    logmsg("Before main loop");

    setBootProt(7); // 0k

    const uint8_t *ptr = bootloader;
    int i;

    for (i = 0; i < BOOTLOADER_K; ++i) {
        int crc = 0;
        for (int j = 0; j < 1024; ++j) {
            crc = add_crc(*ptr++, crc);
        }
        if (bootloader_crcs[i] != crc) {
            logmsg("Invalid checksum. Aborting.");
            panic(1);
        }
    }

    for (i = 0; i < BOOTLOADER_K * 1024; i += FLASH_ROW_SIZE) {
        memcpy(pageBuf, &bootloader[i], FLASH_ROW_SIZE);
        flash_write_row((void *)i, (void *)pageBuf);
    }

    logmsg("Update successful!");

    for (i = 0; i < 20; ++i) {
        LED_MSC_TGL();
        delay(1000);
    }

    // re-base int vector back to bootloader, so that the flash erase below doesn't write over the
    // vectors
    SCB->VTOR = 0;

    // erase first row of this updater app, so the bootloader doesn't start us again
    flash_erase_row((void *)(BOOTLOADER_K * 1024));

    LED_MSC_OFF();

    setBootProt(2); // 8k


    resetIntoBootloader();

    while (1) {
    }
}
