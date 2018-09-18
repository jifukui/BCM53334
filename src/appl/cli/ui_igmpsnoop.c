/*
 * $Id: ui_igmpsnoop.c,v 1.21 Broadcom SDK $
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
#include "appl/cli.h"
#include "utils/ui.h"
#include "utils/ports.h"
#include "appl/igmpsnoop.h"

#if (CFG_CLI_ENABLED && CFG_RXTX_SUPPORT_ENABLED && defined(CFG_SWITCH_MCAST_INCLUDED))

STATIC RES_CONST_DECL char *op_str[] = {"Insert","Delete","PortAdd",
                                        "PortRemove","PortsGet"};

void ui_igmpsnoop_init(void) REENTRANT;

STATIC void _ui_mc_grp(uint8 op) REENTRANT
{
    sys_error_t rv = 0;
    int8    i, byte_len;
    uint8   in_mac[6], *in_uplist;
    uint16  in_vid, in_uport;

    if (op == 0 || op > 5) {
        goto ui_mc_grp_cancel;
    }

    byte_len = board_uport_count()/8 + 
            ((board_uport_count()%8) ? 1 : 0);
    in_uplist = sal_malloc(byte_len);
    sal_memset(in_uplist, 0, byte_len);
    
    sal_printf("\nMcast Group %s:\n", op_str[op-1]);
    for(i=0; i<6; i++) {
        sal_printf("MAC[%bu]: ", i);
        if (ui_get_byte(&in_mac[i], NULL) != UI_RET_OK) {
            break;
        }
    }
    if (i != 6) {
        goto ui_mc_grp_cancel;
    }
    in_vid = 0;
    if (ui_get_word(&in_vid, "VLAN: ") != UI_RET_OK){
        goto ui_mc_grp_cancel;
    }
    
    if (op == 1){
        for (i = 0; i < byte_len; i++){
            sal_printf("LPlist[%bu]: ", i);
            if (ui_get_byte(&in_uplist[i], NULL) != UI_RET_OK) {
                break;
            }
        }
        if (i != byte_len) {
            goto ui_mc_grp_cancel;
        }
        rv = board_mcast_addr_add(&in_mac[0], in_vid, &in_uplist[0]);
        if (rv){
            goto ui_mc_grp_fail;
        }
    } else if (op == 3 || op == 4){
        if (ui_get_word(&in_uport, "LPort: ") != UI_RET_OK) {
            goto ui_mc_grp_cancel;
        }
        if (op == 3){
            rv = board_mcast_port_add(&in_mac[0], in_vid, in_uport);
            if (rv){
                goto ui_mc_grp_fail;
            }
        } else if (op == 4){
            rv = board_mcast_port_remove(&in_mac[0], in_vid, in_uport);
            if (rv){
                goto ui_mc_grp_fail;
            }
        }
    } else {
        if (op == 2){
            rv = board_mcast_addr_remove(&in_mac[0], in_vid);
            if (rv){
                goto ui_mc_grp_fail;
            }
        } else if (op == 5){
            rv = board_mcast_port_get(&in_mac[0], in_vid, &in_uplist[0]);
            if (rv){
                goto ui_mc_grp_fail;
            }
            sal_printf("\nGroup's uplist= 0x");
            for (i = byte_len; i > 0; i--){
                sal_printf("%02bx",in_uplist[byte_len-1]);
            }
        }
    }
    sal_printf("\nDone!\n");
    return;
    
ui_mc_grp_cancel:
    sal_printf("\nCancelled.\n");
    return;
ui_mc_grp_fail:
    if (rv == SYS_ERR_EXISTS || rv == SYS_ERR_NOT_FOUND){
        sal_printf("\n[MAC:%02bx-%02bx-%02bx-%02bx-%02bx-%02bx,VID:0x%x] >> %s!\n",
                in_mac[0],in_mac[1],in_mac[2],in_mac[3],in_mac[4],in_mac[5],
                in_vid, (rv == SYS_ERR_EXISTS) ? "EXISTED" : "NOT_FOUND");
    } else {
        sal_printf("\nOP(%bd) Failed.rv=%d\n",op,(int16)rv);
    }
    return;

}

STATICCBK void
cli_cmd_igmpsnoop(CLI_CMD_OP op) REENTRANT
{    
    uint8 uplist[MAX_UPLIST_WIDTH], enable;
    uint16 count, i, vid, j;
    uint32 gip;
    char c;
    igmp_dbg_count_t dbg_count;
    uint16 tick_timer;
    uint16 router_port_tick_timer;

    if (op == CLI_CMD_OP_HELP) {
        sal_printf("Command to configure/display IGMP Snoop status\n");
    } else if (op == CLI_CMD_OP_DESC) {
        sal_printf("IGMP Snoop");
    } else {
        sal_printf("  d - Dump information\n"
                   "  E - Enable configuration\n"
                   "  D - Disable configuration\n"
                   "  R - Reset igmp debug counter\n"
                   "  M - Mcast Group configuration\n"
                   "Enter your choice: ");
        c = sal_getchar();
        sal_putchar('\n');
        if (c == 'd') {
            igmpsnoop_enable_get(&enable);
            sal_printf("\nThe status of IGMP Snoop is %d\n", (int)enable);

            board_block_unknown_mcast_get(&enable);
            sal_printf("The status of block unknown mcast is %d\n", (int)enable);

            igmpsnoop_dbg_timer_get(&tick_timer, &router_port_tick_timer);
            sal_printf("\nigmp debug timer :\n");
            sal_printf("    tick_timer=%d router_port_tick_timer=%d\n",
                       (int)tick_timer, (int)router_port_tick_timer);
            igmpsnoop_dbg_count_get(&dbg_count);
            sal_printf("\nigmp debug count :\n");
            sal_printf("    total_receive=%d query=%d leave=%d\n",
                       (int)dbg_count.total_receive, (int)dbg_count.query, 
                       (int)dbg_count.leave);
            sal_printf("    report_v1=%d report_v2=%d report_v3=%d\n",
                       (int)dbg_count.report_v1, (int)dbg_count.report_v2, 
                       (int)dbg_count.report_v3);
            sal_printf("    unkonwn_igmp_type=%d unkonwn_igmpv3_record_type=%d non_igmp=%d\n",
                       (int)dbg_count.unkonwn_igmp_type,
                       (int)dbg_count.unkonwn_igmpv3_record_type,
                       (int)dbg_count.non_igmp);
            sal_printf("    relay_query=%d relay_specific_query=%d relay_report_and_leave=%d\n",
                       (int)dbg_count.relay_query, 
                       (int)dbg_count.relay_specific_query, 
                       (int)dbg_count.relay_report_and_leave);
            sal_printf("    relay_success=%d relay_fail=%d\n",
                       (int)dbg_count.relay_success, 
                       (int)dbg_count.relay_fail);
            sal_printf("    database_update_fail=%d leave_q_operation_fail=%d\n",
                       (int)dbg_count.database_update_fail,
                       (int)dbg_count.leave_q_operation_fail);

            count = igmpsnoop_dynamic_router_port_count_get();
            sal_printf("\nDynamic router port count is %d\n", count);
            if(count) {
                sal_printf("VID         UPLIST\n");
                sal_printf("=======================================\n");
                for(i = 0 ; i < count ; i++) {
                    if (igmpsnoop_dynamic_router_port_get(
                                  i, &vid, uplist) == FALSE) {
                        sal_printf("igmpsnoop_dynamic_router_port_get failed "
                                   "with index %d in cli_cmd_igmpsnoop\n",
                                   (int)i);
                    }
                    sal_printf("%d        ", vid);
                    for (j = 0 ; j < MAX_UPLIST_WIDTH ; j++) {
                        sal_printf("  0x%02bx", uplist[j]);
                    }
                    sal_printf("\n");
                }
            }

            count = igmpsnoop_group_count_get();
            sal_printf("\nGroup count is %d\n", count);
            if(count) {
                sal_printf("GIP           VID         UPLIST\n");
                sal_printf("==========================================\n");
                for(i = 0 ; i < count ; i++) {
                    if (igmpsnoop_group_member_get(
                                  i, &gip, &vid, uplist) == FALSE) {
                        sal_printf("igmpsnoop_group_member_get failed "
                                   "with index %d in cli_cmd_igmpsnoop\n", (int)i);
                    }
                    sal_printf("0x%08lx     %d        ", gip, vid);
                    for (j = 0 ; j < MAX_UPLIST_WIDTH ; j++) {
                        sal_printf("  0x%02bx", uplist[j]);
                    }
                    sal_printf("\n");
                }
            }
            return;
        }
        if (c == 'E') {
            sal_printf("  0 - Enable IGMP Snoop\n"
                       "  1 - Enable block unknow mcast\n"
                       "Enter your choice: ");
            c = sal_getchar();
            if (c == '0') {
                if (igmpsnoop_enable_set(TRUE) == FALSE) {
                    sal_printf("igmpsnoop_enable_set "
                               "failed in cli_cmd_igmpsnoop\n");
                }
            } else if (c == '1') {
                board_block_unknown_mcast_set(TRUE);
            }
            return;
        }
        if (c == 'D') {
            sal_printf("  0 - Disable IGMP Snoop\n"
                       "  1 - Disable block unknow mcast\n"
                       "Enter your choice: ");
            c = sal_getchar();
            if (c == '0') {
                if (igmpsnoop_enable_set(FALSE) == FALSE) {
                    sal_printf("igmpsnoop_enable_set " 
                               "failed in cli_cmd_igmpsnoop\n");
                }
            } else if (c == '1') {
                board_block_unknown_mcast_set(FALSE);
            }
            return;
        }
        if (c == 'R') {
            igmpsnoop_dbg_count_reset();
            return;
        }
        if (c == 'M') {
            uint8   op = 0;
            sal_printf("  1 - Group insert\n"
                        "  2 - Group delete\n"
                        "  3 - Group port add\n"
                        "  4 - Group port remove\n"
                        "  5 - Group ports get\n"
                        "Enter choice: ");
            c = sal_getchar();
            if (c == '1'){
                op = 1;
            }
            if (c == '2'){
                op = 2;
            }
            if (c == '3'){
                op = 3;
            }
            if (c == '4'){
                op = 4;
            }
            if (c == '5'){
                op = 5;
            }
            _ui_mc_grp(op);
            return;
        }
    }
}

void 
ui_igmpsnoop_init(void) REENTRANT
{
    cli_add_cmd('I', cli_cmd_igmpsnoop);
}

#endif /*  (CFG_CLI_ENABLED && CFG_RXTX_SUPPORT_ENABLED && defined(CFG_SWITCH_MCAST_INCLUDED)) */
