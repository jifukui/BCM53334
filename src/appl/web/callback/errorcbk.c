/*
 * $Id: errorcbk.c,v 1.2 Broadcom SDK $
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

const char err_button_retry[] = " Retry ";
const char err_action_back[] = "history.go(-1)";

void
sspvar_error_tag_title(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
    char *str = ssputil_psmem_get(psmem, sspvar_error_tag_title);
    if (str == NULL) {
        return;
    }
    
    ret->type = SSPVAR_RET_STRING;
    ret->val_data.string = str;
}

void
sspvar_error_tag_message(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
    char *str = ssputil_psmem_get(psmem, sspvar_error_tag_message);
    if (str == NULL) {
        return;
    }
    
    ret->type = SSPVAR_RET_STRING;
    ret->val_data.string = str;
}

void
sspvar_error_tag_button(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
    char *str = ssputil_psmem_get(psmem, sspvar_error_tag_button);
    if (str == NULL) {
        return;
    }
    
    ret->type = SSPVAR_RET_STRING;
    ret->val_data.string = str;
}

void
sspvar_error_tag_action(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
    char *str = ssputil_psmem_get(psmem, sspvar_error_tag_action);
    if (str == NULL) {
        return;
    }
    
    ret->type = SSPVAR_RET_STRING;
    ret->val_data.string = str;
}

void
webutil_show_error(SSP_HANDLER_CONTEXT *cxt, 
                   SSP_PSMH psmem,
                   const char *title,
                   const char *message,
                   const char *button,
                   const char *action
                   )
{
    char *str;
    
    if (cxt == NULL) {
        return;
    }
    
    if (title != NULL) {
        str = ssputil_psmem_alloc(
                psmem, 
                sspvar_error_tag_title, 
                sal_strlen(title) + 1
                );
        sal_strcpy(str, title);
    }
    
    if (message != NULL) {
        str = ssputil_psmem_alloc(
                psmem, 
                sspvar_error_tag_message, 
                sal_strlen(message) + 1
                );
        sal_strcpy(str, message);
    }
    
    if (button != NULL) {
        str = ssputil_psmem_alloc(
                psmem, 
                sspvar_error_tag_button, 
                sal_strlen(button) + 1
                );
        sal_strcpy(str, button);
    }
    
    if (action != NULL) {
        str = ssputil_psmem_alloc(
                psmem, 
                sspvar_error_tag_action, 
                sal_strlen(action) + 1
                );
        sal_strcpy(str, action);
    }
    
    cxt->page = ssputil_locate_file(psmem, "/errormsg.htm", NULL);
    cxt->flags &= ~SSPF_FORCE_CACHE;
    cxt->flags |= SSPF_NO_CACHE;
}
