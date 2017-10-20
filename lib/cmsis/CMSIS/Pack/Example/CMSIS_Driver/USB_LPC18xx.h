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
 * $Revision:    V2.00
 *
 * Project:      USB Driver Definitions for NXP LPC18xx
 * ---------------------------------------------------------------------------*/

#ifndef __USB_LPC18XX_H
#define __USB_LPC18XX_H

#include <stdint.h>

#ifndef USB_ENDPT_MSK
#define USB_ENDPT_MSK                          (0x3F)
#endif

// USB Command Register
#define USB_USBCMD_D_RS                        (1     <<  0)
#define USB_USBCMD_D_RST                       (1     <<  1)
#define USB_USBCMD_D_SUTW                      (1     << 13)
#define USB_USBCMD_D_ATDTW                     (1     << 14)
#define USB_USBCMD_D_ITC_POS                   (         16)
#define USB_USBCMD_D_ITC_MSK                   (0xFF  << USB_USBCMD_D_ITC_POS)
#define USB_USBCMD_D_ITC(n)                    (((n)  << USB_USBCMD_D_ITC_POS) & USB_USBCMD_D_ITC_MSK)

// USB Status Register
#define USB_USBDSTS_D_UI                       (1     <<  0)
#define USB_USBDSTS_D_UEI                      (1     <<  1)
#define USB_USBDSTS_D_PCI                      (1     <<  2)
#define USB_USBDSTS_D_URI                      (1     <<  6)
#define USB_USBDSTS_D_SRI                      (1     <<  7)
#define USB_USBDSTS_D_SLI                      (1     <<  8)
#define USB_USBDSTS_D_NAKI                     (1     << 16)

// USB Interrupt Register
#define USB_USBINTR_D_UE                       (1     <<  0)
#define USB_USBINTR_D_UEE                      (1     <<  1)
#define USB_USBINTR_D_PCE                      (1     <<  2)
#define USB_USBINTR_D_URE                      (1     <<  6)
#define USB_USBINTR_D_SRE                      (1     <<  7)
#define USB_USBINTR_D_SLE                      (1     <<  8)
#define USB_USBINTR_D_NAKE                     (1     << 16)

// USB Frame Index Register
#define USB_FRINDEX_D_FRINDEX2_0_POS           (          0)
#define USB_FRINDEX_D_FRINDEX2_0_MSK           (7     << USB_FRINDEX_D_FRINDEX2_0_POS)
#define USB_FRINDEX_D_FRINDEX13_3_POS          (          3)
#define USB_FRINDEX_D_FRINDEX13_3_MSK          (0x7FF << USB_FRINDEX_D_FRINDEX13_3_POS)

// USB Device Address Register
#define USB_DEVICEADDR_USBADRA                 (1     << 24)
#define USB_DEVICEADDR_USBADR_POS              (         25)
#define USB_DEVICEADDR_USBADR_MSK              (0x7FUL << USB_DEVICEADDR_USBADR_POS)

// USB Endpoint List Address Register
#define USB_ENDPOINTLISTADDR_EPBASE31_11_POS   (         11)
#define USB_ENDPOINTLISTADDR_EPBASE31_11_MSK   (0x1FFFFFUL << USB_ENDPOINTLISTADDR_EPBASE31_11_POS)

// USB Burst Size Register
#define USB_BURSTSIZE_RXPBURST_POS             (          0)
#define USB_BURSTSIZE_RXPBURST_MSK             (0xFF  << USB_BURSTSIZE_RXPBURST_POS)
#define USB_BURSTSIZE_TXPBURST_POS             (          8)
#define USB_BURSTSIZE_TXPBURST_MSK             (0xFF  << USB_BURSTSIZE_TXPBURST_POS)

// USB ULPI Viewport register (USB1 only)
#define USB_ULPIVIEWPORT_ULPIDATWR_POS         (          0)
#define USB_ULPIVIEWPORT_ULPIDATRW_MSK         (0xFF  << USB_ULPIVIEWPORT_ULPIDATWR_POS)
#define USB_ULPIVIEWPORT_ULPIDATRD_POS         (          8)
#define USB_ULPIVIEWPORT_ULPIDATRD_MSK         (0xFF  << USB_ULPIVIEWPORT_ULPIDATRD_POS)
#define USB_ULPIVIEWPORT_ULPIADDR_POS          (         16)
#define USB_ULPIVIEWPORT_ULPIADDR_MSK          (0xFF  << USB_ULPIVIEWPORT_ULPIADDR_POS)
#define USB_ULPIVIEWPORT_ULPIPORT_POS          (         24)
#define USB_ULPIVIEWPORT_ULPIPORT_MSK          (7     << USB_ULPIVIEWPORT_ULPIPORT_POS)
#define USB_ULPIVIEWPORT_ULPISS                (1     << 27)
#define USB_ULPIVIEWPORT_ULPIRW                (1     << 29)
#define USB_ULPIVIEWPORT_ULPIRUN               (1     << 30)
#define USB_ULPIVIEWPORT_ULPIWU                (1UL   << 31)

