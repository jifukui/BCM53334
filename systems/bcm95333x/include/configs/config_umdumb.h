/*
 * $Id: config_umdumb.h,v 1.1.2.1 Broadcom SDK $
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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define __UMDUMB__

/* Chip type */
#define CFG_XGS_CHIP                        (1)
#define CFG_ROBO_CHIP                       (!CFG_XGS_CHIP)

/* System endian */
#define CFG_LITTLE_ENDIAN                   (1)

#define CONFIG_HURRICANE2_EMULATION         (0)

/* Enable console output */
#define CFG_CONSOLE_ENABLED                 (1)

/* Enable debug output */
#define CFG_DEBUGGING_ENABLED               (0)

/* Enable assertion */
#define CFG_ASSERTION_ENABLED               (0)

/* Enhanced power saving */
#define CFG_ENHANCED_POWER_SAVING           (0)

/* Enable packet RX/TX support */
#define CFG_RXTX_SUPPORT_ENABLED            (1)

/* Enable SAL LIB_support */
#define CFG_SAL_LIB_SUPPORT_ENABLED         (1)

/* CLI support */
#define CFG_CLI_ENABLED                     (1 && CFG_CONSOLE_ENABLED)

/* CLI prompt */
#define CFG_CLI_PROMPT                      "CMD> "

/* CLI system commands support */
#define CFG_CLI_SYSTEM_CMD_ENABLED          (1)

/* CLI switch commands support */
#define CFG_CLI_SWITCH_CMD_ENABLED          (1)

/* CLI power commands support */
#define CFG_CLI_POWER_CMD_ENABLED           (0)

/* CLI RX commands support */
#define CFG_CLI_RX_CMD_ENABLED              (1)

/* CLI TX commands support */
#define CFG_CLI_TX_CMD_ENABLED              (1)

/* CLI RX monitor */
#define CFG_CLI_RX_MON_PRIORITY             (1)

/* uIP main control RX priority */
#define CFG_UIP_RX_PRIORITY                 (10)

/* CLI TX packet configurations */
#define CFG_CLI_TX_MAX_PKTCFGS              (8)

/* UART baudrate */
#if CONFIG_HURRICANE2_EMULATION
#define CFG_UART_BAUDRATE                 (300)
#else
#define CFG_UART_BAUDRATE                (9600)
#endif


/*PCIe serdes powerdown */
#define CFG_PCIE_SERDES_POWERDOWN_ENABLED   (1)

/* Max background tasks */
#define CFG_MAX_BACKGROUND_TASKS            (8)

/* Enable timer callback mechanism */
#define CFG_TIMER_CALLBACK_SUPPORT          (1)

/* Max registered timer (callback) */
#define CFG_MAX_REGISTERED_TIMERS           (16)

/* Enable linkchange callback mechanism */
#define CFG_LINKCHANGE_CALLBACK_SUPPORT     (1)

/* Max registered link change callback */
#define CFG_MAX_REGISTERED_LINKCHANGE       (16)

/* Interval for checking link change (in us) */
#define CFG_LINKCHANGE_CHECK_INTERVAL       (600000UL)

/* Max registered RX callback functions */
#define CFG_MAX_REGISTERED_RX_CBKS          (16)

/* Whether RX uses interrupt for receiving notification */
#define CFG_RX_USE_INTERRUPT                (0)

/* Packet length (excluding CRC) */
#define MIN_PACKET_LENGTH                   (60)
#define MAX_PACKET_LENGTH                   (1514)

/* FP */
#define ENTRIES_PER_SLICE                   (256)

/* chip type */
#define CFG_SWITCH_XGS_NEW_SBUS_FORMAT_INCLUDED

/* Stoarge related */
#define CFG_FLASH_SUPPORT_ENABLED           (1)
#define CFG_CLI_FLASH_CMD_ENABLED           (1)

/* A factory data management engine */
#define CFG_FACTORY_CONFIG_INCLUDED

/* A binary storage management engine, named serializer */
//#define CFG_PERSISTENCE_SUPPORT_ENABLED     (1)

