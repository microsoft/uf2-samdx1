/**
 * \file
 *
 * \brief USB Device Mass Storage Class (MSC) interface.
 *
 * Copyright (c) 2009-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */


#define DEFINE_CONFIG_DATA 1

#include "uf2.h"

#include "lib/usb_msc/sbc_protocol.h"
#include "lib/usb_msc/spc_protocol.h"
#include "lib/usb_msc/usb_protocol.h"
#include "lib/usb_msc/usb_protocol_msc.h"

#if !USE_DBG_MSC
#undef logmsg
#undef logval
#define logmsg(...) NOOP
#define logval(...) NOOP
#endif

// From sam0/utils/compiler.h in ASF3
#define  le32_to_cpu(x) (x)
#define  cpu_to_le32(x) (x)
#define  CPU_TO_BE32(x) ((uint32_t)__builtin_bswap32((uint32_t)(x)))
#define  cpu_to_be16(x) ((uint16_t)(((uint16_t)(x) >> 8) |\
                           ((uint16_t)(x) << 8)))

#define MSB3(u32)   (((uint8_t  *)&(u32))[0]) //!< Most significant byte of 4th rank of \a u32.
#define MSB2(u32)   (((uint8_t  *)&(u32))[1]) //!< Most significant byte of 3rd rank of \a u32.
#define MSB1(u32)   (((uint8_t  *)&(u32))[2]) //!< Most significant byte of 2nd rank of \a u32.
#define MSB0(u32)   (((uint8_t  *)&(u32))[3]) //!< Most significant byte of 1st rank of \a u32.

#define  MSB(u16)   (((uint8_t  *)&(u16))[1]) //!< Most significant byte of \a u16.
#define  LSB(u16)   (((uint8_t  *)&(u16))[0]) //!< Least significant byte of \a u16.

bool mscReset = false;

void msc_reset(void) {
    mscReset = true;
    reset_ep(USB_EP_MSC_IN);
    reset_ep(USB_EP_MSC_OUT);
}

//! Structure to receive a CBW packet
static struct usb_msc_cbw udi_msc_cbw;
//! Structure to send a CSW packet
static struct usb_msc_csw udi_msc_csw = {.dCSWSignature = cpu_to_le32(USB_CSW_SIGNATURE)};
//! Structure with current SCSI sense data
static struct scsi_request_sense_data udi_msc_sense;

#if USE_MSC_CHECKS
/**
 * \brief Stall CBW request
 */
static void udi_msc_cbw_invalid(void);

/**
 * \brief Stall CSW request
 */
static void udi_msc_csw_invalid(void);
#endif

/**
 * \brief Function to check the CBW length and direction
 * Call it after SCSI command decode to check integrity of command
 *
 * \param alloc_len  number of bytes that device want transfer
 * \param dir_flag   Direction of transfer (USB_CBW_DIRECTION_IN/OUT)
 *
 * \retval true if the command can be processed
 */
static bool udi_msc_cbw_validate(uint32_t alloc_len, uint8_t dir_flag);
//@}

/**
 * \name Routines to process small data packet
 */
//@{

/**
 * \brief Sends data on MSC IN endpoint
 * Called by SCSI command which must send a data to host followed by a CSW
 *
 * \param buffer        Internal RAM buffer to send
 * \param buf_size   Size of buffer to send
 */
static void udi_msc_data_send(uint8_t *buffer, uint8_t buf_size);

/**
 * \name Routines to process CSW packet
 */
//@{

/**
 * \brief Build CSW packet and send it
 *
 * Called at the end of SCSI command
 */
static void udi_msc_csw_process(void);

/**
 * \brief Sends CSW
 *
 * Called by #udi_msc_csw_process()
 * or UDD callback when endpoint halt is cleared
 */
void udi_msc_csw_send(void);

/**
 * \name Routines manage sense data
 */
//@{

/**
 * \brief Reinitialize sense data.
 */
static void udi_msc_clear_sense(void);

/**
 * \brief Update sense data with new value to signal a fail
 *
 * \param sense_key     Sense key
 * \param add_sense     Additional Sense Code
 * \param lba           LBA corresponding at error
 */
static void udi_msc_sense_fail(uint8_t sense_key, uint16_t add_sense, uint32_t lba);

