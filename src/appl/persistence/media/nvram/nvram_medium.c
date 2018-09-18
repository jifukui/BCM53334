/*
 * $Id: nvram_medium.c,v 1.6 Broadcom SDK $
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
#include "nvram_medium.h"
#include "board.h"
#include "utils/nvram.h"

/*
 * We now uses version of individual items instead of WSS firmware version.
 */
#define ENABLE_UM_VERSION_CHECKING      0
#define SERIALIZE_SPACE                 0x800
#define NAME_FWVERSION                  "firmware_version"
#define BINARY_IDENTIFIER               '#'
#define STRING_ITEM_VERSION_POSTFIX     "_ver"
#define MAX_ITEM_NAME_LENGTH            (128)

static CODE const char base64[] = "0123456789"
                             "abcdefghijklmnopqrstuvwxyz"
                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                             "+/";
static char *_build_release = "V1.0.0";

#define THIS    ((MEDIUM_NVRAM *)ps)

/* Forwards */
STATICCBK int32 medium_open(BOOL write, struct _smedium_s *ps) REENTRANT;
STATICCBK int32 medium_close(struct _smedium_s *ps) REENTRANT;
STATICCBK BOOL medium_item_begin(const char *name, struct _smedium_s *ps, int32 version) REENTRANT;
STATICCBK BOOL medium_item_end(struct _smedium_s *ps) REENTRANT;
STATICCBK int32 medium_item_size(struct _smedium_s *ps) REENTRANT;
STATICCBK int32 medium_item_version(struct _smedium_s *ps) REENTRANT;
STATICCBK int32 medium_read(uint32 len, uint8 *buf, struct _smedium_s *ps) REENTRANT;
STATICCBK int32 medium_write(uint32 len, uint8 *buf, struct _smedium_s *ps) REENTRANT;
#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
STATICCBK const char * medium_read_string(struct _smedium_s *ps) REENTRANT;
STATICCBK BOOL medium_write_string(const char *str, struct _smedium_s *ps) REENTRANT;
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
STATICCBK BOOL medium_seek(uint32 pos, struct _smedium_s *ps) REENTRANT;
STATICCBK uint32 medium_medium_properties(struct _smedium_s *ps) REENTRANT;
STATICCBK const char *medium_get_medium_identification(struct _smedium_s *ps) REENTRANT;
STATICCBK int32 medium_get_total_data_size(struct _smedium_s *ps) REENTRANT;
STATICCBK uint32 medium_get_valid_groups(struct _smedium_s *ps) REENTRANT;

static const char *
get_final_item_name(struct _smedium_s *ps) REENTRANT
{
    static char name[MAX_ITEM_NAME_LENGTH];

    name[0] = 0;
    if (THIS->prefix != NULL && THIS->prefix[0] != 0) {
        sal_strcpy(name, THIS->prefix);
    }
    sal_strcat(name, THIS->item_name);
    
    return name;
}

static int32
get_item_and_decode(struct _smedium_s *ps) REENTRANT
{
    uint32 nbits, bits_buf;
    
    const char *value = nvram_get(get_final_item_name(ps));
    if (value == NULL) {
        return -1;
    }

    /* Check if it's binary data */    
    if (value[0] != BINARY_IDENTIFIER) {
        return -1;
    }
    value++;
    
    THIS->item_length = nbits = bits_buf = 0;
    for(; *value != 0; value++) {
        char *tmp = sal_strchr(base64, *value);
        if (tmp == NULL) {
            THIS->item_length = -1;
            return -1;
        }
        bits_buf |= ((uint32)tmp - (uint32)base64) << nbits;
        nbits += 6;
        
        while(nbits >= 8) {
            THIS->buf[THIS->item_length] = (unsigned char)(bits_buf & 0xFF);
            THIS->item_length++;
            bits_buf >>= 8;
            nbits -= 8;
        }
    }
    
    THIS->item_index += 2; /* Advance pointer for item version */
    
    return THIS->item_length;
}

static uint32
encode_binary(struct _smedium_s *ps, unsigned char *buf, unsigned char n) REENTRANT
{
    uint32 l = 0;
    
    THIS->bits_buf |= ((uint32)n) << THIS->nbits;
    THIS->nbits += 8;
    
    while (THIS->nbits >= 6) {
        buf[l] = base64[THIS->bits_buf & 0x3F];
        l++;
        THIS->bits_buf >>= 6;
        THIS->nbits -= 6;
    }
    
    return l;
}

