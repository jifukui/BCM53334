/*
 * $Id: vlancbk.c,v 1.36 Broadcom SDK $
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
#include "appl/igmpsnoop.h"

#include "../content/sspmacro_vlan.h"
#include "../content/sspmacro_ports.h"
#include "../content/vlannew_htm.h"
#include "../content/vlanlist_htm.h"
#include "../content/pvlanview_htm.h"
#include "../content/errormsg_htm.h"


#define VLAN_DEBUG 0

#ifdef CFG_SWITCH_VLAN_INCLUDED


#if VLAN_DEBUG
#define VLAN_DBG(x)    do { sal_printf("VLAN: "); \
                       sal_printf x; sal_printf("\n");\
                       } while(0);
#else
#define VLAN_DBG(x)
#endif

#define MAX_PVLAN_NUM  8

#define MAX_VLAN_PER_PAGE  5
#define MAX_PORT_NUM board_uport_count()
#define NUM_PORTS_PER_SYSTEM MAX_PORT_NUM
#define MAX_VLAN_NUM   BOARD_MAX_NUM_OF_QVLANS

#define NON_MEMBER_ICON     '0'
#define UNTAG_ICON                 '1'
#define TAG_ICON                      '2'

#define INT_NON_MEMBER_VLAN     0
#define INT_UNTAG_VLAN                 1
#define INT_TAG_VLAN                      2



typedef enum vlanmgr_vlan_type_s {
    VLANMGR_TYPE_NONE = -1,
    VLANMGR_8021Q_VLAN = 0,
    VLANMGR_PRIVATE_VLAN =1
}vlanmgr_vlan_type_t;

/* Forward declaration */

SSPLOOP_RETVAL
ssploop_vlan_tag_vid_counts(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem);
SSP_HANDLER_RETVAL
ssphandler_showvlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem);
SSP_HANDLER_RETVAL
ssphandler_newvlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem);
SSP_HANDLER_RETVAL
ssphandler_setvlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem);
SSP_HANDLER_RETVAL
ssphandler_pagevlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem);
SSP_HANDLER_RETVAL
ssphandler_switchpage_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem);
void
sspvar_vlan_tag_ssi(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem);

const char STRING_DISABLE[] = "Disabled";
const char STRING_ENABLE[] = "Enabled";
const char STRING_NULL[] = "";
const char STRING_CHECKED[] = "checked";
const char STRING_SELECTED[] = "selected";

const char STRING_NA[] = "--";

const char STRING_COMMENT_START[] = "<!--";
const char STRING_COMMENT_END[] = "-->";
const char STRING_NBSP[] = "&nbsp;";

static uint8 *vlan_info;


static SSP_HANDLER_RETVAL
show_error(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem, int error) {
    VLAN_DBG(("show_error\n"));

    webutil_show_error(
        cxt, psmem,
        "VLAN",
        "system error information!!",
        err_button_retry,
        err_action_back
        );

    /* We don't want to process it more */
    /* cxt->flags = 0; */
    return SSP_HANDLER_RET_MODIFIED;
}

#ifdef CFG_SWITCH_LAG_INCLUDED
static SSP_HANDLER_TYPE vlan_n_trunk_check(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem, uint8 *uplist) {


        uint16 k;
        uint8 tmp_uplist[MAX_UPLIST_WIDTH], lag_uplist[MAX_UPLIST_WIDTH];
        uint8 enable;
        uint8 lag_group_num;

        board_lag_group_max_num(&lag_group_num);

        for (k=1; k <= lag_group_num; k++) {
             board_lag_group_get(k,&enable,lag_uplist);
             if (enable == FALSE)  {
                continue;
             }
             uplist_manipulate(tmp_uplist, lag_uplist, UPLIST_OP_COPY);
             uplist_manipulate(tmp_uplist, uplist, UPLIST_OP_AND);
             if ((uplist_manipulate(tmp_uplist, lag_uplist, UPLIST_OP_EQU) != SYS_OK) &&
                 (uplist_is_empty(tmp_uplist) != SYS_OK)) {

                    webutil_show_error(
                    cxt, psmem,
                    "VLAN",
                    "VLAN and Trunk page setting conflict. Ports of a trunk group should not be partially assigned to any VLAN.",
                    err_button_retry,
                    err_action_back);
                    /* We don't want to process it more */
                    /* cxt->flags = 0; */
                    return SSP_HANDLER_RET_MODIFIED;

             }
        }
        return SSP_HANDLER_RET_INTACT;
}
#endif

