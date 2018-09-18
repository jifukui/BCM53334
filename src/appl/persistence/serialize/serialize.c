/*
 * $Id: serialize.c,v 1.14 Broadcom SDK $
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

#define SERIALIZE_DEBUG   0 /* 1-5 for debug depth */
#define SERIALIZE_TRACE   0

#include "system.h"
#include "serialization.h"
#include "utils/net.h"
#if SERIALIZE_DEBUG
#define _M_DEBUG(depth, x) do { \
    if (SERIALIZE_DEBUG >= depth) { \
        sal_printf("SRL: "); sal_printf x; sal_printf("\n");\
    } \
} while(0)
#else /* !SERIALIZE_DEBUG */
#define _M_DEBUG(depth, x) do { } while(0)
#endif /* SERIALIZE_DEBUG */

#ifndef MAX_NUMBER_SERIALIZERS
#define MAX_NUMBER_SERIALIZERS      (25)
#endif

/* Internal serializer entry */
typedef struct {
    void *callback;
    const char *name;
    uint32 groups;
#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
    unsigned char bstring;      /* 1 if string-type */
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
    unsigned char bdefaults;    /* 1 if SERIALIZE_OP_LOAD_DEFAULTS supported */
} SERIALIZE_S;

static SERIALIZE_S g_serializers[MAX_NUMBER_SERIALIZERS + 1] = { { NULL } };

static PERSISTENT_MEDIUM *pmedium = NULL;
#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
static BOOL is_string_type = FALSE;
static int32 max_strlen;
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
static SERIALIZE_OP current_op;

static const char * const CODE op_captions[] = {
    "COUNT",
    "LOAD",
    "SAVE",
    "VERSION",
    "LOAD_DEFAULTS",
    "SAVE_DEFAULTS",
    "VALIDATE",
    "VALIDATE_DEFAULTS",
};

#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
STATICCBK const char *
serl_read_string(void) REENTRANT
{
    const char *str;
    SAL_ASSERT(pmedium != NULL);

    /* If assertion failed, your serializer is registered w/ wrong type. */
    SAL_ASSERT(is_string_type);

    /* If assertion failed, your serializer calls wrong types of functions. */
    SAL_ASSERT(current_op == SERIALIZE_OP_LOAD);

    str = (*pmedium->read_string)(pmedium);
    if (str != NULL && sal_strlen(str) > max_strlen) {
        _M_DEBUG(5, ("      read_string: NULL or INVALID!"));
        return NULL;
    }
    _M_DEBUG(5, ("      read_string: \"%s\"", str));
    return str;
}

STATICCBK BOOL
serl_write_string(const char *str) REENTRANT
{
    SAL_ASSERT(pmedium != NULL);

    /* If assertion failed, your serializer is registered w/ wrong type. */
    SAL_ASSERT(is_string_type);

    /* If assertion failed, your serializer calls wrong types of functions. */
    SAL_ASSERT(current_op == SERIALIZE_OP_SAVE);

    if (str == NULL || (str != NULL && sal_strlen(str) > max_strlen)) {
        _M_DEBUG(5, ("      write_string: NULL or INVALID!"));
        return FALSE;
    }

    _M_DEBUG(5, ("      write_string: \"%s\"", str));
    return (*pmedium->write_string)(str, pmedium);
}
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */

STATICCBK int32
serl_read(uint8 *buf, uint32 len) REENTRANT
{
    int32 r;

    SAL_ASSERT(pmedium != NULL);

    /* If assertion failed, your serializer is registered w/ wrong type. */
    SAL_ASSERT(!is_string_type);

    /* If assertion failed, your serializer calls wrong types of functions. */
    SAL_ASSERT(current_op == SERIALIZE_OP_LOAD);
    if (len == 0) {
        _M_DEBUG(5, ("      read: len=0"));
        return 0;
    }
    if (buf == NULL) {
        _M_DEBUG(5, ("      ERROR: read: buf == NULL"));
        return -1;
    }

    r = (*pmedium->read)(len, buf, pmedium);

#if (SERIALIZE_DEBUG > 5) || SERIALIZE_TRACE
    {
        int i;
        sal_printf("SRL:       read(%d): ", r);
        for(i=0; i<len; i++) {
            sal_printf("%02X ", buf[i]);
        }
        sal_printf("\n");
    }
#endif

    return r;
}

