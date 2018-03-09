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
 * $Revision:    V2.02
 *
 * Driver:       Driver_USBD1
 * Configured:   via RTE_Device.h configuration file 
 * Project:      USB Device Driver for NXP LPC18xx
 * -----------------------------------------------------------------------------
 * Use the following configuration settings in the middleware component
 * to connect to this driver.
 *
 *   Configuration Setting                Value
 *   ---------------------                -----
 *   Connect to hardware via Driver_USBD# = 1
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 2.02
 *    - Repaired return value in USBD_PowerControl function.
 *  Version 2.01
 *    - Added USB_LPC18xx_USB1.h with register bit definitions
 *    - Pin configuration moved to USB_LPC18xx.c
 *  Version 2.00
 *    - Updated to 2.00 API
 *  Version 1.03
 *    - Re-implementation of the driver
 *  Version 1.02
 *    - Updated USB1 pin configurations
 *  Version 1.01
 *    - Based on API V1.10 (namespace prefix ARM_ added)
 *  Version 1.00
 *    - Initial release
 */
#include <string.h>
#include "LPC18xx.h"

#define   USB_ENDPT_MSK  (0x0F)
#include "USB_LPC18xx.h"
#include "SCU_LPC18xx.h"

#include "Driver_USBD.h"

#include "RTE_Device.h"
#include "RTE_Components.h"

#if (RTE_USB_USB1 == 0)
#error "USB1 is not enabled in the RTE_Device.h!"
#endif

/* USBD Driver ****************************************************************/

#define ARM_USBD_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(2,2) /* USBD driver version */

/* Driver Version */
static const ARM_DRIVER_VERSION usbd_driver_version = { ARM_USBD_API_VERSION, ARM_USBD_DRV_VERSION };


// Driver Capabilities
static const ARM_USBD_CAPABILITIES usbd_driver_capabilities = {
  0,  // vbus_detection
  0,  // event_vbus_on
  0,  // event_vbus_off
};

// Definitions
#define USBD1_EP_MASK  ((RTE_USB_USB1_DEV_EP << 1) | 1)

// Number of Endpoints
#if    (USBD1_EP_MASK & 0x08)
#define USBD_EP_NUM      3
#elif  (USBD1_EP_MASK & 0x04)
#define USBD_EP_NUM      2
#elif  (USBD1_EP_MASK & 0x02)
#define USBD_EP_NUM      1
#else
#define USBD_EP_NUM      0
#endif


static EPQH __align(2048) EPQHx[(USBD_EP_NUM + 1) * 2];
static dTD  __align(32  ) dTDx[ (USBD_EP_NUM + 1) * 2];

#define LPC_USBx            LPC_USB1
#define ENDPTCTRL(EPNum)  *(volatile uint32_t *)((uint32_t)(&LPC_USBx->ENDPTCTRL0) + 4 * EPNum)
#define EP_NUM(ep_addr)    (ep_addr & ARM_USB_ENDPOINT_NUMBER_MASK)
#define EP_VAL(ep_addr)    (ep_addr & ARM_USB_ENDPOINT_DIRECTION_MASK) ? 16 : 0;
#define EP_IDX(ep_addr)    (ep_addr & ARM_USB_ENDPOINT_DIRECTION_MASK) ? (EP_NUM(ep_addr) *2 + 1) : (EP_NUM(ep_addr) * 2)

// Static Variables
static ARM_USBD_SignalDeviceEvent_t   cbDeviceEvent;
static ARM_USBD_SignalEndpointEvent_t cbEndpointEvent;

         static uint8_t  setup_buf[8];
volatile static uint8_t  setup_flag = 0;

static uint32_t usbd_flags = 0;
static ARM_USBD_STATE usbd_state = {0, 0, 0};

// External Variables
extern volatile uint32_t USB1_role;

// External Functions
extern void USB1_PinsConfigure   (void);
extern void USB1_PinsUnconfigure (void);

// Local Functions

