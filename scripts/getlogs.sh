#!/bin/sh

scripts/openocd.sh -c "log_output openocd.log" &
sleep 1
echo "halt; mdb 0x20000000 8000; resume; shutdown" | nc localhost 4444 > logs.tmp
node scripts/getlogs.js
rm -f openocd.log logs.tmp
