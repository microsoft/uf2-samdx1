
#include "uf2.h"

#define SERIAL0 (*(uint32_t *)0x0080A00C)
#define SERIAL1 (*(uint32_t *)0x0080A040)
#define SERIAL2 (*(uint32_t *)0x0080A044)
#define SERIAL3 (*(uint32_t *)0x0080A048)

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

typedef struct {
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
} __attribute__((packed)) DirEntry;

STATIC_ASSERT(sizeof(DirEntry) == 32);

struct TextFile {
    const char name[11];
    const char *content;
};

#if USE_FAT
uint32_t infoPtr;
char infoFile[256];
static const struct TextFile info[] = {
    {.name = "INFO    TXT", .content = infoFile}, {.name = "CURRENT UF2"},
};
#define NUM_INFO (sizeof(info) / sizeof(info[0]))

#define FLASH_SKIP APP_START_ADDRESS
#define UF2_SIZE ((FLASH_SIZE - FLASH_SKIP) * 2)
#define UF2_SECTORS (UF2_SIZE / 512)
#define UF2_FIRST_SECTOR (NUM_INFO + 1)
#define UF2_LAST_SECTOR (UF2_FIRST_SECTOR + UF2_SECTORS - 1)

static void infoWrite(const char *ptr) {
    int len = strlen(ptr);
    memcpy(infoFile + infoPtr, ptr, len);
    infoPtr += len;
}

static void infoWriteNum(uint32_t num) {
    writeNum(infoFile + infoPtr, num, true);
    infoPtr += 8;
}

void init_fat() {
    infoWrite("UF2 Bootloader.\n"
              "Built at: " __DATE__ " " __TIME__ "\n"
              "Model: " VENDOR_NAME " " PRODUCT_NAME "\n"
              "Serial: ");

    infoWriteNum(SERIAL0);
    infoWriteNum(SERIAL1);
    infoWriteNum(SERIAL2);
    infoWriteNum(SERIAL3);

    assert(infoPtr < sizeof(infoFile));
    infoFile[infoPtr] = 0;
}
#endif

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
    .VolumeLabel = VOLUME_LABEL,
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
        // logval("sidx", sectionIdx);
        if (sectionIdx >= SECTORS_PER_FAT)
            sectionIdx -= SECTORS_PER_FAT;
#if USE_FAT
        if (sectionIdx == 0) {
            data[0] = 0xf0;
            for (int i = 1; i < NUM_INFO * 2 + 4; ++i) {
                data[i] = 0xff;
            }
        }
        for (int i = 0; i < 256; ++i) {
            uint32_t v = sectionIdx * 256 + i;
            if (UF2_FIRST_SECTOR <= v && v <= UF2_LAST_SECTOR)
                ((uint16_t *)(void *)data)[i] = v == UF2_LAST_SECTOR ? 0xffff : v + 1;
        }
#else
        if (sectionIdx == 0)
            memcpy(data, "\xf0\xff\xff\xff", 4);
#endif
    }
#if USE_FAT
    else if (block_no < START_CLUSTERS) {
        sectionIdx -= START_ROOTDIR;
        if (sectionIdx == 0) {
            DirEntry *d = (void *)data;
            padded_memcpy(d->name, BootBlock.VolumeLabel, 11);
            d->attrs = 0x28;
            for (int i = 0; i < NUM_INFO; ++i) {
                d++;
                const struct TextFile *inf = &info[i];
                d->size = inf->content ? strlen(inf->content) : UF2_SIZE;
                d->startCluster = i + 2;
                padded_memcpy(d->name, inf->name, 11);
            }
        }
    } else {
        sectionIdx -= START_CLUSTERS;
        if (sectionIdx < NUM_INFO - 1) {
            memcpy(data, info[sectionIdx].content, strlen(info[sectionIdx].content));
        } else {
            sectionIdx -= NUM_INFO - 1;
            uint32_t addr = sectionIdx * 256 + FLASH_SKIP;
            if (addr < FLASH_SIZE) {
                UF2_Block *bl = (void *)data;
                bl->magicStart0 = UF2_MAGIC_START0;
                bl->magicStart1 = UF2_MAGIC_START1;
                bl->magicEnd = UF2_MAGIC_END;
                bl->targetAddr = addr;
                bl->payloadSize = 256;
                memcpy(bl->data, (void *)addr, bl->payloadSize);
            }
        }
    }
#endif
}

void write_block(uint32_t block_no, uint8_t *data) {
    UF2_Block *bl = (void *)data;
    if (bl->magicStart0 != UF2_MAGIC_START0 || bl->magicStart1 != UF2_MAGIC_START1 ||
        bl->magicEnd != UF2_MAGIC_END || (bl->flags & UF2_FLAG_NOFLASH)) {
        // logval("skip write @", block_no);
        return;
    }
    if (bl->payloadSize != 256) {
        logval("bad payload size", bl->payloadSize);
        return;
    }
    if ((bl->targetAddr & 0xff) || bl->targetAddr < APP_START_ADDRESS ||
        bl->targetAddr >= FLASH_SIZE) {
        logval("invalid target addr", bl->targetAddr);
        return;
    }

    logval("write block at", bl->targetAddr);
    flash_write_row((void *)bl->targetAddr, (void *)bl->data);

    blinkHorizon = timerHigh + 5;
    resetHorizon = timerHigh + 50;
}
