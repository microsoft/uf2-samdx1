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

#define NVM_SW_CALIB_DFLL48M_COARSE_VAL 58
#define NVM_SW_CALIB_DFLL48M_FINE_VAL 64

volatile bool g_interrupt_enabled = true;

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

    SysTick_Config(150);
}

void SysTick_Handler(void) {
    LED_TICK();
}
