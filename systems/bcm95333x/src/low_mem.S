/*  
 * $Id: low_mem.S,v 1.5 Broadcom SDK $
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
 * File:    low_mem_ca9.S
 * Purpose: H/W exception handling
 */
       .syntax unified

#include "thumb.h"
#include "board.h"
#include "arm.h"
#include "config.h"
#include "bsp_config.h"
#include "socregs.h"
#include "hndrte_armtrap.h"
/* Trap types */
#define TR_RST      0           /* Reset trap */
#define TR_UND      1           /* Indefined instruction trap */
#define TR_SWI      2           /* Software intrrupt */
#define TR_IAB      3           /* Instruction fetch abort */
#define TR_DAB      4           /* Data access abort */
#define TR_BAD      5           /* Bad trap: Not used by ARM */
#define TR_IRQ      6           /* Interrupt */
#define TR_FIQ      7           /* Fast interrupt */

#define STACK(_b_, _size_)                               \
        .section mem_stack                              ;\
        .balign 16                                      ;\
        .global _b_ ## _base                            ;\
_b_  ## _base:                                          ;\
        .rept _size_                                    ;\
        .byte   0                                       ;\
        .endr                                           ;\
        .balign 16                                      ;\
        .global _b_                                     ;\
_b_:                                                    ;\


#define HEAP(_b_, _size_)                               \
        .section mem_heap                                            ;\
        .balign 16                                      ;\
        .global _b_ ## _base                            ;\
        .global _b_                                     ;\
_b_  ## _base:                                          ;\
        .rept _size_                                    ;\
        .byte   0                                       ;\
        .endr                                           ;\
        .balign 16                                      ;\
_b_  ## _end:                                           ;\
        .section ".rodata"                              ;\
_b_:    .word _b_ ## _base                              ; \


#define DMA_HEAP(_b_, _size_)                           \
        .section mem_dmaheap                                   ;\
        .balign 16                                      ;\
        .global _b_ ## _base                            ;\
_b_  ## _base:                                          ;\
        .rept _size_                                    ;\
        .byte   0                                       ;\
        .endr                                           ;\
        .balign 16                                      ;\
_b_  ## _end:                                           ;\
        .global _b_                                     ;\
        .section ".rodata"                              ; \
_b_:    .word _b_ ## _base                              ;\

#define UMINFO_START()                                \
        .ascii "UM"       ;\
        .byte 0xA                                                                   ;
                       
#define UMINFO_WORD(VARIABLE, VALUE)              \
        .ascii #VARIABLE##"_W"                     ;\
        .byte 0xA                                                                   ;\
        .word  VALUE                             ;

#define UMINFO_STRING(VARIABLE, VALUE)              \
        .ascii #VARIABLE##"_S"                    ;\
        .byte 0xA                                                                   ;\
        .ascii VALUE                              ;\
        .byte 0xA

#define UMINFO_END()                                \
        .ascii "END"                                 ;\
        .byte 0xA                                                                   
 
#define REF_GEN(sym)                                    \
        .global sym ## _ref                                     ;\
        .global sym  ;\
sym ## _ref: ;\
           .word sym       
         

#define EXCEPTION_MODE                                  \
        .code   32
        
#define SA_SP (13*4)
#define SA_R0 (0*4)        

        .section .text, "ax";  \
        .global reset;       \

reset:

//  Fixed-location H/W exception vectors
        b     startup  
        b     undef_instr
        b     sw_rupt
        b     prefetch_abort
        b     data_abort
        b     bad
        b     irq
        b     fiq


        EXCEPTION_MODE

        /* Firmware version , board name and flash layout */ 
        .align  (6)
        UMINFO_START()
        UMINFO_STRING(CFG_BOARDNAME, CFG_BOARDNAME);
        UMINFO_WORD(CFE_VER, (CFE_VER_MAJOR << 24) | (CFE_VER_MINOR << 16) | (CFE_VER_BUILD << 8));
        UMINFO_STRING(target, target);
        UMINFO_WORD(BOARD_LOADER_ADDR, BOARD_LOADER_ADDR);
        UMINFO_WORD(BOARD_FIRMWARE_ADDR, BOARD_FIRMWARE_ADDR);
        UMINFO_WORD(BOARD_SECONDARY_FIRMWARE_ADDR, BOARD_SECONDARY_FIRMWARE_ADDR);
        UMINFO_WORD(CFG_FACTORY_CONFIG_BASE, CFG_FACTORY_CONFIG_BASE);
        UMINFO_WORD(CFG_FACTORY_CONFIG_OFFSET, CFG_FACTORY_CONFIG_OFFSET);
        UMINFO_WORD(CFG_CONFIG_BASE, CFG_CONFIG_BASE);
        UMINFO_WORD(CFG_CONFIG_OFFSET, CFG_CONFIG_OFFSET);
        UMINFO_WORD(CFG_CONFIG_SIZE, CFG_CONFIG_SIZE);
        UMINFO_WORD(CFG_FACTORY_CONFIG_BASE, CFG_FACTORY_CONFIG_BASE);
        UMINFO_WORD(CFG_FACTORY_CONFIG_OFFSET, CFG_FACTORY_CONFIG_OFFSET);
        UMINFO_END();
        .align (6)
        /* End of firmware version , board name and flash layout */ 
