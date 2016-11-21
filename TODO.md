## Current
* [x] separate logs out
* [ ] no volume label shows under Windows
* [x] extend magic with "UF2\n" string
* [x] align data in block to 32 bytes (for hex viewer)
* [x] show board serial number and name in info file
* [ ] organize board configs in directories
* [ ] use BOOTPROT bits
* [x] if `!USE_CDC && !USE_UART` - don't compile monitor
* [x] if `!USE_CDC` don't compile the CDC code (not only exclude descriptors)

## Bigger
* [ ] look into reset into bootloader from host to continue flashing
