#!/bin/sh

p=/Users/michal/Library/Arduino15/packages/arduino
pp=$p/tools/openocd/0.9.0-arduino
$pp/bin/openocd \
	-d2 -s $pp/share/openocd/scripts/ \
	-f $p/hardware/samd/1.6.8/variants/arduino_zero/openocd_scripts/arduino_zero.cfg \
	"$@"
