/*
 * $Id: ssp.c,v 1.10 Broadcom SDK $
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

#define SSP_DEBUG   0
#define SSP_DIAG    0
#define SSP_TRACE   0
#define WCP_DEBUG   0

#include "httpd_arch.h"
#include "httpd_wcp.h"
#include "ssp_internal.h"

#include <sal.h>

/*
 * Debugging diagnostic utilities
 */ 
#define fflush(x)

#if WCP_DEBUG
#define WCP_DBG(x)    do { sal_printf("WCP: "); sal_printf x; sal_printf("\n"); } while(0)
#else
#define WCP_DBG(x)
#endif

#if SSP_DEBUG || SSP_TRACE
#define SSP_DBG(x) do { sal_printf("SSP: "); sal_printf x; sal_printf("\n"); \
                        fflush(stdout); } while(0)
#else
#define SSP_DBG(x)
#endif

#if SSP_DIAG
static char ssp_diag_buf[256] = { 0 };
static int ssp_diag_last_executed;
static void *ssp_diag_last_called = NULL;
static SSPTAG_PARAM ssp_diag_params[3] = { 0, 0, 0 };
#endif /* SSP_DIAG */

#if SSP_DIAG
#define _SSP_DIAG() ssp_diag_last_executed = __LINE__
#define _SSP_DIAGX(fn) sal_sprintf(ssp_diag_buf, "%s, wh=%p file=%s", #fn, wh, \
        (wh == NULL || SSPH->pfile == NULL)? "" : SSPH->pfile->name);
#define _SSP_DIAG_PARAMS() do { \
            ssp_diag_params[0] = params[0]; \
            ssp_diag_params[1] = params[1]; \
            ssp_diag_params[2] = params[2]; \
        } while(0)
#define _SSP_DIAG_CALL(fn) do { ssp_diag_last_called = fn; } while(0)
#else 
#define _SSP_DIAG()
#define _SSP_DIAGX()
#define _SSP_DIAG_PARAMS()
#define _SSP_DIAG_CALL(fn)
#endif /* SSP_DIAG */

#if SSP_TRACE
#define _SSP_TRACE() do { \
    sal_sprintf("ssp:%d (%s)\n", __LINE__, __FUNCTION__); \
    fflush(stdout); \
} while(0)
#define _SSP_TRACEX(fn) sal_sprintf("ssp: %s, wh=%p file=%s\n", #fn, wh, \
        (wh == NULL || SSPH->pfile == NULL)? "" : SSPH->pfile->name);
#else 
#define _SSP_TRACE()
#define _SSP_TRACEX(fn)
#endif /* SSP_TRACE */

#if SSP_DIAG || SSP_TRACE

#define SSP_TRACING() do {_SSP_DIAG(); _SSP_TRACE();} while(0)
#define SSP_ENTER(fn) do {_SSP_DIAGX(fn); _SSP_DIAG(); _SSP_TRACEX();} while(0)

#else /* !(SSP_DIAG || SSP_TRACE) */

#define SSP_TRACING()
#define SSP_ENTER(fn)

#endif /* !(SSP_DIAG || SSP_TRACE) */

#define SSPH            ((SSP_SESSION *)wh)
#define SSP_GHANDLER    (SSPH->vdir->globalhandler)
#define SSP_GFLAGS      (SSPH->vdir->flags)

#if SSP_DIAG
void
cmd_diagssp(int argc, char *argv[])
{
    sal_sprintf("  Last processing: %s\n", ssp_diag_buf);
    sal_sprintf("  Last executed line: %d\n", ssp_diag_last_executed);
    sal_sprintf("  Last callback called: %p\n", ssp_diag_last_called);
    sal_sprintf("  Last parameters: (%u, %u, %u)\n",
            ssp_diag_params[0], ssp_diag_params[1], ssp_diag_params[2]);
}
#endif /* SSP_DIAG */

/* Unit of per-session memory */
typedef struct _SSP_PSMEM_ENTRY {
    void *key;
    int size;
    uint32 flags;
    struct _SSP_PSMEM_ENTRY *next;
} SSP_PSMEM_ENTRY;

#ifdef WCP_REXMIT_SUPPORT

#define SSP_REXMIT_INFO_STATE_NONE      (0)
#define SSP_REXMIT_INFO_STATE_BACKUPED  (1)
#define SSP_REXMIT_INFO_STATE_RESTORED  (2)
typedef struct {
    RES_CONST_DECL SSP_DATA_ENTRY *                page;
    uint32                          buflen;
    uint32                          offset;
    int                             state;  /* none | backuped | restored */ 
    
    /* for loop process */                                                      
    SSPLOOP_INDEX                   loops[SSP_MAX_LOOP_DEPTH];
    unsigned char                   depth;
} SSP_REXMIT_INFO;
#endif  /* WCP_REXMIT_SUPPORT */

/* HTTP session */
typedef struct {
    SSP_FILE_ENTRY *                pfile;
    RES_CONST_DECL SSP_DATA_ENTRY *                page;
    SSP_FILE_FLAGS                  flags;
    uint32                          offset;
    SSPLOOP_INDEX                   loops[SSP_MAX_LOOP_DEPTH];
    unsigned char                   depth;
    char                            buffer[SSP_SESSION_BUFFER_SIZE];
    uint32                          buflen;
    HTTP_SERVICE                    *service;    
    struct SSP_FILESYSTEM           *filesystem;
    struct SSP_FILESYSTEM           *vdir;
    SSP_PSMEM_ENTRY                 *psmem;
    int                             socket;

#ifdef WCP_REXMIT_SUPPORT
    SSP_REXMIT_INFO                 rexmit_info;
#endif  /* WCP_REXMIT_SUPPORT */
} SSP_SESSION;

/* Parsed query strings to call query string handler */
static SSP_STRING_PAIR query_strings[SSP_MAX_NUM_QUERY_STRINGS];

/* Global handler: context instance */
static SSP_GLOBAL_HANDLER_CONTEXT ssp_global_cxt;

/* Shared temp buffer for all callback functions */
char ssputil_shared_buffer[SSPUTIL_CBK_SHARED_SIZE];

#ifdef CFG_BANKED_MEMCPY_SUPPORT
/* 8051 Bank-Switch XDATA buffer for SSP_DATA */
RES_CONST_DECL SSP_DATA_ENTRY  ssp_data_bankswitch_buf;

static void 
_ssp_banksw_buffer_update(SSP_SESSION *ps)
{
    SSP_DATA_BANK   bank_id;

    /* retrieve the bank_id which is this WCP belong to */
    bank_id = ps->pfile->bank_id;

    if (!SSP_DATA_BANK_VALID(bank_id)){
        SSP_DBG(("Invalid SSP bank_id(%d) observed on wcp(%s)\n",
                (int)bank_id, ps->pfile->name));
    }

    /* For SSP current design in UM, the Web related bank-switch design 
     *  limited in bank4 and bank5 .
     */
    if (bank_id == SSP_BANK4) {
        sal_memcpy_bank4(&ssp_data_bankswitch_buf, 
                    ps->page, sizeof(SSP_DATA_ENTRY));
    } else if (bank_id == SSP_BANK5) {
        sal_memcpy_bank5(&ssp_data_bankswitch_buf, 
                    ps->page, sizeof(SSP_DATA_ENTRY));
    } else {
        SSP_DBG(("No respected BANK oriented memcpy for update process!\n"));
    }
        
}
#endif /* CFG_BANKED_MEMCPY_SUPPORT */

/* Report the current WCP page pointer.
 *
 * Note :
 *  1. For 8051 bank-switch deisgn, page buffer will be updated first.
 */
static RES_CONST_DECL SSP_DATA_ENTRY * 
_ssp_page_agent(SSP_SESSION *ps)
{
#ifdef CFG_BANKED_MEMCPY_SUPPORT
    _ssp_banksw_buffer_update(ps);
    return &ssp_data_bankswitch_buf;
#else /* CFG_BANKED_MEMCPY_SUPPORT */
    return (ps->page);
#endif /* CFG_BANKED_MEMCPY_SUPPORT */
}

static int
parse_parameters(SSP_SESSION *ps, SSPTAG_PARAM *params) 
{
    unsigned char i, k;
    RES_CONST_DECL SSP_DATA_ENTRY  *ssp_page;

    ssp_page = _ssp_page_agent(ps);
    sal_memcpy(params, &ssp_page->param1, 
           sizeof(SSPTAG_PARAM) * SSP_MAX_NUM_PARAMETERS);

    k = SSP_DATA_PARAM1_LOOP;
    for(i=0; i<SSP_MAX_NUM_PARAMETERS; i++) {
        if (ssp_page->opbyte & k) { /* param1 is loop index */
            if (params[i] >= ps->depth) {
                SSP_DBG(("*** SSP BUG: parse_parameters: incorrect depth!"));
                return 0;
            }
            params[i] = ps->loops[params[i]];
        }
        k <<= 1;
    }

    return 1;
}

