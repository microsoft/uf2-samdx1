#define BOOTLOADER_K 8

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

uint8_t pageBuf[FLASH_ROW_SIZE];

#define NVM_USER_MEMORY ((volatile uint16_t *)NVMCTRL_USER)

static inline void wait_ready(void) {
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
        uint32_t len = min(FLASH_PAGE_SIZE >> 2, n_words);
        n_words -= len;

        // Execute "PBC" Page Buffer Clear
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
        wait_ready();

        // make sure there are no other memory writes here
        // otherwise we get lock-ups

        while (len--)
            *dst++ = *src++;

        // Execute "WP" Write Page
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
        wait_ready();
    }
}

void flash_write_row(uint32_t *dst, uint32_t *src) {
    flash_erase_row(dst);
    flash_write_words(dst, src, FLASH_ROW_SIZE / 4);
}

#define exec_cmd(cmd)                                                                              \
    do {                                                                                           \
        NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;                                                \
        NVMCTRL->ADDR.reg = (uint32_t)NVMCTRL_USER / 2;                                            \
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

    if (bootprot == v)
        return;

    fuses[0] = (fuses[0] & ~NVMCTRL_FUSES_BOOTPROT_Msk) | (v << NVMCTRL_FUSES_BOOTPROT_Pos);

    NVMCTRL->CTRLB.reg = NVMCTRL->CTRLB.reg | NVMCTRL_CTRLB_CACHEDIS | NVMCTRL_CTRLB_MANW;

    exec_cmd(NVMCTRL_CTRLA_CMD_EAR);
    exec_cmd(NVMCTRL_CTRLA_CMD_PBC);

    *((uint32_t *)NVMCTRL_AUX0_ADDRESS) = fuses[0];
    *(((uint32_t *)NVMCTRL_AUX0_ADDRESS) + 1) = fuses[1];

    exec_cmd(NVMCTRL_CTRLA_CMD_WAP);

    NVIC_SystemReset();
}

void mydelay(int ms) {
    ms <<= 13;
    while (ms--) {
        asm("nop");
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    if (8 << NVMCTRL->PARAM.bit.PSZ != FLASH_PAGE_SIZE)
        while (1) {
        }

    __disable_irq();

    setBootProt(7); // 0k

    const uint8_t *ptr = bootloader;
    int i;

    for (i = 0; i < BOOTLOADER_K; ++i) {
        int crc = 0;
        for (int j = 0; j < 1024; ++j) {
            crc = add_crc(*ptr++, crc);
        }
        if (bootloader_crcs[i] != crc) {
            while (1) {
            }
        }
    }

    for (i = 0; i < BOOTLOADER_K * 1024; i += FLASH_ROW_SIZE) {
        memcpy(pageBuf, &bootloader[i], FLASH_ROW_SIZE);
        flash_write_row((uint32_t *)(void *)i, (uint32_t *)(void *)pageBuf);
    }

    // re-base int vector back to bootloader, so that the flash erase below doesn't write over the
    // vectors
    SCB->VTOR = 0;

    // erase first row of this updater app, so the bootloader doesn't start us again
    flash_erase_row((uint32_t *)(void *)(BOOTLOADER_K * 1024));

    for (i = 0; i < 5; ++i) {
        digitalWrite(LED_BUILTIN, HIGH);
        mydelay(100);
        digitalWrite(LED_BUILTIN, LOW);
        mydelay(200);
    }

    setBootProt(2); // 8k

    while (1) {
    }
}

void loop() {}
