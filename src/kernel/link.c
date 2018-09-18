/*
 * $Id: link.c,v 1.10 Broadcom SDK $
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

 #ifdef __C51__
#ifdef CODE_USERCLASS
#pragma userclass (code = krnlink)
#endif /* CODE_USERCLASS */
#endif /* __C51__ */
 
#include "system.h"

#if CFG_LINKCHANGE_CALLBACK_SUPPORT

extern void sys_linkchange_init(void) REENTRANT;

STATIC SYS_LINKCHANGE_FUNC linkchange_handlers[CFG_MAX_REGISTERED_LINKCHANGE];
STATIC void *linkchange_args[CFG_MAX_REGISTERED_LINKCHANGE];
STATIC BOOL linkchange_status[BOARD_MAX_NUM_OF_PORTS + 1];

/* Forwards */
APISTATIC void sys_linkchange_timer(void *arg) REENTRANT;

BOOL 
APIFUNC(sys_register_linkchange)(SYS_LINKCHANGE_FUNC func, void *arg) REENTRANT
{
    uint8 i;
    
    SAL_ASSERT(func != NULL);
    SAL_DEBUGF(("sys_register_linkchange: %p\n", func));
    
    for(i=0; i<CFG_MAX_REGISTERED_LINKCHANGE; i++) {
        if (linkchange_handlers[i] == NULL) {
            linkchange_handlers[i] = func;
            linkchange_args[i] = arg;
            return TRUE;
        }
    }
    
    return FALSE;
}

void 
APIFUNC(sys_unregister_linkchange)(SYS_LINKCHANGE_FUNC func) REENTRANT
{
    uint8 i;
    
    SAL_ASSERT(func != NULL);
    SAL_DEBUGF(("sys_unregister_linkchange: %p\n", func));
    
    for(i=0; i<CFG_MAX_REGISTERED_LINKCHANGE; i++) {
        if (linkchange_handlers[i] == func) {
            linkchange_handlers[i] = NULL;
            linkchange_args[i] = NULL;
            return;
        }
    }
}

APISTATIC void 
APIFUNC(sys_linkchange_timer)(void *arg) REENTRANT
{
    uint16 uport;
    uint8 i;
    BOOL link;
    
    UNREFERENCED_PARAMETER(arg);
    
    SAL_UPORT_ITER(uport) {
        if (board_get_port_link_status(uport, &link) != SYS_OK) {
            SAL_ASSERT(FALSE);
            continue;
        }
        if (linkchange_status[uport] != link) {
            linkchange_status[uport] = link;
            for(i=0; i<CFG_MAX_REGISTERED_LINKCHANGE; i++) {
                SYS_LINKCHANGE_FUNC func = linkchange_handlers[i];
                if (func != NULL) {
                    (*func)(uport, link, linkchange_args[i]);
                }
            }
        } 
    }
}

void 
APIFUNC(sys_linkchange_init)(void) REENTRANT
{
    uint16 i;

    for(i=0; i<CFG_MAX_REGISTERED_LINKCHANGE; i++) {
        linkchange_handlers[i] = NULL;
    }
    
    SAL_UPORT_ITER(i) {
        linkchange_status[i] = FALSE;
    }
    
    timer_add(sys_linkchange_timer, NULL, CFG_LINKCHANGE_CHECK_INTERVAL);
}

#endif /* CFG_LINKCHANGE_CALLBACK_SUPPORT */