static void 
decode_url_in_place(char *url)
{
    char *where, *to;
    for(where = url, to = url; *where; ) {
        if (*where == '%' && isxdigit(where[1]) && isxdigit(where[2])) {
            *to++ = (isdigit(where[1]) ? (where[1] - '0') : 
                    (toupper(where[1]) - 'A' + 10)) * 16 + 
                    (isdigit(where[2]) ? (where[2] - '0') : 
                    (toupper(where[2]) - 'A' + 10));        
            where += 3;
        } else {
            if (*where == '+') {
                *where = ' ';
            }
            *to++ = *where++;
        }
    }
    *to = '\0';
}

static SSP_FILE_ENTRY*
locate_file_entry(struct SSP_FILESYSTEM **filesystem, const char *url)
{
    struct SSP_FILESYSTEM *pfs;
    SSP_FILE_ENTRY *pfile;
    const char *filename, *str;
    
    if (url == NULL || filesystem == NULL || *filesystem == NULL) {
        return NULL;
    }
    
    /* Extract filename */
    filename = url;
    while((str = sal_strchr(filename, '/')) != NULL) {
        filename = str + 1;
    }
    
    for(pfs = (*filesystem); pfs->dir_prefix != NULL; pfs++) {

        /* the URL must be under dir_prefix */
        if (!sal_strncmp(pfs->dir_prefix, url, sal_strlen(pfs->dir_prefix))) {
            for(pfile = pfs->filetable; pfile->name != NULL; pfile++) {
                if (!strcmpi(filename, pfile->name)) {
                    *filesystem = pfs;
                    return pfile;
                }
            }
            /* If not found, try following dirs but must match prefix */
        }
    }

    return NULL;
}

WCP_HANDLE
wcp_open(uint8 post, 
         const char *path, 
         const char *params, 
         int sock, 
         HTTP_SERVICE *p)
{
    SSP_FILE_ENTRY *pfile;
    SSP_SESSION *wh = NULL;
    struct SSP_FILESYSTEM *vdir;
        
    SSP_ENTER(wcp_open);
    WCP_DBG(("wcp_open: %s %s?%s", ((post == 1)? "POST":"GET"), path, params));
    
    if (p == NULL || p->wcp_param == NULL) {
        SSP_DBG(("ERROR - No service handle supplied!"));
        SSP_TRACING();
        return NULL;
    }

    SSP_TRACING();
    vdir = (struct SSP_FILESYSTEM *)p->wcp_param;
    pfile = locate_file_entry(&vdir, path);
    SSP_TRACING();
    
    if (pfile == NULL || (pfile->flags & SSPF_NO_DIRECT_ACCESS)) {
        /* XXX: show error page instead of simply RST */
        SSP_DBG(("ERROR - Failed to find URL %s!", path));
        SSP_TRACING();
        return NULL;
    }

    /* XXX: Use static memory for sessions */
    SSP_TRACING();
    wh = (SSP_SESSION *)KMALLOC(sizeof(SSP_SESSION), 0);
    SSP_TRACING();
    wh->pfile = pfile;
    wh->page = pfile->file_data;
    wh->flags = pfile->flags;
    wh->offset = 0;
    wh->depth = 0;
    wh->buflen = 0;
    wh->psmem = NULL;
    wh->socket = sock;
    wh->service = p;
    wh->filesystem = (struct SSP_FILESYSTEM *)p->wcp_param;
    wh->vdir = vdir;

#ifdef WCP_REXMIT_SUPPORT
    wh->rexmit_info.page = pfile->file_data;
    wh->rexmit_info.buflen = 0;
    wh->rexmit_info.offset = 0;
    wh->rexmit_info.state= SSP_REXMIT_INFO_STATE_NONE;
#endif  /* WCP_REXMIT_SUPPORT */
    
    /* Give global handler a chance to change the destination */
    if (SSP_GHANDLER != NULL && (SSP_GFLAGS & SSPF_GLOBAL_OPEN_H)) {
        SSP_HANDLER_RETVAL r;
        
        ssp_global_cxt.type = SSP_HANDLER_GLOBAL_OPEN;
        ssp_global_cxt.pfile = pfile;
        ssp_global_cxt.page = pfile->file_data;
        ssp_global_cxt.flags = pfile->flags;
        ssp_global_cxt.url_data.string = (char *)path;
        
        SSP_TRACING();
        r = (*SSP_GHANDLER)(&ssp_global_cxt, wh);
        SSP_TRACING();
        
        if (r == SSP_HANDLER_RET_MODIFIED) {
            wh->pfile = ssp_global_cxt.pfile;
            if (wh->pfile != NULL) {
                wh->page = wh->pfile->file_data;
                wh->flags = wh->pfile->flags;
            }
        }
    }
    
    if (wh->pfile == NULL || (wh->pfile->flags & SSPF_NO_DIRECT_ACCESS)) {

        /* XXX: show error page instead of simply RST */
        SSP_DBG(("ERROR - Failed to find URL %s!", path));
        SSP_TRACING();
        
        wcp_close((WCP_HANDLE)wh);
        return NULL;
    }

    if (!post && params) {
        /* Save query strings for later */
        SSP_TRACING();
        sal_strcpy(wh->buffer, params);
        wh->buflen = sal_strlen(wh->buffer) + 1;
    }
    
    /* Call handler if SSPF_OPEN_CLOSE_H set */
    if (wh->flags & SSPF_OPEN_CLOSE_H && wh->pfile->handler != NULL) {
        SSP_HANDLER_CONTEXT_EXT cxt;
        SSP_HANDLER_RETVAL r;
        
        cxt.type = SSP_HANDLER_OPEN;
        cxt.page = wh->page;
        cxt.flags = wh->flags;
        cxt.url_data.string = NULL;

        _SSP_DIAG_CALL(wh->pfile->handler);
        SSP_TRACING();
        r = (*wh->pfile->handler)((SSP_HANDLER_CONTEXT *)&cxt, wh);
        SSP_TRACING();
        if (r == SSP_HANDLER_RET_MODIFIED) {
            /* Handler has changed data entry and flags */
            wh->page = cxt.page;
            wh->flags = cxt.flags;
        }
    }
    
    SSP_DBG(("Open URL: %s", wh->pfile->name));
    SSP_ENTER(wcp_open);
    return wh;
}

