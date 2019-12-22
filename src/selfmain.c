#include "uf2.h"

#include "sam.h"

#if defined(SAMD21)
#define BOOTLOADER_K 8
#elif defined(SAMD51)
#define BOOTLOADER_K 16
#endif

extern const uint8_t bootloader[];
extern const uint16_t bootloader_crcs[];

uint8_t pageBuf[FLASH_ROW_SIZE];

#if defined(SAMD21)
#define NVM_FUSE_ADDR NVMCTRL_AUX0_ADDRESS
#define exec_cmdaddr(cmd, addr)                                 \
    do {                                                        \
        NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;             \
        NVMCTRL->ADDR.reg = (uint32_t)addr / 2;                 \
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | cmd;     \
        while (NVMCTRL->INTFLAG.bit.READY == 0) {               \
        }                                                       \
    } while (0)
#elif defined(SAMD51)
#define NVM_FUSE_ADDR NVMCTRL_FUSES_BOOTPROT_ADDR
#define exec_cmdaddr(cmd, addr)                                 \
    do {                                                        \
        NVMCTRL->ADDR.reg = (uint32_t)addr;                     \
        NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | cmd;     \
        while (NVMCTRL->STATUS.bit.READY == 0) {                \
        }                                                       \
    } while (0)
#endif
#define exec_cmd(cmd) exec_cmdaddr(cmd, NVMCTRL_USER)

void setBootProt(int v) {
    #if defined(SAMD21)
        uint32_t fuses[2], 
                 newfuses[2];
        while (!(NVMCTRL->INTFLAG.reg & NVMCTRL_INTFLAG_READY)) {
        }
    #elif defined(SAMD51)
        uint32_t fuses[128],    // 512 bytes (whole user page)
                 newfuses[5];
        while (!NVMCTRL->STATUS.bit.READY) {
        }
    #endif

    memcpy(fuses, (uint32_t *)NVM_FUSE_ADDR, sizeof(fuses));
    memcpy(newfuses, fuses, sizeof(newfuses)); // Start with new values equal to current

    // Check for damaged fuses. If the NVM user page was accidentally erased, there
    // will be 1's in wrong places. This would enable the watchdog timer and cause other
    // problems. So check for all ones outside of the SAMD21/51 BOOTPROT fields, or all ones
    // in fuses[1]. If it appears the fuses page was erased, replace fuses with reasonable values.
    bool repair_fuses = ((fuses[0] & 0x03fffff0) == 0x03fffff0 || fuses[1] == 0xffffffff);
    #if defined(SAMD51)
        // SAMD51 needs an additional check
        repair_fuses |= (fuses[4] == 0xffffffff);
    #endif
    if (repair_fuses) {
        // These canonical fuse values taken from working Adafruit boards.
        #if defined(SAMD21)
            newfuses[0] = 0xD8E0C7FF;
            newfuses[1] = 0xFFFFFC5D;
        #elif defined(SAMD51)
            newfuses[0] = 0xFE9A9239;
            newfuses[1] = 0xAEECFF80;
            newfuses[2] = 0xFFFFFFFF;
            newfuses[4] = 0x00804010;
        #endif
    }

    uint32_t bootprot = (newfuses[0] & NVMCTRL_FUSES_BOOTPROT_Msk) >> NVMCTRL_FUSES_BOOTPROT_Pos;

    logval("repair_fuses", repair_fuses);
    logval("fuse0", newfuses[0]);
    logval("fuse1", newfuses[1]);
    #if defined(SAMD51)
      logval("fuse2", newfuses[2]);
      logval("fuse4", newfuses[4]);
    #endif
    logval("bootprot", bootprot);
    logval("needed", v);

    // Don't write if nothing will be changed.
    if (bootprot == v && !repair_fuses) {
        return;
    }

    newfuses[0] = (newfuses[0] & ~NVMCTRL_FUSES_BOOTPROT_Msk) | (v << NVMCTRL_FUSES_BOOTPROT_Pos);

    bool format = false;
    for (int i = 0; i < sizeof(newfuses) / sizeof(newfuses[0]); ++i)
        format |= ((newfuses[i] ^ fuses[i]) & newfuses[i]);

    memcpy(fuses, newfuses, sizeof(newfuses)); // Update page buffer with new fuses

    #if defined(SAMD21)
        NVMCTRL->CTRLB.reg = NVMCTRL->CTRLB.reg | NVMCTRL_CTRLB_CACHEDIS | NVMCTRL_CTRLB_MANW;

        if (format) {
            exec_cmd(NVMCTRL_CTRLA_CMD_EAR);
        }
        exec_cmd(NVMCTRL_CTRLA_CMD_PBC);
    #elif defined(SAMD51)
        NVMCTRL->CTRLA.bit.WMODE = NVMCTRL_CTRLA_WMODE_MAN;

        if (format) {
            exec_cmd(NVMCTRL_CTRLB_CMD_EP);
        }
        exec_cmd(NVMCTRL_CTRLB_CMD_PBC);
    #endif

    // 'endWriteIndex' is initialized with bytes that should be written.
    // N.B. every itearation write a quadword, hence may be written more bytes than requested
    const size_t endWriteIndex = format ? sizeof(fuses) : repair_fuses ? sizeof(newfuses) : 4;

#if defined(SAMD21)
    uint32_t *const qwBlockAddr = (uint32_t *const)NVM_FUSE_ADDR;
    memcpy(qwBlockAddr, fuses, endWriteIndex);
    exec_cmdaddr(NVMCTRL_CTRLA_CMD_WAP, qwBlockAddr);
#elif defined(SAMD51)
    for (int i = 0; i < endWriteIndex; i += 16) {
        uint32_t *const qwBlockAddr = (uint32_t *const)(NVM_FUSE_ADDR + i);
        memcpy(qwBlockAddr, &fuses[i], 16);
	exec_cmdaddr(NVMCTRL_CTRLB_CMD_WQW, qwBlockAddr);
    }
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
    // Disable BOOTPROT while updating bootloader.
    setBootProt(7); // 0k - See "Table 22-2 Boot Loader Size" in datasheet.
#endif
#ifdef SAMD51
    // We only need to set the BOOTPROT once on the SAMD51. For updates, we can
    // temporarily turn the protection off instead.
    // setBootProt() will only write BOOTPROT if it is not already correct.
    // setBootProt() will also fix the fuse values if they appear to be all ones.
    setBootProt(13); // 16k. See "Table 25-10 Boot Loader Size" in datasheet.
    exec_cmd(NVMCTRL_CTRLB_CMD_SBPDIS);
    NVMCTRL->CTRLA.bit.CACHEDIS0 = true;
    NVMCTRL->CTRLA.bit.CACHEDIS1 = true;
#endif

    int i;

#ifdef SAMD21
    const uint8_t *ptr = bootloader;
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
#endif

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
    // Re-enable BOOTPROT
    setBootProt(2); // 8k
#endif
    // For the SAMD51, the boot protection will automatically be re-enabled on
    // reset.

    resetIntoBootloader();

    while (1) {
    }
}
