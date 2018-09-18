/*
 * $Id: serialize.h,v 1.5 Broadcom SDK $
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

#ifndef _SERIALIZE_H_
#define _SERIALIZE_H_

#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
/* 
 * Medium functions for string-type serializer
 */
typedef struct {
    const char * (*read_string)(void) REENTRANT;
    BOOL (*write_string)(const char *str) REENTRANT;
    
    void *dummy1; /* Dummy entry for validation; Do not touch it. */
    void *dummy2; /* Dummy entry for validation; Do not touch it. */
    void *dummy3; /* Dummy entry for validation; Do not touch it. */
    void *dummy4; /* Dummy entry for validation; Do not touch it. */
    void *dummy5; /* Dummy entry for validation; Do not touch it. */
    void *dummy6; /* Dummy entry for validation; Do not touch it. */
} SERL_MEDIUM_STR;
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */

/* 
 * Medium functions for binary-type serializer
 */
typedef struct {

#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
    void *dummy1; /* Dummy entry for validation; Do not touch it. */
    void *dummy2; /* Dummy entry for validation; Do not touch it. */
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
    
    int32 (*read)(uint8 *buf, uint32 len) REENTRANT;
    int32 (*write)(uint8 *buf, uint32 len) REENTRANT;
    int32 (*read_uint32)(uint32 *pval) REENTRANT;
    int32 (*write_uint32)(uint32 val) REENTRANT;
    int32 (*read_uint16)(uint16 *pval) REENTRANT;
    int32 (*write_uint16)(uint16 val) REENTRANT;
} SERL_MEDIUM_BIN;

/*
 * Operations supported for a serializer.
 */
typedef enum {
    SERIALIZE_OP_COUNT = 0,         /* To get the size (bytes) of data */
    SERIALIZE_OP_LOAD,              /* To load data from persistent medium */
    SERIALIZE_OP_SAVE,              /* To save data to persistent medium */
    SERIALIZE_OP_VERSION,           /* To get version of this serializer */
    SERIALIZE_OP_LOAD_DEFAULTS,     /* To load factory defaults */
    SERIALIZE_OP_SAVE_DEFAULTS,     /* Used only in serialization level */
    SERIALIZE_OP_VALIDATE,          /* Used internally */
    SERIALIZE_OP_VALIDATE_DEFAULTS  /* Used internally */
} SERIALIZE_OP;

/*
 * Serializer version: (16 bit)
 *      The value returned from any serializer fro SERIALIZE_OP_VERSION
 *      must contain this magic number as upper word.
 *      This is to avoid developer error for not returning version number.
 */
#define SERIALIZE_VERSION_MASK  (0x0000FFFF)
#define SERIALIZE_VERSION_MAGIC (0x88740000)

/*
 * Convenient macro for making up a valid serializer version
 *      that can be used in the implementation of serializer
 */
#define MAKE_SERIALIZER_VERSION(x) \
        (int32)((x & SERIALIZE_VERSION_MASK) | SERIALIZE_VERSION_MAGIC)

#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
/* 
 * Prototype for string-type serializer
 *   parameters:
 *        op - SERIALIZE_OP_COUNT, 
 *             SERIALIZE_OP_LOAD, 
 *             SERIALIZE_OP_SAVE, or
 *             SERIALIZE_OP_VERSION
 *        medium - use it to load/save string
 *   return:
 *        If op is SERIALIZE_OP_VERSION, the return value is a valid serializer
 *        version containing SERIALIZE_VERSION_MAGIC magic number; Otherwise, 
 *        it's the max string length (excluding string terminator);
 *        (Used mainly for compact medium to reserve space)
 *        return -1 if error.
 */
typedef int32 (*SERIALIZER_STR)(SERIALIZE_OP op, SERL_MEDIUM_STR *medium) REENTRANT;
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */

/* 
 * Prototype for binary-type serializer
 *   parameters:
 *        op - SERIALIZE_OP_COUNT, 
 *             SERIALIZE_OP_LOAD, 
 *             SERIALIZE_OP_SAVE, or
 *             SERIALIZE_OP_VERSION
 *        medium - use it to read/write data from/to
 *   Return value:
 *        If op is SERIALIZE_OP_VERSION, the return value is a valid serializer
 *        version containing SERIALIZE_VERSION_MAGIC magic number; Otherwise, 
 *        it's number of bytes required (for SERIALIZE_OP_COUNT) or 
 *        processed (for SERIALIZE_OP_LOAD or SERIALIZE_OP_SAVE).
 *        return -1 if error.
 */
typedef int32 (*SERIALIZER_BIN)(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;

#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
/*
 * register_string_serializer(): register string-type serializer
 *   Parameters:
 *        name - friendly name of this serializer
 *        groups - 16-bit bitmap of groups this serializer belong to
 *        func - serializer function
 *        defaults - whether it supports SERIALIZE_OP_LOAD_DEFAULTS or not
 *   Return value:
 *        true if succeeded; false otherwise.
 */
BOOL register_string_serializer(
        const char *name, 
        uint32 groups, 
        SERIALIZER_STR func,
        BOOL defaults
        );
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */

/*
 * register_binary_serializer(): register binary-type serializer
 *   Parameters:
 *        name - friendly name of this serializer
 *        groups - 16-bit bitmap of groups this serializer belong to
 *        func - serializer function
 *        defaults - whether it supports SERIALIZE_OP_LOAD_DEFAULTS or not
 *   Return value:
 *        true if succeeded; false otherwise.
 */
BOOL register_binary_serializer(
        const char *name, 
        uint32 groups, 
        SERIALIZER_BIN func,
        BOOL defaults
        );

#endif /* _SERIALIZE_H_ */
