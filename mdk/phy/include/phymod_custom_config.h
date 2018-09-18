/*
 * $Id: phymod_custom_config.h,v 1.5 Broadcom SDK $
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

#ifndef __PHYMOD_CUSTOM_CONFIG_H__
#define __PHYMOD_CUSTOM_CONFIG_H__

#include <cdk/cdk_debug.h>
#include <phy/phy_reg.h>

/* No need to define */
#define PHYMOD_CONFIG_DEFINE_UINT8_T        0
#define PHYMOD_CONFIG_DEFINE_UINT16_T       0
#define PHYMOD_CONFIG_DEFINE_UINT32_T       0
#define PHYMOD_CONFIG_DEFINE_PRIu32         0
#define PHYMOD_CONFIG_DEFINE_PRIx32         0
#define PHYMOD_CONFIG_DEFINE_SIZE_T         0

#define PHYMOD_DEBUG_ERROR(stuff_)          do { CDK_ERR(stuff_); } while(0)
#define PHYMOD_DEBUG_VERBOSE(stuff_)        do { CDK_VERB(stuff_); } while(0)
#define PHYMOD_DIAG_OUT(stuff_)             do { CDK_WARN(stuff_); } while(0)

#define PHYMOD_USLEEP                       phymod_udelay
#define PHYMOD_SLEEP                        my_sleep
#define PHYMOD_MALLOC                       my_malloc
#define PHYMOD_FREE                         my_free

/* These functions map directly to Standard C functions */
#include <cdk/cdk_string.h>
#define PHYMOD_STRCMP                       CDK_STRCMP
#define PHYMOD_MEMSET                       CDK_MEMSET
#define PHYMOD_MEMCPY                       CDK_MEMCPY
#define PHYMOD_STRNCMP                      CDK_STRNCMP
#define PHYMOD_STRCHR                       CDK_STRCHR
#define PHYMOD_STRSTR                       CDK_STRSTR
#define PHYMOD_STRLEN                       CDK_STRLEN
#define PHYMOD_STRCAT                       CDK_STRCAT
#define PHYMOD_STRNCAT(_s1, _s2, _n)        CDK_STRCAT(_s1, _s2)
#define PHYMOD_STRCPY                       CDK_STRCPY
#define PHYMOD_STRNCPY                      CDK_STRNCPY
#include <cdk/cdk_stdlib.h>
#define PHYMOD_STRTOUL                      CDK_STRTOUL
#define PHYMOD_ABS                          CDK_ABS
#include <cdk/cdk_printf.h>
#define PHYMOD_SPRINTF                      CDK_SPRINTF
#define PHYMOD_SNPRINTF                     CDK_SNPRINTF

#define strncpy                             CDK_STRNCPY
#define strlen                              CDK_STRLEN
#define uint32                              uint32_t

/* Definiations for PHYMOD usage */
#define int8_t                              int
#define int16_t                             int
#define int32_t                             int

#endif /* __PHYMOD_CUSTOM_CONFIG_H__ */
