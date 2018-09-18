/*
 * $Id: ns16550.h,v 1.4 Broadcom SDK $
 *
 * $Copyright: Copyright 2016 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 */

/*
 * NS16550 UART registers
 */

#ifndef _NS16550_H_
#define _NS16550_H_

#ifndef NS16550_HZ
#define NS16550_HZ  1843200
#endif

/*
 * NS16550 UART registers
 */

/* Register definitions */

#define R_UART_DATA 0
#define R_UART_IER  4
#define R_UART_IIR  8
#define R_UART_FIFO 8
#define R_UART_CFCR 12
#define R_UART_MCR  16
#define R_UART_LSR  20
#define R_UART_MSR  24
#define R_UART_SCR  28

/* 16 bit baud rate divisor (lower byte in UART_DATA, upper in UART_IER) */
#define BRTC(clk,baud)      (((clk) + 8*(baud)) / (16*(baud)))

/* interrupt enable register */
#define IER_ERXRDY  0x1 /* int on rx ready */
#define IER_ETXRDY  0x2 /* int on tx ready */
#define IER_ERLS    0x4 /* int on line status change */
#define IER_EMSC    0x8 /* int on modem status change */

/* interrupt identification register */
#define IIR_IMASK   0xf /* mask */
#define IIR_RXTOUT  0xc /* receive timeout */
#define IIR_RLS     0x6 /* receive line status */
#define IIR_RXRDY   0x4 /* receive ready */
#define IIR_TXRDY   0x2 /* transmit ready */
#define IIR_NOPEND  0x1 /* nothing */
#define IIR_MLSC    0x0 /* modem status */
#define IIR_FIFO_MASK   0xc0    /* set if FIFOs are enabled */

/* fifo control register */
#define FIFO_ENABLE 0x01    /* enable fifo */
#define FIFO_RCV_RST    0x02    /* reset receive fifo */
#define FIFO_XMT_RST    0x04    /* reset transmit fifo */
#define FIFO_DMA_MODE   0x08    /* enable dma mode */
#define FIFO_TRIGGER_1  0x00    /* trigger at 1 char */
#define FIFO_TRIGGER_4  0x40    /* trigger at 4 chars */
#define FIFO_TRIGGER_8  0x80    /* trigger at 8 chars */
#define FIFO_TRIGGER_14 0xc0    /* trigger at 14 chars */

/* character format control register */
#define CFCR_DLAB   0x80    /* divisor latch */
#define CFCR_SBREAK 0x40    /* send break */
#define CFCR_PZERO  0x30    /* zero parity */
#define CFCR_PONE   0x20    /* one parity */
#define CFCR_PEVEN  0x10    /* even parity */
#define CFCR_PODD   0x00    /* odd parity */
#define CFCR_PENAB  0x08    /* parity enable */
#define CFCR_STOPB  0x04    /* 2 stop bits */
#define CFCR_8BITS  0x03    /* 8 data bits */
#define CFCR_7BITS  0x02    /* 7 data bits */
#define CFCR_6BITS  0x01    /* 6 data bits */
#define CFCR_5BITS  0x00    /* 5 data bits */

/* modem control register */
#define MCR_LOOPBACK    0x10    /* loopback */
#define MCR_IENABLE 0x08    /* output 2 = int enable */
#define MCR_DRS     0x04    /* output 1 = xxx */
#define MCR_RTS     0x02    /* enable RTS */
#define MCR_DTR     0x01    /* enable DTR */

/* line status register */
#define LSR_RCV_FIFO    0x80    /* error in receive fifo */
#define LSR_TSRE    0x40    /* transmitter empty */
#define LSR_TXRDY   0x20    /* transmitter ready */
#define LSR_BI      0x10    /* break detected */
#define LSR_FE      0x08    /* framing error */
#define LSR_PE      0x04    /* parity error */
#define LSR_OE      0x02    /* overrun error */
#define LSR_RXRDY   0x01    /* receiver ready */
#define LSR_RCV_MASK    0x1f

/* modem status register */
#define MSR_DCD     0x80    /* DCD active */
#define MSR_RI      0x40    /* RI  active */
#define MSR_DSR     0x20    /* DSR active */
#define MSR_CTS     0x10    /* CTS active */
#define MSR_DDCD    0x08    /* DCD changed */
#define MSR_TERI    0x04    /* RI  changed */
#define MSR_DDSR    0x02    /* DSR changed */
#define MSR_DCTS    0x01    /* CTS changed */

#endif /* _NS16550_H_ */
