#if defined(__SAMD51__)
#error "update_bootloader*.ino is not available for SAMD51 boards"
#endif

#define BOOTLOADER_K 8

// Error indications:
// 2 quick flashes repeated forever: Flash page size wrong
// 3: checksum error
// 4: write verify failed
//
// Success: 5 slower flashes, then switch to BOOT drive

static uint16_t crcCache[256];

#define CRC16POLY 0x1021

#define FLASH_ROW_SIZE (FLASH_PAGE_SIZE * 4)

uint16_t add_crc(uint8_t ch, unsigned short crc0) {
    if (!crcCache[1]) {
        for (int ptr = 0; ptr < 256; ptr++) {
            uint16_t crc = (int)ptr << 8;
            for (uint16_t cmpt = 0; cmpt < 8; cmpt++) {
                if (crc & 0x8000)
                    crc = crc << 1 ^ CRC16POLY;
                else
                    crc = crc << 1;
            }
            crcCache[ptr] = crc;
        }
    }

    return ((crc0 << 8) ^ crcCache[((crc0 >> 8) ^ ch) & 0xff]) & 0xffff;
}

#define NVM_USER_MEMORY ((volatile uint16_t *)NVMCTRL_USER)

static inline void wait_ready(void) {
    while (NVMCTRL->INTFLAG.bit.READY == 0) {
    }
}

void exec_cmd(uint32_t cmd, const uint32_t *addr) {
    NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;
    NVMCTRL->ADDR.reg = (uint32_t)addr / 2;
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | cmd;
    wait_ready();
}

void flash_erase_row(uint32_t *dst) {
    wait_ready();
    // Execute "ER" Erase Row
    exec_cmd(NVMCTRL_CTRLA_CMD_ER, dst);
}

void flash_write_words(uint32_t *dst, uint32_t *src, uint32_t n_words) {
    while (n_words > 0) {
        uint32_t len = min(FLASH_PAGE_SIZE >> 2, n_words);
        n_words -= len;

        // Execute "PBC" Page Buffer Clear
        const uint32_t* dst_start = dst;
        exec_cmd(NVMCTRL_CTRLA_CMD_PBC, dst);

        // Write data to page buffer.
        while (len--) {
            *dst++ = *src++;
        }

        // Execute "WP" Write Page
        exec_cmd(NVMCTRL_CTRLA_CMD_WP, dst_start);
    }
}

void flash_write_row(uint32_t *dst, uint32_t *src) {
    flash_erase_row(dst);
    flash_write_words(dst, src, FLASH_ROW_SIZE / 4);
}

void setBootProt(int v) {
    uint32_t *NVM_FUSES = (uint32_t *)NVMCTRL_AUX0_ADDRESS;

    uint32_t fuses[2];

    fuses[0] = NVM_FUSES[0];
    fuses[1] = NVM_FUSES[1];

    uint32_t bootprot = (fuses[0] & NVMCTRL_FUSES_BOOTPROT_Msk) >> NVMCTRL_FUSES_BOOTPROT_Pos;

    if (bootprot == v) {
        return;
    }

    fuses[0] = (fuses[0] & ~NVMCTRL_FUSES_BOOTPROT_Msk) | (v << NVMCTRL_FUSES_BOOTPROT_Pos);

    wait_ready();
    exec_cmd(NVMCTRL_CTRLA_CMD_EAR, (uint32_t *)NVMCTRL_USER);
    exec_cmd(NVMCTRL_CTRLA_CMD_PBC, (uint32_t *)NVMCTRL_USER);

    NVM_FUSES[0] = fuses[0];
    NVM_FUSES[1] = fuses[1];

    exec_cmd(NVMCTRL_CTRLA_CMD_WAP, (uint32_t *)NVMCTRL_USER);

    NVIC_SystemReset();
}

// We can't use regular loop_delay() because it uses interrupts, which we turn off.
void loop_delay(int ms) {
    ms <<= 13;
    while (ms--) {
        asm("nop");
    }
}

void blink_n(int n, int interval) {
    // Start out off.
    digitalWrite(LED_BUILTIN, LOW);
    loop_delay(interval);
    for (int i = 0; i < n; ++i) {
        digitalWrite(LED_BUILTIN, HIGH);
        loop_delay(interval);
        digitalWrite(LED_BUILTIN, LOW);
        loop_delay(interval);
    }
}

void blink_n_forever(int n, int interval) {
    while(1) {
        blink_n(n, interval);
        loop_delay(interval*5);
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    if (8 << NVMCTRL->PARAM.bit.PSZ != FLASH_PAGE_SIZE) {
        blink_n_forever(2, 200);
    }

    NVMCTRL->CTRLB.bit.MANW = 1;
    NVMCTRL->CTRLB.bit.CACHEDIS = 1;

    __disable_irq();

    // This will cause a reset and re-enter the program if a change was necessary.
    // If no change was necessary we'll fall through.
    setBootProt(7); // 0kB; disable BOOTPROT while writing.

    const uint8_t *ptr = bootloader;

    for (int i = 0; i < BOOTLOADER_K; ++i) {
        int crc = 0;
        for (int j = 0; j < 1024; ++j) {
            crc = add_crc(*ptr++, crc);
        }
        if (bootloader_crcs[i] != crc) {
            blink_n_forever(3, 200);
        }
    }

    // Writing starts at 0x0, so flash_addr can be used as an index into bootloader[].
    for (int flash_addr = 0; flash_addr < BOOTLOADER_K * 1024; flash_addr += FLASH_ROW_SIZE) {
        flash_write_row((uint32_t *)flash_addr, (uint32_t *)&bootloader[flash_addr]);
        if (memcmp((const void *)flash_addr, &bootloader[flash_addr], FLASH_ROW_SIZE) != 0) {
            // Write verify failed.
            blink_n_forever(4, 200);
        }
    }

    // re-base int vector back to bootloader, so that the flash erase below doesn't write over the
    // vectors
    SCB->VTOR = 0;

    blink_n(5, 750);

    // Write zeros to the stack location and reset handler location so the
    // bootloader doesn't run us a second time. We don't need to erase to write
    // zeros. The remainder of the write unit will be set to 1s which should
    // preserve the existing values but it's not critical.
    uint32_t zeros[2] = {0, 0};
    flash_write_words((uint32_t *)(BOOTLOADER_K * 1024), zeros, 2);

    setBootProt(2); // 8kB

}

void loop() {
}
