/*
 * $Id: httpd.c,v 1.15 Broadcom SDK $
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
#include "uip.h"
#include "utils/net.h"
#include "httpd_arch.h"
#include "httpd_wcp.h"
#include "appl/httpd.h"
#include "appl/ssp.h"

#if CFG_HTTPD_ENABLED
/*  *********************************************************************
    *  Configurations
    ********************************************************************* */
#ifdef HTTPD_MAX_SESSIONS
#define HTTPD_SESS_CNT  HTTPD_MAX_SESSIONS
#else   /* HTTPD_MAX_SESSIONS */
/* this value must be less than the count of TCP/IP connections */
#define HTTPD_SESS_CNT  (10)
#endif  /* HTTPD_MAX_SESSIONS */

#define HTTPD_SERV_CNT  HTTPD_MAX_SERVICES

#define HTTPD_SESSION_LIFE_TIME (45)    /* seconds */
#define HTTPD_DEFAULT_MTU_SIZE  (1400) /* Maximal size for one send() */
/*
 * HTTPD_DEFAULT_MTU_SIZE should be set to a reasonable bigger value 
 * so that PSH flag isn't set for every segment for a complete HTTP response.
 * This significantly improves performance with Windows clients.
 */

#define HTTPD_TCP_RX_BUF_SIZE   HTTPD_SHARED_BUF_SIZE
#define HTTPD_TCP_TX_BUF_SIZE   HTTPD_DEFAULT_MTU_SIZE

/*  *********************************************************************
    *  Debugging and diagnostic
    ********************************************************************* */
    
#if HTTPD_DIAG || HTTPD_TRACE

#define fflush(x)

#if HTTPD_DIAG
#define _HTTPD_DIAG() httpd_diag_last_executed = __LINE__
#else 
#define _HTTPD_DIAG()
#endif /* HTTPD_DIAG */

#if HTTPD_TRACE
#define _HTTPD_TRACE() do { \
    sal_sprintf("httpd:%d\n", __LINE__); \
    fflush(stdout); \
} while(0)
#else 
#define _HTTPD_TRACE()
#endif /* HTTPD_TRACE */

#define HTTPD_TRACING() do { _HTTPD_DIAG(); _HTTPD_TRACE(); } while(0)

#else /* !(HTTPD_DIAG || HTTPD_TRACE) */

#define HTTPD_TRACING()

#endif /* !(HTTPD_DIAG || HTTPD_TRACE) */

#if HTTPD_DIAG 
extern void cmd_diaghttpd(int argc, char *argv[]);
#endif  /* HTTPD_DIAG */

#if HTTPD_DIAG
#define HTTPD_DIAG_PRINTF(fmt, args...) sal_sprintf(httpd_diag_buf, fmt, ##args)
#define HTTPD_DIAG_SETTIME() do { httpd_diag_last_tick = sal_time(); } while(0)
#define MAX_HTTPD_DIAG_BUF_LEN  (256)
#else /* !HTTPD_DIAG */
#define HTTPD_DIAG_PRINTF(x) 
#define HTTPD_DIAG_SETTIME() do { }while(0)
#define MAX_HTTPD_DIAG_BUF_LEN  (4)
#endif /* HTTPD_DIAG */


/*  *********************************************************************
    *  Structures & type definitions
    ********************************************************************* */
/* session state */
typedef enum {
    HTTPD_STATE_CLOSED,
    HTTPD_STATE_LISTENING,
    HTTPD_STATE_CONNECTED,
    HTTPD_STATE_READ_HEADERS,
    HTTPD_STATE_READ_CONTENT,
    HTTPD_STATE_SEND_HEADERS,
    HTTPD_STATE_SEND_CONTENT,
    HTTPD_STATE_SEND_DONE,
} HS_STATE;

#ifdef WCP_REXMIT_SUPPORT
typedef struct {
    HS_STATE        state;          /* session state */
    int             include_header;
} WCP_REXMIT_INFO;
#endif /* WCP_REXMIT_SUPPORT */

typedef struct {
    int16           socket;         /* TCP socket */
    HS_STATE        state;          /* session state */
    WCP_HANDLE      wcph;           /* page handle from WCP */
    int32           content_length; /* for both request & reply */
    sal_time_t      ticks;          /* time of update */
    HTTP_SERVICE    *service;       /* service */
    uint16          mss;            /* max segment size */
#ifdef WCP_REXMIT_SUPPORT
    WCP_REXMIT_INFO rexmit_info;
#endif /* WCP_REXMIT_SUPPORT */

} HTTP_SESSION;

/* Timer callback entry */
typedef struct _HTTPD_TIMER_ENTRY {
    uint16 seconds;
    sal_time_t start_time;
    void *user_data;
    void (*callback)(void *);
    struct _HTTPD_TIMER_ENTRY *next;
} HTTPD_TIMER_ENTRY;

/* Timer callback */
static HTTPD_TIMER_ENTRY *timer_list = NULL;


/* http service table for init */
extern struct SSP_FILESYSTEM ssp_filesystem_table[];
STATIC HTTP_SERVICE CODE http_serv_table[] = {
    {"WEB", HTTP_TCP_PORT, 0, (void *)ssp_filesystem_table, NULL},
    { "", 0, 0, NULL, NULL}
};
#define HTTPD_SERV_WEB  (0)     /* web service id in service table */

/*  *********************************************************************
    *  Globals
    ********************************************************************* */

