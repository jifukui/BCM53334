/*
 * $Id: ramtxt_medium.c,v 1.5 Broadcom SDK $
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
#include "ramtxt_medium.h"

#define NAME_FWVERSION "firmware_version"
#define BINARY_IDENTIFIER   '%'

static const char hex[] = "0123456789ABCDEF";
extern char *um_compatible_versions[];

#define THIS    ((MEDIUM_RAMTXT *)ps)

/* Forwards */
STATICCBK void medium_initialize(void *param, struct _smedium_s *ps) REENTRANT;
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

static int32
find_item(const char *name, struct _smedium_s *ps) REENTRANT
{
    int32 idx = THIS->index;
    unsigned char *pm = THIS->memory;

    for(;;) {
        if (!sal_memcmp(pm + idx, name, sal_strlen(name)) &&
            pm[idx + sal_strlen(name)] == '=') {

            /* Matching "<name>=" : found! */
            return idx + sal_strlen(name) + 1;
        }

        if (pm[idx] == '\n') {

            /* Reach end of file */
            idx = 0;

        } else {

            /* Skip next '\n' */
            for(; pm[idx] != '\n'; idx++) {
                if (pm[idx] == 0) {
                    /* Invalid format! */
                    return -1;
                }
            }
            idx++;
        }

        if (idx == THIS->index) {
            return -1;
        }
    }
    return -1;
}

static uint32
encode_binary(unsigned char *buf, unsigned char n) REENTRANT
{
    buf[0] = hex[n >> 4];
    buf[1] = hex[n & 0x0F];

    return 2;
}

STATICCBK void
medium_initialize(void *param, struct _smedium_s *ps) REENTRANT
{
    THIS->smedium.open = medium_open;
    THIS->smedium.close = medium_close;
    THIS->smedium.item_begin = medium_item_begin;
    THIS->smedium.item_end = medium_item_end;
    THIS->smedium.item_size = medium_item_size;
    THIS->smedium.item_version = medium_item_version;
    THIS->smedium.read = medium_read;
    THIS->smedium.write = medium_write;
#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
    THIS->smedium.read_string = medium_read_string;
    THIS->smedium.write_string = medium_write_string;
#endif
    THIS->smedium.seek = medium_seek;
    THIS->smedium.medium_properties = medium_medium_properties;
    THIS->smedium.get_medium_identification = medium_get_medium_identification;
    THIS->smedium.get_total_data_size = medium_get_total_data_size;
    THIS->smedium.get_valid_groups = medium_get_valid_groups;

    THIS->memory = (unsigned char *)param;
    THIS->open_type = 0;
}

