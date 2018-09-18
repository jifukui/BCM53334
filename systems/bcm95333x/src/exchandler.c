/*
 * $Id: exchandler.c,v 1.5 Broadcom SDK $
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
 * This is the "C" part of the exception handler and the
 * associated setup routines.  We call these routines from
 * the assembly-language exception handler.
 *
 */

#include "types.h"
#include "exception.h"
#include "config.h"

#if CFG_TIMER_USE_INTERRUPT

/*  *********************************************************************
    *  Constants
    ********************************************************************* */

/*  *********************************************************************
    *  Globals
    ********************************************************************* */

exc_handler_t exc_handler;
//extern void _exc_entry(void);
//extern void _exc_setup_locore(long);
extern void CPUCFG_TLBHANDLER(void);
extern void cfe_flushcache(uint32_t,long,long);
extern uint32_t _getstatus(void);
extern void _setstatus(uint32_t);


/*  *********************************************************************
    *  cfe_exception(code,info)
    *
    *  Exception handler.  This routine is called when any CPU
    *  exception that is handled by the assembly-language
    *  vectors is reached.  The usual thing to do here is just to
    *  reboot.
    *
    *  Input parameters:
    *      code - exception type
    *      info - exception stack frame
    *
    *  Return value:
    *      usually reboots
    ********************************************************************* */
void um_exception(trap_t *tr)
{
    /*
     * ARM7TDMI trap types:
     *  0=RST, 1=UND, 2=SWI, 3=IAB, 4=DAB, 5=BAD, 6=IRQ, 7=FIQ
     *
     * ARM CM3 trap types:
     *  1=RST, 2=NMI, 3=FAULT, 4=MM, 5=BUS, 6=USAGE, 11=SVC,
     *  12=DMON, 14=PENDSV, 15=SYSTICK, 16+=ISR
     *
     * ARM CA9 trap types:
     *  0=RST, 1=UND, 2=SWI, 3=IAB, 4=DAB, 5=BAD, 6=IRQ, 7=FIQ
     */

    uint32 *stack = (uint32*)tr->r13;
    char *tr_type_str[8] = {"RST", "UND", "SWI", "IAB", "DAB", "BAD", "IRQ", "FIQ"};
    char *type_str = "UKN";

    if (tr->type < 8)
        type_str = tr_type_str[tr->type];

    /* Note that UTF parses the first line, so the format should not be changed. */
    printf("\nTRAP [%s](x)[%x]: pc[%x], lr[%x], sp[%x], cpsr[%x], spsr[%x]\n",
           type_str, tr->type, (uint32)tr, tr->pc, tr->r14, tr->r13, tr->cpsr, tr->spsr);
    printf("  r0[%x], r1[%x], r2[%x], r3[%x], r4[%x], r5[%x], r6[%x]\n",
           tr->r0, tr->r1, tr->r2, tr->r3, tr->r4, tr->r5, tr->r6);
    printf("  r7[%x], r8[%x], r9[%x], r10[%x], r11[%x], r12[%x]\n",
           tr->r7, tr->r8, tr->r9, tr->r10, tr->r11, tr->r12);

    /*
     * stack content before trap occured
     */
    printf("\n   sp+0 %08x %08x %08x %08x\n",
        stack[0], stack[1], stack[2], stack[3]);
    printf("  sp+10 %08x %08x %08x %08x\n\n",
        stack[4], stack[5], stack[6], stack[7]);

    xprintf("\n");
    _exc_restart();
}

/*  *********************************************************************
    *  cfe_setup_exceptions()
    *
    *  Set up the exception handlers.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      nothing
    ********************************************************************* */
void um_setup_exceptions(void)
{
    /* Set trap handler */
    hndrte_set_trap((uint32)um_exception);
}


/*  *********************************************************************
    *  exc_initialize_block()
    *
    *  Set up the exception handler.  Allow exceptions to be caught.
    *  Allocate memory for jmpbuf and store it away.
    *
    *  Returns NULL if error in memory allocation.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      jmpbuf_t structure, or NULL if no memory
    ********************************************************************* */
jmpbuf_t *exc_initialize_block(void)
{
    jmpbuf_t *jmpbuf_local;

    exc_handler.catch_exc = 1;

    /* Create the jmpbuf_t object */
    jmpbuf_local = (jmpbuf_t *) KMALLOC((sizeof(jmpbuf_t)),0);

    if (jmpbuf_local == NULL) {
    return NULL;
    }

    q_enqueue( &(exc_handler.jmpbuf_stack), &((*jmpbuf_local).stack));

    return jmpbuf_local;
}