static int32
get_string_item_version(struct _smedium_s *ps) REENTRANT
{
    char name[MAX_ITEM_NAME_LENGTH];
    const char *value;

    sal_strcpy(name, get_final_item_name(ps));
    sal_strcat(name, STRING_ITEM_VERSION_POSTFIX);
    
    value = nvram_get(name);
    
    if (value == NULL || value[0] == 0) {
        return -1;
    }    

    return (int32)sal_atoi(value);
}

STATICCBK int32 
medium_open(BOOL write, struct _smedium_s *ps) REENTRANT
{
    if (write) {
        if (THIS->open_type == 1) { /* Already opened with READ */
            return -1; 
        }
        THIS->open_type = 2;
    } else {
        if (THIS->open_type == 2) { /* Already opened with WRITE */
            return -1;
        }
#if ENABLE_UM_VERSION_CHECKING        
        /* Perform version checking */
        {
            char name[MAX_ITEM_NAME_LENGTH];
            const char *value;
            char *str;
        
            name[0] = 0;
            if (THIS->prefix != NULL && THIS->prefix[0] != 0) {
                sal_strcpy(name, THIS->prefix);
            }
            sal_strcat(name, NAME_FWVERSION);
            
            /* format: <boardname>/<sss_version>/<sdk_version> */
            value = nvram_get(name);
            if (value == NULL) {
                return -1; /* Version info does not exist */
            }
            
            /* Do not modify the string directly returned from nvram */
            sal_strcpy(name, value);
            value = name;
            
            /* Check boardname */
            str = sal_strchr(value, '/');
            if (str == NULL) {
                return -1; /* format not matched */
            }
            *str = 0;
            if (sal_strcmp(value, board_name())) {
                return -1; /* boardname not matched */
            }
            
            /* Check sss verion */
            value = str + 1;
            str = strchr(value, '/');
            if (str == NULL) {
                return -1; /* format not matched */
            }
            *str = 0;
            if (strcmp(value, _build_release)) {
                /* 
                 * Not matching current version, 
                 * check compatible versions.
                 */
                char *sv = sss_compatible_versions[0];
                for(; sv != NULL; sv++){
                    if (!strcmp(sv, "all")) {
                        break;
                    } else if (!strcmp(sv, value)) {
                        break;
                    }
                }
                if (sv == NULL) {
                    /* Not compatible versions matched */
                    return -1;
                }
            }
            /* Version verified OK */
        }
#endif /* ENABLE_UM_VERSION_CHECKING */        
        THIS->open_type = 1;
    }
    
    
    THIS->buf = sal_malloc(SERIALIZE_SPACE);
    if (THIS->buf == NULL) {
        sal_printf("medium_open: alloc failed!\n");
        THIS->open_type = 0;
        return -1;
    }
    
    THIS->item_name = NULL;
    
    return 0;
}

STATICCBK int32 
medium_close(struct _smedium_s *ps) REENTRANT
{
    int32 ret = 0;
    
    if (THIS->open_type == 0) {
        return 0;
    }
    
    if (THIS->item_name != NULL) {
        medium_item_end(ps);
    }
    
    if (THIS->open_type == 2) {

        /* Save version info */
        char name[32], value[64];
    
        name[0] = 0;
        if (THIS->prefix != NULL && THIS->prefix[0] != 0) {
            sal_strcpy(name, THIS->prefix);
        }
        sal_strcat(name, NAME_FWVERSION);
        sal_sprintf(value, "%s/%s", 
                board_name(), _build_release);
        nvram_set(name, value);
        
        /* If opened with WRITE, we should save to NVRAM */
        if (nvram_commit() != 0) {
            ret = -1;
        }
    }

    sal_free(THIS->buf);
    THIS->buf = NULL;
    
    /* Mark as closed */
    THIS->open_type = 0;
    
    return ret;
}

STATICCBK BOOL 
medium_item_begin(const char *name, struct _smedium_s *ps, int32 version) REENTRANT
{
    if (name == NULL || name[0] == 0) {
        return FALSE;
    }
    
    if (THIS->open_type == 0) {
        return FALSE;
    }
    
    if (THIS->item_name != NULL) {
        medium_item_end(ps);
    }
    
    THIS->item_name = name;
    
    /* If it's READ, check whether the NVRAM variable doesn't exist */
    if (THIS->open_type == 1) {
        if (nvram_get(get_final_item_name(ps)) == NULL) {
            THIS->item_name = NULL;
            return FALSE;
        }
    }
    
    /* Initialize values for item */
    THIS->item_length = -1;
    THIS->item_index = 0;
    THIS->item_version = version;
    THIS->item_dirty = 0;
    THIS->buf[0] = 0;
    THIS->bits_buf = 0;
    THIS->nbits = 0;
    
    return TRUE;
}

