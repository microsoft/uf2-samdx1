/* ----------------------------------------------------------------------------
 *         SAM Software Package License
 * ----------------------------------------------------------------------------
 * Copyright (c) 2011-2014, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following condition is met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

#include "sam_ba_monitor.h"
#include "cdc_enumerate.h"
#include "compiler.h"
#include "iosamd21.h"
#include "uart_driver.h"
#include "usart_sam_ba.h"
#include <string.h>

const char RomBOOT_Version[] = SAM_BA_VERSION;
const char RomBOOT_ExtendedCapabilities[] = "[Arduino:XYZ]";

/* b_terminal_mode mode (ascii) or hex mode */
volatile bool b_terminal_mode = false;
volatile bool b_sam_ba_interface_usart = false;

void sam_ba_monitor_init(uint8_t com_interface) {
#if USE_UART
    // Selects the requested interface for future actions
    if (com_interface == SAM_BA_INTERFACE_USART) {
        b_sam_ba_interface_usart = true;
    }
#endif
}

/**
 * \brief This function allows data rx by USART
 *
 * \param *data  Data pointer
 * \param length Length of the data
 */
void sam_ba_putdata_term(uint8_t *data, uint32_t length) {
    uint8_t temp, buf[12], *data_ascii;
    uint32_t i, int_value;

    if (b_terminal_mode) {
        if (length == 4)
            int_value = *(uint32_t *)(void *)data;
        else if (length == 2)
            int_value = *(uint16_t *)(void *)data;
        else
            int_value = *(uint8_t *)(void *)data;

        data_ascii = buf + 2;
        data_ascii += length * 2 - 1;

        for (i = 0; i < length * 2; i++) {
            temp = (uint8_t)(int_value & 0xf);

            if (temp <= 0x9)
                *data_ascii = temp | 0x30;
            else
                *data_ascii = temp + 0x37;

            int_value >>= 4;
            data_ascii--;
        }
        buf[0] = '0';
        buf[1] = 'x';
        buf[length * 2 + 2] = '\n';
        buf[length * 2 + 3] = '\r';
        cdc_write_buf(buf, length * 2 + 4);
    } else
        cdc_write_buf(data, length);
    return;
}

volatile uint32_t sp;
void call_applet(uint32_t address) {
    uint32_t app_start_address;

    cpu_irq_disable();

    sp = __get_MSP();

    /* Rebase the Stack Pointer */
    __set_MSP(*(uint32_t *)address);

    /* Load the Reset Handler address of the application */
    app_start_address = *(uint32_t *)(address + 4);

    /* Jump to application Reset Handler in the application */
    asm("bx %0" ::"r"(app_start_address));
}

uint32_t current_number;
uint32_t i, length;
uint8_t command, *ptr_data, *ptr, data[SIZEBUFMAX];
uint8_t j;
uint32_t u32tmp;

static void wait_ready(void) {
    while (NVMCTRL->INTFLAG.bit.READY == 0) {
    }
}

void flash_erase_row(uint32_t *dst) {
    // Execute "ER" Erase Row
    NVMCTRL->ADDR.reg = (uint32_t)dst / 2;
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
    wait_ready();
}

void flash_write_words(uint32_t *dst, uint32_t *src, uint32_t n_words) {
    // Set automatic page write
    NVMCTRL->CTRLB.bit.MANW = 0;

    while (n_words > 0) {
        // Execute "PBC" Page Buffer Clear
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
        wait_ready();

        uint32_t len = min(FLASH_PAGE_SIZE >> 2, n_words);
        n_words -= len;

        while (len--)
            *dst++ = *src++;

        // Execute "WP" Write Page
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
        wait_ready();
    }
}

void flash_write_row(uint32_t *dst, uint32_t *src) {
    for (int i = 0; i < FLASH_ROW_SIZE / 4; ++i)
        if (src[i] != dst[i])
            goto doflash;
    return;
    
doflash:
    flash_erase_row(dst);
    flash_write_words(dst, src, FLASH_ROW_SIZE);
}

// Prints a 32-bit integer in hex.
void put_uint32(uint32_t n) {
    char buff[8];
    int i;
    for (i = 0; i < 8; i++) {
        int d = n & 0XF;
        n = (n >> 4);

        buff[7 - i] = d > 9 ? 'A' + d - 10 : '0' + d;
    }
    cdc_write_buf(buff, 8);
}

/**
 * \brief This function starts the SAM-BA monitor.
 */
