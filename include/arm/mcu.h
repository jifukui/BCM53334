/*
 * $Id: mcu.h,v 1.4 Broadcom SDK $
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

#ifndef _MCU_H_
#define _MCU_H_

/* For declaring re-usable API functions */
#define APIFUNC(_fn)    _fn
#define REENTRANT
#define USE_INTERNAL_RAM

/* For re-usable static function  */
#define APISTATIC

/* Normal static attribute */
#define STATICFN        static
#define STATIC          static
#define STATICCBK       static

/* Add "const" declaration to web related generated files from sspgen.pl
 * so those will be put in .rodata section and reduce RAM usage.
 */
#define RES_CONST_DECL const

#define XDATA
#define CODE

/* System register read/write */
#define SYS_REG_READ8(reg)       \
            (*(volatile uint8 *)(reg))
#define SYS_REG_WRITE8(reg,val)  \
            do { *(volatile uint8 *)(reg) = (val); } while(0)
#define SYS_REG_AND8(reg,val)    \
            SYS_REG_WRITE8((reg), SYS_REG_READ8(reg) & (val))
#define SYS_REG_OR8(reg,val)     \
            SYS_REG_WRITE8((reg), SYS_REG_READ8(reg) | (val))
#define SYS_REG_READ16(reg)      \
            (*(volatile uint16 *)(reg))
#define SYS_REG_WRITE16(reg,val) \
            do { *(volatile uint16 *)(reg) = (val); } while(0)
#define SYS_REG_AND16(reg,val)   \
            SYS_REG_WRITE16((reg), SYS_REG_READ16(reg) & (val))
#define SYS_REG_OR16(reg,val)    \
            SYS_REG_WRITE16((reg), SYS_REG_READ16(reg) | (val))
#define SYS_REG_READ32(reg)      \
            (*(volatile uint32 *)(reg))
#define SYS_REG_WRITE32(reg,val) \
            do { *(volatile uint32 *)(reg) = (val); } while(0)
#define SYS_REG_AND32(reg,val)   \
            SYS_REG_WRITE32((reg), SYS_REG_READ32(reg) & (val))
#define SYS_REG_OR32(reg,val)    \
            SYS_REG_WRITE32((reg), SYS_REG_READ32(reg) | (val))

/* Pointer to address conversion for hyper space (flash space) */
typedef uint32 hsaddr_t;
#define DATAPTR2HSADDR(x)   ((hsaddr_t)(x))
#define HSADDR2DATAPTR(x)   ((uint8 *)((hsaddr_t)(x)))

/* Pointer to address conversion for memory space (data space) */
typedef uint32 msaddr_t;
#define DATAPTR2MSADDR(x)   ((msaddr_t)(x))
#define MSADDR2DATAPTR(x)   ((uint8 XDATA *)((msaddr_t)(x)))

/* Type of system ticks */
typedef uint32 tick_t;

#endif /* _MCU_H_ */
