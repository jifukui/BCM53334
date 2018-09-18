/*
 * $Id: sal_timer.c,v 1.6 Broadcom SDK $
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

#include "system.h"

#define TIMER0_TICK         1UL  /* unit:ms */

/******************************************************************************
 *
 *   COMPARE configuration
 *
 ******************************************************************************/
#if CONFIG_HURRICANE2_EMULATION
/* Clock is too slow in emulation */
#define TICKS_PER_US    (1)
#else
#define TICKS_PER_US    ((BOARD_CPU_CLOCK)/1000000)
#endif

#define COMPARE_VALUE   (TIMER0_TICK*1000UL*TICKS_PER_US)

volatile STATIC uint32 sys_ticks;
volatile STATIC uint32 last_sys_ticks;
volatile STATIC uint32 sys_seconds;

extern uint32 _getticks(void);

#if !CFG_TIMER_USE_INTERRUPT
STATIC uint32 timer_oldcount; /* For keeping track of ticks */
STATIC uint32 timer_remticks;
STATIC uint32 clockspertick;
#endif /* !CFG_TIMER_USE_INTERRUPT */

tick_t
sal_get_ticks(void)
{
    return (tick_t)sys_ticks;
}

uint32
APIFUNC(sal_get_seconds)(void) REENTRANT
{
    uint32 delta;

    delta = sys_ticks - last_sys_ticks;
    last_sys_ticks = sys_ticks - (delta % 1000UL);
    sys_seconds += (delta / 1000UL);

    return sys_seconds;
}

uint32
sal_get_us_per_tick(void)
{
    return TIMER0_TICK * 1000UL;
}

#if CFG_TIMER_USE_INTERRUPT
STATICFN void arm_counter_compapre_isr(int ip)
{
    sys_ticks++;
}
#else
extern void sal_timer_task(void *param);
void
sal_timer_task(void *param)
{
    uint32 count, delta;
    uint32 (*funcptr)(void) = (uint32 (*)(void))_getticks;

    count = (*funcptr)();
    delta = count - timer_oldcount;

    timer_remticks += delta;
    
    if (timer_remticks > (clockspertick << 4)) {
        sys_ticks += (timer_remticks / clockspertick);
        timer_remticks %= clockspertick;
        }
    else {
        while (timer_remticks > clockspertick) {
            timer_remticks -= clockspertick;
            sys_ticks++;
        }
    }
    timer_oldcount = count;
}
#endif /* CFG_TIMER_USE_INTERRUPT */

void
sal_timer_init(uint32 clk_hz, BOOL init)
{
#if CFG_TIMER_USE_INTERRUPT
    uint32 val;

    /* Init global variable system tick */
    if (init) {
        sys_ticks = 0;
        last_sys_ticks = 0;
        sys_seconds = 0;
    }

    val = TIMER0_TICK*clk_hz/2000;

    /*
     * Set up the exception vectors
     */
    um_setup_exceptions();

    um_irq_init();

    um_setup_counter_compare_irq(val, mips_counter_compapre_isr);
#else
    uint32 (*funcptr)(void) = (uint32 (*)(void))_getticks;

    if (init) {
        sys_ticks = 0;
        last_sys_ticks = 0;
        sys_seconds = 0;
        timer_remticks = 0;
        timer_oldcount = (*funcptr)();
    }
    /* Unit in 1ms. */
    clockspertick = clk_hz/1000;
#endif /* CFG_TIMER_USE_INTERRUPT */
}


void
sal_usleep(uint32 usec)
{
    tick_t curr, ticks;
    uint32 (*funcptr)(void) = (uint32 (*)(void))_getticks;
#if CFG_TIMER_USE_INTERRUPT  
    if (usec < TIMER0_TICK * 1000UL) {
        ticks = usec*TICKS_PER_US;
        curr = (*funcptr)();
        while((*funcptr)() - curr < ticks);
    } else {
        curr = sys_ticks;
        ticks = SAL_USEC_TO_TICKS(usec);
        while((sal_get_ticks() - curr) < ticks);
    }
#else

    ticks = usec*TICKS_PER_US;

    curr = (*funcptr)();

    while((*funcptr)() - curr < ticks);

#endif
}
#ifndef __BOOTLOADER__
void
sal_sleep(tick_t ticks)
{
    tick_t curr;
    if (ticks == 0) {
        return;
    }
    curr = sal_get_ticks();
    while(!SAL_TIME_EXPIRED(curr, ticks)) {
        POLL();
    }
}
#endif /* !__BOOTLOADER__ */