SSPLOOP_RETVAL
ssploop_vlan_tag_pagevid_counts(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{

    int *p, page;

    p = (int *)ssputil_psmem_get(psmem, ssphandler_pagevlan_cgi);
    if (p == NULL) {
        page = 0;
    } else {
        page = *p;
    }
    if (index < MAX_VLAN_PER_PAGE &&
        (page*MAX_VLAN_PER_PAGE+index) < board_vlan_count())
        return SSPLOOP_PROCEED;

    return SSPLOOP_STOP;

}

SSPLOOP_RETVAL
ssploop_vlan_tag_vid_counts(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
    if (index < board_vlan_count()){

    return SSPLOOP_PROCEED;
    }

    return SSPLOOP_STOP;
}

SSP_HANDLER_RETVAL
ssphandler_delvlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    int vid;
    uint16  utag_vid;
#ifdef CFG_SWITCH_MCAST_INCLUDED
    uint16 val16;
    uint8 val8;
#endif
    sys_error_t rv;
    char err_msg[128];

    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {
        vid = sal_atoi(cxt->pairs[0].value);
    {
        uint16 i;


         SAL_UPORT_ITER(i){
            board_untagged_vlan_get(i, &utag_vid);
            if(utag_vid == vid){
                VLAN_DBG(("Have to change PVID of port %d first!", i));
                sal_sprintf(err_msg, "Change PVID of ports to other VLAN before removing this VLAN.");

                webutil_show_error(
                            cxt, psmem,
                            "Remove Vlan",
                            err_msg,
                            err_button_retry,
                            err_action_back);

                return SSP_HANDLER_RET_MODIFIED;
            }
        }
    }

#ifdef CFG_SWITCH_MCAST_INCLUDED
           igmpsnoop_enable_get(&val8);
           if (val8) {
               igmpsnoop_vid_get(&val16);
               if(val16 == vid) {
                    sal_sprintf(err_msg, "Please change IGMP snoop VLAN setting before removing this VLAN");

                    webutil_show_error(
                            cxt, psmem,
                            "Remove Vlan",
                            err_msg,
                            err_button_retry,
                            err_action_back);
                    return SSP_HANDLER_RET_MODIFIED;
               }
           }

#endif
        rv = board_vlan_destroy((uint16)vid);
        if(SYS_ERR == rv){
            VLAN_DBG(("board_vlan_destroy vid = %d failed", vid));
            return show_error(cxt, psmem, rv);
        }
#if CFG_PERSISTENCE_SUPPORT_ENABLED
        persistence_save_current_settings("vlan");
#endif
    }
    return SSP_HANDLER_RET_INTACT;
}

SSP_HANDLER_RETVAL
ssphandler_pagevlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    int curpage, mode;
    int *p;

    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {

        p = (int *)ssputil_psmem_alloc(psmem,
                                       ssphandler_pagevlan_cgi,
                                       sizeof(int));
        curpage = sal_atoi(cxt->pairs[0].value);
        mode = sal_atoi(cxt->pairs[1].value);

        VLAN_DBG(("ssphandler_pagevlan_cgi : curpage = %d, mode = %d\n",
                  curpage, mode));

        if (mode == 1) /* next page */
            *p = curpage+1;
        else
            *p = curpage-1;

        VLAN_DBG(("ssphandler_pagevlan_cgi : new page = %d\n", *p));
    }
    return SSP_HANDLER_RET_INTACT;
}

