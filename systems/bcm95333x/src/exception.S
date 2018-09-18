/*
 * $Id: exception.S,v 1.3 Broadcom SDK $
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
 * Exception Handler
 *
 */

#if 1

//#include "sbmips.h"
//#include "exception.h"
#include <arminc.h>
#include <armmacros.h>
//#include "cpu_config.h"		/* for definition of HAZARD and ERET */
//#include "bsp_config.h"

#else

#include "sbmips.h"
#include "exception.h"
#include "mipsmacros.h"
#include "cpu_config.h"		/* for definition of HAZARD and ERET */
#include "bsp_config.h"

/*  *********************************************************************
    *  Macros
    ********************************************************************* */

#ifdef _MIPSREGS32_
#define LREG	lw
#define SREG	sw
#define SRL	srl
#define SLL	sll
#else
#define LREG	ld
#define SREG	sd
#define SRL	dsrl
#define SLL	dsll
#endif


/*  *********************************************************************
    *  Data
    ********************************************************************* */

                .sdata

		.globl	_exc_vectab
_exc_vectab:	_LONG_	0		# XTYPE_RESET
		_LONG_	0		# XTYPE_TLBFILL  (not used)
		_LONG_	0		# XTYPE_XTLBFILL
		_LONG_	0		# XTYPE_CACHEERR (not used)
		_LONG_	0		# XTYPE_EXCEPTION
		_LONG_	0		# XTYPE_INTERRUPT
		_LONG_	0		# XTYPE_EJTAG

/*  *********************************************************************
    *  Common Data
    ********************************************************************* */

		.bss


/*  *********************************************************************
    *  Code
    ********************************************************************* */

		.text

#define R_EXC_CERR_TEMPLATE	_TBLIDX(0)
#define R_EXC_CERR_TEMPLATE_END	_TBLIDX(1)

		.globl	_exc_cerr_htable
_exc_cerr_htable:
		_LONG_	_exc_cerr_template
		_LONG_	_exc_cerr_template_end



/*  *********************************************************************
    *  _exc_cerr_template
    *  
    *  This is a template routine for our cache error handler.  
    *  We save a couple of registers in our magic save area, then
    *  dispatch to code elsewhere in CFE.
    *  
    *  This code is copied right to the vector address, so it has
    *  to be kept tiny!
    *  
    *  Input parameters: 
    *  	   nothing - running uncached, all registers trashed
    *  	   
    *  Return value:
    *  	   might return, might not
    ********************************************************************* */

LEAF(_exc_cerr_template)

	/*
	 * Magic!  When the cache error handler is running,
	 * we are in a very special state, running uncached
	 * and with translations turned off.  We can use offsets
	 * from r0(zero) to store registers we need to use
	 * during the error handler.
	 */

		.set push ; .set noreorder

		SR	k0,CFE_LOCORE_GLOBAL_K0TMP(zero)
		SR	k1,CFE_LOCORE_GLOBAL_K1TMP(zero)
		SR	ra,CFE_LOCORE_GLOBAL_RATMP(zero)
		SR	gp,CFE_LOCORE_GLOBAL_GPTMP(zero)

		LR	k0,CFE_LOCORE_GLOBAL_CERRH(zero)
		jal	k0
		 nop

		LR	k0,CFE_LOCORE_GLOBAL_K0TMP(zero)
		LR	k1,CFE_LOCORE_GLOBAL_K1TMP(zero)
		LR	ra,CFE_LOCORE_GLOBAL_RATMP(zero)
		LR	gp,CFE_LOCORE_GLOBAL_GPTMP(zero)
		ERET

		.set pop

	/*
	 * Note: make sure this routine does not exceed 128 bytes
	 */

_exc_cerr_template_end:

END(_exc_cerr_template)

/*  *********************************************************************
    *  _exc_setup_locore(cerrh)
    *  
    *  Set global data into the low-memory region.  We do this in
    *  assembly language so it's easier to deal with the 32-bit/64-bit
    *  issues that arise in the "C" code.
    *  
    *  Input parameters: 
    *  	   a0 - cache error handler
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

LEAF(_exc_setup_locore)

		move	t4,ra

	/*
	 * Save GP for easy re-use, using uncached writes.  
	 */

		li	t0,PHYS_TO_K1(CFE_LOCORE_GLOBAL_GP)
		SR	gp,0(t0)

	/*
	 * Initialize cache error handler pointer.  Make it 
	 * uncached, since cache error handlers should not 
	 * touch the cache.
	 */

		li	t1,(K0SIZE-1)
		and	a0,a0,t1		# keep just physical part
		li	t1,K1BASE
		or	a0,a0,t1		# make into an uncached address

		li	t0,PHYS_TO_K1(CFE_LOCORE_GLOBAL_CERRH)
		SR	a0,0(t0)

	/*
	 * Move the cache error handler into low RAM.
	 */

		li	t0,PHYS_TO_K1(MIPS_RAM_VEC_CACHEERR)

		LOADREL(t1,_exc_cerr_htable)
		LR	t2,R_EXC_CERR_TEMPLATE_END(t1)
		LR	t1,R_EXC_CERR_TEMPLATE(t1)