void
wcp_req_header(WCP_HANDLE wh, const char *name, const char *value)
{
    SSP_HANDLER_RETVAL r;
    if (wh == NULL) 
        return;

    WCP_DBG(("wcp_req_header(%s): %s: %s", SSPH->pfile->name, name, value));

    /* Check if "If-Modified-Since" present */
    if (name != NULL && !sal_strcmp(name, "If-Modified-Since")) {
        SSPH->flags |= SSPF_IF_MODIFIED_SINCE;
    }
    
    /* Call global handler first */
    if (SSP_GHANDLER && (SSP_GFLAGS & SSPF_REQUEST_HEADER_H)) {
        
        SSP_ENTER(wcp_req_header);
        ssp_global_cxt.type = SSP_HANDLER_REQ_HEADER;
        ssp_global_cxt.pfile = SSPH->pfile;
        ssp_global_cxt.page = SSPH->page;
        ssp_global_cxt.flags = SSPH->flags;
        ssp_global_cxt.url_data.string_pair.name = name;
        ssp_global_cxt.url_data.string_pair.value = value;
        
        SSP_TRACING();
        r = (*SSP_GHANDLER)(&ssp_global_cxt, wh);
        SSP_TRACING();
        
        if (r == SSP_HANDLER_RET_MODIFIED) {
            /* Handler has changed data entry and flags */
            SSPH->page = ssp_global_cxt.page;
            SSPH->flags = ssp_global_cxt.flags;
        }
    }
    
    /* Call URL handler */
    if (SSPH->pfile->handler != NULL && (SSPH->flags & SSPF_REQUEST_HEADER_H)) {
        SSP_HANDLER_CONTEXT_EXT cxt;
        SSP_HANDLER_RETVAL r;
        
        SSP_ENTER(wcp_req_header);
        cxt.type = SSP_HANDLER_REQ_HEADER;
        cxt.page = SSPH->page;
        cxt.flags = SSPH->flags;
        cxt.url_data.string_pair.name = name;
        cxt.url_data.string_pair.value = value;
        
        _SSP_DIAG_CALL(SSPH->pfile->handler);
        SSP_TRACING();
        r = (*SSPH->pfile->handler)((SSP_HANDLER_CONTEXT *)&cxt, wh);
        SSP_TRACING();
        if (r == SSP_HANDLER_RET_MODIFIED) {
            /* Handler has changed data entry and flags */
            SSPH->page = cxt.page;
            SSPH->flags = cxt.flags;
        }
    }

    /* Check if it's cookie only if someone is waiting for */
    if (name != NULL && 
        ((SSPH->pfile->handler != NULL && (SSPH->flags & SSPF_REQUEST_COOKIE_H))
        || (SSP_GHANDLER != NULL && 
            (SSP_GFLAGS & SSPF_REQUEST_COOKIE_H)))) {
                
        if (!sal_strcmp(name, "Cookie") && 
            SSPH->buflen + sal_strlen(value) < SSP_SESSION_BUFFER_SIZE) {
            char *buf;
            
            /* Make use of SSPH->buffer as our temp buffer */
            buf = &SSPH->buffer[SSPH->buflen];
            sal_strcpy(buf, value);
            
            while(buf) {
                char *n, *v;
                
                /* Strip leading spaces */
                while(*buf == ' ') buf++;
                
                /* Find '=' sign that delimits name and value */
                v = sal_strchr(buf, '=');
                if (v == NULL) {
                    break;
                }
                n = buf;    /* n: name */
                *v++ = 0;   /* v: value */
                
                /* Find the end of cookie (and terminate it) */
                buf = sal_strchr(v, ';');
                if (buf) {
                    /* ';' found, it means we have more cookies */
                    *buf++ = 0;
                }
                
                /* Call global handler first */
                SSP_ENTER(wcp_req_header);
                if (SSP_GHANDLER != NULL && 
                    (SSP_GFLAGS & SSPF_REQUEST_COOKIE_H)) {
                    
                    ssp_global_cxt.type = SSP_HANDLER_REQ_COOKIE;
                    ssp_global_cxt.pfile = SSPH->pfile;
                    ssp_global_cxt.page = SSPH->page;
                    ssp_global_cxt.flags = SSPH->flags;
                    ssp_global_cxt.url_data.string_pair.name = n;
                    ssp_global_cxt.url_data.string_pair.value = v;
                    
                    SSP_TRACING();
                    r = (*SSP_GHANDLER)(&ssp_global_cxt, wh);
                    SSP_TRACING();
                    
                    if (r == SSP_HANDLER_RET_MODIFIED) {
                        /* Handler has changed data entry and flags */
                        SSPH->page = ssp_global_cxt.page;
                        SSPH->flags = ssp_global_cxt.flags;
                    }
                }
                
                /* Call URL handler */
                if (SSPH->pfile->handler != NULL && 
                    (SSPH->flags & SSPF_REQUEST_COOKIE_H)) {
                        
                    SSP_HANDLER_CONTEXT_EXT cxt;
                    SSP_HANDLER_RETVAL r;
                    
                    cxt.type = SSP_HANDLER_REQ_COOKIE;
                    cxt.page = SSPH->page;
                    cxt.flags = SSPH->flags;
                    cxt.url_data.string_pair.name = n;
                    cxt.url_data.string_pair.value = v;
                    
                    _SSP_DIAG_CALL(SSPH->pfile->handler);
                    SSP_TRACING();
                    r = (*SSPH->pfile->handler)((SSP_HANDLER_CONTEXT *)&cxt, wh);
                    SSP_TRACING();
                    if (r == SSP_HANDLER_RET_MODIFIED) {
                        /* Handler has changed data entry and flags */
                        SSPH->page = cxt.page;
                        SSPH->flags = cxt.flags;
                    }
                }            
            }
        }
    }
}

void
wcp_req_content_length(WCP_HANDLE wh, uint32 len)
{
    if (wh == NULL) 
        return;

    SSP_ENTER(wcp_req_content_length);
    WCP_DBG(("wcp_req_content_length(%s): %u", SSPH->pfile->name, len));
    SSPH->offset = len;
}

void 
wcp_req_post_data(WCP_HANDLE wh, const uint8 *buf, uint32 len)
{
    if (wh == NULL) 
        return;

    SSP_ENTER(wcp_req_post_data);
    WCP_DBG(("wcp_req_post_data(%s): %u", SSPH->pfile->name, len));

    if (SSPH->pfile->handler != NULL && (SSPH->flags & SSPF_FILE_UPLOAD_H)) {
        SSP_HANDLER_CONTEXT_EXT cxt;
        int32 *pvar; 
        int r;
        
        SSP_TRACING();
        if (buf != NULL) {
            char *p;
            int32 ulen;
            
            SSP_TRACING();
            pvar = (int32 *)ssputil_psmem_get((void *)wh, wcp_req_post_data);
            if (pvar == NULL) { /* The first one */
                
                SSP_TRACING();
                pvar = (int32 *)ssputil_psmem_alloc(
                    (void *)wh, 
                    wcp_req_post_data, 
                    sizeof(int32) * 3);
                SSP_TRACING();
                    
                /* Variables in this memory: 
                 *   *pvar: total file data received,
                 *   *(pvar + 1): data header length,
                 *   *(pvar + 2): data footer length */
                *pvar = *(pvar + 1) = *(pvar + 2) = 0;
                
                /* This is the first one we got, 
                 * it should contain the data header.
                 * XXX We assume the data header is complete. */
                p = sal_strstr((char *)buf, "\r\n\r\n");
                if (p != NULL) {
                    *p = 0;
                    *(pvar + 1) = sal_strlen((char *)buf) + 4; /* data header length */
                    
                    /* Check the first line (containing boudary) */
                    p = sal_strstr((char *)buf, "\r\n");
                    if (p != NULL) {
                        *p = 0;
                        /* buf now: --<boundary> (first line in data header);
                         * footer: \r\n--<boundary>--\r\n */
                        *(pvar + 2) = sal_strlen((char *)buf) + 6;
                    }
                }
                
                /* let buf point to file data */
                buf += *(pvar + 1);
                len -= *(pvar + 1);
                
                /* Notify handler for actual data length */
                cxt.type = SSP_HANDLER_FILE_UPLOAD;
                cxt.page = SSPH->page;
                cxt.flags = SSPH->flags;
                cxt.url_data.upload.buf = NULL;
                cxt.url_data.upload.index = -1; /* Indicates total length */
                cxt.url_data.upload.length = 
                    SSPH->offset - *(pvar + 1) - *(pvar + 2);
                    
                _SSP_DIAG_CALL(SSPH->pfile->handler);
                SSP_TRACING();
                r = (*SSPH->pfile->handler)((SSP_HANDLER_CONTEXT *)&cxt, wh);
                SSP_TRACING();
                if (r == SSP_HANDLER_RET_MODIFIED) {
                    /* Handler has changed data entry and flags */
                    SSPH->page = cxt.page;
                    SSPH->flags = cxt.flags;
                    
                    /* Check if the handler gives up processing the data */
                    if ((SSPH->flags & SSPF_FILE_UPLOAD_H) == 0) {
                        SSP_TRACING();
                        ssputil_psmem_free((void *)wh, wcp_req_post_data);
                        return;
                    }
                }
            }
            
            ulen = (int32)len;
            
            /* Check if footer appears */
            if (*(pvar + 1) + *pvar + ulen > SSPH->offset - *(pvar + 2)) {
                ulen = SSPH->offset - *(pvar + 2) - *(pvar + 1) - *pvar;
            }
            
            /* Call upload handler (if still have file data) */
            if (ulen > 0) {
                cxt.type = SSP_HANDLER_FILE_UPLOAD;
                cxt.page = SSPH->page;
                cxt.flags = SSPH->flags;
                cxt.url_data.upload.buf = buf;
                cxt.url_data.upload.index = *pvar;
                cxt.url_data.upload.length = ulen;
                
                _SSP_DIAG_CALL(SSPH->pfile->handler);
                SSP_TRACING();
                r = (*SSPH->pfile->handler)((SSP_HANDLER_CONTEXT *)&cxt, wh);
                SSP_TRACING();
                if (r == SSP_HANDLER_RET_MODIFIED) {
                    /* Handler has changed data entry and flags */
                    SSPH->page = cxt.page;
                    SSPH->flags = cxt.flags;
                }
            }
            *pvar += ulen;
            
        } else { /* buf == NULL: indicates End of File */
        
            pvar = (int32 *)ssputil_psmem_get((void *)wh, wcp_req_post_data);
            cxt.type = SSP_HANDLER_FILE_UPLOAD;
            cxt.page = SSPH->page;
            cxt.flags = SSPH->flags;
            cxt.url_data.upload.buf = NULL;
            cxt.url_data.upload.length = 0;
            if (pvar) {
                cxt.url_data.upload.index = *pvar;
            } else {
                cxt.url_data.upload.index = 0;
            }

            _SSP_DIAG_CALL(SSPH->pfile->handler);
            SSP_TRACING();
            r = (*SSPH->pfile->handler)((SSP_HANDLER_CONTEXT *)&cxt, wh);
            SSP_TRACING();
            if (r == SSP_HANDLER_RET_MODIFIED) {
                /* Handler has changed data entry and flags */
                SSPH->page = cxt.page;
                SSPH->flags = cxt.flags;
            }
        }
        
    } else if (buf != NULL) {

        /* Simple query strings */
        SSP_TRACING();
        if (SSPH->buflen + len > SSP_SESSION_BUFFER_SIZE - 1) {
            SSP_DBG(("*** SSP BUG: query strings too long: %s!", 
                    SSPH->pfile->name));
            len = SSP_SESSION_BUFFER_SIZE - 1 - SSPH->buflen;
            SSP_TRACING();
            /* Ignore all overflowed date */
        }
        
        sal_memcpy(SSPH->buffer + SSPH->buflen, buf, len);
        SSPH->buflen += len;
    }
    SSP_TRACING();
}

