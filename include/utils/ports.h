/*
 * $Id: ports.h,v 1.7 Broadcom SDK $
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

#ifndef _UTILS_PORTS_H_
#define _UTILS_PORTS_H_


/* Port description */
#define WEB_PORT_DESC_LEN    (20)


/* Check if an uplist is empty; return SYS_ERR_FALSE if not */
extern sys_error_t uplist_is_empty(uint8 *uplist) REENTRANT;

/* Clear an uplist */
extern sys_error_t uplist_clear(uint8 *uplist) REENTRANT;

/* Check if a port is in an uplist; return SYS_ERR_FALSE if not */
extern sys_error_t uplist_port_matched(const uint8 *uplist, uint16 uport) REENTRANT;

/* Add a uport to an uplist */
extern sys_error_t uplist_port_add(uint8 *uplist, uint16 uport) REENTRANT;

/* Remove a uport from an uplist */
extern sys_error_t uplist_port_remove(uint8 *uplist, uint16 uport) REENTRANT;

typedef enum uplist_op_s{
    UPLIST_OP_COPY = 0,
    UPLIST_OP_OR,
    UPLIST_OP_AND,
    UPLIST_OP_EQU
} uplist_op_t;

/* COPY/OR/AND two uplists */
extern sys_error_t uplist_manipulate(uint8 *dst_uplist, uint8 *src_uplist, uplist_op_t op) REENTRANT;


extern sys_error_t get_port_desc(uint16 uport, char *buf, uint8 len);
extern sys_error_t set_port_desc(uint16 uport, const char *buf);



#endif /* _UTILS_PORTS_H_ */