void sam_ba_monitor_run(void) {
    ptr_data = NULL;
    command = 'z';

    // Start waiting some cmd
    while (1) {
        process_msc();
        length = cdc_read_buf(data, SIZEBUFMAX);
        ptr = data;
        for (i = 0; i < length; i++) {
            if (*ptr != 0xff) {
                if (*ptr == '#') {
                    if (b_terminal_mode) {
                        cdc_write_buf("\n\r", 2);
                    }
                    if (command == 'S') {
                        // Check if some data are remaining in the "data" buffer
                        if (length > i) {
                            // Move current indexes to next avail data
                            // (currently ptr points to "#")
                            ptr++;
                            i++;
                            // We need to add first the remaining data of the
                            // current buffer already read from usb
                            // read a maximum of "current_number" bytes
                            u32tmp = min((length - i), current_number);
                            memcpy(ptr_data, ptr, u32tmp);
                            i += u32tmp;
                            ptr += u32tmp;
                            j = u32tmp;
                        }
                        // update i with the data read from the buffer
                        i--;
                        ptr--;
                        // Do we expect more data ?
                        if (j < current_number)
                            cdc_read_buf_xmd(ptr_data, current_number - j);

                        __asm("nop");
                    } else if (command == 'R') {
                        cdc_write_buf_xmd(ptr_data, current_number);
                    } else if (command == 'O') {
                        *ptr_data = (char)current_number;
                    } else if (command == 'H') {
                        *((uint16_t *)(void *)ptr_data) = (uint16_t)current_number;
                    } else if (command == 'W') {
                        *((int *)(void *)ptr_data) = current_number;
                    } else if (command == 'o') {
                        sam_ba_putdata_term(ptr_data, 1);
                    } else if (command == 'h') {
                        current_number = *((uint16_t *)(void *)ptr_data);
                        sam_ba_putdata_term((uint8_t *)&current_number, 2);
                    } else if (command == 'w') {
                        current_number = *((uint32_t *)(void *)ptr_data);
                        sam_ba_putdata_term((uint8_t *)&current_number, 4);
                    } else if (command == 'G') {
                        call_applet(current_number);
                        /* Rebase the Stack Pointer */
                        __set_MSP(sp);
                        cpu_irq_enable();
                        if (b_sam_ba_interface_usart) {
                            cdc_write_buf("\x06", 1);
                        }
                    } else if (command == 'T') {
                        b_terminal_mode = 1;
                        cdc_write_buf("\n\r", 2);
                    } else if (command == 'N') {
                        if (b_terminal_mode == 0) {
                            cdc_write_buf("\n\r", 2);
                        }
                        b_terminal_mode = 0;
                    } else if (command == 'V') {
                        cdc_write_buf("v", 1);
                        cdc_write_buf((uint8_t *)RomBOOT_Version, strlen(RomBOOT_Version));
                        cdc_write_buf(" ", 1);
                        cdc_write_buf((uint8_t *)RomBOOT_ExtendedCapabilities,
                                      strlen(RomBOOT_ExtendedCapabilities));
                        cdc_write_buf(" ", 1);
                        ptr = (uint8_t *)&(__DATE__);
                        cdc_write_buf(ptr, strlen((char *)ptr));
                        cdc_write_buf(" ", 1);
                        ptr = (uint8_t *)&(__TIME__);
                        cdc_write_buf(ptr, strlen((char *)ptr));
                        cdc_write_buf("\n\r", 2);
                    } else if (command == 'X') {
                        // Syntax: X[ADDR]#
                        // Erase the flash memory starting from ADDR to the end
                        // of flash.

                        // Note: the flash memory is erased in ROWS, that is in
                        // block of 4 pages.
                        //       Even if the starting address is the last byte
                        //       of a ROW the entire
                        //       ROW is erased anyway.

                        uint32_t dst_addr = current_number; // starting address

                        while (dst_addr < FLASH_SIZE) {
                            flash_erase_row((void *)dst_addr);
                            dst_addr += FLASH_ROW_SIZE;
                        }

                        // Notify command completed
                        cdc_write_buf("X\n\r", 3);
                    } else if (command == 'Y') {
                        // This command writes the content of a buffer in SRAM
                        // into flash memory.

                        // Syntax: Y[ADDR],0#
                        // Set the starting address of the SRAM buffer.

                        // Syntax: Y[ROM_ADDR],[SIZE]#
                        // Write the first SIZE bytes from the SRAM buffer
                        // (previously set) into
                        // flash memory starting from address ROM_ADDR

                        static uint32_t *src_buff_addr = NULL;

                        if (current_number == 0) {
                            // Set buffer address
                            src_buff_addr = (void *)ptr_data;

                        } else {
                            flash_write_words((void *)ptr_data, src_buff_addr, current_number / 4);
                        }

                        // Notify command completed
                        cdc_write_buf("Y\n\r", 3);
                    } else if (command == 'Z') {
                        // This command calculate CRC for a given area of
                        // memory.
                        // It's useful to quickly check if a transfer has been
                        // done
                        // successfully.

                        // Syntax: Z[START_ADDR],[SIZE]#
                        // Returns: Z[CRC]#

                        uint8_t *data = (uint8_t *)ptr_data;
                        uint32_t size = current_number;
                        uint16_t crc = 0;
                        uint32_t i = 0;
                        for (i = 0; i < size; i++)
                            crc = add_crc(*data++, crc);

                        // Send response
                        cdc_write_buf("Z", 1);
                        put_uint32(crc);
                        cdc_write_buf("#\n\r", 3);
                    }

                    command = 'z';
                    current_number = 0;

                    if (b_terminal_mode) {
                        cdc_write_buf(">", 1);
                    }
                } else {
                    if (('0' <= *ptr) && (*ptr <= '9')) {
                        current_number = (current_number << 4) | (*ptr - '0');

                    } else if (('A' <= *ptr) && (*ptr <= 'F')) {
                        current_number = (current_number << 4) | (*ptr - 'A' + 0xa);

                    } else if (('a' <= *ptr) && (*ptr <= 'f')) {
                        current_number = (current_number << 4) | (*ptr - 'a' + 0xa);

                    } else if (*ptr == ',') {
                        ptr_data = (uint8_t *)current_number;
                        current_number = 0;

                    } else {
                        command = *ptr;
                        current_number = 0;
                    }
                }
                ptr++;
            }
        }
    }
}