/**
 * \brief Update sense data with new value to signal success
 */
static void udi_msc_sense_pass(void);

/**
 * \brief Update sense data to signal a hardware error on memory
 */
static void udi_msc_sense_fail_hardware(void);

#if USE_MSC_CHECKS
/**
 * \brief Update sense data to signal that CDB fields are not valid
 */
static void udi_msc_sense_fail_cdb_invalid(void);
#endif

/**
 * \brief Update sense data to signal that command is not supported
 */
static void udi_msc_sense_command_invalid(void);
//@}

/**
 * \name Routines manage SCSI Commands
 */
//@{

/**
 * \brief Process SPC Request Sense command
 * Returns error information about last command
 */
static void udi_msc_spc_requestsense(void);

/**
 * \brief Process SPC Inquiry command
 * Returns information (name,version) about disk
 */
static void udi_msc_spc_inquiry(void);

/**
 * \brief Checks state of disk
 *
 * \retval true if disk is ready, otherwise false and updates sense data
 */
static bool udi_msc_spc_testunitready_global(void);

/**
 * \brief Process test unit ready command
 * Returns state of logical unit
 */
static void udi_msc_spc_testunitready(void);

/**
 * \brief Process prevent allow medium removal command
 */
static void udi_msc_spc_prevent_allow_medium_removal(void);

/**
 * \brief Process mode sense command
 *
 * \param b_sense10     Sense10 SCSI command, if true
 * \param b_sense10     Sense6  SCSI command, if false
 */
static void udi_msc_spc_mode_sense(bool b_sense10);

/**
 * \brief Process start stop command
 */
static void udi_msc_sbc_start_stop(void);

/**
 * \brief Process read capacity command
 */
static void udi_msc_sbc_read_capacity(void);

/**
 * \brief Process read10 or write10 command
 *
 * \param b_read     Read transfer, if true,
 * \param b_read     Write transfer, if false
 */
static void udi_msc_sbc_trans(bool b_read);

static void udi_msc_read_format_capacity(void);

void udd_ep_set_halt(uint8_t ep) {
    stall_ep(ep);
    reset_ep(ep);
}

#if USE_MSC_CHECKS
static void udi_msc_cbw_invalid(void) {
    logmsg("MSC CBW Invalid; halt");
    udd_ep_set_halt(USB_EP_MSC_OUT);

    // TODO If stall cleared then re-stall it. Only Setup MSC Reset can clear it
}

static void udi_msc_csw_invalid(void) {
    logmsg("MSC CSW Invalid; halt");
    udd_ep_set_halt(USB_EP_MSC_IN);

    // TODO If stall cleared then re-stall it. Only Setup MSC Reset can clear it
}
#endif

bool try_read_cbw(struct usb_msc_cbw *cbw, uint8_t ep, PacketBuffer *handoverCache) {
    if (!USB_ReadCore(NULL, 1, ep, handoverCache))
        return false; // no data available

    uint32_t nb_received = USB_ReadCore((void *)cbw, sizeof(*cbw), ep, handoverCache);

#if USE_MSC_CHECKS
    // Check CBW integrity:
    // transfer status/CBW length/CBW signature
    if ((sizeof(*cbw) != nb_received) || (cbw->dCBWSignature != cpu_to_le32(USB_CBW_SIGNATURE))) {
        if (handoverCache)
            resetIntoBootloader();
        // (5.2.1) Devices receiving a CBW with an invalid signature should
        // stall
        // further traffic on the Bulk In pipe, and either stall further traffic
        // or accept and discard further traffic on the Bulk Out pipe, until
        // reset recovery.
        udi_msc_cbw_invalid();
        udi_msc_csw_invalid();
        return false;
    }

    // in handover mode we don't care about LUN
    if (handoverCache)
        return true;

    // Check LUN asked
    cbw->bCBWLUN &= USB_CBW_LUN_MASK;
    if (cbw->bCBWLUN > MAX_LUN) {
        // Bad LUN, then stop command process
        udi_msc_sense_fail_cdb_invalid();
        udi_msc_csw_process();
        return false;
    }
#else
    (void)nb_received;
#endif

    return true;
}

