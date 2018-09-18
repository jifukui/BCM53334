/*
 * $Id: mirrorcbk.c,v 1.18 Broadcom SDK $
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
#include "appl/ssp.h"
#include "utilcbk.h"
#include "appl/persistence.h"

#include "../content/sspmacro_mirror.h"
#include "../content/sspmacro_ports.h"
#include "../content/sspmacro_pvlan.h"
#include "../content/mirror_htm.h"
#include "../content/errormsg_htm.h"

#define MIRROR_DEBUG 0

#if MIRROR_DEBUG
#define MIRROR_DBG(x)    do { sal_printf("MIRROR: "); \
                       sal_printf x; sal_printf("\n");\
                       } while(0);
#else
#define MIRROR_DBG(x)
#endif

#ifdef CFG_SWITCH_MIRROR_INCLUDED
#define MAX_PORT_NUM  board_uport_count()
#define NUM_PORTS_PER_SYSTEM MAX_PORT_NUM

static SSP_HANDLER_RETVAL
show_error(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem, char *error_msg) {

    webutil_show_error(
        cxt, psmem,
        "MIRROR",
        error_msg,
        err_button_retry,
        err_action_back
        );

    /* We don't want to process it more */
    /* cxt->flags = 0; */
    return SSP_HANDLER_RET_MODIFIED;
}
#endif /* CFG_SWITCH_MIRROR_INCLUDED */


void
sspvar_mirror_tag_ssi(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_MIRROR_INCLUDED
    uint8 enable = FALSE;
    uint16 tmp_uport, uport; /*uport: it stared from 0 to 23 number of port*/
    char *pbuf = NULL;

    ret->type = SSPVAR_RET_STRING;

    uport = SAL_ZUPORT_TO_UPORT(params[1]);

    switch (params[0]) {
      case SSPMACRO_MIRROR_VALUES:
          pbuf = ssputil_shared_buffer;
           /*deafault value*/

          SAL_UPORT_ITER(tmp_uport) {
              board_mirror_port_get(tmp_uport, &enable);
              *pbuf = enable + 48;
              pbuf++;

              *pbuf  = ',';
              pbuf++;
          }
          *(--pbuf)='\0';

          MIRROR_DBG(("1SSPMACRO_MIRROR_VALUES: pbuf=%s", ssputil_shared_buffer));
          ret->val_data.string = ssputil_shared_buffer;
          break;


      case SSPMACRO_MIRROR_XPBM_EN: /*multi mirror ports*/
          board_mirror_port_get(uport, &enable);

          if ((TRUE == enable) && (!SAL_UPORT_IS_NOT_VALID(uport))){
              sal_strcpy(ssputil_shared_buffer, "checked");
          }else {
              sal_strcpy(ssputil_shared_buffer, "");
          }
          ret->val_data.string = ssputil_shared_buffer;

          break;


      case SSPMACRO_MIRROR_MTP_EN:
          if (board_mirror_to_get(&tmp_uport) != SYS_OK) {
              MIRROR_DBG(("bcm_mirror_ingress_get failed\n"));
          } else {
              if(tmp_uport == uport){
                  enable = TRUE;
              }
          }
          if (TRUE == enable) {
              sal_strcpy(ssputil_shared_buffer, "checked");
          } else {
              sal_strcpy(ssputil_shared_buffer, "");
          }
          ret->val_data.string = ssputil_shared_buffer;
          break;

      case SSPMACRO_MIRROR_MODES:
          SAL_UPORT_ITER(tmp_uport) {
              board_mirror_port_get((uint16)tmp_uport, &enable);
              if (TRUE == enable) {
                  MIRROR_DBG(("SSPMACRO_MIRROR_MTP_EN uport:%d\n", tmp_uport));
                  break;
              }
          }

          if (TRUE == enable) {
              sal_strcpy(ssputil_shared_buffer, "checked");
          } else {
              sal_strcpy(ssputil_shared_buffer, "");
          }
          ret->val_data.string = ssputil_shared_buffer;
          break;
    }
#endif /* CFG_SWITCH_MIRROR_INCLUDED */
}

SSP_HANDLER_RETVAL
ssphandler_mirror_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_MIRROR_INCLUDED
    sys_error_t rv = SYS_OK;
    uint8 mode = TRUE;
    uint16 mirror_to_uport;
    uint32 i = 0;
    uint8 mirror_uplist[MAX_UPLIST_WIDTH];
    uint8 mirror_to_uplist[MAX_UPLIST_WIDTH]; 
    uint8 temp_uplist[MAX_UPLIST_WIDTH];
    
    uint32_t mirror_upbmp;
#ifdef CFG_SWITCH_LAG_INCLUDED
    uint8 enable = FALSE;
    uint8 trunk_mirror_uplist[MAX_UPLIST_WIDTH];
    uint8 trunk_mirror_to_uplist[MAX_UPLIST_WIDTH];
