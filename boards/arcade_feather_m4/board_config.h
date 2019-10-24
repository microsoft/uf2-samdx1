#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define VENDOR_NAME "Adafruit Industries"
#define PRODUCT_NAME "Feather Arcade D51"
#define VOLUME_LABEL "ARCADE-D5"
#define INDEX_URL "https://arcade.makecode.com/"
#define BOARD_ID "SAMD51J19A-Feather-Arcade-D51"


#define USB_VID 0x239A
#define USB_PID 0x0022

#define LED_PIN PIN_PA23

#define BOARD_NEOPIXEL_PIN PIN_PB03
#define BOARD_NEOPIXEL_COUNT 1

#define BOOT_USART_MODULE SERCOM0
#define BOOT_USART_MASK APBAMASK
#define BOOT_USART_BUS_CLOCK_INDEX MCLK_APBAMASK_SERCOM0
#define BOOT_USART_PAD_SETTINGS UART_RX_PAD3_TX_PAD0
#define BOOT_USART_PAD3 PINMUX_PA07D_SERCOM0_PAD3
#define BOOT_USART_PAD2 PINMUX_UNUSED
#define BOOT_USART_PAD1 PINMUX_UNUSED
#define BOOT_USART_PAD0 PINMUX_PA04D_SERCOM0_PAD0
#define BOOT_GCLK_ID_CORE SERCOM0_GCLK_ID_CORE
#define BOOT_GCLK_ID_SLOW SERCOM0_GCLK_ID_SLOW

#define HAS_CONFIG_DATA 1
#define BOARD_SCREEN 1


// This configuration data should be edited at https://microsoft.github.io/uf2/patcher/
// Just drop this file there.
// Alternatively, it can be also binary edited there after the bootloader is compiled.

#ifdef DEFINE_CONFIG_DATA
const uint32_t config_data[] = {
    /* CF2 START */
    513675505, 539130489, // magic
    53, 100,  // used entries, total entries
    4, 0xd, // PIN_BTN_A = PIN_SCL
    5, 0x12, // PIN_BTN_B = PIN_D6
    13, 0x17, // PIN_LED = PIN_D13
    18, 0x36, // PIN_MISO = PB22
    19, 0x37, // PIN_MOSI = PB23
    20, 0x23, // PIN_NEOPIXEL = PB03
    21, 0x31, // PIN_RX = PB17
    23, 0x11, // PIN_SCK = PA17
    24, 0xd, // PIN_SCL = PA13
    25, 0xc, // PIN_SDA = PA12
    26, 0xe, // PIN_SPEAKER_AMP = PIN_D4
    28, 0x30, // PIN_TX = PB16
    32, 0x11, // PIN_DISPLAY_SCK = PIN_SCK
    33, 0x36, // PIN_DISPLAY_MISO = PIN_MISO
    34, 0x37, // PIN_DISPLAY_MOSI = PIN_MOSI
    35, 0x28, // PIN_DISPLAY_CS = PIN_A2
    36, 0x29, // PIN_DISPLAY_DC = PIN_A3
    37, 0xa0, // DISPLAY_WIDTH = 160
    38, 0x80, // DISPLAY_HEIGHT = 128
    39, 0x90, // DISPLAY_CFG0 = 0x90
    40, 0xe14ff, // DISPLAY_CFG1 = 0xe14ff
    41, 0x18, // DISPLAY_CFG2 = 0x18
    43, 0x4, // PIN_DISPLAY_RST = PIN_A4
    44, 0x6, // PIN_DISPLAY_BL = PIN_A5
    47, 0x15, // PIN_BTN_LEFT = PIN_D11
    48, 0x13, // PIN_BTN_RIGHT = PIN_D9
    49, 0x14, // PIN_BTN_UP = PIN_D10
    50, 0x16, // PIN_BTN_DOWN = PIN_D12
    51, 0xc, // PIN_BTN_MENU = PIN_SDA
    59, 0x200, // SPEAKER_VOLUME = 512
    60, 0x30, // PIN_JACK_TX = PIN_D1
    100, 0x2, // PIN_A0 = PA02
    101, 0x5, // PIN_A1 = PA05
    102, 0x28, // PIN_A2 = PB08
    103, 0x29, // PIN_A3 = PB09
    104, 0x4, // PIN_A4 = PA04
    105, 0x6, // PIN_A5 = PA06
    150, 0x31, // PIN_D0 = PB17
    151, 0x30, // PIN_D1 = PB16
    154, 0xe, // PIN_D4 = PA14
    155, 0x10, // PIN_D5 = PA16
    156, 0x12, // PIN_D6 = PA18
    159, 0x13, // PIN_D9 = PA19
    160, 0x14, // PIN_D10 = PA20
    161, 0x15, // PIN_D11 = PA21
    162, 0x16, // PIN_D12 = PA22
    163, 0x17, // PIN_D13 = PA23
    200, 0x1, // NUM_NEOPIXELS = 1
    204, 0x80000, // FLASH_BYTES = 0x80000
    205, 0x30000, // RAM_BYTES = 0x30000
    208, 0x239a0022, // BOOTLOADER_BOARD_ID = 0x239a0022
    209, 0x55114460, // UF2_FAMILY = ATSAMD51
    210, 0x20, // PINS_PORT_SIZE = PA_32
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* CF2 END */
};
#endif

#endif
