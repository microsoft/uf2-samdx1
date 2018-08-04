#include "uf2.h"
#include "neopixel.h"
#include "tcc_buzzer.h"

void RGBLED_set_color(uint32_t color);

#ifdef BOARD_NEOPIXEL_PIN
#define COLOR_START 0x040000
#define COLOR_USB 0x000400
#define COLOR_UART 0x040400
#define COLOR_LEAVE 0x000000
#define COLOR_FLASHWR 0x040404
#else
#define COLOR_START 0x000040
#define COLOR_USB 0x004000
#define COLOR_UART 0x404000
#define COLOR_LEAVE 0x400040
#define COLOR_FLASHWR 0x404040
#endif

// Status Signalling - Default Timings, Target Boards can override.
// Heartbeat Default ON/OFF Times (miliseconds).
// NOTE: Can not change REPS, ALWAYS = 0.
#ifndef SIGHB_ON_MS
#define SIGHB_ON_MS 500
#endif

#ifndef SIGHB_OFF_MS
#define SIGHB_OFF_MS 500
#endif
#define SIGHB_REPS 0

// Start Default ON/OFF Times (miliseconds).
#ifndef SIGSTART_ON_MS
#define SIGSTART_ON_MS 100
#endif
#ifndef SIGSTART_OFF_MS
#define SIGSTART_OFF_MS 100
#endif
#ifndef SIGSTART_REPS
#define SIGSTART_REPS 10
#endif

// USB Default ON/OFF Times (miliseconds).
#ifndef SIGUSB_ON_MS
#define SIGUSB_ON_MS 250
#endif
#ifndef SIGUSB_OFF_MS
#define SIGUSB_OFF_MS 250
#endif
#ifndef SIGUSB_REPS
#define SIGUSB_REPS 4
#endif

// UART Default ON/OFF Times (miliseconds).
#ifndef SIGUART_ON_MS
#define SIGUART_ON_MS 750
#endif
#ifndef SIGUART_OFF_MS
#define SIGUART_OFF_MS 250
#endif
#ifndef SIGUART_REPS
#define SIGUART_REPS 1
#endif

// Leave Default ON/OFF Times (miliseconds).
#ifndef SIGLEAVE_ON_MS
#define SIGLEAVE_ON_MS 0
#endif
#ifndef SIGLEAVE_OFF_MS
#define SIGLEAVE_OFF_MS 1000
#endif
#ifndef SIGLEAVE_REPS
#define SIGLEAVE_REPS 255
#endif

// Flash Write Default ON/OFF Times (miliseconds).
#ifndef SIGFLASHWR_ON_MS
#define SIGFLASHWR_ON_MS 1000
#endif
#ifndef SIGFLASHWR_OFF_MS
#define SIGFLASHWR_OFF_MS 1000
#endif
#ifndef SIGFLASHWR_REPS
#define SIGFLASHWR_REPS 10
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

typedef struct {
    uint16_t ontime;
    uint16_t offtime;
    uint8_t  rpt;
} signal;

#define SIGTIME(SIG) {SIG ## _ON_MS, (SIG ## _ON_MS + SIG ## _OFF_MS), SIG ## _REPS}

const signal states[SIGNAL_MAX] = {
    SIGTIME(SIGHB),      // SIGNAL_HB
    SIGTIME(SIGSTART),   // SIGNAL_START
    SIGTIME(SIGUSB),     // SIGNAL_USB
#if USE_UART
    SIGTIME(SIGUART),    // SIGNAL_UART
#endif    
    SIGTIME(SIGLEAVE),   // SIGNAL_LEAVE
    SIGTIME(SIGFLASHWR), // SIGNAL_FLASHWR
};

static uint16_t now;
static signal active_signal;

// signal_tick is the systick IRQ.  
// It SHOULD be called SYSTICK_FREQ times a second.
void SysTick_Handler(void) {
    // Flash the Heartbeat
    now++;
    if (now < active_signal.ontime) {
        SIGNAL_ON();
    } else if (now < (active_signal.offtime)) {
        SIGNAL_OFF();
    } else {
        // This allows us short bursts of flashing at different
        // duty cycles.  Just set hb_ontime and hb_offtime
        // to the required values.  Then set rpt to the number of
        // flash cycles you want.
        // After it completes, it will go back to the normal heartbeat.
        now = 0;
        active_signal.rpt--;
        if (active_signal.rpt == 0xFF) {
            active_signal = states[SIGNAL_HB];
        }
    }
}

#if defined(BOARD_RGBLED_CLOCK_PIN)
// Heartbeat has no color for a multi color led.
const uint32_t color_states[SIGNAL_MAX-1] = {
    COLOR_START,   // SIGNAL_START
    COLOR_USB,     // SIGNAL_USB
#if USE_UART    
    COLOR_UART,    // SIGNAL_UART
#endif    
    COLOR_LEAVE,   // SIGNAL_LEAVE
    COLOR_FLASHWR, // SIGNAL_FLASHWR
};
#endif    


//void signal_state(uint8_t state) {
void signal_state(SigState state) {
    active_signal = states[state];
#if defined(BOARD_RGBLED_CLOCK_PIN)
    if (state != SIGNAL_HB) { // Don't set a color for Heartbeats.
        RGBLED_set_color(color_states[state-1]);
    }
#endif    
}

void signal_init() {
#if defined(LED_PIN)    
    PINOP(LED_PIN, DIRSET);
    SIGNAL_ON();
#elif (defined(BUZZER_PIN) && defined(BUZZER_TCC))
    init_tcc_buzzer(); // TCC Buzzer acts a substitute for a led.
#endif    

    signal_state(SIGNAL_HB);

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
