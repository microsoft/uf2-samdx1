#!/bin/sh
$HOME/Library/Arduino15/packages/arduino/tools/bossac/1.7.0/bossac \
	-i -d --port=cu.usbmodem1411 -U true -i -e -w -v tmp/mpok.bin -R
