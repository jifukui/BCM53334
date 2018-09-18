/*
 * $Id: boot.c,v 1.1 Broadcom SDK $
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
 */

#include "system.h"
#include "arm.h"

extern unsigned int app_text_start;
extern int app_text_length;
extern const unsigned char app_text[];

extern unsigned int app_text2_start;
extern int app_text2_length;
extern const unsigned char app_text2[];

extern unsigned int app_data_start;
extern int app_data_length;
extern const unsigned char app_data[];

/*@api
 * mos_boot
 *
 * @brief
 * Copy the ROM text/data to memory and execute
 *
 * @param=arg - priority to start any additional tasks at
 * @returns void
 *
 * @desc
 *
 */
int main(void)
{
    uint32 *dst, *src;
    int length;
    void (*text_base)() = (void (*)())(app_text_start);

    /* copy the text out */
    dst = (uint32 *)app_text_start;
    src = (uint32 *)app_text;
    length = 0;
    while (length != app_text_length) {
        *dst++ = *src++;
        length += 4;
    }

    dst = (uint32 *)app_text2_start;
    src = (uint32 *)app_text2;
    length = 0;
    while (length != app_text2_length) {
        *dst++ = *src++;
        length += 4;
    }

    /* copy the data out */
    dst = (uint32 *)app_data_start;
    src = (uint32 *)app_data;
    length = 0;
    while (length != app_data_length) {
        *dst++ = *src++;
        length += 4;
    }
    (*text_base)();
}

/* Dummy entries for linker */
void mos_exception(void) { }
void mos_fiq_handler(void) { }
#if 0
void mos_irq_handler(void) { }
void mos_rupt_yield(void) { }
void mos_lock_push_irq(void) { }
void mos_lock_pop_irq(void) { }
#endif