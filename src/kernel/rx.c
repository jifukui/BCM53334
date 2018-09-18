/*
 * $Id: rx.c,v 1.5 Broadcom SDK $
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
#pragma userclass (code = krnrx)
#endif /* CODE_USERCLASS */
#endif /* __C51__ */

#include "system.h"

#if (CFG_RXTX_SUPPORT_ENABLED && !defined(__BOOTLOADER__))

typedef struct rx_cbk_s {
    uint8 priority;
    SYS_RX_CBK_FUNC cbk;
    void *cookie;
    uint16 flags;
    struct rx_cbk_s *next;
} rx_cbk_t;

STATIC rx_cbk_t   rx_cbks[CFG_MAX_REGISTERED_RX_CBKS];
STATIC uint8      rx_cbk_count;
STATIC rx_cbk_t * rx_cbk_list;
STATIC sys_pkt_t *rx_pkt_pool;

/* Forwards */
void APIFUNC(sys_rx_init)(void) REENTRANT;
APISTATIC void sys_rx_handler(sys_pkt_t *pkt) REENTRANT;
APISTATIC sys_error_t sys_rx_refill(sys_pkt_t *pkt) REENTRANT;

sys_error_t 
APIFUNC(sys_rx_register)(
    SYS_RX_CBK_FUNC callback, 
    uint8 priority, 
    void *cookie,
    uint16 flags) REENTRANT
{
    uint8 i;
    rx_cbk_t *cbk = NULL;
    
    if (callback == NULL) {
        return SYS_ERR_PARAMETER;
    }
    
    for(i=0; i<CFG_MAX_REGISTERED_RX_CBKS; i++) {
        if (rx_cbks[i].cbk == NULL) {
            cbk = &rx_cbks[i];
            break;
        }
    }
    SAL_ASSERT(cbk != NULL);
    if (cbk == NULL) {
        return SYS_ERR_FULL;
    }
    
    cbk->priority = priority;
    cbk->cbk = callback;
    cbk->cookie = cookie;
    cbk->flags = flags;
    cbk->next = NULL;
    
    if (rx_cbk_list == NULL) {
        rx_cbk_list = cbk;
    } else {
        rx_cbk_t *cur = rx_cbk_list;
        rx_cbk_t *prev = NULL;
        while(cur != NULL) {
            if (cur->priority >= cbk->priority) {
                cbk->next = cur;
                if (prev == NULL) {
                    rx_cbk_list = cbk;
                } else {
                    prev->next = cbk;
                }
                break;
            }
            prev = cur;
            cur = cur->next;
        }
        if (cur == NULL) {
            prev->next = cbk;
        }
    }
    
    rx_cbk_count++;
    if (rx_cbk_count == 1) {
        sys_pkt_t *pkt;
        while(rx_pkt_pool != NULL) {
            pkt = rx_pkt_pool;
            rx_pkt_pool = rx_pkt_pool->next;
            if (board_rx_fill_buffer(pkt) != SYS_OK) {
                SAL_ASSERT(FALSE);
                sal_free(pkt);
            }
        }
    }
    
    return SYS_OK;
}
    
sys_error_t
APIFUNC(sys_rx_unregister)(SYS_RX_CBK_FUNC callback) REENTRANT
{
    rx_cbk_t *cur = rx_cbk_list;
    rx_cbk_t *prev = NULL;
    while(cur != NULL) {
        if (cur->cbk == callback) {
            if (prev == NULL) {
                rx_cbk_list = cur->next;
            } else {
                prev->next = cur->next;
            }
            cur->cbk = NULL;
            rx_cbk_count--;
            return SYS_OK;
        }
        prev = cur;
        cur = cur->next;
    }    
    
    return SYS_ERR_PARAMETER;
}

APISTATIC sys_error_t
APIFUNC(sys_rx_refill)(sys_pkt_t *pkt) REENTRANT
{
    if (rx_cbk_count > 0) {
        return board_rx_fill_buffer(pkt);
    } else {
        pkt->next = rx_pkt_pool;
        rx_pkt_pool = pkt;
    }
    
    return SYS_OK;
}

APISTATIC void
APIFUNC(sys_rx_handler)(sys_pkt_t *pkt) REENTRANT
{
    rx_cbk_t *cur = rx_cbk_list;
    uint16 flags;
    sys_rx_t r;
    
    SAL_ASSERT(pkt != NULL);
    
    flags = pkt->flags;

    while(cur != NULL) {

        SAL_ASSERT(cur->cbk != NULL);
        
        if (flags & SYS_RX_FLAG_TRUNCATED) {
            if (!(cur->flags & SYS_RX_REGISTER_FLAG_ACCEPT_TRUNCATED_PKT)) {
                cur = cur->next;
                continue;
            }
        }

        if ((flags & (SYS_RX_FLAG_ERROR_CRC | SYS_RX_FLAG_ERROR_OTHER))) {
            if (!(cur->flags & SYS_RX_REGISTER_FLAG_ACCEPT_TRUNCATED_PKT)) {
                cur = cur->next;
                continue;
            }
        }

        r = (*cur->cbk)(pkt, cur->cookie);
        
        switch(r) {

        case SYS_RX_HANDLED:
            board_rx_fill_buffer(pkt);
            return;

        case SYS_RX_HANDLED_AND_OWNED:
            return;

        default:
            /* Invalid return code */
            SAL_ASSERT(FALSE);
            /* Fall through (if assertion not enabled) */

        case SYS_RX_NOT_HANDLED:
            cur = cur->next;
            continue;
        }
    }    
    
    /* If no one handles it */
    sys_rx_refill(pkt);
}

void
APIFUNC(sys_rx_free_packet)(sys_pkt_t *pkt) REENTRANT
{
    if (pkt == NULL) {
        return;
    }
    
    sys_rx_refill(pkt);
}

void
APIFUNC(sys_rx_add_buffer)(uint8 *buffer, uint16 size) REENTRANT
{
    sys_pkt_t *pkt;
    sys_error_t r;
    
    SAL_ASSERT(buffer != NULL && size != 0);
    if (buffer == NULL || size == 0) {
        return;
    }
    
    pkt = (sys_pkt_t *)sal_malloc(sizeof(sys_pkt_t));
    SAL_ASSERT(pkt != NULL);
    if (pkt == NULL) {
        return;
    }
    pkt->pkt_data = buffer;
    pkt->buf_len = size;

    r = sys_rx_refill(pkt);
    SAL_ASSERT(r == SYS_OK);
    if (r != SYS_OK) {
        sal_free(pkt);
    }
}

void
APIFUNC(sys_rx_init)(void) REENTRANT
{
    uint8 i;
    
    /* Initialize global variables */
    rx_cbk_list = NULL;
    rx_pkt_pool = NULL;
    rx_cbk_count = 0;
    for(i=0; i<CFG_MAX_REGISTERED_RX_CBKS; i++) {
        rx_cbks[i].cbk = NULL;
    }

    /* Register switch RX callback */
    board_rx_set_handler(sys_rx_handler);
}

#endif /* CFG_RXTX_SUPPORT_ENABLED && !defined(__BOOTLOADER__) */

