#include "uf2.h"

void system_init(void) {
    /* Set 1 Flash Wait State for 48MHz */
    NVMCTRL->CTRLA.reg |= NVMCTRL_CTRLA_RWS(0);

    // Output GCLK0 to Metro M4 D5. This way we can see if/when we mess it up.
    //PORT->Group[1].PINCFG[14].bit.PMUXEN = true;
    //PORT->Group[1].PMUX[7].bit.PMUXE = 12;

    /* Software reset the module to ensure it is re-initialized correctly */
    /* Note: Due to synchronization, there is a delay from writing CTRL.SWRST until the reset is complete.
     * CTRL.SWRST and STATUS.SYNCBUSY will both be cleared when the reset is complete
     */
    GCLK->CTRLA.bit.SWRST = 1;
    while (GCLK->SYNCBUSY.bit.SWRST) {
        /* wait for reset to complete */
    }

    // Temporarily switch the CPU to the internal 32k oscillator while we
    // reconfigure the DFLL.
    GCLK->GENCTRL[0].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_OSCULP32K) |
                           GCLK_GENCTRL_OE |
                           GCLK_GENCTRL_GENEN;

    while (GCLK->SYNCBUSY.bit.GENCTRL0) {
        /* Wait for synchronization */
    }

    // Configure the DFLL for USB clock recovery.
    OSCCTRL->DFLLCTRLA.reg = 0;

    OSCCTRL->DFLLMUL.reg = OSCCTRL_DFLLMUL_CSTEP( 0x1 ) |
                           OSCCTRL_DFLLMUL_FSTEP( 0x1 ) |
                           OSCCTRL_DFLLMUL_MUL( 0xBB80 );

    while (OSCCTRL->DFLLSYNC.bit.DFLLMUL) {
        /* Wait for synchronization */
    }

    OSCCTRL->DFLLCTRLB.reg = 0;
    while (OSCCTRL->DFLLSYNC.bit.DFLLCTRLB) {
        /* Wait for synchronization */
    }

    OSCCTRL->DFLLCTRLA.bit.ENABLE = true;
    while (OSCCTRL->DFLLSYNC.bit.ENABLE) {
        /* Wait for synchronization */
    }

    OSCCTRL->DFLLVAL.reg = OSCCTRL->DFLLVAL.reg;
    while(OSCCTRL->DFLLSYNC.bit.DFLLVAL ) {}

    OSCCTRL->DFLLCTRLB.reg = OSCCTRL_DFLLCTRLB_WAITLOCK |
    OSCCTRL_DFLLCTRLB_CCDIS | OSCCTRL_DFLLCTRLB_USBCRM ;

    while (!OSCCTRL->STATUS.bit.DFLLRDY) {
        /* Wait for synchronization */
    }

    // 5) Switch Generic Clock Generator 0 to DFLL48M. CPU will run at 48MHz.
    GCLK->GENCTRL[0].reg =
        GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DFLL) |
                         GCLK_GENCTRL_IDC |
                         GCLK_GENCTRL_OE |
                         GCLK_GENCTRL_GENEN;

    while (GCLK->SYNCBUSY.bit.GENCTRL0) {
      /* Wait for synchronization */
    }

    /* Turn on the digital interface clock */
    //MCLK->APBAMASK.reg |= MCLK_APBAMASK_GCLK;

    /*
     * Now that all system clocks are configured, we can set CLKDIV .
     * These values are normally the ones present after Reset.
     */
    MCLK->CPUDIV.reg = MCLK_CPUDIV_DIV_DIV1;

    SysTick_Config(1000);
}

void SysTick_Handler(void) { LED_TICK(); }
