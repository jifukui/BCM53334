/*
 * $Id: main.c,v 1.35.2.1 Broadcom SDK $
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

#include "system.h"
#include "appl/cli.h"
#include "appl/persistence.h"
#include "utils/system.h"
#include "utils/net.h"
#include <sal.h>
#ifdef __C51__
#ifdef CODE_USERCLASS
#pragma userclass (code = init)
#endif /* CODE_USERCLASS */
#ifdef XDATA_USERCLASS
#pragma userclass (xdata = init)
#endif /* XDATA_USERCLASS */
#endif /* __C51__ */

/* Initialization routines called by main */
extern void board_early_init(void) REENTRANT;
extern sys_error_t board_init(void) REENTRANT;
extern void board_late_init(void) REENTRANT;
extern void sal_init(void) REENTRANT;
extern void background_init(void) REENTRANT;
extern void sys_timer_init(void) REENTRANT;
extern void sys_linkchange_init(void) REENTRANT;
extern void sys_rx_init(void) REENTRANT;
extern void sys_tx_init(void) REENTRANT;
extern void appl_init(void) REENTRANT;
#ifdef CFG_ENHANCED_POWER_SAVING
extern void power_management_init(void);
#endif /* CFG_ENHANCED_POWER_SAVING */

/* Forwards */
#if defined(__ARM__) || defined(__LINUX__)
int main(void);
#else
void main(void);
#endif

#if (CFG_RXTX_SUPPORT_ENABLED && !defined(__BOOTLOADER__))
/* RX buffers (it's here so we can customize addr/size later in new code) */
#if defined(__MIPS__) && CFG_RAMAPP
STATIC uint8 rx_buffers[DEFAULT_RX_BUFFER_COUNT][DEFAULT_RX_BUFFER_SIZE] __attribute__ ((aligned (16)));
#elif defined(__LINUX__)
uint8* rx_buffers[DEFAULT_RX_BUFFER_COUNT];
#elif defined(__ARM__)
uint8 rx_buffers[DEFAULT_RX_BUFFER_COUNT][DEFAULT_RX_BUFFER_SIZE] __attribute__ ((section(".packet_buf"), aligned (32)));
#else
STATIC uint8 rx_buffers[DEFAULT_RX_BUFFER_COUNT][DEFAULT_RX_BUFFER_SIZE];
#endif
#endif /* CFG_RXTX_SUPPORT_ENABLED && !defined(__BOOTLOADER__) */


/* Function:
 *   main
 * Description:
 *   C startup function.
 * Parameters:
 *   None
 * Returns:
 *   None
 */
#if defined(__ARM__) || defined(__LINUX__)
int
main(void)
#else
void
main(void)
#endif
{
#if (CFG_RXTX_SUPPORT_ENABLED && !defined(__BOOTLOADER__))
    uint8 i;
#endif /* CFG_RXTX_SUPPORT_ENABLED && !defined(__BOOTLOADER__) */

#if defined(__LINUX__)
   if (sal_bde_init()) {
   	   sal_printf("bde module init fail\n");
   } 
#endif
    board_early_init();

    sal_init();	


    background_init();
#if CFG_TIMER_CALLBACK_SUPPORT    
    sys_timer_init();
#endif /* CFG_TIMER_CALLBACK_SUPPORT */


#if CFG_LINKCHANGE_CALLBACK_SUPPORT
    sys_linkchange_init();
#endif /* CFG_LINKCHANGE_CALLBACK_SUPPORT */
    
    system_utils_init();

#if CFG_UIP_STACK_ENABLED
    net_utils_init();
#endif /* CFG_UIP_STACK_ENABLED */

    if (board_init() == SYS_OK) {
#if defined(__LINUX__)
     for (i=0; i<DEFAULT_RX_BUFFER_COUNT; i++) {
	 	rx_buffers[i] = sal_dma_malloc(DEFAULT_RX_BUFFER_SIZE);
     }
#endif
#if (CFG_RXTX_SUPPORT_ENABLED && !defined(__BOOTLOADER__))
        sys_tx_init();
        sys_rx_init();
        for(i=0; i<DEFAULT_RX_BUFFER_COUNT; i++) {
            sys_rx_add_buffer(rx_buffers[i], DEFAULT_RX_BUFFER_SIZE);
        }
#endif /* CFG_RXTX_SUPPORT_ENABLED && !defined(__BOOTLOADER__) */

        appl_init();

#if CFG_ENHANCED_POWER_SAVING
        power_management_init();
#endif /* CFG_ENHANCED_POWER_SAVING */

#if CFG_PERSISTENCE_SUPPORT_ENABLED

        persistence_init();

        /*
         * Load current settings or factory defaults (if settings are invalid)
         */
        if (persistence_validate_current_settings()) {

            /* 
             * If all current settins are valid, just load them.
             */
            persistence_load_all_current_settings();

        } else {

            /* 
             * Part or all of data in current settings are not valid: 
             * use factory defaults for the invalid items.
             * First we load factory default for all items.
             */
#if CFG_CONSOLE_ENABLED
            sal_printf("Some of current saved settings are invalid. "
                       "Loading factory defaults.....\n");
#endif /* CFG_CONSOLE_ENABLED */
            persistence_restore_factory_defaults();

            /*
             * Then load current settings for valid items.
             */
            persistence_load_all_current_settings();

            /*
             * Loading done; save to flash (current settings) 
             */
            persistence_save_all_current_settings();
        }
#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */

#if defined(__UMDUMB__)
    board_vlan_type_set(VT_PORT_BASED);
    board_qos_type_set(QT_DOT1P_PRIORITY);
#endif
        board_late_init();
    }
#if CFG_CLI_ENABLED
    cli();
#else
    for(;;) POLL();
#endif
#ifdef __ARM__
    return 0;
#endif
}