SSP_HANDLER_RETVAL
ssphandler_switchpage_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    vlanmgr_vlan_type_t vlan_type;

    VLAN_DBG(("ssphandler_switchpage_cgi : vlan_type = %d\n", sal_atoi(cxt->pairs[0].value)));
    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {
        vlan_type = sal_atoi(cxt->pairs[0].value);
        if (vlan_type == VLANMGR_8021Q_VLAN) {
            /* Show VLAN page */
            board_vlan_type_set(VT_DOT1Q);

            cxt->page = sspfile_vlanlist_htm;
#if defined(CFG_SWITCH_PVLAN_INCLUDED)
        } else {
            /* Show PVLAN page */
            board_vlan_type_set(VT_PORT_BASED);

            cxt->page = sspfile_pvlanview_htm;
#endif /* defined(CFG_SWITCH_PVLAN_INCLUDED) */
        }
        cxt->flags= 0;
    }
#if CFG_PERSISTENCE_SUPPORT_ENABLED
    persistence_save_current_settings("vlan");
#endif
    return SSP_HANDLER_RET_MODIFIED;
}

SSP_HANDLER_RETVAL
ssphandler_showvlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    int *p;

    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {
        p = (int *)ssputil_psmem_alloc(psmem,
                                       ssphandler_showvlan_cgi,
                                       sizeof(int));
        *p = sal_atoi(cxt->pairs[0].value);
        VLAN_DBG(("ssphandler_showvlan_cgi : vid = %d\n", *p));

    }
    return SSP_HANDLER_RET_INTACT;
}