static HTTP_SESSION httpd_sessions[HTTPD_SESS_CNT];
extern struct SSP_FILESYSTEM ssp_filesystem_table[];
static HTTP_SERVICE http_services[HTTPD_SERV_CNT];

static char httpd_buf[HTTPD_SHARED_BUF_SIZE];
static int32 hbuf_idx;
static int32 hbuf_len;

#if HTTPD_DEBUG_LINKDROP
/* 1. Since CONSOLE might be inactive while all ports linkdrop in UM project, 
 *  we need the way to check if all creaeted httpd sessions can be closed 
 *  properly. Here we use the count for created and closed event per HTTPD 
 *  session for debugging purpose.
 *
 * 2. httpd_new_sess_cnt[] and  httpd_close_sess_cnt[] on each item is 
 *  expected will be the same any time while there is no HTTP traffic on device. 
 *  Once all ports linkdrop event occurred when multiple HTTPD sessions still 
 *  active, we expect uIP.TCP(for both IPv4 and IPv6) will issue the 
 *  notification per active session.
 */
int httpd_new_sess_cnt[HTTPD_SESS_CNT];                                                            
int httpd_close_sess_cnt[HTTPD_SESS_CNT];
#endif
/*
 * For better maintainence of scrambled code between cfe and wss,
 * these variables are always there.
 */
#if HTTPD_DIAG
static sal_time_t httpd_diag_last_tick;
static int16 httpd_diag_last_executed;
static char httpd_diag_buf[MAX_HTTPD_DIAG_BUF_LEN] = { 0 };
#endif /* HTTPD_DIAG */

/*  *********************************************************************
    *  Functions
    ********************************************************************* */

STATICFN void 
httpd_close_session(HTTP_SESSION *ps)
{
#if HTTPD_DEBUG_LINKDROP
    int i;
#endif  /* HTTPD_DEBUG_LINKDROP */
    HTTPD_DBG(("httpd_close_session: ps=%p, wcph=%p,socket=%d", 
            ps, ps->wcph, ps->socket));

    if (ps == NULL){
        /* this session is closed already */
        return;
    }
    
#if HTTPD_DEBUG_LINKDROP
    HTTPD_LINKDROP_PRINTF(("Session at ps=%p(id=%d) closing ...\n",
            ps, (int)HTTPD_TCPAPPSTATE_THIS));
    httpd_close_sess_cnt[HTTPD_TCPAPPSTATE_THIS] ++;
#endif  /* HTTPD_DEBUG_LINKDROP */
    if (ps->wcph != NULL) {
        HTTPD_TRACING();
        wcp_close(ps->wcph);
        HTTPD_TRACING();
        ps->wcph = NULL;
    }
    if (ps->socket != -1) {
        HTTPD_TRACING();
        socket_close(ps->socket);
        HTTPD_TRACING();
        ps->socket = -1;

        /* clear uip_conn session index */
        HTTPD_TCPAPPSTATE_CLEAR;

    }
    ps->mss = HTTPD_DEFAULT_MTU_SIZE;
    ps->state = HTTPD_STATE_CLOSED;
    HTTPD_STATE_PRINTF(("@ httpd_close_session:ps=%p closed! state change to %d!!\n",
            ps, (int)ps->state));
    HTTPD_REXMIT_PRINTF(("@ httpd_close_session:ps=%p closed!\n",ps));
    ps->service = NULL;

#if HTTPD_DEBUG_LINKDROP
    HTTPD_LINKDROP_PRINTF(("== Closed session info[]={"));
    for (i = 0; i < HTTPD_SESS_CNT; i++){
        HTTPD_LINKDROP_PRINTF(("%d%s", httpd_close_sess_cnt[i],
                (i == (HTTPD_SESS_CNT - 1)) ? "}\n" : ","));
    }
#endif  /* HTTPD_DEBUG_LINKDROP */
}

STATICFN char *
httpd_gets(void)
{
    /* Since browsers always cut headers at the end of a line,
     * we don't need to do complicated processing for incomplete line. */
    int32 org_idx = hbuf_idx;
    char *p = httpd_buf + hbuf_idx;
    
    if (hbuf_idx == hbuf_len)
        return p;
    
    while(hbuf_idx < hbuf_len) {
        if (*p == '\n') {
            *p = 0;
            if (*(p - 1) == '\r') {
                *(p - 1) = 0;
            }
            
            hbuf_idx++;
            return httpd_buf + org_idx;
        }
        
        p++;
        hbuf_idx++;
    }
    return NULL;
}