// USB BInterval Register
#define USB_BINTERVAL_BINT_POS                 (          0)
#define USB_BINTERVAL_BINT_MSK                 (0x0F  << USB_BINTERVAL_BINT_POS)

// USB Endpoint NAK Register
#define USB_ENDPTNAK_EPRN_POS                  (          0)
#define USB_ENDPTNAK_EPRN_MSK                  (USB_ENDPT_MSK << USB_ENDPTNAK_EPRN_POS)
#define USB_ENDPTNAK_EPTN_POS                  (         16)
#define USB_ENDPTNAK_EPTN_MSK                  (USB_ENDPT_MSK << USB_ENDPTNAK_EPTN_POS)

// USB Endpoint NAK Enable Register
#define USB_ENDPTNAKEN_EPRNE_POS               (          0)
#define USB_ENDPTNAKEN_EPRNE_MSK               (USB_ENDPT_MSK << USB_ENDPTNAKEN_EPRNE_POS)
#define USB_ENDPTNAKEN_EPTNE_POS               (         16)
#define USB_ENDPTNAKEN_EPTNE_MSK               (USB_ENDPT_MSK << USB_ENDPTNAKEN_EPTNE_POS)

// USB Port Status and Control Register
#define USB_PORTSC1_D_CCS                      (1     <<  0)
#define USB_PORTSC1_D_PE                       (1     <<  2)
#define USB_PORTSC1_D_PEC                      (1     <<  3)
#define USB_PORTSC1_D_FPR                      (1     <<  6)
#define USB_PORTSC1_D_SUSP                     (1     <<  7)
#define USB_PORTSC1_D_PR                       (1     <<  8)
#define USB_PORTSC1_D_HSP                      (1     <<  9)
#define USB_PORTSC1_D_PIC1_0_POS               (         14)
#define USB_PORTSC1_D_PIC1_0_MSK               (3     << USB_PORTSC1_D_PIC1_0_POS)
#define USB_PORTSC1_D_PIC1_0(n)                (((n)  << USB_PORTSC1_D_PIC1_0_POS) & USB_PORTSC1_D_PIC1_0_MSK)
#define USB_PORTSC1_D_PTC3_0_POS               (         16)
#define USB_PORTSC1_D_PTC3_0_MSK               (0x0F  << USB_PORTSC1_D_PTC3_0_POS)
#define USB_PORTSC1_D_PHCD                     (1     << 23)
#define USB_PORTSC1_D_PFSC                     (1     << 24)
#define USB_PORTSC1_D_PSPD_POS                 (         26)
#define USB_PORTSC1_D_PSPD_MSK                 (3     << USB_PORTSC1_D_PSPD_POS)
#define USB_PORTSC1_D_PTS_POS                  (         30)
#define USB_PORTSC1_D_PTS_MSK                  (3UL   << USB_PORTSC1_D_PTS_POS)
#define USB_PORTSC1_D_PTS(n)                   (((n)  << USB_PORTSC1_D_PTS_POS) & USB_PORTSC1_D_PTS_MSK)

// OTG Status and Control Register (USB0 only)
#define USB_OTGSC_VD                           (1     <<  0)
#define USB_OTGSC_VC                           (1     <<  1)
#define USB_OTGSC_HAAR                         (1     <<  2)
#define USB_OTGSC_OT                           (1     <<  3)
#define USB_OTGSC_DP                           (1     <<  4)
#define USB_OTGSC_IDPU                         (1     <<  5)
#define USB_OTGSC_HADP                         (1     <<  6)
#define USB_OTGSC_HABA                         (1     <<  7)
#define USB_OTGSC_ID                           (1     <<  8)
#define USB_OTGSC_AVV                          (1     <<  9)
#define USB_OTGSC_ASV                          (1     << 10)
#define USB_OTGSC_BSV                          (1     << 11)
#define USB_OTGSC_BSE                          (1     << 12)
#define USB_OTGSC_MS1T                         (1     << 13)
#define USB_OTGSC_DPS                          (1     << 14)
#define USB_OTGSC_IDIS                         (1     << 16)
#define USB_OTGSC_AVVIS                        (1     << 17)
#define USB_OTGSC_ASVIS                        (1     << 18)
#define USB_OTGSC_BSVIS                        (1     << 19)
#define USB_OTGSC_BSEIS                        (1     << 20)
#define USB_OTGSC_MS1S                         (1     << 21)
#define USB_OTGSC_DPIS                         (1     << 22)
#define USB_OTGSC_IDIE                         (1     << 24)
#define USB_OTGSC_AVVIE                        (1     << 25)
#define USB_OTGSC_ASVIE                        (1     << 26)
#define USB_OTGSC_BSVIE                        (1     << 27)
#define USB_OTGSC_BSEIE                        (1     << 28)
#define USB_OTGSC_MS1E                         (1     << 29)
#define USB_OTGSC_DPIE                         (1     << 30)

