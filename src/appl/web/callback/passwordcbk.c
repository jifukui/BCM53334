/*
 * $Id: passwordcbk.c,v 1.7 Broadcom SDK $
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

#ifdef CFG_SYSTEM_PASSWORD_INCLUDED
/*
 * For performace on checking, we save the current corresponding cookie value
 */
#define MAX_COOKIE_LEN      (24)
static char cached_cookie_value[MAX_SSS_USER_COUNT][MAX_COOKIE_LEN + 1];
/* static BOOL cached_cookie_initialized = FALSE; */

/* Forwards */
/* static void make_cookie_cache(int user_id); */

SSP_HANDLER_RETVAL
ssphandler_password_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    char lpwd[MAX_USERNAME_LEN + 1];
    int  user_id = 0;

    if (cxt->type == SSP_HANDLER_REQ_COOKIE) {
        return SSP_HANDLER_RET_INTACT;
    }

    if (cxt->type != SSP_HANDLER_QUERY_STRINGS) {
        return SSP_HANDLER_RET_INTACT;
    }

    if (cxt->count != 3) {
        return SSP_HANDLER_RET_INTACT;
    }

    get_login_password(lpwd, sizeof(lpwd));

    if (sal_strcmp(cxt->pairs[0].value, lpwd)) {
          webutil_show_error(
            cxt, psmem,
            "Change Password",
            "Incorrect old password!",
            err_button_retry,
            err_action_back
            );
        return SSP_HANDLER_RET_MODIFIED;
    }

    if (sal_strcmp(cxt->pairs[1].value, cxt->pairs[2].value)) {
          webutil_show_error(
            cxt, psmem,
            "Change Password",
            "New password unmatched!",
            err_button_retry,
            err_action_back
            );
        return SSP_HANDLER_RET_MODIFIED;
    }

    if (sal_strlen(cxt->pairs[1].value) == 0) {
        sal_sprintf(ssputil_shared_buffer,
                "Can't be without password !");
        webutil_show_error(
            cxt, psmem,
            "Change Password",
            ssputil_shared_buffer,
            err_button_retry,
            err_action_back
            );
        return SSP_HANDLER_RET_MODIFIED;
    }

    if (sal_strlen(cxt->pairs[1].value) > MAX_PASSWORD_LEN) {
        sal_sprintf(ssputil_shared_buffer,
                "Password cannot exceed %d characters!",
                MAX_PASSWORD_LEN);
        webutil_show_error(
            cxt, psmem,
            "Change Password",
            ssputil_shared_buffer,
            err_button_retry,
            err_action_back
            );
        return SSP_HANDLER_RET_MODIFIED;
    }

    set_login_password(cxt->pairs[1].value);

    /* Forced cached cookie to be re-calculated */
    cached_cookie_value[user_id][0] = 0;
    /* make_cookie_cache(user_id); */

    /* Save it to persistent medium */
#if CFG_PERSISTENCE_SUPPORT_ENABLED
    persistence_save_current_settings("password");
#endif

    return SSP_HANDLER_RET_INTACT;
}

#endif  /* CFG_SYSTEM_PASSWORD_INCLUDED */

