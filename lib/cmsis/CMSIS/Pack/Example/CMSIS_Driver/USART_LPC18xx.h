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
 * $Date:        15. May 2014
 * $Revision:    V2.01
 *
 * Project:      USART Driver Definitions for NXP LPC18xx
 * -------------------------------------------------------------------------- */

#ifndef __USART_LPC18XX_H
#define __USART_LPC18XX_H

#include "LPC18xx.h"
#include "Driver_USART.h"

#include "SCU_LPC18xx.h"
#include "GPDMA_LPC18xx.h"

// USART register interface definitions
// USART Divisor Latch register LSB
#define USART_DLL_DLLSB_POS          (        0)
#define USART_DLL_DLLSB_MSK          (0xFF << USART_DLL_DLLSB_POS)

// USART Divisor Latch register MSB
#define USART_DLM_DLMSB_POS          (        0)
#define USART_DLM_DLMSB_MSK          (0xFF << USART_DLM_DLMSB_POS)

// USART Interrupt enable register
#define USART_IER_RBRIE              (1 << 0)
#define USART_IER_THREIE             (1 << 1)
#define USART_IER_RXIE               (1 << 2)
#define UART_IER_MSIE                (1 << 3) // Only for UART1 - modem status interrupt enable
#define USART_IER_ABEOINTEN          (1 << 8)
#define USART_IER_ABTOINTEN          (1 << 9)

// USART Interrupt identification register
#define USART_IIR_INTSTATUS          (1 << 0)
#define USART_IIR_INTID_POS          (     1)
#define USART_IIR_INTID_MSK          (7 << USART_IIR_INTID_POS)
#define USART_IIR_FIFOENABLE_POS     (     6)
#define USART_IIR_FIFOENABLE_MSK     (3 << USART_IIR_FIFOENABLE_POS)
#define USART_IIR_ABEOINT            (1 << 8)
#define USART_IIR_ABTOINT            (1 << 9)

#define USART_IIR_INTID_RLS          (3 << USART_IIR_INTID_POS)
#define USART_IIR_INTID_RDA          (2 << USART_IIR_INTID_POS)
#define USART_IIR_INTID_CTI          (6 << USART_IIR_INTID_POS)
#define USART_IIR_INTID_THRE         (1 << USART_IIR_INTID_POS)
#define UART_IIR_INTID_MS            (0 << USART_IIR_INTID_POS) // UART1 only

// USART FIFO control register
#define USART_FCR_FIFOEN             (1 << 0)
#define USART_FCR_RXFIFORES          (1 << 1)
#define USART_FCR_TXFIFORES          (1 << 2)
#define USART_FCR_DMAMODE            (1 << 3)
#define USART_FCR_RXTRIGLVL_POS      (     6)
#define USART_FCR_RXTRIGLVL_MSK      (3 << USART_FCR_RXTRIGLVL_POS)

// USART Line control register
#define USART_LCR_WLS_POS            (     0)
#define USART_LCR_WLS_MSK            (3 << USART_LCR_WLS_POS)
#define USART_LCR_SBS                (1 << 2)
#define USART_LCR_PE                 (1 << 3)
#define USART_LCR_PS_POS             (     4)
#define USART_LCR_PS_MSK             (3 << USART_LCR_PS_POS)
#define USART_LCR_BC                 (1 << 6)
#define USART_LCR_DLAB               (1 << 7)

// USART Line status register
#define USART_LSR_RDR                (1 << 0)
#define USART_LSR_OE                 (1 << 1)
#define USART_LSR_PE                 (1 << 2)
#define USART_LSR_FE                 (1 << 3)
#define USART_LSR_BI                 (1 << 4)
#define USART_LSR_THRE               (1 << 5)
#define USART_LSR_TEMT               (1 << 6)
#define USART_LSR_RXFE               (1 << 7)
#define USART_LSR_TXERR              (1 << 8)

#define USART_LSR_LINE_INT           (USART_LSR_OE | USART_LSR_PE | USART_LSR_FE | USART_LSR_BI)

