#ifndef UF2_H
#define UF2_H 1

#include "board_config.h"

#include "samd21.h"
#include "uf2format.h"
#include "main.h"
#include "cdc_enumerate.h"
#include "sam_ba_monitor.h"
#include "usart_sam_ba.h"
#include <stdio.h>
#include <string.h>

#include <compiler.h>

#undef DISABLE
#undef ENABLE

#ifndef INDEX_URL
#define INDEX_URL "https://www.pxt.io/"
#endif

#define UF2_VERSION "v1.0.0"

// needs to be more than ~4200 (to force FAT16)
#define NUM_FAT_BLOCKS 8000

extern char serialNumber[];

// Logging to help debugging
#define USE_LOGS 1
// Check various conditions; best leave on
#define USE_ASSERT 1
// Enable reading flash via FAT files; otherwise drive will appear empty
#define USE_FAT 1
// Enable USB CDC (Communication Device Class; i.e., USB serial) monitor for Arduino style flashing
#define USE_CDC 0
// Support the UART (real serial port, not USB)
#define USE_UART 0
// Show serial number in info_uf2.txt file
#define USE_SERIAL_NUMBER 1

// End of config

#define USE_MONITOR (USE_CDC || USE_UART)
#define TIMER_STEP 1500

/*
From CPU config:
#define FLASH_SIZE            0x8000UL
#define FLASH_PAGE_SIZE       64
#define FLASH_NB_OF_PAGES     512
*/

#define FLASH_ROW_SIZE (FLASH_PAGE_SIZE * 4)

#define NOOP                                                                                       \
    do {                                                                                           \
    } while (0)

#if USE_LOGS
void logmsg(const char *msg);
void logval(const char *lbl, uint32_t v);
void logwritenum(uint32_t n);
void logwrite(const char *msg);
#else
#define logmsg(...) NOOP
#define logval(...) NOOP
#define logwritenum(...) NOOP
#define logwrite(...) NOOP
#endif

void panic(void);

#if USE_ASSERT
#define assert(cond)                                                                               \
    if (!(cond)) {                                                                                 \
        logwrite("Assertion failed: ");                                                            \
        logwrite(#cond);                                                                           \
        logwrite("\n");                                                                            \
        panic();                                                                                   \
    }
#else
#define assert(cond) NOOP
#endif

extern volatile bool b_sam_ba_interface_usart;
void flash_write_row(uint32_t *dst, uint32_t *src);
void flash_erase_row(uint32_t *dst);
void flash_write_words(uint32_t *dst, uint32_t *src, uint32_t n_words);

int writeNum(char *buf, uint32_t n, bool full);

// index of highest LUN
#define MAX_LUN 0
void process_msc(void);
void msc_reset(void);
//! Static block size for all memories
#define UDI_MSC_BLOCK_SIZE 512L

void read_block(uint32_t block_no, uint8_t *data);
void write_block(uint32_t block_no, uint8_t *data);
void padded_memcpy(char *dst, const char *src, int len);
void init_fat(void);

void resetIntoApp(void);
void system_init(void);

inline void bulb_init(void) { PORT->Group[BULB_PORT].DIRSET.reg = (1 << BULB_PIN); }
inline void bulb_toggle(void) { PORT->Group[BULB_PORT].OUTTGL.reg = (1 << BULB_PIN); }
inline void bulb_on(void) { PORT->Group[BULB_PORT].OUTSET.reg = (1 << BULB_PIN); }
inline void bulb_off(void) { PORT->Group[BULB_PORT].OUTCLR.reg = (1 << BULB_PIN); }

extern uint32_t timerHigh, resetHorizon, blinkHorizon;
void timerTick(void);

#define CONCAT_1(a, b) a ## b
#define CONCAT_0(a, b) CONCAT_1(a, b)
#define STATIC_ASSERT(e) enum { CONCAT_0(_static_assert_, __LINE__) = 1 / ((e) ? 1 : 0) }

#endif
