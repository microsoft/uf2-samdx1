#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define CRYSTALLESS  1

#define VENDOR_NAME  "Capable Robot Components"
#define PRODUCT_NAME "Programmable USB Hub"
#define VOLUME_LABEL "USBHUBBOOT"
#define INDEX_URL    "http://capablerobot.com"
#define BOARD_ID     "SAMD51G19A-USBHub-v0"

// VID & PID obtained from Microchip
#define USB_VID 0x04D8
#define USB_PID 0xEDB3

#define LED_PIN PIN_PA19

#define BOOT_USART_MODULE                 SERCOM1
#define BOOT_USART_MASK                   APBAMASK
#define BOOT_USART_BUS_CLOCK_INDEX        MCLK_APBAMASK_SERCOM1
#define BOOT_USART_PAD_SETTINGS           UART_RX_PAD1_TX_PAD0
#define BOOT_USART_PAD3                   PINMUX_UNUSED
#define BOOT_USART_PAD2                   PINMUX_UNUSED
#define BOOT_USART_PAD1                   PINMUX_PA17C_SERCOM1_PAD1
#define BOOT_USART_PAD0                   PINMUX_PA16C_SERCOM1_PAD0
#define BOOT_GCLK_ID_CORE                 SERCOM3_GCLK_ID_CORE
#define BOOT_GCLK_ID_SLOW                 SERCOM3_GCLK_ID_SLOW

#endif
