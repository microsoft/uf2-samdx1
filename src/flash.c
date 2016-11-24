#include "uf2.h"

static void wait_ready(void) {
    while (NVMCTRL->INTFLAG.bit.READY == 0) {
    }
}

void flash_erase_row(uint32_t *dst) {
    wait_ready();
    NVMCTRL->STATUS.reg = NVMCTRL_STATUS_MASK;

    // Execute "ER" Erase Row
    NVMCTRL->ADDR.reg = (uint32_t)dst / 2;
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
    wait_ready();
}

void flash_write_words(uint32_t *dst, uint32_t *src, uint32_t n_words) {
    // Set automatic page write
    NVMCTRL->CTRLB.bit.MANW = 0;

    while (n_words > 0) {
        // Execute "PBC" Page Buffer Clear
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
        wait_ready();

        uint32_t len = min(FLASH_PAGE_SIZE >> 2, n_words);
        n_words -= len;

        while (len--)
            *dst++ = *src++;

        // Execute "WP" Write Page
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
        wait_ready();
    }
}

// only disable for debugging/timing
#define QUICK_FLASH 1

void flash_write_row(uint32_t *dst, uint32_t *src) {
#if QUICK_FLASH
    for (int i = 0; i < FLASH_ROW_SIZE / 4; ++i)
        if (src[i] != dst[i])
            goto doflash;
    return;

doflash:
#endif

    flash_erase_row(dst);
    flash_write_words(dst, src, FLASH_ROW_SIZE / 4);
}
