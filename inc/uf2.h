#ifndef UF2_H
#define UF2_H 1

#include "board_config.h"

#include "samd21.h"
#define UF2_DEFINE_HANDOVER 1 // for testing
#include "uf2format.h"
#include "uf2hid.h"
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

#define UF2_VERSION_BASE "v1.1.3"

// needs to be more than ~4200 (to force FAT16)
#define NUM_FAT_BLOCKS 8000

// Logging to help debugging
#define USE_LOGS 1
// Check various conditions; best leave on
#define USE_ASSERT 1
// Enable reading flash via FAT files; otherwise drive will appear empty
#define USE_FAT 0
// Enable USB CDC (Communication Device Class; i.e., USB serial) monitor for Arduino style flashing
#define USE_CDC 0
// Support the UART (real serial port, not USB)
#define USE_UART 0
// Support Human Interface Device (HID) - serial, flashing and debug
#define USE_HID 1
// Expose HID via WebUSB (requires USE_HID)
#define USE_WEBUSB 1

#define USE_DBG_MSC 0
#define USE_HANDOVER 0

#if USE_CDC
#define CDC_VERSION "S"
#else
#define CDC_VERSION ""
#endif

#if USE_LOGS
#define LOGS_VERSION "L"
#else
#define LOGS_VERSION ""
#endif

#if USE_FAT
#define FAT_VERSION "F"
#else
#define FAT_VERSION ""
#endif

#if USE_ASSERT
#define ASSERT_VERSION "A"
#else
#define ASSERT_VERSION ""
#endif

#if USE_HID
#define HID_VERSION "H"
#else
#define HID_VERSION ""
#endif

#define UF2_VERSION UF2_VERSION_BASE " " CDC_VERSION LOGS_VERSION FAT_VERSION ASSERT_VERSION HID_VERSION

// End of config

#define USE_MONITOR (USE_CDC || USE_UART)
#define TIMER_STEP 1500

/*
From CPU config:
#define FLASH_SIZE            0x8000UL
#define FLASH_PAGE_SIZE       64
#define FLASH_NB_OF_PAGES     512
*/

// These two need to be defined as plain decimal numbers, as we're using # on them
#define FLASH_ROW_SIZE 256
#ifndef FLASH_NUM_ROWS
#define FLASH_NUM_ROWS 1024
#endif

#define NOOP                                                                                       \
    do {                                                                                           \
    } while (0)

#if USE_LOGS
void logmsg(const char *msg);
void logval(const char *lbl, uint32_t v);
void logwritenum(uint32_t n);
void logwrite(const char *msg);
void logreset(void);
#else
#define logmsg(...) NOOP
#define logval(...) NOOP
#define logwritenum(...) NOOP
#define logwrite(...) NOOP
#define logreset() NOOP
#endif

#if USE_DBG_MSC
#define DBG_MSC(x) x
#else
#define DBG_MSC(x) NOOP
#endif

void panic(int code);

#if USE_ASSERT
#define assert(cond)                                                                               \
    if (!(cond)) {                                                                                 \
        panic(__LINE__);                                                                           \
    }
#else
#define assert(cond) NOOP
#endif

extern volatile bool b_sam_ba_interface_usart;
void flash_write_row(uint32_t *dst, uint32_t *src);
void flash_erase_row(uint32_t *dst);
void flash_write_words(uint32_t *dst, uint32_t *src, uint32_t n_words);

int writeNum(char *buf, uint32_t n, bool full);

void process_hid(void);

// index of highest LUN
#define MAX_LUN 0
void process_msc(void);
void msc_reset(void);
//! Static block size for all memories
#define UDI_MSC_BLOCK_SIZE 512L

void read_block(uint32_t block_no, uint8_t *data);
#define MAX_BLOCKS (FLASH_SIZE / 256 + 100)
typedef struct {
    uint32_t numBlocks;
    uint32_t numWritten;
    uint8_t writtenMask[MAX_BLOCKS / 8 + 1];
} WriteState;
void write_block(uint32_t block_no, uint8_t *data, bool quiet, WriteState *state);
void padded_memcpy(char *dst, const char *src, int len);

// Last word in RAM
// Unlike for ordinary applications, our link script doesn't place the stack at the bottom
// of the RAM, but instead after all allocated BSS.
// In other words, this word should survive reset.
#define DBL_TAP_PTR ((volatile uint32_t *)(HMCRAMC0_ADDR + HMCRAMC0_SIZE - 4))
#define DBL_TAP_MAGIC 0xf01669ef // Randomly selected, adjusted to have first and last bit set
#define DBL_TAP_MAGIC_QUICK_BOOT 0xf02669ef

void resetIntoApp(void);
void resetIntoBootloader(void);
void system_init(void);

#define LED_TICK led_tick

#define PINOP(pin, OP) (PORT->Group[(pin) / 32].OP.reg = (1 << ((pin) % 32)))

void led_tick(void);
void led_signal(void);
void led_init(void);

#define LED_MSC_OFF() PINOP(LED_PIN, OUTCLR)
#define LED_MSC_ON() PINOP(LED_PIN, OUTSET)
#define LED_MSC_TGL() PINOP(LED_PIN, OUTTGL)

extern uint32_t timerHigh, resetHorizon;
void timerTick(void);
void delay(uint32_t ms);

#define CONCAT_1(a, b) a##b
#define CONCAT_0(a, b) CONCAT_1(a, b)
#define STATIC_ASSERT(e) enum { CONCAT_0(_static_assert_, __LINE__) = 1 / ((e) ? 1 : 0) }

STATIC_ASSERT(FLASH_ROW_SIZE == FLASH_PAGE_SIZE * 4);
STATIC_ASSERT(FLASH_ROW_SIZE == NVMCTRL_ROW_SIZE);
STATIC_ASSERT(FLASH_NUM_ROWS * 4 == FLASH_NB_OF_PAGES);
// WebUSB requires HID
STATIC_ASSERT(!USE_WEBUSB || USE_HID);

extern const char infoUf2File[];

#endif
