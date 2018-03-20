#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define __SAMD21G18A__ 1

#define VENDOR_NAME "eduSense"
#define PRODUCT_NAME "senseBox MCU"
#define VOLUME_LABEL "SENSEBOX"
#define INDEX_URL "http://www.sensebox.de"
#define BOARD_ID "SAMD21G18A-senseBox-MCU-v0"

#define USB_VID 0x04D8
#define USB_PID 0xEF66 // PID sublicensed from Microchip
#define USB_POWER_MA 20

#define LED_PIN PIN_PA27 // red LED

#endif
