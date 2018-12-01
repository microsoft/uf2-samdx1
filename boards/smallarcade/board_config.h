#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define VENDOR_NAME "Adafruit Industries"
#define PRODUCT_NAME "ItsyBitsy-M4"
#define VOLUME_LABEL "ARCADE"
#define INDEX_URL "http://adafru.it/"
#define BOARD_ID "SAMD51J19A-Arcade-Small-v0"

#define USB_VID 0x239A
#define USB_PID 0x002B

#define LED_PIN PIN_PA23

#define BOARD_NEOPIXEL_PIN PIN_PA15
#define BOARD_NEOPIXEL_COUNT 5

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

#define PIN_A0 PIN_PA02
#define PIN_A1 PIN_PA05
#define PIN_A2 PIN_PB08
#define PIN_A3 PIN_PB09
#define PIN_A4 PIN_PA04
#define PIN_A5 PIN_PA06

#define PIN_D2 PIN_PB03
#define PIN_D3 PIN_PB02
#define PIN_D4 PIN_PA14
#define PIN_D5 PIN_PA16
#define PIN_D6 PIN_PA18
#define PIN_D9 PIN_PA19
#define PIN_D10 PIN_PA20
#define PIN_D11 PIN_PA21
#define PIN_D12 PIN_PA22
#define PIN_D13 PIN_PA23
#define PIN_LED PIN_D13

#define PIN_RX PIN_PA17
#define PIN_TX PIN_PA16

#define PIN_SCK PIN_PA17
#define PIN_MISO PIN_PB22
#define PIN_MOSI PIN_PB23
#define PIN_SDA PIN_PA12
#define PIN_SCL PIN_PA13

#define PIN_DISPLAY_BL PIN_PA00
#define PIN_DISPLAY_CS PIN_PB07
#define PIN_DISPLAY_SCK PIN_PB13
#define PIN_DISPLAY_MOSI PIN_PB12
#define PIN_DISPLAY_DC PIN_PB05
#define PIN_DISPLAY_RST PIN_PA01

#define CONFIG_DATA                                                                                \
    CFG(PIN_BTN_LEFT, 100)                                                                         \
    CFG(PIN_BTN_UP, 101)                                                                           \
    CFG(PIN_BTN_RIGHT, 103)                                                                        \
    CFG(PIN_BTN_DOWN, 102)                                                                         \
    CFG(PIN_BTN_A, 107)                                                                            \
    CFG(PIN_BTN_B, 106)                                                                            \
    CFG(PIN_BTN_MENU, 104)                                                                         \
    CFG(PIN_BTN_SOFT_RESET, 105)                                                                   \
    CFG(PIN_DISPLAY_CS, PIN_DISPLAY_CS)                                                            \
    CFG(PIN_DISPLAY_SCK, PIN_DISPLAY_SCK)                                                          \
    CFG(PIN_DISPLAY_MOSI, PIN_DISPLAY_MOSI)                                                        \
    CFG(PIN_DISPLAY_DC, PIN_DISPLAY_DC)                                                            \
    CFG(PIN_DISPLAY_RST, PIN_DISPLAY_RST)                                                          \
    CFG(PIN_DISPLAY_MISO, -1)                                                                      \
    CFG(PIN_DISPLAY_BL, PIN_DISPLAY_BL)                                                            \
    CFG(PIN_SPEAKER_AMP, -1)                                                                       \
    CFG(PIN_JACK_SND, PIN_A0)                                                                      \
    CFG(PIN_JACK_TX, PIN_TX)                                                                       \
    CFG(PIN_BTNMX_LATCH, PIN_PB00)                                                                 \
    CFG(PIN_BTNMX_CLOCK, PIN_PB31)                                                                 \
    CFG(PIN_BTNMX_DATA, PIN_PB30)                                                                  \
    CFG(SPEAKER_VOLUME, 512)                                                                       \
    CFG(DISPLAY_CFG0, 0x00000080)                                                                  \
    CFG(DISPLAY_CFG1, 0x00012C2D)                                                                  \
    CFG(DISPLAY_CFG2, 24)                                                                          \
    CFG(DISPLAY_WIDTH, 160)                                                                        \
    CFG(DISPLAY_HEIGHT, 128)                                                                       \
    CFG(PIN_A0, PIN_A0)                                                                            \
    CFG(PIN_A1, PIN_A1)                                                                            \
    CFG(PIN_A2, PIN_A2)                                                                            \
    CFG(PIN_A3, PIN_A3)                                                                            \
    CFG(PIN_A4, PIN_A4)                                                                            \
    CFG(PIN_A5, PIN_A5)                                                                            \
    CFG(PIN_D2, PIN_D2)                                                                            \
    CFG(PIN_D3, PIN_D3)                                                                            \
    CFG(PIN_D4, PIN_D4)                                                                            \
    CFG(PIN_D5, PIN_D5)                                                                            \
    CFG(PIN_D6, PIN_D6)                                                                            \
    CFG(PIN_D9, PIN_D9)                                                                            \
    CFG(PIN_D10, PIN_D10)                                                                          \
    CFG(PIN_D11, PIN_D11)                                                                          \
    CFG(PIN_D12, PIN_D12)                                                                          \
    CFG(PIN_D13, PIN_D13)                                                                          \
    CFG(PIN_LED, PIN_LED)                                                                          \
    CFG(PIN_SCK, PIN_SCK)                                                                          \
    CFG(PIN_MISO, PIN_MISO)                                                                        \
    CFG(PIN_MOSI, PIN_MOSI)                                                                        \
    CFG(PIN_SDA, PIN_SDA)                                                                          \
    CFG(PIN_SCL, PIN_SCL)                                                                          \
    CFG(PIN_NEOPIXEL, BOARD_NEOPIXEL_PIN)                                                          \
    CFG(NUM_NEOPIXELS, BOARD_NEOPIXEL_COUNT)                                                       \
    CFG(PIN_ACCELEROMETER_INT, PIN_PB14)                                                           \
    CFG(PIN_ACCELEROMETER_SCL, PIN_SCL)                                                            \
    CFG(PIN_ACCELEROMETER_SDA, PIN_SDA)                                                            \
    CFG(ACCELEROMETER_TYPE, ACCELEROMETER_TYPE_LIS3DH)

#endif