const char *
wcp_req_end(WCP_HANDLE wh, int32 *ctlen)
{
    static SSP_HANDLER_CONTEXT cxt_qstr;
    SSP_HANDLER_RETVAL r;
    const char *ret_code = NULL;

    RES_CONST_DECL SSP_DATA_ENTRY  *ssp_page;

    if (wh == NULL) 
        return NULL;

    SSP_ENTER(wcp_req_end);
    WCP_DBG(("wcp_req_end(%s)", SSPH->pfile->name));

    /* Parse & call query strings handlers only if user specifies the flag */
    if ((SSPH->pfile->handler != NULL && (SSPH->flags & SSPF_QUERY_STRINGS_H)) 
        || (SSP_GHANDLER != NULL && 
            (SSP_GFLAGS & SSPF_QUERY_STRINGS_H))) {
                
        int16 count = 0;
        char *pstr = SSPH->buffer, *pstr2;
        
        SSP_TRACING();
        SSPH->buffer[SSPH->buflen] = 0; /* terminate the string */

        /* Parse query strings */
        while(*pstr != '\0') {

            /* The first occurance is name (or "\r\n" if ended) */
            query_strings[count].name = pstr;

            /* It should contain '=' as delimiter for name and value */
            pstr = sal_strchr(pstr, '=');
            if (pstr == NULL) {
                break;
            }

            /* Terminate name string */
            *pstr++ = '\0';

            /* Following is the value */
            query_strings[count].value = pstr;

            /* Each pair may end in '&' or "\r\n&" */
            pstr += sal_strcspn(pstr, "&\r");
            if (*pstr == '\r') {

                /* Terminate value string */
                *pstr++ = '\0';

                /* Skip all garbage before another '&' or end of string*/
                pstr2 = sal_strchr(pstr, '&');
                if (pstr2) {
                    /* Make pstr to be at '&' if found */
                    pstr = pstr2;
                }
            }

            /* pstr should now at '&' if more pairs */
            if (*pstr == '&') {

                /* Terminate value string */
                *pstr++ = '\0';

                /* Skip "\r\n" if any */
                if (*pstr == '\r') {
                    pstr += 2;
                }
            }

            /* Decode URL back */
            decode_url_in_place((char *)query_strings[count].name);
            decode_url_in_place((char *)query_strings[count].value);

            /* We have succesfully got one pair */
            count++;
        }

        if (SSP_DEBUG) {
            int16 i;
            SSP_DBG(("Query strings for %s: %s", 
                SSPH->pfile->name, (count == 0? "(none)" : "")));
            for(i=0; i<count; i++) {
                SSP_DBG(("\t%d [%s]=[%s]", 
                    i, query_strings[i].name, query_strings[i].value));
            }
        }

        /* Call global handler if flag set */
        if ((SSP_GFLAGS & SSPF_QUERY_STRINGS_H) && 
            SSP_GHANDLER != NULL) {
            
            ssp_global_cxt.type = SSP_HANDLER_QUERY_STRINGS;
            ssp_global_cxt.pfile = SSPH->pfile;
            ssp_global_cxt.page = SSPH->page;
            ssp_global_cxt.flags = SSPH->flags;
            ssp_global_cxt.url_data.qstrings.pairs = query_strings;
            ssp_global_cxt.url_data.qstrings.count = count;
            
            SSP_TRACING();
            r = (*SSP_GHANDLER)(&ssp_global_cxt, wh);
            SSP_TRACING();
            
            if (r == SSP_HANDLER_RET_MODIFIED) {
                /* Handler has changed data entry and flags */
                SSPH->page = ssp_global_cxt.page;
                SSPH->flags = ssp_global_cxt.flags;
            }
        }

        /* Call query string handler */
        if (SSPH->pfile->handler != NULL && 
            (SSPH->flags & SSPF_QUERY_STRINGS_H)) {
                
            cxt_qstr.type = SSP_HANDLER_QUERY_STRINGS;
            cxt_qstr.page = SSPH->page;
            cxt_qstr.flags = SSPH->flags;
            cxt_qstr.pairs = query_strings;
            cxt_qstr.count = count;
            
            SSP_DBG(("Calling query string handler %p for %s", 
                    SSPH->pfile->handler, SSPH->pfile->name));
            _SSP_DIAG_CALL(SSPH->pfile->handler);
            SSP_TRACING();
            r = (*SSPH->pfile->handler)(&cxt_qstr, (void *)wh);
            SSP_TRACING();
    
            if (r == SSP_HANDLER_RET_MODIFIED) {
                /* Handler has changed data entry and flags */
                SSP_DBG(("\tHandler %p - modified page=%p flags=0x%x", 
                        SSPH->pfile->handler, cxt_qstr.page, cxt_qstr.flags));
                SSPH->page = cxt_qstr.page;
                SSPH->flags = cxt_qstr.flags;
            }
        }
    }

    /* Call global handler if flag set */
    if ((SSP_GFLAGS & SSPF_REQUEST_DONE_H) && 
        SSP_GHANDLER != NULL) {
        
        ssp_global_cxt.type = SSP_HANDLER_REQ_DONE;
        ssp_global_cxt.pfile = SSPH->pfile;
        ssp_global_cxt.page = SSPH->page;
        ssp_global_cxt.flags = SSPH->flags;
        ssp_global_cxt.url_data.string = NULL;
        
        SSP_TRACING();
        r = (*SSP_GHANDLER)(&ssp_global_cxt, wh);
        SSP_TRACING();
        
        if (r == SSP_HANDLER_RET_MODIFIED) {
            /* Handler has changed data entry and flags */
            SSPH->page = ssp_global_cxt.page;
            SSPH->flags = ssp_global_cxt.flags;
            if (ssp_global_cxt.url_data.string != NULL) {
                ret_code = ssp_global_cxt.url_data.string;
            }
        }
    }

    /* Call handler for SSPF_REQUEST_DONE_H */
    if (ret_code == NULL && 
        SSPH->pfile->handler != NULL && (SSPH->flags & SSPF_REQUEST_DONE_H)) {
        SSP_HANDLER_CONTEXT_EXT cxt;
        
        cxt.type = SSP_HANDLER_REQ_DONE;
        cxt.page = SSPH->page;
        cxt.flags = SSPH->flags;
        cxt.url_data.string = NULL;
        
        _SSP_DIAG_CALL(SSPH->pfile->handler);
        SSP_TRACING();
        r = (*SSPH->pfile->handler)((SSP_HANDLER_CONTEXT *)&cxt, wh);
        SSP_TRACING();
        if (r == SSP_HANDLER_RET_MODIFIED) {
            /* Handler has changed data entry and flags */
            SSPH->page = cxt.page;
            SSPH->flags = cxt.flags;
            if (cxt.url_data.string != NULL) {
                ret_code = cxt.url_data.string;
            }
        }
    }

    /* report ssp page pointer */
    ssp_page = _ssp_page_agent(SSPH);

    /* SSPF_NO_CACHE override all other cache conditions */
    if (SSPH->flags & SSPF_NO_CACHE) {
        
        /* Make sure SSPF_FORCE_CACHE is not set */
        SSP_TRACING();
        SSPH->flags &= ~SSPF_FORCE_CACHE;
        
    } else { /* SSPF_NO_CACHE is not set, check condition */
        /* static content + no handler => cache */
        if (SSPH->pfile->handler == NULL && 
            ssp_page->opbyte == (SSP_DATA_FLAG_LAST | SSP_DATA_TYPE_TEXT)) {
                
            SSP_TRACING();
            SSPH->flags |= SSPF_FORCE_CACHE;
        } else { 
            
            /* otherwise, only cache if SSPF_FORCE_CACHE is set */
            SSP_TRACING();
            if ((SSPH->flags & SSPF_FORCE_CACHE) == 0) {
                SSP_TRACING();
                SSPH->flags |= SSPF_NO_CACHE;
            }
        }
    }
    
    if (ctlen) {
        if (SSPH->page != NULL && 
            (SSP_DATA_FLAG_LAST | SSP_DATA_TYPE_TEXT) == ssp_page->opbyte) {
            /* if static text, the content length is known */
            SSP_TRACING();
            *ctlen = (int32)ssp_page->param1;
        } else {
            /* Don't send "Content-Length" header */
            SSP_TRACING();
            *ctlen = 0;
        }
    }
    
    SSP_TRACING();
    SSPH->offset = 0; /* text offset || index of buffer (if buflen!=0) */
    SSPH->buflen = 0; /* left (unsent) data of previous sspvar tag */

    /* Enhance browser caching: using "Last-Modified / If-Modified-Since" */
    if (SSPH->flags & SSPF_IF_MODIFIED_SINCE) {
        if ((SSPH->flags & SSPF_FORCE_CACHE) &&
            (ret_code == NULL || !sal_strncmp(ret_code, "200", 3))) {
            ret_code = "304 Not Modified";
            SSPH->page = NULL;
            *ctlen = 0;
        }
    }

    return ret_code;
}