SSP_HANDLER_RETVAL
ssphandler_newvlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    sys_error_t rv = SYS_OK;
    char *ptmp, vlan_tmp;
    int *p, i = 0, vid_idx = 0;

    uint16 vid, vid_qry = 0;


    uint8 tag_uplist[MAX_UPLIST_WIDTH], untag_uplist[MAX_UPLIST_WIDTH];
    uint8 uplist[MAX_UPLIST_WIDTH];

    char err_msg[256];


    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {

        vid = sal_atoi(cxt->pairs[0].value);

        if(board_vlan_count() > MAX_VLAN_NUM){
            sal_sprintf(err_msg,"Maximum number of IEEE 802.1Q VLAN is %d",MAX_VLAN_NUM);
            if(vid_qry == vid){
                webutil_show_error(
                    cxt, psmem,
                    "Create New Vlan",
                    err_msg,
                    err_button_retry,
                    err_action_back);
                return SSP_HANDLER_RET_MODIFIED;
            }
        }

        if (vid <= 0 || vid > MAX_VID_NUM) {
            /*return show_error(cxt, psmem, BCM_E_BADID);*/
            return show_error(cxt, psmem, SYS_ERR);
        }
        /*to check if vid existed*/
        for(vid_idx = 0; vid_idx < board_vlan_count(); vid_idx++){
            board_qvlan_get_by_index(vid_idx , &vid_qry, &uplist[0], &tag_uplist[0]);
            if(vid_qry == vid){
                webutil_show_error(
                    cxt, psmem,
                    "Create New Vlan",
                    "Vlan id existed already",
                    err_button_retry,
                    err_action_back);
                return SSP_HANDLER_RET_MODIFIED;
            }
        }

        p = (int *)ssputil_psmem_alloc(psmem,
                                       ssphandler_showvlan_cgi,
                                       sizeof(int));
        *p = vid;

        VLAN_DBG(("cgi_newvlan : new vid = %d, string = %s\n", vid,
                 cxt->pairs[1].value));

        /* get the tag and untag port bit maps:                                   *
          * Tag Vlan: ports_bitsmap_vlantag of the port(i):Ture;          *
          * UTag Vlan: ports_bitsmap_vlantag of the port(i):Faluse     */
       ptmp = (char *)(cxt->pairs[1].value);
       VLAN_DBG(("SHOW VLAN VALUE of port: cxt_addr:%x ptmp_addr:%x\n", cxt->pairs[1].value, ptmp));

       sal_memset(uplist, 0 , sizeof(uint8) * MAX_UPLIST_WIDTH);
       sal_memset(tag_uplist, 0 , sizeof(uint8) * MAX_UPLIST_WIDTH);
       sal_memset(untag_uplist, 0 , sizeof(uint8) * MAX_UPLIST_WIDTH);



       SAL_UPORT_ITER(i) {
           vlan_tmp  =  *(ptmp++);
           VLAN_DBG(("port#%d : 0x %x char :%c", i+1 ,  (uint32)vlan_tmp-48, vlan_tmp));

           if(NON_MEMBER_ICON == vlan_tmp){

           }else if(TAG_ICON == vlan_tmp){
               VLAN_DBG(("index#%d value=0x %x\n", (i / 8),
                    (uint8)((uint8) 1 << (i % 8))));
               uplist_port_add(tag_uplist, i);
               uplist_port_add(uplist, i);
               uplist_port_remove(untag_uplist, i);
           }else if(UNTAG_ICON == vlan_tmp){
               VLAN_DBG(("index#%d value=0x %x bit_idx:%d\n", (i /8),
                    (uint8)((uint8) 1 << (i % 8)), i % 8));

               uplist_port_remove(tag_uplist, i);
               uplist_port_add(untag_uplist, i);
               uplist_port_add(uplist, i);

           }
       }


       VLAN_DBG(("\n end__SHOW VLAN VALUE: vid#%d\n",vid));
       VLAN_DBG(("matrix#%d :t_port:0x %2x\n", 0, (uint8)tag_uplist[0]));
       VLAN_DBG(("matrix#%d :t_port:0x %2x\n", 1, (uint8)tag_uplist[1]));
       VLAN_DBG(("matrix#%d :t_port:0x %2x\n", 2, (uint8)tag_uplist[2]));

       VLAN_DBG(("matrix#%d :ut_port:0x %2x\n", 0, (uint8)untag_uplist[0]));
       VLAN_DBG(("matrix#%d :ut_port:0x %2x\n", 1, (uint8)untag_uplist[1]));
       VLAN_DBG(("matrix#%d :ut_port:0x %2x\n", 2, (uint8)untag_uplist[2]));


       VLAN_DBG(("matrix#%d :uport:0x %2x\n", 0, (uint8)uplist[0]));
       VLAN_DBG(("matrix#%d :uport:0x %2x\n", 1, (uint8)uplist[1]));
       VLAN_DBG(("matrix#%d :uport:0x %2x\n", 2, (uint8)uplist[2]));
#ifdef CFG_SWITCH_LAG_INCLUDED
       if (vlan_n_trunk_check(cxt, psmem, uplist))
           return SSP_HANDLER_RET_MODIFIED;
#endif /* CFG_SWITCH_LAG_INCLUDED */
       /*To set vlan per port*/
       board_vlan_type_set(VT_DOT1Q);
       board_vlan_create(vid);
       board_qvlan_port_set(vid, uplist, tag_uplist);
#if CFG_PERSISTENCE_SUPPORT_ENABLED
       persistence_save_current_settings("vlan");
#endif

    }
        if (rv == SYS_ERR)
            return show_error(cxt, psmem, rv);
        else
            return SSP_HANDLER_RET_INTACT;
}

