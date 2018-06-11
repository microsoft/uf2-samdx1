#ifndef TCC_BUZZER_H
#define TCC_BUZZER_H

#if defined(BUZZER_PIN) && defined(BUZZER_TCC)
// We use the TCC to generate a squarte wave on a output,
// useful for driving a piezo or other speaker when there is 
// no led available.

// Default frequency, can be changed by a board config.
// Some Frequencies might sound better than others on different boards.
#ifndef BUZZER_HZ
#define BUZZER_HZ 440
#endif

#define TCC(x) TCC_(x)
#define TCC_(x) TCC ## x

#define BUZZER_APBCMASK(x) BUZZER_APBCMASK_(x)
#define BUZZER_APBCMASK_(x) (PM->APBCMASK.bit.TCC ## x ## _)

// The timings are taken from Adafruit's NeoPixel library

static void init_tcc_buzzer(void);

#ifdef SAMD21
static void init_tcc_buzzer(void) {

    // 1. Enable the TCC bus clock (CLK_TCCx_APB).
    BUZZER_APBCMASK(BUZZER_TCC) = true;

    /* Set GCLK_GEN0 as source for GCLK_ID_SERCOMx_CORE */
    GCLK_CLKCTRL_Type clkctrl = {0};
    clkctrl.bit.GEN = 0; // CPU Clock (48Mhz)
    clkctrl.bit.CLKEN = true;
#if (BUZZER_TCC == 0) || (BUZZER_TCC == 1)
    clkctrl.bit.ID = GCLK_CLKCTRL_ID_TCC0_TCC1_Val;
#elif (BUZZER_TCC == 2)    
    clkctrl.bit.ID = GCLK_CLKCTRL_ID_TCC2_TC3_Val;
#else
    #error UNKNOWN BUZZER_TCC
#endif
    GCLK->CLKCTRL = clkctrl;

    // 2. Software Reset the TCC (Which also disables it)
    TCC_CTRLA_Type tccctrl_a = {0};
    tccctrl_a.bit.SWRST = true;
    TCC(BUZZER_TCC)->CTRLA = tccctrl_a;
    while ((TCC(BUZZER_TCC)->CTRLA.bit.SWRST) == 1) {}
    while ((TCC(BUZZER_TCC)->SYNCBUSY.bit.SWRST) == 1) {}

    // 3. Set waveform Generation Mode.
    // Note: Default Waveform mode is to toggle the output on overflow, 
    // which is exactly what we want.  Nothing to do.

    // 4. Set the period (which defines our frequency)
    // We use a divide by 64 on the CPU Clock, which gives a 
    // range in a 16 bit counter to generate audible frequencies.
    // The Output toggles every overflow, so we need to do that
    // twice as often as the desired frequency.
    TCC_PER_Type tccper = {0};
    tccper.bit.PER = (CPU_FREQUENCY/64/(BUZZER_HZ*2));
    TCC(BUZZER_TCC)->PER = tccper;

    // 5. Set up the IO Port Mux to enable the waveform output.
    PORT_PMUX_Type pmux = {0};
    pmux.reg = PINMUX(BUZZER_PIN);
#if ((BUZZER_PIN & 1) == 0)
    pmux.bit.PMUXE = PORT_PMUX_PMUXE_E_Val;
#else
    pmux.bit.PMUXO = PORT_PMUX_PMUXE_E_Val;
#endif
    PINMUX(BUZZER_PIN) = pmux.reg;

    // 6. Make sure the Buzzer is off, to start.
    PINCFG(BUZZER_PIN) = 0x60;

    // 7. Finally, set CTRLA and Enable the Timer.
    tccctrl_a.bit.SWRST      = false;
    tccctrl_a.bit.ENABLE     = true;
    tccctrl_a.bit.RESOLUTION = TCC_CTRLA_RESOLUTION_NONE_Val; 
    tccctrl_a.bit.PRESCALER  = TCC_CTRLA_PRESCALER_DIV64_Val;
    tccctrl_a.bit.RUNSTDBY   = true;
    tccctrl_a.bit.PRESCSYNC  = TCC_CTRLA_PRESCSYNC_RESYNC_Val;
    tccctrl_a.bit.ALOCK      = false;
    tccctrl_a.bit.CPTEN0     = false;
    tccctrl_a.bit.CPTEN1     = false;
    tccctrl_a.bit.CPTEN2     = false;
    tccctrl_a.bit.CPTEN3     = false;
    TCC(BUZZER_TCC)->CTRLA = tccctrl_a;

    // NOW, all we need to do to toggle the waveform on/off is
    // Enable or Disable the pinmux, as required.
}
#else
    #error TCC INITIALISATION NOT IMPLEMENTED FOR SAMD51
#endif    


#endif

#endif