#ifdef WCP_REXMIT_SUPPORT
#define SSP_REXMIT_CLEAR    (0)
#define SSP_REXMIT_BACKUP   (1)
#define SSP_REXMIT_RESTORE  (2)

void 
_ssp_rexmit_info_action(WCP_HANDLE wh, int op)
{
    SSP_REXMIT_INFO * rexmit_info;
    
    if (wh == NULL){ 
        return;
    }

    rexmit_info =  &(SSPH->rexmit_info);

    if (op == SSP_REXMIT_BACKUP){
        if (rexmit_info->state == SSP_REXMIT_INFO_STATE_NONE || 
                rexmit_info->state == SSP_REXMIT_INFO_STATE_RESTORED) {
            WCP_REXMIT_PRINTF(("_ssp_rexmit_info_action:BACKUP(page=%p,offset=%ld)\n",
                    SSPH->page, SSPH->offset));
            rexmit_info->page = SSPH->page;
            rexmit_info->buflen = SSPH->buflen;
            rexmit_info->offset = SSPH->offset;
            rexmit_info->state = SSP_REXMIT_INFO_STATE_BACKUPED;

            /* for loop process */
            sal_memcpy(&(rexmit_info->loops), &(SSPH->loops), sizeof(SSPH->loops));
            rexmit_info->depth = SSPH->depth;
        }
    } else if (op == SSP_REXMIT_RESTORE) {
        if (rexmit_info->state == SSP_REXMIT_INFO_STATE_BACKUPED) {
            WCP_REXMIT_PRINTF(("_ssp_rexmit_info_action:RESTORE(page=%p,offset=%ld)\n",
                    SSPH->page, SSPH->offset));
            SSPH->page = rexmit_info->page;
            SSPH->buflen = rexmit_info->buflen;
            SSPH->offset = rexmit_info->offset;
            rexmit_info->state = SSP_REXMIT_INFO_STATE_RESTORED;

            /* for loop process */
            sal_memcpy(&(SSPH->loops), &(rexmit_info->loops), sizeof(SSPH->loops));
            SSPH->depth = rexmit_info->depth;
        }
    } else {
        /* cleared once httpd data transmitted */
        WCP_REXMIT_PRINTF(("_ssp_rexmit_info_action:op=CLEAR\n"));
        rexmit_info->state = SSP_REXMIT_INFO_STATE_NONE;
    }
}

void 
wcp_rexmit_clear(WCP_HANDLE wh)
{
    _ssp_rexmit_info_action(wh, (int)SSP_REXMIT_CLEAR);
}

#endif  /* WCP_REXMIT_SUPPORT */