// Register state on exception, at a fixed position in memory
// Note that these are the register values from the faulting mode,
// not those seen by the exception handler
// Assumes .text is writable (in RAM not ROM)
// System reset
startup:
  
	/* Disable interrupts */
	mrs	r0, cpsr
	orr	r0, r0, #0xc0
	msr	cpsr,r0


  /* calculate offset */
  adr     r0, startup  
  ldr     r1, =startup
  sub     r9, r0, r1 
  
	/* Initialize cp15 registers */
	mov r1, #0
  MCR	p15,0,r1,c5,c0,0   /* DFSR */
  MCR	p15,0,r1,c5,c0,1   /* IFSR */
  MCR	p15,0,r1,c6,c0,0   /* DFAR */
  MCR	p15,0,r1,c6,c0,1   /* WFAR */

	@ Disable MMU
	MRC p15, 0, r1, c1, c0, 0 @ Read Control Register configuration data
	BIC r1, r1, #0x1
  BIC r1, r1, #0x2 @ strict alignment fault checked disable
	MCR p15, 0, r1, c1, c0, 0 @ Write Control Register configuration data

	@ Disable L1 Caches
	MRC p15, 0, r1, c1, c0, 0 @ Read Control Register configuration data
	BIC r1, r1, #(0x1 << 12) @ Disable I Cache
	BIC r1, r1, #(0x1 << 2) @ Disable D Cache
	MCR p15, 0, r1, c1, c0, 0 @ Write Control Register configuration data

	@ Invalidate L1 Caches
	@ Invalidate Instruction cache
	MOV r1, #0
	MCR p15, 0, r1, c7, c5, 0

  @ Invalidate BTAC
	MCR p15, 0, r0, c7, c5, 6 	
  mcr p15, 0, r1, c7, c10, 4 /* DSB */
  mcr p15, 0, r0, c7, c5, 4 /* ISB */ 

  @ Enable I cache
  MRC     p15, 0, r0, c1, c0, 0   @ Read Control Register configuration data
  ORR     r0, r0, #(0x3 << 11)    @ Enable I Cache and Branch prediction
  MCR     p15, 0, r0, c1, c0, 0   @ Write Control Register configuration data

  @  Invalidate Data cache
  @ To make the code general purpose, we calculate the cache size first and loop through each set + way
  MOV  r0, #0                  
  MCR  p15 ,2, r0, c0, c0, 0
  mcr  p15, 0, r0, c7, c5, 4 /* ISB */ 
  MRC  p15, 1, r0, c0, c0, 0   @  Read Cache Size ID
  LDR  r4, =0x1ff
  AND  r4, r4, r0, LSR #3      @
  ADD  r4, r4, #1              @  r4 = no: of ways
  LDR  r3, =0x7fff
  AND  r0, r3, r0, LSR #13     @  r0 = no. of sets/lines - 1
  ADD  r0, r0, #1              @  r0 = no: of ways
  MOV  r1, #0                  @  r1 = way counter way_loop
way_loop:
    MOV  r3, #0                  @  r3 = set counter set_loop
