#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define VENDOR_NAME "Microchip"
#define PRODUCT_NAME "SAME54 Xplained"
#define VOLUME_LABEL "E54XBOOT"
#define INDEX_URL "https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro"
#define BOARD_ID "SAME54P20A-Xplained-v0"

#define USB_VID 0x239A
#define USB_PID 0x00B5

#define LED_PIN PIN_PC18

#define BOARD_NEOPIXEL_PIN PIN_PC24
#define BOARD_NEOPIXEL_COUNT 0

#define BOOT_USART_MODULE                 SERCOM0
#define BOOT_USART_MASK                   APBAMASK
#define BOOT_USART_BUS_CLOCK_INDEX        MCLK_APBAMASK_SERCOM0
#define BOOT_USART_PAD_SETTINGS           UART_RX_PAD1_TX_PAD0
#define BOOT_USART_PAD3                   PINMUX_UNUSED
#define BOOT_USART_PAD2                   PINMUX_UNUSED
#define BOOT_USART_PAD1                   PINMUX_PB25C_SERCOM0_PAD1
#define BOOT_USART_PAD0                   PINMUX_PB24C_SERCOM0_PAD0
#define BOOT_GCLK_ID_CORE                 SERCOM0_GCLK_ID_CORE
#define BOOT_GCLK_ID_SLOW                 SERCOM0_GCLK_ID_SLOW

#endif