#ifdef WCP_REXMIT_SUPPORT
const char *
wcp_reply_headers(WCP_HANDLE wh, int rexmit)
#else  /* WCP_REXMIT_SUPPORT */
const char *
wcp_reply_headers(WCP_HANDLE wh)
#endif  /* WCP_REXMIT_SUPPORT */
{
    SSP_HANDLER_RETVAL r;

if (wh == NULL) 
    return NULL;

#ifdef WCP_REXMIT_SUPPORT
    if (rexmit) {
        _ssp_rexmit_info_action(wh, (int)SSP_REXMIT_RESTORE);
    } else {
        _ssp_rexmit_info_action(wh, (int)SSP_REXMIT_BACKUP);
    }
#endif  /* WCP_REXMIT_SUPPORT */

    SSP_ENTER(wcp_reply_headers);
    WCP_DBG(("wcp_reply_headers(%s)", SSPH->pfile->name));

    /* Check cache control flags */
    if (SSPH->flags & SSPF_NO_CACHE) {
        SSP_TRACING();
        SSPH->flags &= ~SSPF_NO_CACHE;

        if ((SSPH->flags & SSPF_SET_ATTACH_FILENAME_H) &&
            (SSPH->service->flags & HTTPSVC_FLAG_HTTPS)) {
            /*
             * Microsoft KB316431: 
             * "IE is unable to open Office documents from an SSL Web site."
             *
             * Setting "no-cache" will make IE drop whatever it receives
             * from secure channel thus makes file saving not working with 
             * "Content-Disposition" header.  
             */
            return "Expires: -1";     
        } else {
            return "Cache-Control: no-cache" /* Actually only in HTTP/1.1. */
                   "\r\nPragma: no-cache"    /* For request or secure channel */
                   "\r\nExpires: -1";     
        }
        
    } else if (SSPH->flags & SSPF_FORCE_CACHE) {
        SSP_TRACING();
        SSPH->flags &= ~SSPF_FORCE_CACHE;
        
        if (!(SSPH->flags & SSPF_IF_MODIFIED_SINCE)) {
            SSP_TRACING();
            return "Cache-Control: max-age=2592000" /* Only in HTTP/1.1. */
                   "\r\nLast-Modified: Tue, 15 Jan 1980 09:00:00 GMT"
                   "\r\nExpires: Mon, 31 Dec 2046 23:59:59 GMT";
        }
    }

    /* Call global handler if it wants to set cookie */
    if ((SSP_GFLAGS & SSPF_SET_COOKIE_H) && SSP_GHANDLER != NULL) {
        
        ssp_global_cxt.type = SSP_HANDLER_SET_COOKIE;
        ssp_global_cxt.pfile = SSPH->pfile;
        ssp_global_cxt.page = SSPH->page;
        ssp_global_cxt.flags = SSPH->flags;
        ssp_global_cxt.url_data.string = NULL;
        
        /*
         * NOTE: Global handler have to keep track of 
         *       whether it has already sent the cookie or not.
         */
        SSP_TRACING();
        r = (*SSP_GHANDLER)(&ssp_global_cxt, wh);
        SSP_TRACING();
        
        if (r == SSP_HANDLER_RET_MODIFIED) {
            /* Handler has changed data entry and flags */
            SSP_TRACING();
            SSPH->page = ssp_global_cxt.page;
            SSPH->flags = ssp_global_cxt.flags;
            if (ssp_global_cxt.url_data.string_pair.name != NULL && 
                ssp_global_cxt.url_data.string_pair.name[0] != 0 &&
                ssp_global_cxt.url_data.string_pair.value != NULL) {
                    
                SSP_TRACING();
                sal_sprintf(SSPH->buffer,
                        "Set-Cookie: %s=%s; path=/",
                        ssp_global_cxt.url_data.string_pair.name,
                        ssp_global_cxt.url_data.string_pair.value);

                return SSPH->buffer;
            }
        }
    }

    /* Call global handler if flag set */
    if ((SSP_GFLAGS & SSPF_SET_HEADER_H) && 
        SSP_GHANDLER != NULL) {
        
        ssp_global_cxt.type = SSP_HANDLER_SET_HEADER;
        ssp_global_cxt.pfile = SSPH->pfile;
        ssp_global_cxt.page = SSPH->page;
        ssp_global_cxt.flags = SSPH->flags;
        ssp_global_cxt.url_data.string = NULL;
        
        /*
         * NOTE: Global handler have to keep track of 
         *       whether it has already sent the header or not.
         */
        SSP_TRACING();
        r = (*SSP_GHANDLER)(&ssp_global_cxt, wh);
        SSP_TRACING();
        
        if (r == SSP_HANDLER_RET_MODIFIED) {
            /* Handler has changed data entry and flags */
            SSP_TRACING();
            SSPH->page = ssp_global_cxt.page;
            SSPH->flags = ssp_global_cxt.flags;
            if (ssp_global_cxt.url_data.string_pair.name != NULL && 
                ssp_global_cxt.url_data.string_pair.name[0] != 0 &&
                ssp_global_cxt.url_data.string_pair.value != NULL) {

                SSP_TRACING();
                sal_sprintf(SSPH->buffer,
                        "%s: %s",
                        ssp_global_cxt.url_data.string_pair.name,
                        ssp_global_cxt.url_data.string_pair.value);

                return SSPH->buffer;
            }
        }
    }

    /* Check if this handler wants to set cookie */
    if (SSPH->pfile->handler != NULL && (SSPH->flags & SSPF_SET_COOKIE_H)) {
            
        SSP_HANDLER_CONTEXT_EXT cxt;
        
        cxt.type = SSP_HANDLER_SET_COOKIE;
        cxt.page = SSPH->page;
        cxt.flags = SSPH->flags;
        cxt.url_data.string = NULL;

        /*
         * NOTE: The handler have to keep track of 
         *       whether it has already sent the cookie or not.
         *       (may simply turn off SSPF_SET_COOKIE_H flag once set)
         */
        _SSP_DIAG_CALL(SSPH->pfile->handler);
        SSP_TRACING();
        r = (*SSPH->pfile->handler)((SSP_HANDLER_CONTEXT *)&cxt, wh);
        SSP_TRACING();
        if (r == SSP_HANDLER_RET_MODIFIED) {
            /* Handler has changed string, data entry or flags */
            SSP_TRACING();
            SSPH->page = cxt.page;
            SSPH->flags = cxt.flags;
            if (cxt.url_data.string_pair.name != NULL && 
                cxt.url_data.string_pair.name[0] != 0 &&
                cxt.url_data.string_pair.value != NULL) {
                
                SSP_TRACING();
                sal_sprintf(SSPH->buffer,
                        "Set-Cookie: %s=%s; path=/",
                        cxt.url_data.string_pair.name,
                        cxt.url_data.string_pair.value);

                return SSPH->buffer;
            }
        }
    }

    /* Check if this handler wants to set attach filename */
    if (SSPH->pfile->handler != NULL && 
        (SSPH->flags & SSPF_SET_ATTACH_FILENAME_H)) {
            
        SSP_HANDLER_CONTEXT_EXT cxt;
        const char *filename = SSPH->pfile->name;
        
        /* Mark as we have called */
        SSP_TRACING();
        SSPH->flags &= ~SSPF_SET_ATTACH_FILENAME_H;
        
        cxt.type = SSP_HANDLER_SET_FILENAME;
        cxt.page = SSPH->page;
        cxt.flags = SSPH->flags;
        cxt.url_data.string = NULL;

        _SSP_DIAG_CALL(SSPH->pfile->handler);
        SSP_TRACING();
        r = (*SSPH->pfile->handler)((SSP_HANDLER_CONTEXT *)&cxt, wh);
        SSP_TRACING();
        if (r == SSP_HANDLER_RET_MODIFIED) {
            /* Handler has changed string, data entry or flags */
            SSP_TRACING();
            SSPH->page = cxt.page;
            SSPH->flags = cxt.flags;
            if (cxt.url_data.string != NULL && cxt.url_data.string[0] != 0) {
                SSP_TRACING();
                filename = cxt.url_data.string;
            }
        }
        
        sal_sprintf(SSPH->buffer, 
                "Content-Disposition: attachment; filename=\"%s\"",
                filename);

        return SSPH->buffer;
    }
    
    /* Check if this handler wants to set header */
    if (SSPH->pfile->handler != NULL && (SSPH->flags & SSPF_SET_HEADER_H)) {
        SSP_HANDLER_CONTEXT_EXT cxt;
        
        cxt.type = SSP_HANDLER_SET_HEADER;
        cxt.page = SSPH->page;
        cxt.flags = SSPH->flags;
        cxt.url_data.string = NULL;

        /*
         * NOTE: This handler have to keep track of 
         *       whether it has already sent the header or not.
         *       (may simply turn off SSPF_SET_HEADER_H flag once set)
         */
        _SSP_DIAG_CALL(SSPH->pfile->handler);
        SSP_TRACING();
        r = (*SSPH->pfile->handler)((SSP_HANDLER_CONTEXT *)&cxt, wh);
        SSP_TRACING();
        if (r == SSP_HANDLER_RET_MODIFIED) {
            /* Handler has changed string, data entry or flags */
            SSPH->page = cxt.page;
            SSPH->flags = cxt.flags;
            if (cxt.url_data.string_pair.name != NULL && 
                cxt.url_data.string_pair.name[0] != 0 &&
                cxt.url_data.string_pair.value != NULL) {
                    
                SSP_TRACING();
                sal_sprintf(SSPH->buffer,
                        "%s: %s",
                        cxt.url_data.string_pair.name,
                        cxt.url_data.string_pair.value);

                return SSPH->buffer;
            }
        }
    }

    SSP_TRACING();
    return NULL;
}

