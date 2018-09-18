/*
 * $Id: startarm-ca9.S,v 1.7 Broadcom SDK $
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
 * BCM53XXX ARM/Cortext A9 boot code for standalone apps.
 *
 */

#include <arminc.h>
#include <armmacros.h>
#include <hndrte_armtrap.h>


#include "bsp_config.h"
#include "board.h"

#define c_main	main

	.section .sdata

#include "initdata.h"		/* declare variables we use here */


/*
 * This is the size of the stack, rounded to KByte boundaries.
 */

#ifndef CFG_STACK_SIZE
#error "CFG_STACK_SIZE not defined"
#else
#define STACK_SIZE	((CFG_STACK_SIZE+1023) & ~1023)
#endif

/* XXX: should move to some header files */
#define IHOST_PROC_CLK_WR_ACCESS			0x19000000
#define IHOST_PROC_CLK_POLICY_FREQ			0x19000008
#define IHOST_PROC_CLK_POLICY_CTL			0x1900000c
#define IHOST_PROC_CLK_POLICY_CTL__GO			0
#define IHOST_PROC_CLK_POLICY_CTL__GO_AC		1

#define IHOST_PROC_CLK_CORE0_CLKGATE			0x19000200
#define IHOST_PROC_CLK_CORE1_CLKGATE			0x19000204
#define IHOST_PROC_CLK_ARM_SWITCH_CLKGATE		0x19000210
#define IHOST_PROC_CLK_ARM_PERIPH_CLKGATE		0x19000300
#define IHOST_PROC_CLK_APB0_CLKGATE			0x19000400

#define IHOST_PROC_CLK_PLLARMA				0x19000c00
#define IHOST_PROC_CLK_PLLARMA__pllarm_soft_post_resetb	1
#define IHOST_PROC_CLK_PLLARMA__pllarm_lock		28

/* Entry point */
	.section .text

	.globl startup

startup:
	b	tr_rst	/* 0 - reset */
	ldr	pc,_tr_und	/* 4 - undefined instruction */
	ldr	pc,_tr_swi	/* 8 - software interrupt */
	ldr	pc,_tr_iab	/* 0xc - prefetch abort */
	ldr	pc,_tr_dab	/* 0x10 - data abort */
	ldr	pc,_tr_bad	/* 0x14 - reserved */
	ldr	pc,_tr_irq	/* 0x18 - external interrupt */
	ldr	pc,_tr_fiq	/* 0x1c - fast interrupt */

_tr_und:
	.word tr_und
_tr_swi:
	.word tr_swi
_tr_iab:
	.word tr_iab
_tr_dab:
	.word tr_dab
_tr_bad:
	.word tr_bad
_tr_irq:
	.word tr_irq
_tr_fiq:
	.word tr_fiq
_pad:
	.word 0x12345678 /* now 16*4=64 */

#if 0
	/* Record the size of the binary */
	.org	BISZ_OFFSET

	.word	BISZ_MAGIC
	.word	text_start
	.word	text_end
	.word	data_start
	.word	data_end
	.word	bss_start
	.word	bss_end
	.word	_end
#endif

#ifdef _CFE_
#if (CFG_EMBEDDED_PIC)
#define RUNTIME_RELOC_START	__runtime_reloc_start
#define RUNTIME_RELOC_STOP	__runtime_reloc_stop
#else
#define RUNTIME_RELOC_START	0
#define RUNTIME_RELOC_STOP	0
#endif

#include "segtable.h"

	.globl segment_table
segment_table:
	.word	text_end			/* [  0] End of text (R_SEG_ETEXT) */
	.word	data_start			/* [  1] Beginning of data (R_SEG_FDATA) */
	.word	data_end			/* [  2] End of data (R_SEG_EDATA) */
	.word	_end				/* [  3] End of BSS (R_SEG_END) */
	.word	text_start			/* [  4] Beginning of text (R_SEG_FTEXT) */
	.word	bss_start			/* [  5] Beginning of BSS (R_SEG_FBSS) */
	.word	0           		/* [  6] Global Pointer (R_SEG_GP) */
	.word	RUNTIME_RELOC_START	/* [  7] Beginning of reloc entries */
	.word	RUNTIME_RELOC_STOP	/* [  8] End of reloc entries */
	.word	0					/* [  9] R_SEG_APIENTRY */
#endif /* __CFE_ */

