#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define CRYSTALLESS    1

#define VENDOR_NAME "Robotics Masters"
#define PRODUCT_NAME "Robo HAT MM1"
#define VOLUME_LABEL "ROBOBOOT"
#define INDEX_URL "https://roboticsmasters.co"
#define BOARD_ID "SAMD51G19A-robohat-v7"

#define USB_VID 0x1209
#define USB_PID 0x4D44

#define LED_PIN PIN_PA21

#define BOOT_USART_MODULE                 SERCOM0
#define BOOT_USART_MASK                   APBAMASK
#define BOOT_USART_BUS_CLOCK_INDEX        MCLK_APBAMASK_SERCOM0
#define BOOT_USART_PAD_SETTINGS           UART_RX_PAD1_TX_PAD0
#define BOOT_USART_PAD3                   PINMUX_UNUSED
#define BOOT_USART_PAD2                   PINMUX_UNUSED
#define BOOT_USART_PAD1                   PINMUX_PA05D_SERCOM0_PAD1
#define BOOT_USART_PAD0                   PINMUX_PA04D_SERCOM0_PAD0
#define BOOT_GCLK_ID_CORE                 SERCOM0_GCLK_ID_CORE
#define BOOT_GCLK_ID_SLOW                 SERCOM0_GCLK_ID_SLOW

#endif
