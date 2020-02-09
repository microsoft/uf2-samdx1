#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define VENDOR_NAME "maholli"
#define PRODUCT_NAME "PyCubedv04"
#define VOLUME_LABEL "PYCUBEDBOOT"
#define INDEX_URL "http://pycubed.org"
#define BOARD_ID "PYCUBED"

#define USB_VID 0x04D8
#define USB_PID 0xEC44 // PID sublicensed from Microchip

#define LED_PIN PIN_PA16 // not actually used, but build fails without it?
#define BOARD_NEOPIXEL_PIN PIN_PA21
#define BOARD_NEOPIXEL_COUNT 1

#define BOOT_USART_MODULE                 SERCOM0
#define BOOT_USART_MASK                   APBAMASK
#define BOOT_USART_BUS_CLOCK_INDEX        MCLK_APBAMASK_SERCOM0
#define BOOT_USART_PAD_SETTINGS           UART_RX_PAD3_TX_PAD0
#define BOOT_USART_PAD3                   PINMUX_PA07D_SERCOM0_PAD3
#define BOOT_USART_PAD2                   PINMUX_UNUSED
#define BOOT_USART_PAD1                   PINMUX_UNUSED
#define BOOT_USART_PAD0                   PINMUX_PA04D_SERCOM0_PAD0
#define BOOT_GCLK_ID_CORE                 SERCOM0_GCLK_ID_CORE
#define BOOT_GCLK_ID_SLOW                 SERCOM0_GCLK_ID_SLOW

#endif
