# UF2 Bootloader

This archive contains information about the binaries of the bootloader, built from this
repo for various SAMD21 boards. These are derived from Atmel's SAM-BA, 
also supports USB MSC (mass storage) and USB HID update protocols.

**UF2 (USB Flashing Format)** is a name of a file format supported by the bootloader, 
that is particularly suitable for flashing devices over mass storage interfaces. The file 
consists of 512 byte blocks, each of which is self-contained, easy to identify, and 
independent of others. For more details, see [UF2 specification repo](https://github.com/Microsoft/uf2).

The binaries here are built based on the [source repository](https://github.com/Microsoft/uf2-samd21).

## Usage

Once the bootloader is flashed to your device, it will react to a double-click
of the reset button. At that point, the indicator LED of the board should start
pulsing slowly (about 1 per second).

You should also see a new USB drive appear on the connected computer. 
The drive should have an `INFO_UF2.TXT` file on it with various information.
You can drop `.UF2` files on that drive to program the board.
You can also use USB HID protocol dubbed HF2 to talk to the board.

## Choosing the right bootloader

All the bootloaders in this archive use almost the same code and will generally
work on most SAMD21 boards. However, the LED might not blink and you may get the 
device mis-identified by the operating system.

Thus, it's good to pick the bootloader from the right directory.

If you're not sure and you are already running an UF2 bootloader, you can
look at the `INFO_UF2.TXT` file exposed by the current bootloader. In particular
look for `Model:` and `Board-ID:`. Then compare that with the comment at
the beginning of `update-bootloader.ino` in various directories.

As said before, consequences of choosing the wrong one are not too dramatic.

Once you have chosen the right directory, there are three ways to update your bootloader.

## I'm already running a version of UF2

In case you can already see a UF2 drive on your computer (it should have `INFO_UF2.TXT` 
file on it), you can just drop `update-bootloader.uf2` file there.

## I have Arduino running

If you have Arduino IDE available, copy&paste `update-bootloader.ino` sketch
and upload it to the device.

Please note, that if you're running an Arduino/Genuino board and 
Windows 8.1 or earlier, this may prevent Arduino IDE from recognizing 
the serial emulation on the board.

This does not apply to Adafruit boards, only to Arduino and Genuino.
It also does not apply to Windows 10, Mac, or Linux.

## I have a SWD or JTAG programmer

In case you have a programmer (like the one that comes built in on Arduino Zero or an 
external one like IBDAP), you can burn the `bootloader.bin` file. Make sure to use 
`0x00000000` as the base address.

You may also need to remove bootloader protection.
There is a script in the [source repository](https://github.com/Microsoft/uf2-samd21) 
to do it.

Do not use `at91samd bootloader 0` command of OpenOCD - it's buggy.

## Code of Conduct

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## License

The original SAM-BA bootloader is licensed under BSD-like license from Atmel.

The new code is licensed under MIT.
