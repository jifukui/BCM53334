/*
 * $Id: xc_output_buf.c,v 1.3 Broadcom SDK $
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
#include "xc_output_buf.h"

/* Use DOS format as default */
#define DOS_FORMAT_NEWLINE

#define THIS ((XCOUT_BUFFER *)pstream)
#define OVERFLOW(x) (THIS->curpos + x > THIS->maxlen)

static XCMD_ERROR
handle_BOL(void *pstream)
{
    int i;
    
    if (pstream == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_STREAM_CALL;
    }
    
    if (THIS->bol) {
    
        /* Add comment to separate commands if it's at first level */
        if (THIS->depth == 0) {
            if (OVERFLOW(2)) {
                return XCMD_ERR_BUFFER_OVERFLOW;
            }
            THIS->buffer[THIS->curpos++] = DEFAULT_CFGFILE_COMMENT_BEGIN;
#ifdef DOS_FORMAT_NEWLINE
            THIS->buffer[THIS->curpos++] = '\r';
#endif /* DOS_FORMAT_NEWLINE */
            THIS->buffer[THIS->curpos++] = '\n';
        }
    
        /* Add spaces to indicate depth */
        for(i=0; i<THIS->depth; i++) {
            if (OVERFLOW(1)) {
                return XCMD_ERR_BUFFER_OVERFLOW;
            }
            THIS->buffer[THIS->curpos++] = DEFAULT_CFGFILE_DEPTH_CHAR;
        }
        
        THIS->bol = 0;
    }
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
write_str(void *pstream, const char *str, unsigned int len)
{
    XCMD_ERROR r;
    unsigned int i;
    
    if (pstream == NULL || str == NULL || THIS->depth < 0) {
        return XCMD_ERR_INTERNAL_INVALID_STREAM_CALL;
    }
    
    if (len == 0) {
        return XCMD_ERR_OK;
    }
    
    /* Check if it has EOL in it */
    for(i=0; i<len; i++) {
        if (str[i] == '\r' || str[i] == '\n') {
            /* Actuall we can handle it, but for what? */
            return XCMD_ERR_INTERNAL_INVALID_STREAM_CALL;
        }
    }
    
    /* Add line prefixes */
    r = handle_BOL(pstream);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    /* write to buffer if not overflow */
    if (OVERFLOW(len)) {
        return XCMD_ERR_BUFFER_OVERFLOW;
    }
    sal_strncpy(&THIS->buffer[THIS->curpos], str, len);
    THIS->curpos += len;
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
write_cr(void *pstream)
{
    XCMD_ERROR r;

    if (pstream == NULL || THIS->depth < 0) {
        return XCMD_ERR_INTERNAL_INVALID_STREAM_CALL;
    }
    
    /* Check for continuous CR */
    r = handle_BOL(pstream);
    if (r != XCMD_ERR_OK) {
        return r;
    }

    /* Append CR */
    if (OVERFLOW(1)) {
        return XCMD_ERR_BUFFER_OVERFLOW;
    }

#ifdef DOS_FORMAT_NEWLINE
    THIS->buffer[THIS->curpos++] = '\r';
#endif /* DOS_FORMAT_NEWLINE */
    THIS->buffer[THIS->curpos++] = '\n';

    THIS->bol = 1;
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
report_error(void *pstream, XCMD_ERROR err, const char *command)
{
    XCMD_ERROR r;
    char buffer[32];
    int len;

    if (pstream == NULL || THIS->depth < 0) {
        return XCMD_ERR_INTERNAL_INVALID_STREAM_CALL;
    }
    
    if (!THIS->bol) {
        /* If this is not called at the beginning of line */
        r = write_cr(pstream);
        if (r != XCMD_ERR_OK) {
            return r;
        }
    }
    
    /* This line should also be aligned to the same level */
    r = handle_BOL(pstream);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    /* Error description starts as a comment */
    if (OVERFLOW(1)) {
        return XCMD_ERR_BUFFER_OVERFLOW;
    }
    THIS->buffer[THIS->curpos++] = DEFAULT_CFGFILE_COMMENT_BEGIN;
    
    /* Append error header */
    sal_sprintf(buffer, DEFAULT_CFGFILE_ERROR_FORMAT, err);
    buffer[sizeof(buffer) - 1] = 0;
    len = sal_strlen(buffer);
    if (THIS->curpos + len > THIS->maxlen) {
        return XCMD_ERR_BUFFER_OVERFLOW;
    }
    sal_strncpy(&THIS->buffer[THIS->curpos], buffer, len);
    THIS->curpos += len;
    
    /* Append problematic command */
    if (command) {
        len = sal_strlen(command);
        if (THIS->curpos + len > THIS->maxlen) {
            return XCMD_ERR_BUFFER_OVERFLOW;
        }
        sal_strncpy(&THIS->buffer[THIS->curpos], command, len);
        THIS->curpos += len;
    }
    
    /* Write CR */
    THIS->bol = 0;
    r = write_cr(pstream);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
open(void *pstream)
{
    XCMD_ERROR r;

    if (pstream == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_STREAM_CALL;
    }

    THIS->depth++;

    if (!THIS->bol) {
        /* If this is not called at the beginning of line */
        r = write_cr(pstream);
        if (r != XCMD_ERR_OK) {
            return r;
        }
    }
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
close(void *pstream)
{
    XCMD_ERROR r;

    if (pstream == NULL || THIS->depth < 0) {
        return XCMD_ERR_INTERNAL_INVALID_STREAM_CALL;
    }
    
    if (!THIS->bol) {
        /* If this is not called at the beginning of line */
        r = write_cr(pstream);
        if (r != XCMD_ERR_OK) {
            return r;
        }
    }
    
    /* Make the ending more beautiful */
    if (THIS->depth == 0) {
        THIS->buffer[THIS->curpos++] = DEFAULT_CFGFILE_COMMENT_BEGIN;
#ifdef DOS_FORMAT_NEWLINE
        THIS->buffer[THIS->curpos++] = '\r';
#endif /* DOS_FORMAT_NEWLINE */
        THIS->buffer[THIS->curpos++] = '\n';
    }

    THIS->depth--;

    return XCMD_ERR_OK;
}

XCMD_ERROR
xcout_buffer_init(XCOUT_BUFFER *ps, char *buffer, unsigned int len)
{
    if (ps == NULL || (buffer != NULL && len == 0)) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    ps->base.open = open;
    ps->base.write_str = write_str;
    ps->base.write_cr = write_cr;
    ps->base.report_error = report_error;
    ps->base.close = close;
    
    ps->buffer = buffer;
    ps->maxlen = len;
    ps->curpos = 0;
    ps->depth = -1;
    ps->bol = 1;

    /* For counting only, call with buffer = NULL */
    if (buffer == NULL) {
        ps->maxlen = (unsigned int)-1;
    }
    
    return XCMD_ERR_OK;
}

#endif /* CFG_XCOMMAND_INCLUDED */