STATICCBK int32
serl_write(uint8 *buf, uint32 len) REENTRANT
{
    int32 r;

    SAL_ASSERT(pmedium != NULL);

    /* If assertion failed, your serializer is registered w/ wrong type. */
    SAL_ASSERT(!is_string_type);

    /* If assertion failed, your serializer calls wrong types of functions. */
    SAL_ASSERT(current_op == SERIALIZE_OP_SAVE);
    if (len == 0) {
        _M_DEBUG(5, ("      write: len=0"));
        return 0;
    }
    if (buf == NULL) {
        _M_DEBUG(5, ("      ERROR: write: buf == NULL"));
        return -1;
    }

    r = (*pmedium->write)(len, buf, pmedium);

#if (SERIALIZE_DEBUG > 5) || SERIALIZE_TRACE
    {
        int i;
        sal_printf("SRL:       write(%d): ", r);
        for(i=0; i<len; i++) {
            sal_printf("%02X ", buf[i]);
        }
        sal_printf("\n");
    }
#endif

    return r;
}

STATICCBK int32
serl_read_uint32(uint32 *pval) REENTRANT
{
    int32 r;

    SAL_ASSERT(pmedium != NULL);

    /* If assertion failed, your serializer is registered w/ wrong type. */
    SAL_ASSERT(!is_string_type);

    /* If assertion failed, your serializer calls wrong types of functions. */
    SAL_ASSERT(current_op == SERIALIZE_OP_LOAD);

    if (pval == NULL) {
        _M_DEBUG(5, ("      ERROR: read_uint: pval == NULL"));
        return -1;
    }

    r = (*pmedium->read)(sizeof(uint32), (uint8 *)pval, pmedium);

#if CFG_BIG_ENDIAN
    if (r == sizeof(uint32)) {
        *pval = ltoh32(*pval);
    }
#endif /* CFG_BIG_ENDIAN */

    _M_DEBUG(5, ("      read_uint(%d): 0x%08X\n", r, *pval));
    return r;
}

STATICCBK int32
serl_write_uint32(uint32 val) REENTRANT
{
    int32 r;
    uint32 val2;

    SAL_ASSERT(pmedium != NULL);

    /* If assertion failed, your serializer is registered w/ wrong type. */
    SAL_ASSERT(!is_string_type);

    /* If assertion failed, your serializer calls wrong types of functions. */
    SAL_ASSERT(current_op == SERIALIZE_OP_SAVE);

#if CFG_BIG_ENDIAN
    val2 = ltoh32(val);
#else
    val2 = val;
#endif /* CFG_BIG_ENDIAN */

    r = (*pmedium->write)(sizeof(uint32), (uint8 *)&val2, pmedium);

    _M_DEBUG(5, ("      write_uint(%d): 0x%08X\n", r, val));
    return r;
}

STATICCBK int32
serl_read_uint16(uint16 *pval) REENTRANT
{
    int32 r;

    SAL_ASSERT(pmedium != NULL);

    /* If assertion failed, your serializer is registered w/ wrong type. */
    SAL_ASSERT(!is_string_type);

    /* If assertion failed, your serializer calls wrong types of functions. */
    SAL_ASSERT(current_op == SERIALIZE_OP_LOAD);

    if (pval == NULL) {
        _M_DEBUG(5, ("      ERROR: read_ushort: pval == NULL"));
        return -1;
    }

    r = (*pmedium->read)(sizeof(uint16), (uint8 *)pval, pmedium);

#if CFG_BIG_ENDIAN
    if (r == sizeof(uint16)) {
        *pval = ltoh16(*pval);
    }
#endif /* CFG_BIG_ENDIAN */

    _M_DEBUG(5, ("      read_ushort(%d): 0x%04hX\n", r, *pval));
    return r;
}

