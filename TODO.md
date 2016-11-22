## Current
* [x] separate logs out
* [x] no volume label shows under Windows
* [x] extend magic with "UF2\n" string
* [x] align data in block to 32 bytes (for hex viewer)
* [x] show board serial number and name in info file
* [x] organize board configs in directories
* [x] if `!USE_CDC && !USE_UART` - don't compile monitor
* [x] if `!USE_CDC` don't compile the CDC code (not only exclude descriptors)
* [x] write user program for updating bootloader
* [ ] document self-updater
* [ ] write u2fconv in .js
* [ ] investigate some blinking
* [ ] add optional logic to self-updater to check if existing bootloader has the same board-id

## Bigger
* [ ] look into reset into bootloader from host to continue flashing
* [ ] use BOOTPROT bits - requires device reset to set