set_loop:
    MOV  r2, r1, LSL #30 
    ORR  r2, r2, r3, LSL #5
    @  r2 = set/way cache operation format
    MCR  p15, 0, r2, c7, c6, 2   @  Invalidate line described by r2
    ADD  r3, r3, #1              @  Increment set counter
    CMP  r0, r3                  @  Last set reached yet?
    BNE  set_loop                @  if not, iterate set_loop
    ADD  r1, r1, #1              @  else, next
    CMP  r1, r4                  @  Last way reached yet?
    BNE  way_loop                @  if not, iterate way_loop
   
	@ Enable D Cache
    MCR  p15, 0, r1, c7, c10, 4 /* SB */
    MRC  p15, 0, r0, c1, c0, 0  @ read CP15 register 1 into r0
    ORR  r0, r0, #0x4           @ enable D Cache
    MCR  p15, 0, r0, c1, c0, 0  @ write CP15 register 1
    MCR  p15, 0, r0, c7, c5, 4 /* ISB */ 

	@ Invalidate TLB
	MCR p15, 0, r1, c8, c7, 0	

	@ DSB causes completion of all cache maintenance operations appearing in program
	@ order before the DSB instruction
	@ An ISB instruction causes the effect of all branch predictor maintenance
	@ operations before the ISB instruction to be visible to all instructions
	@ after the ISB instruction.
scu_enable:
	/* Invalidate and enable SCU */
    LDR     r0, =IHOST_SCU_INVALIDATE_ALL
    MOV     r1, #0xFF
    STR     r1, [r0]
    LDR     r0, =IHOST_SCU_CONTROL
    LDR     r1, [r0]
    ORR     r1, r1, #0x1
    STR     r1, [r0]
    
	/* MMU initialization */
    /* Configure translation table base (TTB) control register cp15,c2
     to a value of all zeros, indicates we are using TTB register 0 */
    mov     r0,#0x0
    mcr     p15, 0, r0, c2, c0, 2
	/* Load page table address to CP15 */    
	ldr r0, =mmu_table1		
	add r0, r0, r9
	mcr p15, 0, r0, c2, c0, 0

	/* Set the access control to Client */
	ldr r0, =0x55555555
	mcr p15, 0, r0, c3, c0, 0

	/* Enable MMU */
  MRC p15, 0, r1, c1, c0, 0
  ORR r1, r1, #0x01
  MCR p15, 0, r1, c1, c0, 0
	MCR p15, 0, r0, c7, c5, 4 /* ISB */


  bl l2cc_init

  @ Enable I cache
  MRC     p15, 0, r0, c1, c0, 0   @ Read Control Register configuration data
  ORR     r0, r0, #(0x3 << 11)    @ Enable I Cache and Branch prediction
  MCR     p15, 0, r0, c1, c0, 0   @ Write Control Register configuration data
  
relocate_reference:
  adr     r0, relocate_reference
  ldr     r1, =relocate_reference
  sub     r9, r0, r1 
  /* relocate reference pointer at text section */  
  ldr     r5, text_end_ref        
  add     r5, r5, r9
   
copy_data: 
  ldr     r6,data_start_ref
  ldr     r7,bss_start_ref
  sub     r7,r7,r6               /* init data length */
  add     r7,r5               /* end of init data in ROM */
  cmp     r7,r5
  beq     clear_bss                 /* skip if region is empty */
1:
  ldr     r4,[r5],#4
  str     r4,[r6],#4
  cmp     r7,r5
  bhi     1b
  
clear_bss: 
  /* Clear bss*/
  ldr     r5, bss_start_ref
  ldr     r6, dtcm_endaddr_ref
  cmp     r6,r5
  beq     goto_c_entry                      /* skip if region is empty */

   mov     r7,#0
1: str     r7,[r5],#4
   cmp     r6,r5
   bhi     1b

  ldr     r5, itcm_start_ref 
  ldr     r6, itcm_end_ref
  cmp     r6,r5
  beq     goto_c_entry                      /* skip if region is empty */

   mov     r7,#0
1: str     r7,[r5],#4
   cmp     r6,r5
   bhi     1b

relocate_got:
/* Relocate GOT and data.rel */
   ldr    r0, data_end_ref
   ldr    r1, dtcm_endaddr_ref
   bl relocate

   ldr    r0, data_rel_start_ref
   ldr    r1, data_rel_end_ref
   bl relocate

   ldr     r0, data_rel2_start_ref
   ldr     r1, data_rel2_end_ref
   bl      relocate
   
goto_c_entry:
   ldr     sp, boot_stack_ref  
   ldr     r9, data_end_ref        // load GOT into r9                                 
   bl     main   

REF_GEN(bss_start)
REF_GEN(bss_end)
REF_GEN(itcm_start)
REF_GEN(itcm_end)
REF_GEN(text_end)
REF_GEN(data_start)
REF_GEN(data_end)
REF_GEN(__rel_dyn_start)
REF_GEN(data_rel_start)
REF_GEN(data_rel_end)
REF_GEN(dtcm_endaddr)
REF_GEN(boot_stack)
REF_GEN(data_rel2_start)
REF_GEN(data_rel2_end)


