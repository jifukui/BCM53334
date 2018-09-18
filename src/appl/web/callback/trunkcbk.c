/*
 * $Id: trunkcbk.c,v 1.17 Broadcom SDK $
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
#include "appl/ssp.h"
#include "appl/persistence.h"

#include "../content/sspmacro_ports.h"
#include "../content/sspmacro_trunk.h"
#include "../content/errormsg_htm.h"

#define TRUNK_DEBUG   0

#define MAX_TRUNK_GROUP_NUM  BOARD_MAX_NUM_OF_LAG

#if TRUNK_DEBUG
#define TRUNK_DBG(x)    do { sal_printf("TRUNK: "); sal_printf x; sal_printf("\n"); } while(0);
#else
#define TRUNK_DBG(x)
#endif

#define MAC_SA          1
#define MAC_DA          2
#define MAC_SA_DA       3

#define NO_LACP

SSPLOOP_RETVAL
ssploop_trunk_tag_crishow(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_LAG_INCLUDED
    if (index == 0) {
        switch(params[0]) {
        case SSPMACRO_TRUNK_CRI_SA:
            if (MAC_SA) {
                return SSPLOOP_STOP;
            }
            break;
        case SSPMACRO_TRUNK_CRI_DA:
            if (MAC_DA) {
                return SSPLOOP_STOP;
            }
            break;
        case SSPMACRO_TRUNK_CRI_SADA:
            if (MAC_SA_DA) {
                return SSPLOOP_PROCEED;
            }
            break;
        }
    }
#endif /* CFG_SWITCH_LAG_INCLUDED */

    return SSPLOOP_STOP;
}



SSPLOOP_RETVAL
ssploop_trunk_tag_half_groups(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_LAG_INCLUDED
    if (index < ((MAX_TRUNK_GROUP_NUM + 1)/2)) {
        return SSPLOOP_PROCEED;
    }
#endif /* CFG_SWITCH_LAG_INCLUDED */

    return SSPLOOP_STOP;
}

SSPLOOP_RETVAL
ssploop_trunk_tag_all_groups(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_LAG_INCLUDED
    if (index < MAX_TRUNK_GROUP_NUM) {
        return SSPLOOP_PROCEED;
    }
#endif /* CFG_SWITCH_LAG_INCLUDED */
    
   return SSPLOOP_STOP;
}

SSPLOOP_RETVAL
ssploop_trunk_tag_stgroups(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_LAG_INCLUDED
    if (index < MAX_TRUNK_GROUP_NUM) {
        return SSPLOOP_PROCEED;
    }
#endif /* CFG_SWITCH_LAG_INCLUDED */
    
   return SSPLOOP_STOP;
}

SSPLOOP_RETVAL
ssploop_trunk_tag_lacp(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
    if (index == 0) {
        if (params[0] == SSPMACRO_TRUNK_ENABLE) {
#ifndef NO_LACP
            return SSPLOOP_PROCEED;
#endif /* !NO_LACP */
        }
    }

    return SSPLOOP_STOP;
}

void
sspvar_trunk_tag_lacp(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
    switch(params[0]) {

    case SSPMACRO_TRUNK_ENABLE:
        ret->type = SSPVAR_RET_INTEGER;
        ret->val_data.integer = 0;
#ifndef NO_LACP
        ret->val_data.integer = is_lacp_group(psmem, params[1]);
#endif /* !NO_LACP */
        break;

    case SSPMACRO_TRUNK_ALL_GROUP:
        ret->type = SSPVAR_RET_INTEGER;
        ret->val_data.integer = params[1] + 1;
#ifndef NO_LACP
        {
            int *p;
            p = (int *)ssputil_psmem_get(psmem, ssploop_trunk_tag_stgroups);
            if (p != NULL) {
                ret->val_data.integer = *p + 1;
            }
        }
#endif /* !NO_LACP */
        break;
    default:
          break;

    }
}