1:		lw	t3,0(t1)		# get a word
		sw	t3,0(t0)		# write a word
		ADD	t0,4			# next word...
		ADD	t1,4
		blt	t1,t2,1b		# till done

	/*
	 * Now do the whole thing again, but with cached writes.
	 * Writing uncached makes sure the data is actually in memory,
	 * and writing cached makes sure we write the same
	 * stuff again when the cache is evicted.  
	 * This way we don't have to bother with cacheops,
	 * a bonus on the BCM1250 with its funky L2.
	 */

		li	t0,PHYS_TO_K0(CFE_LOCORE_GLOBAL_GP)
		SR	gp,0(t0)

		li	t0,PHYS_TO_K0(CFE_LOCORE_GLOBAL_CERRH)
		SR	a0,0(t0)

		li	t0,PHYS_TO_K0(MIPS_RAM_VEC_CACHEERR)

		LOADREL(t1,_exc_cerr_htable)
		LR	t2,R_EXC_CERR_TEMPLATE_END(t1)
		LR	t1,R_EXC_CERR_TEMPLATE(t1)

1:		lw	t3,0(t1)		# get a word
		sw	t3,0(t0)		# write a word
		ADD	t0,4			# next word...
		ADD	t1,4
		blt	t1,t2,1b		# till done


	/*
	 * done! 
	 */

		move	ra,t4
		j	ra

END(_exc_setup_locore)

#endif

/*  *********************************************************************
    *  _exc_setvector(xtype,addr)
    *  
    *  Set an exception vector address
    *  
    *  Input parameters: 
    *  	   xtype - exception vector type
    *  	   addr - routine address
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */
//NS-XXX

FUNC(_exc_setvector)

#if 0
		la	v0,_exc_vectab
		srl	a0,3		/* convert 8-byte index to array index */
		sll	a0,BPWSIZE	/* convert back to index appropriate for word size */
		add	v0,a0
		SR	a1,(v0)
		j	ra
#else
		mov pc,lr
#endif

END(_exc_setvector)

#if 0

/*  *********************************************************************
    *  _exc_crash_sim()
    *  
    *  Crash the GDB simulator, causing it to exit.
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing - does not return
    ********************************************************************* */


LEAF(_exc_crash_sim)

		li $2,1
		li $3,0xdead
                li $4,0
                syscall	0xca
1:		b	1b

END(_exc_crash_sim)

#endif /* #if 0 */

/*  *********************************************************************
    *  _exc_cache_crash_sim()
    *  
    *  As _exc_crash_sim, but distinguish cache error exception.
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing - does not return
    ********************************************************************* */


FUNC(_exc_cache_crash_sim)
//NS-XXX
#if 0
		li $2,1
		li $3,0xbadc
                li $4,0
                syscall	0xca
1:		b	1b
#else
#endif
END(_exc_cache_crash_sim)


/*  *********************************************************************
    *  _exc_restart()
    *  
    *  Restart the firmware at the boot address
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

FUNC(_exc_restart)

		ldr	lr,=0xFFFF0000		/* ROM restart vector */
		mov	pc,lr

END(_exc_restart)

#if 0
/*  *********************************************************************
    *  _exc_entry(k0)
    *  
    *  Main exception entry point.
    *  
    *  Input parameters: 
    *  	   k0 - exception type
    *  	   
    *  Return value:
    *  	   ...
    ********************************************************************* */

LEAF(_exc_entry)

		.set noreorder
		.set noat

		subu	k1,sp,EXCEPTION_SIZE
		SRL	k1,3
		SLL	k1,3

                SREG    zero,XGR_ZERO(k1)
		SREG 	AT,XGR_AT(k1)

		SREG	v0,XGR_V0(k1)
		SREG	v1,XGR_V1(k1)

		SREG	a0,XGR_A0(k1)
		SREG	a1,XGR_A1(k1)
		SREG	a2,XGR_A2(k1)
		SREG	a3,XGR_A3(k1)

		SREG	t0,XGR_T0(k1)
		SREG	t1,XGR_T1(k1)
		SREG	t2,XGR_T2(k1)
		SREG	t3,XGR_T3(k1)
		SREG	t4,XGR_T4(k1)
		SREG	t5,XGR_T5(k1)
		SREG	t6,XGR_T6(k1)
		SREG	t7,XGR_T7(k1)

		SREG	s0,XGR_S0(k1)
		SREG	s1,XGR_S1(k1)
		SREG	s2,XGR_S2(k1)
		SREG	s3,XGR_S3(k1)
		SREG	s4,XGR_S4(k1)
		SREG	s5,XGR_S5(k1)
		SREG	s6,XGR_S6(k1)
		SREG	s7,XGR_S7(k1)

		SREG	t8,XGR_T8(k1)
		SREG	t9,XGR_T9(k1)

		SREG	gp,XGR_GP(k1)
		SREG	sp,XGR_SP(k1)
		SREG	fp,XGR_FP(k1)
		SREG	ra,XGR_RA(k1)

		mfc0	t0,C0_CAUSE
		mfc0	t1,C0_SR
		MFC0	t2,C0_BADVADDR
		MFC0	t3,C0_EPC
		mfc0    t4,C0_PRID
		mflo    t5
		mfhi    t6	
		SREG	t0,XCP0_CAUSE(k1)
		SREG	t1,XCP0_SR(k1)
		SREG	t2,XCP0_VADDR(k1)
		SREG	t3,XCP0_EPC(k1)
		SREG	t4,XCP0_PRID(k1)
		SREG	t5,XGR_LO(k1)
		SREG	t6,XGR_HI(k1)

