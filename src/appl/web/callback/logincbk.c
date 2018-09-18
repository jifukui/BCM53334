/*
 * $Id: logincbk.c,v 1.6 Broadcom SDK $
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
#include "uip.h"
#include "appl/persistence.h"
#include "../content/errormsg_htm.h"

#ifdef CFG_SYSTEM_PASSWORD_INCLUDED
/*
 * For performace on checking, we save the current corresponding cookie value
 */
/* Max number of concurrent logins */
#define MAX_CONCURRENT_LOGIN    (1)

#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
/*
 * Login timeout support
 */
#ifndef NO_LOGIN_TIMEOUT

/* Login timeout in seconds (macro) */
//#define AUTH_LOGIN_TIMEOUT_SECS        (login_timeout)
#define AUTH_LOGIN_TIMEOUT_SECS CFG_ADMINP_LOGIN_TIMEOUT
/* Login timeout in seconds (setting 0 will disable timeout check) */
//static unsigned int login_timeout = (1 * 60 * 1000);
#endif /* NO_LOGIN_TIMEOUT */

/* Logined identities */
static unsigned int logined_entries[MAX_CONCURRENT_LOGIN];

/* Logined identities */
BOOL logined_entries_valid[MAX_CONCURRENT_LOGIN];
/* IP address of logined identities */
uint32 logined_IPs[MAX_CONCURRENT_LOGIN];

/* Last acces time of logined identities */
unsigned int logined_accessed[MAX_CONCURRENT_LOGIN];

/* Local IP for the logined entries */

#define MAX_COOKIE_LEN      (24)
static char cached_cookie_value[MAX_COOKIE_LEN + 1] = { 0 };

/* static BOOL cached_cookie_initialized = FALSE; */

/* Forwards */
/* static void make_cookie_cache(int user_id); */
static void
make_cookie_cache(void)
{

    /* Check if it has been initialized */
    if (cached_cookie_value[0] == 0) {
        int p = 0;
        int i, l;
        char lpwd[MAX_PASSWORD_LEN + 1];

        get_login_password(lpwd, sizeof(lpwd));
        
        
        l = sal_strlen(lpwd);
        for(i=0; i<l; i++) {
            p += (((int)lpwd[i]) << 8) + (lpwd[i] ^ 0xFF);
        }
        p %= 0xFFFF;
        p += 0x1234;
        p ^= 0xAA55;
        
        sprintf(cached_cookie_value, "%04X", p);
    }
}



