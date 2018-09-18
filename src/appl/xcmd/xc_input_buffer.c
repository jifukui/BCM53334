/*
 * $Id: xc_input_buffer.c,v 1.3 Broadcom SDK $
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
#include "xc_input_buffer.h"

#define THIS ((XCIN_BUFFER *)pstream)
static const char CSTR_EXIT[] = "exit";

static int
char_is_eol(char ch)
{
    if (ch == '\r' || 
        ch == '\n' ||
        ch == '\0') {
        
        return 1;
    }
    
    return 0;
}

static void
skip_line(void *pstream)
{
    while(THIS->curpos < THIS->maxlen && 
          !char_is_eol(THIS->buffer[THIS->curpos])) {
        THIS->curpos++;
    }
    while(THIS->curpos < THIS->maxlen &&  
          char_is_eol(THIS->buffer[THIS->curpos])) {

        THIS->curpos++;
    }
}

static XCMD_ERROR
get_line(void *pstream, const char **pline, unsigned int *plen, XCMDI_OP *pop)
{
    if (pstream == NULL || pline == NULL || plen == NULL || pop == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_STREAM_CALL;
    }
    
    if (THIS->depth < 0) {
        return XCMD_ERR_INTERNAL_INVALID_STREAM_CALL;
    }
    
    for(;;) {
        unsigned int d;
        
        if (THIS->curpos == THIS->maxlen) {
            /* No more data */
            *plen = 0;
            return XCMD_ERR_EXIT;
        }
        
        /* Check depth by spaces */
        d = 0;
        while(THIS->curpos + d < THIS->maxlen && 
              THIS->buffer[THIS->curpos + d] == ' ') {
            d++;
        }
        if (d != THIS->depth) {
            
            /* If spaces < current depth, send a EXIT signal */
            if (d < THIS->depth) {
                
                /* Emulate we got a 'exit' */
                *pline = CSTR_EXIT;
                *plen = sal_strlen(CSTR_EXIT);
                *pop = XCMDI_OP_EXECUTE;
                return XCMD_ERR_OK;
            }
            
            skip_line(pstream);
            continue;
        }
        
        THIS->curpos += d;
        if (THIS->curpos  == THIS->maxlen) {
            /* No more data */
            *plen = 0;
            return XCMD_ERR_EXIT;
        }
        
        /* Check comment line */
        if (THIS->buffer[THIS->curpos] == DEFAULT_CFGFILE_COMMENT_BEGIN) {
            skip_line(pstream);
            continue;
        }
        
        /* Check empty line */
        if (char_is_eol(THIS->buffer[THIS->curpos])) {
            skip_line(pstream);
            continue;
        }

        /* Calculate how many bytes we can provide */
        *plen = 0;
        while(THIS->curpos + (*plen)< THIS->maxlen && 
              !char_is_eol(THIS->buffer[THIS->curpos + (*plen)])) {
                
            (*plen)++;
        }
        while(THIS->curpos + (*plen) < THIS->maxlen && 
              char_is_eol(THIS->buffer[THIS->curpos + (*plen)])) {
                
            (*plen)++;
        }
        
        /* Return the data */
        *pline = &THIS->buffer[THIS->curpos];
        *pop = XCMDI_OP_EXECUTE;

        /* Advance data pointer */
        THIS->curpos += *plen;
        
        break;
    }

    return XCMD_ERR_OK;
}

static XCMD_ERROR
report_error(void *pstream, XCMD_ERROR err, const char *line, unsigned int len)
{
    return XCMD_ERR_OK;
}

static XCMD_ERROR
open(void *pstream, const char *prompt)
{
    if (pstream == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    THIS->depth++;

    return XCMD_ERR_OK;
}

static XCMD_ERROR
close(void *pstream)
{
    if (pstream == NULL || THIS->depth < 0) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    THIS->depth--;

    return XCMD_ERR_OK;
}

XCMD_ERROR
xcin_buffer_init(XCIN_BUFFER *ps, const char *buffer, unsigned int len)
{
    if (ps == NULL || buffer == NULL || len == 0) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    ps->base.open = open;
    ps->base.get_line = get_line;
    ps->base.report_error = report_error;
    ps->base.close = close;

    ps->buffer = buffer;
    ps->curpos = 0;
    ps->maxlen = len;
    ps->depth = -1;
    
    return XCMD_ERR_OK;
}

#endif /* CFG_XCOMMAND_INCLUDED */
