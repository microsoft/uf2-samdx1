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

#ifndef CDC_ENUMERATE_H
#define CDC_ENUMERATE_H

#define PKT_SIZE 64

#define USB_EP_IN 1
#define USB_EP_OUT 2
#define USB_EP_COMM 3

#define USB_EP_MSC_IN 4
#define USB_EP_MSC_OUT 5

#define USB_EP_HID 6
#define USB_EP_WEB 7

#define MAX_EP 8

#define NVM_USB_PAD_TRANSN_POS 45
#define NVM_USB_PAD_TRANSN_SIZE 5
#define NVM_USB_PAD_TRANSP_POS 50
#define NVM_USB_PAD_TRANSP_SIZE 5
#define NVM_USB_PAD_TRIM_POS 55
#define NVM_USB_PAD_TRIM_SIZE 3

typedef struct _USB_CDC {
    // Private members
    uint8_t currentConfiguration;
    uint8_t currentConnection;
} USB_CDC, *P_USB_CDC;

typedef struct {
    uint32_t dwDTERate;
    uint8_t bCharFormat;
    uint8_t bParityType;
    uint8_t bDataBits;
} usb_cdc_line_coding_t;

/**
 * \brief Initializes the USB module
 */
void usb_init(void);

/**
 * \brief Sends a single byte through USB CDC
 *
 * \param Data to send
 * \return number of data sent
 */
int cdc_putc(int value);

/**
 * \brief Reads a single byte through USB CDC
 *
 * \return Data read through USB
 */
int cdc_getc(void);

/**
 * \brief Checks if a character has been received on USB CDC
 *
 * \return \c 1 if a byte is ready to be read.
 */
bool cdc_is_rx_ready(void);

/**
 * \brief Sends buffer on USB CDC
 *
 * \param data pointer
 * \param number of data to send
 * \return number of data sent
 */
uint32_t cdc_write_buf(void const *data, uint32_t length);
uint32_t cdc_write_buf_xmd(void const *data, uint32_t length);

/**
 * \brief Gets data on USB CDC
 *
 * \param data pointer
 * \param number of data to read
 * \return number of data read
 */
uint32_t cdc_read_buf(void *data, uint32_t length);

/**
 * \brief Gets specified number of bytes on USB CDC
 *
 * \param data pointer
 * \param number of data to read
 * \return number of data read
 */
uint32_t cdc_read_buf_xmd(void *data, uint32_t length);

void reset_ep(uint8_t ep);
void stall_ep(uint8_t ep);

typedef struct {
    uint8_t size;
    uint8_t ptr;
    uint8_t read_job;
    uint8_t padding;
    uint8_t buf[PKT_SIZE];
} PacketBuffer __attribute__((aligned(4)));

uint32_t USB_Read(void *pData, uint32_t length, uint32_t ep);
uint32_t USB_Write(const void *pData, uint32_t length, uint8_t ep_num);
uint32_t USB_WriteCore(const void *pData, uint32_t length, uint8_t ep_num, bool handoverMode);
void USB_ReadBlocking(void *dst, uint32_t length, uint32_t ep, PacketBuffer *cache);
uint32_t USB_ReadCore(void *pData, uint32_t length, uint32_t ep, PacketBuffer *cache);
bool USB_Ok(void);

#endif // CDC_ENUMERATE_H
