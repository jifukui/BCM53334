/*
 * $Id: exception.h,v 1.5 Broadcom SDK $
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
 * This module describes the exception handlers, exception
 * trap frames, and dispatch.
 *
 */

#ifdef __ASSEMBLER__
#define _XAIDX(x)   (8*(x))
#else
#define _XAIDX(x)   (x)
#endif


/*  *********************************************************************
    *  Exception vectors from the MIPS specification
    ********************************************************************* */

#define MIPS_ROM_VEC_RESET  0x0000
#define MIPS_ROM_VEC_TLBFILL    0x0200
#define MIPS_ROM_VEC_XTLBFILL   0x0280
#define MIPS_ROM_VEC_CACHEERR   0x0300
#define MIPS_ROM_VEC_EXCEPTION  0x0380
#define MIPS_ROM_VEC_INTERRUPT  0x0400
#define MIPS_ROM_VEC_EJTAG  0x0480

#define MIPS_RAM_VEC_TLBFILL    0x0000
#define MIPS_RAM_VEC_XTLBFILL   0x0080
#define MIPS_RAM_VEC_EXCEPTION  0x0180
#define MIPS_RAM_VEC_INTERRUPT  0x0200
#define MIPS_RAM_VEC_CACHEERR   0x0100
#define MIPS_RAM_VEC_END    0x0300

#define MIPS_RAM_EXL_VEC_TLBFILL  0x0100
#define MIPS_RAM_EXL_VEC_XTLBFILL 0x0180


/*  *********************************************************************
    *  Fixed locations of other low-memory objects.  We stuff some
    *  important data in the spaces between the vectors.
    ********************************************************************* */

#define CFE_LOCORE_GLOBAL_GP    0x0040      /* our "handle" */
#define CFE_LOCORE_GLOBAL_SP    0x0048      /* Stack pointer for exceptions */
#define CFE_LOCORE_GLOBAL_K0TMP 0x0050      /* Used by cache error handler */
#define CFE_LOCORE_GLOBAL_K1TMP 0x0058      /* Used by cache error handler */
#define CFE_LOCORE_GLOBAL_RATMP 0x0060      /* Used by cache error handler */
#define CFE_LOCORE_GLOBAL_GPTMP 0x0068      /* Used by cache error handler */
#define CFE_LOCORE_GLOBAL_CERRH 0x0070      /* Pointer to cache error handler */

#define CFE_LOCORE_GLOBAL_T0TMP 0x0240      /* used by cache error handler */
#define CFE_LOCORE_GLOBAL_T1TMP 0x0248      /* used by cache error handler */
#define CFE_LOCORE_GLOBAL_T2TMP 0x0250      /* used by cache error handler */
#define CFE_LOCORE_GLOBAL_T3TMP 0x0258      /* used by cache error handler */

/*  *********************************************************************
    *  Offsets into our exception handler table.
    ********************************************************************* */

#define XTYPE_RESET 0
#define XTYPE_TLBFILL   8
#define XTYPE_XTLBFILL  16
#define XTYPE_CACHEERR  24
#define XTYPE_EXCEPTION 32
#define XTYPE_INTERRUPT 40
#define XTYPE_EJTAG 48

/*  *********************************************************************
    *  Exception frame definitions.
    ********************************************************************* */

/*
 * The exception frame is divided up into pieces, representing the different
 * parts of the processor that the data comes from:
 *
 * CP0:     Words 0..7
 * Int Regs:    Words 8..41
 * FP Regs:     Words 42..73
 * Total size:  74 words
 */

#define EXCEPTION_SIZE  _XAIDX(74)

#define XCP0_BASE   0
#define XGR_BASE    8
#define XFR_BASE    42

#define _XCP0IDX(x) _XAIDX((x)+XCP0_BASE)
#define XCP0_SR     _XCP0IDX(0)
#define XCP0_CAUSE  _XCP0IDX(1)
#define XCP0_EPC    _XCP0IDX(2)
#define XCP0_VADDR  _XCP0IDX(3)
#define XCP0_PRID   _XCP0IDX(4)