void process_msc(void) {
#if USE_HID || USE_WEBUSB
    process_hid();
#endif

    if (!try_read_cbw(&udi_msc_cbw, USB_EP_MSC_OUT, false))
        return; // no data

    // Prepare CSW residue field with the size requested
    udi_msc_csw.dCSWDataResidue = le32_to_cpu(udi_msc_cbw.dCBWDataTransferLength);

    // if (SBC_WRITE10 != udi_msc_cbw.CDB[0])
    //    logval("MSC CMD", udi_msc_cbw.CDB[0]);

    // Decode opcode
    switch (udi_msc_cbw.CDB[0]) {
    case SPC_REQUEST_SENSE:
        udi_msc_spc_requestsense();
        break;

    case SPC_INQUIRY:
        udi_msc_spc_inquiry();
        break;

    case SPC_MODE_SENSE6:
        udi_msc_spc_mode_sense(false);
        break;
    case SPC_MODE_SENSE10:
        udi_msc_spc_mode_sense(true);
        break;

    case SPC_TEST_UNIT_READY:
        udi_msc_spc_testunitready();
        break;

    case SBC_READ_CAPACITY10:
        udi_msc_sbc_read_capacity();
        break;

    case SBC_START_STOP_UNIT:
        udi_msc_sbc_start_stop();
        break;

    // Accepts request to support plug/plug in case of card reader
    case SPC_PREVENT_ALLOW_MEDIUM_REMOVAL:
        udi_msc_spc_prevent_allow_medium_removal();
        break;

    // Accepts request to support full format from Windows
    case SBC_VERIFY10:
        udi_msc_sense_pass();
        udi_msc_csw_process();
        break;

    case SBC_READ10:
        udi_msc_sbc_trans(true);
        break;

    case SBC_WRITE10:
        udi_msc_sbc_trans(false);
        break;

    case 0x23:
        udi_msc_read_format_capacity();
        break;

    default:
        logval("Invalid MSC command", udi_msc_cbw.CDB[0]);
        udi_msc_sense_command_invalid();
        udi_msc_csw_process();
        break;
    }
}

static bool udi_msc_cbw_validate(uint32_t alloc_len, uint8_t dir_flag) {
/*
 * The following cases should result in a phase error:
 *  - Case  2: Hn < Di
 *  - Case  3: Hn < Do
 *  - Case  7: Hi < Di
 *  - Case  8: Hi <> Do
 *  - Case 10: Ho <> Di
 *  - Case 13: Ho < Do
 */
#if USE_MSC_CHECKS
    if (((udi_msc_cbw.bmCBWFlags ^ dir_flag) & USB_CBW_DIRECTION_IN) ||
        (udi_msc_csw.dCSWDataResidue < alloc_len)) {
        udi_msc_sense_fail_cdb_invalid();
        udi_msc_csw_process();
        return false;
    }
#endif

    /*
     * The following cases should result in a stall and nonzero
     * residue:
     *  - Case  4: Hi > Dn
     *  - Case  5: Hi > Di
     *  - Case  9: Ho > Dn
     *  - Case 11: Ho > Do
     */
    return true;
}

//---------------------------------------------
//------- Routines to process small data packet

static void udi_msc_data_send(uint8_t *buffer, uint8_t buf_size) {
    if (USB_Write((void *)buffer, buf_size, USB_EP_MSC_IN) != buf_size) {
        // If endpoint not available, then exit process command
        udi_msc_sense_fail_hardware();
        udi_msc_csw_process();
    }

    // Update sense data
    udi_msc_sense_pass();
    // Update CSW
    udi_msc_csw.dCSWDataResidue -= buf_size;
    udi_msc_csw_process();
}

//---------------------------------------------
//------- Routines to process CSW packet

static void udi_msc_csw_process(void) {
    if (0 != udi_msc_csw.dCSWDataResidue) {
        logval("left-over residue", udi_msc_csw.dCSWDataResidue);

        /*
        uint8_t buf[64] = {0};
        while (udi_msc_csw.dCSWDataResidue > 0) {
            size_t len = min(udi_msc_csw.dCSWDataResidue, 64);
            USB_Write((void *)buf, len, USB_EP_MSC_IN);
            udi_msc_csw.dCSWDataResidue -= len;
        }
        */

        /*
        // Residue not NULL
        // then STALL next request from USB host on corresponding endpoint
        if (udi_msc_cbw.bmCBWFlags & USB_CBW_DIRECTION_IN)
            udd_ep_set_halt(USB_EP_MSC_IN);
        else
            udd_ep_set_halt(USB_EP_MSC_OUT);
            */
    }

    // Prepare and send CSW
    udi_msc_csw.dCSWTag = udi_msc_cbw.dCBWTag;
    udi_msc_csw.dCSWDataResidue = cpu_to_le32(udi_msc_csw.dCSWDataResidue);
    udi_msc_csw_send();
}

