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

/**
 * --------------------
 * SAM-BA Implementation on SAMD21
 * --------------------
 * Requirements to use SAM-BA :
 *
 * Supported communication interfaces :
 * --------------------
 *
 * SERCOM5 : RX:PB23 TX:PB22
 * Baudrate : 115200 8N1
 *
 * USB : D-:PA24 D+:PA25
 *
 * Pins Usage
 * --------------------
 * The following pins are used by the program :
 * PA25 : input/output
 * PA24 : input/output
 * PB23 : input
 * PB22 : output
 * PA15 : input
 *
 * The application board shall avoid driving the PA25,PA24,PB23,PB22 and PA15
 * signals
 * while the boot program is running (after a POR for example)
 *
 * Clock system
 * --------------------
 * CPU clock source (GCLK_GEN_0) - 8MHz internal oscillator (OSC8M)
 * SERCOM5 core GCLK source (GCLK_ID_SERCOM5_CORE) - GCLK_GEN_0 (i.e., OSC8M)
 * GCLK Generator 1 source (GCLK_GEN_1) - 48MHz DFLL in Clock Recovery mode
 * (DFLL48M)
 * USB GCLK source (GCLK_ID_USB) - GCLK_GEN_1 (i.e., DFLL in CRM mode)
 *
 * Memory Mapping
 * --------------------
 * SAM-BA code will be located at 0x0 and executed before any applicative code.
 *
 * Applications compiled to be executed along with the bootloader will start at
 * 0x2000
 * Before jumping to the application, the bootloader changes the VTOR register
 * to use the interrupt vectors of the application @0x2000.<- not required as
 * application code is taking care of this
 *
*/

#include "uf2.h"

#define NVM_SW_CALIB_DFLL48M_COARSE_VAL 58
#define NVM_SW_CALIB_DFLL48M_FINE_VAL 64

static void check_start_application(void);

static volatile bool main_b_cdc_enable = false;

// Last word in RAM
// Unlike for ordinary applications, our link script doesn't place the stack at the bottom
// of the RAM, but instead after all allocated BSS.
// In other words, this word should survive reset.
#define DBL_TAP_PTR ((volatile uint32_t *)(HMCRAMC0_ADDR + HMCRAMC0_SIZE - 4))
#define DBL_TAP_MAGIC 0xf01669ef // Randomly selected, adjusted to have first and last bit set

/**
 * \brief Check the application startup condition
 *
 */
static void check_start_application(void) {
    uint32_t app_start_address;

    /* Load the Reset Handler address of the application */
    app_start_address = *(uint32_t *)(APP_START_ADDRESS + 4);

    /**
     * Test reset vector of application @APP_START_ADDRESS+4
     * Stay in SAM-BA if *(APP_START+0x4) == 0xFFFFFFFF
     * Application erased condition
     */
    if (app_start_address == 0xFFFFFFFF) {
        /* Stay in bootloader */
        return;
    }

    if (PM->RCAUSE.bit.POR) {
        *DBL_TAP_PTR = 0;
    } else if (*DBL_TAP_PTR == DBL_TAP_MAGIC) {
        *DBL_TAP_PTR = 0;
        return; // stay in bootloader
    } else {
        *DBL_TAP_PTR = DBL_TAP_MAGIC;
        for (int i = 1; i < 100000; ++i) {
            asm("nop");
        }
        *DBL_TAP_PTR = 0;
    }

    bulb_off();

    /* Rebase the Stack Pointer */
    __set_MSP(*(uint32_t *)APP_START_ADDRESS);

    /* Rebase the vector table base address */
    SCB->VTOR = ((uint32_t)APP_START_ADDRESS & SCB_VTOR_TBLOFF_Msk);

    /* Jump to application Reset Handler in the application */
    asm("bx %0" ::"r"(app_start_address));
}

