/*
 * $Id: httpd_wcp.h,v 1.3 Broadcom SDK $
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

/*
 * Interfaces of WCP - Web Content Provider for HTTPd
 */

#ifndef _HTTPD_WCP_H_
#define _HTTPD_WCP_H_

#include "appl/httpd.h"

#ifdef HTTPD_REXMIT_SUPPORT
#define WCP_REXMIT_SUPPORT
#endif  /* HTTPD_REXMIT_SUPPORT */

typedef void *WCP_HANDLE; 

/*  *********************************************************************
    *  wcp_open(post,path,filename,params)
    *  
    *  Open a web page from the WCP.
    *  
    *  Input parameters: 
    *       post - 1 if it's a POST request. 0 if GET.
    *       path - path and filename of the URL
    *       params - parameters after URL (normally for GET request)
    *       sock - socket for this connection
    *       service - HTTP service it's requesting
    *      
    *  Return value:
    *       Handle to this WCP session.
    ********************************************************************* */
WCP_HANDLE wcp_open(uint8, const char *, const char *, int, HTTP_SERVICE *);

void wcp_req_header(WCP_HANDLE wh, const char *name, const char *value);

void wcp_req_content_length(WCP_HANDLE wh, uint32 len);

void wcp_req_post_data(WCP_HANDLE wh, const uint8 *buf, uint32 len);

/* returns: status code string */
const char *wcp_req_end(WCP_HANDLE wh, int32 *ctlen);

void wcp_close(WCP_HANDLE wh);

void *wcputil_memcpy(void *dest, const void *src, size_t n, WCP_HANDLE wh);

#ifdef WCP_REXMIT_SUPPORT

void wcp_rexmit_clear(WCP_HANDLE wh);
const char *wcp_reply_headers(WCP_HANDLE wh, int rexmit);
const uint8 *wcp_reply_content_data(WCP_HANDLE wh, uint32 *len, int rexmit);
#else  /* WCP_REXMIT_SUPPORT */

const char *wcp_reply_headers(WCP_HANDLE wh);
const uint8 *wcp_reply_content_data(WCP_HANDLE wh, uint32 *len);
#endif  /* WCP_REXMIT_SUPPORT */

#endif /* _HTTPD_WCP_H_ */
