
#include "cdc_enumerate.h"
#include <compiler.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    uint8_t JumpInstruction[3];
    uint8_t OEMInfo[8];
    uint16_t SectorSize;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FATCopies;
    uint16_t RootDirectoryEntries;
    uint16_t TotalSectors16;
    uint8_t MediaDescriptor;
    uint16_t SectorsPerFAT;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t TotalSectors32;
    uint8_t PhysicalDriveNum;
    uint8_t Reserved;
    uint8_t ExtendedBootSig;
    uint32_t VolumeSerialNumber;
    uint8_t VolumeLabel[11];
    uint8_t FilesystemIdentifier[8];
} __attribute__((packed)) FAT_BootBlock;

struct DirEntry {
    char name[8];
    char ext[3];
    uint8_t attrs;
    uint8_t reserved;
    uint8_t createTimeFine;
    uint16_t createTime;
    uint16_t createDate;
    uint16_t lastAccessDate;
    uint16_t highStartCluster;
    uint16_t updateTime;
    uint16_t updateDate;
    uint16_t startCluster;
    uint32_t size;
} __attribute__((packed));

struct TextFile {
    const char *name;
    const char *ext;
    const char *content;
};

static const struct TextFile info[] = {
    {.name = "INFO", .ext = "TXT", .content = "Hello world! build at " __TIME__}};

#define NUM_INFO (sizeof(info) / sizeof(info[0]))

#define RESERVED_SECTORS 1
#define ROOT_DIR_SECTORS 1
#define SECTORS_PER_FAT ((NUM_FAT_BLOCKS * 2 + 511) / 512)

#define START_FAT0 RESERVED_SECTORS
#define START_FAT1 (START_FAT0 + SECTORS_PER_FAT)
#define START_ROOTDIR (START_FAT1 + SECTORS_PER_FAT)
#define START_CLUSTERS (START_ROOTDIR + ROOT_DIR_SECTORS)

static const FAT_BootBlock BootBlock = {
    .JumpInstruction = {0xeb, 0x3c, 0x90},
    .OEMInfo = "UF2 UF2 ",
    .SectorSize = 512,
    .SectorsPerCluster = 1,
    .ReservedSectors = RESERVED_SECTORS,
    .FATCopies = 2,
    .RootDirectoryEntries = (ROOT_DIR_SECTORS * 512 / 32),
    .TotalSectors16 = NUM_FAT_BLOCKS - 2,
    .MediaDescriptor = 0xF8,
    .SectorsPerFAT = SECTORS_PER_FAT,
    .SectorsPerTrack = 1,
    .Heads = 1,
    .ExtendedBootSig = 0x29,
    .VolumeSerialNumber = 0xdeadbeef,
    .VolumeLabel = "SAMD UF2   ",
    .FilesystemIdentifier = "FAT16   ",
};

void padded_memcpy(char *dst, const char *src, int len) {
    for (int i = 0; i < len; ++i) {
        if (*src)
            *dst = *src++;
        else
            *dst = ' ';
        dst++;
    }
}

void read_block(uint32_t block_no, uint8_t *data) {
    memset(data, 0, 512);
    uint32_t sectionIdx = block_no;

    if (block_no == 0) {
        memcpy(data, &BootBlock, sizeof(BootBlock));
        data[510] = 0x55;
        data[511] = 0xaa;
        logval("data[0]", data[0]);
    } else if (block_no < START_ROOTDIR) {
        sectionIdx -= START_FAT0;
        logval("sidx", sectionIdx);
        if (sectionIdx >= SECTORS_PER_FAT)
            sectionIdx -= SECTORS_PER_FAT;
        if (sectionIdx == 0) {
            data[0] = 0xf0;
            for (int i = 1; i < NUM_INFO * 2 + 4; ++i) {
                data[i] = 0xff;
            }
        }
    } else if (block_no < START_CLUSTERS) {
        sectionIdx -= START_ROOTDIR;
        if (sectionIdx == 0) {
            for (int i = 0; i < NUM_INFO; ++i) {
                struct DirEntry *d = (void *)(data + i * sizeof(d));
                const struct TextFile *inf = &info[i];
                assert(sizeof(*d) == 32);
                d->size = strlen(inf->content);
                d->startCluster = i + 2;
                padded_memcpy(d->name, inf->name, 8);
                padded_memcpy(d->ext, inf->ext, 3);
            }
        }
    } else {
        sectionIdx -= START_CLUSTERS;
        logval("data cluster", sectionIdx);
        if (sectionIdx < NUM_INFO) {
            memcpy(data, info[sectionIdx].content, strlen(info[sectionIdx].content));
        }
    }
}

void write_block(uint32_t block_no, uint8_t *data) {}
