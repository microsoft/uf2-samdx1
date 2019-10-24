#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define VENDOR_NAME "Adafruit Industries"
#define PRODUCT_NAME "Itsy Arcade D51"
#define VOLUME_LABEL "ARCADE-D5"
#define INDEX_URL "https://arcade.makecode.com/"
#define BOARD_ID "SAMD51G19A-Itsy-Arcade-D51"

#define USB_VID 0x239A
#define USB_PID 0x002B

#define LED_PIN PIN_PA22

#define BOARD_RGBLED_CLOCK_PIN PIN_PB02
#define BOARD_RGBLED_DATA_PIN PIN_PB03

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
#define BOARD_SCREEN 1

// This configuration data should be edited at https://microsoft.github.io/uf2/patcher/
// Just drop this file there.
// Alternatively, it can be also binary edited there after the bootloader is compiled.

#ifdef DEFINE_CONFIG_DATA
const uint32_t config_data[] = {
    /* CF2 START */
    513675505, 539130489, // magic
    60, 100,  // used entries, total entries
    4, 0xd, // PIN_BTN_A = PIN_SCL
    5, 0x12, // PIN_BTN_B = PIN_D7
    7, 0x22, // PIN_DOTSTAR_CLOCK = PB02
    8, 0x23, // PIN_DOTSTAR_DATA = PB03
    9, 0x2b, // PIN_FLASH_CS = PB11
    10, 0x9, // PIN_FLASH_MISO = PA09
    11, 0x8, // PIN_FLASH_MOSI = PA08
    12, 0x2a, // PIN_FLASH_SCK = PB10
    13, 0x16, // PIN_LED = PIN_D13
    18, 0x37, // PIN_MISO = PB23
    19, 0x0, // PIN_MOSI = PA00
    21, 0x10, // PIN_RX = PA16
    23, 0x1, // PIN_SCK = PA01
    24, 0xd, // PIN_SCL = PA13
    25, 0xc, // PIN_SDA = PA12
    26, 0x7, // PIN_SPEAKER_AMP = PIN_D2
    28, 0x11, // PIN_TX = PA17
    32, 0x1, // PIN_DISPLAY_SCK = PIN_SCK
    33, 0x37, // PIN_DISPLAY_MISO = PIN_MISO
    34, 0x0, // PIN_DISPLAY_MOSI = PIN_MOSI
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
    50, 0x17, // PIN_BTN_DOWN = PIN_D12
    51, 0xc, // PIN_BTN_MENU = PIN_SDA
    59, 0x200, // SPEAKER_VOLUME = 512
    60, 0x11, // PIN_JACK_TX = PIN_D1
    100, 0x2, // PIN_A0 = PA02
    101, 0x5, // PIN_A1 = PA05
    102, 0x28, // PIN_A2 = PB08
    103, 0x29, // PIN_A3 = PB09
    104, 0x4, // PIN_A4 = PA04
    105, 0x6, // PIN_A5 = PA06
    150, 0x10, // PIN_D0 = PA16
    151, 0x11, // PIN_D1 = PA17
    152, 0x7, // PIN_D2 = PA07
    153, 0x36, // PIN_D3 = PB22
    154, 0xe, // PIN_D4 = PA14
    155, 0xf, // PIN_D5 = PA15
    157, 0x12, // PIN_D7 = PA18
    159, 0x13, // PIN_D9 = PA19
    160, 0x14, // PIN_D10 = PA20
    161, 0x15, // PIN_D11 = PA21
    162, 0x17, // PIN_D12 = PA23
    163, 0x16, // PIN_D13 = PA22
    201, 0x1, // NUM_DOTSTARS = 1
    204, 0x80000, // FLASH_BYTES = 0x80000
    205, 0x30000, // RAM_BYTES = 0x30000
    208, 0x239a002b, // BOOTLOADER_BOARD_ID = 0x239a002b
    209, 0x55114460, // UF2_FAMILY = ATSAMD51
    210, 0x20, // PINS_PORT_SIZE = PA_32
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* CF2 END */
};
#endif

#endif
