/*
 * $Id: hndrte_armtrap.h,v 1.5 Broadcom SDK $
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

#ifndef _hndrte_armtrap_h
#define _hndrte_armtrap_h


/* ARM trap handling */

/* Trap types defined by ARM (see arminc.h) */

/* Trap locations in lo memory */
#define TRAP_STRIDE 4
#define FIRST_TRAP  TR_RST
#define LAST_TRAP   (TR_FIQ * TRAP_STRIDE)

#if defined(__ARM_ARCH_4T__)
#define MAX_TRAP_TYPE   (TR_FIQ + 1)
#elif defined(__ARM_ARCH_7M__)
#define MAX_TRAP_TYPE   (TR_ISR + ARMCM3_NUMINTS)
#endif  /* __ARM_ARCH_7M__ */

/* The trap structure is defined here as offsets for assembly */
#define TR_TYPE     0x00
#define TR_EPC      0x04
#define TR_CPSR     0x08
#define TR_SPSR     0x0c
#define TR_REGS     0x10
#define TR_REG(n)   (TR_REGS + (n) * 4)
#define TR_SP       TR_REG(13)
#define TR_LR       TR_REG(14)
#define TR_PC       TR_REG(15)

#define TRAP_T_SIZE 80

#ifndef __ASSEMBLER__
typedef struct _trap_struct {
    uint32      type;
    uint32      epc;
    uint32      cpsr;
    uint32      spsr;
    uint32      r0; /* a1 */
    uint32      r1; /* a2 */
    uint32      r2; /* a3 */
    uint32      r3; /* a4 */
    uint32      r4; /* v1 */
    uint32      r5; /* v2 */
    uint32      r6; /* v3 */
    uint32      r7; /* v4 */
    uint32      r8; /* v5 */
    uint32      r9; /* sb/v6 */
    uint32      r10;    /* sl/v7 */
    uint32      r11;    /* fp/v8 */
    uint32      r12;    /* ip */
    uint32      r13;    /* sp */
    uint32      r14;    /* lr */
    uint32      pc; /* r15 */
} trap_t;
#endif  /* !__ASSEMBLY__ */

#endif  /* _hndrte_armtrap_h */
