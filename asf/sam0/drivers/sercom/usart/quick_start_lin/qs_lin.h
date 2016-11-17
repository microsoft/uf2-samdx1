/**
 * \file
 *
 * \brief SAM USART LIN Quick Start
 *
 * Copyright (C) 2015 Atmel Corporation. All rights reserved.
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

/**
 * \page asfdoc_sam0_sercom_usart_lin_use_case Quick Start Guide for SERCOM USART LIN
 *
 * The supported board list:
 *    - SAMC21 Xplained Pro
 *
 * This quick start will set up LIN frame format transmission according to your
 * configuration \c CONF_LIN_NODE_TYPE.
 * For LIN master, it will send LIN command after startup.
 * For LIN salve, once received a format from LIN master with ID \c LIN_ID_FIELD_VALUE,
 * it will reply four data bytes plus a checksum.
 *
 * \section asfdoc_sam0_sercom_usart_lin_use_case_setup Setup
 *
 * \subsection asfdoc_sam0_sercom_usart_lin_use_case_prereq Prerequisites
 * When verify data transmission between LIN master and slave, two boards are needed:
 * one is for LIN master and the other is for LIN slave.
 * connect LIN master LIN PIN with LIN slave LIN PIN.
 *
 * \subsection asfdoc_sam0_usart_lin_use_case_setup_code Code
 * Add to the main application source file, outside of any functions:
 * \snippet qs_lin.c module_var
 *
 * Copy-paste the following setup code to your user application:
 * \snippet qs_lin.c setup
 *
 * Add to user application initialization (typically the start of \c main()):
 * \snippet qs_lin.c setup_init
 *
 * \subsection asfdoc_sam0_usart_lin_use_case_setup_flow Workflow
 * -# Create USART CDC and LIN module software instance structure for the USART module to store
 *    the USART driver state while it is in use.
 *    \snippet qs_lin.c module_inst
 * -# Define LIN ID field for header format.
 *    \snippet qs_lin.c lin_id
 *    \note The ID \c LIN_ID_FIELD_VALUE is eight bits as [P1,P0,ID5...ID0], when it's 0x64, the
 * 		data field length is four bytes plus a checksum byte.
 *
 * -# Define LIN RX/TX buffer.
 *    \snippet qs_lin.c lin_buffer
 *    \note For \c tx_buffer and \c rx_buffer, the last byte is for checksum.
 *
 * -# Configure the USART CDC for output message.
 *     \snippet qs_lin.c CDC_setup
 *
 * -# Configure the USART LIN module.
 *     \snippet qs_lin.c lin_setup
 *    \note The LIN frame format can be configured as master or slave, refer to \c CONF_LIN_NODE_TYPE .
 *
 * \section asfdoc_sam0_usart_lin_use_case_main Use Case
 *
 * \subsection asfdoc_sam0_usart_lin_use_case_main_code Code
 * Copy-paste the following code to your user application:
 * \snippet qs_lin.c main_setup
 *
 * \subsection asfdoc_sam0_usart_lin_use_case_main_flow Workflow
 * -# Set up USART LIN module.
 *     \snippet qs_lin.c configure_lin
 * -# For LIN master, sending LIN command. For LIN slaver, start reading data .
 *     \snippet qs_lin.c lin_master_cmd
 */
