#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define VENDOR_NAME "Radomir Dopieralski"
#define PRODUCT_NAME "PewPew"
#define VOLUME_LABEL "PEWBOOT"
#define INDEX_URL "http://pewpew.rtfd.io"
#define BOARD_ID "SAMD51J19A-PewPew-M4"

#define USB_VID 0x239A
#define USB_PID 0x0034

#define BOARD_NEOPIXEL_COUNT 0

#define BOARD_SCREEN 1

#define BOOT_USART_MODULE SERCOM3
#define BOOT_USART_MASK APBAMASK
#define BOOT_USART_BUS_CLOCK_INDEX MCLK_APBBMASK_SERCOM3
#define BOOT_USART_PAD_SETTINGS UART_RX_PAD1_TX_PAD0
#define BOOT_USART_PAD3 PINMUX_UNUSED
#define BOOT_USART_PAD2 PINMUX_UNUSED
#define BOOT_USART_PAD1 PINMUX_PA22C_SERCOM3_PAD0
#define BOOT_USART_PAD0 PINMUX_PA23C_SERCOM3_PAD1
#define BOOT_GCLK_ID_CORE SERCOM3_GCLK_ID_CORE
#define BOOT_GCLK_ID_SLOW SERCOM3_GCLK_ID_SLOW

#define HAS_CONFIG_DATA 1

// This configuration data should be edited at https://microsoft.github.io/uf2/patcher/
// Just drop this file there.
// Alternatively, it can be also binary edited there after the bootloader is compiled.

#ifdef DEFINE_CONFIG_DATA
const uint32_t config_data[] = {
    /* CF2 START */
    513675505, 539130489, // magic
    23, 100,  // used entries, total entries
    4, 0xa, // PIN_BTN_A = PA10
    5, 0x9, // PIN_BTN_B = PA09
    26, 0x2, // PIN_SPEAKER_AMP = PA02
    32, 0xd, // PIN_DISPLAY_SCK = PA13
    34, 0xf, // PIN_DISPLAY_MOSI = PA15
    35, 0xb, // PIN_DISPLAY_CS = PA11
    36, 0x10, // PIN_DISPLAY_DC = PA16
    37, 0xa0, // DISPLAY_WIDTH = 160
    38, 0x80, // DISPLAY_HEIGHT = 128
    39, 0x80, // DISPLAY_CFG0 = 0x80
    40, 0x12c2d, // DISPLAY_CFG1 = 0x12c2d
    41, 0x18, // DISPLAY_CFG2 = 0x18
    43, 0x11, // PIN_DISPLAY_RST = PA17
    47, 0x37, // PIN_BTN_LEFT = PB23
    48, 0x36, // PIN_BTN_RIGHT = PB22
    49, 0x17, // PIN_BTN_UP = PA23
    50, 0x1b, // PIN_BTN_DOWN = PA27
    51, 0x16, // PIN_BTN_MENU = PA22
    204, 0x80000, // FLASH_BYTES = 0x80000
    205, 0x30000, // RAM_BYTES = 0x30000
    208, 0x470c08e2, // BOOTLOADER_BOARD_ID = 0x470c08e2
    209, 0x55114460, // UF2_FAMILY = ATSAMD51
    210, 0x20, // PINS_PORT_SIZE = PA_32
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* CF2 END */
};
#endif

#endif