/*  *********************************************************************
    *  exc_cleanup_block(dq_jmpbuf)
    *
    *  Remove dq_jmpbuf from the exception handler stack and free
    *  the memory.
    *
    *  Input parameters:
    *      dq_jmpbuf - block to deallocate
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void exc_cleanup_block(jmpbuf_t *dq_jmpbuf)
{
    int count;

    if (dq_jmpbuf == NULL) {
    return;
    }

    count = q_count( &(exc_handler.jmpbuf_stack));

    if( count > 0 ) {
    q_dequeue( &(*dq_jmpbuf).stack );
    KFREE(dq_jmpbuf);
    }
}

/*  *********************************************************************
    *  exc_cleanup_handler(dq_jmpbuf,chain_exc)
    *
    *  Clean a block, then chain to the next exception if required.
    *
    *  Input parameters:
    *      dq_jmpbuf - current exception
    *      chain_exc - true if we should chain to the next handler
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void exc_cleanup_handler(jmpbuf_t *dq_jmpbuf, int chain_exc)
{
    exc_cleanup_block(dq_jmpbuf);

    if( chain_exc == EXC_CHAIN_EXC ) {
    /*Go to next exception on stack */
    exc_longjmp_handler();
    }
}



/*  *********************************************************************
    *  exc_longjmp_handler()
    *
    *  This routine long jumps to the exception handler on the top
    *  of the exception stack.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      nothing
    ********************************************************************* */
void exc_longjmp_handler(void)
{
    int count;
    jmpbuf_t *jmpbuf_local;

    count = q_count( &(exc_handler.jmpbuf_stack));

    if( count > 0 ) {
    jmpbuf_local = (jmpbuf_t *) q_getlast(&(exc_handler.jmpbuf_stack));

    SETLEDS("CFE ");

    lib_longjmp( (*jmpbuf_local).jmpbuf, -1);
    }
}


/*  *********************************************************************
    *  mem_peek(d,addr,type)
    *
    *  Read memory of the specified type at the specified address.
    *  Exceptions are caught in the case of a bad memory reference.
    *
    *  Input parameters:
    *      d - pointer to where data should be placed
    *      addr - address to read
    *      type - type of read to do (MEM_BYTE, etc.)
    *
    *  Return value:
    *      0 if ok
    *      else error code
    ********************************************************************* */

int mem_peek(void *d, long addr, int type)
{

    jmpbuf_t *jb;

    jb = exc_initialize_block();
    if( jb == NULL ) {
    return CFE_ERR_NOMEM;
    }

    if (exc_try(jb) == 0) {

    switch (type) {
        case MEM_BYTE:
        *(uint8_t *)d = *((volatile uint8_t *) addr);
        break;
        case MEM_HALFWORD:
        *(uint16_t *)d = *((volatile uint16_t *) addr);
        break;
        case MEM_WORD:
        *(uint32_t *)d = *((volatile uint32_t *) addr);
        break;
        case MEM_QUADWORD:
        *(uint64_t *)d = *((volatile uint64_t *) addr);
        break;
        default:
        return CFE_ERR_INV_PARAM;
        }

    exc_cleanup_block(jb);
    }
    else {
    /*Exception handler*/

    exc_cleanup_handler(jb, EXC_NORMAL_RETURN);
    return CFE_ERR_GETMEM;
    }

    return 0;
}

/* *********************************************************************
   *  Write memory of type at address addr with value val.
   *  Exceptions are caught, handled (error message) and function
   *  returns with 0.
   *
   *  1 success
   *  0 failure
   ********************************************************************* */

int mem_poke(long addr, uint64_t val, int type)
{

    jmpbuf_t *jb;

    jb = exc_initialize_block();
    if( jb == NULL ) {
    return CFE_ERR_NOMEM;
    }

    if (exc_try(jb) == 0) {

    switch (type) {
        case MEM_BYTE:
        *((volatile uint8_t *) addr) = (uint8_t) val;
        break;
        case MEM_HALFWORD:
        *((volatile uint16_t *) addr) = (uint16_t) val;
        break;
        case MEM_WORD:
        *((volatile uint32_t *) addr) = (uint32_t) val;
        break;
        case MEM_QUADWORD:
        *((volatile uint64_t *) addr) = (uint64_t) val;
        break;
        default:
        return CFE_ERR_INV_PARAM;
        }

    exc_cleanup_block(jb);
    }
    else {
    /*Exception handler*/

    exc_cleanup_handler(jb, EXC_NORMAL_RETURN);
    return CFE_ERR_SETMEM;
    }

    return 0;
}
#endif
