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

#include "uf2.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

typedef struct {
    uint8_t size;
    uint8_t ptr;
    uint8_t read_job;
    uint8_t padding;
    uint8_t buf[PKT_SIZE];
} PacketBuffer __attribute__((aligned(4)));

PacketBuffer ctrlOutCache;
PacketBuffer endpointCache[MAX_EP];

COMPILER_WORD_ALIGNED UsbDeviceDescriptor usb_endpoint_table[MAX_EP] = {0};

COMPILER_WORD_ALIGNED
const char devDescriptor[] = {
    /* Device descriptor */
    0x12,           // bLength
    0x01,           // bDescriptorType
    0x10,           // bcdUSBL
    0x01,           //
    0x00,           // bDeviceClass:    CDC class code
    0x00,           // bDeviceSubclass: CDC class sub code
    0x00,           // bDeviceProtocol: CDC Device protocol
    0x40,           // bMaxPacketSize0
    USB_VID & 0xff, // vendor ID
    USB_VID >> 8,   //
    USB_PID & 0xff, // product ID
    USB_PID >> 8,   //
    0x10,           // bcdDeviceL
    0x01,           //
    0x01,           // iManufacturer    // 0x01
    0x02,           // iProduct
    0x00,           // SerialNumber
    0x01            // bNumConfigs
};

#define CFG_DESC_SIZE (USE_CDC ? 0x5A : 0x20)

COMPILER_WORD_ALIGNED
char cfgDescriptor[] = {
    /* ============== CONFIGURATION 1 =========== */
    /* Configuration 1 descriptor */
    0x09,          // CbLength
    0x02,          // CbDescriptorType
    CFG_DESC_SIZE, // CwTotalLength 2 EP + Control
    0x00,
    USE_CDC ? 0x03 : 0x01, // CbNumInterfaces
    0x01,                  // CbConfigurationValue
    0x00,                  // CiConfiguration
    0xC0,                  // CbmAttributes 0xA0
    0x00,                  // CMaxPower

#if USE_CDC
    /* Communication Class Interface Descriptor Requirement */
    0x09, // bLength
    0x04, // bDescriptorType
    0x00, // bInterfaceNumber
    0x00, // bAlternateSetting
    0x01, // bNumEndpoints
    0x02, // bInterfaceClass
    0x02, // bInterfaceSubclass
    0x00, // bInterfaceProtocol
    0x00, // iInterface

    /* Header Functional Descriptor */
    0x05, // bFunction Length
    0x24, // bDescriptor type: CS_INTERFACE
    0x00, // bDescriptor subtype: Header Func Desc
    0x10, // bcdCDC:1.1
    0x01,

    /* ACM Functional Descriptor */
    0x04, // bFunctionLength
    0x24, // bDescriptor Type: CS_INTERFACE
    0x02, // bDescriptor Subtype: ACM Func Desc
    0x00, // bmCapabilities

    /* Union Functional Descriptor */
    0x05, // bFunctionLength
    0x24, // bDescriptorType: CS_INTERFACE
    0x06, // bDescriptor Subtype: Union Func Desc
    0x00, // bMasterInterface: Communication Class Interface
    0x01, // bSlaveInterface0: Data Class Interface

    /* Call Management Functional Descriptor */
    0x05, // bFunctionLength
    0x24, // bDescriptor Type: CS_INTERFACE
    0x01, // bDescriptor Subtype: Call Management Func Desc
    0x00, // bmCapabilities: D1 + D0
    0x01, // bDataInterface: Data Class Interface 1

    /* Endpoint 1 descriptor */
    0x07, // bLength
    0x05, // bDescriptorType
    0x83, // bEndpointAddress, Endpoint 03 - IN
    0x03, // bmAttributes      INT
    0x08, // wMaxPacketSize
    0x00,
    0xFF, // bInterval

    /* Data Class Interface Descriptor Requirement */
    0x09, // bLength
    0x04, // bDescriptorType
    0x01, // bInterfaceNumber
    0x00, // bAlternateSetting
    0x02, // bNumEndpoints
    0x0A, // bInterfaceClass
    0x00, // bInterfaceSubclass
    0x00, // bInterfaceProtocol
    0x00, // iInterface

    /* First alternate setting */
    /* Endpoint 1 descriptor */
    0x07,     // bLength
    0x05,     // bDescriptorType
    0x81,     // bEndpointAddress, Endpoint 01 - IN
    0x02,     // bmAttributes      BULK
    PKT_SIZE, // wMaxPacketSize
    0x00,
    0x00, // bInterval

    /* Endpoint 2 descriptor */
    0x07,     // bLength
    0x05,     // bDescriptorType
    0x02,     // bEndpointAddress, Endpoint 02 - OUT
    0x02,     // bmAttributes      BULK
    PKT_SIZE, // wMaxPacketSize
    0x00,
    0x00, // bInterval
#endif

    // MSC

    9,               /// descriptor size in bytes
    4,               /// descriptor type - interface
    USE_CDC ? 2 : 0, /// interface number
    0,               /// alternate setting number
    2,               /// number of endpoints
    8,               /// class code - mass storage
    6,               /// subclass code - SCSI transparent command set
    80,              /// protocol code - bulk only transport
    0,               /// interface string index

    7,        /// descriptor size in bytes
    5,        /// descriptor type - endpoint
    0x84,     /// endpoint direction and number - in, 2
    2,        /// transfer type - bulk
    PKT_SIZE, /// maximum packet size
    0,
    0, /// not used

    7,        /// descriptor size in bytes
    5,        /// descriptor type - endpoint
    0x05,     /// endpoint direction and number - out, 1
    2,        /// transfer type - bulk
    PKT_SIZE, /// maximum packet size
    0,
    0, /// maximum NAK rate
};