#define CFG_SWITCH_VLAN_INCLUDED

/* IP above layer related  setting*/
#define MAX_IP_TOTAL_LENGTH                 (1500)
#define CFG_NET_LINKCHANGE_NOTIFY_INCLUDED
#define CFG_NET_MAX_LINKCHANGE_HANDLER      (8)

/*
  *  CFG_NVRAM_SUPPORT_INCLUDED
  *      undefined : exclude the NVRAM defined advance command line engine
  *      defined: include the NVRAM storage
  */

/* #define CFG_NVRAM_SUPPORT_INCLUDED */





/****************************************************************************
 *
 * Features defined below can be selectively add or removed before building the image
 *
 ****************************************************************************/

/*
  *  CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
  *      undefined : exclude VENDOR CONFIG related code
  *      defined: will include the NVRAM ,some XCMD utils and VENDOR CONFIG code
  */

#define CFG_VENDOR_CONFIG_SUPPORT_INCLUDED

/*
  *  CFG_SWITCH_DOS_INCLUDED
  *      undefined : exclude DOS control feature
  *      defined: include DOS control feature
  */  
/* #define CFG_SWITCH_DOS_INCLUDED */

/*
  *  CFG_SWITCH_LAG_INCLUDED
  *      undefined : exclude LAG feature 
  *      defined: include LAG feature
  */  
//#define CFG_SWITCH_LAG_INCLUDED

/*
  *  CFG_UIP_STACK_ENABLED
  *      defined as 0 : disable UIP stack
  *      defined as 1 : enable UIP stack
  */
//#define CFG_UIP_STACK_ENABLED               (1 && CFG_RXTX_SUPPORT_ENABLED)

/*
  *  CFG_UIP_IPV6_ENABLED
  *      defined as 0 : disable IPv6
  *      defined as 1 : enable IPv6
  */
//#define CFG_UIP_IPV6_ENABLED                (1 && CFG_UIP_STACK_ENABLED)

#if CFG_UIP_STACK_ENABLED
/**
  *  CFG_DHCPC_INCLUDED
  *      undefined : DHCP client is disabled
  *      defined: DHCP client is enabled 
  */
#define CFG_DHCPC_INCLUDED
#endif /* CFG_UIP_STACK_ENABLED */

/*
  *  CFG_SWITCH_LOOPDETECT_INCLUDED
  *      undefined : exclude loop-detect feature 
  *      defined: include loop-detect feature 
  */  
//#define CFG_SWITCH_LOOPDETECT_INCLUDED

/*
  *  CFG_SWITCH_MIRROR_INCLUDED
  *      undefined : exclude mirror feature 
  *      defined: include mirror feature 
  */
//#define CFG_SWITCH_MIRROR_INCLUDED

/*
  *  CFG_SWITCH_QOS_INCLUDED
  *      undefined : exclude QoS feature 
  *      defined: include QoS feature
  */
#define CFG_SWITCH_QOS_INCLUDED

/*
  *  CFG_SWITCH_RATE_INCLUDED
  *      undefined : exclude rate feature 
  *      defined: include rate feature
  */
//#define CFG_SWITCH_RATE_INCLUDED

/*
  *  CFG_SWITCH_SNAKETEST_INCLUDED
  *      undefined : exclude snaketest feature 
  *      defined: include snaketest feature
  */
#define CFG_SWITCH_SNAKETEST_INCLUDED

/*
  *  CFG_SWITCH_STAT_INCLUDED
  *      undefined : exclude statistic feature 
  *      defined: include statistic feature
  */
#define CFG_SWITCH_STAT_INCLUDED

/*
  *  CFG_SWITCH_MCAST_INCLUDED
  *      undefined : exclude MCAST feature 
  *      defined: include MCAST feature
  */
//#define CFG_SWITCH_MCAST_INCLUDED

/*
  *  CFG_SWITCH_EEE_INCLUDED
  *      undefined : exclude EEE (Ethernet Energy Effieciency) feature
  *      defined: include EEE (Ethernet Energy Effieciency) feature
  */
