#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define VENDOR_NAME "Adafruit Industries"
#define PRODUCT_NAME "PyBadge"
#define VOLUME_LABEL "PYBADGEBOOT"
#define INDEX_URL "http://adafru.it/4200"
#define BOARD_ID "SAMD51J19A-PyBadge-M4"

#define USB_VID 0x239A
#define USB_PID 0x0033

#define LED_PIN PIN_PA23

#define BOARD_NEOPIXEL_PIN PIN_PA15
#define BOARD_NEOPIXEL_COUNT 5

#define BOARD_SCREEN 1

#define BOOT_USART_MODULE SERCOM5
#define BOOT_USART_MASK APBDMASK
#define BOOT_USART_BUS_CLOCK_INDEX MCLK_APBDMASK_SERCOM5
#define BOOT_USART_PAD_SETTINGS UART_RX_PAD1_TX_PAD0
#define BOOT_USART_PAD3 PINMUX_UNUSED
#define BOOT_USART_PAD2 PINMUX_UNUSED
#define BOOT_USART_PAD1 PINMUX_PB17C_SERCOM5_PAD1
#define BOOT_USART_PAD0 PINMUX_PB16C_SERCOM5_PAD0
#define BOOT_GCLK_ID_CORE SERCOM5_GCLK_ID_CORE
#define BOOT_GCLK_ID_SLOW SERCOM5_GCLK_ID_SLOW


#define HAS_CONFIG_DATA 1

// This configuration data should be edited at https://microsoft.github.io/uf2/patcher/
// Just drop this file there.
// Alternatively, it can be also binary edited there after the bootloader is compiled.

#ifdef DEFINE_CONFIG_DATA
const uint32_t config_data[] = {
    /* CF2 START */
    513675505, 539130489, // magic
    62, 100,  // used entries, total entries
    1, 0x2e, // PIN_ACCELEROMETER_INT = PB14
    2, 0xd, // PIN_ACCELEROMETER_SCL = PIN_SCL (PA13)
    3, 0xc, // PIN_ACCELEROMETER_SDA = PIN_SDA (PA12)
    4, 0x3ee, // PIN_BTN_A = P_1006
    5, 0x3ef, // PIN_BTN_B = P_1007
    13, 0x17, // PIN_LED = PIN_D13 (PA23)
    14, 0x24, // PIN_LIGHT = PB04
    18, 0x36, // PIN_MISO = PB22
    19, 0x37, // PIN_MOSI = PB23
    20, 0xf, // PIN_NEOPIXEL = PA15
    21, 0x31, // PIN_RX = PB17
    23, 0x11, // PIN_SCK = PA17
    24, 0xd, // PIN_SCL = PA13
    25, 0xc, // PIN_SDA = PA12
    26, 0x1b, // PIN_SPEAKER_AMP = PA27
    28, 0x30, // PIN_TX = PB16
    32, 0x2d, // PIN_DISPLAY_SCK = PB13
    34, 0x2f, // PIN_DISPLAY_MOSI = PB15
    35, 0x27, // PIN_DISPLAY_CS = PB07
    36, 0x25, // PIN_DISPLAY_DC = PB05
    37, 0xa0, // DISPLAY_WIDTH = 160
    38, 0x80, // DISPLAY_HEIGHT = 128
    39, 0x80, // DISPLAY_CFG0 = 0x80
    40, 0x12c2d, // DISPLAY_CFG1 = 0x12c2d
    41, 0x18, // DISPLAY_CFG2 = 0x18
    43, 0x0, // PIN_DISPLAY_RST = PA00
    44, 0x1, // PIN_DISPLAY_BL = PA01
    47, 0x3e8, // PIN_BTN_LEFT = P_1000
    48, 0x3eb, // PIN_BTN_RIGHT = P_1003
    49, 0x3e9, // PIN_BTN_UP = P_1001
    50, 0x3ea, // PIN_BTN_DOWN = P_1002
    51, 0x3ec, // PIN_BTN_MENU = P_1004
    59, 0x40, // SPEAKER_VOLUME = 64
    60, 0x17, // PIN_JACK_TX = PIN_D13
    65, 0x2, // PIN_JACK_SND = PIN_A0
    69, 0x3ed, // PIN_BTN_SOFT_RESET = P_1005
    70, 0x30, // ACCELEROMETER_TYPE = 48
    71, 0x20, // PIN_BTNMX_LATCH = PB00
    72, 0x3f, // PIN_BTNMX_CLOCK = PB31
    73, 0x3e, // PIN_BTNMX_DATA = PB30
    100, 0x2, // PIN_A0 = PA02
    101, 0x5, // PIN_A1 = PA05
    102, 0x28, // PIN_A2 = PB08
    103, 0x29, // PIN_A3 = PB09
    104, 0x4, // PIN_A4 = PA04
    105, 0x6, // PIN_A5 = PA06
    152, 0x23, // PIN_D2 = PB03
    153, 0x22, // PIN_D3 = PB02
    154, 0xe, // PIN_D4 = PA14
    155, 0x10, // PIN_D5 = PA16
    156, 0x12, // PIN_D6 = PA18
    159, 0x13, // PIN_D9 = PA19
    160, 0x14, // PIN_D10 = PA20
    161, 0x15, // PIN_D11 = PA21
    162, 0x16, // PIN_D12 = PA22
    163, 0x17, // PIN_D13 = PA23
    200, 0x5, // NUM_NEOPIXELS = 5
    204, 0x80000, // FLASH_BYTES = 0x80000
    205, 0x30000, // RAM_BYTES = 0x30000
    208, 0x239a0033, // BOOTLOADER_BOARD_ID = 0x239a0033
    209, 0x55114460, // UF2_FAMILY = ATSAMD51
    210, 0x20, // PINS_PORT_SIZE = PA_32
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* CF2 END */
};
#endif

#endif
