/*
 * $Id: ports_utils.c,v 1.10 Broadcom SDK $
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
#pragma userclass (code = putl)
#endif /* CODE_USERCLASS */
#endif /* __C51__ */

#include "system.h"
#include "utils/ports.h"

/*
 * PORT description
 */

static char port_desc[BOARD_MAX_NUM_OF_PORTS][WEB_PORT_DESC_LEN+1] = {{0}};

sys_error_t
APIFUNC(uplist_is_empty)(uint8 *uplist) REENTRANT
{
    uint8 index;
    if (uplist == NULL) {
        return SYS_ERR_PARAMETER;
    }
    for(index=0; index<MAX_UPLIST_WIDTH; index++) {
        if (uplist[index] != 0) {
            return SYS_ERR_FALSE;
        }
    }
    return SYS_OK;
}

sys_error_t
APIFUNC(uplist_clear)(uint8 *uplist) REENTRANT
{
    uint8 index;
    if (uplist == NULL) {
        return SYS_ERR_PARAMETER;
    }
    for(index=0; index<MAX_UPLIST_WIDTH; index++) {
        uplist[index] = 0;
    }
    return SYS_OK;
}

sys_error_t
APIFUNC(uplist_port_matched)(const uint8 *uplist, uint16 uport) REENTRANT
{
    if (uplist == NULL) {
        return SYS_ERR_PARAMETER;
    }
    if (SAL_UPORT_IS_NOT_VALID(uport)) {
        return SYS_ERR_PARAMETER;
    }

    uport = SAL_UPORT_TO_ZUPORT(uport);
    
#if (MAX_UPLIST_WIDTH == 1) /* Shortcut to avoid mul/div */
    if (uplist[0] & (1 << uport)) {
        return SYS_OK;
    }
#else
    if (uplist[uport / 8] & (1 << (uport % 8))) {
        return SYS_OK;
    }
#endif
    
    return SYS_ERR_FALSE;
}

sys_error_t
APIFUNC(uplist_port_add)(uint8 *uplist, uint16 uport) REENTRANT
{
    if (uplist == NULL) {
        return SYS_ERR_PARAMETER;
    }
    if (SAL_UPORT_IS_NOT_VALID(uport)) {
        return SYS_ERR_PARAMETER;
    }

    uport = SAL_UPORT_TO_ZUPORT(uport);
    
#if (MAX_UPLIST_WIDTH == 1) /* Shortcut to avoid mul/div */
    uplist[0] |= (1 << uport);
#else
    uplist[uport / 8] |= (1 << (uport % 8));
#endif
    
    return SYS_OK;
}

sys_error_t
APIFUNC(uplist_port_remove)(uint8 *uplist, uint16 uport) REENTRANT
{
    if (uplist == NULL) {
        return SYS_ERR_PARAMETER;
    }
    if (SAL_UPORT_IS_NOT_VALID(uport)) {
        return SYS_ERR_PARAMETER;
    }

    uport = SAL_UPORT_TO_ZUPORT(uport);
#if (MAX_UPLIST_WIDTH == 1) /* Shortcut to avoid mul/div */
    uplist[0] &= ~(1 << uport);
#else
    uplist[uport / 8] &= ~(1 << (uport % 8));
#endif
    
    return SYS_OK;
}

sys_error_t
APIFUNC(uplist_manipulate)(uint8 *dst_uplist, uint8 *src_uplist, uplist_op_t op) REENTRANT
{
    uint8 uport;
    if (dst_uplist == NULL) {
        return SYS_ERR_PARAMETER;
    }

    if (src_uplist == NULL) {
        return SYS_ERR_PARAMETER;
    }

    for(uport=0; uport<MAX_UPLIST_WIDTH; uport++) {
        if(op == UPLIST_OP_COPY) {
            dst_uplist[uport] = src_uplist[uport];
        } else if (op == UPLIST_OP_OR) {
            dst_uplist[uport] |= src_uplist[uport];
        } else if (op == UPLIST_OP_AND) {
            dst_uplist[uport] &= src_uplist[uport];
        } else if (op == UPLIST_OP_EQU) {
            if (dst_uplist[uport] != src_uplist[uport]) {
                return SYS_ERR;
            }
        } else {
            return SYS_ERR_PARAMETER;
        }
    }
    return SYS_OK;
}






sys_error_t
get_port_desc(uint16 uport, char *buf, uint8 len)
{
    if (buf == NULL || len == 0) {
        return SYS_ERR_PARAMETER;
    }

    uport = SAL_UPORT_TO_ZUPORT(uport);
    
    if (sal_strlen(port_desc[uport]) > len) {
        return SYS_ERR_PARAMETER;
    }

    sal_strcpy(buf, port_desc[uport]);

    return SYS_OK;
}

sys_error_t
set_port_desc(uint16 uport, const char *desc)
{
    if (desc == NULL) {
        return SYS_ERR_PARAMETER;
    }

    uport = SAL_UPORT_TO_ZUPORT(uport);

    if (sal_strlen(desc) > WEB_PORT_DESC_LEN) {
        return SYS_ERR_PARAMETER;
    }
    sal_strcpy(port_desc[uport], desc);

    return SYS_OK;
}
