sh scripts/openocd.sh \
	-c "telnet_port disabled; init; halt; at91samd bootloader 0; program {{build/zero/uf2-bootloader.bin}} verify reset; shutdown "