void udi_msc_csw_send(void) { USB_Write((void *)&udi_msc_csw, sizeof(udi_msc_csw), USB_EP_MSC_IN); }

//---------------------------------------------
//------- Routines manage sense data

static void udi_msc_clear_sense(void) {
    memset((uint8_t *)&udi_msc_sense, 0, sizeof(struct scsi_request_sense_data));
    udi_msc_sense.valid_reponse_code = SCSI_SENSE_VALID | SCSI_SENSE_CURRENT;
    udi_msc_sense.AddSenseLen = SCSI_SENSE_ADDL_LEN(sizeof(udi_msc_sense));
}

static void udi_msc_sense_fail(uint8_t sense_key, uint16_t add_sense, uint32_t lba) {
    logval("MSC sense fail", sense_key);
    udi_msc_clear_sense();
    udi_msc_csw.bCSWStatus = USB_CSW_STATUS_FAIL;
    udi_msc_sense.sense_flag_key = sense_key;
    udi_msc_sense.information[0] = lba >> 24;
    udi_msc_sense.information[1] = lba >> 16;
    udi_msc_sense.information[2] = lba >> 8;
    udi_msc_sense.information[3] = lba;
    udi_msc_sense.AddSense = add_sense;
}

static void udi_msc_sense_pass(void) {
    udi_msc_clear_sense();
    udi_msc_csw.bCSWStatus = USB_CSW_STATUS_PASS;
}

static void udi_msc_sense_fail_hardware(void) {
    udi_msc_sense_fail(SCSI_SK_HARDWARE_ERROR, SCSI_ASC_NO_ADDITIONAL_SENSE_INFO, 0);
}

#if USE_MSC_CHECKS
static void udi_msc_sense_fail_cdb_invalid(void) {
    udi_msc_sense_fail(SCSI_SK_ILLEGAL_REQUEST, SCSI_ASC_INVALID_FIELD_IN_CDB, 0);
}
#endif

static void udi_msc_sense_command_invalid(void) {
    udi_msc_sense_fail(SCSI_SK_ILLEGAL_REQUEST, SCSI_ASC_INVALID_COMMAND_OPERATION_CODE, 0);
}

//---------------------------------------------
//------- Routines manage SCSI Commands

static void udi_msc_spc_requestsense(void) {
    uint8_t length = udi_msc_cbw.CDB[4];

    // Can't send more than sense data length
    if (length > sizeof(udi_msc_sense))
        length = sizeof(udi_msc_sense);

    if (!udi_msc_cbw_validate(length, USB_CBW_DIRECTION_IN))
        return;
    // Send sense data
    udi_msc_data_send((uint8_t *)&udi_msc_sense, length);
}

static void udi_msc_read_format_capacity(void) {
    uint8_t buf[12] = {0,
                       0,
                       0,
                       8, // length
                       (NUM_FAT_BLOCKS >> 24) & 0xFF,
                       (NUM_FAT_BLOCKS >> 16) & 0xFF,
                       (NUM_FAT_BLOCKS >> 8) & 0xFF,
                       (NUM_FAT_BLOCKS >> 0) & 0xFF,
                       2, // Descriptor Code: Formatted Media
                       0,
                       (512 >> 8) & 0xff,
                       0};

    size_t length = 12;

    if (udi_msc_csw.dCSWDataResidue > length)
        udi_msc_csw.dCSWDataResidue = length;

    if (!udi_msc_cbw_validate(length, USB_CBW_DIRECTION_IN))
        return;
    udi_msc_data_send(buf, length);
}

