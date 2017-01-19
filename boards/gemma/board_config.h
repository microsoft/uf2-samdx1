#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define __SAMD21E18A__ 1
#define CRYSTALLESS    1

#define VENDOR_NAME "Adafruit Industries"
#define PRODUCT_NAME "Gemma M0"
#define VOLUME_LABEL "GEMMABOOT"
#define INDEX_URL "https://adafru.it/gemmam0"
#define BOARD_ID "SAMD21E18A-Gemma-v0"

#define USB_VID 0x239A
#define USB_PID 0x001A

#define LED_PIN PIN_PA23
//#define LED_TX_PIN PIN_PA27
//#define LED_RX_PIN PIN_PB03

#define BOARD_RGBLED_CLOCK_PIN            PIN_PA05
#define BOARD_RGBLED_DATA_PIN             PIN_PA04

#endif