SSP_HANDLER_RETVAL
ssphandler_setvlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    char *ptmp, vlan_tmp;
    int *p,  rv = SYS_OK;
    int i = 0;
    uint16 vid = 0, utag_vid =0;
    uint8 tag_uplist[MAX_UPLIST_WIDTH], untag_uplist[MAX_UPLIST_WIDTH];
    uint8 uplist[MAX_UPLIST_WIDTH];

    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {

        vid = sal_atoi(cxt->pairs[0].value);



        if (vid <= 0 || vid > MAX_VID_NUM) {
            return show_error(cxt, psmem, SYS_ERR);
        }

        p = (int *)ssputil_psmem_alloc(psmem,
                                       ssphandler_showvlan_cgi,
                                       sizeof(int));
        *p = vid;
        VLAN_DBG(("cgi_setvlan : set vid = %d, string = %s\n", vid,
                              cxt->pairs[1].value));

        /* get the tag and untag port bit maps:                                 *
          * Tag Vlan: ports_bitsmap_vlantag of the port(i):Ture;        *
          * UTag Vlan: ports_bitsmap_vlantag of the port(i):Faluse    */
       ptmp = (char *)(cxt->pairs[1].value);


       sal_memset(uplist, 0 , sizeof(uint8) * MAX_UPLIST_WIDTH);
       sal_memset(tag_uplist, 0 , sizeof(uint8) * MAX_UPLIST_WIDTH);
       sal_memset(untag_uplist, 0 , sizeof(uint8) * MAX_UPLIST_WIDTH);

       SAL_UPORT_ITER(i) {
           vlan_tmp  =  *(ptmp++);
           VLAN_DBG(("port#%d : 0x %x char :%c", i+1 ,  (uint32)vlan_tmp-48, vlan_tmp));

           if(NON_MEMBER_ICON == vlan_tmp){

               char err_msg[128];

               board_untagged_vlan_get(i, &utag_vid);
               if(utag_vid == vid){
                   VLAN_DBG(("Have to change PVID of port %d first!", i));
                   sal_sprintf(err_msg, "Change PVID of port %d to other VLAN before removing this port member.", i);

                   webutil_show_error(
                            cxt, psmem,
                            "Remove Port member of VLAN",
                            err_msg,
                            err_button_retry,
                            err_action_back);

                    return SSP_HANDLER_RET_MODIFIED;
               }

           }else if(TAG_ICON == vlan_tmp){
                VLAN_DBG(("index#%d value=0x %x\n", (i / 8),
                    (uint8)((uint8) 1 << (i % 8))));
                uplist_port_add(tag_uplist, i);
                uplist_port_add(uplist, i);
                uplist_port_remove(untag_uplist, i);
           }else if(UNTAG_ICON == vlan_tmp){
               VLAN_DBG(("index#%d value=0x %x bit_idx:%d\n", (i /8),
                    (uint8)((uint8) 1 << (i % 8)), i % 8));
               uplist_port_remove(tag_uplist, i);
               uplist_port_add(untag_uplist, i);
               uplist_port_add(uplist, i);
           }
       }
    VLAN_DBG(("\n end__SHOW VLAN VALUE: vid#%d\n",vid));
       VLAN_DBG(("matrix#%d :t_port:0x %2x\n", 0, (uint8)tag_uplist[0]));
       VLAN_DBG(("matrix#%d :t_port:0x %2x\n", 1, (uint8)tag_uplist[1]));
       VLAN_DBG(("matrix#%d :t_port:0x %2x\n", 2, (uint8)tag_uplist[2]));

       VLAN_DBG(("matrix#%d :ut_port:0x %2x\n", 0, (uint8)untag_uplist[0]));
       VLAN_DBG(("matrix#%d :ut_port:0x %2x\n", 1, (uint8)untag_uplist[1]));
       VLAN_DBG(("matrix#%d :ut_port:0x %2x\n", 2, (uint8)untag_uplist[2]));


       VLAN_DBG(("matrix#%d :uport:0x %2x\n", 0, (uint8)uplist[0]));
       VLAN_DBG(("matrix#%d :uport:0x %2x\n", 1, (uint8)uplist[1]));
       VLAN_DBG(("matrix#%d :uport:0x %2x\n", 2, (uint8)uplist[2]));
#ifdef CFG_SWITCH_LAG_INCLUDED
       if (vlan_n_trunk_check(cxt, psmem, uplist))
           return SSP_HANDLER_RET_MODIFIED;
#endif /* CFG_SWITCH_LAG_INCLUDED */
       /*To set vlan per port*/
       board_vlan_type_set(VT_DOT1Q);
       board_vlan_create(vid);
       rv = board_qvlan_port_set(vid, uplist, tag_uplist);

#if CFG_PERSISTENCE_SUPPORT_ENABLED
        if (rv == SYS_OK) {
            persistence_save_current_settings("vlan");
        }
#endif
    }
        if (rv != SYS_OK)
            return show_error(cxt, psmem, rv);
        else
            return SSP_HANDLER_RET_INTACT;
}

