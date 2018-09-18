/*
 * $Id: config.c,v 1.3 Broadcom SDK $
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


#include <system.h>


void config_check(void) {
	
/* Checking for config dependence */
#if CFG_UIP_STACK_ENABLED
#if !CFG_RXTX_SUPPORT_ENABLED
#error Need to set CFG_RXTX_SUPPORT_ENABLED to 1 for UIP STACK
#endif
#endif
    
#ifdef CFG_SWITCH_SNAKETEST_INCLUDED
#if !CFG_RXTX_SUPPORT_ENABLED
#error Need to set CFG_RXTX_SUPPORT_ENABLED to 1 for CFG_SWITCH_SNAKETEST_INCLUDED
#endif
#if (!defined(CFG_SWITCH_VLAN_INCLUDED) && (!defined(CFG_SOC_SNAKE_TEST)))
#error Need to define CFG_SWITCH_VLAN_INCLUDED for CFG_SWITCH_SNAKETEST_INCLUDED
#endif
#endif
    
#ifdef CFG_SWITCH_MCAST_INCLUDED
#if !CFG_RXTX_SUPPORT_ENABLED
#error Need to set CFG_RXTX_SUPPORT_ENABLED to 1 for CFG_SWITCH_MCAST_INCLUDED
#endif
if (SAL_IS_UMWEB() || SAL_IS_RAMAPP()) {
#if !defined(CFG_SWITCH_VLAN_INCLUDED)
#error Need to define CFG_SWITCH_VLAN_INCLUDED to 1 for CFG_SWITCH_MCAST_INCLUDED
#endif
}
#endif

#if CFG_PERSISTENCE_SUPPORT_ENABLED
#if !CFG_FLASH_SUPPORT_ENABLED
#error Need to set CFG_FLASH_SUPPORT_ENABLED to 1 for CFG_PERSISTENCE_SUPPORT_ENABLED
#endif
#endif
        
#if CFG_CLI_ENABLED
#if !CFG_CONSOLE_ENABLED
#error Need to set CFG_CONSOLE_ENABLED to 1 for CLI
#endif
#endif
        
        
#if CFG_CLI_FLASH_CMD_ENABLED
#if !CFG_FLASH_SUPPORT_ENABLED
#error Need to set CFG_FLASH_SUPPORT_ENABLED to 1 for FLASH CLI
#endif
#endif


}