STATICCBK int32
serl_write_uint16(uint16 val) REENTRANT
{
    int32 r;
    uint16 val2;

    SAL_ASSERT(pmedium != NULL);

    /* If assertion failed, your serializer is registered w/ wrong type. */
    SAL_ASSERT(!is_string_type);

    /* If assertion failed, your serializer calls wrong types of functions. */
    SAL_ASSERT(current_op == SERIALIZE_OP_SAVE);

#if CFG_BIG_ENDIAN
    val2 = ltoh16(val);
#else
    val2 = val;
#endif /* CFG_BIG_ENDIAN */

    r = (*pmedium->write)(sizeof(uint16), (uint8 *)&val2, pmedium);

    _M_DEBUG(5, ("      write_ushort(%d): 0x%04hX\n", r, val));
    return r;
}

#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
static const SERL_MEDIUM_STR CODE serl_med_str = {
            serl_read_string,
            serl_write_string,
            serl_read,
            serl_write,
            serl_read_uint32,
            serl_write_uint32,
            serl_read_uint16,
            serl_write_uint16
       };
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
static const SERL_MEDIUM_BIN CODE serl_med_bin = {
#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
            serl_read_string,
            serl_write_string,
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
            serl_read,
            serl_write,
            serl_read_uint32,
            serl_write_uint32,
            serl_read_uint16,
            serl_write_uint16
       };

static BOOL
register_serializer(
    const char *name,
    uint32 groups,
    void *func,
    BOOL defaults,
    int bstring)
{
    int i;

    /* Check parameters */
    SAL_ASSERT(func != NULL);
    SAL_ASSERT(name != NULL && name[0] != 0);


    for(i=0; i<MAX_NUMBER_SERIALIZERS; i++) {
        if (g_serializers[i].callback == NULL) {
            break;
        } else {
            /* Duplicated serializer names are not allowed */
            SAL_ASSERT(strcmp(name, g_serializers[i].name));
        }
    }

    /* Enlarge the number if assertion failed. */
    SAL_ASSERT(i < MAX_NUMBER_SERIALIZERS);

    if (i >= MAX_NUMBER_SERIALIZERS) {
        sal_printf("Serializer %s (%p) registered fail\n", name, func);
        return FALSE;
    }
    /* Add this entry */
    g_serializers[i].callback = func;
    g_serializers[i].name = name;
    g_serializers[i].groups = groups;
#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
    g_serializers[i].bstring = (unsigned char)bstring;
#else /* !CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
    UNREFERENCED_PARAMETER(bstring);
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
    g_serializers[i].bdefaults = (unsigned char)defaults;

    /* here i < MAX_NUMBER_SERILIZERS  */
    g_serializers[i + 1].callback = NULL;


    _M_DEBUG(1, ("Serializer %s (%p) registered.", name, func));
    return TRUE;
}

BOOL
register_binary_serializer(
    const char *name,
    uint32 grps,
    SERIALIZER_BIN func,
    BOOL defaults)
{
    return register_serializer(name, grps, func, defaults, FALSE);
}

#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
BOOL
register_string_serializer(
    const char *name,
    uint32 grps,
    SERIALIZER_STR func,
    BOOL defaults)
{
    return register_serializer(name, grps, func, defaults, TRUE);
}
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */

static int32
serialize_item(SERIALIZE_S *ps, int32 index)
{
    int32 r;

#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
    /* set string type for serializer */
    is_string_type = (BOOL)ps->bstring;
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */

    if (current_op == SERIALIZE_OP_LOAD_DEFAULTS) {
        if (ps->bdefaults) {
            /* If this serializer supports LOAD_DEFAULTS */
            _M_DEBUG(3, ("  Calling serializer for LOAD_DEFAULTS"));
#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
            if (is_string_type) {
                r = (*(SERIALIZER_STR)ps->callback)(
                            SERIALIZE_OP_LOAD_DEFAULTS,
                            (SERL_MEDIUM_STR *)&serl_med_str);
            } else
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
            {
                r = (*(SERIALIZER_BIN)ps->callback)(
                            SERIALIZE_OP_LOAD_DEFAULTS,
                            (SERL_MEDIUM_BIN *)&serl_med_bin);
            }
            /* Indicates the LOAD_DEFAULTS support is incorrect */
            SAL_ASSERT(r == 0);
            return 0;
        } else {
            /* Otherwise, it's simply a LOAD operation */
            current_op = SERIALIZE_OP_LOAD;
        }
    } else if (current_op == SERIALIZE_OP_SAVE_DEFAULTS) {
        if (ps->bdefaults) {
            _M_DEBUG(3, ("  Skipping serializer for SAVE_DEFAULTS"));
            return 0;
        } else {
            /* Otherwise, it's simply a SAVE operation */
            current_op = SERIALIZE_OP_SAVE;
        }
    } else if (current_op == SERIALIZE_OP_VALIDATE_DEFAULTS) {
        if (ps->bdefaults) {
            _M_DEBUG(3, ("  Skipping serializer for VALIDATE_DEFAULTS"));
            return 0;
        } else {
            /* Otherwise, it's simply a VALIDATE operation */
            current_op = SERIALIZE_OP_VALIDATE;
        }
    }

    /* get version of serializer */
#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
    if (is_string_type) {
        r = (*(SERIALIZER_STR)ps->callback)(SERIALIZE_OP_VERSION,
                                            (SERL_MEDIUM_STR *)&serl_med_str);
    } else
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
    {
        r = (*(SERIALIZER_BIN)ps->callback)(SERIALIZE_OP_VERSION,
                                            (SERL_MEDIUM_BIN *)&serl_med_bin);
    }

    /* Assertion failed means your serializer doesn't return version number */
    SAL_ASSERT((uint32)(r & (~SERIALIZE_VERSION_MASK)) == SERIALIZE_VERSION_MAGIC);
    r &= SERIALIZE_VERSION_MASK;

    _M_DEBUG(3, ("  BEGIN: item %s (ver %d) with op=%s",
                    ps->name, r, op_captions[current_op]));

    /* Item begin */
    if (pmedium->item_begin(ps->name, pmedium, r) == FALSE) {

        _M_DEBUG(3, ("  item_begin() failed, checking if it's dummy....."));

        /* Check special case: dummy serializer (count == 0) */
#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
        if (is_string_type) {
            _M_DEBUG(3, ("  Calling string serializer, op=COUNT"));
            r = (*(SERIALIZER_STR)ps->callback)(
                            SERIALIZE_OP_COUNT, (SERL_MEDIUM_STR *)&serl_med_str);
        } else
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
        {
            _M_DEBUG(3, ("  Calling binary serializer, op=COUNT"));
            r = (*(SERIALIZER_BIN)ps->callback)(
                            SERIALIZE_OP_COUNT, (SERL_MEDIUM_BIN *)&serl_med_bin);
        }
        if (r < 0) {
            _M_DEBUG(3, ("    Failed!"));
            return -1;
        }
        _M_DEBUG(3, ("    Return: %d", r));

        if (r == 0) {
            _M_DEBUG(3, ("  Confirmed DUMMY serializer"));
            _M_DEBUG(3, ("  END: item %s", ps->name));
            return 0;
        } else {
            _M_DEBUG(3, ("  item_begin() failed (confirmed)!"));
        }

        return -1;
    }

    /* If seekable, seek to start address */
    if ((*pmedium->medium_properties)(pmedium) & SMEDIUM_PROP_COMPACT) {
        if ((*pmedium->seek)(index, pmedium) == FALSE) {
            _M_DEBUG(3, ("  seek(%s, %d) failed!", ps->name, index));
            return -1;
        }
    }

#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
    if (is_string_type) { /* string type */

        /* Get max string length */
        _M_DEBUG(3, ("  Calling string serializer, op=COUNT"));
        r = (*(SERIALIZER_STR)ps->callback)(SERIALIZE_OP_COUNT,
                                            (SERL_MEDIUM_STR *)&serl_med_str);
        if (r < 0) {
            _M_DEBUG(3, ("    Failed!"));
            return -1;
        }
        _M_DEBUG(3, ("    Return: %d", r));

        /* For error/bug checking */
        max_strlen = r;

        /* Validate size first if it's loading & can get item size */
        if ((current_op == SERIALIZE_OP_LOAD ||
             current_op == SERIALIZE_OP_VALIDATE) &&
            ((*pmedium->medium_properties)(pmedium) &
                SMEDIUM_PROP_GET_ITEM_DATA_SIZE)) {

            _M_DEBUG(3, ("  Matching item size"));
            if ((*pmedium->item_size)(pmedium) == -1) {
                _M_DEBUG(3, ("    Failed: item not found"));
                return -1;
            }
            if ((max_strlen + 1) < (*pmedium->item_size)(pmedium)) {
                _M_DEBUG(3, ("    Failed: %d != %d",
                          (max_strlen + 1),
                          (*pmedium->item_size)(pmedium)));
                return -1;
            }
            _M_DEBUG(3, ("    OK"));
        }

        /* Call serializer */
        if (current_op != SERIALIZE_OP_VALIDATE) {
            _M_DEBUG(3, ("  Calling string serializer, op=%s",
                         op_captions[current_op]));
            r = (*(SERIALIZER_STR)ps->callback)(current_op,
                                                (SERL_MEDIUM_STR *)&serl_med_str);
            if (r < 0) {
                _M_DEBUG(3, ("    Failed!"));
                return -1;
            }
            _M_DEBUG(3, ("    Return: %d", r));
        }

        r = max_strlen + 1;

    } else
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
    { /* binary type */

        /* Get required data size */
        _M_DEBUG(3, ("  Calling binary serializer, op=COUNT"));
        r = (*(SERIALIZER_BIN)ps->callback)(SERIALIZE_OP_COUNT,
                                            (SERL_MEDIUM_BIN *)&serl_med_bin);
        if (r < 0) {
            _M_DEBUG(3, ("    Failed!"));
            return -1;
        }
        _M_DEBUG(3, ("    Return: %d", r));

        /* Validate size first if it's loading & can get item size */
        if ((current_op == SERIALIZE_OP_LOAD ||
             current_op == SERIALIZE_OP_VALIDATE) &&
            ((*pmedium->medium_properties)(pmedium) &
                SMEDIUM_PROP_GET_ITEM_DATA_SIZE)) {

            _M_DEBUG(3, ("  Matching item size"));
            if ((*pmedium->item_size)(pmedium) == -1) {
                _M_DEBUG(3, ("    Failed: item not found"));
                return -1;
            }
            if (r != (*pmedium->item_size)(pmedium)) {
                _M_DEBUG(3, ("    Failed: %d != %d", r,
                          (*pmedium->item_size)(pmedium)));
                return -1;
            }
            _M_DEBUG(3, ("    OK"));
        }

        /* Call serializer */
        if (current_op != SERIALIZE_OP_VALIDATE) {
            _M_DEBUG(3, ("  Calling binary serializer, op=%s",
                         op_captions[current_op]));
            r = (*(SERIALIZER_BIN)ps->callback)(current_op,
                                                (SERL_MEDIUM_BIN *)&serl_med_bin);
            if (r < 0) {
                _M_DEBUG(3, ("    Failed!"));
                return -1;
            }
            _M_DEBUG(3, ("    Return: %d", r));
        }
    }

    /* If seekable, seek to ending address */
    if ((*pmedium->medium_properties)(pmedium) & SMEDIUM_PROP_COMPACT) {
        if ((*pmedium->seek)(index + r, pmedium) == FALSE) {
            _M_DEBUG(3, ("  seek(%s, %d) failed!", ps->name, index));
            return -1;
        }
    }

    if (pmedium->item_end(pmedium) == FALSE) {
        _M_DEBUG(3, ("  item_end() failed!"));
        return -1;
    }

    _M_DEBUG(3, ("  END: item %s", ps->name));
    return r;
}

