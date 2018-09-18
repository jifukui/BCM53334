/*
 * $Id: brd_rxtx.c,v 1.8 Broadcom SDK $
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
#pragma userclass (code = brdimpl)
#endif /* CODE_USERCLASS */
#endif /* __C51__ */

#include "system.h"
#include "utils/ports.h"

/*
 * Typical implementation of certain board functions 
 * 
 * Note: Only applicable for single-unit board.
 */

#if CFG_RXTX_SUPPORT_ENABLED

STATIC BOARD_RX_HANDLER rx_handler;
STATIC sys_pkt_t *rx_syspkt_list;
STATIC soc_rx_packet_t *rx_pkt_list;
STATIC soc_tx_packet_t *tx_pkt_list;

/* Forwards */
APISTATIC void brdimpl_rx_handler(soc_rx_packet_t *pkt) REENTRANT;
APISTATIC void brdimpl_tx_cbk(struct soc_tx_packet_s *pkt) REENTRANT;

APISTATIC void
APIFUNC(brdimpl_rx_handler)(soc_rx_packet_t *pkt) REENTRANT
{
    uint16 flags0, flags1;
    sys_pkt_t *wrap;
    
    SAL_ASSERT(pkt != NULL);
    if (pkt == NULL) {
        return;
    }
    
    wrap = rx_syspkt_list;
    SAL_ASSERT(wrap != NULL);
    if (wrap == NULL) {
        return;
    }
    rx_syspkt_list = wrap->next;

    flags0 = pkt->flags;

    wrap->pkt_data = pkt->buffer;
    wrap->pkt_len = pkt->pktlen;
    wrap->buf_len = pkt->buflen;
    board_lport_to_uport(0, pkt->lport, &wrap->rx_src_uport);
    wrap->next = NULL;
    flags1 = 0;
    if (flags0 & SOC_RX_FLAG_TIMESTAMP) {
        flags1 |= SYS_RX_FLAG_TIMESTAMP;
        wrap->rx_timestamp = pkt->timestamp;
    }
    if (flags0 & SOC_RX_FLAG_TRAFFIC_CLASS) {
        flags1 |= SYS_RX_FLAG_COS;
        wrap->cos = pkt->traffic_class;
    }
    if (flags0 & SOC_RX_FLAG_TRUNCATED) {
        flags1 |= SYS_RX_FLAG_TRUNCATED;
    }
    if (flags0 & SOC_RX_FLAG_ERROR_CRC) {
        flags1 |= SYS_RX_FLAG_ERROR_CRC;
    }
    if (flags0 & SOC_RX_FLAG_ERROR_OTHER) {
        flags1 |= SYS_RX_FLAG_ERROR_OTHER;
    }
    wrap->flags = flags1;
    
    pkt->next = rx_pkt_list;
    rx_pkt_list = pkt;
    
    (*rx_handler)(wrap);
}

sys_error_t
APIFUNC(brdimpl_rx_set_handler)(BOARD_RX_HANDLER fn) REENTRANT
{
    soc_switch_t *soc;

    SAL_ASSERT(board_unit_count() == 1);
    SAL_ASSERT(rx_handler == NULL);
    if (rx_handler != NULL) {
        return SYS_ERR_STATE;
    }
    
    if (fn == NULL) {
        return SYS_ERR_PARAMETER;
    }
    
    soc = board_get_soc_by_unit(0);
    SAL_ASSERT(soc != NULL);
    
    rx_handler = fn;
    return (*soc->rx_set_handler)(0, brdimpl_rx_handler, FALSE);
}
    
sys_error_t 
APIFUNC(brdimpl_rx_fill_buffer)(sys_pkt_t *pkt) REENTRANT
{
    soc_switch_t *soc;
    soc_rx_packet_t *spkt;
    
    SAL_ASSERT(board_unit_count() == 1);
    if (pkt == NULL) {
        return SYS_ERR_PARAMETER;
    }

    spkt = rx_pkt_list;
    if (spkt == NULL) {
        spkt = (soc_rx_packet_t *)sal_malloc(sizeof(soc_rx_packet_t));
        if (spkt == NULL) {
            return SYS_ERR_OUT_OF_RESOURCE;
        }
    } else {
        rx_pkt_list = spkt->next;
    }
    
    spkt->buffer = pkt->pkt_data;
    spkt->buflen = pkt->buf_len;

    pkt->next = rx_syspkt_list;
    rx_syspkt_list = pkt;
    
    soc = board_get_soc_by_unit(0);
    SAL_ASSERT(soc != NULL);

    return (*soc->rx_fill_buffer)(0, spkt);
}