static void udi_msc_spc_inquiry(void) {
    uint8_t length;
    __attribute__((__aligned__(4)))
    // Constant inquiry data for all LUNs
    static struct scsi_inquiry_data udi_msc_inquiry_data = {
        .pq_pdt = SCSI_INQ_PQ_CONNECTED | SCSI_INQ_DT_DIR_ACCESS,
        .version = 2, // SCSI_INQ_VER_SPC,
        .flags1 = SCSI_INQ_RMB,
        .flags3 = SCSI_INQ_RSP_SPC2,
        .addl_len = 36 - 4, // SCSI_INQ_ADDL_LEN(sizeof(struct scsi_inquiry_data)),
        // Linux displays this; Windows shows it in Dev Mgr
        .vendor_id = "",
        .product_id = "",
        .product_rev = {'1', '.', '0', '0'},
    };

    // we use both product_id and vendor_id fields and hope for the best
    padded_memcpy(udi_msc_inquiry_data.vendor_id, PRODUCT_NAME, 8 + 16);

    length = udi_msc_cbw.CDB[4];

    // Can't send more than inquiry data length
    if (length > sizeof(udi_msc_inquiry_data))
        length = sizeof(udi_msc_inquiry_data);

    if (!udi_msc_cbw_validate(length, USB_CBW_DIRECTION_IN))
        return;

    /*
if ((0 != (udi_msc_cbw.CDB[1] & (SCSI_INQ_REQ_EVPD | SCSI_INQ_REQ_CMDT))) ||
    (0 != udi_msc_cbw.CDB[2])) {
    logval("unsupp", udi_msc_cbw.CDB[1]);
    // CMDT and EPVD bits are not at 0
    // PAGE or OPERATION CODE fields are not empty
    //  = No standard inquiry asked
    udi_msc_sense_fail_cdb_invalid(); // Command is unsupported
    udi_msc_csw_process();
    return;
}
*/

    // logval("Sense Size", length);

    // Send inquiry data
    udi_msc_data_send((uint8_t *)&udi_msc_inquiry_data, length);
}

static bool udi_msc_spc_testunitready_global(void) { return true; }

static void udi_msc_spc_testunitready(void) {
    if (udi_msc_spc_testunitready_global()) {
        // LUN ready, then update sense data with status pass
        udi_msc_sense_pass();
    }
    // Send status in CSW packet
    udi_msc_csw_process();
}

static void udi_msc_spc_mode_sense(bool b_sense10) {
    // Union of all mode sense structures
    union sense_6_10 {
        struct {
            struct scsi_mode_param_header6 header;
            struct spc_control_page_info_execpt sense_data;
        } s6;
        struct {
            struct scsi_mode_param_header10 header;
            struct spc_control_page_info_execpt sense_data;
        } s10;
    };

    uint8_t data_sense_lgt;
    uint8_t mode;
    uint8_t request_lgt;
    struct spc_control_page_info_execpt *ptr_mode;
    __attribute__((__aligned__(4))) static union sense_6_10 sense;

    // Clear all fields
    memset(&sense, 0, sizeof(sense));

    // Initialize process
    if (b_sense10) {
        request_lgt = udi_msc_cbw.CDB[8];
        ptr_mode = &sense.s10.sense_data;
        data_sense_lgt = sizeof(struct scsi_mode_param_header10);
    } else {
        request_lgt = udi_msc_cbw.CDB[4];
        ptr_mode = &sense.s6.sense_data;
        data_sense_lgt = sizeof(struct scsi_mode_param_header6);
    }

    // No Block descriptor

    // Fill page(s)
    mode = udi_msc_cbw.CDB[2] & SCSI_MS_MODE_ALL;
    if ((SCSI_MS_MODE_INFEXP == mode) || (SCSI_MS_MODE_ALL == mode)) {
        // Informational exceptions control page (from SPC)
        ptr_mode->page_code = SCSI_MS_MODE_INFEXP;
        ptr_mode->page_length = SPC_MP_INFEXP_PAGE_LENGTH;
        ptr_mode->mrie = SPC_MP_INFEXP_MRIE_NO_SENSE;
        data_sense_lgt += sizeof(struct spc_control_page_info_execpt);
    }
    // Can't send more than mode sense data length
    if (request_lgt > data_sense_lgt)
        request_lgt = data_sense_lgt;
    if (!udi_msc_cbw_validate(request_lgt, USB_CBW_DIRECTION_IN))
        return;

    // Fill mode parameter header length

    if (b_sense10) {
        sense.s10.header.mode_data_length = cpu_to_be16((data_sense_lgt - 2));
    } else {
        sense.s6.header.mode_data_length = data_sense_lgt - 1;
    }

    // Send mode sense data
    udi_msc_data_send((uint8_t *)&sense, request_lgt);
}