void sspvar_trunk_tag_info(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_LAG_INCLUDED
    sys_error_t rv = SYS_OK;
    int i = 0;
    uint8 uplist_get[MAX_UPLIST_WIDTH];
    uint8 enable = TRUE;
    SSPTAG_PARAM group;
    uint16 uport;

    switch (params[0]) {
      case SSPMACRO_TRUNK_HALF_GROUP:
        TRUNK_DBG(("SSPMACRO_TRUNK_HALF_GROUP\n"));
        if (params[2] == SSPMACRO_TRUNK_LEFT_HALF) {
            ret->val_data.integer = params[1] + 1;
            TRUNK_DBG(("params[1]+1:%d\n", params[1] + 1));
        } else {
            ret->val_data.integer = params[1] + 1 + ((MAX_TRUNK_GROUP_NUM + 1)/2);
            TRUNK_DBG(("params[1]+1:%d\n", ret->val_data.integer));

        }
        ret->type = SSPVAR_RET_INTEGER;
        break;

     case SSPMACRO_TRUNK_TRUNK_STRING:
        if (params[2] == SSPMACRO_TRUNK_LEFT_HALF) {
                ret->val_data.integer = params[1] + 1;
        } else {
                ret->val_data.integer = params[1] + 1 + ((MAX_TRUNK_GROUP_NUM + 1)/2);
        }
        ret->type = SSPVAR_RET_STRING;
        if (ret->val_data.integer == (MAX_TRUNK_GROUP_NUM + 1)) {
            /* trunk trunk_group_num + 1 will not be shown */
                ret->val_data.string = "&nbsp;";
        } else {
            sal_sprintf(ssputil_shared_buffer, "Trunk %2d", ret->val_data.integer);
                ret->val_data.string = ssputil_shared_buffer;
        }
        break;

    case SSPMACRO_TRUNK_ALL_GROUP:
        ret->val_data.integer = params[1] + 1;
        ret->type = SSPVAR_RET_INTEGER;
        break;

    case SSPMACRO_TRUNK_GROUP_MEMBER_STRING:
        sal_sprintf(ssputil_shared_buffer, "");
        sal_memset(&uplist_get[0], 0, sizeof(uint8)*MAX_UPLIST_WIDTH);

           /* get trunk member from trunk manager */
        group = params[1];  // trunk group
        uport = SAL_ZUPORT_TO_UPORT(params[2]);  // port number
        if (group == 65535) {
           for (i=0 ; i < MAX_TRUNK_GROUP_NUM ; i++) {
                rv = board_lag_group_get((uint8)i + 1, &enable, &uplist_get[0]);

                if (rv == SYS_OK) {
                    if (uplist_port_matched(uplist_get, uport) == SYS_OK) {
                       break;
                    }
                }
           }

           if (i == MAX_TRUNK_GROUP_NUM )
               sal_sprintf(ssputil_shared_buffer, "checked=\"checked\"");
        } else {
               rv = board_lag_group_get((uint8)group+1, &enable, &uplist_get[0]);
               if (rv == SYS_OK) {
                   if (uplist_port_matched(uplist_get, uport) == SYS_OK) {
                       sal_sprintf(ssputil_shared_buffer, "checked=\"checked\"");
                   }
               }
        }


        TRUNK_DBG(("sspvar_trunk_tag_info SSPMACRO_TRUNK_GROUP_MEMBER_STRING: %d %s\n", i, ssputil_shared_buffer));
        ret->type = SSPVAR_RET_STRING;
        ret->val_data.string = ssputil_shared_buffer;
        break;

    case SSPMACRO_TRUNK_CURCRI:
        ret->type = SSPVAR_RET_INTEGER;
        ret->val_data.integer = MAC_SA_DA;
        break;

    case SSPMACRO_TRUNK_MAX:
        ret->type = SSPVAR_RET_INTEGER;
        ret->val_data.integer =  BOARD_MAX_PORT_PER_LAG; //ports_per_trunk_num;
        break;

    case SSPMACRO_TRUNK_MAX_GROUP:
        ret->type = SSPVAR_RET_INTEGER;
        ret->val_data.integer = MAX_TRUNK_GROUP_NUM;
        break;

    case SSPMACRO_TRUNK_TYPE:
        ret->type = SSPVAR_RET_NULL;
        break;
    case SSPMACRO_TRUNK_MAX_PORT:
        ret->type = SSPVAR_RET_INTEGER;
        ret->val_data.integer = board_uport_count();
        break;
    default:
        TRUNK_DBG(("**** sspvar_trunk_tag_info: params[0]=%d NOT FOUND",
                  params[0]));
        ret->type = SSPVAR_RET_NULL;
    }
    return;
#endif /* CFG_SWITCH_LAG_INCLUDED */
}

SSPLOOP_RETVAL
ssploop_trunk_tag_ports(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_LAG_INCLUDED
    if (index < board_uport_count()) {
        return SSPLOOP_PROCEED;
    }
#endif /* CFG_SWITCH_LAG_INCLUDED */
    return SSPLOOP_STOP;
}

