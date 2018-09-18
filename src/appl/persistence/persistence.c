/*
 * $Id: persistence.c,v 1.9 Broadcom SDK $
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
#include "board.h"
#include "appl/serialize.h"
#include "serialize/serialization.h" 
#include "appl/persistence.h"

#include "media/flash/flash_medium.h"
#include "media/ramtxt/ramtxt_medium.h"

#if CFG_PERSISTENCE_SUPPORT_ENABLED

/* Persistent-data-compatible versions */
const char * CODE um_compatible_versions[] = { "all", NULL };

static MEDIUM_FLASH medium_current;

/* We use this definition to keep the medium flexible */
#define MED_CURRENT_SETTING  ((PERSISTENT_MEDIUM *)&medium_current)

void
persistence_init(void)
{
    extern void serializers_init(void);
    
#ifdef CFG_NVRAM_SUPPORT_INCLUDED
//    nvram_medium_initialize(&medium_current, NULL, ALL_CURRENT_SETTINGS);
#endif

    flash_medium_initialize(&medium_current, 
                            MEDIUM_FLASH_START_ADDRESS, ALL_CURRENT_SETTINGS);


    serializers_init();
}

BOOL
persistence_load_all_current_settings(void)
{
    return persistence_load_group_current_settings(ALL_CURRENT_SETTINGS);
}

BOOL
persistence_save_all_current_settings(void)
{
    return persistence_save_group_current_settings(ALL_CURRENT_SETTINGS);
}

BOOL
persistence_load_current_settings(const char *name)
{
    if (serialize_by_name(name, SERIALIZE_OP_LOAD, MED_CURRENT_SETTING)) {
        return TRUE;
    }
    return FALSE;
}

BOOL
persistence_load_group_current_settings(uint32 groups)
{
    if (serialize_by_groups(
            groups, 
            SERIALIZE_OP_LOAD, 
            MED_CURRENT_SETTING) < 0) {
        return FALSE;
    }

    return TRUE;
}

BOOL
persistence_save_current_settings(const char *name)
{
    if (serialize_by_name(name, SERIALIZE_OP_SAVE, MED_CURRENT_SETTING)) {
        return TRUE;
    }
    
    return FALSE;
}

BOOL
persistence_save_group_current_settings(uint32 groups)
{
    if (serialize_by_groups(
            groups, 
            SERIALIZE_OP_SAVE, 
            MED_CURRENT_SETTING) < 0) {

        return FALSE;
    }
    
    return TRUE;
}

BOOL
persistence_validate_current_settings(void)
{
    if (serialize_medium_for_validation(MED_CURRENT_SETTING, FALSE) < 0) {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
persistence_restore_factory_defaults(void)
{
    if (serialize_by_groups(
            ALL_FACTORY_DEFAULTS, 
            SERIALIZE_OP_LOAD_DEFAULTS, 
            MED_CURRENT_SETTING) < 0) {
        return FALSE;
    }
         
    return TRUE;
}

BOOL
persistence_restore_factory_defaults_by_name(const char *name)
{
    if (serialize_by_name(
                name, 
                SERIALIZE_OP_LOAD_DEFAULTS, 
                MED_CURRENT_SETTING)) {
        return TRUE;
    }
    
    return FALSE;
}

BOOL
persistence_restore_factory_defaults_by_group(uint32 groups)
{
    if (serialize_by_groups(
            groups, 
            SERIALIZE_OP_LOAD_DEFAULTS, 
            MED_CURRENT_SETTING) < 0) {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
persistence_backup_to_memory(void *buffer)
{
    MEDIUM_RAMTXT medium;

    if (buffer == NULL) {
        return FALSE;
    }

    ramtxt_medium_initalize(&medium, buffer);
    if (serialize_by_groups(
            ALL_BACKUP_RESTORE, 
            SERIALIZE_OP_SAVE, 
            (PERSISTENT_MEDIUM *)&medium) < 0) {
        return FALSE;
    }

    return TRUE;
}

BOOL
persistence_restore_from_memory(void *buffer)
{
    MEDIUM_RAMTXT medium;

    if (buffer == NULL) {
        return FALSE;
    }

    ramtxt_medium_initalize(&medium, buffer);
    if (serialize_by_groups(
            ALL_BACKUP_RESTORE, 
            SERIALIZE_OP_LOAD, 
            (PERSISTENT_MEDIUM *)&medium) < 0) {
        return FALSE;
    }

    return TRUE;
}

BOOL
persistence_validate_data_in_memory(void *buffer)
{
    MEDIUM_RAMTXT medium;

    if (buffer == NULL) {
        return FALSE;
    }

    ramtxt_medium_initalize(&medium, buffer);
    if (serialize_medium_for_validation((PERSISTENT_MEDIUM *)&medium, FALSE) < 0) {
        return FALSE;
    }

    return TRUE;
}

#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */
