/*
 * $Id: cache.S,v 1.1 Broadcom SDK $
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
 * File:    cache.S
 * Purpose: cache support
 */

#include "thumb.h"        
#include "board.h"
#include "arm.h"        

        .syntax unified
        .code   32

//
// Issue a membar
//
        .global membar
membar:
        mov     r0, #0
        mcr     p15, 0, r0, c7, c10, 4
        mcr     p15, 0, r0, c7, c10, 5
        bx      lr

//
// Routine to flush dcache
//   r0 - start addr
//   r1 - length
        .global dcache_flush
dcache_flush:
        adds    r1,r0,r1        // Compute the end addr
        and     r0,r0,CACHELINE_MASK
1:
        mcr     p15,0,r0,c7,c11,1
        add     r0,r0,CACHELINE_SIZE
        cmp     r0,r1
        blt     1b
        mcr     p15,0,r0,c7,c10,5       // mem bar
        bx      lr
        
//
// Routine to invalidate dcache
//   r0 - start addr
//   r1 - length
        .global dcache_invalidate
dcache_invalidate:
        adds    r1,r0,r1        // Compute the end addr
        and     r0,r0,CACHELINE_MASK
1:
        mcr     p15,0,r0,c7,c6,1
        add     r0,r0,CACHELINE_SIZE
        cmp     r0,r1
        blt     1b
        mcr     p15,0,r0,c7,c10,5       // mem bar
        bx      lr
        
//
// Routine to flush and invalidate dcache
//   r0 - start addr
//   r1 - length
        .global dcache_flush_and_inv
dcache_flush_and_inv:
        adds    r1,r0,r1        // Compute the end addr
        and     r0,r0,CACHELINE_MASK
1:
        mcr     p15,0,r0,c7,c14,1
        add     r0,r0,CACHELINE_SIZE
        cmp     r0,r1
        blt     1b
        mcr     p15,0,r0,c7,c10,5       // mem bar
        bx      lr
#if 0
//
// Routine to flush and invalidate the entire dcache
//
        .global dcache_flush_and_inv_all
dcache_flush_and_inv_all:
	ldr	r0,=system_baseaddr
	ldr	r1,=system_endaddr
1:
        mcr     p15,0,r0,c7,c14,1
        add     r0,r0,CACHELINE_SIZE
        cmp     r0,r1
        blt     1b
        mcr     p15,0,r0,c7,c10,5       // mem bar
        bx      lr
	
//
// Routine to flush the entire dcache
//
        .global dcache_flush_all
dcache_flush_all:
	ldr	r0,=system_baseaddr
	ldr	r1,=system_endaddr
1:
        mcr     p15,0,r0,c7,c10,1
        add     r0,r0,CACHELINE_SIZE
        cmp     r0,r1
        blt     1b
        mcr     p15,0,r0,c7,c10,5       // mem bar
        bx      lr
	
//
// Routine to invalidate the entire dcache
//
        .global dcache_inv_all
dcache_inv_all:
	ldr	r0,=system_baseaddr
	ldr	r1,=system_endaddr
1:
        mcr     p15,0,r0,c7,c6,1
        add     r0,r0,CACHELINE_SIZE
        cmp     r0,r1
        blt     1b
        mcr     p15,0,r0,c7,c10,5       // mem bar
        bx      lr
#endif

//
// Routine to invalidate icache
//   r0 - start addr
//   r1 - length
        .global icache_invalidate
icache_invalidate:
        adds    r1,r0,r1        // Compute the end addr
1:
        mcr     p15,0,r0,c7,c5,1
        add     r0,r0,CACHELINE_SIZE
        cmp     r0,r1
        blt     1b
        mcr     p15,0,r0,c7,c10,5       // mem bar
#ifdef CORTEX_R4
        mcr     p15,0,r0,c7,c5,6        // Invalidate branch predictor array
        mcr     p15,0,r0,c7,c5,4        // Flush prefetch buffer
        mcr     p15,0,r0,c7,c10,5       // mem bar
#endif
        bx      lr