static void udi_msc_spc_prevent_allow_medium_removal(void) {
#if USE_MSC_CHECKS
    uint8_t prevent = udi_msc_cbw.CDB[4];
    if (0 == prevent) {
        udi_msc_sense_pass();
    } else {
        udi_msc_sense_fail_cdb_invalid(); // Command is unsupported
    }
#else
    udi_msc_sense_pass();
#endif
    udi_msc_csw_process();
}

static void udi_msc_sbc_start_stop(void) {
#if 0
	bool start = 0x1 & udi_msc_cbw.CDB[4];
	bool loej = 0x2 & udi_msc_cbw.CDB[4];
	if (loej) {
		mem_unload(udi_msc_cbw.bCBWLUN, !start);
	}
#endif
    udi_msc_sense_pass();
    udi_msc_csw_process();
}

static void udi_msc_sbc_read_capacity(void) {
    __attribute__((__aligned__(4))) static struct sbc_read_capacity10_data udi_msc_capacity;

    if (!udi_msc_cbw_validate(sizeof(udi_msc_capacity), USB_CBW_DIRECTION_IN))
        return;

    udi_msc_capacity.max_lba = NUM_FAT_BLOCKS - 1;
    // Format capacity data
    udi_msc_capacity.block_len = CPU_TO_BE32(UDI_MSC_BLOCK_SIZE);
    udi_msc_capacity.max_lba = CPU_TO_BE32(udi_msc_capacity.max_lba);
    // Send the corresponding sense data
    udi_msc_data_send((uint8_t *)&udi_msc_capacity, sizeof(udi_msc_capacity));
}

__attribute__((__aligned__(4))) static uint8_t block_buffer[UDI_MSC_BLOCK_SIZE];
static WriteState usbWriteState;

static void udi_msc_sbc_trans(bool b_read) {
    uint32_t trans_size;

    //! Memory address to execute the command
    uint32_t udi_msc_addr;

    //! Number of block to transfer
    uint16_t udi_msc_nb_block;

    // Read/Write command fields (address and number of block)
    MSB0(udi_msc_addr) = udi_msc_cbw.CDB[2];
    MSB1(udi_msc_addr) = udi_msc_cbw.CDB[3];
    MSB2(udi_msc_addr) = udi_msc_cbw.CDB[4];
    MSB3(udi_msc_addr) = udi_msc_cbw.CDB[5];
    MSB(udi_msc_nb_block) = udi_msc_cbw.CDB[7];
    LSB(udi_msc_nb_block) = udi_msc_cbw.CDB[8];

    // Compute number of byte to transfer and valid it
    trans_size = (uint32_t)udi_msc_nb_block * UDI_MSC_BLOCK_SIZE;
    if (!udi_msc_cbw_validate(trans_size, (b_read) ? USB_CBW_DIRECTION_IN : USB_CBW_DIRECTION_OUT))
        return;

#if USE_DBG_MSC
    logwrite(b_read ? "read @" : "write @");
    logwritenum(udi_msc_addr);
    logwrite(" sz:");
    logwritenum(trans_size);
    logwrite("\n");
#endif

    for (uint32_t i = 0; i < udi_msc_nb_block; ++i) {
        if (!USB_Ok()) {
            logmsg("Transfer aborted.");
            return;
        }

        // logval("readblk", i);
        if (b_read) {
            read_block(udi_msc_addr + i, block_buffer);
            USB_Write(block_buffer, UDI_MSC_BLOCK_SIZE, USB_EP_MSC_IN);
        } else {
            USB_ReadBlocking(block_buffer, UDI_MSC_BLOCK_SIZE, USB_EP_MSC_OUT, 0);

#if 0
            check_uf2_handover(block_buffer, udi_msc_nb_block - i - 1, USB_EP_MSC_IN,
                               USB_EP_MSC_OUT, udi_msc_cbw.dCBWTag);
#endif

            write_block(udi_msc_addr + i, block_buffer, false, &usbWriteState);
            led_signal();
        }
        udi_msc_csw.dCSWDataResidue -= UDI_MSC_BLOCK_SIZE;
    }

    udi_msc_sense_pass();

    // Send status of transfer in CSW packet
    udi_msc_csw_process();
}

