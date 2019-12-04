#include "uf2.h"
#define SYSCTRL_FUSES_OSC32K_CAL_ADDR   (NVMCTRL_OTP5 + 4)
#define SYSCTRL_FUSES_OSC32K_CAL_Pos   6
#define 	SYSCTRL_FUSES_OSC32K_ADDR   SYSCTRL_FUSES_OSC32K_CAL_ADDR
#define 	SYSCTRL_FUSES_OSC32K_Pos   SYSCTRL_FUSES_OSC32K_CAL_Pos
#define 	SYSCTRL_FUSES_OSC32K_Msk   (0x7Fu << SYSCTRL_FUSES_OSC32K_Pos)

volatile bool g_interrupt_enabled = true;

static void gclk_sync(void) {
    while (OSCCTRL->DPLLSYNCBUSY.reg & OSCCTRL_DPLLSYNCBUSY_MASK)
        ;
}

static void dfll_sync(void) {
    while ((OSCCTRL->STATUS.reg & OSCCTRL_STATUS_DFLLRDY) == 0)
        ;
}

#define NVM_SW_CALIB_DFLL48M_COARSE_VAL   26


void system_init(void) {

  //NVMCTRL->CTRLB.bit.RWS = 1;

  NVMCTRL->CTRLB.bit.MANW = 1;

  NVMCTRL->CTRLB.reg |= NVMCTRL_CTRLB_RWS_DUAL ; // two wait states
  
  /* Turn on the digital interface clock */
  MCLK->APBAMASK.reg |= MCLK_APBAMASK_GCLK ;

  PM->INTFLAG.reg = PM_INTFLAG_PLRDY; //clear flag
  PM->PLCFG.reg |= PM_PLCFG_PLSEL_PL2 ;	// must set to highest performance level
  while ( (PM->INTFLAG.reg & PM_INTFLAG_PLRDY) != PM_INTFLAG_PLRDY );
  PM->INTFLAG.reg = PM_INTFLAG_PLRDY; //clear flag

  GCLK->CTRLA.reg = GCLK_CTRLA_SWRST ;

  while ( (GCLK->CTRLA.reg & GCLK_CTRLA_SWRST) && (GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_MASK) );	/* Wait for reset to complete */

#if defined(CRYSTALLESS)
  
  OSCCTRL->DFLLCTRL.bit.ONDEMAND = 0 ;
  dfll_sync();

  uint32_t coarse =( *((uint32_t *)(NVMCTRL_OTP5)
		       + (NVM_SW_CALIB_DFLL48M_COARSE_VAL / 32))
		     >> (NVM_SW_CALIB_DFLL48M_COARSE_VAL % 32))
    & ((1 << 6) - 1);
  if (coarse == 0x3f) {
    coarse = 0x1f;
  }

  OSCCTRL->DFLLVAL.reg = OSCCTRL_DFLLVAL_COARSE(coarse) | OSCCTRL_DFLLVAL_FINE(512);

  OSCCTRL->DFLLMUL.reg = OSCCTRL_DFLLMUL_CSTEP( 0x1f / 4 ) | // Coarse step is 31, half of the max value
                         OSCCTRL_DFLLMUL_FSTEP( 10 ) |
                         OSCCTRL_DFLLMUL_MUL(48000);
 
  OSCCTRL->DFLLCTRL.reg = OSCCTRL_DFLLCTRL_USBCRM | /* USB correction */
                          OSCCTRL_DFLLCTRL_MODE |  /* Closed loop mode */
                          OSCCTRL_DFLLCTRL_CCDIS;
  dfll_sync();
  
  /* Enable the DFLL */
  OSCCTRL->DFLLCTRL.reg |= OSCCTRL_DFLLCTRL_ENABLE ;
  dfll_sync();

  /* Switch Generic Clock Generator 0 to DFLL48M. CPU will run at 48MHz */
  GCLK->GENCTRL[0u].reg = GCLK_GENCTRL_DIV(1) | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN;
  gclk_sync();

  /* Note that after reset, the L21 starts with the OSC16M set to 4MHz, NOT the DFLL@48MHz as stated in some documentation. */
  /* Modify FSEL value of OSC16M to have 8MHz */
  OSCCTRL->OSC16MCTRL.bit.FSEL = OSCCTRL_OSC16MCTRL_FSEL_8_Val;
  gclk_sync();

#else

    OSCCTRL->XOSC32K.reg =
        SYSCTRL_XOSC32K_STARTUP(4) | SYSCTRL_XOSC32K_XTALEN | SYSCTRL_XOSC32K_EN32K | OSC32KCTRL_XOSC32K_EN1K;
    OSCCTRL->XOSC32K.bit.ENABLE = 1;
    while ((OSC32KCTRL->STATUS.reg & OSC32KCTRL_STATUS_XOSC32KRDY) == 0) /* Wait for oscillator stabilization */
        ;

    //GCLK->GENDIV.reg = GCLK_GENDIV_ID(1);
    //gclk_sync();

    GCLK->GENCTRL[0u].reg = GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_GENEN;
    gclk_sync();

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(0) | GCLK_CLKCTRL_GEN_GCLK1 | GCLK_CLKCTRL_CLKEN;
    gclk_sync();

    OSCCTRL->DFLLCTRL.bit.ONDEMAND = 0;
    dfll_sync();

    OSCCTRL->DFLLMUL.reg = OSCCTRL_DFLLMUL_CSTEP(31) | SYSCTRL_DFLLMUL_FSTEP(511) |
                           OSCCTRL_DFLLMUL_MUL((CPU_FREQUENCY / (32 * 1024)));
    dfll_sync();

    OSCCTRL->DFLLCTRL.reg |=
        OSCCTRL_DFLLCTRL_MODE | OSCCTRL_DFLLCTRL_WAITLOCK | OSCCTRL_DFLLCTRL_QLDIS;
    dfll_sync();

    OSCCTRL->DFLLCTRL.reg |= OSCCTRL_DFLLCTRL_ENABLE;
    dfll_sync();

#endif

    // Add GCLK_GENCTRL_OE below to output GCLK0 on the SWCLK pin.
    GCLK->GENCTRL[3u].reg = (GCLK_GENCTRL_DIV(1) | GCLK_GENCTRL_SRC_OSC16M | GCLK_GENCTRL_GENEN);
    gclk_sync();

    MCLK->CPUDIV.reg  = MCLK_CPUDIV_CPUDIV_DIV1 ;

    SysTick_Config(1000);

    // Uncomment these two lines to output GCLK0 on the SWCLK pin.
    // PORT->Group[0].PINCFG[30].bit.PMUXEN = 1;
    // Set the port mux mask for odd processor pin numbers, PA30 = 30 is even number, PMUXE = PMUX Even
    // PORT->Group[0].PMUX[30 / 2].reg |= PORT_PMUX_PMUXE_H;
}

void SysTick_Handler(void) { LED_TICK(); }
