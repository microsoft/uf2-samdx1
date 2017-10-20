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
 * $Date:        14. May 2014
 * $Revision:    V2.01
 *
 * Driver:       Driver_USBH1_HCI
 * Configured:   via RTE_Device.h configuration file 
 * Project:      USB Host 1 HCI Controller (EHCI) Driver for NXP LPC18xx
 * -----------------------------------------------------------------------------
 * Use the following configuration settings in the middleware component
 * to connect to this driver.
 *
 *   Configuration Setting                Value
 *   ---------------------                -----
 *   Connect to hardware via Driver_USBH# = 1
 *   USB Host controller interface        = EHCI
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 2.01
 *    - Moved register initialization and uninitialization to PowerControl
 *      function and removed from Initialize/Uninitialize functions
 *    - Pin configuration moved to USB_LPC18xx_USB0.c
 *  Version 2.00
 *    - Initial release for USB Host EHCI Driver API v2.0
 *  Version 1.00
 *    - Initial release
 */

#include "LPC18xx.h"
#include "SCU_LPC18xx.h"
#include "USB_LPC18xx.h"

#include "Driver_USBH.h"

#include "RTE_Device.h"
#include "RTE_Components.h"

#if (RTE_USB_USB1 == 0)
#error "USB1 is not enabled in the RTE_Device.h!"
#endif
#if (RTE_USB_USB1_FS_PHY_EN && RTE_USB_USB1_HS_PHY_EN)
#error "Both full-speed and high-speed PHY can not be selected at the same time!"
#endif

/* External Variables */
extern volatile uint32_t USB1_role;

/* External Functions */
extern void USB1_PinsConfigure   (void);
extern void USB1_PinsUnconfigure (void);

static uint32_t usbh_state = 0;


#define ARM_USBH_EHCI_DRIVER_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(2,01)

/* Driver Version */
static const ARM_DRIVER_VERSION usbh_ehci_driver_version = { ARM_USBH_API_VERSION, ARM_USBH_EHCI_DRIVER_VERSION };

/* Driver Capabilities */
static const ARM_USBH_HCI_CAPABILITIES usbh_ehci_driver_capabilities = {
  0x0001, /* Root HUB available Ports Mask   */
};

static ARM_USBH_HCI_Interrupt_t handle_interrupt;


/**
  \fn          ARM_DRIVER_VERSION ARM_USBH_HCI_GetVersion (void)
  \brief       Get USB Host HCI (OHCI/EHCI) driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION ARM_USBH_HCI_GetVersion (void) { return usbh_ehci_driver_version; }

/**
  \fn          ARM_USBH_HCI_CAPABILITIES ARM_USBH_HCI_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_USBH_HCI_CAPABILITIES
*/
static ARM_USBH_HCI_CAPABILITIES ARM_USBH_HCI_GetCapabilities (void) { return usbh_ehci_driver_capabilities; }