typedef struct {
    uint8_t len;
    uint8_t type;
    uint8_t data[40];
} StringDescriptor;

#define STRING_DESCRIPTOR_COUNT 4

static const char *string_descriptors[STRING_DESCRIPTOR_COUNT] = {
    0, PRODUCT_NAME, VENDOR_NAME, serialNumber,
};

static usb_cdc_line_coding_t line_coding = {
    115200, // baudrate
    0,      // 1 Stop Bit
    0,      // None Parity
    8       // 8 Data bits
};

static USB_CDC pCdc;

/* USB standard request code */
#define STD_GET_STATUS_ZERO 0x0080
#define STD_GET_STATUS_INTERFACE 0x0081
#define STD_GET_STATUS_ENDPOINT 0x0082

#define STD_CLEAR_FEATURE_ZERO 0x0100
#define STD_CLEAR_FEATURE_INTERFACE 0x0101
#define STD_CLEAR_FEATURE_ENDPOINT 0x0102

#define STD_SET_FEATURE_ZERO 0x0300
#define STD_SET_FEATURE_INTERFACE 0x0301
#define STD_SET_FEATURE_ENDPOINT 0x0302

#define STD_SET_ADDRESS 0x0500
#define STD_GET_DESCRIPTOR 0x0680
#define STD_SET_DESCRIPTOR 0x0700
#define STD_GET_CONFIGURATION 0x0880
#define STD_SET_CONFIGURATION 0x0900
#define STD_GET_INTERFACE 0x0A81
#define STD_SET_INTERFACE 0x0B01
#define STD_SYNCH_FRAME 0x0C82

/* CDC Class Specific Request Code */
#define GET_LINE_CODING 0x21A1
#define SET_LINE_CODING 0x2021
#define SET_CONTROL_LINE_STATE 0x2221

// MSC
#define MSC_RESET 0xFF21
#define MSC_GET_MAX_LUN 0xFEA1

static void AT91F_CDC_Enumerate(void);

STATIC_ASSERT(sizeof(cfgDescriptor) == CFG_DESC_SIZE);

/**
 * \fn AT91F_InitUSB
 *
 * \brief Initializes USB
 */
