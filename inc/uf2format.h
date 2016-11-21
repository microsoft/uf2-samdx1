#ifndef UF2FORMAT_H
#define UF2FORMAT_H 1

#include <stdint.h>
#include <stdbool.h>

// All entries are little endian.

// if you increase that, you will also need to update the linker script file
#define APP_START_ADDRESS 0x00002000

#define UF2_MAGIC_START0 0x0A324655UL // "UF2\n"
#define UF2_MAGIC_START1 0x9E5D5157UL // Randomly selected
#define UF2_MAGIC_END 0x0AB16F30UL    // Ditto

// If set, the block is "comment" and should not be flashed to the device
#define UF2_FLAG_NOFLASH 0x00000001

typedef struct {
    // 32 byte header
    uint32_t magicStart0;
    uint32_t magicStart1;
    uint32_t flags;
    uint32_t targetAddr;
    uint32_t payloadSize;
    uint32_t reserved0;
    uint32_t reserved1;
    uint32_t reserved2;

    // raw data;
    uint8_t data[476];

    // store magic also at the end to limit damage from partial block reads
    uint32_t magicEnd;
} UF2_Block;

#endif