STATICFN void 
httpd_process(HTTP_SESSION *ps, uint16 flags)
{
    int32 r;
    uint8 *pbuf, *ptk, *ptk2;
    uint8 start_send = 0; /* 1: changing state from req to response */

#ifdef WCP_REXMIT_SUPPORT
    WCP_REXMIT_INFO *rexmit_info;
    int rexmit;

    rexmit_info = &(ps->rexmit_info);
    if (flags == TCPCBK_REXMIT) {
        HTTPD_REXMIT_PRINTF(("httpd_process: REXMIT action!! ps=%p,(include_header=%d,state=%d)\n", 
                ps,rexmit_info->include_header,(int)(rexmit_info->state)));
        rexmit = 1;
        flags = TCPCBK_ACK;
        if (rexmit_info->include_header) {
            /* process will be started from SEND_HEADER */
            start_send = 1;
        } else {
            if (rexmit_info->state == HTTPD_STATE_SEND_DONE){
                /* HTTPD_STATE_SEND_DONE here means all HTTPD data for this
                 *  session is trasmitted and ACKed already. Expected no 
                 *  rexmit here.
                 */
#ifndef __BOOTLOADER__
                HTTPD_LOG(("httpd_process: unexpect REXMIT on closed session!\n"));
#endif  /* !__BOOTLOADER__ */
            }
            ps->state = rexmit_info->state;
        }
    } else {
        rexmit = 0;
    }
#endif /* WCP_REXMIT_SUPPORT */

    HTTPD_DIAG_PRINTF(("httpd_process: ps=%p flags=0x%x state=%d wcph=%p ticks=%lu\n",
            ps, flags, (int)ps->state, ps->wcph, sal_time()));

    HTTPD_DBG(("httpd_process: ps=%p flags=0x%x state=%d wcph=%p ticks=%lu\n",
            ps, flags, (int)ps->state, ps->wcph, sal_time()));

    HTTPD_TRACING();

    if (flags & TCPCBK_CLOSED) {
        HTTPD_TRACING();
        httpd_close_session(ps);
        HTTPD_TRACING();
        return;
    }

    if (flags & TCPCBK_CONNECTED) {

        HTTPD_TRACING();
        if (ps->state == HTTPD_STATE_LISTENING) {
            
            HTTPD_TRACING();
            /* Initialize all sessions fields when connected */
            ps->state = HTTPD_STATE_CONNECTED;
            ps->wcph = NULL;
            ps->ticks = sal_time();
            ps->content_length = 0;

            if ((flags & TCPCBK_NEWDATA) == 0) {
                return;
            }
        } else {
            HTTPD_TRACING();
            return; /* duplicated "connected" message */
        }
    }

    if ((flags & TCPCBK_POLL) && 
        (ps->state != HTTPD_STATE_CLOSED) && 
        (ps->state != HTTPD_STATE_LISTENING)) {
            
        HTTPD_TRACING();
        if (sal_time() - ps->ticks >= HTTPD_SESSION_LIFE_TIME) {
            HTTPD_TRACING();
#ifndef __BOOTLOADER__
            HTTPD_LOG(("httpd: session %p (wcph=%p) timeout!\n", ps, ps->wcph));
#endif /* !__BOOTLOADER__ */
            httpd_close_session(ps);
            return;
        }
    }

    if (flags & TCPCBK_NEWDATA) {
        int16 bpost;

        ps->ticks = sal_time();
        
        /* Retrieve all rx data */
        HTTPD_TRACING();
        hbuf_len = socket_recv(ps->socket, (uint8 *)httpd_buf, HTTPD_SHARED_BUF_SIZE, 0);
        HTTPD_TRACING();
        hbuf_idx = 0;

        if (hbuf_len == 0) { /* shouldn't happen */
            HTTPD_TRACING();
            return;
        }
        
        HTTPD_TRACING();
        switch(ps->state) {

        case HTTPD_STATE_CONNECTED:
            HTTPD_STATE_PRINTF(("@ HTTPD_STATE_CONNECTED:ps=%p,state=%d\n",
                    ps,(int)ps->state));
            HTTPD_TRACING();
            pbuf = (uint8 *)httpd_gets();
            if (pbuf == NULL) { /* ERROR: Request line not complete! */
                HTTPD_TRACING();
                HTTPD_DBG(("httpd_process: No request! httpd closing...",  ps));
                HTTPD_STATE_PRINTF(("httpd_process: No request! httpd closing...",
                        ps));
                httpd_close_session(ps);
                return;
            }

            /* parse method */
            HTTPD_TRACING();
            ptk = (uint8 *)gettoken((char **)&pbuf);
            bpost = -1;
            if (ptk) {
                if (!strcmpi((char *)ptk, "GET")) {
                    bpost = 0;
                } else if (!strcmpi((char *)ptk, "POST")) {
                    bpost = 1;
                } /* else - ERROR: not GET or POST */ 
                
                /* parse path */
                ptk = NULL;
                if (bpost != -1) {
                    /* Get URI */
                    ptk = (uint8 *)gettoken((char **)&pbuf);
                }
                if (ptk) {
                    /* Get parameters appended at the end of URI */
                    ptk2 = (uint8 *)sal_strchr((const char *)ptk, '?');
                    if (ptk2) {
                        *ptk2 = 0;
                        ptk2++;
                    }

                    /* We have got all info we need, call WCP for file handle */
                    HTTPD_TRACING();
                    HTTPD_DBG(("httpd_process: ps=%p method=%s,url=%s\n", 
                            ps, (bpost) ? "POST" : "GET", ptk));
                    HTTPD_STATE_PRINTF(("httpd_process: ps=%p method=%s,url=%s\n", 
                            ps, (bpost) ? "POST" : "GET", ptk));
                    HTTPD_REXMIT_PRINTF(("httpd_process: ps=%p method=%s,url=%s\n", 
                            ps, (bpost) ? "POST" : "GET", ptk));
                    ps->wcph = wcp_open(
                                (uint8)bpost, 
                                (char *)ptk, 
                                (char *)ptk2, 
                                ps->socket, 
                                ps->service
                                );
                    HTTPD_TRACING();
                    if (ps->wcph == NULL) { /* ERROR: File not found! */
                        bpost = -1; 
                    }
                } else { /* No URI or not GET/POST */
                    bpost = -1;
                }

                /* Let's save some CPU time by not checking "HTTP/1.x" */
            }
            
            if (bpost == -1) { /* Invalid request or file not found */
                HTTPD_TRACING();
                HTTPD_DBG(("httpd_process: invalid reqeust! httpd closing...\n",  ps));
                HTTPD_STATE_PRINTF(("\t Invalid reqeust! httpd closing...\n",  ps));
                httpd_close_session(ps);
                return;
            }

            HTTPD_TRACING();
            ps->state = HTTPD_STATE_READ_HEADERS;
            HTTPD_STATE_PRINTF(("@ HTTPD_STATE_READ_HEADERS: state changed to %d!!\n",
                    (int)ps->state));
            
            /* Fall through */
            
        case HTTPD_STATE_READ_HEADERS:
            HTTPD_TRACING();
            for(;;) {
                HTTPD_STATE_PRINTF(("@ HTTPD_STATE_READ_HEADERS:ps=%p,state=%d\n",
                        ps, (int)ps->state));
                if (hbuf_idx == hbuf_len) { 
                    /* All data in buffer has been consumed,
                     * but still some headers has not yet been received. */
                    return;
                }
                HTTPD_TRACING();
                pbuf = (uint8 *)httpd_gets();
                if (pbuf == NULL) { /* Header line not complete */
                    /* XXX Actually we shouldn't expect header lines are always
                     *     complete (ending with CR/LF). However, in this httpd 
                     *     design, we try to catch up the step of TCP layer and
                     *     handle every packet it received as soon as possible.
                     *     Besides, all browsers seem to split packet by lines
                     *     (except for binary post data). Therefore, let's
                     *     assume header lines are always complete for now. */
                    HTTPD_TRACING();
                    HTTPD_STATE_PRINTF(("\t Header line not complete! httpd closing...\n",  
                            ps));
                    httpd_close_session(ps);
                    return;
                }
                if (*pbuf == '\0') { /* Simply CR/LF: end of headers */
                    HTTPD_TRACING();
                    wcp_req_header(ps->wcph, NULL, NULL); /* Done: headers */
                    HTTPD_TRACING();
                    break;
                }
                
                ptk = (uint8 *)gettoken((char **)&pbuf);

                /* ptk: header name; pbuf: header value; */
                if (ptk) {
                    ptk[sal_strlen((char *)ptk) - 1] = '\0'; /* Assume "HEADER:" format */
                    if (!strcmpi((char *)ptk, "Content-Length")) {
                        ps->content_length = sal_atoi((char *)pbuf);
                        HTTPD_TRACING();
                        wcp_req_content_length(ps->wcph, 
                                (uint32)ps->content_length);
                        HTTPD_TRACING();
                    }
                    
                    /* Call WCP to process this header if applicable */
                    HTTPD_TRACING();
                    wcp_req_header(ps->wcph, (char *)ptk, (char *)pbuf);
                    HTTPD_TRACING();
                }
            }

            if (ps->content_length == 0) {
                /* No post data expected, let's start sending our reply */
                HTTPD_TRACING();
                ps->state = HTTPD_STATE_SEND_HEADERS;
                HTTPD_STATE_PRINTF(("@ HTTPD_STATE_READ_HEADERS: state changed to %d!!\n",
                        (int)ps->state));
                flags |= TCPCBK_ACK;
                start_send = 1;
                break;
            } else {
                HTTPD_TRACING();
                ps->state = HTTPD_STATE_READ_CONTENT;
                HTTPD_STATE_PRINTF(("@ HTTPD_STATE_READ_HEADERS: state changed to %d!!\n",
                        (int)ps->state));
                /* Fall through */
            }
            
        case HTTPD_STATE_READ_CONTENT:
            HTTPD_STATE_PRINTF(("@ HTTPD_STATE_CONNECTED:ps=%p,state=%d\n",
                    ps, (int)ps->state));
            if (ps->content_length && (hbuf_idx == hbuf_len)) { 
                HTTPD_TRACING();
                HTTPD_STATE_PRINTF(("@ HTTPD_STATE_CONNECTED:Come later..\n"));
                return; /* Not all content received; Come later. */
            }
            
            /* Call WCP to handle current post data we got */
            r = hbuf_len - hbuf_idx;
            HTTPD_TRACING();
            wcp_req_post_data(ps->wcph, (uint8 *)(httpd_buf + hbuf_idx), (uint32)r);
            HTTPD_TRACING();
            ps->content_length -= r;
            
            if (ps->content_length < 0) {
                HTTPD_TRACING();
                HTTPD_DBG(("httpd: BUG? recv data > content_length!"));
                ps->content_length = 0;
            }
            
            if (ps->content_length) {
                HTTPD_TRACING();
                HTTPD_STATE_PRINTF(("@ HTTPD_STATE_CONNECTED:Not all content received..\n"));
                return; /* Not all content received */
            } else {
                HTTPD_TRACING();
                wcp_req_post_data(ps->wcph, NULL, 0); /* Done: post data */
                HTTPD_TRACING();
                ps->state = HTTPD_STATE_SEND_HEADERS;
                HTTPD_STATE_PRINTF(("@ HTTPD_STATE_READ_CONTENT: state changed to %d!!\n",
                        (int)ps->state));
                flags |= TCPCBK_ACK;
                start_send = 1;
                break;
            }
            
        default:
            HTTPD_TRACING();
            HTTPD_DBG(("httpd: BUG? NEWDATA when state=%d", (int)ps->state));
        }
    }

    /* ACK is only useful for sending data */
    HTTPD_STATE_PRINTF(("@@ (flags & TCPCBK_ACK)=%s, ps=%p\n",
            (flags & TCPCBK_ACK) ? "TRUE" : "FALSE", ps));
    if (flags & TCPCBK_ACK) {
        
        HTTPD_TRACING();
        hbuf_idx = 0;
        
        /* get MTU from TCP layer*/
        HTTPD_DBG(("httpd_process: ps.mss=%d,uip_mss=%d\n", (int)ps->mss, uip_mss()));
        hbuf_len = ps->mss;
        
        if (start_send == 1) { /* Just begin to send back data */
        
#ifdef WCP_REXMIT_SUPPORT
            /* updated REXMIT informaiton */
            rexmit_info->include_header = 1;
#endif /* WCP_REXMIT_SUPPORT */
            /* Check WCP for web page status */
            HTTPD_TRACING();
            ptk = (uint8 *)wcp_req_end(ps->wcph, &ps->content_length);
            HTTPD_TRACING();
            if (ptk == NULL) {
                ptk = (uint8 *)"200 OK";
            }

            /* Send response line & default headers */
            sal_sprintf(httpd_buf, "HTTP/1.0 %s\r\nConnection: close\r\n", (char *)ptk);
            if (ps->content_length > 0) {
                HTTPD_TRACING();
                sal_sprintf(httpd_buf + sal_strlen(httpd_buf),
                    "Content-Length: %ld\r\n", ps->content_length);
            }
            
            hbuf_idx = sal_strlen(httpd_buf);
            ps->state = HTTPD_STATE_SEND_HEADERS;
            HTTPD_STATE_PRINTF(("@ (start_send == 1): state set to %d!!\n",
                    (int)ps->state));
        }
        
#ifdef WCP_REXMIT_SUPPORT
        /* clear WCP rexmit information in the begining of data replying 
         *
         * Note : 
         *  WCP rexmit state must be cleared for the REXMIT action !!
         */
        HTTPD_REXMIT_PRINTF(("@ HTTPD data replying...(ps=%p,rexmit=%d)\n", 
                ps, rexmit));
        if (!rexmit) {
            wcp_rexmit_clear(ps->wcph);
        }
#endif /* WCP_REXMIT_SUPPORT */
        HTTPD_TRACING();
        switch(ps->state) {
        case HTTPD_STATE_SEND_HEADERS:
            for(;;) {
                HTTPD_STATE_PRINTF(("@ HTTPD_STATE_SEND_HEADERS:ps=%p,state=%d\n",
                        ps, (int)ps->state));

                HTTPD_TRACING();
#ifdef WCP_REXMIT_SUPPORT
                ptk = (uint8 *)wcp_reply_headers(ps->wcph, rexmit);
#else /* WCP_REXMIT_SUPPORT */
                ptk = (uint8 *)wcp_reply_headers(ps->wcph);
#endif /* WCP_REXMIT_SUPPORT */
                HTTPD_TRACING();
                if (ptk) {
                    r = sal_strlen((char *)ptk);
                    if (hbuf_idx + r > hbuf_len) {
                        HTTPD_TRACING();
                        HTTPD_STATE_PRINTF(("@ HTTPD_STATE_SEND_HEADERS:special case!\n"));
                        /* XXX Because we can't tell WCP to re-generate header 
                         *     (this is what wcp_reply_headers() is designed),
                         *     we actually drop this header in current
                         *     implementation as it requires per-session 
                         *     storage to to save this header,
                         *     and this shouldn't happen if we control WCP
                         *     not to generate so lengthy headers.
                         */
                         break;
                    } else {
                        HTTPD_TRACING();
                        sal_strcat(httpd_buf, (char *)ptk);
                        sal_strcat(httpd_buf, "\r\n");
                        hbuf_idx += r + 2;
                        continue;
                    }
               } else {
                    HTTPD_TRACING();
                    sal_strcat(httpd_buf, "\r\n");
                    hbuf_idx += 2;
                    ps->state = HTTPD_STATE_SEND_CONTENT;
                    HTTPD_STATE_PRINTF(("@ HTTPD_STATE_SEND_HEADERS: state changed to %d!!\n",
                             (int)ps->state));
                    HTTPD_REXMIT_PRINTF(("@ HTTPD_STATE_SEND_HEADERS:(normal)break to send...(ps=%p)\n",
                        ps));
                    break; /* Done: additional headers by WCP */
                }
            }
            
            /* Fall through */
            
        case HTTPD_STATE_SEND_CONTENT:
            HTTPD_TRACING();
            if (ps->state != HTTPD_STATE_SEND_CONTENT)
                break;
#ifdef WCP_REXMIT_SUPPORT
            /* updated rexmit_info (for non-rexmit process) */
            if (hbuf_idx == 0 && !rexmit) {
                /* means the 1st http data(included header+content(parts))
                 *  has been replied and ACK already.
                 */
                HTTPD_REXMIT_PRINTF(("@ SEND_CONTENT:rexmit_info set include_header = 0,(ps=%p,)\n", ps));
                rexmit_info->include_header = 0;
                rexmit_info->state = HTTPD_STATE_SEND_CONTENT;
            }
#endif /* WCP_REXMIT_SUPPORT */
            for(;;) {
                HTTPD_STATE_PRINTF(("@ HTTPD_STATE_SEND_CONTENT:ps=%p,state=%d\n",
                        ps, (int)ps->state));
                if (hbuf_idx == hbuf_len) {
                    HTTPD_TRACING();
                    HTTPD_REXMIT_PRINTF(("@ SEND_CONTENT:(buff full)break to send...,ps=%p\n", ps));
                    break; /* buffer full; wait for next chance */
                }
                r = hbuf_len - hbuf_idx;
                HTTPD_TRACING();
#ifdef WCP_REXMIT_SUPPORT
                ptk = (uint8 *)wcp_reply_content_data(ps->wcph, (uint32 *)&r, 
                        rexmit);
#else /* WCP_REXMIT_SUPPORT */
                ptk = (uint8 *)wcp_reply_content_data(ps->wcph, (uint32 *)&r); 
#endif /* WCP_REXMIT_SUPPORT */
                HTTPD_TRACING();
                if (ptk) {
                    if (r > hbuf_len - hbuf_idx)
                        r = hbuf_len - hbuf_idx;

                    wcputil_memcpy(httpd_buf + hbuf_idx, ptk, r, ps->wcph);

                    hbuf_idx += r;
                    continue;
                    
                } else {
                    HTTPD_TRACING();
                    HTTPD_REXMIT_PRINTF(("@ SEND_CONTENT:(Done)break to send...,ps=%p\n", ps));
                    ps->state = HTTPD_STATE_SEND_DONE;
                    HTTPD_STATE_PRINTF(("@ HTTPD_STATE_SEND_CONTENT: state changed to %d!!\n",
                            (int)ps->state));
                    break; /* Done: content provided by WCP */
                }
            }
            break;
            
        case HTTPD_STATE_SEND_DONE:
            HTTPD_STATE_PRINTF(("@ HTTPD_STATE_SEND_DONE:ps=%p,state=%d, Closing..\n", 
                    ps,(int)ps->state));
            /* All previous sent data has been ACKed, we can close it now */
            HTTPD_TRACING();
#ifdef WCP_REXMIT_SUPPORT
            /* updated rexmit_info 
             *
             *  Note :
             *   - This state log is here for debug tracing. Proper rexmit is 
             *      expected won't reach this HTTPD state.
             *      (Since the connenction in this stae means TCP layer already been 
             *      notified the tcp session is closed. And TCP layer is expected 
             *      handle the remote ACK status for our close notification.
             */
            rexmit_info->state = HTTPD_STATE_SEND_DONE;
#endif /* WCP_REXMIT_SUPPORT */
            httpd_close_session(ps);
            return;

        default:
            HTTPD_TRACING();
            break;
        }

        if (hbuf_idx > 0) {        
            ps->ticks = sal_time();
            HTTPD_TRACING();
            HTTPD_STATE_PRINTF(("@ (hbuf_idx > 0):ps=%p,state=%d, Sending...\n",
                    ps, (int)ps->state));
            HTTPD_REXMIT_PRINTF(("@ Sending...( ps=%p, phbuf_idx=%d)\n",
                    ps, (int)hbuf_idx));
            r = socket_send(ps->socket, (uint8 *)httpd_buf, hbuf_idx, 0);

#ifdef WCP_REXMIT_SUPPORT 
        if (rexmit) { 
            HTTPD_REXMIT_PRINTF(("REXMIT: ps=%p, DONE!\n",ps));
        }
#endif /* WCP_REXMIT_SUPPORT */
            HTTPD_TRACING();
        }
    }
}