void AT91F_InitUSB(void) {
    uint32_t pad_transn, pad_transp, pad_trim;

    /* Enable USB clock */
    PM->APBBMASK.reg |= PM_APBBMASK_USB;

    /* Set up the USB DP/DN pins */
    PORT->Group[0].PINCFG[PIN_PA24G_USB_DM].bit.PMUXEN = 1;
    PORT->Group[0].PMUX[PIN_PA24G_USB_DM / 2].reg &= ~(0xF << (4 * (PIN_PA24G_USB_DM & 0x01u)));
    PORT->Group[0].PMUX[PIN_PA24G_USB_DM / 2].reg |= MUX_PA24G_USB_DM
                                                     << (4 * (PIN_PA24G_USB_DM & 0x01u));
    PORT->Group[0].PINCFG[PIN_PA25G_USB_DP].bit.PMUXEN = 1;
    PORT->Group[0].PMUX[PIN_PA25G_USB_DP / 2].reg &= ~(0xF << (4 * (PIN_PA25G_USB_DP & 0x01u)));
    PORT->Group[0].PMUX[PIN_PA25G_USB_DP / 2].reg |= MUX_PA25G_USB_DP
                                                     << (4 * (PIN_PA25G_USB_DP & 0x01u));

    /* Setup clock for module */
    GCLK_CLKCTRL_Type clkctrl = {0};
    uint16_t temp;
    /* GCLK_ID - USB - 0x06 */
    GCLK->CLKCTRL.bit.ID = 0x06;
    temp = GCLK->CLKCTRL.reg;
    clkctrl.bit.CLKEN = true;
    clkctrl.bit.WRTLOCK = false;
    clkctrl.bit.GEN = GCLK_CLKCTRL_GEN_GCLK1_Val;
    GCLK->CLKCTRL.reg = (clkctrl.reg | temp);

    /* Reset */
    USB->HOST.CTRLA.bit.SWRST = 1;
    while (USB->HOST.SYNCBUSY.bit.SWRST) {
        /* Sync wait */
    }

    /* Load Pad Calibration */
    pad_transn = (*((uint32_t *)(NVMCTRL_OTP4) + (NVM_USB_PAD_TRANSN_POS / 32)) >>
                  (NVM_USB_PAD_TRANSN_POS % 32)) &
                 ((1 << NVM_USB_PAD_TRANSN_SIZE) - 1);

    if (pad_transn == 0x1F) {
        pad_transn = 5;
    }

    USB->HOST.PADCAL.bit.TRANSN = pad_transn;

    pad_transp = (*((uint32_t *)(NVMCTRL_OTP4) + (NVM_USB_PAD_TRANSP_POS / 32)) >>
                  (NVM_USB_PAD_TRANSP_POS % 32)) &
                 ((1 << NVM_USB_PAD_TRANSP_SIZE) - 1);

    if (pad_transp == 0x1F) {
        pad_transp = 29;
    }

    USB->HOST.PADCAL.bit.TRANSP = pad_transp;
    pad_trim = (*((uint32_t *)(NVMCTRL_OTP4) + (NVM_USB_PAD_TRIM_POS / 32)) >>
                (NVM_USB_PAD_TRIM_POS % 32)) &
               ((1 << NVM_USB_PAD_TRIM_SIZE) - 1);

    if (pad_trim == 0x7) {
        pad_trim = 3;
    }

    USB->HOST.PADCAL.bit.TRIM = pad_trim;

    /* Set the configuration */
    /* Set mode to Device mode */
    USB->HOST.CTRLA.bit.MODE = 0;
    /* Enable Run in Standby */
    USB->HOST.CTRLA.bit.RUNSTDBY = true;
    /* Set the descriptor address */
    USB->HOST.DESCADD.reg = (uint32_t)(&usb_endpoint_table[0]);
    /* Set speed configuration to Full speed */
    USB->DEVICE.CTRLB.bit.SPDCONF = USB_DEVICE_CTRLB_SPDCONF_FS_Val;
    /* Attach to the USB host */
    USB->DEVICE.CTRLB.reg &= ~USB_DEVICE_CTRLB_DETACH;

    /* Initialize endpoint table RAM location to a known value 0 */
    memset((uint8_t *)(&usb_endpoint_table[0]), 0, sizeof(usb_endpoint_table));
}