int32
serialize_by_groups(uint32 groups, SERIALIZE_OP op, PERSISTENT_MEDIUM *med)
{
    int32 index = 0; /* It's actually used only for compact medium */
    int r;
    int32 error = 0; /* To count how many error occurred */
    SERIALIZE_S *ps = g_serializers;

    _M_DEBUG(1, ("BEGIN serialize_by_groups: groups=%x, op=%s, medium=%s",
              groups, op_captions[op], (*med->get_medium_identification)(med)));

    SAL_ASSERT(med != NULL);

    /*
     * Do not open/close medium when loading factory default.
     * We rely on serializer itself to restore its default values.
     */
    if (op != SERIALIZE_OP_LOAD_DEFAULTS) {
        /* Open medium */
        _M_DEBUG(1, (" Open medium with write=%d",
                    (op == SERIALIZE_OP_SAVE ||
                     op == SERIALIZE_OP_SAVE_DEFAULTS)));
        index = (*med->open)(
                    op == SERIALIZE_OP_SAVE || op == SERIALIZE_OP_SAVE_DEFAULTS,
                    med
                    );
        if (index < 0) {
            return -1;
        }
    }
    pmedium = med;

    for(; ps->callback != NULL; ps++) {
        if ((ps->groups & med->get_valid_groups(med)) == 0) {
            /*
             * The serializer is not stored in this medium.
             * Thus we'll process it only if it's "loading defaults" and
             * the serializer supports SERIALIZE_OP_LOAD_DEFAULTS.
             */
            if (op != SERIALIZE_OP_LOAD_DEFAULTS || !ps->bdefaults) {
                continue;
            }
        }
        if (groups == 0 || /* Special case: 0 means all w/o checking mask */
            (ps->groups & groups)) {
            _M_DEBUG(2, (" Matched: %s", ps->name));
            current_op = op;
        } else {
            _M_DEBUG(2, (" Unmatched: %s", ps->name));
            if (ps->bdefaults && op == SERIALIZE_OP_LOAD_DEFAULTS) {
                continue;
            }
            current_op = SERIALIZE_OP_COUNT;
        }

        r = serialize_item(ps, index);
        if (r < 0) {
            error -= 1;
            if ((*med->medium_properties)(med) & SMEDIUM_PROP_COMPACT) {
                /* For compact medium, error cannot be skipped */
                break;
            }
        } else {
            index += r;
        }
    }

    if (op != SERIALIZE_OP_LOAD_DEFAULTS) {
        /* Close this medium */
        _M_DEBUG(1, (" Close medium"));
        if ((*med->close)(med) < 0) {
            error -= 1;
        }
    }

    _M_DEBUG(1, ("END serialize_by_groups: count=%d, error=%d", index, error));
    return error;
}

