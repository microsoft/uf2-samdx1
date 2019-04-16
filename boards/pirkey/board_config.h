#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define CRYSTALLESS    1

#define VENDOR_NAME "Adafruit Industries"
#define PRODUCT_NAME "pIRKey M0"
#define VOLUME_LABEL "PIRKEYBOOT"
#define INDEX_URL "http://adafru.it/3364"
#define BOARD_ID "SAMD21E18A-pIRKey-v0"

#define USB_VID 0x239A
#define USB_PID 0x0027

#define LED_PIN PIN_PA02  // not actually connected
//#define LED_TX_PIN PIN_PA27
//#define LED_RX_PIN PIN_PB03

#define BOARD_RGBLED_CLOCK_PIN            PIN_PA01
#define BOARD_RGBLED_DATA_PIN             PIN_PA00

#endif
