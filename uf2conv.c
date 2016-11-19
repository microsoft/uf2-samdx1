#include <stdio.h>
#include <string.h>
#include "uf2format.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s file.bin [file.uf2]\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        fprintf(stderr, "No such file: %s\n", argv[1]);
        return 1;
    }

    const char *outname = argc > 2 ? argv[2] : "flash.uf2";

    FILE *fout = fopen("flash.uf2", "wb");

    UF2_Block bl;
    memset(&bl, 0, sizeof(bl));
   
    bl.magicStart = UF2_MAGIC_START;
    bl.magicEnd = UF2_MAGIC_END;
    bl.targetAddr = APP_START_ADDRESS;
    bl.payloadSize = 256;
    int numbl = 0;
    while (fread(bl.data, 1, bl.payloadSize, f)) {
        fwrite(&bl, 1, sizeof(bl), fout);
        numbl++;
        bl.targetAddr += bl.payloadSize;
    }
    fclose(fout);
    fclose(f);
    printf("Wrote %d blocks to %s\n", numbl, outname);
    return 0;
}