// USART IrDA control register
#define USART_ICR_IRDAEN             (1 << 0)
#define USART_ICR_FIXPULSEEN         (1 << 1)
#define USART_ICR_IRDAINV            (1 << 2)
#define USART_ICR_PULSEDIV_POS       (     3)
#define USART_ICR_PULSEDIV_MSK       (7 << USART_ICR_PULSEDIV_POS)


// USART Fractional divider register
#define USART_FDR_DIVADDVAL_POS      (      0)
#define USART_FDR_DIVADDVAL_MSK      (0x0F << USART_FDR_DIVADDVAL_POS)
#define USART_FDR_MULVAL_POS         (      4)
#define USART_FDR_MULVAL_MSK         (0x0F << USART_FDR_MULVAL_POS)

// USART Half duplex enable register
#define USART_HDEN_HDEN              (1 << 0)

// USART SmartCard interface control register
#define USART_SCICTRL_SCIEN          (1 << 0)
#define USART_SCICTRL_NACKDIS        (1 << 1)
#define USART_SCICTRL_PROTSEL        (1 << 2)
#define USART_SCICTRL_TXRETRY_POS    (     5)
#define USART_SCICTRL_TXRETRY_MSK    (7 << USART_SCICTRL_TXRETRY_POS)
#define USART_SCICTRL_GUARDTIME_POS  (     8)
#define USART_SCICTRL_GUARDTIME_MSK  (0xFF << USART_SCICTRL_GUARDTIME_POS)

// USART Synchronous mode control register
#define USART_SYNCCTRL_SYNC          (1 << 0)
#define USART_SYNCCTRL_CSRC          (1 << 1)
#define USART_SYNCCTRL_FES           (1 << 2)
#define USART_SYNCCTRL_TSBYPASS      (1 << 3)
#define USART_SYNCCTRL_CSCEN         (1 << 4)
#define USART_SYNCCTRL_SSSDIS        (1 << 5)
#define USART_SYNCCTRL_CCCLR         (1 << 6)

// UART Modem control register
#define UART_MCR_DTRCTRL             (1 << 0)
#define UART_MCR_RTSCTRL             (1 << 1)
#define UART_MCR_LMS                 (1 << 4)
#define UART_MCR_RTSEN               (1 << 6)
#define UART_MCR_CTSEN               (1 << 7)

// UART Modem status register
#define UART_MSR_DCTS                (1 << 0)
#define UART_MSR_DDSR                (1 << 1)
#define UART_MSR_TERI                (1 << 2)
#define UART_MSR_DDCD                (1 << 3)
#define UART_MSR_CTS                 (1 << 4)
#define UART_MSR_DSR                 (1 << 5)
#define UART_MSR_RI                  (1 << 6)
#define UART_MSR_DCD                 (1 << 7)

// USART RS485 control register
#define USART_RS485CTRL_NMMEN        (1 << 0)
#define USART_RS485CTRL_RXDIS        (1 << 1)
#define USART_RS485CTRL_AADEN        (1 << 2)
#define USART_RS485CTRL_DCTRL        (1 << 4)
#define USART_RS485CTRL_OINV         (1 << 5)

// USART Transmitter enable register
#define USART_TER_TXEN               (1 << 0)


// USART flags
#define USART_FLAG_INITIALIZED       (1 << 0)
#define USART_FLAG_POWERED           (1 << 1)
#define USART_FLAG_CONFIGURED        (1 << 2)
#define USART_FLAG_TX_ENABLED        (1 << 3)
#define USART_FLAG_RX_ENABLED        (1 << 4)
#define USART_FLAG_SEND_ACTIVE       (1 << 5)

// USART synchronous xfer modes
#define USART_SYNC_MODE_TX           ( 1 )
#define USART_SYNC_MODE_RX           ( 2 )
#define USART_SYNC_MODE_TX_RX        (USART_SYNC_MODE_TX | \
                                      USART_SYNC_MODE_RX)

// Baudrate accepted error
#define UART_ACCEPTED_BAUDRATE_ERROR ( 3 )