/*************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * setup Memory and board specific bits prior to relocation.
 * relocate armboot to ram
 * setup stack
 *
 *************************************************************************/

/*
 * the actual reset code
 */
tr_rst:
	mrs	r0,cpsr
	mov	r1,#(PS_MM)
	bic	r0,r0,r1
	/* Switch to Supervisor mode and make sure interrupts are disabled */
	mov 	r1,#(PS_I | PS_F | PS_SVC)
	orr	r0,r0,r1
	msr	cpsr,r0
#if (!CFG_RAMAPP)
initcpu:
	mov	r0,#0
	mcr	p15,0,r0,c8,c7,0		/* Invalidate TLB */
	mcr	p15,0,r0,c7,c5,0		/* Invalidate icache */
	/* Initialize system control register */
	mrc	p15,0,r0,c1,c0,0
	bic	r0,r0,#0x7			/* Clear C, A, M bits */
    bic	r0, r0, #0x2000	    /* Clear bits 13 (--V-) */
#if 0
	orr	r0,r0,#0x2000			/* Set V bit: high vectors (0xffff0000) */
#endif
	orr	r0,r0,#0x2			/* Set A bit: enable strict alignment */
	orr	r0,r0,#0x800			/* Set Z bit: enable program flow prediction */
#if 0
	orr	r0,r0,#0x1000			/* Set I bit: enable i$ */
	orr	r0,r0,#0x4			/* set C bit: enable d$ */
#endif
	mcr	p15,0,r0,c1,c0,0

    mov	ip, lr			/* persevere link reg across call */
	bl	l2cache_init    /* go setup pll,mux,memory        */
	mov	lr, ip			/* restore link                   */
#endif /* !CFG_RAMAPP */

#if 0
	/* Identify current CPU */
	mrc	p15,0,r0,c0,c0,5
	and	r0,r0,#0xf			/* retrieve cpu core id */
	cmp	r0,#0				/* cpu core 0? */
	beq	check_startaddr
	wfe

check_startaddr:
	adrl	r0,startup		/* r0: source start address */
	ldr	r1,=startup		/* r1: target address */
	cmp	r0,r1
	beq	szmem

#if !defined(CFG_ROMBOOT)	
set_cru_policy7:
	ldr	r0,=IHOST_PROC_CLK_WR_ACCESS
	ldr	r1,=0xa5a501
	str	r1,[r0]
	ldr	r0,=IHOST_PROC_CLK_PLLARMA
	ldr	r1,=0x1006001
	str	r1,[r0]
	ldr	r2,=0x0
	
	ldr	r4,=0x100000
poll_plllock:
	cmp	r4,r2
	beq	poll_plllock_done
	sub	r4,r4,#1
	ldr	r1,[r0]
	and	r3,r1,#(1 << IHOST_PROC_CLK_PLLARMA__pllarm_lock)
	cmp	r3,r2
	beq	poll_plllock
poll_plllock_done:
	orr	r1,r1,#(1 << IHOST_PROC_CLK_PLLARMA__pllarm_soft_post_resetb)
	str	r1,[r0]
	ldr	r0,=IHOST_PROC_CLK_POLICY_FREQ
	ldr	r1,=0x87070707
	str	r1,[r0]
	ldr	r0,=IHOST_PROC_CLK_CORE0_CLKGATE
	ldr	r1,=0x00000301
	str	r1,[r0]
	ldr	r0,=IHOST_PROC_CLK_CORE1_CLKGATE
	str	r1,[r0]
	ldr	r0,=IHOST_PROC_CLK_ARM_SWITCH_CLKGATE
	str	r1,[r0]
	ldr	r0,=IHOST_PROC_CLK_ARM_PERIPH_CLKGATE
	str	r1,[r0]
	ldr	r0,=IHOST_PROC_CLK_APB0_CLKGATE
	str	r1,[r0]

	ldr	r0,=IHOST_PROC_CLK_POLICY_CTL
	ldr	r2,=(1 << IHOST_PROC_CLK_POLICY_CTL__GO)
	ldr	r3,=(1 << IHOST_PROC_CLK_POLICY_CTL__GO_AC)
	orr	r3,r3,r2
	str	r3,[r0]
	ldr	r3,=0x0
	ldr	r4,=0x100000
poll_ccu:
	cmp	r4,r3
	beq	poll_ccu_done
	sub	r4,r4,#1
	ldr	r1,[r0]
	and	r1,r1,r2
	cmp	r1,r3
	bne	poll_ccu