STATICCBK int32
medium_open(BOOL write, struct _smedium_s *ps) REENTRANT
{
    uint8 major, minor, eco, misc;

    if (write) {

        if (THIS->open_type == 1) { /* Already opened with READ */
            return -1;
        }

        /* Add version info */
        board_firmware_version_get(&major, &minor, &eco, &misc);
        sal_sprintf((char*)THIS->memory, "%s=%s/%02d%02d%02d\n",
                NAME_FWVERSION,
                board_name(),
                major, minor, eco, misc);
        THIS->index = sal_strlen((char *)THIS->memory);
        THIS->open_type = 2;
    } else {
        if (THIS->open_type == 2) { /* Already opened with WRITE */
            return -1;
        }

        /* Perform version checking */
        {
            char buf[256], _build_release[16];
            const char *value;
            char *str;

            /* copy the first line (containing version info) to buf */
            str = sal_strchr((char *)THIS->memory, '\n');
            if (str == NULL) {
                return -1; /* Invalid format */
            }
            sal_memcpy(buf, THIS->memory, (int)str - (int)THIS->memory);
            buf[(int)str - (int)THIS->memory] = 0;
            /* We've used some bytes */
            THIS->index = ((int)str - (int)THIS->memory) + 1;

            /* format: <boardname>/<um_version> */
            value = buf;
            if (sal_strncmp(NAME_FWVERSION, value, sal_strlen(NAME_FWVERSION))) {
                return -1; /* Invalid format */
            }
            value += sal_strlen(NAME_FWVERSION);
            if (*value != '=') {
                return -1; /* Invalid format */
            }
            value++;

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
            board_firmware_version_get(&major, &minor, &eco, &misc);
            sal_sprintf(_build_release, "%02d%02d%02d", major, minor, eco, misc);
                      
            if (sal_strcmp(value, _build_release)) {

                sal_printf("version check fail\n");
                /*
                 * Not matching current version,
                 * check compatible versions.
                 */
#if 0
                char *sv = um_compatible_versions[0];
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
#endif
                return -1;
            }
            /* Version verified OK */
          
        }

        THIS->open_type = 1;
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

    if (THIS->open_type == 2) { /* WRITE */
        sal_strcat((char *)&THIS->memory[THIS->index], "\n");
        THIS->index++;
    }

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

    /* If it's READ, find the start address of this item */
    if (THIS->open_type == 1) {

        /* find_item returns: start address of the value */
        int32 r = find_item(name, ps);
        if (r < 0) {
            return FALSE;
        }
        THIS->index = r;
        THIS->item_index = r;
        THIS->item_modified = 0;
        THIS->item_read = 0;

    } else {

        THIS->item_dirty = 0;
    }

    THIS->item_name = name;

    return TRUE;
}

STATICCBK BOOL
medium_item_end(struct _smedium_s *ps) REENTRANT
{
    BOOL ret = TRUE;
    if (THIS->open_type == 0) {
        return FALSE;
    }

    THIS->item_name = NULL;

    if (THIS->open_type == 1) {

        if (THIS->item_modified) {
            THIS->memory[THIS->index] = '\n';
        } else {
            for(; THIS->memory[THIS->index] != '\n'; THIS->index++) {
                if (THIS->memory[THIS->index] == 0) {
                    /* Invalid format! */
                    return FALSE;
                }
            }
        }
        THIS->index++;

    } else if (THIS->open_type == 2 && THIS->item_dirty) {
        sal_strcat((char *)&THIS->memory[THIS->index], "\n");
        THIS->index++;
    }

    return ret;
}

STATICCBK int32
medium_item_size(struct _smedium_s *ps) REENTRANT
{
    int32 idx = THIS->item_index;

    if (THIS->open_type != 1) {
        return -1;
    }

    for(; THIS->memory[idx] != '\n'; idx++) {
        if (THIS->memory[idx] == 0) {
            /* Invalid format! */
            return -1;
        }
    }
    if (THIS->memory[THIS->item_index] == BINARY_IDENTIFIER) {
        return (idx - THIS->item_index - 1) / 2;
    } else {
        return (idx - THIS->item_index) + 1;
    }
}

STATICCBK int32
medium_item_version(struct _smedium_s *ps) REENTRANT
{
    /* Not supported */
    return -1;
}

STATICCBK int32
medium_read(uint32 len, uint8 *buf, struct _smedium_s *ps) REENTRANT
{
    int32 idx, dig, sum;
    char *tmp;
    char *value = (char *)&THIS->memory[THIS->index];

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

    if (THIS->item_read == 0) {
        if (*value != BINARY_IDENTIFIER) {
            return -1;
        }
        THIS->item_read = 1;
        value++;
        THIS->index++;
    }

    idx = dig = sum = 0;
    for(; *value != '\n'; value++, THIS->index++) {

        if (*value == 0) {
            return -1; /* Invalid format */
        }
        tmp = sal_strchr(hex, *value);
        if (tmp == NULL) {
            return -1;
        }
        sum |= (tmp - hex);

        if (dig == 1) {
            buf[idx++] = sum;
            dig = 0;
            sum = 0;
        } else {
            dig++;
            sum <<= 4;
        }

        if (idx == len) {
            break;
        }
    }

    /* Point index to the next available data */
    if (*value != '\n') {
        THIS->index++;
    }

    return idx;
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

    /* Add <name>=<binary_identifier> if not yet */
    if (THIS->item_dirty == 0) {
        sal_strcat((char *)&THIS->memory[THIS->index], THIS->item_name);
        sal_strcat((char *)&THIS->memory[THIS->index], "=%");
        THIS->index += sal_strlen((char *)THIS->item_name) + 2;
    }

    /* encode data into buffer */
    for(i=0; i<len; i++) {
        THIS->index +=
                encode_binary(&THIS->memory[THIS->index], buf[i]);
    }
    THIS->memory[THIS->index] = 0; /* string terminator */

    /* It's touched */
    THIS->item_dirty = 1;

    return len;
}
#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
STATICCBK const char *
medium_read_string(struct _smedium_s *ps) REENTRANT
{
    const char *ret;
    int32 idx = THIS->index;

    if (THIS->open_type != 1) {
        return NULL;
    }
    if (THIS->item_name == NULL) {
        return NULL;
    }

    if (THIS->open_type != 1) {
        return NULL;
    }

    for(; THIS->memory[idx] != '\n'; idx++) {
        if (THIS->memory[idx] == 0) {
            /* Invalid format! */
            return NULL;
        }
    }

    /* Set string terminator */
    THIS->memory[idx] = 0;
    THIS->item_modified = 1;

    ret = (char *)&THIS->memory[THIS->index];
    THIS->index = idx;

    return ret;
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

    /* Add <name>= if not yet */
    if (THIS->item_dirty == 0) {
        sal_strcat((char *)&THIS->memory[THIS->index], THIS->item_name);
        sal_strcat((char *)&THIS->memory[THIS->index], "=");
        THIS->index += sal_strlen(THIS->item_name) + 1;
    }

    sal_strcpy((char *)&THIS->memory[THIS->index], str);
    THIS->index += sal_strlen(str);

    /* It's touched */
    THIS->item_dirty = 1;

    return TRUE;
}
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */

STATICCBK BOOL
medium_seek(uint32 pos, struct _smedium_s *ps) REENTRANT
{
    return FALSE;
}

STATICCBK uint32
medium_medium_properties(struct _smedium_s *ps) REENTRANT
{
    /* It's a sparse medium with "get item size" capability */
    return SMEDIUM_PROP_GET_ITEM_DATA_SIZE;
}

STATICCBK int32
medium_get_total_data_size(struct _smedium_s *ps) REENTRANT
{
    return -1; /* Not supported */
}

STATICCBK uint32
medium_get_valid_groups(struct _smedium_s *ps) REENTRANT
{
    return (uint32)-1;
}

STATICCBK const char *
medium_get_medium_identification(struct _smedium_s *ps) REENTRANT
{
    return "RAM-TXT";
}

BOOL
ramtxt_medium_initalize(MEDIUM_RAMTXT *medium, unsigned char *buf) REENTRANT
{
    medium_initialize((void *)buf, (PERSISTENT_MEDIUM *)medium);
    return TRUE;
}
