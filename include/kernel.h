/*
 * $Id: kernel.h,v 1.6 Broadcom SDK $
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

#ifndef _KERNEL_H_
#define _KERNEL_H_

/* Tasking - Release CPU to other tasks */
extern void background_init(void) REENTRANT;
extern void background(void) REENTRANT;
#define POLL() background()

/* Tasking - Background task registration */
typedef void (*BACKGROUND_TASK_FUNC)(void *) REENTRANT;
extern void task_add(BACKGROUND_TASK_FUNC func, void *arg) REENTRANT;
extern void task_remove(BACKGROUND_TASK_FUNC func) REENTRANT;
extern void task_suspend(BACKGROUND_TASK_FUNC func) REENTRANT;
extern void task_resume(BACKGROUND_TASK_FUNC func) REENTRANT;


/* Timer registration/notification */
typedef void (*TIMER_FUNC)(void *) REENTRANT;
extern BOOL timer_add(TIMER_FUNC func, void *arg, uint32 usec) REENTRANT;
extern void timer_remove(TIMER_FUNC func) REENTRANT;

/* Link change registration/notification */
typedef void (*SYS_LINKCHANGE_FUNC)(uint16 port, BOOL link, void *arg) REENTRANT;
extern BOOL sys_register_linkchange(SYS_LINKCHANGE_FUNC func, void *arg) REENTRANT;
extern void sys_unregister_linkchange(SYS_LINKCHANGE_FUNC func) REENTRANT;

/* RX handler - prototype */
typedef enum {
    SYS_RX_INVALID,
    SYS_RX_NOT_HANDLED,
    SYS_RX_HANDLED,
    SYS_RX_HANDLED_AND_OWNED
} sys_rx_t;
typedef sys_rx_t (*SYS_RX_CBK_FUNC)(sys_pkt_t *pkt, void *cookie) REENTRANT;

/* RX handler - call to free packet buffer if SYS_RX_HANDLED_AND_OWNED */
extern void sys_rx_free_packet(sys_pkt_t *pkt) REENTRANT;

/* RX handler - registration
   Note: Do not register/unregister RX callbacks in RX callback function. */
#define SYS_RX_REGISTER_FLAG_ACCEPT_PKT_ERRORS      (1 << 0)
#define SYS_RX_REGISTER_FLAG_ACCEPT_TRUNCATED_PKT   (1 << 1)
extern sys_error_t sys_rx_register(
    SYS_RX_CBK_FUNC callback, 
    uint8 priority, 
    void *cookie,
    uint16 flags) REENTRANT;
extern sys_error_t sys_rx_unregister(SYS_RX_CBK_FUNC callback) REENTRANT;

/* Provide buffers to RX engine (at initialization phase) */
extern void sys_rx_add_buffer(uint8 *buffer, uint16 size) REENTRANT;

/* TX callback */
typedef void (*SYS_TX_CALLBACK)(sys_pkt_t *pkt, sys_error_t status) REENTRANT;

/* TX: if cbk is NULL, it will wait until completion */
extern sys_error_t sys_tx(sys_pkt_t *pkt, SYS_TX_CALLBACK cbk) REENTRANT;

#endif /* _KERNEL_H_ */