#endif /* CFG_SWITCH_LAG_INCLUDED */
    MIRROR_DBG(("ssphandler_mirror_cgi\n"));
    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {

        MIRROR_DBG(("__ssphandler_mirror_cgi:pairs[0]: %s", cxt->pairs[0].value));
        MIRROR_DBG(("__ssphandler_mirror_cgi:pairs[1]: %s", cxt->pairs[1].value));
        MIRROR_DBG(("__ssphandler_mirror_cgi:paris[2]: %s", cxt->pairs[2].value));

        uplist_clear(mirror_uplist);
        uplist_clear(mirror_to_uplist);
        uplist_clear(temp_uplist);
        mirror_to_uport = SAL_NZUPORT_TO_UPORT(sal_atoi(cxt->pairs[1].value));
        uplist_port_add(mirror_to_uplist, mirror_to_uport);       
        mode = sal_atoi(cxt->pairs[2].value);
        if (mode == TRUE) {
         mirror_upbmp = sal_atoi(cxt->pairs[0].value);
         for (i=0 ; i< MAX_UPLIST_WIDTH; i++) {
              mirror_uplist[i] = (mirror_upbmp >> (i * 8)) & 0xFF;
         }
#ifdef CFG_SWITCH_LAG_INCLUDED
            uplist_clear(trunk_mirror_to_uplist);
            uplist_clear(trunk_mirror_uplist);

            /*check if mirror-to port belongs to a port trunk*/
            for (i=0; i < BOARD_MAX_NUM_OF_LAG ; i++) {
                    rv = board_lag_group_get((uint8)i + 1, &enable, &temp_uplist[0]);                  
                    if (rv == SYS_OK && (enable == TRUE)) {
                        uplist_manipulate(temp_uplist, mirror_to_uplist, UPLIST_OP_AND);                        
                        if(uplist_manipulate(temp_uplist, mirror_to_uplist, UPLIST_OP_EQU) == SYS_OK) {
                            /* add all trunk member ports into mirror-to-port bit map */
                            board_lag_group_get((uint8)i + 1, &enable, &trunk_mirror_to_uplist[0]);                  
                            break; // trunk check finished
                        }
                    }
            }

            /*check if any mirror port belongs to a port trunk */
            for (i=0; i < BOARD_MAX_NUM_OF_LAG ; i++) {
                    rv = board_lag_group_get((uint8)i + 1, &enable, temp_uplist);
                    if (rv == SYS_OK && (enable == TRUE)) {
                        uplist_manipulate(temp_uplist, mirror_uplist, UPLIST_OP_AND);       
                        if(uplist_is_empty(temp_uplist) != SYS_OK) {
                           board_lag_group_get((uint8)i + 1, &enable, temp_uplist);
                           uplist_manipulate(trunk_mirror_uplist, temp_uplist, UPLIST_OP_OR);                           
                        }
                    }
            }


            /* check if trunk mirror port  != trunk mirror-to port */
            uplist_manipulate(temp_uplist, trunk_mirror_uplist, UPLIST_OP_COPY); 
            uplist_manipulate(temp_uplist, trunk_mirror_to_uplist, UPLIST_OP_AND);
            if (uplist_is_empty(temp_uplist) != SYS_OK) {
                    return show_error(cxt, psmem, "Please check Trunk page setting. <BR></BR>The mirror ports and mirror-to port can not belong to the same trunk group.");
            }


            /* check if trunk mirror port  != trunk mirror-to port */
            uplist_manipulate(temp_uplist, mirror_uplist, UPLIST_OP_COPY); 
            uplist_manipulate(temp_uplist, trunk_mirror_to_uplist, UPLIST_OP_AND);
            if (uplist_is_empty(temp_uplist) != SYS_OK) {
                return show_error(cxt, psmem, "Please check Trunk page setting. <BR></BR>The mirror ports and mirror-to port can not belong to the same trunk group.");
            }

            /* check if trunk mirror port  != trunk mirror-to port */
            uplist_manipulate(temp_uplist, trunk_mirror_to_uplist, UPLIST_OP_COPY); 
            uplist_manipulate(temp_uplist, mirror_uplist, UPLIST_OP_AND);
            if (uplist_is_empty(temp_uplist) != SYS_OK) {
                return show_error(cxt, psmem, "Please check Trunk page setting. <BR></BR>The mirror ports and mirror-to port can not belong to the same trunk group.");
            }
#endif /* CFG_SWITCH_LAG_INCLUDED */
            /* check if mirror port  != mirror-to port */
            uplist_manipulate(temp_uplist, mirror_uplist, UPLIST_OP_COPY); 
            uplist_manipulate(temp_uplist, mirror_to_uplist, UPLIST_OP_AND);
            if (uplist_is_empty(temp_uplist) != SYS_OK) {
                return show_error(cxt, psmem, "One of mirror ports is the same with mirror-to port\n");
            }
            SAL_UPORT_ITER(i) {
                if (uplist_port_matched(mirror_uplist, i) == SYS_OK) {
                    rv = board_mirror_port_set(i, TRUE);
                    MIRROR_DBG(("__ssphandler_mirror_cgi:mirror port: %d", i));
                } else {
                    rv = board_mirror_port_set(i, FALSE);
                }
            }

            rv = board_mirror_to_set(mirror_to_uport);

        }else{
            SAL_UPORT_ITER(i) {
                rv = board_mirror_port_set(i, mode);
            }
        }
#if CFG_PERSISTENCE_SUPPORT_ENABLED
        if(SYS_OK == rv){
            persistence_save_current_settings("mirror");
        }
#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */
    }

    if (rv != SYS_OK) {
        return show_error(cxt, psmem, "error occuring as apply setting");
    } else {
        return SSP_HANDLER_RET_INTACT;
    }
#else
    return SSP_HANDLER_RET_INTACT;
#endif /* CFG_SWITCH_MIRROR_INCLUDED */
}