trap_hook:
	.word 0
/*
 * Setup the trap handler.
 */
LEAF(hndrte_set_trap)
	ldr	r2,=trap_hook
	ldr	r1,[r2]
	str	r0,[r2]
	mov	r0,r1
	bx	lr
END(hndrte_set_trap)

/* Interrupt handlers */
/* Note: hndrte_arm.c for EXT_CBALL expects each trap handler to be 16 bytes */
LEAF(__traps)

/* Undefined instruction exception handler */
undef_instr:
  	ldr   sp, exception_stack_ref
	/* now cr4 is in system mode */
	/* lr has the value before the exception, push it to stack */
	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}	/* save r0-r7 to the stack */
	eor	r0,r0,r0
	add	r0,r0, #TR_UND
	b	trap

/* Software interrupt exception handler */
sw_rupt:
        ldr   sp, exception_stack_ref
	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_SWI
	b	trap

/* Prefetch abort exception handler */
prefetch_abort:
  ldr   sp, exception_stack_ref
	/* adjust lr to the address of abort insturction fetch */
	mov 	sp,lr
	sub	sp,sp,#4
	mov 	lr,sp

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_IAB
	b	trap

/* Data abort exception handler */
data_abort:
  ldr   sp, exception_stack_ref
	/* adjust lr to the address of aborted data fetch */
	mov 	sp,lr
	sub	sp,sp,#8
	mov 	lr,sp

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_DAB
	b	trap

bad: /* reserved */
  ldr   sp, exception_stack_ref
	mov 	sp,lr
	sub	sp,sp,#4
	mov 	lr,sp

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_BAD
	b	trap

/* Interrupt handler */
irq:
  ldr   sp, exception_stack_ref
	/* subtract the offset */
	mov 	sp,lr
	sub	sp,sp,#4
	mov 	lr,sp

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_IRQ
	b	trap


/* Fast interrupt handler */
fiq:
  ldr   sp, exception_stack_ref
	mov 	sp,lr
	sub	  sp,sp,#4
	mov 	lr,sp
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
REF_GEN(exception_stack)

