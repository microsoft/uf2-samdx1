/**
 * \file
 *
 * \brief Management of the virtual memory.
 *
 * This file manages the virtual memory.
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
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef _VIRTUAL_MEM_H_
#define _VIRTUAL_MEM_H_


#include "conf_access.h"

#if VIRTUAL_MEM == ENABLE

#include "ctrl_access.h"

#ifdef __cplusplus
extern "C" {
#endif

//_____ D E F I N I T I O N S ______________________________________________

#define VMEM_SECTOR_SIZE   512


//---- CONTROL FUNCTIONS ----

extern Ctrl_status  virtual_test_unit_ready(void);
extern Ctrl_status  virtual_read_capacity(uint32_t *u32_nb_sector);
extern bool         virtual_wr_protect(void);
extern bool         virtual_removal(void);
extern bool         virtual_unload(bool unload);


//---- ACCESS DATA FUNCTIONS ----

// USB interface
#if ACCESS_USB == true
extern Ctrl_status  virtual_usb_read_10 (uint32_t addr, uint16_t nb_sector);
extern Ctrl_status  virtual_usb_write_10(uint32_t addr, uint16_t nb_sector);
#endif

// RAM interface
#if ACCESS_MEM_TO_RAM == true
extern Ctrl_status  virtual_mem_2_ram(uint32_t addr,       void *ram);
extern Ctrl_status  virtual_ram_2_mem(uint32_t addr, const void *ram);
#endif

#ifdef __cplusplus
}
#endif

#endif

#endif  // _VIRTUAL_MEM_H_