STATICFN int 
httpd_newsession(void)
{
#if HTTPD_DEBUG_LINKDROP
    int j;
#endif  /* HTTPD_DEBUG_LINKDROP */
    int i;
    HTTP_SESSION *ps;

    ps = &httpd_sessions[0];
    for (i = 0; i < HTTPD_SESS_CNT; i++, ps++){
        if ((ps->socket == -1) && (ps->state == HTTPD_STATE_CLOSED)) {
            /* assigning socket=0 to indicate this is a active TCP connect 
             *   -> uIP.TCP support no BSD like TCP socket.
             */
            ps->socket = 0; 
            HTTPD_TCPAPPSTATE_THIS = i;

            /* get mss of this connection */
            ps->mss = tcpapp_mss_update();

            ps->state = HTTPD_STATE_LISTENING;
            HTTPD_STATE_PRINTF(("@ httpd_newsession: state init to %d!!\n", 
                    (int)ps->state));
            ps->service = &http_services[HTTPD_SERV_WEB];

#if HTTPD_DEBUG_LINKDROP
            httpd_new_sess_cnt[i] ++;
            HTTPD_LINKDROP_PRINTF(("\nNew session created!ps=%p(id=%d)\n", ps, i));
            HTTPD_LINKDROP_PRINTF(("== Created session info[]={"));
            for (j = 0;j < HTTPD_SESS_CNT; j++){
                HTTPD_LINKDROP_PRINTF(("%d%s", httpd_new_sess_cnt[j], 
                        (j == (HTTPD_SESS_CNT - 1)) ? "}\n" : ","));
            }
#endif  /* HTTPD_DEBUG_LINKDROP */
            return i;
        }
    }
    HTTPD_TCPAPPSTATE_CLEAR;
    return -1;
}