APISTATIC void
APIFUNC(brdimpl_tx_cbk)(struct soc_tx_packet_s *pkt) REENTRANT
{
    sys_pkt_t *kpkt;
    BOARD_TX_CALLBACK cbk;
    sys_error_t r;

    SAL_ASSERT(pkt != NULL);
    if (pkt == NULL) {
        return;
    }

    kpkt = (sys_pkt_t *)pkt->cookie;
    cbk = (BOARD_TX_CALLBACK)kpkt->internal1;

    /* Get status */
    r = pkt->status;

    /* Return pkt to pool */        
    pkt->next = tx_pkt_list;
    tx_pkt_list = pkt;

    /* Invoke callback */ 
    (*cbk)(kpkt, r);
}

sys_error_t
APIFUNC(brdimpl_tx)(sys_pkt_t *pkt, BOARD_TX_CALLBACK cbk) REENTRANT
{
    soc_tx_packet_t *spkt;
    soc_switch_t *soc;
    uint16 flags = 0;
    sys_error_t r;

    SAL_ASSERT(board_unit_count() == 1);
    
    if (pkt == NULL || cbk == NULL) {
        return SYS_ERR_PARAMETER;
    }
    
    soc = board_get_soc_by_unit(0);
    SAL_ASSERT(soc != NULL);

    spkt = tx_pkt_list;
    if (spkt == NULL) {
        spkt = (soc_tx_packet_t *)sal_malloc(sizeof(soc_tx_packet_t));
        if (spkt == NULL) {
            return SYS_ERR_OUT_OF_RESOURCE;
        }
    } else {
        tx_pkt_list = spkt->next;
    }

    sal_memset(spkt, 0, sizeof(soc_tx_packet_t));
    spkt->buffer = pkt->pkt_data;
    spkt->pktlen = pkt->pkt_len;
    board_uplist_to_lpbmp(pkt->tx_uplist, 0, &spkt->port_bitmap);
    spkt->traffic_class = pkt->cos;
    if (pkt->flags & SYS_TX_FLAG_TIMESTAMP_REQUEST) {
        flags |= SOC_TX_FLAG_TIMESTAMP_REQUEST;
    }
    if (pkt->flags & SYS_TX_FLAG_USE_UNTAG_PORT_LIST) {
        flags |= SOC_TX_FLAG_USE_UNTAG_PORT_BITMAP;
        board_uplist_to_lpbmp(pkt->tx_untag_uplist, 0, &spkt->untag_bitmap);
    } else {
        flags &= ~SOC_TX_FLAG_USE_UNTAG_PORT_BITMAP;
        BCM_PBMP_CLEAR(spkt->untag_bitmap);
    }
    if (uplist_is_empty(pkt->tx_uplist) == SYS_OK) {
        switch(pkt->tx_tag_mode) {
        case SYS_TX_TAG_MODE_FOLLOW_SWITCH_RULES:
            spkt->tag_mode = SOC_TX_TAG_MODE_FOLLOW_SWITCH_RULES; 
            break;
        case SYS_TX_TAG_MODE_UNTAG_ALL:
            spkt->tag_mode = SOC_TX_TAG_MODE_UNTAG_ALL; 
            break;
        case SYS_TX_TAG_MODE_TAG_ALL:
            spkt->tag_mode = SOC_TX_TAG_MODE_TAG_ALL; 
            break;
        default: 
            spkt->tag_mode = SOC_TX_TAG_MODE_FOLLOW_SWITCH_RULES; 
            break;
        }
    }
    spkt->flags = flags;
    spkt->callback = brdimpl_tx_cbk;
    spkt->cookie = (void *)pkt;
    pkt->internal1 = (uint32)cbk;
    
    r = (*soc->tx)(0, spkt);
    if (r != SYS_OK) {
        spkt->next = tx_pkt_list;
        tx_pkt_list = spkt;
    }
    
    return r;
}

void
APIFUNC(brdimpl_rxtx_init)(void) REENTRANT
{
    rx_handler = NULL;
    rx_syspkt_list = NULL;
    rx_pkt_list = NULL;
    tx_pkt_list = NULL;
}

#endif /* CFG_RXTX_SUPPORT_ENABLED */