void sspvar_vlan_tag_list(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
    uint16 cur_vid;
    int *p, page, i = 0;
    uint8 uplist[MAX_UPLIST_WIDTH], tag_uplist[MAX_UPLIST_WIDTH];

    p = (int *)ssputil_psmem_get(psmem, ssphandler_pagevlan_cgi);
    if (p == NULL) {
        page = 0;
    } else {
        page = *p;
        VLAN_DBG(("current page = %d\n", *p));
    }

    if (params[0] == SSPMACRO_VLAN_CURPAGE) {
        ret->type = SSPVAR_RET_INTEGER;
        ret->val_data.integer = page;
        return;
    }
    board_qvlan_get_by_index(page*MAX_VLAN_PER_PAGE+params[1], &cur_vid, &uplist[0], &tag_uplist[0]);
    ret->type = SSPVAR_RET_STRING;
    ret->val_data.string = ssputil_shared_buffer;
    sal_memset(ssputil_shared_buffer, 0, sizeof(ssputil_shared_buffer));

    switch (params[0]) {

      case SSPMACRO_VLAN_NPAGE_ENABLE:
          if ((page+1)*MAX_VLAN_PER_PAGE >= board_vlan_count()) {
              sal_sprintf(ssputil_shared_buffer, STRING_DISABLE);
          }
          break;

      case SSPMACRO_VLAN_PPAGE_ENABLE:
          if (page == 0) {
              sal_sprintf(ssputil_shared_buffer, STRING_DISABLE);
          }
          break;

      case SSPMACRO_VLAN_PAGEVID:
          ret->type = SSPVAR_RET_INTEGER;
          ret->val_data.integer = cur_vid;
          break;

      case SSPMACRO_VLAN_ALLOW_CREATE:
          ret->type = SSPVAR_RET_INTEGER;
          if (board_vlan_count() == MAX_VLAN_NUM) {
              ret->val_data.integer = 0;
          } else {
              ret->val_data.integer = 1;
          }
          break;

      case SSPMACRO_VLAN_MAX_NUM:
          ret->type = SSPVAR_RET_INTEGER;
          ret->val_data.integer = MAX_VLAN_NUM;
          break;

      case SSPMACRO_VLAN_GET_BODR:
          ret->type = SSPVAR_RET_INTEGER;

          if(uplist_is_empty(uplist) == SYS_OK){
              ret->val_data.integer = 0;
              VLAN_DBG(("query once for vid = %d, boader:%d\n", cur_vid, ret->val_data.integer));
          }else{
              ret->val_data.integer = 1;
              VLAN_DBG(("query once for vid = %d, boader:%d\n", cur_vid, ret->val_data.integer));
          }

          break;

      case SSPMACRO_VLAN_GET_MEMBER:
          if (cur_vid > 0) {
              /* query hardware table once for each vlan */
                  if (params[2] == 0) {
                     VLAN_DBG(("query once for vid = %d\n", cur_vid));
                  if (params[1] == 0) {
                      vlan_info = ssputil_psmem_alloc(psmem,
                              sspvar_vlan_tag_list,
                              MAX_PORT_NUM+1);
                  } else {
                      vlan_info = ssputil_psmem_get(psmem, sspvar_vlan_tag_list);
                      if (vlan_info == NULL) {
                          VLAN_DBG(("vlan_info is NULL!!!"));
                      }
                  }

                  sal_memset(vlan_info, 0, MAX_PORT_NUM+1);
                  sal_memset(&uplist[0], 0, sizeof(uint8) * MAX_UPLIST_WIDTH);
                  sal_memset(&tag_uplist[0], 0, sizeof(uint8) * MAX_UPLIST_WIDTH);

                  board_qvlan_port_get(cur_vid, &uplist[0], &tag_uplist[0]);
                  SAL_UPORT_ITER(i) {
                      if(uplist_port_matched(tag_uplist, i) == SYS_OK) {
                            vlan_info[SAL_UPORT_TO_ZUPORT(i)] = 2;   /* tagged */
                      }
                  }

                  SAL_UPORT_ITER(i) {
                      if ((uplist_port_matched(tag_uplist, i) != SYS_OK) &&
                          (uplist_port_matched(uplist, i) == SYS_OK)) {
                            vlan_info[SAL_UPORT_TO_ZUPORT(i)] = 1;   /* untagged */
                      }
                  }

              } else {
                  vlan_info = ssputil_psmem_get(psmem, sspvar_vlan_tag_list);
                  if (vlan_info == NULL) {
                      VLAN_DBG(("vlan_info is NULL!!!"));
                  }
              }

              if (vlan_info[params[2]] == 1) {
                   sal_sprintf(ssputil_shared_buffer, "<TD BGCOLOR=\"#33FFCC\">%02d</TD>", SAL_ZUPORT_TO_UPORT(params[2]));
              } else if (vlan_info[params[2]] == 2) {
                   sal_sprintf(ssputil_shared_buffer, "<TD BGCOLOR=\"#FFCC33\">%02d</TD>", SAL_ZUPORT_TO_UPORT(params[2]));
              } else {
                   sal_strcpy(ssputil_shared_buffer, STRING_NULL);
              }
              if (vlan_info[params[2]] != 0) {
                  vlan_info[MAX_PORT_NUM]++;
                  if (MAX_PORT_NUM > MAX_PORTS_PER_ROW && vlan_info[MAX_PORT_NUM] == MAX_PORT_NUM/2) {
                      sal_strcat(ssputil_shared_buffer, "<TR>");
                  }
              }
          }
          break;
    }
}

