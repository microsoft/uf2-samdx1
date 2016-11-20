#ifndef UF2FORMAT_H
#define UF2FORMAT_H 1

#include <stdint.h>
#include <stdbool.h>

// if you increase that, you will also need to update the linker script file
#define APP_START_ADDRESS 0x00002000

#define UF2_MAGIC_START 0x9E5D5157UL
#define UF2_MAGIC_END   0xD8B16F30UL

typedef struct {
    uint32_t magicStart;
    uint32_t flags;
    uint32_t targetAddr;
    uint32_t payloadSize;
    uint32_t reserved[3];
    uint8_t data[512 - 8 * 4];
    uint32_t magicEnd;
} UF2_Block;

#endif