#ifdef HTTPD_TIMER_SUPPORT
STATICCBK void
httpd_timer_check(void *param) REENTRANT
{
    HTTPD_TIMER_ENTRY *pt, *ppt;
    sal_time_t tm;

    UNREFERENCED_PARAMETER(param);
    ppt = NULL;
    pt = timer_list;
    for(; pt != NULL; ppt = pt, pt = pt->next) {
        
        /* Delete it if mark "deleted" */
        if (pt->callback == NULL) {
            if (ppt == NULL) {
                timer_list = pt->next;
            } else {
                ppt->next = pt->next;
            }
            
            /* Free it */
            KFREE(pt);
            
            if (ppt == NULL) {
                break;
            }
            
            pt = ppt;
            continue;
        } 
        
        tm = sal_time();
        if (pt->seconds == 0 || tm - pt->start_time >= pt->seconds) {
            (*pt->callback)(pt->user_data);
            pt->start_time = sal_time();
        }
    }
}

HTTPD_TIMER_HANDLE
httpd_create_timer(unsigned int s, void (*func)(void *), void *in_data)
{
    HTTPD_TIMER_ENTRY *pt;
    
    /* Error checking: callback function is required */
    if (func == NULL) {
        return NULL;
    }
    
    /* Allocate timer entry */
    pt = (HTTPD_TIMER_ENTRY *)KMALLOC(sizeof(HTTPD_TIMER_ENTRY), 4);
    pt->seconds = s;
    pt->callback = func;
    pt->user_data = in_data;
    pt->start_time = sal_time();
    
    /* Insert into list */
    if (timer_list == NULL) {
        timer_list = pt;
        pt->next = NULL;
    } else {
        pt->next = timer_list;
        timer_list = pt;
    }
    
    return (HTTPD_TIMER_HANDLE)pt;
}

