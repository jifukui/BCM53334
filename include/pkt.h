/*
 * $Id: pkt.h,v 1.3 Broadcom SDK $
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

#ifndef _PKT_H_
#define _PKT_H_

/* RX flags (read by handler) */
#define SYS_RX_FLAG_TIMESTAMP                       (1 << 0)
#define SYS_RX_FLAG_COS                             (1 << 1)
#define SYS_RX_FLAG_TRUNCATED                       (1 << 2)
#define SYS_RX_FLAG_ERROR_CRC                       (1 << 3)
#define SYS_RX_FLAG_ERROR_OTHER                     (1 << 4)

/* TX flags (used by caller) */
#define SYS_TX_FLAG_TIMESTAMP_REQUEST               (1 << 8)
#define SYS_TX_FLAG_USE_UNTAG_PORT_LIST             (1 << 9) 

/* VLAN tagging mode (Used if tx_uplist is empty) */
#define SYS_TX_TAG_MODE_FOLLOW_SWITCH_RULES         (0)
#define SYS_TX_TAG_MODE_UNTAG_ALL                   (1)
#define SYS_TX_TAG_MODE_TAG_ALL                     (2)

typedef struct sys_pkt_s {
    uint8 * pkt_data;
    uint16  pkt_len;
    uint16  buf_len;
    uint16  flags;
    uint16  cos;
            
    uint16  rx_src_uport;
    uint32  rx_timestamp;
            
    uint8   tx_uplist[MAX_UPLIST_WIDTH];    /* Follow switch ARL if empty */
    uint8   tx_untag_uplist[MAX_UPLIST_WIDTH];   /* If USE_UNTAG_PORT_BITMAP set */
    uint8   tx_tag_mode;                    /* Only if tx_uplist is empty */
    void *  cookie;
            
    uint32  reserved1;
    uint32  reserved2;
    uint32  reserved3;
    uint32  reserved4;
            
    uint32  internal0;  /* Used internally in kernel */
    uint32  internal1;  /* Used internally in board */
    
    struct sys_pkt_s *  next;
} sys_pkt_t;

#endif /* _PKT_H_ */
