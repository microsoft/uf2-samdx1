#include "uf2.h"

static uint32_t timerLow;
uint32_t timerHigh, resetHorizon;

void delay(uint32_t ms) {
    ms <<= 8;
    for (int i = 1; i < ms; ++i) {
        asm("nop");
    }
}

void timerTick(void) {
    if (timerLow-- == 0) {
        timerLow = TIMER_STEP;
        timerHigh++;
        if (resetHorizon && timerHigh >= resetHorizon) {
            resetHorizon = 0;
            resetIntoApp();
        }
    }
}

void panic(int code) {
    logval("PANIC", code);
    while (1) {
    }
}

int writeNum(char *buf, uint32_t n, bool full) {
    int i = 0;
    int sh = 28;
    while (sh >= 0) {
        int d = (n >> sh) & 0xf;
        if (full || d || sh == 0 || i) {
            buf[i++] = d > 9 ? 'A' + d - 10 : '0' + d;
        }
        sh -= 4;
    }
    return i;
}

void resetIntoApp() {
    // reset without waiting for double tap (only works for one reset)
    *DBL_TAP_PTR = DBL_TAP_MAGIC_QUICK_BOOT;
    NVIC_SystemReset();
}

void resetIntoBootloader() {
    // reset without waiting for double tap (only works for one reset)
    *DBL_TAP_PTR = DBL_TAP_MAGIC;
    NVIC_SystemReset();
}

#if USE_LOGS
struct LogStore logStoreUF2;

void logreset() {
    logStoreUF2.ptr = 0;
    logmsg("Reset logs.");
}

void logwritenum(uint32_t n) {
    char buff[9];
    buff[writeNum(buff, n, false)] = 0;
    logwrite("0x");
    logwrite(buff);
}

void logwrite(const char *msg) {
    const int jump = sizeof(logStoreUF2.buffer) / 4;
    if (logStoreUF2.ptr >= sizeof(logStoreUF2.buffer) - jump) {
        logStoreUF2.ptr -= jump;
        memmove(logStoreUF2.buffer, logStoreUF2.buffer + jump, logStoreUF2.ptr);
    }
    int l = strlen(msg);
    if (l + logStoreUF2.ptr >= sizeof(logStoreUF2.buffer)) {
        logwrite("TOO LONG!\n");
        return;
    }
    memcpy(logStoreUF2.buffer + logStoreUF2.ptr, msg, l);
    logStoreUF2.ptr += l;
    logStoreUF2.buffer[logStoreUF2.ptr] = 0;
}

void logmsg(const char *msg) {
    logwrite(msg);
    logwrite("\n");
}

void logval(const char *lbl, uint32_t v) {
    logwrite(lbl);
    logwrite(": ");
    logwritenum(v);
    logwrite("\n");
}
#endif

static uint32_t now;
static uint32_t signal_end;
int8_t led_tick_step = 1;
static uint8_t limit = 200;

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
    PINOP(LED_PIN, DIRSET);
    LED_MSC_ON();

#if defined(BOARD_RGBLED_CLOCK_PORT)
    // using APA102, set pins to outputs
    PORT->Group[BOARD_RGBLED_CLOCK_PORT].DIRSET.reg = (1 << BOARD_RGBLED_CLOCK_PIN);
    PORT->Group[BOARD_RGBLED_DATA_PORT].DIRSET.reg = (1 << BOARD_RGBLED_DATA_PIN);
#endif
    // and clock 0x00000 out!
    RGBLED_set_color(0, 0, 0);
}

void write_apa_byte(uint8_t x) {
#if defined(BOARD_RGBLED_CLOCK_PORT)
    for (uint8_t i = 0x80; i != 0; i >>= 1) {
        if (x & i)
            PORT->Group[BOARD_RGBLED_DATA_PORT].OUTSET.reg = (1 << BOARD_RGBLED_DATA_PIN);
        else
            PORT->Group[BOARD_RGBLED_DATA_PORT].OUTCLR.reg = (1 << BOARD_RGBLED_DATA_PIN);

        PORT->Group[BOARD_RGBLED_CLOCK_PORT].OUTSET.reg = (1 << BOARD_RGBLED_CLOCK_PIN);
        // for (uint8_t j=0; j<25; j++) /* 0.1ms */
        //  __asm__ __volatile__("");

        PORT->Group[BOARD_RGBLED_CLOCK_PORT].OUTCLR.reg = (1 << BOARD_RGBLED_CLOCK_PIN);
        // for (uint8_t j=0; j<25; j++) /* 0.1ms */
        //  __asm__ __volatile__("");
    }
#endif
}

void RGBLED_set_color(uint8_t red, uint8_t green, uint8_t blue) {
#if defined(BOARD_RGBLED_CLOCK_PORT)
    write_apa_byte(0x0);
    write_apa_byte(0x0);
    write_apa_byte(0x0);
    write_apa_byte(0x0);

    write_apa_byte(0xFF);
    write_apa_byte(blue);
    write_apa_byte(green);
    write_apa_byte(red);

    write_apa_byte(0xFF);
    write_apa_byte(0xFF);
    write_apa_byte(0xFF);
    write_apa_byte(0xFF);

    // set clock port low for ~10ms
    delay(10);
#endif
}