void
httpd_delete_timer(HTTPD_TIMER_HANDLE handle)
{
    HTTPD_TIMER_ENTRY *pt = timer_list;
    
    for(; pt != NULL; pt = pt->next) {
        
        if ((HTTPD_TIMER_HANDLE)pt == handle) {
            
            /* Make it as deleted */
            pt->callback = NULL;
            return;
        }
    }
    
    /* Not found */
    return;
}

void
httpd_delete_timers_by_callback(void (*func)(void *))
{
    HTTPD_TIMER_ENTRY *pt = timer_list;
    
    for(; pt != NULL; pt = pt->next) {
        
        if (pt->callback == func) {
            
            /* Make it as deleted */
            pt->callback = NULL;
        }
    }
    
    return;
}
#endif /* HTTPD_TIMER_SUPPORT */

/* uIP.TCP adaptor to HTTPD engine */
void 
httpd_appcall(void)
{
    HTTP_SESSION *s;
    uint32 httpd_tcpcbk_flag = 0;

    HTTPD_STATE_PRINTF(("\nhttp_appcall: === START ===\n"));
    if (HTTPD_TCPAPPSTATE_IS_VALID) {
        s = &httpd_sessions[HTTPD_TCPAPPSTATE_THIS];
    } else {
        s = NULL;
    }    
    HTTPD_DBG(("http_appcall:uip_flags=%02x,tcpflags=%d,datalen=%d,ps=%p,uip_conn.appstate=%d\n", 
            (int)uip_flags, (int)(uip_conn->tcpstateflags), uip_datalen(),s, (int)HTTPD_TCPAPPSTATE_THIS));
    HTTPD_STATE_PRINTF(("http_appcall: uip_flags=%02x,tcpflags=%d,datalen=%d\n",                           
            (int)uip_flags, (int)(uip_conn->tcpstateflags), uip_datalen()));

    if (s == NULL){
        if (uip_connected()){
            int new_s;
            new_s = httpd_newsession();
            if (new_s == -1){
                uip_abort();
#ifndef __BOOTLOADER__
                HTTPD_LOG(("NO free HTTPd session!\n"));
#endif /* !__BOOTLOADER__ */
                return;
            }

            s = &httpd_sessions[new_s];
            httpd_tcpcbk_flag = TCPCBK_CONNECTED;
            HTTPD_DBG(("http_appcall:New Session established(new_s=%d,ps=%p)\n", new_s, s));
            if (uip_newdata()){
                httpd_tcpcbk_flag |= TCPCBK_NEWDATA;
            }
            httpd_process(s, httpd_tcpcbk_flag);
            HTTPD_STATE_PRINTF(("http_appcall: httpd_process(ps=%p,s.state=%d), new session, DONE!\n", 
                    s, (int)s->state));
        }
    } else {
        if (uip_closed() || uip_aborted()) {
            /* uip.TCP might reach this case for :
             *  - TCP_RST on remote peer -> uip_flags&UIP_ABORT
             *  - TCP_FIN on remote peer -> uip_flags&UIP_CLOSE
             *  - UIP_LAST_ACK : Remote peer ACK our FIN 
             *      -> uip_flags&UIP_CLOSE and tcpstateflags=UIP_CLOSED
             */
            if (uip_conn->tcpstateflags == UIP_CLOSED){
                httpd_tcpcbk_flag = TCPCBK_CLOSED;
                httpd_process(s, httpd_tcpcbk_flag);
                HTTPD_DBG(("http_appcall: httpd_process(ps=%p), close DONE!\n", s));
                HTTPD_STATE_PRINTF(("http_appcall: httpd_process(ps=%p,s.state=%d), close DONE!\n", 
                        s, s->state));
            }
            return;
        } else if (uip_timedout()){
            HTTPD_TRACING();
            HTTPD_STATE_PRINTF(("http_appcall: TCP timeout!(ps=%p,s.state=%d) timeout! Session closing...\n", 
                    s, (int)s->state));
            HTTPD_REXMIT_PRINTF(("http_appcall: TCP timeout!(ps=%p) timeout! Session closing...\n", s));
            /*  uIP.TCP timeout occurred due to remote peer has no proper ACK. 
             *   - Close this session to prevent to release this session.
             *
             *  p.s. uIP.TCP timeout due to serval REXMIT but no ACK already.
             */
            httpd_close_session(s);
            uip_abort();
            return;
        } else {
            /* might be 
             *  1. uip_acked() || uip_newdata() 
             *  2. uip_poll() || uip_rexmit()
             */
            httpd_tcpcbk_flag = 0;
            if (uip_poll()){
                httpd_tcpcbk_flag |= TCPCBK_POLL;
            } 
            if (uip_newdata()){
                httpd_tcpcbk_flag |= TCPCBK_NEWDATA;
            }
            if (uip_acked()){
                httpd_tcpcbk_flag |= TCPCBK_ACK;
            }
            
            if (uip_rexmit()){
                HTTPD_REXMIT_PRINTF(("http_appcall:TCP REXMIT requested,ps=%p, ps.state=%d!\n", 
                        s, (int)s->state));
                httpd_tcpcbk_flag = TCPCBK_REXMIT;
            }

            if (httpd_tcpcbk_flag == 0) {
#ifndef __BOOTLOADER__
                HTTPD_LOG(("httpd:Unexpect uip_flags=%02x!\n", (int)uip_flags));
#endif /* !__BOOTLOADER__ */
            }
            httpd_process(s, httpd_tcpcbk_flag);
            HTTPD_DBG(("http_appcall:httpd_process(ps=%p),others(uip_flags=%02x) DONE!\n", 
                    s, (int)uip_flags));
            HTTPD_STATE_PRINTF(("http_appcall:httpd_process(ps=%p,s.state=%d),others(uip_flags=%02x) DONE!\n", 
                    s,(int)s->state,(int)uip_flags));
        }
    }
    HTTPD_STATE_PRINTF(("http_appcall: === END ===\n"));
}

