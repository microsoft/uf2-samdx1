#include "uf2.h"
#include "neopixel.h"
#include "tcc_buzzer.h"

void RGBLED_set_color(uint32_t color);

#ifdef BOARD_NEOPIXEL_PIN
#define COLOR_START 0x040000
#define COLOR_USB 0x000400
#define COLOR_UART 0x040400
#define COLOR_LEAVE 0x000000
#else
#define COLOR_START 0x000040
#define COLOR_USB 0x004000
#define COLOR_UART 0x404000
#define COLOR_LEAVE 0x400040
#endif

// Status Signalling
#ifndef HB_ON_TIME 
// Default to Heartbeat ON of 500ms
#define HB_ON_TIME 500
#endif

#ifndef HB_OFF_TIME 
// Default to Heartbeat OFF of 500ms
#define HB_OFF_TIME 500
#endif

#ifdef LED_PIN
#define SIGNAL_OFF() PINOP(LED_PIN, OUTCLR)
#define SIGNAL_ON() PINOP(LED_PIN, OUTSET)
#define SIGNAL_TGL() PINOP(LED_PIN, OUTTGL)
#elif (defined(BUZZER_PIN) && defined(BUZZER_TCC))
// Enable/Disable buzzer by toggling the pin mux.
#define SIGNAL_OFF() PINCFG(BUZZER_PIN) = 0x60;
#define SIGNAL_ON()  PINCFG(BUZZER_PIN) = 0x61;
#define SIGNAL_TGL() PINCFG(BUZZER_PIN) = PINCFG(BUZZER_PIN) ^ 0x01;
#else
#define SIGNAL_OFF()
#define SIGNAL_ON()
#define SIGNAL_TGL()
#endif

static uint32_t now = 0;
static uint8_t  rpt = 0;
static uint32_t hb_ontime = HB_ON_TIME;
static uint32_t hb_offtime = HB_ON_TIME + HB_ON_TIME;

// signal_tick is called by the systick IRQ.  It SHOULD be called
// SYSTICK_FREQ times a second.

void signal_tick() {
    // Flash the Heartbeat
    now++;
    if (now < hb_ontime) {
        SIGNAL_ON();
    } else if (now < (hb_offtime)) {
        SIGNAL_OFF();
    } else {
        // This allows us short bursts of flashing at different
        // duty cycles.  Just set hb_ontime and hb_offtime
        // to the required values.  Then set rpt to the number of
        // flash cycles you want.
        // After it completes, it will go back to the normal heartbeat.
        now = 0;
        if (rpt > 0) {
            rpt--;
            if (rpt == 0) {
                hb_ontime = HB_ON_TIME;
                hb_offtime = HB_ON_TIME + HB_OFF_TIME;
            }
        }
    }

}

void signal_state(uint8_t state) {
    switch(state) {
        case SIGNAL_START:
            RGBLED_set_color(COLOR_START);
            hb_ontime = 100;
            hb_offtime = 200;
            rpt = 10;
        break;
        case SIGNAL_USB:
            RGBLED_set_color(COLOR_USB);
            hb_ontime =  250;
            hb_offtime = 500;
            rpt = 4;
        break;
        case SIGNAL_UART:
            RGBLED_set_color(COLOR_UART);
            hb_ontime =  750;
            hb_offtime = 1000;
            rpt = 1;
        break;
        case SIGNAL_LEAVE:
            RGBLED_set_color(COLOR_LEAVE);
            hb_ontime =  0;
            hb_offtime = 255;
            rpt = 255;
            SIGNAL_OFF();
        break;
        case SIGNAL_FLASHWR:
            hb_ontime  = 1000;
            hb_offtime = 2000;
            rpt        = 10;
        case SIGNAL_HB:
        default:
            // Force immediate return to normal heartbeat timing.
            rpt = 1;
        break;
    }
}

void signal_init() {
#if defined(LED_PIN)    
    PINOP(LED_PIN, DIRSET);
    LED_MSC_ON();
#elif (defined(BUZZER_PIN) && defined(BUZZER_TCC))
    init_tcc_buzzer(); // TCC Buzzer acts a substitute for a led.
#endif    

#if defined(BOARD_RGBLED_CLOCK_PIN)
    // using APA102, set pins to outputs
    PINOP(BOARD_RGBLED_CLOCK_PIN, DIRSET);
    PINOP(BOARD_RGBLED_DATA_PIN, DIRSET);

    // This won't work for neopixel, because we're running at 1MHz or thereabouts...
    RGBLED_set_color(COLOR_LEAVE);
#endif
}

#if defined(BOARD_RGBLED_CLOCK_PIN)
void write_apa_byte(uint8_t x) {
    for (uint8_t i = 0x80; i != 0; i >>= 1) {
        if (x & i)
            PINOP(BOARD_RGBLED_DATA_PIN, OUTSET);
        else
            PINOP(BOARD_RGBLED_DATA_PIN, OUTCLR);

        PINOP(BOARD_RGBLED_CLOCK_PIN, OUTSET);
        // for (uint8_t j=0; j<25; j++) /* 0.1ms */
        //  __asm__ __volatile__("");

        PINOP(BOARD_RGBLED_CLOCK_PIN, OUTCLR);
        // for (uint8_t j=0; j<25; j++) /* 0.1ms */
        //  __asm__ __volatile__("");
    }
}
#endif

void RGBLED_set_color(uint32_t color) {
#if defined(BOARD_RGBLED_CLOCK_PIN)
    write_apa_byte(0x0);
    write_apa_byte(0x0);
    write_apa_byte(0x0);
    write_apa_byte(0x0);

    write_apa_byte(0xFF);
    write_apa_byte(color >> 16);
    write_apa_byte(color >> 8);
    write_apa_byte(color);

    write_apa_byte(0xFF);
    write_apa_byte(0xFF);
    write_apa_byte(0xFF);
    write_apa_byte(0xFF);

    // set clock port low for ~10ms
    delay(50);
#elif defined(BOARD_NEOPIXEL_PIN)
    uint8_t buf[BOARD_NEOPIXEL_COUNT * 3];
#if 0
    memset(buf, 0, sizeof(buf));
    buf[0] = color >> 8;
    buf[1] = color >> 16;
    buf[2] = color;
#else
    for (int i = 0; i < BOARD_NEOPIXEL_COUNT * 3; i += 3) {
        buf[i + 0] = color >> 8;
        buf[i + 1] = color >> 16;
        buf[i + 2] = color;
    }
#endif
    neopixel_send_buffer(buf, BOARD_NEOPIXEL_COUNT * 3);
#endif
}
