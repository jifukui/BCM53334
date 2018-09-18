/*
 * $Id: xcmd_cli.c,v 1.3 Broadcom SDK $
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
#include "system.h"

#ifdef CFG_XCOMMAND_INCLUDED

#include "appl/xcmd/xcmd_internal.h"
#include "xcmd_core.h"
#include "xc_input_cli.h"
#include "xc_input_buffer.h"
#include "xc_output_buf.h"

extern const XCNODE_CONTEXT xcmd_context_global;

/* #ifdef FEAT_BROADCOM_XCMD_XCLI */
#define XCLI_START_CONTEXT (&xcmd_context_global)
#define XCPRINT sal_printf


XCMD_ERROR
xcli_start_shell(const char *prompt, const char *user, const char *password)
{
    XCIN_CLI in;
    XCMD_ERROR r;
    char prompt_with_user[32];

    xcin_cli_init(&in);

    in.flags = xcli_auth_login(user, password);

    if (in.flags == 0) {

        XCPRINT("user name or password error\n");
        
        return XCMD_ERR_AUTH_FAIL;
    }

    sal_sprintf(prompt_with_user, "%s/%s:", prompt, user);
    
    r = xcmd_process_inputs(
            prompt_with_user , 
            (XCMD_INPUT *)&in,
            XCLI_START_CONTEXT
            );
    
    return r;
}

XCMD_ERROR
xcli_execute_commands_from_buffer(const char *buffer, unsigned int len)
{
    XCIN_BUFFER in;
    XCMD_ERROR r;
    
    xcin_buffer_init(&in, buffer, len);
    
    r = xcmd_process_inputs(
            "", 
            (XCMD_INPUT *)&in, 
            &xcmd_context_global
            );
    
    return r;
}

XCMD_ERROR
xcli_build_commands_to_buffer(char *buffer, unsigned int *plen)
{
    XCOUT_BUFFER out;
    XCMD_ERROR r;
    
    if (buffer == NULL || plen == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    xcout_buffer_init(&out, buffer, *plen);
    
    r = xcmd_context_generate(&xcmd_context_global, (XCMD_OUTPUT *)&out);
    out.buffer[out.curpos] = 0;
    *plen = out.curpos;
    
    return r;    
}

#endif /* CFG_XCOMMAND_INCLUDED */