int
webutil_check_cookie(const char *cookie, SSP_PSMH psmem)
{
    unsigned int tl = 0;
    BOOL single_user = FALSE;
    int user_num;

    make_cookie_cache();
    
    /*
     * Cookie format: 
     *   <8-digit password hash> <8-digit login time> <8-digit access time>
     */
    if (sal_strncmp(cookie, cached_cookie_value, 8)) {
        return -1;
    }
    
    if (sal_strlen(cookie + 8) != 16) {
        /* Incorrect length */
        return -1;
    }

    get_adminpv(&single_user);
    if (single_user == TRUE) {
        user_num = MAX_CONCURRENT_LOGIN;
    }
    
    if (user_num >= 1) {
    
        if (MAX_CONCURRENT_LOGIN > 0) {
            unsigned int i;
            char buf[9];
            const char *endptr;
            uint32 ip;

            /*
            * Parse the login entry value
            */
            sal_strncpy(buf, cookie + 8, 8);
            buf[8] = 0;
            tl = sal_strtoul(buf, &endptr, 16);
            if (endptr == NULL || *endptr != '\0') {
                /* contains non-hex characters */
                return -1;
            }
        
            /*
            * Check if it belongs to the ones that have logined
            */
            ip = (BUF->srcipaddr.u8[0] & 0xFF) | ((BUF->srcipaddr.u8[1]) << 8) | ((BUF->srcipaddr.u8[2]) << 16)
                | ((BUF->srcipaddr.u8[3]) << 24);
                
            for(i=0; i<MAX_CONCURRENT_LOGIN; i++) {
                if (logined_entries_valid[i] == TRUE && 
                    tl == logined_entries[i] && 
                    logined_IPs[i] == ip) {
                    
                    /* Update last acces time */
                    uint32 t;
                    t = sal_get_ticks();
                    logined_accessed[i] = t;
                    break;
                }
            }
            if (i == MAX_CONCURRENT_LOGIN) {
                /* No matching login found */
                return -1;
            }
        
#ifndef NO_LOGIN_TIMEOUT
            /*
            * Store the login entry value for set-cookie later
            * if login timeout is enabled.
            */
            if (AUTH_LOGIN_TIMEOUT_SECS != 0) {
                unsigned int *ptl = ssputil_psmem_get(psmem, webutil_check_cookie);
                if (ptl == NULL) {
                    ptl = (unsigned int *)ssputil_psmem_alloc(
                                        psmem, 
                                        webutil_check_cookie, 
                                        sizeof(unsigned int)
                                        );
                    if (ptl != NULL) {
                        *ptl = tl;
                    }
                }
            }
#endif /* NO_LOGIN_TIMEOUT */
        }
    }
    
#ifndef NO_LOGIN_TIMEOUT
    /* Check time elapsed since last access */
    if (AUTH_LOGIN_TIMEOUT_SECS != 0) {
        const char *s = cookie + 16;
        const char *endptr;
        uint32 t, cur;

        t = sal_strtoul(s, &endptr, 16);
        if (endptr == NULL || *endptr != '\0') {
            /* contains non-hex characters */
            return -1;
        }
        cur = sal_get_ticks();
        if ((cur - t) > AUTH_LOGIN_TIMEOUT_SECS) {
            uint32 ip;
            ip = (BUF->srcipaddr.u8[0] & 0xFF) | ((BUF->srcipaddr.u8[1]) << 8) | ((BUF->srcipaddr.u8[2]) << 16)
                | ((BUF->srcipaddr.u8[3]) << 24);


            /* 
             * Expired 
             */
            sal_printf("\n User session from %u.%u.%u.%u has expired.",
                (logined_IPs[0] & 0xFF),
                ((logined_IPs[0]  >> 8) & 0xFF),
                ((logined_IPs[0]  >> 16) & 0xFF),
                ((logined_IPs[0]  >> 24) & 0xFF)
                );
            
#ifndef NO_LOGIN_LIMITATION
            /* Remove this user from the logined list */
            if (MAX_CONCURRENT_LOGIN > 0) {
                int i;
                for(i=0; i<MAX_CONCURRENT_LOGIN; i++) {
                    if (logined_entries_valid[i] == TRUE) {
                        if (logined_entries[i] == tl && 
                            logined_IPs[i] == ip) {
                            logined_entries_valid[i] = FALSE;
                            break;
                        }
                    }
                }
            }
#endif /* NO_LOGIN_LIMITATION */
            
            return -1;
        }
    }
#endif /* NO_LOGIN_TIMEOUT */

    return 0;
}


