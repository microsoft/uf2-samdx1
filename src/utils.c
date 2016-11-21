#include "uf2.h"

static uint32_t timerLow;
uint32_t timerHigh, resetHorizon, blinkHorizon;

void timerTick(void) {
    if (timerLow-- == 0) {
        timerLow = TIMER_STEP;
        timerHigh++;
        if (resetHorizon && timerHigh >= resetHorizon) {
            resetHorizon = 0;
            NVIC_SystemReset();
        }
        if (timerHigh < blinkHorizon)
            bulb_toggle();
        else
            bulb_on();
    }
}

void panic(void) {
    while (1) {
    }
}

void writeNum(char *buf, uint32_t n) {
    buf[0] = '0';
    buf[1] = 'x';
    int i = 2;
    int sh = 28;
    while (sh >= 0) {
        int d = (n >> sh) & 0xf;
        if (d || sh == 0 || i > 2) {
            buf[i++] = d > 9 ? 'a' + d - 10 : '0' + d;
        }
        sh -= 4;
    }
    buf[i] = 0;
}

#if USE_LOGS
static struct {
    int ptr;
    char buffer[4096];
} logStoreUF2;

void logwritenum(uint32_t n) {
    char buff[12];
    writeNum(buff, n);
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