//*----------------------------------------------------------------------------
//* \fn    USB_IsConfigured
//* \brief Test if the device is configured and handle
// enumerationDEVICE.DeviceEndpoint[ep_num].EPCFG.bit.EPTYPE1
//*----------------------------------------------------------------------------
bool USB_Ok() {
    Usb *pUsb = pCdc.pUsb;

    timerTick();

    /* Check for End of Reset flag */
    if (pUsb->DEVICE.INTFLAG.reg & USB_DEVICE_INTFLAG_EORST) {
        /* Clear the flag */
        pUsb->DEVICE.INTFLAG.reg = USB_DEVICE_INTFLAG_EORST;
        /* Set Device address as 0 */
        pUsb->DEVICE.DADD.reg = USB_DEVICE_DADD_ADDEN | 0;
        /* Configure endpoint 0 */
        /* Configure Endpoint 0 for Control IN and Control OUT */
        pUsb->DEVICE.DeviceEndpoint[0].EPCFG.reg =
            USB_DEVICE_EPCFG_EPTYPE0(1) | USB_DEVICE_EPCFG_EPTYPE1(1);
        pUsb->DEVICE.DeviceEndpoint[0].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_BK0RDY;
        pUsb->DEVICE.DeviceEndpoint[0].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK1RDY;
        /* Configure control OUT Packet size to 64 bytes */
        usb_endpoint_table[0].DeviceDescBank[0].PCKSIZE.bit.SIZE = 3;
        /* Configure control IN Packet size to 64 bytes */
        usb_endpoint_table[0].DeviceDescBank[1].PCKSIZE.bit.SIZE = 3;
        /* Configure the data buffer address for control OUT */
        usb_endpoint_table[0].DeviceDescBank[0].ADDR.reg = (uint32_t)&ctrlOutCache.buf;
        /* Configure the data buffer address for control IN */
        usb_endpoint_table[0].DeviceDescBank[1].ADDR.reg = (uint32_t)&endpointCache[0].buf;
        /* Set Multipacket size to 8 for control OUT and byte count to 0*/
        usb_endpoint_table[0].DeviceDescBank[0].PCKSIZE.bit.MULTI_PACKET_SIZE = 8;
        usb_endpoint_table[0].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT = 0;
        pUsb->DEVICE.DeviceEndpoint[0].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK0RDY;

        // Reset current configuration value to 0
        pCdc.currentConfiguration = 0;
    } else if (pUsb->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg & USB_DEVICE_EPINTFLAG_RXSTP) {
        AT91F_CDC_Enumerate();
    }

    return pCdc.currentConfiguration != 0;
}

//*----------------------------------------------------------------------------
//* \fn    USB_Read
//* \brief Read available data from Endpoint OUT
//*----------------------------------------------------------------------------
uint32_t USB_Read(void *pData, uint32_t length, uint32_t ep) {
    Usb *pUsb = pCdc.pUsb;
    uint32_t packetSize = 0;
    PacketBuffer *cache = &endpointCache[ep];

    timerTick();

    if (cache->ptr < cache->size) {
        packetSize = MIN(cache->size - cache->ptr, length);
        if (pData) {
            memcpy(pData, cache->buf + cache->ptr, packetSize);
            cache->ptr += packetSize;
        }
        return packetSize;
    }

    if (!cache->read_job) {
        /* Set the buffer address for ep data */
        usb_endpoint_table[ep].DeviceDescBank[0].ADDR.reg = (uint32_t)&cache->buf;
        /* Set the byte count as zero */
        usb_endpoint_table[ep].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT = 0;
        /* Set the byte count as zero */
        usb_endpoint_table[ep].DeviceDescBank[0].PCKSIZE.bit.MULTI_PACKET_SIZE = 0;
        /* Start the reception by clearing the bank 0 ready bit */
        pUsb->DEVICE.DeviceEndpoint[ep].EPSTATUSCLR.bit.BK0RDY = true;
        /* set the user flag */
        cache->read_job = true;
    }

    /* Check for Transfer Complete 0 flag */
    if (pUsb->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg & USB_DEVICE_EPINTFLAG_TRCPT0) {
        /* Set packet size */
        cache->size = usb_endpoint_table[ep].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT;
        packetSize = MIN(cache->size, length);
        if (pData) {
            cache->ptr = packetSize;
            /* Copy read data to user buffer */
            memcpy(pData, cache->buf, packetSize);
        } else {
            cache->ptr = 0;
        }
        /* Clear the Transfer Complete 0 flag */
        pUsb->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT0;
        /* Clear the user flag */
        cache->read_job = false;
    }

    //if (packetSize)
    //    logval("read", packetSize);

    return packetSize;
}

void USB_ReadBlocking(void *dst, uint32_t length, uint32_t ep) {
    /* Blocking read till specified number of bytes is received */
    while (length) {
        uint32_t curr = USB_Read(dst, length, ep);
        //if (curr > 0)
        //    logval("readbl", length);
        length -= curr;
        dst = (char *)dst + curr;
    }
}