poll_ccu_done:

#endif /* !defined(CFG_ROMBOOT) */

init_pcie:
	ldr	r0,=SI_ENUM_BASE		/* r0:	core regs SI base address */
	ldr	r1,=CC_EROMPTR
	ldr	r1,[r0,r1]			/* r1:	erom pointer */
1:	ldr	r2,[r1]				/* r2:	erom entry */
	ldr	r3,=(ER_TAG | ER_VALID)
	and	r3,r3,r2
	cmp	r3,#(ER_END | ER_VALID)		/* Is this the END marker? */
	beq	no_pcie				/* Yes: done and did not find pcie */

	cmp	r3,#(ER_CI | ER_VALID)
	beq	2f

	add	r1,r1,#4			/* Next erom entry */
	b	1b

no_pcie:
	b	find_bootdev
	
	/* Found a CIA */
2:	add	r1,r1,#8			/* Skip both CIA & CIB */
	ldr	r3,=CIA_CID_MASK
	and	r3,r3,r2
	ldr	r4,=(NS_PCIEG2_CORE_ID << CIA_CID_SHIFT)
	cmp	r3,r4
	bne	1b

	/* Found pcie, get regs and wrapper and save them */
	bl	ai_get_slave
	mov	r8,r0
	bl	ai_get_wrapper
	mov	r9,r0
	
	/* Initialize memory controller and size memory */
	mov	r0,r8
	mov	r1,r9
	bl	pcie_init
	bl	pcie_phy_init

find_bootdev:
	ldr	r0,=SI_ENUM_BASE		/* r0:	core regs SI base address */
	ldr	r1,=CC_EROMPTR
	ldr	r1,[r0,r1]			/* r1:	erom pointer */
1:	ldr	r2,[r1]				/* r2:	erom entry */
	ldr	r3,=(ER_TAG | ER_VALID)
	and	r3,r3,r2
	cmp	r3,#(ER_END | ER_VALID)		/* Is this the END marker? */
	beq	no_socrom_core			/* Yes: done and did not find rom core */

	cmp	r3,#(ER_CI | ER_VALID)
	beq	2f

	add	r1,r1,#4			/* Next erom entry */
	b	1b

no_socrom_core:
	b	init_ddr
	
	/* Found a CIA */
