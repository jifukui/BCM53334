/*
 * $Id: accesscontrolcbk.c,v 1.2 Broadcom SDK $
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

#include "appl/ssp.h"
#include "utilcbk.h"
#include "utils/net.h"
#include "appl/persistence.h"
#include "../content/errormsg_htm.h"
#include "../content/sspmacro_access.h"
#include "../content/sspmacro_adminp.h"

extern BOOL logined_entries_valid[1];
extern uint32 logined_IPs[1];
extern unsigned int logined_accessed[1];
#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

SSP_HANDLER_RETVAL
ssphandler_accesscontrol_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    char *acctmp = NULL, *acc_tmp = NULL;
    char buf[5];
    int ip_mask, idx, good_ip, good_mask, ip_idx;
    int j;
    uint8 accip[4], maskip[4];
    uint8 accctrlip[MAX_ACCESSCONTROL_IP][4], accmaskip[MAX_ACCESSCONTROL_IP][4];
    BOOL acc_valid;

    if (cxt->type == SSP_HANDLER_REQ_COOKIE) {
        return SSP_HANDLER_RET_INTACT;
    }
    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {
    
        acctmp = (char *)(cxt->pairs[0].value);
        acc_tmp = (char *)&buf[0];
        if (cxt->count == 3) {
            acc_valid = FALSE;
            set_accessip_addr(acc_valid, 0, NULL, NULL);
        } else {
            acctmp = (char *)(cxt->pairs[1].value);
        
            /* merge Access Ctrl IP change request into a array */
            /* 0 is ip, 1 is mask */
            ip_mask = 0;
            idx = 0;
            good_ip = 0;
            good_mask = 0;
            ip_idx = 0;
            acc_valid = FALSE;
            while (*acctmp != '\0') {
                if (*acctmp == '.') {
                    acctmp++;
                    *acc_tmp = '\0';
                    if (ip_mask == 0) {
                        /* Set ip */
                        accip[idx] = sal_atoi(buf);
                    } else {
                       maskip[idx] = sal_atoi(buf);
                    }
                    idx++;

                    sal_memset(buf, 0, sizeof(char)*5);
                    acc_tmp = &buf[0];
                    } else if (*acctmp == '#') {
                        /* get all ip, change to get mask */
                        ip_mask = 1;
                        acctmp++;
                        *acc_tmp = '\0';
                        accip[idx] = sal_atoi(buf);
                        if (idx != 3) {
                            good_ip = 1;
                        }
                        idx = 0;
                        sal_memset(buf, 0, sizeof(char)*5);
                        acc_tmp = &buf[0];
                    } else if (*acctmp == '$') {
                        /* get all mask, change to get ip */
                        ip_mask = 0;
                        acctmp++;
                        *acc_tmp = '\0';
                        maskip[idx] = sal_atoi(buf);
                        if (idx != 3) {
                            good_mask = 1;
                        }
                        idx = 0;
                        sal_memset(buf, 0, sizeof(char)*5);
                        acc_tmp = &buf[0];
                        if (!(good_ip || good_mask)) {
                            for (j = 0; j < 4; j++) {
                                accctrlip[ip_idx][j] = accip[j];
                                accmaskip[ip_idx][j] = maskip[j];
                            }
                            acc_valid = TRUE;
                            ip_idx++;       
                        }
                        good_ip = 0;
                        good_mask = 0;
                        
                   } else {
                     *acc_tmp = *acctmp;
                     acc_tmp++;
                     acctmp++;
                   }
            } 
            set_accessip_addr(acc_valid, ip_idx, accctrlip, accmaskip);
        }
    }

    /* Save it to persistent medium */
#if CFG_PERSISTENCE_SUPPORT_ENABLED
    persistence_save_current_settings("accesscontrol");
#endif
    return SSP_HANDLER_RET_INTACT;
}