uint32_t USB_Write(const void *pData, uint32_t length, uint8_t ep_num) {
    Usb *pUsb = pCdc.pUsb;
    uint32_t data_address;

    /* Check for requirement for multi-packet or auto zlp */
    if (length >= (1 << (usb_endpoint_table[ep_num].DeviceDescBank[1].PCKSIZE.bit.SIZE + 3))) {
        /* Update the EP data address */
        data_address = (uint32_t)pData;
        // data must be in RAM!
        assert(data_address >= HMCRAMC0_ADDR);

        // always disable AUTO_ZLP on MSC channel, otherwise enable
        usb_endpoint_table[ep_num].DeviceDescBank[1].PCKSIZE.bit.AUTO_ZLP =
            ep_num == USB_EP_MSC_IN ? false : true;
    } else {
        /* Copy to local buffer */
        memcpy(endpointCache[ep_num].buf, pData, length);
        /* Update the EP data address */
        data_address = (uint32_t)&endpointCache[ep_num].buf;
    }

    /* Set the buffer address for ep data */
    usb_endpoint_table[ep_num].DeviceDescBank[1].ADDR.reg = data_address;
    /* Set the byte count as zero */
    usb_endpoint_table[ep_num].DeviceDescBank[1].PCKSIZE.bit.BYTE_COUNT = length;
    /* Set the multi packet size as zero for multi-packet transfers where length
     * > ep size */
    usb_endpoint_table[ep_num].DeviceDescBank[1].PCKSIZE.bit.MULTI_PACKET_SIZE = 0;
    /* Clear the transfer complete flag  */
    pUsb->DEVICE.DeviceEndpoint[ep_num].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT1;
    /* Set the bank as ready */
    pUsb->DEVICE.DeviceEndpoint[ep_num].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_BK1RDY;

    /* Wait for transfer to complete */
    while (!(pUsb->DEVICE.DeviceEndpoint[ep_num].EPINTFLAG.reg & USB_DEVICE_EPINTFLAG_TRCPT1)) {
        if (ep_num && !USB_Ok())
            return -1;
    }

    return length;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_USB_SendData
//* \brief Send Data through the control endpoint
//*----------------------------------------------------------------------------

static void AT91F_USB_SendData(const char *pData, uint32_t length) { USB_Write(pData, length, 0); }

//*----------------------------------------------------------------------------
//* \fn    AT91F_USB_SendZlp
//* \brief Send zero length packet through the control endpoint
//*----------------------------------------------------------------------------
void AT91F_USB_SendZlp(void) {
    char c = 0;
    AT91F_USB_SendData(&c, 0);
}

static void configureInOut(Usb *pUsb, uint8_t in_ep) {
    /* Configure BULK OUT endpoint for CDC Data interface*/
    pUsb->DEVICE.DeviceEndpoint[in_ep + 1].EPCFG.reg = USB_DEVICE_EPCFG_EPTYPE0(3);
    /* Set maximum packet size as 64 bytes */
    usb_endpoint_table[in_ep + 1].DeviceDescBank[0].PCKSIZE.bit.SIZE = 3;
    pUsb->DEVICE.DeviceEndpoint[in_ep + 1].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_BK0RDY;
    /* Configure the data buffer */
    usb_endpoint_table[in_ep + 1].DeviceDescBank[0].ADDR.reg =
        (uint32_t)&endpointCache[in_ep + 1].buf;

    /* Configure BULK IN endpoint for CDC Data interface */
    pUsb->DEVICE.DeviceEndpoint[in_ep].EPCFG.reg = USB_DEVICE_EPCFG_EPTYPE1(3);
    /* Set maximum packet size as 64 bytes */
    usb_endpoint_table[in_ep].DeviceDescBank[1].PCKSIZE.bit.SIZE = 3;
    pUsb->DEVICE.DeviceEndpoint[in_ep].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK1RDY;
    /* Configure the data buffer */
    usb_endpoint_table[in_ep].DeviceDescBank[1].ADDR.reg = (uint32_t)&endpointCache[in_ep].buf;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_CDC_Enumerate
//* \brief This function is a callback invoked when a SETUP packet is received
//*----------------------------------------------------------------------------
void AT91F_CDC_Enumerate() {
    Usb *pUsb = pCdc.pUsb;
    static volatile uint8_t bmRequestType, bRequest, dir;
    static volatile uint16_t wValue, wIndex, wLength, wStatus;

    /* Clear the Received Setup flag */
    pUsb->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_RXSTP;

    /* Read the USB request parameters */
    bmRequestType = ctrlOutCache.buf[0];
    bRequest = ctrlOutCache.buf[1];
    wValue = (ctrlOutCache.buf[2] & 0xFF);
    wValue |= (ctrlOutCache.buf[3] << 8);
    wIndex = (ctrlOutCache.buf[4] & 0xFF);
    wIndex |= (ctrlOutCache.buf[5] << 8);
    wLength = (ctrlOutCache.buf[6] & 0xFF);
    wLength |= (ctrlOutCache.buf[7] << 8);

    /* Clear the Bank 0 ready flag on Control OUT */
    pUsb->DEVICE.DeviceEndpoint[0].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK0RDY;

    uint32_t reqId = (bRequest << 8) | bmRequestType;

    logwrite("USBReq: ");
    logwritenum(reqId);
    logwrite(" wValue: ");
    logwritenum(wValue);
    logval(" wLen", wLength);

    /* Handle supported standard device request Cf Table 9-3 in USB
     * specification Rev 1.1 */
    switch (reqId) {
    case STD_GET_DESCRIPTOR:
        if (wValue == 0x100)
            /* Return Device Descriptor */
            AT91F_USB_SendData(devDescriptor, MIN(sizeof(devDescriptor), wLength));
        else if (wValue == 0x200)
            /* Return Configuration Descriptor */
            AT91F_USB_SendData(cfgDescriptor, MIN(sizeof(cfgDescriptor), wLength));
        else if (ctrlOutCache.buf[3] == 3) {
            if (ctrlOutCache.buf[2] >= STRING_DESCRIPTOR_COUNT)
                stall_ep(0);
            StringDescriptor desc = {0};
            desc.type = 3;
            if (ctrlOutCache.buf[2] == 0) {
                desc.len = 4;
                desc.data[0] = 0x09;
                desc.data[1] = 0x04;
            } else {
                const char *ptr = string_descriptors[ctrlOutCache.buf[2]];
                desc.len = strlen(ptr) * 2 + 2;
                for (int i = 0; ptr[i]; i++) {
                    desc.data[i * 2] = ptr[i];
                }
            }
            AT91F_USB_SendData((void *)&desc, MIN(sizeof(StringDescriptor), wLength));
        } else
            /* Stall the request */
            stall_ep(0);
        break;
    case STD_SET_ADDRESS:
        /* Send ZLP */
        AT91F_USB_SendZlp();
        /* Set device address to the newly received address from host */
        pUsb->DEVICE.DADD.reg = USB_DEVICE_DADD_ADDEN | wValue;
        break;
    case STD_SET_CONFIGURATION:
        /* Store configuration */
        pCdc.currentConfiguration = (uint8_t)wValue;
        /* Send ZLP */
        AT91F_USB_SendZlp();

        configureInOut(pUsb, USB_EP_IN);

        /* Configure INTERRUPT IN endpoint for CDC COMM interface*/
        pUsb->DEVICE.DeviceEndpoint[USB_EP_COMM].EPCFG.reg = USB_DEVICE_EPCFG_EPTYPE1(4);
        /* Set maximum packet size as 64 bytes */
        usb_endpoint_table[USB_EP_COMM].DeviceDescBank[1].PCKSIZE.bit.SIZE = 0;
        pUsb->DEVICE.DeviceEndpoint[USB_EP_COMM].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK1RDY;

        configureInOut(pUsb, USB_EP_MSC_IN);

        break;
    case STD_GET_CONFIGURATION:
        /* Return current configuration value */
        AT91F_USB_SendData((char *)&(pCdc.currentConfiguration), sizeof(pCdc.currentConfiguration));
        break;
    case STD_GET_STATUS_ZERO:
        wStatus = 0;
        AT91F_USB_SendData((char *)&wStatus, sizeof(wStatus));
        break;
    case STD_GET_STATUS_INTERFACE:
        wStatus = 0;
        AT91F_USB_SendData((char *)&wStatus, sizeof(wStatus));
        break;
    case STD_GET_STATUS_ENDPOINT:
        wStatus = 0;
        dir = wIndex & 80;
        wIndex &= 0x0F;
        if (wIndex < MAX_EP) {
            if (dir) {
                wStatus = (pUsb->DEVICE.DeviceEndpoint[wIndex].EPSTATUS.reg &
                           USB_DEVICE_EPSTATUSSET_STALLRQ1)
                              ? 1
                              : 0;
            } else {
                wStatus = (pUsb->DEVICE.DeviceEndpoint[wIndex].EPSTATUS.reg &
                           USB_DEVICE_EPSTATUSSET_STALLRQ0)
                              ? 1
                              : 0;
            }
            /* Return current status of endpoint */
            AT91F_USB_SendData((char *)&wStatus, sizeof(wStatus));
        } else
            /* Stall the request */
            stall_ep(0);
        break;
    case STD_SET_FEATURE_ZERO:
        /* Stall the request */
        stall_ep(0);
        break;
    case STD_SET_FEATURE_INTERFACE:
        /* Send ZLP */
        AT91F_USB_SendZlp();
        break;
    case STD_SET_FEATURE_ENDPOINT:
        dir = wIndex & 0x80;
        wIndex &= 0x0F;
        if ((wValue == 0) && wIndex && (wIndex < MAX_EP)) {
            /* Set STALL request for the endpoint */
            if (dir) {
                pUsb->DEVICE.DeviceEndpoint[wIndex].EPSTATUSSET.reg =
                    USB_DEVICE_EPSTATUSSET_STALLRQ1;
            } else {
                pUsb->DEVICE.DeviceEndpoint[wIndex].EPSTATUSSET.reg =
                    USB_DEVICE_EPSTATUSSET_STALLRQ0;
            }
            /* Send ZLP */
            AT91F_USB_SendZlp();
        } else
            /* Stall the request */
            stall_ep(0);
        break;
    case STD_CLEAR_FEATURE_ZERO:
        /* Stall the request */
        stall_ep(0);
        break;
    case STD_CLEAR_FEATURE_INTERFACE:
        /* Send ZLP */
        AT91F_USB_SendZlp();
        break;
    case STD_CLEAR_FEATURE_ENDPOINT:
        dir = wIndex & 0x80;
        wIndex &= 0x0F;
        if ((wValue == 0) && wIndex && (wIndex < MAX_EP)) {
            if (dir) {
                if (pUsb->DEVICE.DeviceEndpoint[wIndex].EPSTATUS.reg &
                    USB_DEVICE_EPSTATUSSET_STALLRQ1) {
                    // Remove stall request
                    pUsb->DEVICE.DeviceEndpoint[wIndex].EPSTATUSCLR.reg =
                        USB_DEVICE_EPSTATUSCLR_STALLRQ1;
                    if (pUsb->DEVICE.DeviceEndpoint[wIndex].EPINTFLAG.reg &
                        USB_DEVICE_EPINTFLAG_STALL1) {
                        pUsb->DEVICE.DeviceEndpoint[wIndex].EPINTFLAG.reg =
                            USB_DEVICE_EPINTFLAG_STALL1;
                        // The Stall has occurred, then reset data toggle
                        pUsb->DEVICE.DeviceEndpoint[wIndex].EPSTATUSCLR.reg =
                            USB_DEVICE_EPSTATUSSET_DTGLIN;
                    }
                }
            } else {
                if (pUsb->DEVICE.DeviceEndpoint[wIndex].EPSTATUS.reg &
                    USB_DEVICE_EPSTATUSSET_STALLRQ0) {
                    // Remove stall request
                    pUsb->DEVICE.DeviceEndpoint[wIndex].EPSTATUSCLR.reg =
                        USB_DEVICE_EPSTATUSCLR_STALLRQ0;
                    if (pUsb->DEVICE.DeviceEndpoint[wIndex].EPINTFLAG.reg &
                        USB_DEVICE_EPINTFLAG_STALL0) {
                        pUsb->DEVICE.DeviceEndpoint[wIndex].EPINTFLAG.reg =
                            USB_DEVICE_EPINTFLAG_STALL0;
                        // The Stall has occurred, then reset data toggle
                        pUsb->DEVICE.DeviceEndpoint[wIndex].EPSTATUSCLR.reg =
                            USB_DEVICE_EPSTATUSSET_DTGLOUT;
                    }
                }
            }
            /* Send ZLP */
            AT91F_USB_SendZlp();
        } else {
            stall_ep(0);
        }
        break;

    // handle CDC class requests
    case SET_LINE_CODING:
        /* Send ZLP */
        AT91F_USB_SendZlp();
        break;
    case GET_LINE_CODING:
        /* Send current line coding */
        AT91F_USB_SendData((char *)&line_coding, MIN(sizeof(usb_cdc_line_coding_t), wLength));
        break;
    case SET_CONTROL_LINE_STATE:
        /* Store the current connection */
        pCdc.currentConnection = wValue;
        /* Send ZLP */
        AT91F_USB_SendZlp();
        break;

    // MSC
    case MSC_RESET:
        logmsg("MSC reset");
        msc_reset();
        break;

    case MSC_GET_MAX_LUN:
        logmsg("MSC maxlun");
        wStatus = MAX_LUN;
        AT91F_USB_SendData((char *)&wStatus, 1);
        break;

    default:
        logval("Invalid CTRL command", reqId);
        /* Stall the request */
        stall_ep(0);
        break;
    }
}

static bool isInEP(uint8_t ep) { return ep == USB_EP_IN || ep == USB_EP_MSC_IN; }

void reset_ep(uint8_t ep) {
    assert(ep != 0);

    // Stop transfer
    if (isInEP(ep)) {
        pCdc.pUsb->DEVICE.DeviceEndpoint[ep].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK1RDY;
        // Eventually ack a transfer occur during abort
        pCdc.pUsb->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT1;
    } else {
        pCdc.pUsb->DEVICE.DeviceEndpoint[ep].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_BK0RDY;
        // Eventually ack a transfer occur during abort
        pCdc.pUsb->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT0;
    }
}

void stall_ep(uint8_t ep) {
    logval("Stall EP", ep);
    /* Check the direction */
    if (ep == 0 || isInEP(ep)) {
        /* Set STALL request on IN direction */
        pCdc.pUsb->DEVICE.DeviceEndpoint[ep].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_STALLRQ1;
    } else {
        /* Set STALL request on OUT direction */
        pCdc.pUsb->DEVICE.DeviceEndpoint[ep].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_STALLRQ0;
    }
}

void usb_init(void) {
    pCdc.pUsb = USB;
    /* Initialize USB */
    AT91F_InitUSB();
    pCdc.currentConfiguration = 0;
    pCdc.currentConnection = 0;
    pCdc.pUsb->HOST.CTRLA.bit.ENABLE = true;
}

#include "usart_sam_ba.h"

#if USE_UART
#define UART(e)                                                                                    \
    if (b_sam_ba_interface_usart)                                                                  \
        return e;
#else
#define UART(e)
#endif

bool cdc_is_rx_ready(void) {
    UART(usart_is_rx_ready())

    /* Check whether the device is configured */
    if (!USB_Ok())
        return 0;

    /* Return transfer complete 0 flag status */
    return (pCdc.pUsb->DEVICE.DeviceEndpoint[USB_EP_OUT].EPINTFLAG.reg &
            USB_DEVICE_EPINTFLAG_TRCPT0);
}

uint32_t cdc_write_buf(void const *data, uint32_t length) {
    UART(usart_putdata(data, length))
    /* Send the specified number of bytes on USB CDC */
    USB_Write((const char *)data, length, USB_EP_IN);
    return length;
}

uint32_t cdc_write_buf_xmd(void const *data, uint32_t length) {
    UART(usart_putdata_xmd(data, length))
    /* Send the specified number of bytes on USB CDC */
    USB_Write((const char *)data, length, USB_EP_IN);
    return length;
}

uint32_t cdc_read_buf(void *data, uint32_t length) {
    UART(usart_getdata(data, length))

    /* Check whether the device is configured */
    if (!USB_Ok())
        return 0;

    /* Read from USB CDC */
    return USB_Read((char *)data, length, USB_EP_OUT);
}

uint32_t cdc_read_buf_xmd(void *data, uint32_t length) {
    UART(usart_getdata_xmd(data, length))
    /* Check whether the device is configured */
    if (!USB_Ok())
        return 0;

    /* Blocking read till specified number of bytes is received */
    USB_ReadBlocking((char *)data, length, USB_EP_OUT);

    return length;
}