#ifdef WCP_REXMIT_SUPPORT
const uint8 * 
wcp_reply_content_data(WCP_HANDLE wh, uint32 *len, int rexmit)
#else  /* WCP_REXMIT_SUPPORT */
const uint8 * 
wcp_reply_content_data(WCP_HANDLE wh, uint32 *len)
#endif  /* WCP_REXMIT_SUPPORT */
{
    const uint8 *p_pdata = NULL;
    uint32 blen = 0;
    SSPLOOP_RETVAL loop_ret;
    static SSPTAG_PARAM params[SSP_MAX_NUM_PARAMETERS];
    static SSPVAR_RET var_ret;
    
    RES_CONST_DECL SSP_DATA_ENTRY  *ssp_page;

    if (wh == NULL) 
        return NULL;
    
    SSP_ENTER(wcp_reply_content_data);
    WCP_DBG(("wcp_reply_content_data(%s): %u", SSPH->pfile->name, *len));
    
    if (*len == 0) 
        return (uint8 *)""; /* For safety */

#ifdef WCP_REXMIT_SUPPORT
    if (rexmit){
        _ssp_rexmit_info_action(wh, SSP_REXMIT_RESTORE);
    } else {
        _ssp_rexmit_info_action(wh, SSP_REXMIT_BACKUP);
    }
#endif  /* WCP_REXMIT_SUPPORT */
        
    /* Check if anything left in the last call of SSPVAR function */
    SSP_TRACING();
    if (SSPH->buflen > 0) {
        *len = (SSPH->buflen > *len) ? (*len) : SSPH->buflen;

        SSP_TRACING();
        p_pdata = (uint8 *)(SSPH->buffer + SSPH->offset);
        SSPH->buflen -= *len;

        if (SSPH->buflen == 0) {
            SSPH->offset = 0; 
        } else {
            SSPH->offset += *len;
        }
        
        SSP_TRACING();
        return p_pdata;
    }

    /* Now get data from data entries */
    for(;;) {
        
        SSP_TRACING();
        if (SSPH->page == NULL) {
            return NULL; /* All entries has been processed */
        }

        SSP_TRACING();

        /* report the ssp page pointer */
        ssp_page = _ssp_page_agent(SSPH);

        switch(ssp_page->opbyte & SSP_DATA_TYPE_MASK) {
        case SSP_DATA_TYPE_TEXT:
            /* Determine number of bytes we have */
            blen = ((uint32)ssp_page->param1) - SSPH->offset;
            if (blen > *len) 
                blen = *len;

            /* These are to be sent */
            p_pdata = ((uint8 *)ssp_page->dataptr) + SSPH->offset;
            *len = blen;

            /* Advance data pointer */
            SSPH->offset += blen;

            /* Check if we have consumed all text in this entry */
            if (SSPH->offset == (uint32)ssp_page->param1) {
                if (ssp_page->opbyte & SSP_DATA_FLAG_LAST) {
                    SSPH->page = NULL; /* No more entries */
                } else {
                    (SSPH->page)++;
                    SSPH->offset = 0;
                }
            }
            if (*len) {
                SSP_TRACING();
                return p_pdata;
            }
            SSP_TRACING();
            break;
            
        case SSP_DATA_TYPE_LOOP:
            blen = 0; /* Re-used for jumping back or not */
            if (ssp_page->dataptr == NULL) { /* Loop End */

                SSP_TRACING();
                if (SSPH->depth == 0) {
                    SSP_DBG(("*** SSP BUG: loop end when depth=0!"));
                    SSP_TRACING();
                    return NULL;
                }

                /* Increase loop index */
                SSPH->loops[SSPH->depth - 1]++;
                
                /* Go to loop start */
                blen = 1; /* Jumping back */
                (SSPH->page) -= ssp_page->pidx;

                /* wcp page agent need be updated due to SSPH->page is 
                 *  changed and will be referenced later. 
                 */
                ssp_page = _ssp_page_agent(SSPH);
                
                SSP_TRACING();
            }

            /* Loop Start */
            if (blen == 0) { /* New loop */
                SSP_TRACING();
                if (SSPH->depth == SSP_MAX_LOOP_DEPTH) {
                    SSP_DBG(("*** SSP BUG: new loop when depth=%d!",
                            SSP_MAX_LOOP_DEPTH));
                    SSP_TRACING();
                    return NULL;
                }

                SSPH->loops[SSPH->depth] = 0; /* loop index from 0 */
                SSPH->depth++; /* Advance depth */
                SSP_TRACING();
            }

            if (0 == parse_parameters(SSPH, params)) {
                SSP_TRACING();
                return NULL; /* parser bug (maybe), let's quit */
            }
            
            /* Call function to see if we can proceed */
            SSP_DBG(("Loop tag %p (%u,%u,%u) for %s", 
                    SSPH->page->dataptr, 
                    (int)params[0], (int)params[1], (int)params[2],
                    SSPH->pfile->name));
            _SSP_DIAG_CALL(SSPH->page->dataptr);
            _SSP_DIAG_PARAMS();
            SSP_TRACING();
            loop_ret = (*((SSPLOOP_CBKFUNC)ssp_page->dataptr)) 
                    (
                    params, 
                    SSPH->loops[SSPH->depth - 1], 
                    (void *)wh
                    );
            SSP_TRACING();

            if (loop_ret == SSPLOOP_STOP) {

                SSP_DBG(("\tSTOP")); 
                   
                /* Decrease depth*/
                if (SSPH->depth == 0) {
                    SSP_DBG(("*** SSP BUG: exit loop when depth=0!"));
                    SSP_TRACING();
                    return NULL;
                }
                SSPH->depth--; 

                /* Go to loop end first */
                (SSPH->page) += ssp_page->pidx;

                /* wcp page agent need be updated due to SSPH->page is 
                 *  changed and will be referenced later. 
                 */
                ssp_page = _ssp_page_agent(SSPH);
 
                if (ssp_page->opbyte & SSP_DATA_FLAG_LAST) {
                    /* No more entries */
                    SSPH->page = NULL;
                    SSP_TRACING();
                    return NULL;
                }
                SSP_TRACING();
            }  else {
                SSP_DBG(("\tPROCEED")); 
            }
            
            /* Go to next entry */
            (SSPH->page)++;

            blen = 0;
            break;
            
        case SSP_DATA_TYPE_VAR:
            /* Prepare parameters for SSPVAR function */
            var_ret.type = SSPVAR_RET_NULL;
            var_ret.val_data.string = NULL;
            SSP_TRACING();
            if (0 == parse_parameters(SSPH, params)) {
                SSP_TRACING();
                return NULL; /* parser bug (maybe), let's quit */
            }

            /* Call SSPVAR function to get data */
            SSP_DBG(("Var  tag %p (%u,%u,%u) for %s", 
                    SSPH->page->dataptr, 
                    (int)params[0], (int)params[1], (int)params[2],
                    SSPH->pfile->name));
            _SSP_DIAG_CALL(SSPH->page->dataptr);
            _SSP_DIAG_PARAMS();
            SSP_TRACING();
            (*((SSPVAR_CBKFUNC)ssp_page->dataptr)) 
                    (
                    params, 
                    &var_ret, 
                    (void *)wh
                    );
            SSP_TRACING();
            
            if (var_ret.type == SSPVAR_RET_INTEGER) {
                SSP_DBG(("\tRet: integer %d", (int16)var_ret.val_data.integer));
                SSP_TRACING();
                sal_sprintf(SSPH->buffer, "%d", (int16)var_ret.val_data.integer);
                p_pdata = (uint8 *)SSPH->buffer;
                blen = sal_strlen(SSPH->buffer);
                if (blen > *len) { /* Cannot send all for this time */
                    SSP_TRACING();
                    SSPH->offset = *len;
                    SSPH->buflen = blen - (*len);
                    blen = *len;
                }
            } else if (var_ret.type == SSPVAR_RET_STRING) {
                SSP_DBG(("\tRet: string \"%s\"", 
                        var_ret.val_data.string ? var_ret.val_data.string : ""));
                SSP_TRACING();
                if (var_ret.val_data.string == NULL) {
                    SSP_TRACING();
                    blen = 0;
                } else {
                    SSP_TRACING();
                    blen = sal_strlen(var_ret.val_data.string);
                }
                if (blen) {
                    SSP_TRACING();
                    p_pdata = (uint8 *)var_ret.val_data.string;
                    if (blen > *len) {
                        SSP_TRACING();
                        if (blen - *len > SSP_SESSION_BUFFER_SIZE) {
                            /* Incorrect behavior or bug from sspvar: quit */
                            SSP_DBG(("*** BUG: SSPVAR func - too many bytes"));
                            SSP_TRACING();
                            return NULL;
                        }
                        sal_memcpy(
                            SSPH->buffer, 
                            (var_ret.val_data.string + *len), 
                            blen - *len
                            );
                        SSPH->offset = 0;
                        SSPH->buflen = blen - *len;
                        blen = *len;
                    }
                }
            } else if (var_ret.type == SSPVAR_RET_BINARY) {
                SSP_DBG(("\tRet: binary %u bytes at %p)", 
                        var_ret.length, var_ret.val_data.address));
                SSP_TRACING();
                if (var_ret.val_data.address == NULL) {
                    SSP_TRACING();
                    blen = 0;
                } else {
                    SSP_TRACING();
                    blen = var_ret.length;
                    if (blen > SSPVAR_MAX_RAWDATA_BYTES) {
                        SSP_DBG(("*** Too many bytes; data truncated."));
                        SSP_TRACING();
                        blen = SSPVAR_MAX_RAWDATA_BYTES;
                    }
                }
                if (blen) {
                    SSP_TRACING();
                    p_pdata = var_ret.val_data.address;
                    if (blen > *len) {
                        SSP_TRACING();
                        if (blen - *len > SSP_SESSION_BUFFER_SIZE) {
                            /* Incorrect behavior or bug from sspvar: quit */
                            SSP_DBG(("*** BUG: SSPVAR func - too many bytes"));
                            SSP_TRACING();
                            return NULL;
                        }
                        sal_memcpy(
                            SSPH->buffer, 
                            (var_ret.val_data.address + *len), 
                            blen - *len
                            );
                        SSPH->offset = 0;
                        SSPH->buflen = blen - *len;
                        blen = *len;
                    }
                }
            } else {
                SSP_DBG(("\tRet: (empty)"));
            }

            /* advance to next entry */
            if (ssp_page->opbyte & SSP_DATA_FLAG_LAST) {
                SSP_TRACING();
                SSPH->page = NULL; /* No more entries */
            } else {
                SSP_TRACING();
                (SSPH->page)++;
            }

            /* Check if we got any data */
            if (blen) {
                *len = blen;
                SSP_TRACING();
                return p_pdata;
            }
            SSP_TRACING();
            break; /* Got no bytes to send, try next entry */
            
        case SSP_DATA_TYPE_CUSTOM:
            SSP_TRACING();
            return NULL;
        default:
            SSP_DBG(("*** SSP BUG: unknown opbyte in SSP data entry!"));
            return NULL;
        }

        /* Loop again (only if nothing to send for the previous entry) */
    }
}