STATICCBK BOOL 
medium_item_end(struct _smedium_s *ps) REENTRANT
{
    BOOL ret = TRUE;
    
    if (THIS->open_type == 0) {
        return FALSE;
    }
    
    if (THIS->open_type == 2 && THIS->item_dirty) { /* open for WRITE*/
    
        if (THIS->buf[0] == BINARY_IDENTIFIER && THIS->nbits > 0) {
            /* 
             * Some leftover bits are not yet written (for binary item)
             */
            while(THIS->nbits < 6) {
                THIS->bits_buf |= ((uint32)1) << THIS->nbits;
                THIS->nbits++;
            }
            THIS->buf[THIS->item_index] = base64[THIS->bits_buf & 0x3F];
            THIS->item_index++;
            THIS->buf[THIS->item_index] = 0;
        }
    
        /* 
         * Write to NVRAM
         */
        if (nvram_set(get_final_item_name(ps), (char *)THIS->buf) != 0) {
            ret = FALSE;
        }
        
        /*
         * Write version number for string item in another NVRAM with postfix
         */
        if (THIS->buf[0] != BINARY_IDENTIFIER) {
            char name[MAX_ITEM_NAME_LENGTH];
            char ver[6];
            
            sal_strcpy(name, get_final_item_name(ps));
            sal_strcat(name, STRING_ITEM_VERSION_POSTFIX);
            sal_sprintf(ver, "%d", (int)((uint16)THIS->item_version));
            if (nvram_set(name, ver) != 0) {
                ret = FALSE;
            }
        }
    }
    
    THIS->item_name = NULL;
    
    return ret;
}

STATICCBK int32 
medium_item_size(struct _smedium_s *ps) REENTRANT
{
    const char *value;
    if (THIS->open_type != 1) {
        return -1;
    }
    if (THIS->item_name == NULL) {
        return -1;
    }

    value = nvram_get(get_final_item_name(ps));
    if (value == NULL) {
        return -1;
    }
    
    if (value[0] == BINARY_IDENTIFIER) {
        /* Read value from NVRAM if not yet */
        if (THIS->item_length == -1) {
            if (get_item_and_decode(ps) < 0) {
                return -1;
            }
        }
        
        /* Check version */
        if ((*(uint16 *)THIS->buf) != (uint16)THIS->item_version) {
            return -1;
        }
        
        return THIS->item_length - 2; /* 2: item version (16 bits) */

    } else {

        /* Check version */
        int32 v = get_string_item_version(ps);
        if (v < 0 || (uint16)v != (uint16)THIS->item_version) {
            return -1;
        }
        
        return sal_strlen(value) + 1;

    }
}

STATICCBK int32
medium_item_version(struct _smedium_s *ps) REENTRANT
{
    const char *value;
    if (THIS->open_type != 1) {
        return -1;
    }
    if (THIS->item_name == NULL) {
        return -1;
    }

    value = nvram_get(get_final_item_name(ps));
    if (value == NULL) {
        return -1;
    }
    
    if (value[0] == BINARY_IDENTIFIER) {
        /* Read value from NVRAM if not yet */
        if (THIS->item_length == -1) {
            if (get_item_and_decode(ps) < 0) {
                return -1;
            }
        }
        return *(uint16 *)THIS->buf;
    } else {
        return get_string_item_version(ps);
    }
}

STATICCBK int32 
medium_read(uint32 len, uint8 *buf, struct _smedium_s *ps) REENTRANT
{
    if (len == 0) {
        return 0;
    }
    if (buf == NULL) {
        return -1;
    }
    
    if (THIS->open_type != 1) {
        return -1;
    }
    if (THIS->item_name == NULL) {
        return -1;
    }
    
    /* Read value from NVRAM if not yet */
    if (THIS->item_length == -1) {
        if (get_item_and_decode(ps) < 0) {
            return -1;
        }
    }
    
    /* Check item version */
    if ((*(uint16 *)THIS->buf) != (uint16)THIS->item_version) {
        return -1;
    }
    
    /* Check length */
    if (THIS->item_index + len > THIS->item_length) {
        /* If user expect to read more than we have,
         * it's an error. */
        return -1;
    }
    
    /* Copy data into user buffer */
    sal_memcpy(buf, &THIS->buf[THIS->item_index], len);
    
    /* Advance current index */
    THIS->item_index += len;
    
    return len;
}

