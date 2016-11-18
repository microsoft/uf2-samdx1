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

#include "cdc_enumerate.h"
#include <stdint.h>
#include <compiler.h>
#include <string.h>

#include "services/usb/class/msc/sbc_protocol.h"
#include "services/usb/class/msc/spc_protocol.h"
#include "services/usb/class/msc/usb_protocol_msc.h"


bool mscReset = false;


void msc_reset(void) {
	mscReset = true;
	reset_ep(USB_EP_MSC_IN);
	reset_ep(USB_EP_MSC_OUT);
}



//! Structure to receive a CBW packet
static struct usb_msc_cbw udi_msc_cbw;
//! Structure to send a CSW packet
static struct usb_msc_csw udi_msc_csw =
		{.dCSWSignature = CPU_TO_BE32(USB_CSW_SIGNATURE) };
//! Structure with current SCSI sense data
static struct scsi_request_sense_data udi_msc_sense;
static bool udi_msc_b_cbw_invalid = false;


/**
 * \brief Stall CBW request
 */
static void udi_msc_cbw_invalid(void);

/**
 * \brief Stall CSW request
 */
static void udi_msc_csw_invalid(void);

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
static void udi_msc_data_send(uint8_t * buffer, uint8_t buf_size);


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
static void udi_msc_sense_fail(uint8_t sense_key, uint16_t add_sense,
		uint32_t lba);

/**
 * \brief Update sense data with new value to signal success
 */
static void udi_msc_sense_pass(void);

/**
 * \brief Update sense data to signal a hardware error on memory
 */
static void udi_msc_sense_fail_hardware(void);

/**
 * \brief Update sense data to signal that CDB fields are not valid
 */
static void udi_msc_sense_fail_cdb_invalid(void);

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

void udd_ep_set_halt(uint8_t ep) {
	stall_ep(ep);
	reset_ep(ep);
}

static void udi_msc_cbw_invalid(void)
{
	if (!udi_msc_b_cbw_invalid)
		return;	// Don't re-stall endpoint if error reseted by setup

	udd_ep_set_halt(USB_EP_MSC_OUT);

	// TODO If stall cleared then re-stall it. Only Setup MSC Reset can clear it
}

static void udi_msc_csw_invalid(void)
{
	if (!udi_msc_b_cbw_invalid)
		return;	// Don't re-stall endpoint if error reseted by setup
	
	stall_ep(USB_EP_MSC_IN);
	reset_ep(USB_EP_MSC_IN);
	
	// TODO If stall cleared then re-stall it. Only Setup MSC Reset can clear it
}


void process_msc(void)
{
	if (!USB_Read(NULL, 1, USB_EP_MSC_OUT))
		return; // no data available
	
	uint32_t nb_received = USB_Read((void*)&udi_msc_cbw, sizeof(udi_msc_cbw), USB_EP_MSC_OUT);

	// Check CBW integrity:
	// transfer status/CBW length/CBW signature
	if ((sizeof(udi_msc_cbw) != nb_received)
			|| (udi_msc_cbw.dCBWSignature !=
					CPU_TO_BE32(USB_CBW_SIGNATURE))) {
		// (5.2.1) Devices receiving a CBW with an invalid signature should stall
		// further traffic on the Bulk In pipe, and either stall further traffic
		// or accept and discard further traffic on the Bulk Out pipe, until
		// reset recovery.
		udi_msc_b_cbw_invalid = true;
		udi_msc_cbw_invalid();
		udi_msc_csw_invalid();
		return;
	}
	// Check LUN asked
	udi_msc_cbw.bCBWLUN &= USB_CBW_LUN_MASK;
	if (udi_msc_cbw.bCBWLUN > MAX_LUN) {
		// Bad LUN, then stop command process
		udi_msc_sense_fail_cdb_invalid();
		udi_msc_csw_process();
		return;
	}
	// Prepare CSW residue field with the size requested
	udi_msc_csw.dCSWDataResidue =
			le32_to_cpu(udi_msc_cbw.dCBWDataTransferLength);

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

	default:
		udi_msc_sense_command_invalid();
		udi_msc_csw_process();
		break;
	}
	
}


