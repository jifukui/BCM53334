/*
 * $Id: igmpsnoop.h,v 1.18 Broadcom SDK $
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

#ifndef _IGMP_H_
#define _IGMP_H_
#include "system.h"

typedef struct igmp_dbg_count_s {
    uint16  total_receive;
    uint16  report_v1;
    uint16  report_v2;
    uint16  report_v3;
    uint16  leave;
    uint16  query;
    uint16  unkonwn_igmp_type;
    uint16  unkonwn_igmpv3_record_type;
    uint16  non_igmp;
    uint16  relay_report_and_leave;
    uint16  relay_query;
    uint16  relay_specific_query;
    uint16  relay_fail;
    uint16  relay_success;
    uint16  database_update_fail;
    uint16  leave_q_operation_fail;
    uint16  invalid_vlan_group;
} igmp_dbg_count_t;

extern uint16 IGMPsnoop_vid;
extern void igmpsnoop_dbg_timer_get(uint16 *tick_timer, uint16 *router_port_tick_timer);
extern void igmpsnoop_dbg_count_get(igmp_dbg_count_t *count);
extern void igmpsnoop_dbg_count_reset(void);

extern BOOL igmpsnoop_enable_set(uint8 enable);
extern void igmpsnoop_enable_get(uint8 *enable);
extern void igmpsnoop_database_init(void);
extern void igmpsnoop_vid_get(uint16 *vid);
extern sys_error_t igmpsnoop_vid_set(uint16 vid);
extern uint16 igmpsnoop_get_vid_by_index(int16 index);

#if (CFG_CLI_ENABLED)
extern uint16 igmpsnoop_group_count_get(void);
extern BOOL igmpsnoop_group_member_get(uint16 index,
                    uint32 *gip, uint16 *vid, uint8 *uplist);
extern uint16 igmpsnoop_dynamic_router_port_count_get(void);
extern BOOL igmpsnoop_dynamic_router_port_get(uint8 index,
                              uint16 *vid, uint8 *uplist);
#endif /* (CFG_CLI_ENABLED) */

#endif /* _IGMP_H_ */
