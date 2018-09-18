/*
 * $Id: portscbk.c,v 1.34 Broadcom SDK $
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
#include "utilcbk.h"
#include "appl/ssp.h"
#include "appl/persistence.h"
#include "utils/ports.h"

#include "../content/sspmacro_ports.h"
#include "../content/sspmacro_pvid.h"
#include "../content/sspmacro_port_desc.h"

#define PORT_DEBUG 0

#if PORT_DEBUG
#define PORT_DBG(x)    do { sal_printf("PORT-CBK: "); sal_printf x; sal_printf("\n"); } while(0);
#else
#define PORT_DBG(x)
#endif

#if  (CFG_WEB_SWITCH_PORT_STATUS || CFG_WEB_SWITCH_PORT_SETUP || defined(CFG_SWITCH_PVLAN_INCLUDED))
#define PVID
#endif  /* CFG_WEB_SWITCH_PORT_STATUS || CFG_WEB_SWITCH_PORT_SETUP || defined(CFG_SWITCH_PVLAN_INCLUDED) */


/* GUI related. */

/* ============== funcitons ================= */
void
sspvar_ports_tag_status(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT
{
    int link, speed, duplex;
    uint16 uport;
    port_mode_t mode;
    BOOL portenable, an;
    BOOL tx_pause, rx_pause;
    
    char portdesc[WEB_PORT_DESC_LEN + 1]; /* index is uport, max is board_uport_count */
#ifdef PVID
    #ifdef CFG_SWITCH_VLAN_INCLUDED
    vlan_type_t type;
    #endif
#endif

    UNREFERENCED_PARAMETER(psmem);
    if (params[2] == SSPMACRO_PORTS_LEFT_HALF) {
        /* params[0] start from 0, nzuport need to be added 1 */
        uport = SAL_ZUPORT_TO_UPORT(params[0]);
    } else if(params[2] == SSPMACRO_PORTS_RIGHT_HALF) {
        /* params[0] start from 0, this is matching with gs108ev2's uport for board API */
        uport = SAL_ZUPORT_TO_UPORT(params[0] + (board_uport_count()/2 + board_uport_count()%2));
    } else {
        PORT_DBG(("sspvar_ports_tag_status : params[2]=%d is wrong value\n", params[2]));
        ret->type = SSPVAR_RET_STRING;
        sal_strcpy(ssputil_shared_buffer, "");
        ret->val_data.string = ssputil_shared_buffer;
        return;
    }

    if (SAL_UPORT_IS_NOT_VALID(uport)) {   /* uport is zero basis number */
        sal_printf("invalid uport = %d\n", uport);
        ret->type = SSPVAR_RET_NULL;
        return;
    }

    /* port enable status get */
    board_port_enable_get((uint16)uport, &portenable);

    board_port_mode_get((uint16)uport, &mode);
    if(mode > PM_LINKDOWN) {
        link = TRUE;
        speed = mode & (~0x1);
        duplex = (mode == PM_10MB_HD || mode == PM_100MB_HD) ? 0 : 1; /* 0:half; 1:full */
    } else {
        link = FALSE;
        speed = 10;
        duplex = 0; /* half */
    }
    board_port_an_get((uint16)uport, &an);
    board_port_pause_get((uint16)uport, &tx_pause, &rx_pause);

    switch (params[1]) {
        case SSPMACRO_PORTS_PORTDESC:
            if (get_port_desc((uint16)uport, portdesc, WEB_PORT_DESC_LEN) != SYS_OK) {
                PORT_DBG(("Get port description failed."));
            }
            sal_strcpy(ssputil_shared_buffer, portdesc);
            break;
        case SSPMACRO_PORTS_PORTEN:
            if (portenable == 1) {
                sal_strcpy(ssputil_shared_buffer, "checked");
            } else {
                sal_strcpy(ssputil_shared_buffer, "");
            }
            break;
        case SSPMACRO_PORTS_LINK:
            if(mode != PM_LINKDOWN) {
                sal_strcpy(ssputil_shared_buffer, "<b>Up</b>");
            } else {
                sal_strcpy(ssputil_shared_buffer, "<b>Down</b>");
            }
            break;
        case SSPMACRO_PORTS_SPEED:
            if(link == TRUE) {
                if (speed < 1000) {
                    if(duplex == 1) {
                       sal_sprintf(ssputil_shared_buffer, "<b>%dMbps&nbspFull</b>", speed);
                    } else {
                       sal_sprintf(ssputil_shared_buffer, "<b>%dMbps&nbspHalf</b>", speed);
                    }
                } else {
                    if (speed == 2500) {
					    sal_sprintf(ssputil_shared_buffer, "<b>2.5Gbps&nbspFull</b>");
                    } else {
   					    sal_sprintf(ssputil_shared_buffer, "<b>%dGbps&nbspFull</b>", speed/1000);
                    }
                }
            } else {
                sal_strcpy(ssputil_shared_buffer, "<b>--</b>");
            }
            break;

        case SSPMACRO_PORTS_FLOWCTRL:
            if(link == TRUE) {
                if(tx_pause && rx_pause) {
                    sal_sprintf(ssputil_shared_buffer, "<b>Enable</b>"); /*bcm95333 default flctr value is enable*/
                } else {
                    sal_sprintf(ssputil_shared_buffer, "<b>Disable</b>"); /*bcm95333 default flctr value is disable*/
                }
            }
            else
                sal_strcpy(ssputil_shared_buffer, "<b>--</b>");
            break;
        case SSPMACRO_PORTS_AUTONEGO:
            if(link == TRUE) {
                if(an) {
                    sal_sprintf(ssputil_shared_buffer, "<b>On</b>"); 
                } else {
                    sal_sprintf(ssputil_shared_buffer, "<b>Off</b>"); 
                }
            }
            else
                sal_strcpy(ssputil_shared_buffer, "<b>--</b>");
            break;

#ifdef PVID
        #ifdef CFG_SWITCH_VLAN_INCLUDED
        /*To disable PVID field when vlan is port base vlan*/
        case SSPMACRO_PORTS_PVID    :
            board_vlan_type_get(&type);

            if(type == VT_PORT_BASED){
                sal_sprintf(ssputil_shared_buffer, "disabled");
            }else{
                sal_sprintf(ssputil_shared_buffer, " ");
            }
            break;
        #endif
#endif
        default:
            PORT_DBG(("**** DEFAULT in sspvar_port_tag_info : params[1]=%d", params[0]));
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = "";
            break;
    }


    ret->type = SSPVAR_RET_STRING;
    ret->val_data.string = ssputil_shared_buffer;

    return;
}

SSPLOOP_RETVAL
ssploop_ports_tag_ports(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem) REENTRANT
{
    if (params[0] == SSPMACRO_PORTS_ALLPORTS) {
        if (index < board_uport_count()) {
            return SSPLOOP_PROCEED;
        }
    } else if (params[0] == SSPMACRO_PORTS_COLUMNPORTS) {
#ifdef SHOW_PORTS_IN_ONE_COLUMN
        /* show ports in single column */
        if (index < board_uport_count()) {
            return SSPLOOP_PROCEED;
        }
#else
        if (index < (board_uport_count()/2 + board_uport_count()%2)) {
            return SSPLOOP_PROCEED;
        }
#endif
    }

    return SSPLOOP_STOP;
}

SSPLOOP_RETVAL
ssploop_ports_tag_two_column(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem) REENTRANT
{
    UNREFERENCED_PARAMETER(params);
    UNREFERENCED_PARAMETER(psmem);

    if (index > 0) {
        /* loop only once */
        return SSPLOOP_STOP;
    } else {
        return SSPLOOP_PROCEED;
    }
    return SSPLOOP_STOP;
}

void
sspvar_ports_tag_num(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT
{
    int uport=0;

    switch (params[1]) {
        case SSPMACRO_PORTS_LEFT_HALF:
        case SSPMACRO_PORTS_ALLPORTS:
            /* params[0] start from 0, nzuport need to be added 1 */
            uport = SAL_ZUPORT_TO_UPORT(params[0]);
            break;
        case SSPMACRO_PORTS_RIGHT_HALF:
            /* params[0] start from 0, nzuport need to be added 1 */
            uport = SAL_ZUPORT_TO_UPORT(params[0] + (board_uport_count()/2 + board_uport_count()%2));
            break;
        case SSPMACRO_PORTS_TOTAL_COUNT:
             /* params[0] start from 0, nzuport need to be added 1 */
             uport = board_uport_count();
             ret->type = SSPVAR_RET_STRING;
             sal_sprintf(ssputil_shared_buffer, "%02d", uport);
             ret->val_data.string = ssputil_shared_buffer;
            return;

        default:
            break;
    }

    ret->type = SSPVAR_RET_STRING;
    if (SAL_UPORT_IS_NOT_VALID(uport)) {
        /* port userp_num + 1 will not be shown */
        ret->val_data.string = "&nbsp;";
    } else {
        if (params[2] == SSPMACRO_PORTS_NOALIGN) {
            ret->type = SSPVAR_RET_INTEGER;
            ret->val_data.integer = SAL_UPORT_TO_NZUPORT(uport);
        } else {
            sal_sprintf(ssputil_shared_buffer, "%02d", SAL_UPORT_TO_NZUPORT(uport));
            ret->val_data.string = ssputil_shared_buffer;
        }
    }

    return;
}

SSP_HANDLER_RETVAL
ssphandler_setport_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT
{
    uint16 uport =0, port_en = 0;
    char *ptmp = NULL, *pbuf_tmp = NULL;
    char buf[5], descbuf[WEB_PORT_DESC_LEN + 1];
#if defined(CFG_SWITCH_VLAN_INCLUDED) || defined(CFG_SWITCH_LAG_INCLUDED)
    uint16 pvid = 0;
#endif
#ifdef CFG_SWITCH_LAG_INCLUDED    
    uint8 tid, enable[BOARD_MAX_NUM_OF_LAG], laguplist[BOARD_MAX_NUM_OF_LAG][MAX_UPLIST_WIDTH]; 
    uint16 perport_pvid[BOARD_MAX_NUM_OF_PORTS], trunk_vlan_id;
    vlan_type_t vlan_type;
#endif    

    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {
        ptmp = (char *)(cxt->pairs[0].value);
        pbuf_tmp = (char *)&buf[0];
        PORT_DBG(("__ssphandler_setport_cgi port#%s cxt:%s\n", ptmp, cxt->pairs[0].value));

#ifdef CFG_SWITCH_LAG_INCLUDED
        board_vlan_type_get(&vlan_type);

        if (vlan_type == VT_DOT1Q) {
            /* Load LAG bit map */
            for (tid = 0; tid < BOARD_MAX_NUM_OF_LAG; tid++) {
                 enable[tid] = FALSE;
                 board_lag_group_get(tid+1, &enable[tid] , &laguplist[tid][0]);
                 if (enable[tid] == FALSE) {
                     sal_memset(&laguplist[tid][0], 0, MAX_UPLIST_WIDTH);
                 }
            }
            /* load current PVID setting into a array */ 
            sal_memset(perport_pvid, 0, sizeof(perport_pvid));
            SAL_UPORT_ITER(uport) {
                 board_untagged_vlan_get(uport, &perport_pvid[SAL_UPORT_TO_ZUPORT(uport)]);
            }
            /* merge PVID change request into a array */
            while (*ptmp != '\0') {
                   if (*ptmp == ',') {
                     ptmp++;
                     *pbuf_tmp = '\0';
                     uport = SAL_NZUPORT_TO_UPORT(sal_atoi(buf));
                     sal_memset(buf, 0, sizeof(buf));
                     pbuf_tmp = &buf[0];
                    } else if (*ptmp == '#') {
                     ptmp++;
                     *pbuf_tmp = '\0';
                     pvid = sal_atoi(buf);
                     sal_memset(buf, 0, sizeof(buf));
                     pbuf_tmp = &buf[0];
                     perport_pvid[SAL_UPORT_TO_ZUPORT(uport)] = pvid;
                   } else {
                      if (pbuf_tmp < &buf[sizeof(buf)-1]) {
                          *pbuf_tmp = *ptmp;
                           pbuf_tmp++;
                      }
                      ptmp++;
                   }
            } 
       
            for (tid = 0; tid < BOARD_MAX_NUM_OF_LAG; tid++) {
                trunk_vlan_id = 0; /* Initial trunk VID is zero --> trunk VID not valid */
                if (enable[tid] == FALSE) {
                    continue;
                }
                SAL_UPORT_ITER(uport) { 
                     if (uplist_port_matched(&laguplist[tid][0], uport) == SYS_OK) {
                         if (trunk_vlan_id == 0) {
                             trunk_vlan_id = perport_pvid[SAL_UPORT_TO_ZUPORT(uport)];
                         } else {
                             if (trunk_vlan_id != perport_pvid[SAL_UPORT_TO_ZUPORT(uport)]) {
                                 /* all trunk port should belong to the same one port vlan */
                                 webutil_show_error(
                                     cxt, psmem,
                                     "TRUNK & PORT",
                                     "Ports belong to the same trunk group should have the same PVID setting.",
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

        ptmp = (char *)(cxt->pairs[0].value);
        pbuf_tmp = (char *)&buf[0];
#endif  

#ifdef CFG_SWITCH_VLAN_INCLUDED
        while(*ptmp != '\0') {
            if(*ptmp == ',') {
                ptmp++;
                *pbuf_tmp = '\0';
                uport = SAL_NZUPORT_TO_UPORT(sal_atoi(buf));
                sal_memset(buf, 0, sizeof(buf));
                pbuf_tmp = &buf[0];
            } else if(*ptmp == '#') {
                ptmp++;
                *pbuf_tmp = '\0';
                pvid = sal_atoi(buf);
                sal_memset(buf, 0, sizeof(buf));
                pbuf_tmp = &buf[0];
                board_untagged_vlan_set(uport, pvid);
            } else {
               if (pbuf_tmp < &buf[sizeof(buf)-1]) {
                   *pbuf_tmp = *ptmp;
                    pbuf_tmp++;
                }
                ptmp++;
            }
         }
#endif /* CFG_SWITCH_VLAN_INCLUDED */        
#ifdef CFG_SWITCH_VLAN_INCLUDED
#if CFG_PERSISTENCE_SUPPORT_ENABLED
    persistence_save_current_settings("pvid");
#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */
#endif /* CFG_SWITCH_VLAN_INCLUDED */

/* Port Description */
       *ptmp = NULL;
       *pbuf_tmp = NULL;
       ptmp = (char *)(cxt->pairs[1].value);
       pbuf_tmp = (char *)&descbuf[0];
        
       while(*ptmp != '\0') {
           if(*ptmp == ',') {
               ptmp++;
               *pbuf_tmp = '\0';
               uport = SAL_NZUPORT_TO_UPORT(sal_atoi(descbuf));
               sal_memset(descbuf, 0, sizeof(descbuf));
               pbuf_tmp = &descbuf[0];
           } else if(*ptmp == '#') {
               ptmp++;
               *pbuf_tmp = '\0';
               set_port_desc(uport, descbuf);
               sal_memset(descbuf, 0, sizeof(descbuf));
               pbuf_tmp = &descbuf[0];
           } else {
               if (pbuf_tmp < &descbuf[sizeof(descbuf)-1]) {
                   *pbuf_tmp = *ptmp;
                    pbuf_tmp++;
               }
               ptmp++;
           }
       }
#if CFG_PERSISTENCE_SUPPORT_ENABLED       
       persistence_save_current_settings("portdesc");
#endif

/* End of Port Description */

/* Port enable */
       *ptmp = NULL;
       *pbuf_tmp = NULL;
       ptmp = (char *)(cxt->pairs[2].value);
       pbuf_tmp = (char *)&buf[0];

       while(*ptmp != '\0') {
           if(*ptmp == ',') {
               ptmp++;
               *pbuf_tmp = '\0';
               uport = SAL_NZUPORT_TO_UPORT(sal_atoi(buf));
               sal_memset(buf, 0, sizeof(char)*5);
               pbuf_tmp = &buf[0];
           } else if(*ptmp == '#') {
               ptmp++;
               *pbuf_tmp = '\0';
               port_en = sal_atoi(buf);
               sal_memset(buf, 0, sizeof(char)*5);
               pbuf_tmp = &buf[0];
               board_port_enable_set(uport, port_en);
           } else {
               *pbuf_tmp = *ptmp;
               pbuf_tmp++;
               ptmp++;
           }
        }
#if CFG_PERSISTENCE_SUPPORT_ENABLED       
        persistence_save_current_settings("portenable");
#endif
/* End of Port enable */
	}

    return SSP_HANDLER_RET_INTACT;
}
#if 0
STATICFN int
get_state_by_uport(int uport, int bAuthorized)
{
    UNREFERENCED_PARAMETER(uport);
    UNREFERENCED_PARAMETER(bAuthorized);

    return 0;
    /* TBD on this function.
     *
     *  Fucntion target :
     *      - Return the given port's basic information in a single return value.
     *
     *      Return formate :
     *      1. bit0:        Link(if enabled)
     *      2. bit1:        Duplex(if enabled && linked)
     *      3. bit3-bit2:   Speed(if enabled && linked) >> b00:10,b01:100,b11:1000
     *      4. bit4:        Disable
     *      5. bit5:        Loop
     *      6. bit6:        -- (can be Medium usage ??)
     *      7. bit7:        GE (P.S redundant definition ??)

     */
}

STATICFN char *
get_port_states(int bAuthorized)
{
    char *pbuf;
    int k;
    int uport;

    pbuf = ssputil_shared_buffer;
    for (uport=1; uport<=board_uport_count(); uport++) {
        k = get_state_by_uport(uport, bAuthorized);
        sal_sprintf(pbuf, "0x%x", (int)k);
        pbuf += sal_strlen(pbuf);
        if (uport < board_uport_count()) {
            *(pbuf++) = ',';
        }
    }
    *pbuf = 0;
    return ssputil_shared_buffer;
}

void
sspvar_ports_tag_states(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT
{
    int bauth = 1;

    UNREFERENCED_PARAMETER(params);

    /* Someone will alloc this psmem if NOT authorized */
    if (ssputil_psmem_get(psmem, sspvar_ports_tag_states) != NULL) {
        bauth = 0;
    }
    ret->type = SSPVAR_RET_STRING;
    ret->val_data.string = get_port_states(bauth);
}
#endif

#ifdef CFG_SWITCH_VLAN_INCLUDED
SSPLOOP_RETVAL
ssploop_pvid_tag_list(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem) REENTRANT
{
    uint16 vid = 1;
    int uport;
    uint8 uplist[3], tag_uplist[3];
    int32 i = 0, count = 0; /*0 of  index is set as pvid value*/

    UNREFERENCED_PARAMETER(params);

    if (params[1] == SSPMACRO_PVID_LEFT_HALF) {
        /* params[0] start from 0, uport need to be added 1 */
        uport = SAL_ZUPORT_TO_UPORT(params[0]);
    } else if(params[1] == SSPMACRO_PVID_RIGHT_HALF) {
        /* params[0] start from 0, this is matching with gs108ev2's uport for board API */
        uport = SAL_ZUPORT_TO_UPORT(params[0] + (board_uport_count()/2 + board_uport_count()%2));
    } else {
        return SSPLOOP_STOP;
    }

    if (SAL_UPORT_IS_NOT_VALID(uport)){   
        return SSPLOOP_STOP;
    }

    for(i = 0; i < board_vlan_count(); i++){
        board_qvlan_get_by_index(i, &vid, &uplist[0], &tag_uplist[0]);
        if(uplist_port_matched(uplist, uport) != SYS_OK){
            continue;
        }
        count++;
    }

    if (index < count){
        return SSPLOOP_PROCEED;
    }
    return SSPLOOP_STOP;
}

void
sspvar_pvid_tag_vidshow(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT
{
    uint16 vid = 1;
    int uport;
    uint8 uplist[3], taglp_list[3];
    int i = 0, count = 0;

    if (params[2] == SSPMACRO_PVID_LEFT_HALF) {
        /* params[1] start from 0, uport need to be added 1 */
        uport = SAL_ZUPORT_TO_UPORT(params[1]);
    } else if(params[2] == SSPMACRO_PVID_RIGHT_HALF) {
        /* params[1] start from 0, this is matching with gs108ev2's uport for board API */
        uport = SAL_ZUPORT_TO_UPORT(params[1] + (board_uport_count()/2 + board_uport_count()%2));
    } else {
        return;
    }

    ret->type = SSPVAR_RET_INTEGER;
    
    while(i < board_vlan_count()){
        board_qvlan_get_by_index(i, &vid, &uplist[0], &taglp_list[0]);
        if(uplist_port_matched(uplist, uport) == SYS_OK){
            count ++;
            if (count == (params[0]+1)) {
                ret->val_data.integer = vid;
                return;
            }
        }
        i++;
    }
}

void
sspvar_pvid_tag_selected(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT
{
    uint16 pvid = 0, vid = 0;
    int uport;
    uint8 uplist[3], taglp_list[3];
    int i = 0, count = 0;

    if (params[2] == SSPMACRO_PVID_LEFT_HALF) {
        /* params[1] start from 0, uport need to be added 1 */
        uport = SAL_ZUPORT_TO_UPORT(params[1]);
    } else if(params[2] == SSPMACRO_PVID_RIGHT_HALF) {
        /* params[1] start from 0, this is matching with gs108ev2's uport for board API */
        uport = SAL_ZUPORT_TO_UPORT(params[1] + (board_uport_count()/2 + board_uport_count()%2));
    } else {
        return;
    }

    ret->type = SSPVAR_RET_STRING;
    ret->val_data.string = ssputil_shared_buffer;
        
    board_untagged_vlan_get(uport, &pvid);

    while(i < board_vlan_count()){
        board_qvlan_get_by_index(i, &vid, &uplist[0], &taglp_list[0]);
        if(uplist_port_matched(uplist, uport) == SYS_OK){
            count ++;
            if (count == (params[0]+1)) {
               break;
            }
        }
        i++;
    }
    
    if (vid == pvid) {
        sal_strcpy(ssputil_shared_buffer, "selected");
    } else {
        sal_strcpy(ssputil_shared_buffer, "");
    }
}
#endif

void
sspvar_ports_tag_rownum(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
       ret->type = SSPVAR_RET_STRING;
       ssputil_shared_buffer[0] = 0;
#ifdef SHOW_PORTS_IN_ONE_COLUMN
    /* show ports in single column */
#else
    switch (params[0]) {
        case SSPMACRO_PORTS_ROWSPAN:
            if (board_uport_count() > MAX_PORTS_PER_ROW) {
                   sprintf(ssputil_shared_buffer, "rowspan=2");
            }
            break;

        case SSPMACRO_PORTS_NEWROW:
            if (board_uport_count() > MAX_PORTS_PER_ROW &&
                params[1] == (board_uport_count()/2 - 1)) {
                   sprintf(ssputil_shared_buffer, "<TR>");
            }
            break;
    }
#endif
    ret->val_data.string = ssputil_shared_buffer;
}

SSPLOOP_RETVAL
ssploop_ports_tag_rows(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
    switch (params[0]) {
        case SSPMACRO_PORTS_ONEROW:
            if (index > 0 || board_uport_count() > MAX_PORTS_PER_ROW) {
                   /* loop only once */
                return SSPLOOP_STOP;
            } else {
                return SSPLOOP_PROCEED;
            }
            break;
        case SSPMACRO_PORTS_TWOROWS:
#ifdef SHOW_PORTS_IN_ONE_COLUMN
            /* show ports in single column */
            return SSPLOOP_STOP;
#else
            if (index > 0 || board_uport_count() <= MAX_PORTS_PER_ROW) {
                   /* loop only once */
                return SSPLOOP_STOP;
            } else {
                return SSPLOOP_PROCEED;
            }
            break;
#endif
        }
    return SSPLOOP_STOP;
}
