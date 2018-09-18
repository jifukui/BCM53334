/*
 * $Id: httpd_arch.c,v 1.5 Broadcom SDK $
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
#include "httpd_arch.h"

char *
gettoken(char **ptr)
{
    char *p = *ptr;
    char *ret;

    /* skip white space */

    while(*p && isspace(*p))
        p++;
    ret = p;

    /* check for end of string */

    if (!*p) {
        *ptr = p;
        return NULL;
    }

    /* skip non-whitespace */

    while(*p) {
        if (isspace(*p))
            break;

        /* do quoted strings */

        if (*p == '"') {
            p++;
            ret = p;
            while(*p && (*p != '"'))
                p++;
            if (*p == '"')
                *p = '\0';
        }

        p++;

    }

    if (*p) {
        *p++ = '\0';
    }
    *ptr = p;

    return ret;
}


/* uIP.TCP adaptor for HTTPd */
int  
uiptcp_send(uint8 * buf, int32 len)
{
    /* To send httpd data through uIP, the length must not exceeed the 
     *  defined max segement size in uIP. This size is the max length that 
     *  we can send through uip_send and this value will be different 
     *  while device is or isn't in LAN area. 
     *
     *  Normally, the ps->mss is expected be no larger than uip_mss().
     *    Once the len > uip_mss condition occurred, the reason might be the 
     *  limitation of uIP native arch. An complex network environment might 
     *  causes the uip_mss been updated due to different network connection 
     *  been established(TCP/UDP, IPv4/IPv6...).
     *
     *    Here we need to ensure the sending data is under the most current 
     *  uip_mss() to prevent the unexpect behavior. 
     */
    if (len > uip_mss()){
#ifndef __BOOTLOADER__
        HTTPD_LOG(("uiptcp_send: failed due to len=%d exceeds mss=%d\n", (int)len, uip_mss()));
#endif /* __BOOTLOADER__ */
        len = uip_mss();
    }
    HTTPD_CONTENT_PRINTF(("uiptcp_send:len=%d:\n%s\n",(int)len,buf));
    uip_send(buf, (int)len);
    return 0;
}

int32
uiptcp_recv(uint8 * buf, int32 len)
{
    int ret_len;

    ret_len = uip_datalen();
    if (len > UIP_BUFSIZE) {
        len = UIP_BUFSIZE;
    }

    if (len < ret_len){
        ret_len = len;
    }

    sal_memcpy(buf, uip_appdata, ret_len);
    HTTPD_CONTENT_PRINTF(("uiptcp_recv:\n%s\n",buf));
    return (int32)ret_len;
}