SSPLOOP_RETVAL
ssploop_access_tag_enable(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem) REENTRANT
{
    UNREFERENCED_PARAMETER(params);
    UNREFERENCED_PARAMETER(psmem);
    
    BOOL acc_valid;
    int acc_num;
    uint8 ip[MAX_ACCESSCONTROL_IP][4], mask[MAX_ACCESSCONTROL_IP][4];
    get_accessip_addr(&acc_valid, &acc_num, ip, mask);
    
    if (acc_valid == FALSE) {
        /* loop only once */
        return SSPLOOP_STOP;
    } else {
        return SSPLOOP_PROCEED;
    }
    return SSPLOOP_PROCEED;
}


SSP_HANDLER_RETVAL
ssphandler_adminpriv_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    BOOL valid;
    uint32 ip;

    if (cxt->type == SSP_HANDLER_REQ_COOKIE) {
        return SSP_HANDLER_RET_INTACT;
    }

    if (cxt->count == 0)
    {
        valid = FALSE;

        logined_entries_valid[0] = FALSE;
        logined_IPs[0] = 0;
        logined_accessed[0] = 0;
    } else {
        valid = TRUE;
            
        logined_entries_valid[0] = TRUE;
        ip = (BUF->srcipaddr.u8[0] & 0xFF) | ((BUF->srcipaddr.u8[1]) << 8) 
            | ((BUF->srcipaddr.u8[2]) << 16) | ((BUF->srcipaddr.u8[3]) << 24);
        logined_IPs[0] = ip;
        logined_accessed[0] = sal_get_ticks();
    }    
    
    set_adminpv(valid);
    /* Save it to persistent medium */
#if CFG_PERSISTENCE_SUPPORT_ENABLED
    persistence_save_current_settings("adminpriv");
#endif
    return SSP_HANDLER_RET_INTACT;
}


void
sspvar_access_tag_status(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT
{
    int idx = 0;
    BOOL acc_valid;
    int acc_num;
    uint8 ip[MAX_ACCESSCONTROL_IP][4], mask[MAX_ACCESSCONTROL_IP][4];

    UNREFERENCED_PARAMETER(psmem);
    if ((params[1] == SSPMACRO_ACCESS_IP) || (params[1] == SSPMACRO_ACCESS_MASK)) {
        idx = params[0];
    }
    get_accessip_addr(&acc_valid, &acc_num, ip, mask);

    switch (params[1]) {
        case SSPMACRO_ACCESS_ENABLE:
            if (acc_valid == TRUE) {
                sal_strcpy(ssputil_shared_buffer, "checked");
            } else {
                sal_strcpy(ssputil_shared_buffer, "");
            }
            break;
        case SSPMACRO_ACCESS_IP:
            if (idx < acc_num) {
                sal_sprintf(ssputil_shared_buffer, "%d.%d.%d.%d", ip[idx][0],ip[idx][1],ip[idx][2],ip[idx][3]);
            } else {
                sal_strcpy(ssputil_shared_buffer, "");
            }
            break;
        case SSPMACRO_ACCESS_MASK:
            if (idx < acc_num) {
                sal_sprintf(ssputil_shared_buffer, "%d.%d.%d.%d", mask[idx][0],mask[idx][1],mask[idx][2],mask[idx][3]);
            } else {
                sal_strcpy(ssputil_shared_buffer, "");
            }
            break;
        default:
//            PORT_DBG(("**** DEFAULT in sspvar_access_tag_status : params[0]=%d", params[0]));
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = "";
    }
    ret->type = SSPVAR_RET_STRING;
    ret->val_data.string = ssputil_shared_buffer;

    return;
}

void
sspvar_adminp_tag_status(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT
{
    BOOL valid;

    UNREFERENCED_PARAMETER(psmem);
    get_adminpv(&valid);

    switch (params[0]) {
        case SSPMACRO_ADMINP_ENABLE:
            if (valid == TRUE) {
                sal_strcpy(ssputil_shared_buffer, "checked");
            } else {
                sal_strcpy(ssputil_shared_buffer, "");
            }
            break;
        default:
//            PORT_DBG(("**** DEFAULT in sspvar_access_tag_status : params[0]=%d", params[0]));
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = "";
    }
    ret->type = SSPVAR_RET_STRING;
    ret->val_data.string = ssputil_shared_buffer;
    return;
}
