/*
 * $Id: base.h,v 1.3 Broadcom SDK $
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

#ifndef _BOARDAPI_BASE_H_
#define _BOARDAPI_BASE_H_

/* Number of units (should bed defined in board.h) */
#define board_unit_count() BOARD_NUM_OF_UNITS

/* Max number of ports */
#define board_max_port_count() BOARD_MAX_NUM_OF_PORTS

/* Board name */
extern const char *board_name(void) REENTRANT;

/* Get the number of user ports */
extern uint8 board_uport_count(void) REENTRANT;

/* Convert user port to unit and chip internal logical port */
extern sys_error_t board_uport_to_lport(uint16 uport, 
                                          uint8 *unit, 
                                          uint8 *lport) REENTRANT;

/* Convert unit and chip internal logical port to user port */
extern sys_error_t board_lport_to_uport(uint8 unit, 
                                         uint8 lport, 
                                         uint16 *uport) REENTRANT;

/* Convert user port list to  chip internal logical port bitmap for specified unit */
extern sys_error_t board_uplist_to_lpbmp(uint8 *uplist, uint8 unit,
                                          pbmp_t *lpbmp) REENTRANT;

/* Convert unit and  chip internal logical port bitmap to user port list */
extern sys_error_t board_lpbmp_to_uplist(uint8 unit, pbmp_t lpbmp,
                                           uint8 *uplist) REENTRANT;

/* Get SOC instance by unit */
extern soc_switch_t *board_get_soc_by_unit(uint8 unit) REENTRANT;

/* Get phy driver by user port */
extern phy_driver_t *board_get_phy_drv(uint16 uport) REENTRANT;

#endif /* _BOARDAPI_BASE_H_ */
