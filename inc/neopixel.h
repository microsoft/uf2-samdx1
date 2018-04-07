#ifndef DEVICE_NEOPIXEL_H
#define DEVICE_NEOPIXEL_H

#ifdef BOARD_NEOPIXEL_PIN

// The timings are taken from Adafruit's NeoPixel library

static void neopixel_send_buffer_core(volatile uint32_t *clraddr, uint32_t pinMask,
                                      const uint8_t *ptr, int numBytes) __attribute__((naked));

static void neopixel_send_buffer_core(volatile uint32_t *clraddr, uint32_t pinMask,
                                      const uint8_t *ptr, int numBytes) {
    asm volatile("        push    {r4, r5, r6, lr};"
                 "        add     r3, r2, r3;"
                 "loopLoad:"
                 "        ldrb r5, [r2, #0];" // r5 := *ptr
                 "        add  r2, #1;"       // ptr++
                 "        movs    r4, #128;"  // r4-mask, 0x80
                 "loopBit:"
                 "        str r1, [r0, #4];"                    // set
                 #ifdef SAMD21
                 "        movs r6, #3; d2: sub r6, #1; bne d2;" // delay 3
                 #endif
                 #ifdef SAMD51
                 "        movs r6, #3; d2: subs r6, #1; bne d2;" // delay 3
                 #endif
                 "        tst r4, r5;"                          // mask&r5
                 "        bne skipclr;"
                 "        str r1, [r0, #0];" // clr
                 "skipclr:"
                 #ifdef SAMD21
                 "        movs r6, #6; d0: sub r6, #1; bne d0;" // delay 6
                 #endif
                 #ifdef SAMD51
                 "        movs r6, #6; d0: subs r6, #1; bne d0;" // delay 6
                 #endif
                 "        str r1, [r0, #0];"   // clr (possibly again, doesn't matter)
                 #ifdef SAMD21
                 "        asr     r4, r4, #1;" // mask >>= 1
                 #endif
                 #ifdef SAMD51
                 "        asrs     r4, r4, #1;" // mask >>= 1
                 #endif
                 "        beq     nextbyte;"
                 "        uxtb    r4, r4;"
                 #ifdef SAMD21
                 "        movs r6, #2; d1: sub r6, #1; bne d1;" // delay 2
                 #endif
                 #ifdef SAMD51
                 "        movs r6, #2; d1: subs r6, #1; bne d1;" // delay 2
                 #endif
                 "        b       loopBit;"
                 "nextbyte:"
                 "        cmp r2, r3;"
                 "        bcs stop;"
                 "        b loopLoad;"
                 "stop:"
                 "        pop {r4, r5, r6, pc};"
                 "");
}

// this assumes the pin has been configured correctly
static inline void neopixel_send_buffer(const uint8_t *ptr, int numBytes) {
    uint8_t portNum = BOARD_NEOPIXEL_PIN / 32;
    uint32_t pinMask = 1ul << (BOARD_NEOPIXEL_PIN % 32);

    PINOP(BOARD_NEOPIXEL_PIN, DIRSET);

#if (USB_VID == 0x239a) && (USB_PID == 0x0013)  // Adafruit Metro M0
    // turn off mux too, needed for metro m0
    PORT->Group[BOARD_NEOPIXEL_PIN / 32].PINCFG[BOARD_NEOPIXEL_PIN % 32].reg =
        (uint8_t)(PORT_PINCFG_INEN);
#endif

    PINOP(BOARD_NEOPIXEL_PIN, OUTCLR);
    delay(1);

    volatile uint32_t *clraddr = &PORT->Group[portNum].OUTCLR.reg;

    // equivalent to cpu_irq_is_enabled()
    if (__get_PRIMASK() == 0) {
        __disable_irq();
        neopixel_send_buffer_core(clraddr, pinMask, ptr, numBytes);
        __enable_irq();
    } else {
        neopixel_send_buffer_core(clraddr, pinMask, ptr, numBytes);
    }
}

#endif

#endif
