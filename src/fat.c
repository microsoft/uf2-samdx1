
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
    const uint32_t length;
    const uint32_t startCluster;
};

#define STR0(x) #x
#define STR(x) STR0(x)
const char infoUf2File[] = //
    "UF2 Bootloader " UF2_VERSION "\r\n"
    "Model: " PRODUCT_NAME "\r\n"
    "Board-ID: " BOARD_ID "\r\n";

#if USE_FAT
#if USE_INDEX_HTM
const char indexFile[] = //
    "<!doctype html>\n"
    "<html>"
    "<body>"
    "<script>\n"
    "location.replace(\"" INDEX_URL "\");\n"
    "</script>"
    "</body>"
    "</html>\n";
#endif

#define UF2_SIZE (FLASH_SIZE * 2)
#define UF2_SECTORS (UF2_SIZE / 512)
#define UF2_FIRST_SECTOR (4)
#define UF2_LAST_SECTOR (UF2_FIRST_SECTOR + UF2_SECTORS - 1)

// Binary files use clusters behind UF2_LAST_SECTOR
#define BIN_SECTORS ((FLASH_SIZE - APP_START_ADDRESS) / 512)
#define BIN_FIRST_SECTOR (UF2_LAST_SECTOR + 1)
#define BIN_LAST_SECTOR (BIN_FIRST_SECTOR + BIN_SECTORS - 1)

static const struct TextFile info[] = {
    {.name = "INFO_UF2TXT", .content = infoUf2File, .length = sizeof(infoUf2File)-1, .startCluster = 2},
#if USE_INDEX_HTM
    {.name = "INDEX   HTM", .content = indexFile, .length = sizeof(indexFile)-1, .startCluster = 3},
#endif
    {.name = "CURRENT UF2", .length = (UF2_SIZE), .startCluster = UF2_FIRST_SECTOR},  // 4
#if USE_BINARY_FILES
    {.name = "CURRENT BIN", .length = (FLASH_SIZE - APP_START_ADDRESS), .startCluster = BIN_FIRST_SECTOR}, //  0x405
#endif
};
#define NUM_INFO (sizeof(info) / sizeof(info[0]))

#endif

#define RESERVED_SECTORS 1
#define ROOT_DIR_SECTORS 4
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
    .VolumeSerialNumber = 0x00420042,
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
        // logval("data[0]", data[0]);
    } else if (block_no < START_ROOTDIR) {
        sectionIdx -= START_FAT0;
        // logval("sidx", sectionIdx);
        if (sectionIdx >= SECTORS_PER_FAT)
            sectionIdx -= SECTORS_PER_FAT;
#if USE_FAT
        uint16_t *fat_data =  (void *)data;

        if (sectionIdx == 0) {
		    fat_data[0] = 0xfff0;
		    fat_data[1] = 0xffff;

		    for (int i = 0; i < NUM_INFO; ++i) {
                fat_data[i + 2] = (info[i].length >= 256)? info[i].startCluster : 0xffff;
            }
        }

        for (int i = 0; i < 256; ++i) {
            uint32_t v = sectionIdx * 256 + i;
            if (UF2_FIRST_SECTOR <= v && v <= UF2_LAST_SECTOR)
                fat_data[i] = (v == UF2_LAST_SECTOR) ? 0xffff : v + 1;
#if USE_BINARY_FILES
            if (BIN_FIRST_SECTOR <= v && v <= BIN_LAST_SECTOR)
                fat_data[i] = (v == BIN_LAST_SECTOR) ? 0xffff : v + 1;
#endif
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
            padded_memcpy(d->name, (const char *)BootBlock.VolumeLabel, 11);
            d->attrs = 0x28;
            for (int i = 0; i < NUM_INFO; ++i) {
                d++;
                const struct TextFile *inf = &info[i];
                d->size = inf->length;
                d->startCluster = inf->startCluster;
                padded_memcpy(d->name, inf->name, 11);
            }
        }
    } else {
        sectionIdx -= START_CLUSTERS;
        if (sectionIdx < 2) {
            memcpy(data, info[sectionIdx].content, strlen(info[sectionIdx].content));
        } else  {
            sectionIdx -= 2; //NUM_INFO - 1;

            if(sectionIdx < FLASH_SIZE/256) {
                uint32_t addr = sectionIdx * 256;
                UF2_Block *bl = (void *)data;
                bl->magicStart0 = UF2_MAGIC_START0;
                bl->magicStart1 = UF2_MAGIC_START1;
                bl->magicEnd = UF2_MAGIC_END;
                bl->blockNo = sectionIdx;
                bl->numBlocks = FLASH_SIZE / 256;
                bl->targetAddr = addr;
                bl->payloadSize = 256;
                memcpy(bl->data, (void *)addr, bl->payloadSize);
            }
#if USE_BINARY_FILES
            else {
                sectionIdx -= FLASH_SIZE/256;
                uint32_t addr = APP_START_ADDRESS + sectionIdx * 512;
                if (addr < FLASH_SIZE) {
                    memcpy(data, (void *)addr, 512);
                }
            }
#endif
        }
    }