/**
  \fn          void USBD_Reset (void)
  \brief       Called after usbd reset interrupt to reset configuration
*/
static void USBD_Reset (void) {
  uint32_t i;

  for (i = 1; i < USBD_EP_NUM + 1; i++) {
    ENDPTCTRL(i) &= ~(USB_ENDPTCTRL_RXE | USB_ENDPTCTRL_TXE);
  }

  // Clear interrupts
  LPC_USBx->ENDPTNAK       = 0xFFFFFFFF;
  LPC_USBx->ENDPTNAKEN     = 0;
  LPC_USBx->USBSTS_D       = 0xFFFFFFFF;
  LPC_USBx->ENDPTSETUPSTAT = LPC_USBx->ENDPTSETUPSTAT;
  LPC_USBx->ENDPTCOMPLETE  = LPC_USBx->ENDPTCOMPLETE;

  while (LPC_USBx->ENDPTPRIME);

  // Clear all Primed buffers
  LPC_USBx->ENDPTFLUSH = 0xFFFFFFFF;
  while (LPC_USBx->ENDPTFLUSH);

  // Interrupt threshold control: no threshold
  LPC_USBx->USBCMD_D &= ~(USB_USBCMD_D_ITC(0xFF));

  // Clear endpoint queue heads and endpoint transfer descriptors
  memset(EPQHx, 0, sizeof(EPQH)*((USBD_EP_NUM + 1) * 2));
  memset(dTDx,  0, sizeof(dTD) *((USBD_EP_NUM + 1) * 2));

  // Set start of endpoint list address
  LPC_USBx->ENDPOINTLISTADDR = (uint32_t)EPQHx;

  // Setup lockouts off
  LPC_USBx->USBMODE_D |= USB_USBMODE_SLOM;
}

/**
  \fn          bool USBD_EndpointFlush (uint8_t ep_addr)
  \brief       Flush Endpoint
*/
static bool USBD_EndpointFlush (uint8_t ep_addr) {
  uint32_t val, num;

  num =  EP_NUM(ep_addr);
  val =  EP_VAL(ep_addr);

  // Flush endpoint
  LPC_USBx->ENDPTFLUSH =        (1 << (num + val));
  while (LPC_USBx->ENDPTFLUSH & (1 << (num + val)));

  return true;
}

/**
  \fn          bool USBD_ReadSetup (void)
  \brief       Read Setup packet to buffer
*/
static bool USBD_ReadSetup (void) {

  do {
    // Setup trip wire
    LPC_USBx->USBCMD_D |= USB_USBCMD_D_SUTW;

    // Copy Setup packet to buffer
    *((__packed uint32_t*) setup_buf)      = EPQHx[0].setup[0];
    *((__packed uint32_t*)(setup_buf + 4)) = EPQHx[0].setup[1];
  } while ((LPC_USBx->USBCMD_D & LPC_USBx->USBCMD_D) == 0);

  // Clear Setup trip wire
  LPC_USBx->USBCMD_D &= ~USB_USBCMD_D_SUTW;

  // Clear Setup bit
  LPC_USBx->ENDPTSETUPSTAT = 1;

  return true;
}

/**
\fn          void USBD_EndpointPrime (uint8_t ep_addr, uint8_t *buf, uint32_t len)
\brief       Prime USB Endpoint
\param[in]   ep_addr specifies Endpoint Address
              ep_addr.0..3: Address
              ep_addr.7:    Direction
\param[out]  buf specifies buffer
\param[in]   len specifies buffer length
*/
static void USBD_EndpointPrime (uint8_t ep_addr, uint8_t *buf, uint32_t len) {
  uint32_t val, num;

  num =  EP_NUM(ep_addr);
  val =  EP_VAL(ep_addr);

  // Set buffer addresses
  dTDx[EP_IDX(ep_addr)].buf[0]  =  (uint32_t)(buf         );
  dTDx[EP_IDX(ep_addr)].buf[1]  =  (uint32_t)(buf + 0x1000);
  dTDx[EP_IDX(ep_addr)].buf[2]  =  (uint32_t)(buf + 0x2000);
  dTDx[EP_IDX(ep_addr)].buf[3]  =  (uint32_t)(buf + 0x3000);
  dTDx[EP_IDX(ep_addr)].buf[4]  =  (uint32_t)(buf + 0x4000);

  // Driver does not support linked endpoint descriptors
  // Next link pointer is not valid
  dTDx[EP_IDX(ep_addr)].next_dTD  = 1;

  // Set number of transactions for isochronous endpoint
  if (EPQHx[EP_IDX(ep_addr)].ep.type == ARM_USB_ENDPOINT_ISOCHRONOUS) {
    if (EPQHx[EP_IDX(ep_addr)].ep.maxPacketSize <= len) {
     // MultO = 1
      dTDx[EP_IDX(ep_addr)].dTD_token = USB_bTD_TOKEN_MULTO(1);
    }
    else if ((EPQHx[EP_IDX(ep_addr)].ep.maxPacketSize * 2) <= len) {
      // MultO = 2
      dTDx[EP_IDX(ep_addr)].dTD_token = USB_bTD_TOKEN_MULTO(2);
    }
    else {
      // MultO = 3
      dTDx[EP_IDX(ep_addr)].dTD_token = USB_bTD_TOKEN_MULTO(3);
    }
  }
  else {
    dTDx[EP_IDX(ep_addr)].dTD_token = 0;
  }

  // Maximum transfer length is 16k
  if (len > 0x4000) len = 0x4000;

  // IN Endpoints
  if (ep_addr & ARM_USB_ENDPOINT_DIRECTION_MASK) {
    EPQHx[EP_IDX(ep_addr)].ep.bufferIndex += len;
  }

  // Configure endpoint transfer descriptor
  dTDx[EP_IDX(ep_addr)].dTD_token  |= USB_bTD_TOKEN_TOTAL_BYTES(len) |  // Bytes to transfer
                                      USB_bTD_TOKEN_IOC              |  // Interrupt on complete
                                      USB_bTD_TOKEN_STATUS_ACTIVE;

  // Save transfer descriptor address to endpoint queue head
  EPQHx[EP_IDX(ep_addr)].next_dTD   = (uint32_t)(&dTDx[EP_IDX(ep_addr)]);

  // Clear endpoint queue head status
  EPQHx[EP_IDX(ep_addr)].dTD_token &= ~(USB_bTD_TOKEN_STATUS_ACTIVE       |
                                        USB_bTD_TOKEN_STATUS_HALTED       |
                                        USB_bTD_TOKEN_STATUS_BUFFER_ERROR |
                                        USB_bTD_TOKEN_STATUS_TRAN_ERROR);

  // Prime endpoint
  LPC_USBx->ENDPTPRIME = (1 << (val + num));
}


