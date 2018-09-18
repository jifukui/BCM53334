/*
 * $Id: sal_config.c,v 1.8 Broadcom SDK $
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
#include "../../../src/appl/xcommands/callback/xccvt_converters.h"
#include "utils/nvram.h"

#define SAL_CONFIG_STR_MAX              128     /* Max len of "NAME=VALUE\0" */

/**
 * sal_config_init: init config 
 * 
 *  @return initialization result
 *               SYS_OK, SYS_ERR
 *    
 *   
 */

sys_error_t sal_config_init(void) {
       return SYS_OK;
}

void sal_config_show(void) {
      return;
}
/**
 * sal_config_get: get ascii value associated with name
 * 
 * @param name (IN)- name of config item
 * @return  address of 
 *     NULL : config get fail
 *     else config get success
 *   
 */

const char *sal_config_get(const char *name) {
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED    
      return nvram_get(name);
#else
      return NULL;
#endif
}
/**
 * sal_config_uplist_get: get uplist associated with name
 * 
 * @param name (IN)- name of config item
 * @param p (IN)- address of ouput uplist 
 * @return 
 *     SYS_ERR_FALSE : config get fail
 *     SYS_OK: config get success
 *   
 */


sys_error_t sal_config_uplist_get(const char *name, uint8 *uplist) {

    const char *value;
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
    uplist_t tmp_uplist;
#endif
    value = sal_config_get(name);

    if (value == NULL) {
        return SYS_ERR_FALSE;
    } 
    if (*value == 0) {
        return SYS_ERR_FALSE;
    }
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
    if (xc_uplist_from_string(value, sal_strlen(value), NULL, (uplist_t *) &tmp_uplist) != XCMD_ERR_OK) {
        return SYS_ERR_FALSE;
    }
    uplist_manipulate(uplist, tmp_uplist.bytes, UPLIST_OP_COPY);

#else
    return SYS_ERR_FALSE;
#endif

    return SYS_OK;
}

/**
 * sal_config_pbmp_get: get pbmp_t associated with name
 * 
 * @param name (IN)- name of config item
 * @param p (IN)- address of ouput port bit map 
 * @return 
 *     SYS_ERR_FALSE : config get fail
 *     SYS_OK: config get success
 *   
 */

sys_error_t sal_config_pbmp_get(const char *name, pbmp_t *p) {

    const char *value;
    pbmp_t tmp_pbmp;
    value = sal_config_get(name);

    if (value == NULL) {
        return SYS_ERR_FALSE;
    } 
    if (*value == 0) {
        return SYS_ERR_FALSE;
    }
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
    if (xc_pbmp_from_string(value, sal_strlen(value), NULL, &tmp_pbmp) != XCMD_ERR_OK) {     
        return SYS_ERR_FALSE;
    };   
#else
        return SYS_ERR_FALSE;
#endif
    BCM_PBMP_ASSIGN(*p, tmp_pbmp);

    return SYS_OK;
}

sys_error_t sal_config_decimal_valid(const char *value, const char *max_value) {

    int i;

    if (value == NULL) return SYS_ERR_FALSE;

    if (*value == 0) return SYS_ERR_FALSE;
    

    /* check if it is decimal character */
    for (i = 0; i < sal_strlen(value); i++) {
         if (value[i] > '9' || value[i] < '0') {
             return SYS_ERR_FALSE;   
         }            
    }
    if (max_value) {
        
         if (sal_strlen(value) > sal_strlen(max_value)) {
             return SYS_ERR_FALSE;
         } 

         if (sal_strlen(value) != sal_strlen(max_value)) {
             return SYS_OK;
         } 

         if (sal_strcmp(value, max_value) > 0) {
             return SYS_ERR_FALSE;
         }    
    }    

    return SYS_OK;
}