#if CFG_EMBEDDED_PIC
		la	gp,PHYS_TO_K0(CFE_LOCORE_GLOBAL_GP)
		LR	gp,0(gp)		# get our GP handle from low memory vector
#else
		la	gp,_gp			# Load up GP, not relocated so it's easy
#endif

		move	a0,k0			# Pass exception type 
		move	a1,k1			# Pass frame to exception handler
		la	t0,_exc_vectab		# get base of exception vectors
		srl	k0,3			# convert 8-byte index to array index 
		sll	k0,BPWSIZE		# convert back to index appropriate for word size 
		addu	t0,k0			# get vector address
		LR	t0,(t0)			# to call handler

		move	sp,k1			# "C" gets fresh stack area

		jalr	t0			# Call exception handler
		 nop
	
		move    k1, sp
		LREG  	AT,XGR_AT(k1)

		LREG	t0,XGR_LO(k1)
		LREG	t1,XGR_HI(k1)
		mtlo    t0
		mthi    t1

		LREG	a0,XGR_A0(k1)
		LREG	a1,XGR_A1(k1)
		LREG	a2,XGR_A2(k1)
		LREG	a3,XGR_A3(k1)

		LREG	t0,XGR_T0(k1)
		LREG	t1,XGR_T1(k1)
		LREG	t2,XGR_T2(k1)
		LREG	t3,XGR_T3(k1)
		LREG	t4,XGR_T4(k1)
		LREG	t5,XGR_T5(k1)
		LREG	t6,XGR_T6(k1)
		LREG	t7,XGR_T7(k1)

		LREG	s0,XGR_S0(k1)
		LREG	s1,XGR_S1(k1)
		LREG	s2,XGR_S2(k1)
		LREG	s3,XGR_S3(k1)
		LREG	s4,XGR_S4(k1)
		LREG	s5,XGR_S5(k1)
		LREG	s6,XGR_S6(k1)
		LREG	s7,XGR_S7(k1)

		LREG	t8,XGR_T8(k1)
		LREG	t9,XGR_T9(k1)

		LREG	gp,XGR_GP(k1)
		LREG	sp,XGR_SP(k1)
		LREG	fp,XGR_FP(k1)
		LREG	ra,XGR_RA(k1)

/* do any CP0 cleanup here */

		LREG	v0,XGR_V0(k1)
		LREG	v1,XGR_V1(k1)
	
		ERET

		.set at
		.set reorder


END(_exc_entry)

#endif

/*  *********************************************************************
    * _exc_clear_sr_exl()
    *	
    * Clear SR(EXL) and return to caller.
    *
    * Input parameters:	
    *	nothing
    *
    * Return value:	
    *	nothing					
    ********************************************************************* */
	
FUNC(_exc_clear_sr_exl)
//NS-XXX
#if 0
		mfc0	t0,C0_SR
		and	t0,t0,~(0x02)	# clear SR(EXL). Bit 1
		mtc0	t0,C0_SR

		HAZARD

		j	ra
#else
		mov pc,lr
#endif
END(_exc_clear_sr_exl)			

/*  *********************************************************************
    * _exc_clear_sr_erl()
    *	
    * Clear SR(ERL) and return to caller.
    *
    * Input parameters:	
    *	nothing
    *
    * Return value:	
    *	nothing					
    ********************************************************************* */
	
FUNC(_exc_clear_sr_erl)
//NS-XXX
#if 0
		mfc0	t0,C0_SR
		and	t0,t0,~(0x04)		# clear SR(ERL). Bit 2
		mtc0	t0,C0_SR

		HAZARD
	
		j	ra
#else
		mov pc,lr
#endif
	
END(_exc_clear_sr_erl)			
	

/*  *********************************************************************
    *  End
    ********************************************************************* */