static const char *
create_cookie(BOOL newuser, SSP_PSMH psmem)
{
    uint32 ta, tl;
    
    make_cookie_cache();
    ta = sal_get_ticks();
    tl = ta;
    
    if (newuser && (MAX_CONCURRENT_LOGIN > 0)) {
        int i, idx = -1;
        unsigned int *ptl;
        uint32 cur;
        uint32 ip;
        int match = 0;

        ip = (BUF->srcipaddr.u8[0] & 0xFF) | ((BUF->srcipaddr.u8[1]) << 8) 
           | ((BUF->srcipaddr.u8[2]) << 16) | ((BUF->srcipaddr.u8[3]) << 24);
            /* Check if local IP has been changed */
        if (logined_IPs[0] == 0) {
            
            logined_IPs[0] = (BUF->srcipaddr.u8[0] & 0xFF) | 
                            ((BUF->srcipaddr.u8[1]) << 8) | 
                            ((BUF->srcipaddr.u8[2]) << 16)| 
                            ((BUF->srcipaddr.u8[3]) << 24);

        } else {
            if (((logined_IPs[0] & 0xFF) == (BUF->srcipaddr.u8[0] & 0xFF)) &&
                (((logined_IPs[0] >> 8) & 0xFF) == BUF->srcipaddr.u8[1]) &&
                (((logined_IPs[0] >> 16) & 0xFF) == BUF->srcipaddr.u8[2]) &&
                (((logined_IPs[0] >> 24) & 0xFF) == BUF->srcipaddr.u8[3]))
            {
                match = 0;
            } else {
                match = 1;
            }
                
            if (!match) {
/*            
                sal_printf("\nIP the same as last login %u.%u.%u.%u",
                        (logined_IPs[0] & 0xFF),
                        ((logined_IPs[0]  >> 8) & 0xFF),
                        ((logined_IPs[0]  >> 16) & 0xFF),
                        ((logined_IPs[0]  >> 24) & 0xFF));  
*/                
                /* The IP is the same as last login */
                for(i=0; i<MAX_CONCURRENT_LOGIN; i++) {
                    logined_entries_valid[i] = FALSE;
                }
            } else {
/*            
                sal_printf("\nIP is different as last login %u.%u.%u.%u",
                        (logined_IPs[0] & 0xFF),
                        ((logined_IPs[0]  >> 8) & 0xFF),
                        ((logined_IPs[0]  >> 16) & 0xFF),
                        ((logined_IPs[0]  >> 24) & 0xFF));  
*/                
            }
        }

        /* Check if there is any empty slot */
        for(i=0; i<MAX_CONCURRENT_LOGIN; i++) {
            if (logined_entries_valid[i] == FALSE) {
                idx = i;
                break;
            } 
#ifndef NO_LOGIN_TIMEOUT
            else if (AUTH_LOGIN_TIMEOUT_SECS != 0) {
                cur = sal_get_ticks();
                if ((cur - logined_accessed[i]) > AUTH_LOGIN_TIMEOUT_SECS) {
                    
                    /* One user has expired, replace him */
                    logined_IPs[i] = (BUF->srcipaddr.u8[0] & 0xFF) | 
                                    ((BUF->srcipaddr.u8[1]) << 8) | 
                                    ((BUF->srcipaddr.u8[2]) << 16)| 
                                    ((BUF->srcipaddr.u8[3]) << 24);
                        
                    logined_entries_valid[i] = FALSE;
                    idx = i;
                    ta = cur;
                    sal_printf(
                        "\n User session from %u.%u.%u.%u has expired.",
                        (logined_IPs[i] & 0xFF),
                        ((logined_IPs[i]  >> 8) & 0xFF),
                        ((logined_IPs[i]  >> 16) & 0xFF),
                        ((logined_IPs[i]  >> 24) & 0xFF));  
                    break;
                }
            }
#endif /* NO_LOGIN_TIMEOUT */
        }
        if (idx == -1) {
            /* Maximal concurrent login reached! */
            return NULL;
        }
        
        /* Try to make it different than anyone else */
        for(;;) {
            for(i=0; i<MAX_CONCURRENT_LOGIN; i++) {
                if (logined_entries_valid[i] != FALSE) {
                    if (logined_entries[i] == tl) {
                        break;
                    }
                }
            }
            if (i == MAX_CONCURRENT_LOGIN) {
                break;
            }
            tl++;
        }
        
        /* Get the IP address of the connection */
        ip = (BUF->srcipaddr.u8[0] & 0xFF) | ((BUF->srcipaddr.u8[1]) << 8) 
          | ((BUF->srcipaddr.u8[2]) << 16) | ((BUF->srcipaddr.u8[3]) << 24);
        
        /* Save the new login entry */
        logined_entries[idx] = tl;
        logined_IPs[idx] = ip;
        logined_accessed[idx] = ta;
        logined_entries_valid[idx] = TRUE;
        /* Store the login entry value in psmem for use later */
        ptl = ssputil_psmem_get(psmem, webutil_check_cookie);
        if (ptl == NULL) {
            ptl = (unsigned int *)ssputil_psmem_alloc(
                                psmem, 
                                webutil_check_cookie, 
                                sizeof(unsigned int)
                                );
            if (ptl != NULL) {
                *ptl = tl;
            }
        }
        
    } else if (ssputil_psmem_get(psmem, webutil_check_cookie) != NULL) {
        tl = *(unsigned int *)ssputil_psmem_get(psmem, webutil_check_cookie);
    }

    sprintf(cached_cookie_value + 8, "%08X%08X", tl, ta);

    return cached_cookie_value;
}


