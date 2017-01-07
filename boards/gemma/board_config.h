#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define __SAMD21E18A__ 1
#define CRYSTALLESS    1

#define VENDOR_NAME "Adafruit Industries"
#define PRODUCT_NAME "Gemma M0"
#define VOLUME_LABEL "GEMMABOOT"

#define BOARD_ID "SAMD21E18A-Trinket-v0"

#define USB_VID 0x239A
#define USB_PID 0x801A

#define LED_PIN PIN_PA23
//#define LED_TX_PIN PIN_PA27
//#define LED_RX_PIN PIN_PB03

#define BOARD_RGBLED_CLOCK_PORT           (0) // PA
#define BOARD_RGBLED_CLOCK_PIN            (5) // 05
#define BOARD_RGBLED_DATA_PORT            (0) // PA
#define BOARD_RGBLED_DATA_PIN             (4) // 04

#endif