STATICCBK int32 
medium_write(uint32 len, uint8 *buf, struct _smedium_s *ps) REENTRANT
{
    uint32 i;
    
    if (len == 0) {
        return 0;
    }
    if (buf == NULL) {
        return -1;
    }
    
    if (THIS->open_type != 2) {
        return -1;
    }
    if (THIS->item_name == NULL) {
        return -1;
    }

    if (THIS->item_index == 0) {
        unsigned char buf[2]; 
        
        /* Add binary identifier */
        THIS->buf[0] = BINARY_IDENTIFIER;
        THIS->item_index++;
        
        /* Write item version */
        *(uint16 *)buf = (uint16)THIS->item_version;
        THIS->item_index += encode_binary(
                                ps, 
                                &THIS->buf[THIS->item_index], 
                                buf[0]
                                );
        THIS->item_index += encode_binary(
                                ps, 
                                &THIS->buf[THIS->item_index], 
                                buf[1]
                                );
    }    
    
    /* Encode data into buffer */
    for(i=0; i<len; i++) {
        THIS->item_index += 
                encode_binary(ps, &THIS->buf[THIS->item_index], buf[i]);
    }
    THIS->buf[THIS->item_index] = 0; /* string terminator */
    
    /* It's touched */
    THIS->item_dirty = 1;

    return len;
}

#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
STATICCBK const char * 
medium_read_string(struct _smedium_s *ps) REENTRANT
{
    int32 v;
    
    if (THIS->open_type != 1) {
        return NULL;
    }
    if (THIS->item_name == NULL) {
        return NULL;
    }
    
    /* Check item version */
    v = get_string_item_version(ps);
    if (v < 0 || (uint16)v != (uint16)THIS->item_version) {
        return NULL;
    }
    
    return nvram_get(get_final_item_name(ps));
}

STATICCBK BOOL 
medium_write_string(const char *str, struct _smedium_s *ps) REENTRANT
{
    if (THIS->open_type != 2) {
        return FALSE;
    }
    if (THIS->item_name == NULL) {
        return -1;
    }
    
    if (str == NULL) {
        str = "";
    }
    
    sal_strcpy((void *)THIS->buf, str);
    
    /* It's touched */
    THIS->item_dirty = 1;
    
    return TRUE;
}
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */

STATICCBK BOOL 
medium_seek(uint32 pos, struct _smedium_s *ps) REENTRANT
{
    UNREFERENCED_PARAMETER(pos);
    UNREFERENCED_PARAMETER(ps);
    return FALSE;
}

STATICCBK uint32 
medium_medium_properties(struct _smedium_s *ps) REENTRANT
{
    UNREFERENCED_PARAMETER(ps);

    /* It's a sparse medium with "item size" and "item version" capability */
    return SMEDIUM_PROP_GET_ITEM_DATA_SIZE | SMEDIUM_PROP_GET_ITEM_VERSION;
}

STATICCBK int32 
medium_get_total_data_size(struct _smedium_s *ps) REENTRANT
{
    UNREFERENCED_PARAMETER(ps);
    return -1; /* Not supported */
}

STATICCBK uint32 
medium_get_valid_groups(struct _smedium_s *ps) REENTRANT
{
    UNREFERENCED_PARAMETER(ps);
    return THIS->groups;
}

STATICCBK const char *
medium_get_medium_identification(struct _smedium_s *ps) REENTRANT
{
    UNREFERENCED_PARAMETER(ps);
    return "BRCM-NVRAM";
}

BOOL 
nvram_medium_initialize(MEDIUM_NVRAM *medium, const char *prefix, uint32 groups) REENTRANT
{
    medium->smedium.open = medium_open;
    medium->smedium.close = medium_close;
    medium->smedium.item_begin = medium_item_begin;
    medium->smedium.item_end = medium_item_end;
    medium->smedium.item_size = medium_item_size;
    medium->smedium.item_version = medium_item_version;
    medium->smedium.read = medium_read;
    medium->smedium.write = medium_write;
#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
    medium->smedium.read_string = medium_read_string;
    medium->smedium.write_string = medium_write_string;
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
    medium->smedium.seek = medium_seek;
    medium->smedium.medium_properties = medium_medium_properties;
    medium->smedium.get_medium_identification = medium_get_medium_identification;
    medium->smedium.get_total_data_size = medium_get_total_data_size;
    medium->smedium.get_valid_groups = medium_get_valid_groups;

    medium->groups = groups;
    medium->open_type = 0;
    medium->prefix = prefix;
    medium->buf = NULL;

    return TRUE;
}
