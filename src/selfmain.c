#include "uf2.h"

#include "sam.h"

#if defined(SAMD21)
#define BOOTLOADER_K 8
#elif defined(SAMD51)
#define BOOTLOADER_K 16
#endif

extern const uint8_t bootloader[];
extern const uint16_t bootloader_crcs[];

uint8_t bootloader_page_buf[FLASH_ROW_SIZE];

#if defined(SAMD21)
#define NVM_FUSE_ADDR ((uint32_t *)NVMCTRL_AUX0_ADDRESS)
#elif defined(SAMD51)
#define NVM_FUSE_ADDR ((uint32_t *)NVMCTRL_FUSES_BOOTPROT_ADDR)
#endif

static inline void nvmctrl_wait_ready(void) {
#if defined(SAMD21)
    while (NVMCTRL->INTFLAG.bit.READY == 0) { }
#elif defined(SAMD51)
    while (NVMCTRL->STATUS.bit.READY == 0) { }
#endif
}

static inline void nvmctrl_set_addr(const uint32_t *addr) {
#if defined(SAMD21)
    NVMCTRL->ADDR.reg = (uint32_t)addr / 2;
#elif defined(SAMD51)
    NVMCTRL->ADDR.reg = (uint32_t)addr;
#endif
}

static inline void nvmctrl_exec_cmd(uint32_t cmd) {
#if defined(SAMD21)
    NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;  // Clear error status bits.
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | cmd;
    nvmctrl_wait_ready();
#elif defined(SAMD51)
    NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | cmd;
#endif
    nvmctrl_wait_ready();
}

