/*
 * $Id: medium.h,v 1.4 Broadcom SDK $
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

#ifndef _MEDIUM_H_
#define _MEDIUM_H_

/*
 * Capabilities/properties of a persistent medium
 */
enum {
    /* Compact form; no extra meta; no naming but can be seekable */
    SMEDIUM_PROP_COMPACT = 0x01,
    
    /* Can get (data) size of individual item */
    SMEDIUM_PROP_GET_ITEM_DATA_SIZE = 0x02,
    
    /* Can get (data) summed size of all items */
    SMEDIUM_PROP_GET_TOTAL_DATA_SIZE = 0x04,

    /* Can get serializer version of individual item */
    SMEDIUM_PROP_GET_ITEM_VERSION = 0x08
};

typedef struct _smedium_s {
    
    int32 (*open)(BOOL write, struct _smedium_s *ps) REENTRANT;
    int32 (*close)(struct _smedium_s *ps) REENTRANT;
    
    BOOL (*item_begin)(const char *name, struct _smedium_s *ps, int32 version) REENTRANT;
    BOOL (*item_end)(struct _smedium_s *ps) REENTRANT;
    int32 (*item_size)(struct _smedium_s *ps) REENTRANT;
    int32 (*item_version)(struct _smedium_s *ps) REENTRANT;
    
    int32 (*read)(uint32 len, uint8 *buf, struct _smedium_s *ps) REENTRANT;
    int32 (*write)(uint32 len, uint8 *buf, struct _smedium_s *ps) REENTRANT;
    
#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
    const char * (*read_string)(struct _smedium_s *ps) REENTRANT;
    BOOL (*write_string)(const char *str, struct _smedium_s *ps) REENTRANT;
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
    
    BOOL (*seek)(uint32 pos, struct _smedium_s *ps) REENTRANT;
    
    const char * (*get_medium_identification)(struct _smedium_s *ps) REENTRANT;
    uint32 (*medium_properties)(struct _smedium_s *ps) REENTRANT;
    int32 (*get_total_data_size)(struct _smedium_s *ps) REENTRANT;
    uint32 (*get_valid_groups)(struct _smedium_s *ps) REENTRANT;
    
} PERSISTENT_MEDIUM;

#endif /* _MEDIUM_H_ */