// USART TX FIFO trigger level
#define USART_TRIG_LVL_1             (0x00)
#define USART_TRIG_LVL_4             (0x40)
#define USART_TRIG_LVL_8             (0x80)
#define USART_TRIG_LVL_14            (0xC0)

// USART Transfer Information (Run-Time)
typedef struct _USART_TRANSFER_INFO {
  uint32_t                rx_num;        // Total number of data to be received
  uint32_t                tx_num;        // Total number of data to be send
  uint8_t                *rx_buf;        // Pointer to in data buffer
  uint8_t                *tx_buf;        // Pointer to out data buffer
  uint32_t                rx_cnt;        // Number of data received
  uint32_t                tx_cnt;        // Number of data sent
  uint8_t                 tx_def_val;    // Transmit default value (used in USART_SYNC_MASTER_MODE_RX)
  uint8_t                 rx_dump_val;   // Receive dump value (used in USART_SYNC_MASTER_MODE_TX)
  uint8_t                 sync_mode;     // Synchronous mode
} USART_TRANSFER_INFO;

// USART Information (Run-Time)
typedef struct _USART_INFO {
  ARM_USART_SignalEvent_t cb_event;      // Event callback
  ARM_USART_STATUS        status;        // Status flags
  USART_TRANSFER_INFO     xfer;          // Transfer information
  uint8_t                 mode;          // USART mode
  uint8_t                 flags;         // USART driver flags
  uint32_t                baudrate;      // Baudrate
} USART_INFO;

// USART DMA
typedef const struct _USART_DMA {
  uint8_t                 channel;       // DMA Channel
  uint8_t                 peripheral;    // DMA mux
  uint8_t                 peripheral_sel;// DMA mux selection
  GPDMA_SignalEvent_t     cb_event;      // DMA Event callback
} USART_DMA;

// USART Pin Configuration
typedef const struct _USART_PINS {
  PIN_ID                 *tx;            // TX  Pin identifier
  PIN_ID                 *rx;            // RX  Pin identifier
  PIN_ID                 *clk;           // CLK  Pin identifier
  PIN_ID                 *cts;           // CTS Pin identifier
  PIN_ID                 *rts;           // RTS Pin identifier
  PIN_ID                 *dcd;           // DCD Pin identifier
  PIN_ID                 *dsr;           // DSR Pin identifier
  PIN_ID                 *dtr;           // DTR Pin identifier
  PIN_ID                 *ri;            // RI  Pin identifier
} USART_PINS;

// USART Clocks Configuration
typedef const struct _USART_CLOCK {
  __IO uint32_t          *reg_cfg;       // USART register interface clock configuration register
  __I  uint32_t          *reg_stat;      // USART register interface clock status register
  __IO uint32_t          *peri_cfg;      // USART peripheral clock configuration register
  __I  uint32_t          *peri_stat;     // USART peripheral clock status register
  __IO uint32_t          *base_clk;      // USART base clock
} USART_CLOCKS;

// USART Reset Configuration
typedef const struct _USART_RESET {
       uint32_t           reg_cfg_val;   // USART reset bit 
  __IO uint32_t          *reg_cfg;       // USART reset control register
  __I  uint32_t          *reg_stat;      // USART reset active status register
} USART_RESET;

// USART Resources definitions
typedef struct {
  ARM_USART_CAPABILITIES  capabilities;  // Capabilities
  LPC_USARTn_Type        *reg;           // Pointer to USART peripheral
  LPC_UART1_Type         *uart_reg;      // Pointer to UART peripheral
  USART_PINS              pins;          // USART pins configuration
  USART_CLOCKS            clk;           // USART clocks configuration
  USART_RESET             rst;           // USART reset configuration
  IRQn_Type               irq_num;       // USART IRQ Number
  uint32_t                trig_lvl;      // FIFO Trigger level
  USART_DMA              *dma_tx;
  USART_DMA              *dma_rx;
  USART_INFO             *info;          // Run-Time Information
} const USART_RESOURCES;

#endif /* __USART_LPC18XX_H */