void
sspvar_trunk_tag_portnum(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
    int uport = SAL_ZUPORT_TO_UPORT(params[0]);

    ret->type = SSPVAR_RET_STRING;
    sal_sprintf(ssputil_shared_buffer, "%02d", SAL_UPORT_TO_NZUPORT(uport));
    ret->val_data.string = ssputil_shared_buffer;
}
SSP_HANDLER_RETVAL
ssphandler_trunk_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_LAG_INCLUDED
    uint8 uplist[MAX_TRUNK_GROUP_NUM+1][MAX_UPLIST_WIDTH];
    uint8 enable_list[MAX_TRUNK_GROUP_NUM+1], lag_enable;
    uint16 i, trunk_num;
    uint16 uport;
#ifdef CFG_SWITCH_MIRROR_INCLUDED
    uint16 j, mirror_to_port;
    uint8 mirror_port_enable;
#endif /* CFG_SWITCH_MIRROR_INCLUDED */
#ifdef CFG_SWITCH_VLAN_INCLUDED
    uint16 vlan_id, k;
    vlan_type_t vlan_type;
    uint8 vlan_uplist[MAX_UPLIST_WIDTH];
    uint8 vlan_tag_uplist[MAX_UPLIST_WIDTH];
    uint8 tmp_uplist[MAX_UPLIST_WIDTH];
    uint16 trunk_vlan_id;