LEAF(l2cc_init)
/* L2CC Initialization */
	ldr r0, =IHOST_L2C_CACHE_ID /* L2CC reg base */
	ldr r1, [r0, #0x104]
	bic r1, r1, #0x000F0000 /* Clear the Way-size and associativity */
	ldr r2, =0x0A120000     /* 8 x 16KB = 128KB L2 cache */
	orr r1, r1, r2          /* Non-secure interrupt access, Way-size 16KB, 
                              16 ways and event monitoring */
  str r1, [r0, #0x104]
  /* Invalidate the entire L2 cache */
  ldr r1, =0xFFFF         /* 16 ways */
  str r1, [r0, #0x77C]    /* inv_way */
    
  mov r2, #0
test_inv_way:    
  ldr r1, [r0, #0x77C]
  cmp r1,r2
  BNE test_inv_way
    
  ldr r1, =0x1FF
  str r1, [r0, #0x220] /* int_clear */
  mov r1, #0x01
  str r1, [r0, #0x100] /* control */
	
  /* Memory barrier */
	mov r1, #0
	mcr p15, 0, r1, c7, c10, 4 /* DSB */
	MCR p15, 0, r0, c7, c5, 4 /* ISB */

  LDR r1, =0x80000000
  LDR r2, =0x80020000
  LDR r4, =0xa5a5a5a5
  LDR r5, =0xa5a5a5a5
  LDR r6, =0xa5a5a5a5
  LDR r7, =0xa5a5a5a5
  LDR r8, =0xa5a5a5a5
  LDR r9, =0xa5a5a5a5
  LDR r10, =0xa5a5a5a5
  LDR r11, =0xa5a5a5a5
  /* Enable lockdown by line */
  mov r3, #0x01
  ldr r0, =IHOST_L2C_CACHE_ID
  str r3, [r0, #0x950]
loop2:
  STMIA r1!, {r4-r11}
  ldr r3, =0x10
cdelay2:
  sub r3, r3, #1
  cmp r3, #0
  bne cdelay2
  add r4, #1
  add r5, #1
  add r6, #1
  add r7, #1
  add r8, #1
  add r9, #1
  add r10, #1
  add r11, #1
  cmp r1, r2
  bne loop2

  /* Disable lockdown by line */
  mov r3, #0
  ldr r0, =IHOST_L2C_CACHE_ID
  str r3, [r0, #0x950]		
l2cc_init_done:
#if 1
/* Configure ARM clock : 125 MHz */
set_cru_policy7:
    ldr	r0,=IHOST_PROC_CLK_WR_ACCESS
    ldr	r1,=0xa5a501
    str	r1,[r0]
    ldr	r0,=IHOST_PROC_CLK_PLLARMCTRL5
    ldr	r1,[r0]
    and r1,r1,#0xFFFFFF00
    /* arm PLL post-divider = 12 */
    orr r1,r1,#0xC
	str	r1,[r0]
	ldr	r0,=IHOST_PROC_CLK_PLLARMA
    /* VCO = 25M * 60 */
	ldr	r1,=0x1003c01
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
    ldr	r1,=0x00000303
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

/* Setup CCA UART clock divider to 2*/
    LDR r1, =ChipcommonA_ClkDiv
    LDR r2, [r1]
    AND r2, r2, #0xFFFFFF00
    ORR r2, r2, #0x2
    STR r2, [r1]
coreo_init_done:	
#endif	
  mov pc, lr        
/* End of L2CC Initialization */  
END(l2cc_init)  

LEAF(relocate)
        ldr     r3,=relocate  /* Fix locations starting with 0xF0xx-xxxx */
        adr     r4,relocate
        sub     r5, r4, r3    /* r5 is the offset */
        cmp     r5, #0
        bne     start_fix
        bx      lr
start_fix:
        ldr     r4,=reset
        ldr     r3,=text_end
fixloop:
        ldr     r6, [r0]
        cmp     r6, r4
        bcc     fixnext
        cmp     r6, r3
        bhi     fixnext 
        add     r6,r6,r5
        str     r6,[r0]
fixnext:
        add     r0, r0, #4
        cmp     r0, r1
        blo     fixloop
        bx      lr
END(relocate)

THUMBLEAF(enable_arm_cyclecount)
         mrc     p15,0,r1,c9,c12,0
         ldr     r2,=1
         orr     r1,r1,r2
         mcr     p15,0,r1,c9,c12,0       /* bit[0] in PMCR: enable all counters */
         ldr     r1,=0x80000000
         mcr     p15,0,r1,c9,c12,1   /* C,bit[31]: enable PMCCNTR */
         bx      lr
END(enable_arm_cyclecount)

THUMBLEAF(disable_arm_cyclecount)
         ldr     r1,=0x80000000
         mcr     p15,0,r1,c9,c12,2   /* Count enable clear register: PMCNTENCLR */
         bx      lr
END(disable_arm_cyclecount)


THUMBLEAF(_getticks)
        mrc     p15,0,r0,c9,c13,0   /* Cycle count depends on PMCR.D bit[3] */
        bx      lr
END(_getticks)

THUMBLEAF(um_launch)

        sub  sp,sp,#8
        push {r0}

        /*
         * Mask all interrupts.
         */

        mrs     r0,cpsr
        ldr     r1,=0xc0
        orr     r0,r0,r1
        msr     cpsr,r0

        /*
         * Flush the D-Cache, since the program we loaded is "data".
         * Invalidate the I-Cache, so that addresses in the program
         * region will miss and need to be filled from the data we
         * just flushed above.
         */

#if 0 /* not-implemented */
        ldr     r0,=(CFE_CACHE_FLUSH_D|CFE_CACHE_INVAL_I)
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
THUMBLEAF(RunProgram)
#if 0
        LOADREL(r2,=segment_table)
        ldr r2,[r2,#R_SEG_APIENTRY]     /* A2 = code entry */

#if (CFG_EMBEDDED_PIC)
        ldr r1,mem_textreloc    /* relocate table entry */
        add r2,r2,r1
#endif
#endif
        mov r4,r0
        mov r1,#0               /* A1 = 0 */
        mov r0,#0               /* A0 = handle */
        mov r3,#0       /* A3 = entrypoint signature */
        pop {r4}                /* entry point */
        mov pc,r4               /* go for it. */


END(RunProgram)
STACK(boot_stack, CFG_STACK_SIZE);
STACK(exception_stack, 128);
STACK(FIQStack, 128);
HEAP(mem_heapstart, (CFG_HEAP_SIZE * 1024));
DMA_HEAP(mem_dmaheapstart, 128);

