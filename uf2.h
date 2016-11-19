#ifndef UF2_H
#define UF2_H 1

#include <stdint.h>

// needs to be more than ~4200 (to force FAT16)
#define NUM_FAT_BLOCKS 4200
#define VENDOR_NAME "PXT.IO"
#define PRODUCT_NAME "UF2 Bootloader"
#define SERIAL_NUMBER "F23456789ABC"
#define USE_LOGS 1
#define USE_ASSERT 1
#define USE_UART 1
#define USE_FAT 1


#define NOOP do{}while(0)

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
#define assert(cond) \
  if (!(cond)) { \
    logwrite("Assertion failed: "); \
    logwrite(#cond); \
    logwrite("\n"); \
    panic(); }
#else
#define assert(cond) NOOP
#endif

//extern volatile bool b_sam_ba_interface_usart;

#endif