/**
  \fn          int32_t ARM_USBH_HCI_Initialize (ARM_USBH_HCI_Interrupt_t *cb_interrupt)
  \brief       Initialize USB Host HCI (OHCI/EHCI) Interface.
  \param[in]   cb_interrupt Pointer to Interrupt Handler Routine
  \return      \ref execution_status
*/
static int32_t ARM_USBH_HCI_Initialize (ARM_USBH_HCI_Interrupt_t cb_interrupt) {

  if (usbh_state & USB_INITIALIZED) return ARM_DRIVER_OK;
  if (usbh_state & USB_POWERED)     return ARM_DRIVER_ERROR;

  handle_interrupt = cb_interrupt;

  USB1_role = ARM_USB_ROLE_HOST;

  // Set pin functions
  USB1_PinsConfigure ();

  usbh_state = USB_INITIALIZED;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t ARM_USBH_HCI_Uninitialize (void)
  \brief       De-initialize USB Host HCI (OHCI/EHCI) Interface.
  \return      \ref execution_status
*/
static int32_t ARM_USBH_HCI_Uninitialize (void) {

  if (!(usbh_state & USB_INITIALIZED)) return ARM_DRIVER_OK;
  if (  usbh_state & USB_POWERED)      return ARM_DRIVER_ERROR;

  USB1_role = ARM_USB_ROLE_NONE;

  // Reset pin functions
  USB1_PinsUnconfigure ();

  usbh_state = 0;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t ARM_USBH_HCI_PowerControl (ARM_POWER_STATE state)
  \brief       Control USB Host HCI (OHCI/EHCI) Interface Power.
  \param[in]   state Power state
  \return      \ref execution_status
*/
static int32_t ARM_USBH_HCI_PowerControl (ARM_POWER_STATE state) {

  if (!(usbh_state & USB_INITIALIZED)) return ARM_DRIVER_ERROR;

  switch (state) {
    case ARM_POWER_OFF:
      if (usbh_state & USB_POWERED) {
        NVIC_DisableIRQ  (USB1_IRQn);     // Disable USB1 interrupt

        usbh_state &= ~USB_POWERED;

#if (!RTE_USB_USB1_HS_PHY_EN)
        LPC_SCU->SFSUSB  = 2;             // Load with reset value
#endif

        LPC_CGU->BASE_USB1_CLK  = 0;

        // Disable USB1 register interface clock
        LPC_CCU1->CLK_M3_USB1_CFG &=~1;
        while (LPC_CCU1->CLK_M3_USB1_STAT & 1);     // Wait clock disable
      }
      break;

    case ARM_POWER_FULL:
      if (!(usbh_state & USB_POWERED)) {
        // Enable USB1 register interface clock
        LPC_CGU->BASE_USB1_CLK  = (0x01 << 11) |    // Autoblock En
                                  (0x0C << 24) ;    // Clock source: IDIVA

        LPC_CCU1->CLK_M3_USB1_CFG |= 1;
        while (!(LPC_CCU1->CLK_M3_USB1_STAT & 1));  // Wait clock enable

#if (RTE_USB_USB1_HS_PHY_EN)
        LPC_USB1->PORTSC1_H &= ~(3U << 30);         // Reset PTS field
        LPC_USB1->PORTSC1_H |=  (2U << 30);         // Activate ULPI
#else
        LPC_SCU->SFSUSB = 0x17;           // USB_AIM=1, USB_ESEA=1,
                                          // USB_EPD = 1, USB_EPWR=1
#endif

        usbh_state |=  USB_POWERED;

        NVIC_ClearPendingIRQ (USB1_IRQn); // Clear pending USB1 interrupt
        NVIC_EnableIRQ       (USB1_IRQn); // Enable USB1 interrupt
      }
      break;

    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t ARM_USBH_HCI_PortVbusOnOff (uint8_t port, bool vbus)
  \brief       USB Host HCI (OHCI/EHCI) Root HUB Port VBUS on/off.
  \param[in]   port  Root HUB Port Number
  \param[in]   vbus
                - \b false VBUS off
                - \b true  VBUS on
  \return      \ref execution_status
*/
static int32_t ARM_USBH_HCI_PortVbusOnOff (uint8_t port, bool power) {
  /* No GPIO pins used for VBUS control it is controlled by EHCI Controller */
  return ARM_DRIVER_OK;
}

/**
  \fn          void USBH1_IRQ (void)
  \brief       USB0 Interrupt handling routine.
*/
void USBH1_IRQ (void) {
  handle_interrupt();
}

ARM_DRIVER_USBH_HCI Driver_USBH1_HCI = {
  ARM_USBH_HCI_GetVersion,
  ARM_USBH_HCI_GetCapabilities,
  ARM_USBH_HCI_Initialize,
  ARM_USBH_HCI_Uninitialize,
  ARM_USBH_HCI_PowerControl,
  ARM_USBH_HCI_PortVbusOnOff
};
