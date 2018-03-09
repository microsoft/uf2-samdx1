/* -----------------------------------------------------------------------------
 * Copyright (c) 2013-2014 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        26. May 2014
 * $Revision:    V1.00
 *
 * Project:      USB common (Device and Host) module for NXP LPC18xx
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 1.00
 *    - Initial release
 */

#include "LPC18xx.h"
#include "SCU_LPC18xx.h"

#include "Driver_USB.h"

#include "RTE_Device.h"
#include "RTE_Components.h"

volatile uint32_t USB0_role = ARM_USB_ROLE_NONE;

__weak void USBH0_IRQ (void) {};
__weak void USBD0_IRQ (void) {};

/**
  \fn          void USB0_IRQHandler (void)
  \brief       USB Interrupt Routine (IRQ).
*/
void USB0_IRQHandler (void) {
  switch (USB0_role) {
    case ARM_USB_ROLE_HOST:
      USBH0_IRQ ();
      break;
    case ARM_USB_ROLE_DEVICE:
      USBD0_IRQ ();
      break;
    case ARM_USB_ROLE_NONE:
      break;
  }
}

/**
  \fn          void USB0_PinsConfigure (void)
  \brief       Configure USB pins
*/
void USB0_PinsConfigure (void) {

  // Common (Device and Host) Pins
#if (RTE_USB0_IND0_PIN_EN)
  SCU_PinConfigure(RTE_USB0_IND0_PORT, RTE_USB0_IND0_BIT, RTE_USB0_IND0_FUNC);
#endif
#if (RTE_USB0_IND1_PIN_EN)
  SCU_PinConfigure(RTE_USB0_IND1_PORT, RTE_USB0_IND1_BIT, RTE_USB0_IND1_FUNC);
#endif

  // Host Pins
  if (USB0_role == ARM_USB_ROLE_HOST) {
#if (RTE_USB0_PPWR_PIN_EN)
    SCU_PinConfigure(RTE_USB0_PPWR_PORT,      RTE_USB0_PPWR_BIT,      RTE_USB0_PPWR_FUNC);
#endif
#if (RTE_USB0_PWR_FAULT_PIN_EN)
    SCU_PinConfigure(RTE_USB0_PWR_FAULT_PORT, RTE_USB0_PWR_FAULT_BIT, RTE_USB0_PWR_FAULT_FUNC);
#endif
  }
}

/**
  \fn          void USB0_PinsUnconfigure (void)
  \brief       Unconfigure USB pins
*/
void USB0_PinsUnconfigure (void) {

  // Common (Device and Host) Pins
#if (RTE_USB0_IND0_PIN_EN)
  SCU_PinConfigure(RTE_USB0_IND0_PORT, RTE_USB0_IND0_BIT, 0);
#endif
#if (RTE_USB0_IND1_PIN_EN)
  SCU_PinConfigure(RTE_USB0_IND1_PORT, RTE_USB0_IND1_BIT, 0);
#endif

  // Host Pins
  if (USB0_role == ARM_USB_ROLE_HOST) {
#if (RTE_USB0_PPWR_PIN_EN)
    SCU_PinConfigure(RTE_USB0_PPWR_PORT,      RTE_USB0_PPWR_BIT,      0);
#endif
#if (RTE_USB0_PWR_FAULT_PIN_EN)
    SCU_PinConfigure(RTE_USB0_PWR_FAULT_PORT, RTE_USB0_PWR_FAULT_BIT, 0);
#endif
  }
}