SSP_HANDLER_RETVAL
ssphandler_logout_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    int i;

    for(i=0; i < MAX_CONCURRENT_LOGIN; i++) {
        logined_entries_valid[i] = FALSE;
        logined_IPs[i] = 0;
        logined_accessed[i] = 0;
    }

    return SSP_HANDLER_RET_INTACT;

}
SSP_HANDLER_RETVAL
ssphandler_login_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    char lpwd[MAX_USERNAME_LEN + 1];
    BOOL single_user = FALSE;
    uint32 ip;
    int  user_id = 0, user_num = 0;

    get_adminpv(&single_user);
    if (single_user == TRUE) {
        user_num = MAX_CONCURRENT_LOGIN;
    } 

    if (cxt->type == SSP_HANDLER_REQ_COOKIE) {
        char *p;
        
        p = ssputil_psmem_alloc(psmem, ssphandler_login_cgi, MAX_COOKIE_LEN+1);
        if (p != NULL) {
            SSP_HANDLER_CONTEXT_EXT *cxte = (SSP_HANDLER_CONTEXT_EXT *)cxt;
            sal_strncpy(p, cxte->url_data.string, MAX_COOKIE_LEN);
            p[MAX_COOKIE_LEN] = 0;
        }
        return SSP_HANDLER_RET_INTACT;
    }
    
    if (cxt->type == SSP_HANDLER_SET_COOKIE) {
        SSP_HANDLER_CONTEXT_EXT *cxte = (SSP_HANDLER_CONTEXT_EXT *)cxt;
        cxte->url_data.string = create_cookie(FALSE, psmem);
        cxte->flags &= ~SSPF_SET_COOKIE_H;
        
        return SSP_HANDLER_RET_MODIFIED;
    }
    
    if (cxt->type != SSP_HANDLER_QUERY_STRINGS) {
        return SSP_HANDLER_RET_INTACT;
    }

    if (cxt->count != 1) {
        return SSP_HANDLER_RET_INTACT;
    }

#if CFG_RAMAPP
    /* RAM based */
    sal_strcpy(lpwd, "1234");
#else
    get_login_password(lpwd, sizeof(lpwd));
#endif /* CFG_RAMAPP */


    if (sal_strcmp(cxt->pairs[0].value, lpwd)) {
          webutil_show_error(
            cxt, psmem,
            "Input Password",
            "Incorrect Password!",
            err_button_retry,
            err_action_back
            );
        return SSP_HANDLER_RET_MODIFIED;
    }

    /* Login succeed */
    ip = (BUF->srcipaddr.u8[0] & 0xFF) | ((BUF->srcipaddr.u8[1]) << 8) | ((BUF->srcipaddr.u8[2]) << 16)
         | ((BUF->srcipaddr.u8[3]) << 24);
/*         
    sal_printf("\nLogin in Success from %u.%u.%u.%u",
                        (ip & 0xFF),
                        ((ip  >> 8) & 0xFF),
                        ((ip  >> 16) & 0xFF),
                        ((ip  >> 24) & 0xFF)
                        );
*/                        
        /* Check if max login reached by calling create_cookie() */
    while (user_num > 0) {
        const char *cookie;
        char *p;
                
        p = ssputil_psmem_get(psmem, ssphandler_login_cgi);
        if (p != NULL) {
            if (webutil_check_cookie(p, psmem) == 0) {
            /* 
             * User has already logined, skip cookie update
             */
                cxt->flags &= ~SSPF_SET_COOKIE_H;
                break;
            }
        }
        cookie = create_cookie(TRUE, psmem);
        if (cookie == NULL) {
            /* Max number of login reached */
                
            sal_printf("\nLogin from %u.%u.%u.%u is rejected due to single user occupied",
                    (ip & 0xFF),
                    ((ip  >> 8) & 0xFF),
                    ((ip  >> 16) & 0xFF),
                    ((ip  >> 24) & 0xFF)
                    );
            /* So it won't set cookie */
            cxt->flags &= ~SSPF_SET_COOKIE_H;
            return SSP_HANDLER_RET_MODIFIED;
        }
                
        break;
    }

    /* Forced cached cookie to be re-calculated */
    cached_cookie_value[user_id] = 0;
    /* make_cookie_cache(user_id); */

    /* Save it to persistent medium */

    cxt->page = ssputil_locate_file(psmem, "/index.htm", NULL);
    cxt->flags &= ~SSPF_FORCE_CACHE;
    cxt->flags |= SSPF_NO_CACHE;
    cxt->flags &= ~SSPF_SET_COOKIE_H;
    return SSP_HANDLER_RET_MODIFIED;
}

#endif  /* CFG_SYSTEM_PASSWORD_INCLUDED */