// USB Mode Register
#define USB_USBMODE_CM1_0_POS                  (          0)
#define USB_USBMODE_CM1_0_MSK                  (3     << USB_USBMODE_CM1_0_POS)
#define USB_USBMODE_CM1_0(n)                   (((n)  << USB_USBMODE_CM1_0_POS) & USB_USBMODE_CM1_0_MSK)
#define USB_USBMODE_ES                         (1     <<  2)
#define USB_USBMODE_SLOM                       (1     <<  3)
#define USB_USBMODE_SDIS                       (1     <<  4)

// USB Endpoint Setup Status Register
#define USB_ENDPTSETUPSTAT_POS                 (          0)
#define USB_ENDPTSETUPSTAT_MSK                 (USB_ENDPT_MSK << USB_ENDPTSETUPSTAT_POS)

// USB Endpoint Prime Register
#define USB_ENDPTRPRIME_PERB_POS               (          0)
#define USB_ENDPTRPRIME_PERB_MSK               (USB_ENDPT_MSK << USB_ENDPTRPRIME_PERB_POS)
#define USB_ENDPTRPRIME_PETB_POS               (         16)
#define USB_ENDPTRPRIME_PETB_MSK               (USB_ENDPT_MSK << USB_ENDPTRPRIME_PETB_POS)

// USB Endpoint Flush Register
#define USB_ENDPTFLUSH_FERB_POS                (          0)
#define USB_ENDPTFLUSH_FERB_MSK                (USB_ENDPT_MSK << USB_ENDPTFLUSH_FERB_POS)
#define USB_ENDPTFLUSH_FETB_POS                (         16)
#define USB_ENDPTFLUSH_FETB_MSK                (USB_ENDPT_MSK << USB_ENDPTFLUSH_FETB_POS)

// USB Endpoint Status Register
#define USB_ENDPTSTAT_ERBR_POS                 (          0)
#define USB_ENDPTSTAT_ERBR_MSK                 (USB_ENDPT_MSK << USB_ENDPTSTAT_ERBR_POS)
#define USB_ENDPTSTAT_ETBR_POS                 (         16)
#define USB_ENDPTSTAT_ETBR_MSK                 (USB_ENDPT_MSK << USB_ENDPTSTAT_ETBR_POS)

// USB Endpoint Complete Register
#define USB_ENDPTCOMPLETE_ERCE_POS             (          0)
#define USB_ENDPTCOMPLETE_ERCE_MSK             (USB_ENDPT_MSK << USB_ENDPTCOMPLETE_ERCE_POS)
#define USB_ENDPTCOMPLETE_ETCE_POS             (         16)
#define USB_ENDPTCOMPLETE_ETCE_MSK             (USB_ENDPT_MSK << USB_ENDPTCOMPLETE_ETCE_POS)

// USB Endpoint Control Register
#define USB_ENDPTCTRL_RXS                      (1     <<  0)
#define USB_ENDPTCTRL_RXT_POS                  (          2)
#define USB_ENDPTCTRL_RXT_MSK                  (3     << USB_ENDPTCTRL_RXT_POS)
#define USB_ENDPTCTRL_RXT(n)                   (((n)  << USB_ENDPTCTRL_RXT_POS) & USB_ENDPTCTRL_RXT_MSK)
#define USB_ENDPTCTRL_RXI                      (1     <<  5)
#define USB_ENDPTCTRL_RXR                      (1     <<  6)
#define USB_ENDPTCTRL_RXE                      (1     <<  7)
#define USB_ENDPTCTRL_TXS                      (1     << 16)
#define USB_ENDPTCTRL_TXT_POS                  (         18)
#define USB_ENDPTCTRL_TXT_MSK                  (3     << USB_ENDPTCTRL_TXT_POS)
#define USB_ENDPTCTRL_TXT(n)                   (((n)  << USB_ENDPTCTRL_TXT_POS) & USB_ENDPTCTRL_TXT_MSK)
#define USB_ENDPTCTRL_TXI                      (1     << 21)
#define USB_ENDPTCTRL_TXR                      (1     << 22)
#define USB_ENDPTCTRL_TXE                      (1     << 23)