void
httpd_init(void)
{
    uint8 i;
    
    for(i=0; i < HTTPD_SESS_CNT; i++) {
        httpd_sessions[i].state = HTTPD_STATE_CLOSED;
        httpd_sessions[i].socket = -1;
        httpd_sessions[i].wcph = NULL;
        httpd_sessions[i].mss = HTTPD_DEFAULT_MTU_SIZE;
#ifdef  WCP_REXMIT_SUPPORT
        httpd_sessions[i].rexmit_info.include_header = 0;
        httpd_sessions[i].rexmit_info.state = HTTPD_STATE_CLOSED;
#endif  /* WCP_REXMIT_SUPPORT */
#if     HTTPD_DEBUG_LINKDROP
        httpd_new_sess_cnt[i] = 0;
        httpd_close_sess_cnt[i] = 0;
#endif  /* HTTPD_DEBUG_LINKDROP */
    }

    for(i=0; i < HTTPD_SERV_CNT; i++) {
        http_services[i].name[0] = 0;
        http_services[i].port = 0;
        http_services[i].reserved = (void *)-1;
    }
    
    for(i=0; i < HTTPD_SERV_CNT; i++) {
        if (http_serv_table[i].name[0] == 0 || http_serv_table[i].port == 0) {
            break;
        }
        sal_memcpy(&http_services[i], &http_serv_table[i], sizeof(HTTP_SERVICE));
    }
    if (i == 0) {
        return; /* No services */
    }

    /* to listen TCP port=80 */
    uip_listen(UIP_HTONS(HTTP_TCP_PORT));

    /* uIP.TCP appstate init */
    for(i=0; i < UIP_CONNS; i++) {
        uip_conns[i].appstate = HTTPD_TCPAPPSTATE_INIT;
    }

#ifdef HTTPD_TIMER_SUPPORT
    timer_add(httpd_timer_check, NULL, HTTPD_TIMER_INTERVAL);
#endif /* #ifdef HTTPD_TIMER_SUPPORT */

    HTTPD_DBG(("httpd started.\n"));
}