void system_init(void) {
    /* Configure flash wait states */
    NVMCTRL->CTRLB.bit.RWS = FLASH_WAIT_STATES;

    /* Set OSC8M prescalar to divide by 1 */
    SYSCTRL->OSC8M.bit.PRESC = 0;

    /* Configure OSC8M as source for GCLK_GEN0 */
    GCLK_GENCTRL_Type genctrl = {0};
    uint32_t temp_genctrl;
    GCLK->GENCTRL.bit.ID = 0; /* GENERATOR_ID - GCLK_GEN_0 */
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
        ;
    temp_genctrl = GCLK->GENCTRL.reg;
    genctrl.bit.SRC = GCLK_GENCTRL_SRC_OSC8M_Val;
    genctrl.bit.GENEN = true;
    genctrl.bit.RUNSTDBY = false;
    GCLK->GENCTRL.reg = (genctrl.reg | temp_genctrl);
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
        ;

    SYSCTRL_DFLLCTRL_Type dfllctrl_conf = {0};
    SYSCTRL_DFLLVAL_Type dfllval_conf = {0};
    uint32_t coarse = (*((uint32_t *)(NVMCTRL_OTP4) + (NVM_SW_CALIB_DFLL48M_COARSE_VAL / 32)) >>
                       (NVM_SW_CALIB_DFLL48M_COARSE_VAL % 32)) &
                      ((1 << 6) - 1);
    if (coarse == 0x3f) {
        coarse = 0x1f;
    }
    uint32_t fine = (*((uint32_t *)(NVMCTRL_OTP4) + (NVM_SW_CALIB_DFLL48M_FINE_VAL / 32)) >>
                     (NVM_SW_CALIB_DFLL48M_FINE_VAL % 32)) &
                    ((1 << 10) - 1);
    if (fine == 0x3ff) {
        fine = 0x1ff;
    }
    dfllval_conf.bit.COARSE = coarse;
    dfllval_conf.bit.FINE = fine;
    dfllctrl_conf.bit.USBCRM = true;
    dfllctrl_conf.bit.BPLCKC = false;
    dfllctrl_conf.bit.QLDIS = false;
    dfllctrl_conf.bit.CCDIS = true;
    dfllctrl_conf.bit.ENABLE = true;

    SYSCTRL->DFLLCTRL.bit.ONDEMAND = false;
    while (!(SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY))
        ;
    SYSCTRL->DFLLMUL.reg = 48000;
    SYSCTRL->DFLLVAL.reg = dfllval_conf.reg;
    SYSCTRL->DFLLCTRL.reg = dfllctrl_conf.reg;

    GCLK_CLKCTRL_Type clkctrl = {0};
    uint16_t temp;
    GCLK->CLKCTRL.bit.ID = 0; /* GCLK_ID - DFLL48M Reference */
    temp = GCLK->CLKCTRL.reg;
    clkctrl.bit.CLKEN = true;
    clkctrl.bit.WRTLOCK = false;
    clkctrl.bit.GEN = GCLK_CLKCTRL_GEN_GCLK0_Val;
    GCLK->CLKCTRL.reg = (clkctrl.reg | temp);

    /* Configure DFLL48M as source for GCLK_GEN1 */
    GCLK->GENCTRL.bit.ID = 1; /* GENERATOR_ID - GCLK_GEN_1 */
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
        ;
    temp_genctrl = GCLK->GENCTRL.reg;
    genctrl.bit.SRC = GCLK_GENCTRL_SRC_DFLL48M_Val;
    genctrl.bit.GENEN = true;
    genctrl.bit.RUNSTDBY = false;
    GCLK->GENCTRL.reg = (genctrl.reg | temp_genctrl);
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
        ;
}

extern char _etext;

/**
 *  \brief SAMD21 SAM-BA Main loop.
 *  \return Unused (ANSI-C compatibility).
 */
int main(void) {
    bulb_init();
    bulb_on();

    logmsg("Start");
    assert((uint32_t)&_etext < APP_START_ADDRESS);

    assert(8 << NVMCTRL->PARAM.bit.PSZ == FLASH_PAGE_SIZE);
    assert(FLASH_PAGE_SIZE * NVMCTRL->PARAM.bit.NVMP == FLASH_SIZE);

    /* Jump in application if condition is satisfied */
    check_start_application();

    /* We have determined we should stay in the monitor. */
    /* System initialization */
    system_init();
    cpu_irq_enable();

#if USE_UART
    /* UART is enabled in all cases */
    usart_open();
#endif

    logmsg("Before main loop");

    init_fat();
    usb_init();

    /* Wait for a complete enum on usb or a '#' char on serial line */
    while (1) {
        if (USB_Ok()) {
            main_b_cdc_enable = true;
        }

#if USE_MONITOR
        // Check if a USB enumeration has succeeded
        // And com port was opened
        if (main_b_cdc_enable) {
            logmsg("entering monitor loop");
            sam_ba_monitor_init(SAM_BA_INTERFACE_USBCDC);
            // SAM-BA on USB loop
            while (1) {
                sam_ba_monitor_run();
            }
        }
#if USE_UART
        /* Check if a '#' has been received */
        if (!main_b_cdc_enable && usart_sharp_received()) {
            sam_ba_monitor_init(SAM_BA_INTERFACE_USART);
            /* SAM-BA on UART loop */
            while (1) {
                sam_ba_monitor_run();
            }
        }
#endif
#else // no monitor
    if (main_b_cdc_enable) {
        process_msc();
    }
#endif

    }
}