#endif /* CFG_SWITCH_VLAN_INCLUDED */

    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {

        /* Initialize trunk enable list  and trunk-port bit map*/
        for (i=0; i<= MAX_TRUNK_GROUP_NUM; i++) {
            enable_list[i] = FALSE;
        }
        sal_memset(uplist, 0, sizeof(uplist));
        /* Parse feedback trunk string information */
        SAL_UPORT_ITER(i) {
             if (SAL_UPORT_TO_NZUPORT(i) >= (cxt->count)) {
                 break;
             }         
             trunk_num = sal_atoi(cxt->pairs[SAL_UPORT_TO_NZUPORT(i)].value);
             uport = SAL_NZUPORT_TO_UPORT(sal_atoi(cxt->pairs[SAL_UPORT_TO_NZUPORT(i)].name+1)); 
             if (!SAL_UPORT_IS_NOT_VALID(uport)) {
                if ((trunk_num > 0) && (trunk_num <= MAX_TRUNK_GROUP_NUM)) {
                   uplist_port_add(&uplist[trunk_num][0], uport);
                   enable_list[trunk_num] = TRUE;
                }
             }
        }
#ifdef CFG_SWITCH_VLAN_INCLUDED
        board_vlan_type_get(&vlan_type);
        if (vlan_type == VT_PORT_BASED) {
            for (i=0, vlan_id = 1; (i < board_vlan_count()) && (vlan_id < board_uport_count()); vlan_id++) {
                 if (board_pvlan_port_get(vlan_id, vlan_uplist) == SYS_OK) {
                     i++;
                     /* all trunk port should belong to the same one port vlan */
                     for (k=1; k <= MAX_TRUNK_GROUP_NUM; k++) {
                          /* To check the web uplist is belong to single vlan */
                          uplist_manipulate(tmp_uplist, vlan_uplist, UPLIST_OP_COPY);
                          uplist_manipulate(tmp_uplist, &uplist[k][0], UPLIST_OP_AND);
                          if ((uplist_manipulate(tmp_uplist, &uplist[k][0], UPLIST_OP_EQU) != SYS_OK) &&
                              (uplist_is_empty(tmp_uplist) != SYS_OK)) {
                              webutil_show_error(
                                           cxt, psmem,
                                           "TRUNK",
                                           "Ports of a trunk group should not be partially assigned to any single VLAN or across more than 2 VLANs.",
                                           err_button_retry,
                                           err_action_back);
                                           /* We don't want to process it more */
                                           /* cxt->flags = 0; */
                                           return SSP_HANDLER_RET_MODIFIED;
                          }
                     }
                 }
             }
      
        } else if (vlan_type == VT_DOT1Q) {
                /* all trunk port should belong to the same one port vlan */
                for (i=0, vlan_id = 1; (i < board_vlan_count()) && (vlan_id < 4095); vlan_id++) {
                     if (board_qvlan_port_get(vlan_id, vlan_uplist, vlan_tag_uplist) == SYS_OK) {
                         i++;
                         /* all trunk port should belong to the same one port vlan for each trunk*/
                         for (k=1; k <= MAX_TRUNK_GROUP_NUM; k++) {
                              /* To check the web uplist is belong to single vlan */
                              uplist_manipulate(tmp_uplist, vlan_uplist, UPLIST_OP_COPY);
                              uplist_manipulate(tmp_uplist, &uplist[k][0], UPLIST_OP_AND);
                              if ((uplist_manipulate(tmp_uplist, &uplist[k][0], UPLIST_OP_EQU) != SYS_OK) &&
                                  (uplist_is_empty(tmp_uplist) != SYS_OK)) {
                                  webutil_show_error(
                                               cxt, psmem,
                                               "TRUNK",
                                                "Ports of a trunk group should not be partially assigned to any single VLAN or across more than 2 VLANs.",                                 err_button_retry,
                                               err_action_back);
                                               /* We don't want to process it more */
                                               /* cxt->flags = 0; */
                                               return SSP_HANDLER_RET_MODIFIED;
                              }
                         }
                     }
                }

                /* per port PVID check */
                for (k=1; k <= MAX_TRUNK_GROUP_NUM; k++) {
                     /* Initial trunk VID is zero that means not valid */
                     trunk_vlan_id = 0; 
                     SAL_UPORT_ITER(i) {
                          if (uplist_port_matched(&uplist[k][0], i) == SYS_OK) {
                               board_untagged_vlan_get(i, &vlan_id);
                               if (trunk_vlan_id == 0) {
                                   trunk_vlan_id = vlan_id;
                               } else {
                                   if (trunk_vlan_id != vlan_id) {
                                       webutil_show_error(
                                             cxt, psmem,
                                             "TRUNK & PORT",
                                              "Ports belong to a trunk group should have the same PVID.",  
                                              err_button_retry,
                                              err_action_back);
                                             /* We don't want to process it more */
                                             /* cxt->flags = 0; */
                                             return SSP_HANDLER_RET_MODIFIED;
                                   }                 
                               }
                          }
                     }     
                }
        }
#endif /* CFG_SWITCH_VLAN_INCLUDED */

#ifdef CFG_SWITCH_MIRROR_INCLUDED
        
        /* find which trunk contains mirror to port  */
        if (board_mirror_to_get(&mirror_to_port) == SYS_OK) {
            for (i=1; i <= MAX_TRUNK_GROUP_NUM; i++) {
                if (enable_list[i] == TRUE) {
                    if (uplist_port_matched(&uplist[i][0], mirror_to_port) == SYS_OK) {
                        /* check if there is any mirror port in this trunk*/
                        SAL_UPORT_ITER(j) {
                            mirror_port_enable = 0;
                            board_mirror_port_get(j,&mirror_port_enable);
                            if (mirror_port_enable) {
                                if (uplist_port_matched(&uplist[i][0],j) == SYS_OK) {
                                    webutil_show_error(
                                         cxt, psmem,
                                         "TRUNK",
                                         "Please check Mirror page setting. <br></br>The mirror-to port and mirror ports can not belong to the same trunk group.",
                                          err_button_retry,
                                          err_action_back);
                                    /* We don't want to process it more */
                                    /* cxt->flags = 0; */
                                   return SSP_HANDLER_RET_MODIFIED;
                                }
                            }
                       }
                     }
                }
            }
        }
#endif /* CFG_SWITCH_MIRROR_INCLUDED */

        lag_enable = FALSE;
        for (i=1; i<=MAX_TRUNK_GROUP_NUM; i++) {
            if (enable_list[i] == TRUE) {
                lag_enable = TRUE;
                break;
            }
        }

        if (lag_enable == FALSE) {
            board_lag_set(FALSE);
            for (i=1; i<=MAX_TRUNK_GROUP_NUM; i++) {
                 board_lag_group_set(i, FALSE, &uplist[i][0]);
            }
        } else {
            board_lag_set(TRUE);
            for (i=1; i<=MAX_TRUNK_GROUP_NUM; i++) {
                if (enable_list[i] == TRUE) {
                    board_lag_group_set((uint8) i, enable_list[i], &uplist[i][0]);
                } else {
                    board_lag_group_set((uint8) i, enable_list[i], &uplist[i][0]);
                }
            }

        }
    }

#if CFG_PERSISTENCE_SUPPORT_ENABLED
    persistence_save_current_settings("lag");
#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */
    return SSP_HANDLER_RET_MODIFIED;
#else
    return SSP_HANDLER_RET_INTACT;
#endif /* CFG_SWITCH_LAG_INCLUDED */
}


