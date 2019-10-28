#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define CRYSTALLESS    1

#define VENDOR_NAME "Adafruit Industries"
#define PRODUCT_NAME "HalloWing M4"
#define VOLUME_LABEL "HALLOM4BOOT"
#define INDEX_URL "http://adafru.it/"
#define BOARD_ID "SAMD51J19A-HalloM4-v0"

#define USB_VID 0x239A
#define USB_PID 0x0049

#define LED_PIN PIN_PA23

#define BOARD_NEOPIXEL_PIN PIN_PB16
#define BOARD_NEOPIXEL_COUNT 4

#define BOOT_USART_MODULE                 SERCOM4
#define BOOT_USART_MASK                   APBDMASK
#define BOOT_USART_BUS_CLOCK_INDEX        MCLK_APBDMASK_SERCOM4
#define BOOT_USART_PAD_SETTINGS           UART_RX_PAD1_TX_PAD0
#define BOOT_USART_PAD3                   PINMUX_UNUSED
#define BOOT_USART_PAD2                   PINMUX_UNUSED
#define BOOT_USART_PAD1                   PINMUX_PB13C_SERCOM4_PAD1
#define BOOT_USART_PAD0                   PINMUX_PB12C_SERCOM4_PAD0
#define BOOT_GCLK_ID_CORE                 SERCOM4_GCLK_ID_CORE
#define BOOT_GCLK_ID_SLOW                 SERCOM4_GCLK_ID_SLOW

#endif
