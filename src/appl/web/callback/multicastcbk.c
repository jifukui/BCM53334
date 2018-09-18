/*
 * $Id: multicastcbk.c,v 1.14 Broadcom SDK $
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
#include "brdimpl/vlan.h"

#include "appl/ssp.h"
#include "utilcbk.h"
#include "appl/persistence.h"
#include "appl/igmpsnoop.h"

#include "../content/sspmacro_multicast.h"
#include "../content/sspmacro_pvlan.h"
#include "../content/multicast_htm.h"
#include "../content/errormsg_htm.h"


#define MULTICAST_DEBUG 0

#if MULTICAST_DEBUG
#define MULTICAST_DBG(x)    do { sal_printf("MULTICAST: "); \
                       sal_printf x; sal_printf("\n");\
                       } while(0);
#else
#define MULTICAST_DBG(x)
#endif




SSPLOOP_RETVAL
ssploop_multicast_tag_vlan(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_VLAN_INCLUDED
    if (index < board_vlan_count()) {
        return SSPLOOP_PROCEED;
    }
#endif
    return SSPLOOP_STOP;

}

#if defined(CFG_SWITCH_MCAST_INCLUDED)

static SSP_HANDLER_RETVAL
show_error(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem, int error) {

    webutil_show_error(
        cxt, psmem,
        "MULTICAST",
        "System error information!!",
        err_button_retry,
        err_action_back
        );

    /* We don't want to process it more */
    /* cxt->flags = 0; */
    return SSP_HANDLER_RET_MODIFIED;
}


void
sspvar_multicast_tag_ssi(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
    uint8 enable = FALSE;
#ifdef CFG_SWITCH_VLAN_INCLUDED
    vlan_type_t vlan_type;
    uint16 vid;
#endif
    ret->type = SSPVAR_RET_STRING;

    sal_strcpy(ssputil_shared_buffer, "");

    switch (params[0]) {
      case SSPMACRO_MULTICAST_IGMPSPMD:

          igmpsnoop_enable_get(&enable);

          if (enable) {
              sal_strcpy(ssputil_shared_buffer, "checked");
          }
          break;
      case SSPMACRO_MULTICAST_VLAN_ID_ENABLE:
#ifdef CFG_SWITCH_VLAN_INCLUDED
          if (board_vlan_type_get(&vlan_type) != SYS_OK) {
              sal_sprintf(ssputil_shared_buffer, "%d", 0);
          } else {
              sal_sprintf(ssputil_shared_buffer, "%d", 1);
          }
#else
          sal_sprintf(ssputil_shared_buffer, "%d", 0);
#endif
          break;
#ifdef CFG_SWITCH_VLAN_INCLUDED
      case SSPMACRO_MULTICAST_VLAN_ID_SELECT:
          igmpsnoop_enable_get(&enable);
          if (enable) {
              igmpsnoop_vid_get(&vid);
          } else {
              vid = igmpsnoop_get_vid_by_index(0);
          }

           sal_sprintf(ssputil_shared_buffer, "%d", vid);
      break;
      case SSPMACRO_MULTICAST_VLAN_RANGE:
           if (board_vlan_type_get(&vlan_type) != SYS_OK) {
               break;
           }
           if (vlan_type == VT_PORT_BASED) {
               sal_sprintf(ssputil_shared_buffer, " (1-%d)", board_uport_count());
           } else if (vlan_type == VT_DOT1Q) {
               sal_sprintf(ssputil_shared_buffer, " (1-4094)");
           }
      break;
#endif
      case SSPMACRO_MULTICAST_UNKNOWMT:

          board_block_unknown_mcast_get(&enable);
          if (enable) {
              sal_strcpy(ssputil_shared_buffer, "checked");
          }
          break;
      default:
          break;
    }



    ret->val_data.string = ssputil_shared_buffer;
}


SSP_HANDLER_RETVAL
ssphandler_multicast_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    sys_error_t rv = SYS_OK;

    uint8 igmp = TRUE;

    uint8 unknowmlti = TRUE;

    MULTICAST_DBG(("__ssphandler_multicast_cgi\n"));

    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {

        MULTICAST_DBG(("__ssphandler_multicast_cgi:pairs[0]: %s", cxt->pairs[0].value));
        MULTICAST_DBG(("__ssphandler_multicast_cgi:pairs[1]: %s", cxt->pairs[1].value));
        MULTICAST_DBG(("__ssphandler_multicast_cgi:pairs[2]: %s", cxt->pairs[2].value));
        igmp = sal_atoi(cxt->pairs[0].value);
        rv = igmpsnoop_enable_set(igmp);
#ifdef CFG_SWITCH_VLAN_INCLUDED
        if (igmp) {
            rv = igmpsnoop_vid_set(sal_atoi(cxt->pairs[2].value));
            if (rv == SYS_ERR_NOT_FOUND) {
                webutil_show_error(
                    cxt, psmem,
                    "MULTICAST",
                    "IGMP VLAN ID setting fail. <br></br> The VLAN ID is not exist. <br></br>",
                    err_button_retry,
                    err_action_back
                );

                /* We don't want to process it more */
                /* cxt->flags = 0; */
                return SSP_HANDLER_RET_MODIFIED;

            } else if (rv != SYS_OK) {
                webutil_show_error(
                    cxt, psmem,
                    "MULTICAST",
                    "IGMP VLAN ID setting fail. <br></br> Please check VLAN setting. <br></br>",
                    err_button_retry,
                    err_action_back
                );

                /* We don't want to process it more */
                /* cxt->flags = 0; */
                return SSP_HANDLER_RET_MODIFIED;

            }
        }
#endif

#if CFG_PERSISTENCE_SUPPORT_ENABLED
        rv = persistence_save_current_settings("igmpsnoop");
#endif
        unknowmlti = sal_atoi(cxt->pairs[1].value);
        rv = board_block_unknown_mcast_set(unknowmlti);
#if CFG_PERSISTENCE_SUPPORT_ENABLED
        rv = persistence_save_current_settings("mcast");
#endif
    }

    if (rv < 0)
        return show_error(cxt, psmem, rv);
    else
        return SSP_HANDLER_RET_INTACT;
}
#endif /* defined(CFG_SWITCH_MCAST_INCLUDED) */