static bool udi_msc_cbw_validate(uint32_t alloc_len, uint8_t dir_flag)
{
	/*
	 * The following cases should result in a phase error:
	 *  - Case  2: Hn < Di
	 *  - Case  3: Hn < Do
	 *  - Case  7: Hi < Di
	 *  - Case  8: Hi <> Do
	 *  - Case 10: Ho <> Di
	 *  - Case 13: Ho < Do
	 */
	if (((udi_msc_cbw.bmCBWFlags ^ dir_flag) & USB_CBW_DIRECTION_IN)
			|| (udi_msc_csw.dCSWDataResidue < alloc_len)) {
		udi_msc_sense_fail_cdb_invalid();
		udi_msc_csw_process();
		return false;
	}

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


static void udi_msc_data_send(uint8_t * buffer, uint8_t buf_size)
{
	if (USB_Write((void*)buffer, buf_size, USB_EP_MSC_IN) != buf_size) {
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

static void udi_msc_csw_process(void)
{
	if (0 != udi_msc_csw.dCSWDataResidue) {
		// Residue not NULL
		// then STALL next request from USB host on corresponding endpoint
		if (udi_msc_cbw.bmCBWFlags & USB_CBW_DIRECTION_IN)
			udd_ep_set_halt(USB_EP_MSC_IN);
		else
			udd_ep_set_halt(USB_EP_MSC_OUT);
	}
	// Prepare and send CSW
	udi_msc_csw.dCSWTag = udi_msc_cbw.dCBWTag;
	udi_msc_csw.dCSWDataResidue = cpu_to_le32(udi_msc_csw.dCSWDataResidue);
	udi_msc_csw_send();
}


void udi_msc_csw_send(void)
{
	USB_Write((void*)& udi_msc_csw, sizeof(udi_msc_csw), USB_EP_MSC_IN);
}


//---------------------------------------------
//------- Routines manage sense data

static void udi_msc_clear_sense(void)
{
	memset((uint8_t*)&udi_msc_sense, 0, sizeof(struct scsi_request_sense_data));
	udi_msc_sense.valid_reponse_code = SCSI_SENSE_VALID | SCSI_SENSE_CURRENT;
	udi_msc_sense.AddSenseLen = SCSI_SENSE_ADDL_LEN(sizeof(udi_msc_sense));
}

static void udi_msc_sense_fail(uint8_t sense_key, uint16_t add_sense,
		uint32_t lba)
{
	udi_msc_clear_sense();
	udi_msc_csw.bCSWStatus = USB_CSW_STATUS_FAIL;
	udi_msc_sense.sense_flag_key = sense_key;
	udi_msc_sense.information[0] = lba >> 24;
	udi_msc_sense.information[1] = lba >> 16;
	udi_msc_sense.information[2] = lba >> 8;
	udi_msc_sense.information[3] = lba;
	udi_msc_sense.AddSenseCode = add_sense >> 8;
	udi_msc_sense.AddSnsCodeQlfr = add_sense;
}

static void udi_msc_sense_pass(void)
{
	udi_msc_clear_sense();
	udi_msc_csw.bCSWStatus = USB_CSW_STATUS_PASS;
}


static void udi_msc_sense_fail_hardware(void)
{
	udi_msc_sense_fail(SCSI_SK_HARDWARE_ERROR,
			SCSI_ASC_NO_ADDITIONAL_SENSE_INFO, 0);
}

static void udi_msc_sense_fail_cdb_invalid(void)
{
	udi_msc_sense_fail(SCSI_SK_ILLEGAL_REQUEST,
			SCSI_ASC_INVALID_FIELD_IN_CDB, 0);
}

static void udi_msc_sense_command_invalid(void)
{
	udi_msc_sense_fail(SCSI_SK_ILLEGAL_REQUEST,
			SCSI_ASC_INVALID_COMMAND_OPERATION_CODE, 0);
}


//---------------------------------------------
//------- Routines manage SCSI Commands

static void udi_msc_spc_requestsense(void)
{
	uint8_t length = udi_msc_cbw.CDB[4];

	// Can't send more than sense data length
	if (length > sizeof(udi_msc_sense))
		length = sizeof(udi_msc_sense);

	if (!udi_msc_cbw_validate(length, USB_CBW_DIRECTION_IN))
		return;
	// Send sense data
	udi_msc_data_send((uint8_t*)&udi_msc_sense, length);
}


static void udi_msc_spc_inquiry(void)
{
	uint8_t length, i;
	COMPILER_ALIGNED(4)
	// Constant inquiry data for all LUNs
	static struct scsi_inquiry_data udi_msc_inquiry_data = {
		.pq_pdt = SCSI_INQ_PQ_CONNECTED | SCSI_INQ_DT_DIR_ACCESS,
		.version = SCSI_INQ_VER_SPC,
		.flags3 = SCSI_INQ_RSP_SPC2,
		.addl_len = SCSI_INQ_ADDL_LEN(sizeof(struct scsi_inquiry_data)),
		.vendor_id = {'A', 'T', 'M', 'E', 'L', ' ', ' ', ' '},
		.product_rev = {'1', '.', '0', '0'},
	};

	length = udi_msc_cbw.CDB[4];

	// Can't send more than inquiry data length
	if (length > sizeof(udi_msc_inquiry_data))
		length = sizeof(udi_msc_inquiry_data);

	if (!udi_msc_cbw_validate(length, USB_CBW_DIRECTION_IN))
		return;
	if ((0 != (udi_msc_cbw.CDB[1] & (SCSI_INQ_REQ_EVPD | SCSI_INQ_REQ_CMDT)))
			|| (0 != udi_msc_cbw.CDB[2])) {
		// CMDT and EPVD bits are not at 0
		// PAGE or OPERATION CODE fields are not empty
		//  = No standard inquiry asked
		udi_msc_sense_fail_cdb_invalid(); // Command is unsupported
		udi_msc_csw_process();
		return;
	}

	udi_msc_inquiry_data.flags1 = SCSI_INQ_RMB; // removable

	//* Fill product ID field
	// Copy name in product id field
	memcpy(udi_msc_inquiry_data.product_id,
			"MSC Bootloader",
			sizeof(udi_msc_inquiry_data.product_id));

	// Search end of name '/0' or '"'
	i = 0;
	while (sizeof(udi_msc_inquiry_data.product_id) != i) {
		if ((0 == udi_msc_inquiry_data.product_id[i])
				|| ('"' == udi_msc_inquiry_data.product_id[i])) {
			break;
		}
		i++;
	}
	// Padding with space char
	while (sizeof(udi_msc_inquiry_data.product_id) != i) {
		udi_msc_inquiry_data.product_id[i] = ' ';
		i++;
	}

	// Send inquiry data
	udi_msc_data_send((uint8_t *) & udi_msc_inquiry_data, length);
}


static bool udi_msc_spc_testunitready_global(void)
{
	return true;
}


static void udi_msc_spc_testunitready(void)
{
	if (udi_msc_spc_testunitready_global()) {
		// LUN ready, then update sense data with status pass
		udi_msc_sense_pass();
	}
	// Send status in CSW packet
	udi_msc_csw_process();
}


static void udi_msc_spc_mode_sense(bool b_sense10)
{
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
	uint8_t wp;
	struct spc_control_page_info_execpt *ptr_mode;
	COMPILER_ALIGNED(4)  static union sense_6_10 sense;

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
	if ((SCSI_MS_MODE_INFEXP == mode)
			|| (SCSI_MS_MODE_ALL == mode)) {
		// Informational exceptions control page (from SPC)
		ptr_mode->page_code =
				SCSI_MS_MODE_INFEXP;
		ptr_mode->page_length =
				SPC_MP_INFEXP_PAGE_LENGTH;
		ptr_mode->mrie =
				SPC_MP_INFEXP_MRIE_NO_SENSE;
		data_sense_lgt += sizeof(struct spc_control_page_info_execpt);
	}
	// Can't send more than mode sense data length
	if (request_lgt > data_sense_lgt)
		request_lgt = data_sense_lgt;
	if (!udi_msc_cbw_validate(request_lgt, USB_CBW_DIRECTION_IN))
		return;

	// Fill mode parameter header length
	wp = 0; // not write protected SCSI_MS_SBC_WP

	if (b_sense10) {
		sense.s10.header.mode_data_length =
				cpu_to_be16((data_sense_lgt - 2));
		//sense.s10.header.medium_type                 = 0;
		sense.s10.header.device_specific_parameter = wp;
		//sense.s10.header.block_descriptor_length     = 0;
	} else {
		sense.s6.header.mode_data_length = data_sense_lgt - 1;
		//sense.s6.header.medium_type                  = 0;
		sense.s6.header.device_specific_parameter = wp;
		//sense.s6.header.block_descriptor_length      = 0;
	}

	// Send mode sense data
	udi_msc_data_send((uint8_t *) & sense, request_lgt);
}


static void udi_msc_spc_prevent_allow_medium_removal(void)
{
	uint8_t prevent = udi_msc_cbw.CDB[4];
	if (0 == prevent) {
		udi_msc_sense_pass();
	} else {
		udi_msc_sense_fail_cdb_invalid(); // Command is unsupported
	}
	udi_msc_csw_process();
}


static void udi_msc_sbc_start_stop(void)
{
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


static void udi_msc_sbc_read_capacity(void)
{
	COMPILER_ALIGNED(4) static struct sbc_read_capacity10_data udi_msc_capacity;

	if (!udi_msc_cbw_validate(sizeof(udi_msc_capacity),
					USB_CBW_DIRECTION_IN))
		return;
	
	udi_msc_capacity.max_lba = NUM_FAT_BLOCKS;
	// Format capacity data
	udi_msc_capacity.block_len = CPU_TO_BE32(UDI_MSC_BLOCK_SIZE);
	udi_msc_capacity.max_lba = cpu_to_be32(udi_msc_capacity.max_lba);
	// Send the corresponding sense data
	udi_msc_data_send((uint8_t *) & udi_msc_capacity,
			sizeof(udi_msc_capacity));
}

void read_block(uint32_t block_no, uint8_t *data) {
	memset(data, 0, 512);
}

void write_block(uint32_t block_no, uint8_t *data) {
}

COMPILER_ALIGNED(4) static uint8_t block_buffer[UDI_MSC_BLOCK_SIZE];

static void udi_msc_sbc_trans(bool b_read)
{
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
	trans_size = (uint32_t) udi_msc_nb_block *UDI_MSC_BLOCK_SIZE;
	if (!udi_msc_cbw_validate(trans_size,
					(b_read) ? USB_CBW_DIRECTION_IN :
					USB_CBW_DIRECTION_OUT))
		return;
	
	for (uint32_t i = 0; i < udi_msc_nb_block; ++i) {
		if (b_read) {
			read_block(udi_msc_addr + i, block_buffer);
			USB_Write(block_buffer, UDI_MSC_BLOCK_SIZE, USB_EP_MSC_IN);
		}
		else {
			USB_ReadBlocking(block_buffer, UDI_MSC_BLOCK_SIZE, USB_EP_MSC_OUT);
			write_block(udi_msc_addr + i, block_buffer);	
		}
		udi_msc_csw.dCSWDataResidue -= UDI_MSC_BLOCK_SIZE;
	}	

	udi_msc_sense_pass();
	
	// Send status of transfer in CSW packet
	udi_msc_csw_process();
}