void set_fuses_and_bootprot(uint32_t new_bootprot) {
#if defined(SAMD21)
    uint32_t fuses[2];
#elif defined(SAMD51)
    uint32_t fuses[128];    // 512 bytes (whole user page)
#endif
    nvmctrl_wait_ready();

    memcpy(fuses, (uint32_t *)NVM_FUSE_ADDR, sizeof(fuses));

    // If it appears the fuses page was erased (all ones), replace fuses with reasonable values.

#if defined(SAMD21)
    bool repair_fuses = (fuses[0] == 0xffffffff ||
                         fuses[1] == 0xffffffff);
#elif defined(SAMD51)
    bool repair_fuses = (fuses[0] == 0xffffffff ||
                         fuses[1] == 0xffffffff ||
                         fuses[4] == 0xffffffff);
#endif

    if (repair_fuses) {
        // These canonical fuse values taken from working Adafruit boards.
        // BOOTPROT is set to nothing in these values.
#if defined(SAMD21)
        fuses[0] = 0xD8E0C7FF;
        fuses[1] = 0xFFFFFC5D;
#elif defined(SAMD51)
        fuses[0] = 0xFE9A9239;
        fuses[1] = 0xAEECFF80;
        fuses[2] = 0xFFFFFFFF;
        // fuses[3] is for user use, so we don't change it.
        fuses[4] = 0x00804010;
#endif
    }

    uint32_t current_bootprot = (fuses[0] & NVMCTRL_FUSES_BOOTPROT_Msk) >> NVMCTRL_FUSES_BOOTPROT_Pos;

    logval("repair_fuses", repair_fuses);
    logval("current_bootprot", current_bootprot);
    logval("new_bootprot", new_bootprot);

    // Don't write if nothing will be changed.
    if (current_bootprot == new_bootprot && !repair_fuses) {
        return;
    }

    // Update fuses BOOTPROT value with desired value.
    fuses[0] = (fuses[0] & ~NVMCTRL_FUSES_BOOTPROT_Msk) | (new_bootprot << NVMCTRL_FUSES_BOOTPROT_Pos);

    // Write the fuses.

#if defined(SAMD21)
    NVMCTRL->CTRLB.reg = NVMCTRL->CTRLB.reg | NVMCTRL_CTRLB_CACHEDIS | NVMCTRL_CTRLB_MANW;
    nvmctrl_set_addr(NVM_FUSE_ADDR);  // Set address to auxiliary row (fuses).
    nvmctrl_exec_cmd(NVMCTRL_CTRLA_CMD_EAR);  // Erase auxiliary row.
    nvmctrl_exec_cmd(NVMCTRL_CTRLA_CMD_PBC);  // Clear page buffer (64 bytes).
    // Writes must be 16 or 32 bits at a time.
    NVM_FUSE_ADDR[0] = fuses[0];
    NVM_FUSE_ADDR[1] = fuses[1];
    nvmctrl_exec_cmd(NVMCTRL_CTRLA_CMD_WAP);
#elif defined(SAMD51)
    NVMCTRL->CTRLA.bit.WMODE = NVMCTRL_CTRLA_WMODE_MAN;
    nvmctrl_set_addr(NVM_FUSE_ADDR);  // Set address to user page.
    nvmctrl_exec_cmd(NVMCTRL_CTRLB_CMD_EP);   // Erase user page.
    nvmctrl_exec_cmd(NVMCTRL_CTRLB_CMD_PBC);  // Clear page buffer.
    for (size_t i = 0; i < sizeof(fuses) / sizeof(uint32_t); i += 4) {
        // Copy a quadword, one 32-bit word at a time. Writes to page
        // buffer must be 16 or 32 bits at a time, so we use explicit
        // word writes
        NVM_FUSE_ADDR[i + 0] = fuses[i + 0];
        NVM_FUSE_ADDR[i + 1] = fuses[i + 1];
        NVM_FUSE_ADDR[i + 2] = fuses[i + 2];
        NVM_FUSE_ADDR[i + 3] = fuses[i + 3];
        nvmctrl_set_addr(&NVM_FUSE_ADDR[i]); // Set write address to the current quad word.
	nvmctrl_exec_cmd(NVMCTRL_CTRLB_CMD_WQW); // Write quad word.
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
    set_fuses_and_bootprot(7); // 0k - See "Table 22-2 Boot Loader Size" in datasheet.
#endif
#ifdef SAMD51
    // set_fuses_and_bootprot() will cause a reset and not return if
    // the fuses are changed. We'll reenter main() and run this again,
    // and it will do nothing the second time and fall hrough.
    set_fuses_and_bootprot(13); // 16k. See "Table 25-10 Boot Loader Size" in datasheet.

    // We only need to set the BOOTPROT once on the SAMD51. For updates, we can
    // temporarily turn the protection off instead.
    nvmctrl_exec_cmd(NVMCTRL_CTRLB_CMD_SBPDIS);
    // Disable NVM caches, per errata.
    NVMCTRL->CTRLA.bit.CACHEDIS0 = true;
    NVMCTRL->CTRLA.bit.CACHEDIS1 = true;
#endif

#ifdef SAMD21
    const uint8_t *ptr = bootloader;
    for (uint32_t i = 0; i < BOOTLOADER_K; ++i) {
        int crc = 0;
        for (uint32_t j = 0; j < 1024; ++j) {
            crc = add_crc(*ptr++, crc);
        }
        if (bootloader_crcs[i] != crc) {
            logmsg("Invalid checksum. Aborting.");
            panic(1);
        }
    }
#endif

    for (uint32_t i = 0; i < BOOTLOADER_K * 1024; i += FLASH_ROW_SIZE) {
        memcpy(bootloader_page_buf, &bootloader[i], FLASH_ROW_SIZE);
        flash_write_row((void *)i, (void *)bootloader_page_buf);
    }

    logmsg("Update successful!");

    // re-base int vector back to bootloader, so that the flash erase
    // below doesn't write over the vectors.
    SCB->VTOR = 0;

    // Write zeros to the stack location and reset handler location so the
    // bootloader doesn't run us a second time. We don't need to erase to write
    // zeros. The remainder of the write unit will be set to 1s which should
    // preserve the existing values but its not critical.
    uint32_t zeros[2] = {0, 0};
    flash_write_words((void *)(BOOTLOADER_K * 1024), zeros, 2);

    for (uint32_t i = 0; i < 8; ++i) {
        LED_MSC_TGL();
        delay(1000);
    }

    LED_MSC_OFF();

#ifdef SAMD21
    // Re-enable BOOTPROT
    set_fuses_and_bootprot(2); // 8k
#endif
    // For the SAMD51, the boot protection will automatically be re-enabled on
    // reset.

    resetIntoBootloader();

    // We should not reach here normally.
    while (1) {
    }
}