void
httpd_uninit(void)
{
    uint8 i;
    
    for(i=0; i<HTTPD_MAX_SESSIONS; i++) {
        if (httpd_sessions[i].state != HTTPD_STATE_CLOSED) {
            httpd_close_session(&httpd_sessions[i]);
        }
    }
    
    for(i=0; i<HTTPD_SERV_CNT; i++) {
        http_services[i].name[0] = 0;
        if (http_services[i].port != 0) {
            http_services[i].port = 0;
            if ((int)http_services[i].reserved != -1) {
                socket_close((int)http_services[i].reserved);
                http_services[i].reserved = (void *)-1;
            }
        }
    }
}

#if HTTPD_DIAG
void 
cmd_diaghttpd(int argc, char *argv[])
{
    int i;
    
    /* 
     * For better maintainence on scrambled code,
     * HTTPD_DIAG is now checked in C logic instead of preprocesor (#if)
     */
    if (HTTPD_DIAG) {
        sal_printf("  Last processing: %s\n", httpd_diag_buf);
        sal_printf("  Last looping tick: %lu\n", httpd_diag_last_tick);
        sal_printf("  Last executed line: %d\n", httpd_diag_last_executed);
        
        sal_printf("  Sessions:\n");
        for(i=0; i<HTTPD_MAX_SESSIONS; i++) {
            if (httpd_sessions[i].state != HTTPD_STATE_CLOSED) {
                sal_printf("   session[%d]: state=%d ticks=%lu socket=%u wcph=%p\n",
                    i, 
                    httpd_sessions[i].state, 
                    httpd_sessions[i].ticks, 
                    httpd_sessions[i].socket, 
                    httpd_sessions[i].wcph
                    );
            }
        }
    }
}
#endif

#endif  /* CFG_HTTPD_ENABLED */