// Endpoint Queue Head Capabilities and Characteristics
#define USB_EPQH_CAP_IOS                       (1     << 15)
#define USB_EPQH_CAP_MAX_PACKET_LEN_POS        (         16)
#define USB_EPQH_CAP_MAX_PACKET_LEN_MSK        (0x7FF << USB_EPQH_CAP_MAX_PACKET_LEN_POS)
#define USB_EPQH_CAP_MAX_PACKET_LEN(n)         (((n)  << USB_EPQH_CAP_MAX_PACKET_LEN_POS) & USB_EPQH_CAP_MAX_PACKET_LEN_MSK)
#define USB_EPQH_CAP_ZLT                       (1     << 29)
#define USB_EPQH_CAP_MULT_POS                  (         30)
#define USB_EPQH_CAP_MULT_MSK                  (3UL   << USB_EPQH_CAP_MULT_POS)

// Transfer Descriptor Token
#define USB_bTD_TOKEN_STATUS_POS               (          0)
#define USB_bTD_TOKEN_STATUS_MSK               (0xFF  << USB_bTD_TOKEN_STATUS_POS)
#define USB_bTD_TOKEN_STATUS(n)                (((n)  << USB_bTD_TOKEN_STATUS_POS) & USB_bTD_TOKEN_STATUS_MSK)
#define USB_bTD_TOKEN_STATUS_TRAN_ERROR        ((0x08 << USB_bTD_TOKEN_STATUS_POS) & USB_bTD_TOKEN_STATUS_MSK)
#define USB_bTD_TOKEN_STATUS_BUFFER_ERROR      ((0x20 << USB_bTD_TOKEN_STATUS_POS) & USB_bTD_TOKEN_STATUS_MSK)
#define USB_bTD_TOKEN_STATUS_HALTED            ((0x40 << USB_bTD_TOKEN_STATUS_POS) & USB_bTD_TOKEN_STATUS_MSK)
#define USB_bTD_TOKEN_STATUS_ACTIVE            ((0x80 << USB_bTD_TOKEN_STATUS_POS) & USB_bTD_TOKEN_STATUS_MSK)
#define USB_bTD_TOKEN_MULTO_POS                (         10)
#define USB_bTD_TOKEN_MULTO_MSK                (3     << USB_bTD_TOKEN_MULTO_POS)
#define USB_bTD_TOKEN_MULTO(n)                 (((n)  << USB_bTD_TOKEN_MULTO_POS) & USB_bTD_TOKEN_MULTO_MSK)
#define USB_bTD_TOKEN_IOC                      (1     << 15)
#define USB_bTD_TOKEN_TOTAL_BYTES_POS          (         16)
#define USB_bTD_TOKEN_TOTAL_BYTES_MSK          (0x7FFF << USB_bTD_TOKEN_TOTAL_BYTES_POS)
#define USB_bTD_TOKEN_TOTAL_BYTES(n)           (((n) << USB_bTD_TOKEN_TOTAL_BYTES_POS) & USB_bTD_TOKEN_TOTAL_BYTES_MSK)

// USB Driver Flags
#define USB_INITIALIZED                        (1     <<  0)
#define USB_POWERED                            (1     <<  1)
#define USB_CONNECTED                          (1     <<  2)

// USB Device Endpoint Flags
#define USBD_EP_FLAG_CONFIGURED                (1     <<  0)
#define USBD_EP_FLAG_BUSY                      (1     <<  1)

// USB Device Endpoint Structure
typedef struct {
  uint8_t  *buffer;
  uint32_t  bufferIndex;
  uint32_t  dataSize;
  uint16_t  maxPacketSize;
  uint8_t   type;
  uint8_t   flags;
} ENDPOINT;

// USB Device Endpoint Queue Head
typedef struct __EPQH {
  uint32_t  cap;
  uint32_t  curr_dTD;
  uint32_t  next_dTD;
  uint32_t  dTD_token;
  uint32_t  buf[5];
  uint32_t  reserved;
  uint32_t  setup[2];
  ENDPOINT  ep;
} EPQH;

// USB Device Endpoint Transfer Descriptor
typedef struct __dTD {
  uint32_t  next_dTD;
  uint32_t  dTD_token;
  uint32_t  buf[5];
  uint32_t  reserved;
} dTD;

#endif /* __USB_LPC18XX_H */