// USB Device Driver Functions

/**
  \fn          ARM_DRIVER_VERSION USBD_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION USBD_GetVersion (void) { return usbd_driver_version; }

/**
  \fn          ARM_USBD_CAPABILITIES USBD_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_USBD_CAPABILITIES
*/
static ARM_USBD_CAPABILITIES USBD_GetCapabilities (void) { return usbd_driver_capabilities; }

/**
  \fn          int32_t USBD_Initialize (ARM_USBD_SignalDeviceEvent_t   cb_device_event,
                                        ARM_USBD_SignalEndpointEvent_t cb_endpoint_event)
  \brief       Initialize USB Device Interface.
  \param[in]   cb_device_event    Pointer to \ref ARM_USBD_SignalDeviceEvent
  \param[in]   cb_endpoint_event  Pointer to \ref ARM_USBD_SignalEndpointEvent
  \return      \ref execution_status
*/
static int32_t USBD_Initialize (ARM_USBD_SignalDeviceEvent_t   cb_device_event,
                                ARM_USBD_SignalEndpointEvent_t cb_endpoint_event) {

  if (usbd_flags & USB_POWERED) {
    // Device is powered - could not be re-initialized
    return ARM_DRIVER_ERROR;
  }

  if (usbd_flags & USB_INITIALIZED){
    // Driver is already initialized
    return ARM_DRIVER_OK;
  }

  // Initialize USBD Run-time Resources
  cbDeviceEvent   = cb_device_event;
  cbEndpointEvent = cb_endpoint_event;

  USB1_role = ARM_USB_ROLE_DEVICE;

  // Set pin functions
  USB1_PinsConfigure ();

  usbd_flags = USB_INITIALIZED;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_Uninitialize (void)
  \brief       De-initialize USB Device Interface.
  \return      \ref execution_status
*/
static int32_t USBD_Uninitialize (void) {

  if (usbd_flags & USB_POWERED) {
    // Device is powered - could not be uninitialized
    return ARM_DRIVER_ERROR;
  }

  if ((usbd_flags & USB_INITIALIZED) == 0){
    // Driver not initialized
    return ARM_DRIVER_OK;
  }

  usbd_flags = 0;

  USB1_role = ARM_USB_ROLE_NONE;

  // Reset pin functions
  USB1_PinsUnconfigure ();

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_PowerControl (ARM_POWER_STATE state)
  \brief       Control USB Device Interface Power.
  \param[in]   state  Power state
  \return      \ref execution_status
*/
static int32_t USBD_PowerControl (ARM_POWER_STATE state) {

  if ((usbd_flags & USB_INITIALIZED) == 0){
    // Driver is not initialized
    return ARM_DRIVER_ERROR;
  }

  switch (state) {
    case ARM_POWER_OFF:
      if ((usbd_flags & USB_POWERED) == 0)
        return ARM_DRIVER_OK;

      // Disable interrupts
      NVIC_DisableIRQ(USB1_IRQn);

      // Disable USB1 base clock
      LPC_CCU1->CLK_USB1_CFG &= ~1;
      while (LPC_CCU1->CLK_USB1_STAT & 1);

      // Disable USB1 register interface clock
      LPC_CCU1->CLK_M3_USB1_CFG &= ~1;
      while (LPC_CCU1->CLK_M3_USB1_STAT & 1);

      usbd_flags = USB_INITIALIZED;
      break;

    case ARM_POWER_LOW:
      return ARM_DRIVER_ERROR_UNSUPPORTED;

    case ARM_POWER_FULL:
      if (usbd_flags & USB_POWERED)
        return ARM_DRIVER_OK;

      // BASE_USB1_CLK
      LPC_CGU->BASE_USB1_CLK   = (0x01 << 11) |   // Auto-block Enable
                                 (0x0C << 24) ;   // Clock source: IDIVA
      // Enable USB1 register interface clock
      LPC_CCU1->CLK_M3_USB1_CFG |= 1;
      while ((LPC_CCU1->CLK_M3_USB1_STAT & 1) == 0);

      // Enable USB1 base clock
      LPC_CCU1->CLK_USB1_CFG |= 1;
      while ((LPC_CCU1->CLK_USB1_STAT & 1) == 0);

      // USB reset
      LPC_USBx->USBCMD_D = USB_USBCMD_D_RST;
      while (LPC_USBx->USBCMD_D & (USB_USBCMD_D_RS | USB_USBCMD_D_RST));

      // Force device mode and set Setup lockouts off
      LPC_USBx->USBMODE_D  = USB_USBMODE_CM1_0(2) |
                             USB_USBMODE_SLOM;

      // Clear transceiver selection
      LPC_USBx->PORTSC1_D &= ~(USB_PORTSC1_D_PTS_MSK | USB_PORTSC1_D_PFSC);
#if (RTE_USB_USB1_HS_PHY_EN)
      // ULPI Selected
      LPC_USBx->PORTSC1_D |= USB_PORTSC1_D_PTS(2);
#else
      // Serial/1.1 PHY selected and Full speed forced
      LPC_USBx->PORTSC1_D |= USB_PORTSC1_D_PTS(3UL) |
                             USB_PORTSC1_D_PFSC;
#endif

      // Enable interrupts
      LPC_USBx->USBINTR_D  = USB_USBINTR_D_UE  |  // USB interrupt enable
                             USB_USBINTR_D_PCE |  // Port change detect interrupt enable
                             USB_USBINTR_D_SLE |  // Suspend interrupt enable
                             USB_USBINTR_D_URE ;  // Reset interrupt enable

      SCU_USB1_PinConfigure (SCU_USB1_PIN_CFG_ESEA |
                             SCU_USB1_PIN_CFG_EPWR);

      usbd_flags = USB_INITIALIZED | USB_POWERED;

      NVIC_ClearPendingIRQ(USB1_IRQn);
      NVIC_EnableIRQ(USB1_IRQn);
      break;

    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_DeviceConnect (void)
  \brief       Connect USB Device.
  \return      \ref execution_status
*/
static int32_t USBD_DeviceConnect (void) {

  if ((usbd_flags & USB_POWERED) == 0) {
    // Device not powered
    return ARM_DRIVER_ERROR;
  }

  if (usbd_flags & USB_CONNECTED) {
    // Device is already connected
    return ARM_DRIVER_OK;
  }

  // Attach the device
  LPC_USBx->USBCMD_D |= USB_USBCMD_D_RS;

  usbd_flags |= USB_CONNECTED;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_DeviceDisconnect (void)
  \brief       Disconnect USB Device.
  \return      \ref execution_status
*/
static int32_t USBD_DeviceDisconnect (void) {

  if ((usbd_flags & USB_POWERED) == 0) {
    // Device not powered
    return ARM_DRIVER_ERROR;
  }

  if ((usbd_flags & USB_CONNECTED) == 0) {
    // Device is already disconnected
    return ARM_DRIVER_OK;
  }

  // Detach the device
  LPC_USBx->USBCMD_D &= ~USB_USBCMD_D_RS;

  usbd_state.active   =  false;
  usbd_flags         &= ~USB_CONNECTED;

#if (RTE_USB1_IND0_PIN_EN)
    // Clear indicator LED0
    LPC_USBx->PORTSC1_D &= ~USB_PORTSC1_D_PIC1_0(1);
#endif

  return ARM_DRIVER_OK;
}

/**
  \fn          ARM_USBD_STATE USBD_DeviceGetState (void)
  \brief       Get current USB Device State.
  \return      Device State \ref ARM_USBD_STATE
*/
static ARM_USBD_STATE USBD_DeviceGetState (void) {
  return usbd_state;
}

/**
  \fn          int32_t USBD_DeviceRemoteWakeup (void)
  \brief       Trigger USB Remote Wakeup.
  \return      \ref execution_status
*/
static int32_t USBD_DeviceRemoteWakeup (void) {

  if ((usbd_flags & USB_CONNECTED) == 0) {
    // Device is not connected
    return ARM_DRIVER_ERROR;
  }

  // Enable PHY clock
  LPC_USBx->PORTSC1_D &= ~USB_PORTSC1_D_PHCD;

  // Force port resume
  LPC_USBx->PORTSC1_D |=  USB_PORTSC1_D_FPR;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_DeviceSetAddress (uint8_t dev_addr)
  \brief       Set USB Device Address.
  \param[in]   dev_addr  Device Address
  \return      \ref execution_status
*/
static int32_t USBD_DeviceSetAddress (uint8_t dev_addr) {

  if ((usbd_flags & USB_CONNECTED) == 0) {
    // Device is not connected
    return ARM_DRIVER_ERROR;
  }

  LPC_USBx->DEVICEADDR  = (dev_addr << USB_DEVICEADDR_USBADR_POS) &
                           USB_DEVICEADDR_USBADR_MSK;
  LPC_USBx->DEVICEADDR |= USB_DEVICEADDR_USBADRA;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_ReadSetupPacket (uint8_t *setup)
  \brief       Read setup packet received over Control Endpoint.
  \param[out]  setup  Pointer to buffer for setup packet
  \return      \ref execution_status
*/
static int32_t USBD_ReadSetupPacket (uint8_t *setup) {

  if ((usbd_flags & USB_CONNECTED) == 0) {
    // Device is not connected
    return ARM_DRIVER_ERROR;
  }

  if (setup_flag == 0) {
    // No setup packet waiting
    return ARM_DRIVER_ERROR;
  }

  setup_flag = 0;
  memcpy(setup, setup_buf, 8);

  if (setup_flag) {
    // Interrupted with new setup packet
    return ARM_DRIVER_ERROR;
  }

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_EndpointConfigure (uint8_t  ep_addr,
                                               uint8_t  ep_type,
                                               uint16_t ep_max_packet_size)
  \brief       Configure USB Endpoint.
  \param[in]   ep_addr  Endpoint Address
                - ep_addr.0..3: Address
                - ep_addr.7:    Direction
  \param[in]   ep_type  Endpoint Type (ARM_USB_ENDPOINT_xxx)
  \param[in]   ep_max_packet_size Endpoint Maximum Packet Size
  \return      \ref execution_status
*/
static int32_t USBD_EndpointConfigure (uint8_t ep_addr,
                                       uint8_t  ep_type,
                                       uint16_t ep_max_packet_size) {
  uint32_t val, num;
  uint16_t sz;

  if ((usbd_flags & USB_CONNECTED) == 0) {
    // Device is not connected
    return ARM_DRIVER_ERROR;
  }

  if (EPQHx[EP_IDX(ep_addr)].ep.flags & USBD_EP_FLAG_BUSY) {
    // Endpoint is busy
    return ARM_DRIVER_ERROR_BUSY;
  }

  if (EPQHx[EP_IDX(ep_addr)].ep.flags & USBD_EP_FLAG_CONFIGURED) {
    // Endpoint is already configured
    return ARM_DRIVER_OK;
  }

  num =  EP_NUM(ep_addr);
  val =  EP_VAL(ep_addr);
  sz =   ep_max_packet_size & ARM_USB_ENDPOINT_MAX_PACKET_SIZE_MASK;

  // Check if endpoint number is valid
  if ((USBD1_EP_MASK & (1 << num)) == 0) return ARM_DRIVER_ERROR;

  // Set Endpoint queue head
  EPQHx[EP_IDX(ep_addr)].ep.buffer         =  NULL;
  EPQHx[EP_IDX(ep_addr)].ep.dataSize       =  0;
  EPQHx[EP_IDX(ep_addr)].ep.maxPacketSize  =  sz;
  EPQHx[EP_IDX(ep_addr)].ep.type           =  ep_type;
  EPQHx[EP_IDX(ep_addr)].cap               =  USB_EPQH_CAP_MAX_PACKET_LEN(sz) |
                                              USB_EPQH_CAP_ZLT;
  if (ep_addr == 0)
    EPQHx[EP_IDX(ep_addr)].cap            |=  USB_EPQH_CAP_IOS;

  EPQHx[EP_IDX(ep_addr)].next_dTD          =  1;

  EPQHx[EP_IDX(ep_addr)].dTD_token         =  0;

  if (USBD_EndpointFlush(ep_addr) == false) return ARM_DRIVER_ERROR;

  // Reset endpoint control settings
  ENDPTCTRL(num) &= ~((USB_ENDPTCTRL_RXS     |
                       USB_ENDPTCTRL_RXT_MSK |
                       USB_ENDPTCTRL_RXI     |
                       USB_ENDPTCTRL_RXR     |
                       USB_ENDPTCTRL_RXE) << val);

  // Set endpoint control
  ENDPTCTRL(num) |=  ((USB_ENDPTCTRL_RXT(ep_type) |     // Endpoint type
                       USB_ENDPTCTRL_RXR          |     // Data toggle reset
                       USB_ENDPTCTRL_RXE) << val);      // Endpoint enable

  // Set Endpoint configured flag
  EPQHx[EP_IDX(ep_addr)].ep.flags |= USBD_EP_FLAG_CONFIGURED;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_EndpointUnconfigure (uint8_t ep_addr)
  \brief       Unconfigure USB Endpoint.
  \param[in]   ep_addr  Endpoint Address
                - ep_addr.0..3: Address
                - ep_addr.7:    Direction
  \return      \ref execution_status
*/
static int32_t USBD_EndpointUnconfigure (uint8_t ep_addr) {
  uint32_t val, num;

  if ((usbd_flags & USB_CONNECTED) == 0) {
    // Device is not connected
    return ARM_DRIVER_ERROR;
  }

  if (EPQHx[EP_IDX(ep_addr)].ep.flags & USBD_EP_FLAG_BUSY) {
    // Endpoint is busy
    return ARM_DRIVER_ERROR_BUSY;
  }

  if ((EPQHx[EP_IDX(ep_addr)].ep.flags & USBD_EP_FLAG_CONFIGURED) == 0) {
    // Endpoint is not configured
    return ARM_DRIVER_OK;
  }

  num =  EP_NUM(ep_addr);
  val =  EP_VAL(ep_addr);

  // Reset endpoint control settings
  ENDPTCTRL(num) &= ~((USB_ENDPTCTRL_RXS     |
                       USB_ENDPTCTRL_RXT_MSK |
                       USB_ENDPTCTRL_RXI     |
                       USB_ENDPTCTRL_RXR     |
                       USB_ENDPTCTRL_RXE) << val);

  // Data toggle reset
  ENDPTCTRL(num) |=  (USB_ENDPTCTRL_RXR << val);

  // Clear endpoint queue head and endpoint transfer descriptor
  memset(&(EPQHx[EP_IDX(ep_addr)]), 0, sizeof(EPQH));
  memset(&(dTDx[EP_IDX(ep_addr)]),  0, sizeof(dTD));

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_EndpointStall (uint8_t ep_addr, bool stall)
  \brief       Set/Clear Stall for USB Endpoint.
  \param[in]   ep_addr  Endpoint Address
                - ep_addr.0..3: Address
                - ep_addr.7:    Direction
  \param[in]   stall  Operation
                - \b false Clear
                - \b true Set
  \return      \ref execution_status
*/
static int32_t USBD_EndpointStall (uint8_t ep_addr, bool stall) {
  uint32_t val, num;

  if ((usbd_flags & USB_CONNECTED) == 0) {
    // Device is not connected
    return ARM_DRIVER_ERROR;
  }

  if ((EPQHx[EP_IDX(ep_addr)].ep.flags & USBD_EP_FLAG_CONFIGURED) == 0) {
    // Endpoint is not configured
    return ARM_DRIVER_ERROR;
  }

  if (EPQHx[EP_IDX(ep_addr)].ep.flags & USBD_EP_FLAG_BUSY) {
    // Endpoint is busy
    return ARM_DRIVER_ERROR_BUSY;
  }

  num =  EP_NUM(ep_addr);
  val =  EP_VAL(ep_addr);
  
  if (stall) {
    // Set endpoint stall
    ENDPTCTRL(num) |=  (USB_ENDPTCTRL_RXS << val);
  } else {
    // Clear endpoint stall
    ENDPTCTRL(num) &= ~(USB_ENDPTCTRL_RXS << val);

    EPQHx[EP_IDX(ep_addr)].dTD_token = 0;
    if (USBD_EndpointFlush(ep_addr) == false) return ARM_DRIVER_ERROR;

    // Data toggle reset
    ENDPTCTRL(num) |=  (USB_ENDPTCTRL_RXR << val);
  }
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_EndpointTransfer (uint8_t ep_addr, uint8_t *data, uint32_t num)
  \brief       Read data from or Write data to USB Endpoint.
  \param[in]   ep_addr  Endpoint Address
                - ep_addr.0..3: Address
                - ep_addr.7:    Direction
  \param[out]  data Pointer to buffer for data to read or with data to write
  \param[in]   num  Number of data bytes to transfer
  \return      \ref execution_status
*/
static int32_t USBD_EndpointTransfer (uint8_t ep_addr, uint8_t *data, uint32_t num) {

  if ((usbd_flags & USB_CONNECTED) == 0) {
    // Device is not connected
    return ARM_DRIVER_ERROR;
  }

  if ((EPQHx[EP_IDX(ep_addr)].ep.flags & USBD_EP_FLAG_CONFIGURED) == 0) {
    // Endpoint is not configured
    return ARM_DRIVER_ERROR;
  }

  if (EPQHx[EP_IDX(ep_addr)].ep.flags & USBD_EP_FLAG_BUSY) {
    // Endpoint is busy
    return ARM_DRIVER_ERROR_BUSY;
  }

  // Set endpoint busy
  EPQHx[EP_IDX(ep_addr)].ep.flags |= USBD_EP_FLAG_BUSY;

  // Save buffer information
  EPQHx[EP_IDX(ep_addr)].ep.bufferIndex = 0;
  EPQHx[EP_IDX(ep_addr)].ep.dataSize    = num;
  EPQHx[EP_IDX(ep_addr)].ep.buffer      = data;

  // Prime endpoint
  USBD_EndpointPrime (ep_addr, data, num);

  return ARM_DRIVER_OK;
}

/**
  \fn          uint32_t USBD_EndpointTransferGetResult (uint8_t ep_addr)
  \brief       Get result of USB Endpoint transfer.
  \param[in]   ep_addr  Endpoint Address
                - ep_addr.0..3: Address
                - ep_addr.7:    Direction
  \return      number of successfully transfered data bytes
*/
static uint32_t USBD_EndpointTransferGetResult (uint8_t ep_addr) {

  return (EPQHx[EP_IDX(ep_addr)].ep.bufferIndex);
}

/**
  \fn          int32_t USBD_EndpointTransferAbort (uint8_t ep_addr)
  \brief       Abort current USB Endpoint transfer.
  \param[in]   ep_addr  Endpoint Address
                - ep_addr.0..3: Address
                - ep_addr.7:    Direction
  \return      \ref execution_status
*/
static int32_t USBD_EndpointTransferAbort (uint8_t ep_addr) {
  uint32_t val, num;

  if ((usbd_flags & USB_CONNECTED) == 0) {
    // Device is not connected
    return ARM_DRIVER_ERROR;
  }

  if ((EPQHx[EP_IDX(ep_addr)].ep.flags & USBD_EP_FLAG_CONFIGURED) == 0) {
    // Endpoint is not configured
    return ARM_DRIVER_ERROR;
  }

  num =  EP_NUM(ep_addr);
  val =  EP_VAL(ep_addr);

  if (USBD_EndpointFlush(ep_addr) == false) return ARM_DRIVER_ERROR;

  // Clear completed flag
  LPC_USBx->ENDPTCOMPLETE     = (1 << (num + val));

  // Data toggle reset
  ENDPTCTRL(num) |=  (USB_ENDPTCTRL_RXR << val);

  EPQHx[EP_IDX(ep_addr)].dTD_token  &= ~0xFF;
  EPQHx[EP_IDX(ep_addr)].ep.buffer   = NULL;
  EPQHx[EP_IDX(ep_addr)].ep.dataSize = 0;

  // Clear endpoint busy
  EPQHx[EP_IDX(ep_addr)].ep.flags &= ~USBD_EP_FLAG_BUSY;

  return ARM_DRIVER_OK;
}

/**
  \fn          uint16_t USBD_GetFrameNumber (void)
  \brief       Get current USB Frame Number.
  \return      Frame Number
*/
static uint16_t USBD_GetFrameNumber (void) {

  return ((LPC_USBx->FRINDEX_D & USB_FRINDEX_D_FRINDEX13_3_MSK) >>
           USB_FRINDEX_D_FRINDEX13_3_POS);
}

/**
  \fn          void USBD1_IRQ (void)
  \brief       USB Device Interrupt Routine (IRQ).
*/
void USBD1_IRQ (void) {
  uint32_t sts, cmpl, num, len, ep_addr, dev_evt;

  // Save USB and endpoint status
  sts  = LPC_USBx->USBSTS_D & LPC_USBx->USBINTR_D;
  cmpl = LPC_USBx->ENDPTCOMPLETE;

  // Clear endpoint complete flag
  LPC_USBx->ENDPTCOMPLETE = cmpl;

  // Clear interrupt flag
  LPC_USBx->USBSTS_D = sts;

  // Reset device event variable
  dev_evt = 0;

  // Reset interrupt
  if (sts & USB_USBDSTS_D_URI) {
    USBD_Reset();
    dev_evt |= ARM_USBD_EVENT_RESET;
  }

  // Suspend interrupt
  if (sts & USB_USBDSTS_D_SLI) {
    usbd_state.active = false;
    dev_evt |= ARM_USBD_EVENT_SUSPEND;

#if (RTE_USB1_IND0_PIN_EN)
    // Clear indicator LED0
    LPC_USBx->PORTSC1_D &= ~USB_PORTSC1_D_PIC1_0(1);
#endif
  }

  // Sort change detect interrupt
  if (sts & USB_USBDSTS_D_PCI) {
    if (((LPC_USBx->PORTSC1_D & USB_PORTSC1_D_PSPD_MSK) >>
          USB_PORTSC1_D_PSPD_POS) == 2) {
      usbd_state.speed = ARM_USB_SPEED_HIGH;
      dev_evt |= ARM_USBD_EVENT_HIGH_SPEED;
    } else {
      usbd_state.speed = ARM_USB_SPEED_FULL;
    }
    usbd_state.active = true;
#if (RTE_USB1_IND0_PIN_EN)
    // Set indicator LED0
    LPC_USBx->PORTSC1_D |= USB_PORTSC1_D_PIC1_0(1);
#endif
    dev_evt |= ARM_USBD_EVENT_RESUME;
  }

  // Signal device event
  if (dev_evt && cbDeviceEvent) cbDeviceEvent(dev_evt);

  // USB interrupt - completed transfer
  if (sts & USB_USBDSTS_D_UI) {
    // Setup Packet
    if (LPC_USBx->ENDPTSETUPSTAT) {
      if (USBD_ReadSetup() == true) {
        setup_flag = 1;
        cbEndpointEvent(0, ARM_USBD_EVENT_SETUP);
      }
    }
    // IN Packet
    if (cmpl & USB_ENDPTCOMPLETE_ETCE_MSK) {
      // For each physical IN endpoint
      for (num = 0; num <= USBD_EP_NUM; num++) {
        
        //Check if endpoint complete is set
        if ((cmpl  & USB_ENDPTCOMPLETE_ETCE_MSK) & 
            (1 << (num + USB_ENDPTCOMPLETE_ETCE_POS))) {

          ep_addr = num | ARM_USB_ENDPOINT_DIRECTION_MASK;

          // Check if all required IN data is sent
          if (EPQHx[EP_IDX(ep_addr)].ep.dataSize == EPQHx[EP_IDX(ep_addr)].ep.bufferIndex) {

            // Clear endpoint busy
            EPQHx[EP_IDX(ep_addr)].ep.flags &= ~USBD_EP_FLAG_BUSY;

            // Set IN endpoint event
            cbEndpointEvent(ep_addr, ARM_USBD_EVENT_IN);

          } else {
            // Prepare next transfer
            len = EPQHx[EP_IDX(ep_addr)].ep.dataSize - EPQHx[EP_IDX(ep_addr)].ep.bufferIndex;

            USBD_EndpointPrime (ep_addr, (uint8_t *) (EPQHx[EP_IDX(ep_addr)].ep.buffer + EPQHx[EP_IDX(ep_addr)].ep.bufferIndex), len);
          }
        }
      }
    }

    // OUT Packet
    if (cmpl & USB_ENDPTCOMPLETE_ERCE_MSK) {
      // For each physical OUT endpoint
      for (num = 0; num <= USBD_EP_NUM; num++) {

        // Check if endpoint complete is set
        if ((cmpl & USB_ENDPTCOMPLETE_ERCE_MSK) & (1 << num)) {

          // Update buffer index, regarding size of received data
          len = EPQHx[EP_IDX(num)].ep.dataSize -
                ((dTDx[EP_IDX(num)].dTD_token & USB_bTD_TOKEN_TOTAL_BYTES_MSK) >>
                  USB_bTD_TOKEN_TOTAL_BYTES_POS);

          EPQHx[EP_IDX(num)].ep.bufferIndex += len;
          EPQHx[EP_IDX(num)].ep.dataSize    -= len;

          // Check if all OUT data is received:
          //  - Data terminated with ZLP or short packet
          //  - All required data received
          if ((len % EPQHx[EP_IDX(num)].ep.maxPacketSize) ||
              (EPQHx[EP_IDX(num)].ep.dataSize == 0)) {

            // Clear endpoint busy
            EPQHx[EP_IDX(num)].ep.flags &= ~USBD_EP_FLAG_BUSY;

            // Set OUT endpoint event
            cbEndpointEvent(num, ARM_USBD_EVENT_OUT);

          } else {
            // Prepare next out transaction 
            USBD_EndpointPrime (num, (uint8_t *) (EPQHx[EP_IDX(num)].ep.buffer + EPQHx[EP_IDX(num)].ep.bufferIndex), EPQHx[EP_IDX(num)].ep.dataSize);
          }
        }
      }
    }
  }
}

ARM_DRIVER_USBD Driver_USBD1 = {
  USBD_GetVersion,
  USBD_GetCapabilities,
  USBD_Initialize,
  USBD_Uninitialize,
  USBD_PowerControl,
  USBD_DeviceConnect,
  USBD_DeviceDisconnect,
  USBD_DeviceGetState,
  USBD_DeviceRemoteWakeup,
  USBD_DeviceSetAddress,
  USBD_ReadSetupPacket,
  USBD_EndpointConfigure,
  USBD_EndpointUnconfigure,
  USBD_EndpointStall,
  USBD_EndpointTransfer,
  USBD_EndpointTransferGetResult,
  USBD_EndpointTransferAbort,
  USBD_GetFrameNumber
};
