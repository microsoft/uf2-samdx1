#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define VENDOR_NAME "Adafruit Industries"
#define PRODUCT_NAME "Metro M4 Rev B"
#define VOLUME_LABEL "METROM4BOOT"
#define INDEX_URL "http://adafru.it/3505"
#define BOARD_ID "SAMD51J19A-Metro-v0"

#define USB_VID 0x239A
#define USB_PID 0x0021

#define LED_PIN PIN_PA21
#define LED_TX_PIN PIN_PA27
#define LED_RX_PIN PIN_PB06

#define BOARD_NEOPIXEL_PIN PIN_PB17
#define BOARD_NEOPIXEL_COUNT 1

#define BOOT_USART_MODULE                 SERCOM0
#define BOOT_USART_MASK                   APBAMASK
#define BOOT_USART_BUS_CLOCK_INDEX        MCLK_APBAMASK_SERCOM0
#define BOOT_USART_PAD_SETTINGS           UART_RX_PAD3_TX_PAD2
#define BOOT_USART_PAD3                   PINMUX_UNUSED
#define BOOT_USART_PAD2                   PINMUX_UNUSED
#define BOOT_USART_PAD1                   PINMUX_PA10C_SERCOM0_PAD2
#define BOOT_USART_PAD0                   PINMUX_PA11C_SERCOM0_PAD3
#define BOOT_GCLK_ID_CORE                 SERCOM0_GCLK_ID_CORE
#define BOOT_GCLK_ID_SLOW                 SERCOM0_GCLK_ID_SLOW

#endif
