#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define CRYSTALLESS    1

#define VENDOR_NAME "Adafruit Industries"
#define PRODUCT_NAME "Hallowing Mask M4"
#define VOLUME_LABEL "MASKM4BOOT"
#define INDEX_URL "http://adafru.it/"
#define BOARD_ID "SAMD51G19A-Mask-v0"

#define USB_VID 0x239A
#define USB_PID 0x0047

#define LED_PIN PIN_PA27

// There isnt a clear UART but we can make one on the D2/D3 ports
#define BOOT_USART_MODULE                 SERCOM4
#define BOOT_USART_MASK                   APBDMASK
#define BOOT_USART_BUS_CLOCK_INDEX        MCLK_APBDMASK_SERCOM4
#define BOOT_USART_PAD_SETTINGS           UART_RX_PAD1_TX_PAD0
#define BOOT_USART_PAD3                   PINMUX_UNUSED
#define BOOT_USART_PAD2                   PINMUX_UNUSED
#define BOOT_USART_PAD1                   PINMUX_PB09D_SERCOM4_PAD1
#define BOOT_USART_PAD0                   PINMUX_PB08D_SERCOM4_PAD0
#define BOOT_GCLK_ID_CORE                 SERCOM4_GCLK_ID_CORE
#define BOOT_GCLK_ID_SLOW                 SERCOM4_GCLK_ID_SLOW

#endif
