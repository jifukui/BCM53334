/*
 * $Id: segtable.h,v 1.5 Broadcom SDK $
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
 * The 'segment table' (bad name) is just a list of addresses
 * of important stuff used during initialization.  We use these
 * indirections to make life less complicated during code
 * relocation.
 *
 */

#if !defined(__ASSEMBLER__)
#define _TBLIDX(x)   (x)        /* C handles indexing for us */
typedef long segtable_t;        /* 32 for long32, 64 for long64 */
#endif

/*
 * Definitions for the segment_table
 */

#define R_SEG_ETEXT      _TBLIDX(0)     /* end of text segment */
#define R_SEG_FDATA      _TBLIDX(1)     /* Beginning of data segment */
#define R_SEG_EDATA      _TBLIDX(2)     /* end of data segment */
#define R_SEG_END        _TBLIDX(3)     /* End of BSS */
#define R_SEG_FTEXT          _TBLIDX(4)     /* Beginning of text segment */
#define R_SEG_FBSS           _TBLIDX(5)     /* Beginning of BSS */
#define R_SEG_GP         _TBLIDX(6)     /* Global Pointer */
#define R_SEG_RELOCSTART     _TBLIDX(7)     /* Start of reloc table */
#define R_SEG_RELOCEND       _TBLIDX(8)     /* End of reloc table */
#define R_SEG_APIENTRY       _TBLIDX(9)     /* API Entry address */

/*
 * Definitions for the init_table
 */

#define R_INIT_EARLYINIT     _TBLIDX(0)     /* pointer to board_earlyinit */
#define R_INIT_SETLEDS       _TBLIDX(1)     /* pointer to board_setleds */
#define R_INIT_DRAMINFO      _TBLIDX(2)     /* pointer to board_draminfo */
#define R_INIT_CPUINIT       _TBLIDX(3)     /* pointer tp cpuinit */
#define R_INIT_ALTCPU_START1 _TBLIDX(4)     /* pointer to altcpu_start1 */
#define R_INIT_ALTCPU_START2 _TBLIDX(5)     /* pointer to altcpu_start2 */
#define R_INIT_ALTCPU_RESET  _TBLIDX(6)     /* pointer to altcpu_reset */
#define R_INIT_CPURESTART    _TBLIDX(7)     /* pointer to cpurestart */
#define R_INIT_DRAMINIT      _TBLIDX(8)     /* pointer to draminit */
#define R_INIT_CACHEOPS      _TBLIDX(9)     /* pointer to cacheops */
#define R_INIT_TLBHANDLER    _TBLIDX(10)    /* pointer to TLB fault handler */
#define R_INIT_CMDSTART      _TBLIDX(11)    /* pointer to cfe_main */
#define R_INIT_CMDRESTART    _TBLIDX(12)    /* pointer to cfe_cmd_restart */
#define R_INIT_DOXREQ        _TBLIDX(13)    /* pointer to cfe_doxreq */


/*
 * Definitions for the diag_table
 */

#define R_DIAG_TEST1        _TBLIDX(0)      /* after CPU and cache init, before DRAM init */
#define R_DIAG_TEST2        _TBLIDX(1)      /* after DRAM init, before main */
