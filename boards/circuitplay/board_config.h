#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define __SAMD21G18A__ 1
#define CRYSTALLESS    1

#define VENDOR_NAME "Adafruit Industries"
#define PRODUCT_NAME "Circuit Playground Express"
#define VOLUME_LABEL "CPLAYBOOT"
#define INDEX_URL "https://adafru.it/cplayxpress"

#define BOARD_ID "SAMD21G18A-CPlay-v0"

#define USB_VID 0x239A
#define USB_PID 0x0019

#define LED_PIN PIN_PA17
//#define LED_TX_PIN PIN_PA27
//#define LED_RX_PIN PIN_PB03

#define BOARD_RGBLED_NEOPIX_PORT           (1) // PB
#define BOARD_RGBLED_NEOPIX_PIN            (23) // 23
#define NUM_NEOPIXELS                      1
#endif
