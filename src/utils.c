#include "uf2.h"

static uint32_t timerLow;
uint32_t timerHigh, resetHorizon;

void delay(uint32_t ms) {
    // SAMD21 starts up at 1mhz by default.
    #ifdef SAMD21
    ms <<= 8;
    #endif
    // SAMD51 starts up at 48mhz by default.
    #ifdef SAMD51
    ms <<= 12;
    #endif
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
    RGBLED_set_color(COLOR_LEAVE);
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
