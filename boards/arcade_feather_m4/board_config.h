#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define VENDOR_NAME "Adafruit Industries"
#define PRODUCT_NAME "Arcade 51 Feather"
#define VOLUME_LABEL "ARCADE"
#define INDEX_URL "http://adafru.it/"
#define BOARD_ID "SAMD51J19A-Arcade51"

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

#define PIN_A0 PIN_PA02
#define PIN_A1 PIN_PA05
#define PIN_A2 PIN_PB08
#define PIN_A3 PIN_PB09
#define PIN_A4 PIN_PA04
#define PIN_A5 PIN_PA06

#define PIN_TX PIN_PB16
#define PIN_RX PIN_PB17

#define PIN_D0 PIN_PB17 //RX
#define PIN_D1 PIN_PB16 //TX
#define PIN_D4 PIN_PA14
#define PIN_D5 PIN_PA16
#define PIN_D6 PIN_PA18
#define PIN_D9 PIN_PA19
#define PIN_D10 PIN_PA20
#define PIN_D11 PIN_PA21
#define PIN_D12 PIN_PA22
#define PIN_D13 PIN_PA23

#define PIN_LED PIN_PA23
#define PIN_SCK PIN_PA17
#define PIN_MISO PIN_PB22
#define PIN_MOSI PIN_PB23
#define PIN_SDA PIN_PA12
#define PIN_SCL PIN_PA13


#if 0
// game controller make
#define BUTTONS                                                                                    \
    CFG(PIN_BTN_LEFT, PIN_SDA)                                                                     \
    CFG(PIN_BTN_UP, PIN_D6)                                                                        \
    CFG(PIN_BTN_RIGHT, PIN_SCL)                                                                    \
    CFG(PIN_BTN_DOWN, PIN_D9)                                                                      \
    CFG(PIN_BTN_A, PIN_D10)                                                                        \
    CFG(PIN_BTN_B, PIN_D11)                                                                        \
    CFG(PIN_BTN_MENU, PIN_D12)
#else
// breadboarded Itsy
#define BUTTONS                                                                                    \
    CFG(PIN_BTN_LEFT, PIN_D11)                                                                     \
    CFG(PIN_BTN_UP, PIN_D10)                                                                       \
    CFG(PIN_BTN_RIGHT, PIN_D9)                                                                     \
    CFG(PIN_BTN_DOWN, PIN_D12)                                                                     \
    CFG(PIN_BTN_A, PIN_SCL)                                                                        \
    CFG(PIN_BTN_B, PIN_D6)                                                                         \
    CFG(PIN_BTN_MENU, PIN_SDA)
#endif

#define CONFIG_DATA                                                                                \
    BUTTONS                                                                                        \
    CFG(PIN_DISPLAY_CS, PIN_A2)                                                                    \
    CFG(PIN_DISPLAY_SCK, PIN_SCK)                                                                  \
    CFG(PIN_DISPLAY_MOSI, PIN_MOSI)                                                                \
    CFG(PIN_DISPLAY_DC, PIN_A3)                                                                    \
    CFG(PIN_DISPLAY_RST, PIN_A4)                                                                   \
    CFG(PIN_DISPLAY_MISO, PIN_MISO)                                                                \
    CFG(PIN_DISPLAY_BL, PIN_A5)                                                                    \
    CFG(PIN_SPEAKER_AMP, PIN_D4)                                                                   \
    CFG(SPEAKER_VOLUME, 512)                                                                       \
    CFG(DISPLAY_CFG0, 0x00000090)                                                                  \
    CFG(DISPLAY_CFG1, 0x000e14ff)                                                                  \
    CFG(DISPLAY_CFG2, 24)                                                                          \
    CFG(DISPLAY_WIDTH, 160)                                                                        \
    CFG(DISPLAY_HEIGHT, 128)

#endif
