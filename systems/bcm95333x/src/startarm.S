/*
 * $Id: startarm.S,v 1.3 Broadcom SDK $
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
 * HNDRTE ARM run-time initialization and first level exception handling.
 *
 */

#ifndef	__arm__
#error __arm__ is NOT defined
#endif

	
/* Routine begin/end macro */
#if defined(__thumb__)
#define FUNC(x)	THUMBLEAF(x)
#else
#define FUNC(x)	LEAF(x)
#endif	/* __thumb__ */

	
/* Debug macro - write a number to memory - use it with caution,
 *  it changes r0 and r1 registers.
 */
#if defined(BCMDBG)
#define TRACE(x) \
	ldr	r0,=x; \
	ldr	r1,=__watermark; \
	str	r0,[r1]
#else
#define TRACE(x)
#endif	/* BCMDBG */

	
/* Instruction macros that hide machine/arch details. */
.macro _LSL_ rd,rm,im
	lsl \rd,\rm,\im
.endm
.macro _LSR_ rd,rm,im
	lsr \rd,\rm,\im
.endm
.macro _ROR_ rd,rm,im
	ror \rd,\rm,\im
.endm
.macro _MUL_ rd,rm,rs
	mov \rd,\rm
	mul \rd,\rd,\rs
.endm

#if defined(__ARM_ARCH_7R__)
#include "startarm-cr4.S"
#elif defined(__ARM_ARCH_7M__)
#include "startarm-cm3.S"
#elif defined(__ARM_ARCH_7A__)
#include "startarm-ca9.S"
#elif defined(__ARM_ARCH_4T__)
#include "startarm-7s.S"
#else
#error Unrecognized ARM architecture
#endif
