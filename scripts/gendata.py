import sys
import os


def update_crc(new_byte, current_crc):
    crc = current_crc ^ new_byte << 8
    for cmpt in range(8):
        if crc & 0x8000:
            crc = crc << 1 ^ 0x1021
        else:
            crc = crc << 1
        crc &= 0xffff
    return crc


# Load the bootloader file
bootloader_size = int(sys.argv[1])
bin_name = sys.argv[2]
bootloader = bytearray()
with open(bin_name, "rb") as bootloader_bin:
    bootloader.extend(bootloader_bin.read())

# Fill the remaining space with 0xff.
bootloader.extend([0xff] * (bootloader_size - len(bootloader)))

# Output the bootloader binary data into C code to use in the self updater.
with open(os.path.dirname(bin_name) + "/selfdata.c", "w") as output:
    output.write("#include <stdint.h>\n")
    output.write("const uint8_t bootloader[{}] ".format(bootloader_size) +
                 "__attribute__ ((aligned (4))) = {")
    crcs = []
    crc = 0
    for row in range(bootloader_size / 16):
        # Save the crc every 1k.
        if row % (1024 / 16) == 0 and row > 0:
            crcs.append(crc)
            crc = 0
        start_index = row * 16
        row_bytes = bootloader[start_index:start_index+16]
        formatted_bytes = ["0x{:02x}".format(x) for x in row_bytes]
        output.write(", ".join(formatted_bytes) + ",\n")

        # Update the crc
        for b in row_bytes:
            crc = update_crc(b, crc)
    crcs.append(crc)  # Add the last crc
    output.write("\n};\n")

    crcs = ["0x{:04x}".format(x) for x in crcs]
    output.write("const uint16_t bootloader_crcs[] = {" +
                 " ,".join(crcs) + "};\n")