sys_error_t sal_config_hex_valid(const char *value, int bit_count) {

    int max_chars = bit_count / 4;
    int i;
    
    if (value == NULL) return SYS_ERR_FALSE;

    if (*value == 0) return SYS_ERR_FALSE;
    
    if (sal_strlen(value) > max_chars) {
        return SYS_ERR_FALSE;
    }

    for (i=0 ; i< sal_strlen(value); i++) {
         if ((value[i] <= 'f' && value[i] >= 'a') || 
             (value[i] <= 'F' && value[i] >= 'A') || 
             (value[i] <= '9' && value[i] >= '0') ) 
         {
               continue;
         }
         return SYS_ERR_FALSE;
    }

    return SYS_OK;

}
sys_error_t sal_config_variable_get(const char*name, void *variable, int bit_count) {

    const char *decimal_max_list[] = { "4294967295", "65535", "255" };
    const char *decimal_max = NULL;
    uint8 base = 10;
    const char *value;
    uint32 result;

    value = sal_config_get(name);  

    if (value == NULL) {
        return SYS_ERR_FALSE;
    } 
    
    if (sal_strncmp("0X", value, 2) == 0 ||
        sal_strncmp("0x", value, 2) == 0 ) 
    {
         base = 16;
         value += 2;
        if (sal_config_hex_valid(value, bit_count) != SYS_OK) {
            return SYS_ERR_FALSE;
        }         
    } else {

        if (bit_count == 32) {
            decimal_max = decimal_max_list[0];
        } else if (bit_count == 16) {
            decimal_max = decimal_max_list[1];
        } else if (bit_count == 8) {
            decimal_max = decimal_max_list[2];
        }

        /* if the decimal is out of range */
        if (sal_config_decimal_valid(value, decimal_max) != SYS_OK) {
            return SYS_ERR_FALSE;
        }

    }

    result = sal_strtoul(value, NULL, base);

    if (bit_count == 32) {
        *((uint32 *) variable) = result;
    } else if (bit_count == 16) {
        *((uint16 *) variable) = (uint16) result;
    } else if (bit_count == 8) {
        *((uint8 *) variable) = (uint8) result;
    } else {
        return SYS_ERR_FALSE;
    }

    return SYS_OK;
}



/**
 * sal_config_uint32_get: get a uint32 variable associated with name
 * 
 * @param name (IN)- name of config item
 * @param word (IN) - output word buffer address
 * @param len (IN) - output word buffer length
 * @return word count of output result
 *          0 : means no output 
 *    
 *   
 */

sys_error_t sal_config_uint32_get(const char*name, uint32* word) {


    return sal_config_variable_get(name, word, 32);

}

/**
 * sal_config_uint16_get: get a uint16 variable associated with name
 * 
 * @param name (IN)- name of config item
 * @param hword (IN) - output hword address
 * @param len (IN) - output word buffer length
 * @return word count of output result
 *          0 : means no output 
 *    
 *   
 */

sys_error_t sal_config_uint16_get(const char*name, uint16* hword) {


    return sal_config_variable_get(name, hword, 16);

}

/**
 * sal_config_uint8_get: get a uint8 variable associated with name
 * 
 * @param name (IN)- name of config item
 * @param byte (IN) - output byte address
 * @param len (IN) - output word buffer length
 * @return word count of output result
 *          0 : means no output 
 *    
 *   
 */

sys_error_t sal_config_uint8_get(const char*name, uint8* byte) {


    return sal_config_variable_get(name, byte, 8);

}


/**
 * sal_config_bytes_get: get bytes buffer associated with name
 * 
 * @param name (IN)- name of config item
 * @param buf (IN) - output word buffer address
 * @param len (IN) - output word buffer length
 * @return byte count of output result
 *    0 : means no output 
 *    
 *   
 */

int sal_config_bytes_get(const char*name, uint8* buf, int len) {

    const char *value, *end;
    int i;
    char p[3];
    uint8 base = 10;
    uint8 inc;
    value = sal_config_get(name);    

    if (value == NULL) {
        return 0;
    } 

    end = &value[sal_strlen(value)];

    if (sal_strncmp("0x", value, 2) == 0) {
        base = 16;
        inc = 2;
        value += 2;
    } else {
        base = 10;
        inc = 3;        
    }

    if (*value == 0) {
        return 0;
    }

    sal_memset(buf, 0, len);
    sal_memset(p, 0, sizeof(p));    

    for (i=0;(value < end && i<len); i++) {
         sal_strncpy(p, value, inc);
         buf[i] = sal_strtoul(p, NULL, base);
         value += inc;
    }
    return i;             

}

