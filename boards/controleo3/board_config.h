#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define VENDOR_NAME  "Whizoo"
#define PRODUCT_NAME "Controleo3"
#define VOLUME_LABEL "CLEOBOOT"

#define BOARD_ID "SAMD21J18A-Controleo3"

//#define USB_VID 0x2341
//#define USB_PID 0x024D

// Cause bootloader to detect if running under a SW Debugger
// And pass that information to the App.
// This ALSO defines the code value passed to the application.
#define DETECT_DEBUGGER_M0 0xAFFEC7ED


// We do not have any status leds.
// We do have a Buzzer, so we can beep the buzzer instead of a LED. Better.
// A Buzzer needs to be on a TCC to use that hardware for buzzing purposes.
// You can have a Status LED or a BUZZER but not both.

#define BUZZER_PIN PIN_PA12
#define BUZZER_TCC 2

// No Led
//#define LED_PIN PIN_PA17

// No RGB Led either.
//#define LED_TX_PIN PIN_PA27
//#define LED_RX_PIN PIN_PB03

// This target has NO RESET BUTTON, so double tap reset can not work.
// However we can use the SDCard Card Detect.  
// If there is no SDCard inserted, hold in the Bootloader.

// The Pin that will tell us to stay in the bootloader or not.
#define HOLD_PIN PIN_PA02

// Optional, define if a Pull up or pulldown is needed.
#define HOLD_PIN_PULLUP
//#define HOLD_PIN_PULLDOWN

// What is the Hold state of the GPIO, 0 or 1.
#define HOLD_STATE 1


#endif