#endif
}

void write_block(uint32_t block_no, uint8_t *data, bool quiet, WriteState *state) {
    UF2_Block *bl = (void *)data;

    if (!is_uf2_block(bl)) {
#if USE_BINARY_FILES
        static int32_t base_block_no = -1;

        // ignore writings in the first 3 seconds
        // binary_files_timer increments 48000 times per second
        if(binary_files_timer < (3 * 48000)) {
            return;
        }

        if (block_no >= START_CLUSTERS) {
            // first block written to flash ?
            if (base_block_no < 0 ) {
                // check 7 reserved handlers of SAMD21 (located after hard-fault handler)
                for(uint8_t i=15; i < (15+(7*4)); i++) {
                    if(data[i] != 0x00)
                        return;
                }

                // check block data length and ignore files <= 256 Bytes
                uint16_t len;
                for(len = 511; len > 0; len--) {
                    if((data[len] != 0x00) && (data[len] != 0xFF))
                        break;
                }
                if(len < 256)
                    return;

                // save first block no
                base_block_no = block_no;
            }

            // write block to flash
            uint32_t targetAddr = APP_START_ADDRESS + (block_no - base_block_no) * 512;
            if (targetAddr < FLASH_SIZE) {
                flash_write_row((void *)targetAddr, (void *)data);
                flash_write_row((void *)(targetAddr + 256), (void *)&data[256]);
            }
        }
        else if (base_block_no >= 0) {
            base_block_no = -1;
            resetHorizon = timerHigh + 150;
            binary_files_timer = 0;
        }
#endif
        return;
    }

    if ((bl->flags & UF2_FLAG_NOFLASH) || bl->payloadSize != 256 || (bl->targetAddr & 0xff) ||
        bl->targetAddr < APP_START_ADDRESS || bl->targetAddr >= FLASH_SIZE) {
#if USE_DBG_MSC
        if (!quiet)
            logval("invalid target addr", bl->targetAddr);
#endif
        // this happens when we're trying to re-flash CURRENT.UF2 file previously
        // copied from a device; we still want to count these blocks to reset properly
    } else {
        // logval("write block at", bl->targetAddr);
        flash_write_row((void *)bl->targetAddr, (void *)bl->data);
    }

    if (state && bl->numBlocks) {
        if (state->numBlocks != bl->numBlocks) {
            if (bl->numBlocks >= MAX_BLOCKS || state->numBlocks)
                state->numBlocks = 0xffffffff;
            else
                state->numBlocks = bl->numBlocks;
        }
        if (bl->blockNo < MAX_BLOCKS) {
            uint8_t mask = 1 << (bl->blockNo % 8);
            uint32_t pos = bl->blockNo / 8;
            if (!(state->writtenMask[pos] & mask)) {
                // logval("incr", state->numWritten);
                state->writtenMask[pos] |= mask;
                state->numWritten++;
            }
            if (state->numWritten >= state->numBlocks) {
                // wait a little bit before resetting, to avoid Windows transmit error
                // https://github.com/Microsoft/uf2-samd21/issues/11
                if (!quiet)
                    resetHorizon = timerHigh + 30;
                // resetIntoApp();
            }
        }
    } else {
        if (!quiet)
            resetHorizon = timerHigh + 300;
    }
}
