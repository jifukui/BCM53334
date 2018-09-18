/*
 * $Id: utilcbk.h,v 1.2 Broadcom SDK $
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

#ifndef _UTILCBK_H_
#define _UTILCBK_H_

#include "appl/ssp.h" 

/* Utility: show web page of error messages.
 *          Used only in SSP handlers. 
 *          Must return SSP_HANDLER_RET_MODIFIED after calling this.
 */
void webutil_show_error(SSP_HANDLER_CONTEXT *cxt, 
           SSP_PSMH psmem,
           const char *title,
           const char *message,
           const char *button,
           const char *action
           );
           
/* Common string used in webutil_show_error() */
extern const char err_button_retry[];
extern const char err_action_back[];

#define MACADDR_STR_LEN 18              /* Formatted MAC address */
#define IPADDR_STR_LEN  16              /* Formatted IP address */

/*
 * util_format_macaddr requires a buffer of 18 bytes minimum.
 * It does not use sprintf so it can be called from an interrupt context.
 */
#ifdef ACL_MAC_SUPPORT
void utilcbk_format_macaddr(char buf[MACADDR_STR_LEN], unsigned char *macaddr);
#endif

/*
 * util_format_ipaddr requires a buffer of 16 bytes minimum.
 * It does not use sprintf so it can be called from an interrupt context.
 */
void utilcbk_format_ipaddr(char buf[IPADDR_STR_LEN], unsigned char * ipaddr);

/*
 * check whether ip mask is correct or not
 */
int utilcbk_ip_mask_check(unsigned char *ipmask);

#if 0
/*
 * Gets IP address in numberic form for the current HTTP session
 */
in_addr_t utilcbk_get_ip_address(SSP_PSMH psmem);
#endif

#endif /* _UTILCBK_H_ */