#if USE_MSC_HANDOVER
static void handover_flash(UF2_HandoverArgs *handover, PacketBuffer *handoverCache,
                           WriteState *state) {
    for (uint32_t i = 0; i < handover->blocks_remaining; ++i) {
        USB_ReadBlocking(handover->buffer, UDI_MSC_BLOCK_SIZE, handover->ep_out, handoverCache);
        write_block(0x1000 + i, handover->buffer, true, state);
    }
}

static void process_handover_initial(UF2_HandoverArgs *handover, PacketBuffer *handoverCache,
                                     WriteState *state) {
    struct usb_msc_csw csw = {.dCSWTag = handover->cbw_tag,
                              .dCSWSignature = cpu_to_le32(USB_CSW_SIGNATURE),
                              .bCSWStatus = USB_CSW_STATUS_PASS,
                              .dCSWDataResidue = 0};
    // write out the block passed from user space
    write_block(0xfff, handover->buffer, true, state);
    // read-write remaining blocks
    handover_flash(handover, handoverCache, state);
    // send USB response, as the user space isn't gonna do it
    USB_WriteCore((void *)&csw, sizeof(csw), handover->ep_in, true);
}

static void process_handover(UF2_HandoverArgs *handover, PacketBuffer *handoverCache,
                             WriteState *state) {
    struct usb_msc_cbw cbw;
    int num = 0;

    while (!try_read_cbw(&cbw, handover->ep_out, handoverCache)) {
        // TODO is this the right value?
        if (num++ > TIMER_STEP * 50) {
            resetIntoApp();
        }
    }

    struct usb_msc_csw csw = {.dCSWTag = cbw.dCBWTag,
                              .dCSWSignature = cpu_to_le32(USB_CSW_SIGNATURE),
                              .bCSWStatus = USB_CSW_STATUS_PASS,
                              .dCSWDataResidue = le32_to_cpu(cbw.dCBWDataTransferLength)};

    // if (SBC_WRITE10 != udi_msc_cbw.CDB[0])
    //    logval("MSC CMD", udi_msc_cbw.CDB[0]);

    uint16_t udi_msc_nb_block;

    switch (cbw.CDB[0]) {
    case SPC_TEST_UNIT_READY:
        // ready, nothing to do
        break;

    case SBC_WRITE10:
        MSB(udi_msc_nb_block) = cbw.CDB[7];
        LSB(udi_msc_nb_block) = cbw.CDB[8];
        handover->blocks_remaining = udi_msc_nb_block;
        handover_flash(handover, handoverCache, state);
        csw.dCSWDataResidue -= UDI_MSC_BLOCK_SIZE * udi_msc_nb_block;
        break;

    default:
        resetIntoBootloader();
        break;
    }

    USB_WriteCore((void *)&csw, sizeof(csw), handover->ep_in, true);
}

void handoverPrep() {
    __disable_irq();
    __DMB();

    USB->DEVICE.INTENCLR.reg = USB_DEVICE_INTENCLR_MASK;
    USB->DEVICE.INTFLAG.reg = USB_DEVICE_INTFLAG_MASK;

    SCB->VTOR = 0;
}

static void handover(UF2_HandoverArgs *args) {
    handoverPrep();

    PacketBuffer cache = {0};
    WriteState writeState = {0};
    cache.read_job = 2;

    // They may have 0x80 bit set
    args->ep_in &= 0xf;
    args->ep_out &= 0xf;

    process_handover_initial(args, &cache, &writeState);
    while (1) {
        process_handover(args, &cache, &writeState);
    }
}
#endif

__attribute__((section(".binfo"))) __attribute__((__used__)) const UF2_BInfo binfo = {
#ifdef HAS_CONFIG_DATA
    .config_data = config_data,
#endif
#if USE_MSC_HANDOVER
    .handoverMSC = handover,
#endif
#if USE_HID_HANDOVER
    .handoverHID = hidHandoverLoop,
#endif
    .info_uf2 = infoUf2File,
};