BOOL
serialize_by_name(const char *name, SERIALIZE_OP op, PERSISTENT_MEDIUM *med)
{
    int32 index = 0; /* It's actually used only for compact medium */
    int r;
    BOOL bFound = FALSE, bError = FALSE;
    SERIALIZE_S *ps = g_serializers;

    _M_DEBUG(1, ("BEGIN serialize_by_name: name=%s, op=%s, medium=%s",
              name, op_captions[op], (*med->get_medium_identification)(med)));

    SAL_ASSERT(name != NULL && name[0] != 0);
    SAL_ASSERT(med != NULL);

    /*
     * Do not open/close medium when loading factory default.
     * We rely on serializer itself to restore its default values.
     */
    if (op != SERIALIZE_OP_LOAD_DEFAULTS) {
        /* Open medium */
        _M_DEBUG(1, (" Open medium with write=%d",
                        (op == SERIALIZE_OP_SAVE ||
                         op == SERIALIZE_OP_SAVE_DEFAULTS)));
        index = (*med->open)(
                    op == SERIALIZE_OP_SAVE || op == SERIALIZE_OP_SAVE_DEFAULTS,
                    med
                    );
        if (index < 0) {
            return FALSE;
        }
    }

    pmedium = med;

    for(; ps->callback != NULL && !bFound; ps++) {

        if ((ps->groups & med->get_valid_groups(med)) == 0) {
            /*
             * The serializer is not stored in this medium.
             * Thus we'll process it only if it's "loading defaults" and
             * the serializer supports SERIALIZE_OP_LOAD_DEFAULTS.
             */
            if (op != SERIALIZE_OP_LOAD_DEFAULTS || !ps->bdefaults) {
                continue;
            }
        }

        if (!sal_strcmp(ps->name, name)) {
            _M_DEBUG(2, (" Matched: %s", ps->name));
            bFound = TRUE;
            current_op = op;
        } else {
            _M_DEBUG(2, (" Unmatched: %s", ps->name));
            if (ps->bdefaults && op == SERIALIZE_OP_LOAD_DEFAULTS) {
                continue;
            }
            current_op = SERIALIZE_OP_COUNT;
        }

        r = serialize_item(ps, index);

        if (r < 0) {
            if (((*med->medium_properties)(med) & SMEDIUM_PROP_COMPACT) ||
                bFound) {

                /* For compact medium, error cannot be skipped */
                bError = TRUE;
                break;
            }
        } else {
            index += r;
        }

        if(bFound) {
            break;
        }
    }

    if (op != SERIALIZE_OP_LOAD_DEFAULTS) {
        /* Close this medium */
        _M_DEBUG(1, (" Close medium"));
        if ((*med->close)(med) < 0) {
            bError = TRUE;
        }
    }
    _M_DEBUG(1, ("END serialize_by_name: count=%d, error=%d", index, bError));
    return !bError;
}

