/*
 * $Id: xccvt_converters.c,v 1.11 Broadcom SDK $
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

#include <system.h>
#if defined(CFG_XCOMMAND_INCLUDED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED)


#include "xccvt_converters.h"

/*
 * Port list converters
 */
const XCMD_TYPE_CONVERTER xccvt_port_list_converters = {
    (XCCONV_FROM_STRING) xc_uplist_from_string,
    (XCCONV_TO_STRING) xc_uplist_to_string,
    NULL,
    xccvt_common_free_string,
};

/*
 * Function:
 *      xc_uplist_from_string
 * Purpose:
 *      Convert logical port bitmap array into string, ASCII port number start from 1. Logical port bitmap starts from bit0.
 * Parameters:
 *      string : port list , etc 1, 3, 5-9
 *         len : length of string 
 *      prop: for XCMD_CORE
 *      puplist: user port bitmap list, each bit represents a port 
 * Returns:
 *      XCMD_ERR_xxx
 */

XCMD_ERROR 
xc_uplist_from_string(const char *string,unsigned int len, void *prop, uplist_t *puplist)
{
    unsigned int i;
    const char *endptr;
    int prev;

    uplist_t uplist;
        
    if (prop == NULL || string == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    sal_memset(&uplist, 0, sizeof(uplist));
    
    /* 
         *   Parse string to local upbmp_t 
         */
    prev = -1;

    for(i=0; i<len; i++) {
        
        unsigned int uport;
        
        /* skip spaces */
        while (i < len && string[i] == ' ') {
            i++;
        }
        if (i >= len) {
            break;
        }

        /* Get the port number */
        uport = SAL_NZUPORT_TO_UPORT(sal_strtoul(&string[i], &endptr, 10));

        if (endptr == &string[i]) {
            return XCMD_ERR_INVALID_COMMAND_INPUT;
        }

        
        if (SAL_UPORT_IS_NOT_VALID(uport)) {
            return XCMD_ERR_INVALID_COMMAND_INPUT;
        }

        /* Advance pointer to the first non-number one */
        i += (endptr - &string[i]);
        if (i > len) {
            return XCMD_ERR_INTERNAL_INVALID_ENGINE_ROUTE;
        }
        
        /* skip spaces */
        while (i < len && string[i] == ' ') {
            i++;
        }
        
        if (i == len || string[i] == ',') {

            if (prev != -1) {
                
                /* In continuos mode */
                int p;
                if ((int)uport < prev) {
                    return XCMD_ERR_INVALID_COMMAND_INPUT;
                }
                
                for(p = prev; p<=(int)uport; p++) {
                    uplist_port_add(uplist.bytes, p);
                }
                
                prev = -1;

            } else {
                
                /* Single port */
                uplist_port_add(uplist.bytes, uport);
            }
            
            if (i == len) {
                break;
            }
            
        } else if (string[i] == '-') {
            
            if (prev != -1) {
                /* already in continuous mode */
                return XCMD_ERR_INVALID_COMMAND_INPUT;
            }
            
            /* start continuous mode */
            prev = (int)uport;
            
        } else {
            return XCMD_ERR_INVALID_COMMAND_INPUT;
        }
    }
    
    if (prev != -1) {
        /* End in continuous mode */
        return XCMD_ERR_INVALID_COMMAND_INPUT;
    }

    if (puplist == NULL) {
         /* Means to do validation only */
        return XCMD_ERR_OK;
    }

    /* Copy data for user */
    sal_memcpy((void *)puplist, &uplist, sizeof(uplist));
    
    return XCMD_ERR_OK;
}

/*
 * Function:
 *      xc_uplist_to_string
 * Purpose:
 *      Convert port list string to logical port bitmap array, ASCII port number start from 1. Logical port bitmap starts from bit0.
 * Parameters:
 *      pstring : port list , etc 1, 3, 5-9
 *      prop: 
 *      puplist: user port bitmap list, each bit represents a port 
 * Returns:
 *      XCMD_ERR_xxx
 */

XCMD_ERROR 
xc_uplist_to_string(const uplist_t *puplist, void *prop,char **pstring)
{
    uint16 uport, prev;
    char *p;
    int cont;
    
    if (puplist == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    if (pstring == NULL) {
        return XCMD_ERR_OK;
    }

    *pstring = xcmd_alloc(64, "xc_uplist_to_string");

    if (*pstring == NULL) {
        return XCMD_ERR_OUT_OF_RESOURCE;
    }

    /* Convert to port list string */
    p = *pstring;
    cont = 0;
    prev = -1;
    SAL_UPORT_ITER(uport) {
        if (uplist_port_matched(puplist->bytes, uport) != SYS_OK) {
            continue;
        }
        if (prev == -1) {
            p += sal_sprintf(p, "%d", SAL_NZUPORT_TO_UPORT(uport));
        } else {
            if (uport == prev + 1) {
                if (!cont) {
                    p += sal_sprintf(p, "-");
                    cont = 1;
                }
            } else {
                if (cont) {
                    p += sal_sprintf(p, "%d", prev);
                    cont = 0;
                }
                p += sal_sprintf(p, ",%d", SAL_NZUPORT_TO_UPORT(uport));
            }
        }
        prev  = uport;
    }
    if (cont) {
        p += sprintf(p, "%d", SAL_NZUPORT_TO_UPORT(prev));
        cont = 0;
    }
    *p = 0;

    return XCMD_ERR_OK;
}

/*
 * Function:
 *      xc_pbmp_from_string
 * Purpose:
 *      Convert logical port bitmap array from string, ASCII port number start from 1. Logical port bitmap starts from bit0.
 * Parameters:
 *      string : port list , etc 1, 3, 5-9 is null terminated string
 *      prop: for XCMD_CORE
 *      puplist: user port bitmap list, each bit represents a port 
 * Returns:
 *      sys_error_t
 */

XCMD_ERROR xc_pbmp_from_string(const char *string, int len, void *prop, pbmp_t *pbmp) {


   int prev, i;
   pbmp_t local_pbmp;
   const char *endptr;

  
   BCM_PBMP_CLEAR(local_pbmp);

   endptr = &string[len];
   
   /* 
       *   Parse string to local upbmp_t 
       */
   prev = -1;

   for(i=0; i<len; i++) {
    
    unsigned int port;
    
    /* skip spaces */
    while (i < len && string[i] == ' ') {
         i++;
    }
    if (i >= len) {
        break;
    }

    /* Get the port number */
    port = sal_strtoul(&string[i], &endptr, 10);

    if (endptr == &string[i]) {
        return SYS_ERR_PARAMETER;
    }

    /* Advance pointer to the first non-number one */
    i += (endptr - &string[i]);
    if (i > len) {
        return XCMD_ERR_INTERNAL_INVALID_ENGINE_ROUTE;
    }
    
    /* skip spaces */
    while (i < len && string[i] == ' ') {
        i++;
    }
    
    if (i == len || string[i] == ',') {

        if (prev != -1) {
            
            /* In continuos mode */
            int p;
            if ((int)port < prev) {
                return XCMD_ERR_INVALID_COMMAND_INPUT;
            }
            
            for(p = prev; p<=(int)port; p++) {
                if (p < _SHR_PBMP_PORT_MAX) {
                    BCM_PBMP_PORT_ADD(local_pbmp, p);
                }
            }
            
            prev = -1;

        } else {
            
            if (port < _SHR_PBMP_PORT_MAX) {
                BCM_PBMP_PORT_ADD(local_pbmp, port);
            }

        }
        
        if (i == len) {
            break;
        }
        
    } else if (string[i] == '-') {
        
        if (prev != -1) {
            /* already in continuous mode */
            return XCMD_ERR_INVALID_COMMAND_INPUT;
        }
        
        /* start continuous mode */
        prev = (int)port;
        
    } else {
        return XCMD_ERR_INVALID_COMMAND_INPUT;
    }
}

if (prev != -1) {
    /* End in continuous mode */
    return XCMD_ERR_INVALID_COMMAND_INPUT;
}

   /* Copy data for user */
   sal_memcpy((void *)pbmp, &local_pbmp, sizeof(pbmp));

   return XCMD_ERR_OK;
}

/*
 * Function:
 *      pbmp_to_string
 * Purpose:
 *      Convert logical port bitmap array from string, ASCII port number start from 1. Logical port bitmap starts from bit0.
 * Parameters:
 *      string : port list , etc 1, 3, 5-9 is null terminated string
 *      prop: for XCMD_CORE
 *      puplist: user port bitmap list, each bit represents a port 
 * Returns:
 *      sys_error_t
 */

XCMD_ERROR xc_pbmp_to_string(const pbmp_t pbmp, void *prop, char **pstring) {


   int prev;

   char local_string[256];

   int port;
   char *p = local_string;
   int cont = 0;
    
   /* 
       *   Parse string to local upbmp_t 
       */
   prev = -1;

   BCM_PBMP_ITER(pbmp, port) {
    if (prev == -1) {
        p += sal_sprintf(p, "%d", port);
    } else {
        if (port == prev + 1) {
            if (!cont) {
                p += sal_sprintf(p, "-");
                cont = 1;
            }
        } else {
            if (cont) {
                p += sal_sprintf(p, "%d", prev);
                cont = 0;
            }
            p += sal_sprintf(p, ",%d", port);
        }
    }
    prev  = port;

   }
   *p = 0;
   return XCMD_ERR_OK;
}



const XCMD_TYPE_CONVERTER xccvt_pbmp_converters = {
    (XCCONV_FROM_STRING) xc_pbmp_from_string,
    (XCCONV_TO_STRING) xc_pbmp_to_string,
    NULL,
    NULL,
};

#endif

