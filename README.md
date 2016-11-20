# UF2 Bootloader

This repository contains a bootloader, derived from Atmel's SAM-BA,
which in addition to the USB CDC (serial) protocol, also supports
the USB MSC (mass storage).

## UF2 

UF2 (USB Flashing Format) is a name of a file format, that is particularly 
suitable for flashing devices over MSC devices. The file consists
of 512 byte blocks, each of which is self-contained and independent
of others.

Each 512 byte block consist of (see `uf2format.h` for details):
* magic numbers at the beginning and at the end
* address where the data should be flashed
* size of data
* data (up to 480 bytes; for SAMD it's 256 bytes so it's easy to flash in one go)

Thus, it's really easy for the microcontroller to recognize a block of
a UF2 file is written and immediately write it to flash.

In `uf2conv.c` you can find a small converter from `.bin` to `.uf2`.

## Features

* USB CDC (Serial) monitor mode compatible with Arduino (including XYZ commands) and BOSSA flashing tool
* USB MSC interface for writing UF2 files
* reading of the contests of the flash as an UF2 file via USB MSC
* UART Serial monitor mode (typically disabled due to space constraints)
* In-memory logging for debugging - use the `logs` target to extract the logs using `openocd`
* double-tap reset to stay in the bootloader mode
* automatic reset after UF2 file is written

## Build

### Requirements

* `make` and an Unix environment
* `arm-none-eabi-gcc` in the path (the one coming with Yotta will do just fine)
* `openocd` - you can use the one coming with Arduino (after your install the M0 board support)

Atmel Studio is not supported.

You will need a board with `openocd` support.

Arduino Zero (or M0 Pro) will work just fine as it has an integrated USB EDBG
port. You need to connect both USB ports to your machine to debug - one is for
flashing and getting logs, the other is for the exposed MSC interface.

Otherwise, you can use other SAMD21 board and an external `openocd` compatible
debugger.

### Configuration

There is a number of configuration parameters at the top of `uf2.h` file.
Adjust them to your liking.

By default, you cannot enable all the features, as the bootloader would exceed 
the 8k allocated to it by Arduino etc. It will assert on startup that it's not bigger
than 8k. Also, the linker script will not allow it.

The bootloader sits at 0x00000000, and the application starts at 0x00002000.


## License

The original SAM-BA bootloader is licensed under BSD-like license from Atmel.

The new code is licensed under MIT.
