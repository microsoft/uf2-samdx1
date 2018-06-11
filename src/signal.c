#include "uf2.h"
#include "neopixel.h"

// Status Signalling
#ifndef ON_TIME 

static uint32_t now;
static uint32_t signal_end;
int8_t led_tick_step = 1;
static uint8_t limit = 200;

// led_tick is called by the systick IRQ.  It SHOULD be called
// 1000 times a second.

void led_tick() {
    now++;
    if (signal_end) {
        if (now == signal_end - 1000) {
            LED_MSC_ON();
        }
        if (now == signal_end) {
            signal_end = 0;
        }
    } else {
        uint8_t curr = now & 0xff;
        if (curr == 0) {
            LED_MSC_ON();
            if (limit < 10 || limit > 250) {
                led_tick_step = -led_tick_step;
            }
            limit += led_tick_step;
        } else if (curr == limit) {
            LED_MSC_OFF();
        }
    }
}

void led_signal() {
    if (signal_end < now) {
        signal_end = now + 2000;
        LED_MSC_OFF();
    }
}

void led_init() {
#if defined(LED_PIN)    
    PINOP(LED_PIN, DIRSET);
#endif    

#if (defined(BUZZER_PIN) && defined(BUZZER_TCC))
    init_tcc_buzzer(); // TCC Buzzer acts a substitute for a led.
#endif    

    LED_MSC_ON();

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
