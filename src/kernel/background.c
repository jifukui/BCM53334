/*
 * $Id: background.c,v 1.6 Broadcom SDK $
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
#pragma userclass (code = krnbg)
#endif /* CODE_USERCLASS */
#endif /* __C51__ */

#include "system.h"

enum {
  TASK_FLAG_SUSPEND = 0x01
} TASK_FLAG_T;

/*  *********************************************************************
    *  Globals
    ********************************************************************* */

STATIC BACKGROUND_TASK_FUNC bg_tasklist[CFG_MAX_BACKGROUND_TASKS];
STATIC void *bg_args[CFG_MAX_BACKGROUND_TASKS];
STATIC uint8 bg_task_flag[CFG_MAX_BACKGROUND_TASKS];



/*  *********************************************************************
    *  background_init()
    *
    *  Initialize the background task list
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void
APIFUNC(background_init)(void) REENTRANT
{
#if CFG_SAL_LIB_SUPPORT_ENABLED
    sal_memset(bg_tasklist, 0, sizeof(bg_tasklist));
#else /* !CFG_SAL_LIB_SUPPORT_ENABLED */
    int i;
    for (i=0; i < CFG_MAX_BACKGROUND_TASKS; i++) {
        bg_tasklist[i] = NULL;
        bg_task_flag[i] = 0;
    }
#endif /* CFG_SAL_LIB_SUPPORT_ENABLED */
}


/*  *********************************************************************
    *  task_add(func,arg)
    *
    *  Add a function to be called periodically in the background
    *  polling loop.
    *
    *  Input parameters:
    *      func - function pointer
    *      arg - arg to pass to function
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void
APIFUNC(task_add)(BACKGROUND_TASK_FUNC func, void *arg) REENTRANT
{
    int idx;

    SAL_ASSERT(func != NULL);
    SAL_DEBUGF(("task_add: %p\n", func));
    for (idx = 0; idx < CFG_MAX_BACKGROUND_TASKS; idx++) {
        if (bg_tasklist[idx] == NULL) {
            bg_tasklist[idx] = func;
            bg_args[idx] = arg;
            return;
        }
    }

    /* Too many background tasks */
    sal_assert("Too many tasks!", __FILE__, __LINE__);
}

/*  *********************************************************************
    *  task_suspend(func)
    *
    *  Put task into suspend state 
    *
    *  
    *      func - function pointer
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void
APIFUNC(task_suspend)(BACKGROUND_TASK_FUNC func) REENTRANT
{
    int idx;

    SAL_ASSERT(func != NULL);
    SAL_DEBUGF(("task_suspend: %p\n", func));
    for (idx = 0; idx < CFG_MAX_BACKGROUND_TASKS; idx++) {
        if (bg_tasklist[idx] == func) {
            bg_task_flag[idx] |= TASK_FLAG_SUSPEND;
            return;
        }
    }

}

/*  *********************************************************************
    *  task_resume(func)
    *
    *  To resume task 
    *
    *  Input parameters:
    *      func - function pointer
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void
APIFUNC(task_resume)(BACKGROUND_TASK_FUNC func) REENTRANT
{
    int idx;

    SAL_ASSERT(func != NULL);
    SAL_DEBUGF(("task_resume: %p\n", func));
    for (idx = 0; idx < CFG_MAX_BACKGROUND_TASKS; idx++) {
        if (bg_tasklist[idx] == func) {
            bg_task_flag[idx] &= (~TASK_FLAG_SUSPEND);
            return;
        }
    }

}


/*  *********************************************************************
    *  task_remove(func)
    *
    *  Remove a function from the background polling loop
    *
    *  Input parameters:
    *      func - function pointer
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

#ifndef __BOOTLOADER__
void
APIFUNC(task_remove)(BACKGROUND_TASK_FUNC func) REENTRANT
{
    int idx;

    SAL_ASSERT(func);
    SAL_DEBUGF(("task_remove: %p\n", func));
    for (idx = 0; idx < CFG_MAX_BACKGROUND_TASKS; idx++) {
        if (bg_tasklist[idx] == func) {
            break;
        }
    }

    if (idx == CFG_MAX_BACKGROUND_TASKS) {
        return;
    }

    for (; idx < CFG_MAX_BACKGROUND_TASKS - 1; idx++) {
        bg_tasklist[idx] = bg_tasklist[idx + 1];
        bg_args[idx] = bg_args[idx + 1];
    }

    bg_tasklist[idx] = NULL;
}
#endif /* !__BOOTLOADER__ */

/*  *********************************************************************
    *  background()
    *
    *  The main loop and other places that wait for stuff call
    *  this routine to make sure the background handlers get their
    *  time.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void
APIFUNC(background)(void) REENTRANT
{
    int idx;
    BACKGROUND_TASK_FUNC func;

    for (idx = 0; idx < CFG_MAX_BACKGROUND_TASKS; idx++) {
        func = bg_tasklist[idx];
        if (func == NULL) {
            break;
        }
        if (bg_task_flag[idx] & TASK_FLAG_SUSPEND) {
            continue;
        }
        (*func)(bg_args[idx]);
    }
}
