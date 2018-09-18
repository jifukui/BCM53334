/*
 * $Id: board.h,v 1.19 Broadcom SDK $
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

#ifndef _BOARD_H_
#define _BOARD_H_

/* Number of units */
#define BOARD_NUM_OF_UNITS              (1)


/* Maximal port number for pbmp_t structure */
#define SOC_MAX_NUM_PORTS               (29)

/* Maximal number of ports (may be different than actual port count) */
#define  BOARD_MAX_NUM_OF_PORTS         (28)

/* Maximal number of qvlan groups (may be different than actual vlan group) */
#define  BOARD_MAX_NUM_OF_QVLANS        (128)

/* Maximal number of port based vlan groups : 
 *  - groups for every ports and uplink port(egress on all ports)
 */
#define  BOARD_MAX_NUM_OF_PVLANS         (BOARD_MAX_NUM_OF_PORTS + 1)

#define  BOARD_MAX_NUM_OF_LAG           (4)
#define  BOARD_MAX_PORT_PER_LAG         (8)

/* Max required width (in bytes) for logical port list */
#define MAX_UPLIST_WIDTH  ((BOARD_MAX_NUM_OF_PORTS + 7) / 8)

/* Memory pool (for malloc) */

/* System AXI clock in Hz (400MHz) */
#define BOARD_DEFAULT_AXI_CLOCK     (400000000)

#if CONFIG_HURRICANE3_EMULATION

/* UART clock in ChipCommonA in Hz (refer to c_clk100 emulation clock) */
#define BOARD_CCA_HR3_UART_CLOCK           (81545)

/* UART clock in ChipCommonA in Hz (refer to c_clk50 emulation clock) */
#define BOARD_CCA_WF2_UART_CLOCK           (81545)

/* UART clock in ChipCommonA in Hz (refer to c_clk31.25 emulation clock) */
#define BOARD_CCA_FH2_UART_CLOCK          (81545)

/* CPU clock in Hz (emulation clock) */
#define BOARD_CPU_CLOCK               (114872)

#else
/* APB clock in ChipCommonA in Hz (100MHz, AXI clock/4) */
#define BOARD_CCA_HR3_UART_CLOCK           (100000000)

/* APB clock in ChipCommonA in Hz (50MHz, AXI clock/4) */
#define BOARD_CCA_WF2_UART_CLOCK           (50000000)

/* APB clock in ChipCommonA in Hz (25MHz, AXI clock/4) */
#define BOARD_CCA_FH2_UART_CLOCK           (25000000)


/* CPU clock in Hz (100MHz) */
#define BOARD_CPU_CLOCK               (hr3_sw_info.cpu_clock)

#endif

/* Max size of RX packet buffer */
#define DEFAULT_RX_BUFFER_SIZE          (1600)

/* Max number of packet buffers */
#define DEFAULT_RX_BUFFER_COUNT         (1)

/* Loader address */
#define BOARD_LOADER_ADDR              (0xF0000000)

/* Frimware address to program */
#define BOARD_FIRMWARE_ADDR            (0xF0100000)

/* Frimware address to program */
#define BOARD_SECONDARY_FIRMWARE_ADDR  (0xF0280000)

/* Shared internal SRAM memory address for loader and firmware */
#define BOARD_BOOKKEEPING_ADDR         (0x02000000)

/* Factory address */
#define CFG_FACTORY_CONFIG_BASE        (0xF00FE000)
#define CFG_FACTORY_CONFIG_OFFSET      (0x0)

/* Persistence address */
#define CFG_FLASH_START_ADDRESS        (0xF0000000)

/* Config address */
#define CFG_CONFIG_BASE                 (0xF00FD000)
#define CFG_CONFIG_OFFSET               (0x000)
#define CFG_CONFIG_SIZE                 (0x1000 - CFG_CONFIG_OFFSET)

#define MEDIUM_FLASH_START_ADDRESS     (0xF00FF000)

#define MEDIUM_FLASH_SIZE              (0x1000)

/* Shortcut TX/RX for boot loader */
#define BOOT_SOC_INCLUDE_FILE          "soc/bcm5343x.h"
#define BOOT_FUNC_TX                    bcm5343x_tx
#define BOOT_FUNC_RX_SET_HANDLER        bcm5343x_rx_set_handler
#define BOOT_FUNC_RX_FILL_BUFFER        bcm5343x_rx_fill_buffer

/* The MPU supports zero, 12, or 16 regions */
#define MPU_NUM_REGIONS                 (12)

#ifndef __ASSEMBLER__
extern void board_early_init(void);
extern sys_error_t board_init(void);
extern void board_late_init(void); 
extern uint8  board_linkscan_disable;
extern uint8  board_linkdown_message;
extern uint8  board_upload_vc;
#endif

/* default 1Q-VLAN disabled(PVLAN mode) */
#ifdef CFG_SWITCH_VLAN_INCLUDED
/* SWITCH_VLAN_FEATURE_IS_READY :
 *  - This symbol will be removed after VLAN Board/SOC API implementation is finished.
 */
#define SWITCH_VLAN_FEATURE_IS_READY    0
#if SWITCH_VLAN_FEATURE_IS_READY
#define DEFAULT_QVLAN_DISABLED
#endif  /* SWITCH_VLAN_FEATURE_IS_READY */
#endif  /* CFG_SWITCH_VLAN_INCLUDED */


#endif /* _BOARD_H_ */