2:	add	r1,r1,#8			/* Skip both CIA & CIB */
	ldr	r3,=CIA_CID_MASK
	and	r3,r3,r2
	ldr	r4,=(NS_ROM_CORE_ID << CIA_CID_SHIFT)
	cmp	r3,r4
	bne	1b
	
	/* Found socrom, get regs and wrapper and save them */
	bl	ai_get_wrapper
	ldr	r1,[r0,#AI_IOSTATUS]
	ldr	r2,=SISF_NS_BOOTDEV_MASK
	and	r1,r1,r2
	mov	r7,r1
	ldr	r0,=SISF_NS_BOOTDEV_OFFLOAD
	cmp	r1,r0
	bne	init_ddr

offload_mode:
	ldr	r4,=0x1
	ldr	r5,=0xffff042c
	str	r4,[r5]
	ldr	r4,=0x0
	ldr	r5,=0xffff0430
wait_for_host:
	ldr	r6,[r5]
	cmp	r6,r4
	beq	wait_for_host
	bx	r6	

/*
 * Find ddr core
 */
init_ddr:
	ldr	r0,=SI_ENUM_BASE		/* r0:	core regs SI base address */
	ldr	r1,=CC_EROMPTR
	ldr	r1,[r0,r1]			/* r1:	erom pointer */
1:	ldr	r2,[r1]				/* r2:	erom entry */
	ldr	r3,=(ER_TAG | ER_VALID)
	and	r3,r3,r2
	cmp	r3,#(ER_END | ER_VALID)		/* Is this the END marker? */
	beq	panic				/* Yes: done and did not find ddr */

	cmp	r3,#(ER_CI | ER_VALID)
	beq	2f

	add	r1,r1,#4			/* Next erom entry */
	b	1b

panic:	b	.
	
	/* Found a CIA */
2:	add	r1,r1,#8			/* Skip both CIA & CIB */
	ldr	r3,=CIA_CID_MASK
	and	r3,r3,r2
	ldr	r4,=(NS_DDR23_CORE_ID << CIA_CID_SHIFT)
	cmp	r3,r4
	bne	1b

	/* Found ddr, get regs and wrapper and save them */
	bl	ai_get_slave
	mov	r8,r0
	bl	ai_get_wrapper
	mov	r9,r0
	
	/* Initialize memory controller and size memory */
	mov	r0,r8
	mov	r1,r9
	mov	r2,r7			/* boot_dev */
	bl	ddr_init
szmem:
	ldr	r3,=0x00000000
	ldr	r2,=0xaa55beef
	ldr	r1,=0x00000000
	str	r1,[r3,#0x3c]
	ldr	r0,=(1 << 20)
	ldr	r5,=(128 << 20)
1:	mov	r4,r0
	str	r2,[r4,#0x3c]
	ldr	r1,[r4,#0x3c]			/* Read back to ensure completion */
	ldr	r1,[r3,#0x3c]
	cmp	r1,r2
	beq	2f

	lsl	r0,r0,#1
	cmp	r0,r5
	bne	1b

	/* Didn't find an alias, must be 128MB */
2:
	mov	r8,r0			/* r8: the memory size in bytes */

        /* Copy to RAM */
copy2ram:
	adrl	r0,startup		/* r0: source start address */
	ldr	r1,=startup		/* r1: target address */
	cmp	r0,r1
	beq	clear_bss
	ldr	r3,_bss_start_ofs
	add	r2,r0,r3		/* r2: source end address */
copy_loop:
	ldmia	r0!,{r9-r10}		/* copy from source address [r0] */
	stmia	r1!,{r9-r10}		/* copy to   target address [r1] */
	cmp	r0,r2			/* until source end address [r2] */
	blo	copy_loop

	/* Clear BSS */
clear_bss:
	ldr	r0,_bss_start_ofs
	ldr	r1,_bss_end_ofs
	ldr	r4,=startup
	add	r0,r0,r4
	add	r1,r1,r4
	mov	r2,#0x00000000
	/* empty bss section checking */
	cmp	r0,r1
	beq	call_c_main

clbss_l:
	str	r2,[r0]
	add	r0,r0,#4
	cmp	r0,r1
	bne	clbss_l
#endif

#if (!CFG_RAMAPP)
/* Copy data into L2 cache */
copy2cache:
    ldr r0,_data_load_ofs       /* r0: source start address */
    ldr	r4,=startup
    add	r0,r0,r4
	ldr r1,=CFE_DATA_START	    /* r1: target address */
	ldr r2,_data_end_ofs
	add r2, r2, r4
copy2cache_loop:
	ldmia	r0!,{r9-r10}		/* copy from source address [r0] */
	stmia	r1!,{r9-r10}		/* copy to   target address [r1] */
	cmp	r1,r2			/* until source end address [r2] */
	blo	copy2cache_loop       
#endif /* !CFG_RAMAPP */

	/* Clear BSS */
clear_cache_bss:
	ldr	r0,_bss_start_ofs
	ldr	r4,=startup
	add	r0,r0,r4
	ldr	r1,_bss_end_ofs
	add	r1,r1,r4
	mov	r2,#0x00000000
	/* empty bss section checking */
	cmp	r0,r1
	beq	call_c_main

cl_cachebss_l:
	str	r2,[r0]
	add	r0,r0,#4
	cmp	r0,r1
	bne	cl_cachebss_l

/* Set stackpointer in internal RAM to call c_main */
call_c_main:
	TRACE(0x00000006)

	/* Save memory size in bytes */
	mov	r0, #0x20000
	ldr	r1,=_memsize
	str r0,[r1]

#ifdef _CFE_
	_LSR_ r0,r0,#10			/* memory size in Kbytes */
	ldr	r1,=mem_totalsize
	str	r0,[r1]

	/* not relocating, no offset */
	ldr	r0,=0
	ldr	r1,=mem_datareloc
	str	r0,[r1]
	ldr	r1,=mem_textreloc
	str	r0,[r1]

	ldr r0,_text_start_ofs
	add	r0,r0,r4
	ldr	r1,=mem_bottomofmem
	str	r0,[r1]

    ldr	r0,=CFG_DMA_HEAP_START
    ldr	r1,=mem_dmaheapstart
    str	r0,[r1]

    ldr	r0,=CFE_DATA_START
    add	r0,r0,#0x20000
    sub r0,r0,#(CFG_HEAP_SIZE*1024)
    ldr	r1,=mem_heapstart
	str	r0,[r1]

	add	r0,#(CFG_HEAP_SIZE*1024)
	ldr	r1,=mem_topofmem
	str	r0,[r1]

	ldr	r0,_text_start_ofs
	add	r0,r0,r4
	ldr	r2,_text_end_ofs
	add	r2,r2,r4
	sub	r2,r2,r0
	ldr	r1,=mem_textsize
	str	r2,[r1]
	ldr	r1,=mem_textbase
	str	r0,[r1]

	ldr	r0,=0x00000000	/* XXX */
	ldr	r1,=cpu_prid
	str	r0,[r1]
#if 0
	/* Set the segment_table */
	/* text_end */
	ldr	r0,_text_end_ofs
	add	r0,r0,r4
	ldr	r1,=segment_table
	str	r0,[r1]

	/* data_start */
	ldr	r0,_data_start_ofs
	add	r0,r0,r4
	add	r1,r1,#4
	str	r0,[r1]

	/* data_end */
	ldr	r0,_data_end_ofs
	add	r0,r0,r4
	add	r1,r1,#4
	str	r0,[r1]

	/* _end */
	ldr	r0,__end_ofs
	add	r0,r0,r4
	add	r1,r1,#4
	str	r0,[r1]

	/* text_start */
	ldr	r0,_text_start_ofs
	add	r0,r0,r4
	add	r1,r1,#4
	str	r0,[r1]

	/* bss_start */
	ldr	r0,_bss_start_ofs
	add	r0,r0,r4
	add	r1,r1,#4
	str	r0,[r1]

	/* _gp (0) */
	mov	r0,#0
	add	r1,r1,#4
	str	r0,[r1]

	/* RUNTIME_RELOC_START (0) */
	mov	r0,#0
	add	r1,r1,#4
	str	r0,[r1]

	/* RUNTIME_RELOC_STOP (0) */
	mov	r0,#0
	add	r1,r1,#4
	str	r0,[r1]

	/* cpu_apientry (0) */
	mov	r0,#0
	add	r1,r1,#4
	str	r0,[r1]
#endif
	ldr	r1,=mem_topofmem
#else /* !_CFE_ */
	ldr	r1,=_memsize
#endif /* _CFE_ */

#if 0
    ldr	r0,[r1]
    ldr	r0,=0x80020000
    sub r0,r0,#(CFG_HEAP_SIZE*1024)
#endif

    ldr	r1,=mem_heapstart
    ldr	r0,[r1]

    sub	r0,r0,#8
    mov	sp,r0
    bic	sp,sp,#7		/* 8-byte alignment for ABI compliance */
#if 0
    ldr	r0,_c_main_ofs
    ldr	r1,=startup
    add	lr, r0, r1
#endif
    ldr lr, =main
	/* setup parameters for c_main */
#ifdef _CFE_
	/* CFE: clear argument a & b in cfe_main */
	ldr	r0,=0x00000000
	ldr	r1,=0x00000000
#else
	/* CFEZ: set return addr to argment ra */
	ldr	r0,_theend_ofs
	add	r0,r0,r2
#endif
	/* jump to it ... */
	TRACE(0x00000007)
	mov	pc,lr

/* In case c_main returns */
theend:
	TRACE(0x424feedd)
	wfe
	b	theend

/*
 * These are defined in the board-specific linker script.
 */

#ifdef _CFE_

.globl _data_load_ofs
#if (!CFG_RAMAPP)
_data_load_ofs:
	.word data_start_load - startup
#endif
.globl _text_start_ofs
_text_start_ofs:
	.word text_start - startup

.globl _text_end_ofs
_text_end_ofs:
	.word text_end - startup

.globl _data_start_ofs
_data_start_ofs:
	.word data_start - startup

.globl _data_end_ofs
_data_end_ofs:
	.word data_end - startup

.globl __end_ofs
__end_ofs:
	.word _end - startup
#endif /* _CFE_ */

.globl _bss_start_ofs
_bss_start_ofs:
	.word bss_start - startup

.globl _bss_end_ofs
_bss_end_ofs:
	.word bss_end - startup

_c_main_ofs:
	.word c_main - startup

_theend_ofs:
	.word theend - startup

#if 0
/* Init PCIE mappings */
FUNC(pcie_init)
	ldr	r5,=REG_BAR2_CONFIG_F0
	str	r5,[r0,#PAXB_INDR_ADDR]
	ldr	r3,[r0,#PAXB_INDR_DATA]
	and	r3,r3,#0xfffffff0
	orr	r3,r3,#0xc
	str	r3,[r0,#PAXB_INDR_DATA]

	ldr	r3,[r0,#PAXB_IARR_1_LOWER]
	and	r3,r3,#0xffffff00
	orr	r3,r3,#0x80
	str	r3,[r0,#PAXB_IARR_1_LOWER]
	ldr	r3,=0x40000000
	str	r3,[r0,#PAXB_IARR_1_UPPER]
	
	ldr	r3,=0x00000001
	str	r3,[r0,#PAXB_FUNC0_IMAP1]
	bx	lr
END(pcie_init)

/*
 * Find the first slave address for a core in the AI EROM
 * Input:
 *	r1 - pointer to EROM after CIB
 * Output:
 *	r0 - slave address
 * Changed:
 *	r0, r1, r2
 */
FUNC(ai_get_slave)
1:	ldr	r0,[r1]
	ldr	r2,=(ER_TAG | ER_VALID)
	add	r1,r1,#4			/* Next erom entry */
	and	r2,r2,r0
	cmp	r2,#(ER_ADD | ER_VALID)
	bne	1b

2:	ldr	r2,=AD_ADDR_MASK
	and	r0,r0,r2
	bx	lr
END(ai_get_slave)


/*
 * Find the first wrapper address for a core in the AI EROM
 * Input:
 *	r1 - pointer to EROM after first slave ADD
 * Output:
 *	r0 - wrapper address
 * Changed:
 *	r0, r1, r2, r3
 */
FUNC(ai_get_wrapper)
1:	ldr	r0,[r1]
	ldr	r2,=(ER_TAG | ER_VALID)
	add	r1,r1,#4			/* Next erom entry */
	and	r2,r2,r0
	cmp	r2,#(ER_ADD | ER_VALID)
	bne	1b

	/* An address descriptor, is it a wrapper? */
	ldr	r2,=AD_ST_SWRAP			/* We test just for the SWRAP bit set, */
	tst	r0,r2				/*  that gets both Master and Slave */
	beq	1b				/*  wrappers. */

	/* It is, isolate address and return */
	ldr	r2,=AD_ADDR_MASK
	and	r0,r0,r2
	bx	lr
END(ai_get_wrapper)
#endif

/* Interrupt handlers */
/* Note: hndrte_arm.c for EXT_CBALL expects each trap handler to be 16 bytes */
LEAF(__traps)

/* Undefined instruction exception handler */
tr_und:
	srsdb	sp!, #PS_SYS	/* use system mode stack */
	cps	#PS_SYS		

	/* now cr4 is in system mode */
	/* lr has the value before the exception, push it to stack */
	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}	/* save r0-r7 to the stack */
	eor	r0,r0,r0
	add	r0,r0,#TR_UND
	b	trap

/* Software interrupt exception handler */
tr_swi:
	srsdb	sp!, #PS_SYS	/* use system mode stack */
	cps	#PS_SYS

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_SWI
	b	trap

/* Prefetch abort exception handler */
tr_iab:
	/* adjust lr to the address of abort insturction fetch */
	mov 	sp,lr
	sub	sp,sp,#4
	mov 	lr,sp
	srsdb	sp!, #PS_SYS	/* use system mode stack */
	cps	#PS_SYS

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_IAB
	b	trap

/* Data abort exception handler */
tr_dab:
	/* adjust lr to the address of aborted data fetch */
	mov 	sp,lr
	sub	sp,sp,#8
	mov 	lr,sp
	srsdb	sp!, #PS_SYS	/* use system mode stack */
	cps	#PS_SYS	

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_DAB
	b	trap

tr_bad: /* reserved */
	mov 	sp,lr
	sub	sp,sp,#4
	mov 	lr,sp
	srsdb	sp!, #PS_SYS	/* use system mode stack */
	cps	#PS_SYS

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_BAD
	b	trap

/* Interrupt handler */
tr_irq:
	/* subtract the offset */
	mov 	sp,lr
	sub	sp,sp,#4
	mov 	lr,sp
	srsdb	sp!, #PS_SYS	/* use system mode stack */
	cps	#PS_SYS

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_IRQ
	b	trap


/* Fast interrupt handler */
tr_fiq:
	mov 	sp,lr
	sub	sp,sp,#4
	mov 	lr,sp
	srsdb	sp!, #PS_SYS	/* use system mode stack */
	cps	#PS_SYS

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_FIQ
	b	trap

/* Generic exception handler */
trap:
	/*
	 * construt the trap structure in system mode stack 
	*/
	/* save trap type, epc, cpsr and spsr */
	mov 	r4, sp	/* sp is now at r0 */
	add	r4,r4,#64
	ldmia	r4!, {r1,r3} /* r1<-epc, r3<-spsr */
	mrs	r2, cpsr
	push	{r0,r1,r2,r3}	/* trap type(r0), epc(r1), cpsr,spsr */

	/* fix the value of pc in trap structure */
	sub	r4,r4,#12
	str	r1,[r4]	/* save the value or epc as r15 into the stack */ 
	
	/* save r8 - r13 */
	mov 	r7,sp	/* sp is at the being of the trap structure now */
	add	r7,r7,#TRAP_T_SIZE + 8	/* sp value before the exception */
	mov 	r6,r12
	mov	r5,r11
	mov	r4,r10
	mov	r3,r9
	mov	r2,r8
	add	sp,sp,#72 /* move sp to r14 */
	push	{r2-r7}	/* save r8-r13 to stack */		

	sub	sp,sp,#48 /* move sp back to the top of trap structure */	

	/* If trap_hook is null, go to the end */
	ldr	r4,=trap_hook
	ldr	r4,[r4]
	cmp	r4,#0
1:	beq	1b
	/* else call (*trap_hook)(trap_t) */
	mov	r0,sp
	blx	r4

/* Restore the state from the trap_t */
rfe:	
	/* pop r8-r14 */
	add	sp,sp,#48
	pop	{r0-r6} /* r8 - r14 */
	mov	r8,r0
	mov	r9,r1
	mov	r10,r2
	mov	r11,r3
	mov	r12,r4
	mov	lr,r6	/* restore lr */

	sub	sp,sp,#60 /* move sp to point to r0 */
	pop	{r0-r7}

	/* move sp to point to epc and spsr saved by srs */
	add	sp,sp,#32

	/* Restore cpsr, sp and return */
	rfeia	sp!	/* done! */
END(__traps)

/* Cycle Count */
FUNC(enable_arm_cyclecount)
	mrc	p15,0,r1,c9,c12,0
	ldr	r2,=1
	orr	r1,r1,r2
	mcr	p15,0,r1,c9,c12,0	/* bit[0] in PMCR: enable all counters */
	ldr	r1,=0x80000000
	mcr	p15,0,r1,c9,c12,1   /* C,bit[31]: enable PMCCNTR */
	bx	lr
END(enable_arm_cyclecount)

FUNC(disable_arm_cyclecount)
	ldr	r1,=0x80000000
	mcr	p15,0,r1,c9,c12,2   /* Count enable clear register: PMCNTENCLR */
	bx	lr
END(disable_arm_cyclecount)

FUNC(_getticks)
	mrc	p15,0,r0,c9,c13,0   /* Cycle count depends on PMCR.D bit[3] */
	bx	lr
END(_getticks)

FUNC(set_arm_cyclecount)
	mcr	p15,0,r0,c9,c13,0
	bx	lr
END(set_arm_cyclecount)

/*
 * Common code/data start here...
 */
#ifdef CONFIG_XIP
	.data
#endif

	DW(_memsize, 0)

/* C trap handler */
	DW(trap_hook, 0)

#ifdef	CONFIG_XIP
	.text
#endif

#ifdef _CFE_

/*
 * Setup the trap handler.
 */
FUNC(hndrte_set_trap)

	ldr	r2,=trap_hook
	ldr	r1,[r2]
	str	r0,[r2]
	mov	r0,r1
	bx	lr

END(hndrte_set_trap)

/*
 * Turn remap off and then jump to an given address
 */
FUNC(arm_jumpto)

	bx	r0

END(arm_jumpto)

/*  *********************************************************************
    *  CFE_FLUSHCACHE
    *
    *  Perform certain cache operations
    *
    *  Input parameters:
    *  	   a0 - flags (CFE_CACHE_xxx flags, or zero for a default)
    *      a1,a2 - start/end of range for "range invalidate" operations
    *      (not used otherwise)
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */

FUNC(_cfe_flushcache)

	mov	pc,lr

END(_cfe_flushcache)

/*  *********************************************************************
    *  CFE_LAUNCH
    *
    *  Start the user program.  The program is passed a handle
    *  that must be passed back when calling the firmware.
    *
    *  Parameters passed to the called program are as follows:
    *
    *      a0 - CFE handle
    *      a1 - entry vector
    *      a2 - reserved, will be 0
    *      a3 - entrypoint signature.
    *
    *  Input parameters:
    *  	   a0 - entry vector
    *
    *  Return value:
    *  	   does not return
    ********************************************************************* */

FUNC(um_launch)

	sub  sp,sp,#8
	push {r0}

	/*
	 * Mask all interrupts.
	 */

	mrs	r0,cpsr
	ldr	r1,=(PS_I | PS_F)
	orr	r0,r0,r1
	msr	cpsr,r0

	/*
	 * Flush the D-Cache, since the program we loaded is "data".
	 * Invalidate the I-Cache, so that addresses in the program
	 * region will miss and need to be filled from the data we
	 * just flushed above.
	 */

#if 0 /* not-implemented */
	ldr	r0,=(CFE_CACHE_FLUSH_D|CFE_CACHE_INVAL_I)
	CALLINIT_RELOC(init_table,R_INIT_CACHEOPS)
#endif

	/*
	 * Set things up for launching the program.  Pass the
	 * handle in A0 - apps need to remember that and pass it
	 * back.
	 */
	b RunProgram

END(um_launch)

/*
 * This is a nice place to set a breakpoint.
 */
FUNC(RunProgram)
#if 0
	LOADREL(r2,=segment_table)
	ldr r2,[r2,#R_SEG_APIENTRY]	/* A2 = code entry */

#if (CFG_EMBEDDED_PIC)
	ldr r1,mem_textreloc	/* relocate table entry */
	add r2,r2,r1
#endif
#endif
	mov r4,r0
	mov r1,#0		/* A1 = 0 */
	mov r0,#0		/* A0 = handle */
	mov r3,#0       /* A3 = entrypoint signature */
	pop {r4}		/* entry point */
	mov pc,r4		/* go for it. */

END(RunProgram)

#if 0
/*  *********************************************************************
    *  BOARD_SETLEDS(x)
    *
    *  Set LEDs for boot-time progress indication.  Not used if
    *  the board does not have progress LEDs.  This routine
    *  must not call any other routines, since it may be invoked
    *  either from KSEG0 or KSEG1 and it may be invoked
    *  whether or not the icache is operational.
    *
    *  Input parameters:
    *  	   a0 - LED value (8 bits per character, 4 characters)
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */
FUNC(board_setleds)

	mov	pc, lr			@ back to my caller

END(board_setleds)

/*  *********************************************************************
    *  CFE_LEDS
    *
    *  Set the on-board LEDs.
    *
    *  Input parameters:
    *  	   a0 - LEDs
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */
FUNC(cfe_leds)

	/* jump to BSP routine */
	//bl	board_setleds

	/* return */
	mov	pc, lr

END(cfe_leds)
#endif
/*  *********************************************************************
    *  _GETSEGTBL()
    *
    *  Return the address of the segment table.  We use this
    *  to display the startup messages.
    *
    *  You can't just address the table from C because it lives
    *  in the text segment.
    *
    *  Input parameters:
    *  	   nothing
    *
    *  Return value:
    *      address of table
    ********************************************************************* */

FUNC(_getsegtbl)

	mov	ip, lr		/* persevere link reg across call */
	LOADREL(r0,=segment_table)
	mov	lr, ip		/* restore link */
	mov pc, lr		/* back to my caller */

END(_getsegtbl)

#if 0
/*  *********************************************************************
    *  _SETCONTEXT()
    *
    *  Set the CONTEXT register.
    *
    *  Input parameters:
    *  	   a0 - context
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */

FUNC(_setcontext)

	mov	pc,lr

END(_setcontext)

/*  *********************************************************************
    *  CFE_WARMSTART
    *
    *  Restart the command interpreter
    *
    *  Input parameters:
    *      A0 - command status
    *  	   nothing (GP has already been set up for us)
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */

FUNC(cfe_warmstart)

	mov	pc,lr

END(cfe_warmstart)
#endif

#endif /* _CFE_ */