#define CFG_SWITCH_EEE_INCLUDED

/*
  *  CFG_SWITCH_PVLAN_INCLUDED
  *      undefined : exclude port-based VLAN feature
  *      defined: include port-based VLAN feature
  */
#define CFG_SWITCH_PVLAN_INCLUDED

/*
  *  CFG_HW_CABLE_DIAG_INCLUDED
  *      undefined : exclude cable dialognostic feature 
  *      defined: include cable dialognostic feature 
  */
#define CFG_HW_CABLE_DIAG_INCLUDED

/*
  *  CFG_ETHERNET_WIRESPEED_INCLUDED
  *      undefined : exclude ethernet wirespeed feature
  *      defined: include ethernet wirespeed feature
  */
/* #define CFG_ETHERNET_WIRESPEED_INCLUDED */

#ifdef CFG_ETHERNET_WIRESPEED_INCLUDED
/*
 * Ethernet@wirespeed retry disable.
 *   defined as 0 1'b1 : downgrade after 1 failed link attemp
 *   defined as 0 1'b0 : use WIRESPEED_RETRY_LIMIT
 */
#define CFG_ETHERNET_WIRESPEED_RETRY_DIS       (0)
/*
 * Ethernet@wirespeed retry limit.
 * It is the number of auto-negotiation attemps to link-up prior to speed downgrade.
 * The ethernet@wirespeed mode must be enabled for retry limit. 
 * The retry limit can be chosen from 2-8.
 */
#define CFG_ETHERNET_WIRESPEED_RETRY_LIMIT     (5)
#endif /* CFG_ETHERNET_WIRESPEED_INCLUDED */

/*
  *  CFG_LED_MICROCODE_INCLUDED
  *      undefined : exclude serial LED feature
  *      defined as 1 : includee serial LED feature and use "direct serial out" 
  *      defined as 2 : includee serial LED feature and use "internal serial-to-parallel to chip ballout" 
  */
#define CFG_LED_MICROCODE_INCLUDED   (2)

/*
  *  CFG_CONFIG_OPTION : option in SW (option in PRD)
  * BCM53394: Option 1-3 
  *                             1(1): 10P 1G + 4x1/2.5/5/10G  (no PHY)
  *                             2(2): 10P 1G + 1P XAUI (no Phy) 
  *                             3(3): 6P 1G + 3x1/10G + 1P XAUI (no Phy)
  * BCM53344: Option 1-3 
  *                             1(1): 24P 1G + 4x1G (PHY)
  *                             2(2): 24P 1G + 2P 1G + 2P 13G (PHY)
  *                             3(2A): 24P 1G + 2P 13G + 2P 1G (PHY)
  * BCM53346: Option 1-7 
  *                             1(1): 24P 1G + 4P 1G/10G(PHY, TSC0)
  *                             2(1A): 24P 1G + 2P 1G/10G + 2P 1G/10G (PHY)
  *                             3(1B): 24P 1G + 4P 1G/10G  (PHY, TSC1)
  *                             4(2): 24P 1G + 2P 1G/10G + 2P 13G (PHY)
  *                             5(2A): 24P 1G + 2P 13G + 2P 1G/10G (PHY) 
  *                             6(3): 24P 1G + 3P 1G/10G + 1P XAUI (PHY)
  *                             7(4): 24P 1G + 1P XAUI + 1P XAUI (PHY)
  */
#define CFG_CONFIG_OPTION  (1)

/*
  *  CFG_CONFIG_1G_PORT_AN : default AN mode on ports with fiber mode and max speed is 1G 
  *      defined as 0 : disable AN
  *      defined as 1 : CL73
  *      defined as 2 : CL37
  */
#define CFG_CONFIG_1G_PORT_AN  (2)

/*
  *  CFG_CONFIG_10G_PORT_AN : default AN mode on ports with fiber mode and max speed is 10G 
  *      defined as 0 : disable AN
  *      defined as 1 : CL73
  */
#define CFG_CONFIG_10G_PORT_AN  (0)
#endif /* _CONFIG_H_ */