void
wcp_close(WCP_HANDLE wh)
{
    SSP_PSMEM_ENTRY *pm = SSPH->psmem;
    SSP_PSMEM_ENTRY *npm;
    
    if (wh == NULL) 
        return;

    SSP_ENTER(wcp_close);
    if (SSPH->pfile) {
        WCP_DBG(("wcp_close(%s)", SSPH->pfile->name));
        SSP_DBG(("Close URL: %s", SSPH->pfile->name));
    }

    /* Call global handler if flag set */
    if ((SSP_GFLAGS & SSPF_OPEN_CLOSE_H) && 
        SSP_GHANDLER != NULL) {
        
        ssp_global_cxt.type = SSP_HANDLER_CLOSE;
        ssp_global_cxt.pfile = SSPH->pfile;
        ssp_global_cxt.page = SSPH->page;
        ssp_global_cxt.flags = SSPH->flags;
        ssp_global_cxt.url_data.string = NULL;
        
        SSP_TRACING();
        (*SSP_GHANDLER)(&ssp_global_cxt, wh);
        SSP_TRACING();
    }

    /* Call handler if SSPF_OPEN_CLOSE_H set */
    if (SSPH->pfile != NULL
        && SSPH->pfile->handler != NULL 
        && (SSPH->flags & SSPF_OPEN_CLOSE_H)) {

        SSP_HANDLER_CONTEXT_EXT cxt;
        
        cxt.type = SSP_HANDLER_CLOSE;
        cxt.page = SSPH->page;
        cxt.flags = SSPH->flags;
        cxt.url_data.string = NULL;

        _SSP_DIAG_CALL(SSPH->pfile->handler);
        SSP_TRACING();
        (*SSPH->pfile->handler)((SSP_HANDLER_CONTEXT *)&cxt, wh);
        SSP_TRACING();
    }

    /* Free all per-session memory */
    for(; pm != NULL; pm = npm) {
        npm = pm->next;
        KFREE(pm);
    }
    
    KFREE(SSPH);
    SSP_TRACING();
}

void *
wcputil_memcpy(void *dest, const void *src, size_t n, WCP_HANDLE wh)
{
#ifdef CFG_BANKED_MEMCPY_SUPPORT
    SSP_DATA_BANK   bank_id;
    SSP_FILE_ENTRY  *ssp_file;
    
    /* retrieve the bank_id which is this WCP belong to */
    if (wh == NULL) {
        bank_id = SSP_BANK0;
    } else {
        ssp_file = SSPH->pfile;
        if (ssp_file == NULL) {
            bank_id = SSP_BANK0;
        } else {
            bank_id = ssp_file->bank_id;
        }
    }

    /* For SSP current design in UM, the Web related bank-switch design 
     *  limited in bank4 and bank5 .
     */
    if (bank_id == SSP_BANK4) {
        sal_memcpy_bank4(dest, src, n);
    } else if (bank_id == SSP_BANK5) {
        sal_memcpy_bank5(dest, src, n);
    } else {
        /* call default memcpy for no bank switch condition */
        sal_memcpy(dest, src, n);
    }
#else   /* CFG_BANKED_MEMCPY_SUPPORT */
    /* for compiler complain */
    wh = wh;

    sal_memcpy(dest, src, n);
#endif  /* CFG_BANKED_MEMCPY_SUPPORT */
    return dest;
}

RES_CONST_DECL SSP_DATA_ENTRY *
ssputil_locate_file(SSP_PSMH psmem, const char *path, SSP_FILE_FLAGS *pflag)
{
    SSP_FILE_ENTRY *pfile;
    struct SSP_FILESYSTEM *pfs;
    
    if (psmem == NULL) {
        return NULL;
    }
    
    pfs = ((SSP_SESSION *)psmem)->filesystem;
        
    pfile = locate_file_entry(&pfs, path);
    if (pfile == NULL) {
        return NULL;
    }

    if (pflag) {
        *pflag = pfile->flags;
    }

    return pfile->file_data;
}

void *
ssputil_psmem_alloc(SSP_PSMH handle, void *key, int size)
{
    SSP_PSMEM_ENTRY *pm;

    SSP_DBG(("ssputil_psmem_alloc: session=%p key=%p size=%d", 
        handle, key, size));

    /* The handle is actually the pointer of SSP session */
    if (handle == NULL) {
        return NULL;
    }
    
    /* Check if we already have the key */
    pm = ((SSP_SESSION *)handle)->psmem;
    for(; pm != NULL; pm = pm->next) {
        if (pm->key == key) {
            break;
        }
    }

    /* Found existing psmem for this key */
    if (pm != NULL) {
        /* If the new request wants more size, then we re-alloc it.
         * Otherwise (new size <= old size), we return the old mem. */
        if (size <= pm->size) {
            SSP_DBG(("\tFound existing: %p size=%d", pm, pm->size));
            return ((void *)pm) + sizeof(SSP_PSMEM_ENTRY);
        } else {
            /* Free it first for re-allocation */
            SSP_DBG(("\tFound existing (re-alloc): old size=%d", pm->size));
            ssputil_psmem_free(handle, key);
        }
    }

    /* XXX: Use static memory as pool */
    pm = (SSP_PSMEM_ENTRY *)KMALLOC(sizeof(SSP_PSMEM_ENTRY) + size, 4);
    if (pm == NULL) {
        SSP_DBG(("*** ERROR: fail to allocate memory!"));
        return NULL;
    }

    /* Insert into the chain */
    if (((SSP_SESSION *)handle)->psmem == NULL) {
        ((SSP_SESSION *)handle)->psmem = pm;
        pm->next = NULL;
    } else {
        pm->next = ((SSP_SESSION *)handle)->psmem;
        ((SSP_SESSION *)handle)->psmem = pm;
    }
    pm->key = key;
    pm->size = size;

    SSP_DBG(("\tAllocated: %p", pm));
    return ((void *)pm) + sizeof(SSP_PSMEM_ENTRY);
}

void *
ssputil_psmem_get(SSP_PSMH handle, void *key)
{
    /* The handle is actually the pointer of SSP session */
    SSP_PSMEM_ENTRY *pm;

    SSP_DBG(("ssputil_psmem_get: session=%p key=%p", 
        handle, key));

    /* The handle is actually the pointer of SSP session */
    if (handle == NULL) {
        return NULL;
    }
    
    /* Try to find the key */
    pm = ((SSP_SESSION *)handle)->psmem;
    for(; pm != NULL; pm = pm->next) {
        if (pm->key == key) {
            SSP_DBG(("\tFound: %p size=%d", pm, pm->size));
            return ((void *)pm) + sizeof(SSP_PSMEM_ENTRY);
        }
    }

    SSP_DBG(("\tNot found."));
    return NULL;
}

int
ssputil_psmem_getsize(SSP_PSMH handle, void *key)
{
    /* The handle is actually the pointer of SSP session */
    SSP_PSMEM_ENTRY *pm;

    SSP_DBG(("ssputil_psmem_getsize: session=%p key=%p", 
        handle, key));

    if (handle == NULL) {
        return 0;
    }

    /* Try to find the key */
    pm = ((SSP_SESSION *)handle)->psmem;
    for(; pm != NULL; pm = pm->next) {
        if (pm->key == key) {
            SSP_DBG(("\tFound: %p size=%d", pm, pm->size));
            return pm->size;
        }
    }

    SSP_DBG(("\tNot found."));
    return 0;
}

void 
ssputil_psmem_free(SSP_PSMH handle, void *key) 
{
    /* The handle is actually the pointer of SSP session */
    SSP_PSMEM_ENTRY *ppm = NULL; /* previous one */
    SSP_PSMEM_ENTRY *pm;

    SSP_DBG(("ssputil_psmem_free: session=%p key=%p", 
        handle, key));

    if (handle == NULL) {
        return;
    }

    /* Try to find the key */
    pm = ((SSP_SESSION *)handle)->psmem;
    for(; pm != NULL; ppm = pm, pm = pm->next) {
        if (pm->key == key) {
            if (ppm == NULL) {
                ((SSP_SESSION *)handle)->psmem = pm->next;
            } else {
                ppm->next = pm->next;
            }

            /* Free it */
            KFREE(pm);
            return;
        }
    }

    /* Not found */
    return;
}

int
ssputil_get_session_socket(SSP_PSMH handle, int *secure)
{
    if (handle == NULL) {
        return -1;
    }
    
    if (secure) {
        *secure = 
            (((SSP_SESSION *)handle)->service->flags & HTTPSVC_FLAG_HTTPS) ?
            TRUE : FALSE;
    }
    
    return ((SSP_SESSION *)handle)->socket;
}

