#include "uf2.h"

#include "sam.h"

#ifdef SAMD21
#define BOOTLOADER_K 8
#endif
#ifdef SAMD51
#define BOOTLOADER_K 16
#endif

extern const uint8_t bootloader[];
extern const uint16_t bootloader_crcs[];

uint8_t pageBuf[FLASH_ROW_SIZE];

#ifdef SAMD21
#define NVM_FUSE_ADDR NVMCTRL_AUX0_ADDRESS
#define exec_cmd(cmd)                                                          \
    do {                                                                       \
        NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;                            \
        NVMCTRL->ADDR.reg = (uint32_t)NVMCTRL_USER / 2;                        \
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | cmd;                    \
        while (NVMCTRL->INTFLAG.bit.READY == 0) {}                             \
    } while (0)
#endif
#ifdef SAMD51
#define NVM_FUSE_ADDR NVMCTRL_FUSES_BOOTPROT_ADDR
#define exec_cmd(cmd)                                                          \
    do {                                                                       \
        NVMCTRL->ADDR.reg = (uint32_t)NVMCTRL_USER;                        \
        NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | cmd;                    \
        while (NVMCTRL->STATUS.bit.READY == 0) {}                              \
    } while (0)
#endif

void setBootProt(int v) {
    uint32_t fuses[2];

    #ifdef SAMD21
    while (!(NVMCTRL->INTFLAG.reg & NVMCTRL_INTFLAG_READY)) {}
    #endif
    #ifdef SAMD51
    while (NVMCTRL->STATUS.bit.READY == 0) {}
    #endif

    fuses[0] = *((uint32_t *)NVM_FUSE_ADDR);
    fuses[1] = *(((uint32_t *)NVM_FUSE_ADDR) + 1);

    uint32_t bootprot = (fuses[0] & NVMCTRL_FUSES_BOOTPROT_Msk) >> NVMCTRL_FUSES_BOOTPROT_Pos;

    logval("fuse0", fuses[0]);
    logval("fuse1", fuses[1]);
    logval("bootprot", bootprot);
    logval("needed", v);

    if (bootprot == v)
        return;

    fuses[0] = (fuses[0] & ~NVMCTRL_FUSES_BOOTPROT_Msk) | (v << NVMCTRL_FUSES_BOOTPROT_Pos);

    #ifdef SAMD21
    NVMCTRL->CTRLB.reg = NVMCTRL->CTRLB.reg | NVMCTRL_CTRLB_CACHEDIS | NVMCTRL_CTRLB_MANW;

    exec_cmd(NVMCTRL_CTRLA_CMD_EAR);
    exec_cmd(NVMCTRL_CTRLA_CMD_PBC);
    #endif
    #ifdef SAMD51
    NVMCTRL->CTRLA.bit.WMODE = NVMCTRL_CTRLA_WMODE_MAN;

    exec_cmd(NVMCTRL_CTRLB_CMD_EP);
    exec_cmd(NVMCTRL_CTRLB_CMD_PBC);
    #endif

    *((uint32_t *)NVM_FUSE_ADDR) = fuses[0];
    *(((uint32_t *)NVM_FUSE_ADDR) + 1) = fuses[1];

    #ifdef SAMD21
    exec_cmd(NVMCTRL_CTRLA_CMD_WAP);
    #endif
    #ifdef SAMD51
    exec_cmd(NVMCTRL_CTRLB_CMD_WQW);
    #endif

    resetIntoApp();
}

int main(void) {
    led_init();

    logmsg("Start");

    assert((8 << NVMCTRL->PARAM.bit.PSZ) == FLASH_PAGE_SIZE);
    // assert(FLASH_PAGE_SIZE * NVMCTRL->PARAM.bit.NVMP == FLASH_SIZE);

    /* We have determined we should stay in the monitor. */
    /* System initialization */
    system_init();
    __disable_irq();
    __DMB();

    logmsg("Before main loop");

    #ifdef SAMD21
    setBootProt(7); // 0k
    #endif
    #ifdef SAMD51
    // We only need to set the BOOTPROT once on the SAMD51. For updates, we can
    // temporarily turn the protection off instead.
    if (NVMCTRL->STATUS.bit.BOOTPROT != 13) {
        setBootProt(13); // 16k
    }
    exec_cmd(NVMCTRL_CTRLB_CMD_SBPDIS);
    NVMCTRL->CTRLA.bit.CACHEDIS0 = true;
    NVMCTRL->CTRLA.bit.CACHEDIS1 = true;
    #endif

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

    // re-base int vector back to bootloader, so that the flash erase below doesn't write over the
    // vectors
    SCB->VTOR = 0;

    // Write zeros to the stack location and reset handler location so the
    // bootloader doesn't run us a second time. We don't need to erase to write
    // zeros. The remainder of the write unit will be set to 1s which should
    // preserve the existing values but its not critical.
    uint32_t zeros[2] = {0, 0};
    flash_write_words((void *)(BOOTLOADER_K * 1024), zeros, 2);

    for (i = 0; i < 20; ++i) {
        LED_MSC_TGL();
        delay(1000);
    }

    LED_MSC_OFF();

    #ifdef SAMD21
    setBootProt(2); // 8k
    #endif
    // For the SAMD51, the boot protection will automatically be re-enabled on
    // reset.

    resetIntoBootloader();

    while (1) {
    }
}
