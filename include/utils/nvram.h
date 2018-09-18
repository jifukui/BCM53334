/*
 * $Id: nvram.h,v 1.6 Broadcom SDK $
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

#ifndef _UTILS_NVRAM_H_
#define _UTILS_NVRAM_H_

/*  *********************************************************************
    *  Data structures.
    ********************************************************************* */

typedef struct nvram_header_s {
    uint32  magic;
    uint32  chksum;
    uint32  len;
    uint32  crc_ver_init;
} nvram_header_t;

#define NVRAM_VER_SHIFT        0
#define NVRAM_VER_MASK         (0xFF << NVRAM_VER_SHIFT)
#define NVRAM_CRC_SHIFT        8
#define NVRAM_CRC_MASK         (0xFF << NVRAM_CRC_SHIFT)
#define NVRAM_INIT_SHIFT       16
#define NVRAM_INIT_MASK        (0xFFF << NVRAM_INIT_SHIFT)


/*  *********************************************************************
    *  Constants
    ********************************************************************* */

#define NVRAM_MAGIC            0x48534C46     /* 'FLSH' (little-endian) */
#define NVRAM_VERSION          1

#ifdef CFG_CONFIG_BASE
#define NVRAM_BASE            CFG_CONFIG_BASE
#else
#define NVRAM_BASE            0xBFC0F000        /* The last sector */
#endif

#ifdef CFG_CONFIG_OFFSET
#define NVRAM_OFFSET            CFG_CONFIG_OFFSET
#else
#define NVRAM_OFFSET            0x0        /* The last sector */
#endif

#ifdef CFG_CONFIG_SIZE
#define NVRAM_SPACE            CFG_CONFIG_SIZE
#else
#define NVRAM_SPACE            0x1000         /* Header plus tuples */
#endif

/*  *********************************************************************
    *  Prototypes
    ********************************************************************* */

/* Initialize from non-volatile storage. */
sys_error_t nvram_init (void);

/* Lookup, add, delete variables. */
const char *nvram_get(const char *name);
sys_error_t nvram_set(const char *name, const char *value);
sys_error_t nvram_unset(const char *name);

/* Commit to non-volatile storage. */
sys_error_t nvram_commit(void);

/* Enumerate all tuples in arbitrary order.  Calls of set and unset
   have unpredictable effect. */
sys_error_t nvram_enum (sys_error_t (*proc)(const char *tuple));
sys_error_t nvram_show_tuple(const char *tuple);

#endif /* _UTILS_NVRAM_H_ */
