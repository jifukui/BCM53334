/*
 * $Id: timer.c,v 1.10 Broadcom SDK $
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

#ifdef __C51__
#ifdef CODE_USERCLASS
#pragma userclass (code = krntime)
#endif /* CODE_USERCLASS */
#endif /* __C51__ */

#include "system.h"

#if CFG_TIMER_CALLBACK_SUPPORT

struct timer_t {
    tick_t      interval;   /* In ticks */
    tick_t      start;      /* In ticks */
    TIMER_FUNC  cbk;
    void *      arg;
    BOOL        inprogress; /* To avoid reentrance */
};

STATIC struct timer_t timer[CFG_MAX_REGISTERED_TIMERS];
STATIC tick_t previous_tick;

/* Forwards */
APISTATIC void sys_timer_task(void *param) REENTRANT;

extern void sys_timer_init(void) REENTRANT;

APISTATIC void
APIFUNC(sys_timer_task)(void *param) REENTRANT
{
    /* Check and run timers only once per tick */
    if (previous_tick != sal_get_ticks()) {

        uint8 i;
        TIMER_FUNC cbk;
        tick_t curr;
        struct timer_t *pt = &timer[0];

        UNREFERENCED_PARAMETER(param);

        for(i=0; i<CFG_MAX_REGISTERED_TIMERS; i++, pt++) {
            cbk = pt->cbk;
            if (cbk == NULL) {
                break;
            }
            if (!pt->inprogress) {
                curr = sal_get_ticks();

                if (pt->interval == 0 ||
                    curr - pt->start >= pt->interval) {

                    pt->inprogress = TRUE;
                    (*cbk)(pt->arg);

                    /* Double check in case it's removed during execution */
                    if (pt->cbk == cbk) {
                        pt->inprogress = FALSE;
                        pt->start = curr;
                    }
                }
            }
        }

        previous_tick = sal_get_ticks();
    }
}

void
APIFUNC(sys_timer_init)(void) REENTRANT
{
    uint8 i;
    for(i=0; i<CFG_MAX_REGISTERED_TIMERS; i++) {
        timer[i].cbk = NULL;
    }

    /* Get current ticks */
    previous_tick = sal_get_ticks();

    /* Register background process for handling timer callback */
    task_add(sys_timer_task, NULL);
}

BOOL
APIFUNC(timer_add)(TIMER_FUNC func, void *arg, uint32 usec) REENTRANT
{
    uint8 i;
    tick_t curr = sal_get_ticks();

    SAL_DEBUGF(("timer_add: %p\n", func));
    if (func == NULL) {
        return FALSE;
    }

    /* Check if it has been registered */
    for(i=0; i<CFG_MAX_REGISTERED_TIMERS; i++) {
        if (timer[i].cbk != NULL) {

            if (func == timer[i].cbk) {

                /* Already registered */
                timer[i].interval = usec / sal_get_us_per_tick();
                timer[i].start = curr;
                timer[i].arg = arg;

                return TRUE;
            }
        }
    }

    /* Found an empty slot */
    for(i=0; i<CFG_MAX_REGISTERED_TIMERS; i++) {
        if (timer[i].cbk == NULL) {

            timer[i].cbk = func;
            timer[i].interval = usec / sal_get_us_per_tick();
            timer[i].start = curr;
            timer[i].arg = arg;
            timer[i].inprogress = FALSE;

            return TRUE;
        }
    }

    return FALSE;
}
void
APIFUNC(timer_remove)(TIMER_FUNC func) REENTRANT
{
    uint8 i;
    SAL_DEBUGF(("timer_remove: %p\n", func));
    for(i=0; i<CFG_MAX_REGISTERED_TIMERS; i++) {
        if (timer[i].cbk == func) {
            break;
        }
    }

    if (i == CFG_MAX_REGISTERED_TIMERS) {
        return;
    }

    for (; i < CFG_MAX_REGISTERED_TIMERS - 1; i++) {
        timer[i].cbk = timer[i + 1].cbk;
        timer[i].interval = timer[i + 1].interval;
        timer[i].start = timer[i + 1].start;
        timer[i].arg = timer[i + 1].arg;
        timer[i].inprogress = timer[i + 1].inprogress;
    }
    timer[i].cbk = NULL;
}

#endif /* CFG_TIMER_CALLBACK_SUPPORT */

