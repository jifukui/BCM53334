/*
 * $Id: httpd_arch.h,v 1.5 Broadcom SDK $
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

#ifndef _HTTPD_ARCH_H
#define _HTTPD_ARCH_H

#include "system.h"

#include "uip.h"
#include "uip_task.h"

#define HTTPD_DEBUG (0)
#define HTTPD_DIAG  (0)
#define HTTPD_TRACE (0)
#define HTTPD_WARN_LOG          (0)

#define HTTPD_DEBUG_CONTENT     (0)
#define HTTPD_DEBUG_SSP         (0)
#define HTTPD_DEBUG_STATE       (0)     /* for HTTPD_STATE */
#define HTTPD_DEBUG_REXMIT      (0)     /* for HTTPD_REXMIT*/
#define HTTPD_DEBUG_LINKDROP    (0)     /* for Link Drop event*/

/* project specific  configurations */
#define HTTPD_MAX_SESSIONS      (8)
#define HTTPD_MAX_SERVICES      (2)

/* This is used for httpd_buf[] for receive/transmit httpd buffer.
 *
 * Note :
 *  - Since HTTPD in uIP can serve only one session from  at a time,  
 *      this content in this buffer can never be guaranteed once next 
 *      httpd process(httpd_process()) is called.  We don't need to 
 *      prepare too large buffer size to improve performance.
 *  - The proper size define will be proper between TCP MSS(Max 
 *      Segement Size) and device RX buffer.
 */
#define HTTPD_SHARED_BUF_SIZE   (1600)

/* Common utilities */
#define fflush(x)

#define _ACT_PRINTF(x)  do { sal_printf x; sal_printf("\n"); fflush(stdout); } while(0)  

#define HTTPD_GETTICKS()  

#if HTTPD_WARN_LOG
#define HTTPD_LOG(x)    _ACT_PRINTF(x)
#else   /* HTTPD_WARN_LOG */
#define HTTPD_LOG(x)
#endif  /* HTTPD_WARN_LOG */

#if HTTPD_DEBUG || HTTPD_TRACE
#define HTTPD_DBG(x)    _ACT_PRINTF(x)
#else
#define HTTPD_DBG(x)
#endif /* HTTPD_DEBUG */

#if HTTPD_DEBUG_CONTENT && CFG_CONSOLE_ENABLED
#define HTTPD_CONTENT_PRINTF(x) _ACT_PRINTF(x)
#else
#define HTTPD_CONTENT_PRINTF(x)
#endif  /* HTTPD_DEBUG_CONTENT && CFG_CONSOLE_ENABLED */

#if HTTPD_DEBUG_SSP && CFG_CONSOLE_ENABLED
#define HTTPD_SSP_PRINTF(x) _ACT_PRINTF(x)
#else
#define HTTPD_SSP_PRINTF(x)
#endif  /* HTTPD_DEBUG_SSP  && CFG_CONSOLE_ENABLED */

#if HTTPD_DEBUG_STATE && CFG_CONSOLE_ENABLED
#define HTTPD_STATE_PRINTF(x) _ACT_PRINTF(x)
#else
#define HTTPD_STATE_PRINTF(x)
#endif  /* HTTPD_DEBUG_STATE  && CFG_CONSOLE_ENABLED */

#if HTTPD_DEBUG_REXMIT && CFG_CONSOLE_ENABLED
#define HTTPD_REXMIT_PRINTF(x)  _ACT_PRINTF(x)
#define WCP_REXMIT_PRINTF(x)    _ACT_PRINTF(x)
#else
#define HTTPD_REXMIT_PRINTF(x)
#define WCP_REXMIT_PRINTF(x)
#endif  /* HTTPD_DEBUG_REXMIT  && CFG_CONSOLE_ENABLED */

#if HTTPD_DEBUG_LINKDROP && CFG_CONSOLE_ENABLED
#define HTTPD_LINKDROP_PRINTF(x)  do { sal_printf x; fflush(stdout); } while(0)  
#else
#define HTTPD_LINKDROP_PRINTF(x)
#endif  /* HTTPD_DEBUG_LINKDROP  && CFG_CONSOLE_ENABLED */

/* Utilities */
#define strcmpi sal_stricmp
#define KMALLOC(x,y) sal_malloc(x)
#define KFREE(x) sal_free(x)
char *gettoken(char **ptr);

/* UM SAL timer */
typedef uint32          sal_time_t;
#define INTERVAL_TO_SEC(t)   ((t)/1000)   /* tick unit is 'ms' in UM */
#define sal_time()      INTERVAL_TO_SEC(sal_get_ticks())

/* Socket API */
#define socket_close(s)     uip_close()
#define socket_recv(s, buf, len, flags) uiptcp_recv(buf, len)
#define socket_send(s, msg, len, flags) uiptcp_send(msg, len)

#define tcpapp_mss_update()     \
    ((HTTPD_DEFAULT_MTU_SIZE < uip_mss()) ? HTTPD_DEFAULT_MTU_SIZE : uip_mss())


/* TCP callback flags */
#define TCPCBK_POLL         (1)
#define TCPCBK_CONNECTED    (2)
#define TCPCBK_CLOSED       (4)
#define TCPCBK_ACK          (8)
#define TCPCBK_NEWDATA      (16)
#define TCPCBK_SYN_RECVD    (32)
#define TCPCBK_REXMIT       (64)    /* -- TBD -- */

/* HTTPD session (for uip_conn.appstate) */
#define HTTPD_TCPAPPSTATE_INIT          (0xFF)
#define HTTPD_TCPAPPSTATE_MAX           (HTTPD_MAX_SESSIONS - 1)
#define HTTPD_TCPAPPSTATE_THIS          (uip_conn->appstate)
#define HTTPD_TCPAPPSTATE_CLEAR         \
        (HTTPD_TCPAPPSTATE_THIS = HTTPD_TCPAPPSTATE_INIT)
#define HTTPD_TCPAPPSTATE_IS_VALID    \
        ((HTTPD_TCPAPPSTATE_THIS != HTTPD_TCPAPPSTATE_INIT) && \
        (HTTPD_TCPAPPSTATE_THIS <= HTTPD_TCPAPPSTATE_MAX))

/* TCP addaptor functions */
int uiptcp_send(uint8 * buf, int32 len);
int32 uiptcp_recv(uint8 * buf, int32 len);

#endif /* _HTTPD_ARCH_H */