#define _XGRIDX(x)  _XAIDX((x)+XGR_BASE)
#define XGR_ZERO        _XGRIDX(0)
#define XGR_AT      _XGRIDX(1)
#define XGR_V0      _XGRIDX(2)
#define XGR_V1      _XGRIDX(3)
#define XGR_A0      _XGRIDX(4)
#define XGR_A1      _XGRIDX(5)
#define XGR_A2      _XGRIDX(6)
#define XGR_A3      _XGRIDX(7)
#define XGR_T0      _XGRIDX(8)
#define XGR_T1      _XGRIDX(9)
#define XGR_T2      _XGRIDX(10)
#define XGR_T3      _XGRIDX(11)
#define XGR_T4      _XGRIDX(12)
#define XGR_T5      _XGRIDX(13)
#define XGR_T6      _XGRIDX(14)
#define XGR_T7      _XGRIDX(15)
#define XGR_S0      _XGRIDX(16)
#define XGR_S1      _XGRIDX(17)
#define XGR_S2      _XGRIDX(18)
#define XGR_S3      _XGRIDX(19)
#define XGR_S4      _XGRIDX(20)
#define XGR_S5      _XGRIDX(21)
#define XGR_S6      _XGRIDX(22)
#define XGR_S7      _XGRIDX(23)
#define XGR_T8      _XGRIDX(24)
#define XGR_T9      _XGRIDX(25)
#define XGR_K0      _XGRIDX(26)
#define XGR_K1      _XGRIDX(27)
#define XGR_GP      _XGRIDX(28)
#define XGR_SP      _XGRIDX(29)
#define XGR_FP      _XGRIDX(30)
#define XGR_RA      _XGRIDX(31)
#define XGR_LO      _XGRIDX(32)
#define XGR_HI      _XGRIDX(33)


#define _XFRIDX(x)  _XAIDX((x)+XFR_BASE)
#define XR_F0       _XFRIDX(0)
#define XR_F1       _XFRIDX(1)
#define XR_F2       _XFRIDX(2)
#define XR_F3       _XFRIDX(3)
#define XR_F4       _XFRIDX(4)
#define XR_F5       _XFRIDX(5)
#define XR_F6       _XFRIDX(6)
#define XR_F7       _XFRIDX(7)
#define XR_F8       _XFRIDX(8)
#define XR_F9       _XFRIDX(9)
#define XR_F10      _XFRIDX(10)
#define XR_F11      _XFRIDX(11)
#define XR_F12      _XFRIDX(12)
#define XR_F13      _XFRIDX(13)
#define XR_F14      _XFRIDX(14)
#define XR_F15      _XFRIDX(15)
#define XR_F16      _XFRIDX(16)
#define XR_F17      _XFRIDX(17)
#define XR_F18      _XFRIDX(18)
#define XR_F19      _XFRIDX(19)
#define XR_F20      _XFRIDX(20)
#define XR_F21      _XFRIDX(21)
#define XR_F22      _XFRIDX(22)
#define XR_F23      _XFRIDX(23)
#define XR_F24      _XFRIDX(24)
#define XR_F25      _XFRIDX(25)
#define XR_F26      _XFRIDX(26)
#define XR_F27      _XFRIDX(27)
#define XR_F28      _XFRIDX(28)
#define XR_F29      _XFRIDX(29)
#define XR_F30      _XFRIDX(30)
#define XR_F31      _XFRIDX(31)
#define XR_FCR      _XFRIDX(32)
#define XR_FID      _XFRIDX(33)


#ifndef __ASSEMBLER__

#if 0
#include <typedefs.h>
#endif
#include <hndrte_armtrap.h>

extern void _exc_setvector(int vectype, void *vecaddr);
extern void _exc_crash_sim(void);
extern void _exc_cache_crash_sim(void);
extern void _exc_restart(void);
extern void _exc_clear_sr_exl(void);
extern void _exc_clear_sr_erl(void);

extern uint32 hndrte_set_trap(uint32 hook);
extern void hndrte_die(uint32 line);

//void cfe_exception(int code,uint64_t *info);
void um_exception(trap_t *tr);

void um_setup_exceptions(void);

#endif