int32
serialize_medium_for_validation(PERSISTENT_MEDIUM *med, BOOL defaults)
{
    int32 index; /* It's actually used only for compact medium */
    int r;
    int32 error = 0; /* To count how many error occurred */
    SERIALIZE_S *ps = g_serializers;

    _M_DEBUG(1, ("BEGIN serialize_medium_for_validation: medium=%s",
              (*med->get_medium_identification)(med)));

    SAL_ASSERT(med != NULL);

    /* Open medium */
    _M_DEBUG(1, (" Open medium with write=%d", FALSE));
    index = (*med->open)(FALSE, med);
    if (index < 0) {
	_M_DEBUG(5,("open medium error"));
        return -1;
    }

    pmedium = med;

    for(; ps->callback != NULL; ps++) {
        if ((ps->groups & med->get_valid_groups(med)) == 0) {
            /* The serializer is not stored in this medium */
            continue;
        }

        _M_DEBUG(2, (" Validate: %s", ps->name));
        if (defaults) {
            current_op = SERIALIZE_OP_VALIDATE_DEFAULTS;
        } else {
            current_op = SERIALIZE_OP_VALIDATE;
        }

        r = serialize_item(ps, index);
        if (r < 0) {
            error -= 1;
            if ((*med->medium_properties)(med) & SMEDIUM_PROP_COMPACT) {
                /* For compact medium, error cannot be skipped */
                break;
            }
        } else {
            index += r;
        }
    }

    /* Check total size if applicable */
    if ((*med->medium_properties)(med) & SMEDIUM_PROP_GET_TOTAL_DATA_SIZE) {
        _M_DEBUG(1, (" Checking total size: %d", index));
        if ((*med->get_total_data_size)(med) != index) {
            _M_DEBUG(1, (" Total size not matched: %d", (*med->get_total_data_size)(med)));
            error -= 1;
        }
        _M_DEBUG(1, ("  OK"));
    }

    /* Close this medium */
    _M_DEBUG(1, (" Close medium"));
    if ((*med->close)(med) < 0) {
        error -= 1;
    }

    _M_DEBUG(1, ("END serialize_medium_for_validation: count=%d, error=%d",
              index, error));
    return error;
}