void sspvar_vlan_tag_ssi(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT
{
    uint8 uplist[MAX_UPLIST_WIDTH];
    uint8 tag_uplist[MAX_UPLIST_WIDTH];
    uint16 vid = 1;
    int cur_vid = 1, i = 0;
    char *pbuf;
    int *p;
    vlan_type_t vlan_type = VT_DOT1Q;
    
    switch(params[0]){

      case SSPMACRO_VLAN_MAXID:
          board_vlan_type_get(&vlan_type);
          ret->type = SSPVAR_RET_INTEGER;
          ret->val_data.integer = (vlan_type == VT_DOT1Q)? MAX_QVLAN_ID : board_uport_count();
          break;

      case SSPMACRO_VLAN_VID:
      case SSPMACRO_VLAN_ORDER:
          ret->type = SSPVAR_RET_INTEGER;
          board_qvlan_get_by_index(params[1], &vid, &uplist[0], &tag_uplist[0]);
          ret->val_data.integer = vid;
          VLAN_DBG(("SSPMACRO_VLAN_ORDER: x_value = %d\n", params[1]));
          break;

      case SSPMACRO_VLAN_CURID:
          p = (int *)ssputil_psmem_get(psmem, ssphandler_showvlan_cgi);
          if (p == NULL)
              cur_vid = 1; /* should NEVER happen */
          else
              cur_vid = *p;

          VLAN_DBG(("sspvar_vlan_tag_ssi : cur_vid = %d\n", cur_vid));
          ret->type = SSPVAR_RET_INTEGER;
          ret->val_data.integer = cur_vid;
          break;

      case SSPMACRO_VLAN_ENABLE:
          p = (int *)ssputil_psmem_get(psmem, ssphandler_showvlan_cgi);
          if (p == NULL)
              cur_vid = 1;
          else
              cur_vid = *p;

          if (cur_vid == 0 || cur_vid == 1)
              sal_strcpy(ssputil_shared_buffer, STRING_DISABLE);
          else
              sal_strcpy(ssputil_shared_buffer, STRING_NULL);

          ret->val_data.string = ssputil_shared_buffer;
          ret->type = SSPVAR_RET_STRING;
          break;


      case SSPMACRO_VLAN_TYPE:
          ret->type = SSPVAR_RET_STRING;
          ret->val_data.string = ssputil_shared_buffer;
          board_vlan_type_get(&vlan_type);
          VLAN_DBG(("wss_vlanmgr_vlantype_get = %d\n", vlan_type));
          if (vlan_type == VT_PORT_BASED) {
              sal_sprintf(ssputil_shared_buffer, "port-based");
          } else {
              sal_sprintf(ssputil_shared_buffer, "8021q");
          }
          break;

        case SSPMACRO_VLAN_STATES:
            pbuf = ssputil_shared_buffer; /* Set to not member at first */
            for(i = 0; i < NUM_PORTS_PER_SYSTEM; i++) {
                *(pbuf++) = NON_MEMBER_ICON;  /* not member */
                if (i < NUM_PORTS_PER_SYSTEM - 1)
                    *(pbuf++) = ',';
            }
            *pbuf = 0;
            p = (int *)ssputil_psmem_get(psmem, ssphandler_showvlan_cgi);
            if (p != NULL) { /* p = NULL should only happen when creating new vlan */
                board_vlan_type_set(VT_DOT1Q);
                cur_vid = *p;
                pbuf = ssputil_shared_buffer;
                VLAN_DBG(("cur_vid = %d\n", cur_vid));
                if (cur_vid > 0) {

/*for vlan status get */
                    uint8 t_ports[MAX_UPLIST_WIDTH];
                    uint8 uport[MAX_UPLIST_WIDTH];

                    uint8 *uplist;
                    uint8 *tag_uplist;

                    uplist = &uport[0];
                    tag_uplist = &t_ports[0];

                    sal_memset(uplist, 0 , sizeof(uint8) * MAX_UPLIST_WIDTH);
                    sal_memset(tag_uplist, 0 , sizeof(uint8) * MAX_UPLIST_WIDTH);

                    board_qvlan_port_get(cur_vid, uplist, tag_uplist);
                    VLAN_DBG(("__board_qvlan_port_get : vid = %d\n", uplist));
                    VLAN_DBG(("__board_qvlan_port_get : uplist = %d\n", tag_uplist));

                    VLAN_DBG(("VID = %d\n", cur_vid));

                    for(i = 0; i < MAX_UPLIST_WIDTH; i++){
                        VLAN_DBG(("__board_qvlan_port_get : uplistt[%d] = 0x%x\n", i, uplist[i]));
                        VLAN_DBG(("__board_qvlan_port_get : tag_uplistt[%d] = 0x%x\n", i, tag_uplist[i]));
                    }
/*__end for vlan status get */

                    SAL_UPORT_ITER(i) {
                        if(uplist_port_matched(tag_uplist, i) == SYS_OK) {
                            pbuf[SAL_UPORT_TO_ZUPORT(i)*2] = TAG_ICON;   /* tagged */
                        }
                   }

                   SAL_UPORT_ITER(i) {
                       if ((uplist_port_matched(tag_uplist, i) != SYS_OK) &&
                            (uplist_port_matched(uplist, i) == SYS_OK)) {
                            pbuf[SAL_UPORT_TO_ZUPORT(i)*2] = UNTAG_ICON;   /* untagged */
                        }
                    }
                }
            }
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            VLAN_DBG(("__SSPMACRO_VLAN_STATES  VLAN_STATES : %s\n", ssputil_shared_buffer));
        break;

    }

    return;
}

void sspvar_ports_tag_portcount(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT
{
    ret->type = SSPVAR_RET_INTEGER;
    ret->val_data.integer = board_uport_count();
    return;
}
#endif



