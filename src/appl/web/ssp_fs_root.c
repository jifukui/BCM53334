/*
 * $Id: ssp_fs_root.c,v 1.37 Broadcom SDK $
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

/* WFS root currently for UM30 is manual configured instead of auto-generated
 *
 *  Manual change required at
 *  1. add Web Content header file.
 *  2. CGI prototype declaration
 *  3. File entry in WFS with Bank_ID included(bank_id is expected no impact on MIPS solution.)
 */

#include "appl/ssp.h"

#include "content/bg_gif.h"
#include "content/form_css.h"

#ifdef __BOOTLOADER__

#include "content/fupgrade_htm.h"
#include "content/frestart_htm.h"

#else  /*  __BOOTLOADER__ */


#include "content/errormsg_htm.h"
#include "content/portconf_htm.h"
#include "content/porthelp1_htm.h"
#include "content/porthelp2_htm.h"
#include "content/setport_htm.h"
#include "content/system_help_htm.h"
#include "content/system_htm.h"
#include "content/system_name_htm.h"
#include "content/reset2_htm.h"


#include "content/vlanlist_help_htm.h"
#include "content/vlanlist_htm.h"
#include "content/vlan_help_htm.h"
#include "content/vlannew_htm.h"
#include "content/vlannew_help_htm.h"
#include "content/vlan_htm.h"
#include "content/vlan_redirect_htm.h"
#include "content/addpvlan_htm.h"
#include "content/pvlanhelp_htm.h"
#include "content/pvlannew_htm.h"
#include "content/pvlanview_htm.h"
#include "content/statist_htm.h"
#include "content/statist_help_htm.h"
#include "content/trunkhelp1_htm.h"
#include "content/trunk_htm.h"
#include "content/mirror_htm.h"
#include "content/mirror_help_htm.h"
#include "content/qos_htm.h"
#include "content/qos_help1_htm.h"
#include "content/pqos_htm.h"
#include "content/qos_redirect_htm.h"
#include "content/qoshelp1_htm.h"
#include "content/pqoshelp_htm.h"
#include "content/multicast_htm.h"
#include "content/multicast_help_htm.h"
#include "content/b_gif.h"
#include "content/t_gif.h"
#include "content/u_gif.h"

#include "content/password_htm.h"
#include "content/password_help_htm.h"
#include "content/accesscontrol_htm.h"
#include "content/accesscontrol_help_htm.h"
#include "content/errormsg_htm.h"
#include "content/loopdet_htm.h"
#include "content/loopdet_help_htm.h"
#include "content/rate_htm.h"
#include "content/ratehelp1_htm.h"
#include "content/ratehelp2_htm.h"
#include "content/setrate_htm.h"
#include "content/storm_htm.h"
#include "content/stormhelp_htm.h"
#include "content/cable_htm.h"
#include "content/cable_help_htm.h"

#include "content/top_css.h"
#include "content/top_paneld_gif.h"
#include "content/top_bar_gif.h"
#include "content/top_logo_gif.h"
#include "content/top_lgray_gif.h"
#include "content/top_lgreen_gif.h"
#include "content/fold_gif.h"
#include "content/index_htm.h"
#include "content/left_htm.h"
#include "content/left_css.h"
#include "content/top_htm.h"
#include "content/top_left_htm.h"
#include "content/top_right_htm.h"
#include "content/top_mid_htm.h"
#include "content/login_htm.h"
#include "content/restore_htm.h"
#include "content/upload_vc_htm.h"

#endif  /*  __BOOTLOADER__ */

#ifdef __BOOTLOADER__
SSP_HANDLER_RETVAL ssphandler_fupgrade_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_fupg_abort_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_fupgrade_htm(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
#else  /*  __BOOTLOADER__ */

SSP_HANDLER_RETVAL ssphandler_setport_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_portset_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_system_name_set_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_bonjour_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_reset_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_sys_action_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_showvlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_newvlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_delvlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_setvlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_switchpage_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_addpvlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_showaddpvlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_delpvlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_pagevlan_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_clear_counter_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_trunk_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_password_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_accesscontrol_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_adminpriv_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_login_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_logout_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_loopdet_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_mirror_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_rate_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_setrate_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_storm_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_qos_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_pqos_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_swqospage_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_cable_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_multicast_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_text_css(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_config_data_restore_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_config_data_save_handler(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_vendor_config_upload_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;

#endif  /*  __BOOTLOADER__ */


SSP_FILE_ENTRY CODE ssp_fs_root[] = {
#ifdef __BOOTLOADER__
    { "", ssphandler_fupgrade_htm, sspfile_fupgrade_htm, 0, SSP_BANK5 },
    { "fupgrade.cgi", ssphandler_fupgrade_cgi, sspfile_frestart_htm, SSPF_FILE_UPLOAD_H|SSPF_OPEN_CLOSE_H|SSPF_NO_CACHE, SSP_BANK5 },
    { "fupg_abort.cgi", ssphandler_fupg_abort_cgi, sspfile_frestart_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
#else  /*  __BOOTLOADER__ */
    { "", NULL, sspfile_login_htm, 0, SSP_BANK4 },
    { "reset.cgi", ssphandler_reset_cgi, sspfile_reset2_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "sys_action.cgi",  ssphandler_sys_action_cgi, sspfile_system_htm, SSPF_QUERY_STRINGS_H, SSP_BANK4 },
    { "password.htm", NULL, sspfile_password_htm, 0, SSP_BANK4 },
    { "password.cgi", ssphandler_password_cgi, sspfile_password_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "accesscontrol.htm", NULL, sspfile_accesscontrol_htm, SSPF_NO_CACHE, SSP_BANK4 },
    { "accesscontrol.cgi", ssphandler_accesscontrol_cgi, sspfile_accesscontrol_htm, SSPF_QUERY_STRINGS_H|SSPF_NO_CACHE, SSP_BANK5 },
    { "accesscontrol_help.htm", NULL, sspfile_accesscontrol_help_htm, 0, SSP_BANK5 },
    { "adminpriv.cgi", ssphandler_adminpriv_cgi, sspfile_accesscontrol_htm, SSPF_QUERY_STRINGS_H|SSPF_NO_CACHE, SSP_BANK5 },
    { "login.cgi", ssphandler_login_cgi, sspfile_login_htm, SSPF_QUERY_STRINGS_H|SSPF_REQUEST_COOKIE_H|SSPF_SET_COOKIE_H, SSP_BANK5 },
    { "password_help.htm", NULL, sspfile_password_help_htm, 0, SSP_BANK5 },
    { "errormsg.htm", NULL, sspfile_errormsg_htm, 0, SSP_BANK5 },
    { "loopdet.cgi", ssphandler_loopdet_cgi, sspfile_loopdet_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "loopdet.htm", NULL, sspfile_loopdet_htm, 0, SSP_BANK4 },
    { "loopdet_help.htm", NULL, sspfile_loopdet_help_htm, 0, SSP_BANK4 },
    { "rate.cgi", ssphandler_rate_cgi, sspfile_setrate_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "rate.htm", NULL, sspfile_rate_htm, 0, SSP_BANK4 },
    { "ratehelp1.htm", NULL, sspfile_ratehelp1_htm, 0, SSP_BANK5 },
    { "ratehelp2.htm", NULL, sspfile_ratehelp2_htm, 0, SSP_BANK5 },
    { "setrate.cgi", ssphandler_setrate_cgi, sspfile_rate_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "setrate.htm", NULL, sspfile_setrate_htm, 0, SSP_BANK4 },
    { "storm.cgi", ssphandler_storm_cgi, sspfile_storm_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "storm.htm", NULL, sspfile_storm_htm, 0, SSP_BANK4 },
    { "stormhelp.htm", NULL, sspfile_stormhelp_htm, 0, SSP_BANK5 },
    { "cable.cgi", ssphandler_cable_cgi, sspfile_cable_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "cable.htm", NULL, sspfile_cable_htm, 0, SSP_BANK4 },
    { "cable_help.htm", NULL, sspfile_cable_help_htm, 0, SSP_BANK4 },
/*  Port Status Web Page */
    { "portconf.htm", NULL, sspfile_portconf_htm, 0, SSP_BANK5 },
    { "porthelp1.htm", NULL, sspfile_porthelp1_htm, 0, SSP_BANK5 },
    { "setport.cgi", ssphandler_setport_cgi, sspfile_portconf_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    /*{ "portset.cgi", ssphandler_portset_cgi, sspfile_portconf_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },*/
    { "porthelp2.htm", NULL, sspfile_porthelp2_htm, 0, SSP_BANK5 },

    { "login.htm", NULL, sspfile_login_htm, SSPF_NO_CACHE, SSP_BANK4 },
    { "logout.htm", ssphandler_logout_cgi, sspfile_login_htm, SSPF_NO_CACHE|SSPF_OPEN_CLOSE_H, SSP_BANK4 },
    { "index.htm", NULL, sspfile_index_htm, SSPF_NO_CACHE, SSP_BANK4 },
    { "left.htm", NULL, sspfile_left_htm, SSPF_NO_CACHE, SSP_BANK4 },
    { "top.htm", NULL, sspfile_top_htm, SSPF_NO_CACHE, SSP_BANK4 },
    { "top_left.htm", NULL, sspfile_top_left_htm, SSPF_NO_CACHE, SSP_BANK4 },
    { "top_right.htm", NULL, sspfile_top_right_htm, SSPF_NO_CACHE, SSP_BANK4 },
    { "top_mid.htm", NULL, sspfile_top_mid_htm, SSPF_NO_CACHE, SSP_BANK4 },
    { "system.htm", NULL, sspfile_system_htm, SSPF_NO_CACHE, SSP_BANK4 },
    { "system_help.htm", NULL, sspfile_system_help_htm, 0, SSP_BANK5 },
    { "system_name.htm", NULL, sspfile_system_name_htm, 0, SSP_BANK5 },
    { "system_name_set.cgi", ssphandler_system_name_set_cgi, sspfile_system_htm, SSPF_QUERY_STRINGS_H, SSP_BANK4 },
#if defined(CFG_ZEROCONF_MDNS_INCLUDED)
    { "bonjour.cgi", ssphandler_bonjour_cgi, sspfile_system_htm, SSPF_QUERY_STRINGS_H, SSP_BANK4 },
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */
    { "left.css", ssphandler_text_css, sspfile_left_css, SSPF_SET_HEADER_H|SSPF_PUBLIC_RESOURCE|SSPF_FORCE_CACHE, SSP_BANK5 },
    { "fold.gif", NULL, sspfile_fold_gif, SSPF_PUBLIC_RESOURCE|SSPF_FORCE_CACHE, SSP_BANK5 },
    { "top_bar.gif", NULL, sspfile_top_bar_gif, SSPF_PUBLIC_RESOURCE|SSPF_FORCE_CACHE, SSP_BANK5 },
    { "top_logo.gif", NULL, sspfile_top_logo_gif, SSPF_PUBLIC_RESOURCE|SSPF_FORCE_CACHE, SSP_BANK5 },
    { "top_paneld.gif", NULL, sspfile_top_paneld_gif, SSPF_PUBLIC_RESOURCE|SSPF_FORCE_CACHE, SSP_BANK5 },
    { "top_lgray.gif", NULL, sspfile_top_lgray_gif, SSPF_PUBLIC_RESOURCE|SSPF_FORCE_CACHE, SSP_BANK5 },
    { "top_lgreen.gif", NULL, sspfile_top_lgreen_gif, SSPF_PUBLIC_RESOURCE|SSPF_FORCE_CACHE, SSP_BANK5 },
    { "top.css", ssphandler_text_css, sspfile_top_css, SSPF_SET_HEADER_H|SSPF_PUBLIC_RESOURCE|SSPF_FORCE_CACHE, SSP_BANK5 },
    { "bg.gif", NULL, sspfile_bg_gif, SSPF_PUBLIC_RESOURCE|SSPF_FORCE_CACHE, SSP_BANK5 },
    { "form.css", ssphandler_text_css, sspfile_form_css, SSPF_SET_HEADER_H|SSPF_PUBLIC_RESOURCE|SSPF_FORCE_CACHE, SSP_BANK5 },
    { "newvlan.cgi", ssphandler_newvlan_cgi, sspfile_vlan_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "showvlan.cgi", ssphandler_showvlan_cgi, sspfile_vlan_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "setvlan.cgi", ssphandler_setvlan_cgi, sspfile_vlan_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "delvlan.cgi", ssphandler_delvlan_cgi, sspfile_vlanlist_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "switchpage.cgi", ssphandler_switchpage_cgi, sspfile_vlan_redirect_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },

    { "pagevlan.cgi", ssphandler_pagevlan_cgi, sspfile_vlanlist_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "trunk.cgi", ssphandler_trunk_cgi, sspfile_trunk_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "mirror.cgi", ssphandler_mirror_cgi, sspfile_mirror_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "qos.cgi", ssphandler_qos_cgi, sspfile_qos_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "pqos.cgi", ssphandler_pqos_cgi, sspfile_pqos_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "swqospage.cgi", ssphandler_swqospage_cgi, sspfile_pqos_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
#ifdef CFG_SWITCH_MCAST_INCLUDED
    { "multicast.cgi", ssphandler_multicast_cgi, sspfile_multicast_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "multicast.htm", NULL, sspfile_multicast_htm, 0, SSP_BANK5 },
    { "multicast_help.htm", NULL, sspfile_multicast_help_htm, 0, SSP_BANK5 },
#endif
    { "vlannew.htm", NULL, sspfile_vlannew_htm, 0, SSP_BANK5 },
    { "vlannew_help.htm", NULL, sspfile_vlannew_help_htm, 0, SSP_BANK5 },
    { "vlan.htm", NULL, sspfile_vlan_htm, 0, SSP_BANK5 },
    { "vlan_help.htm", NULL, sspfile_vlan_help_htm, 0, SSP_BANK5 },
    { "vlanlist_help.htm", NULL, sspfile_vlanlist_help_htm, 0, SSP_BANK5 },
    { "vlanlist.htm", NULL, sspfile_vlanlist_htm, 0, SSP_BANK5 },
    { "vlan_redirect.htm", NULL, sspfile_vlan_redirect_htm, 0, SSP_BANK5 },
#if defined(CFG_SWITCH_PVLAN_INCLUDED)
    { "pvlannew.htm", NULL, sspfile_pvlannew_htm, 0, SSP_BANK5 },
    { "addpvlan.htm", NULL, sspfile_addpvlan_htm, 0, SSP_BANK5 },
    { "pvlanview.htm", NULL, sspfile_pvlanview_htm, 0, SSP_BANK5 },
    { "pvlanhelp.htm", NULL, sspfile_pvlanhelp_htm, 0, SSP_BANK5 },
    { "addpvlan.cgi", ssphandler_addpvlan_cgi, sspfile_pvlanview_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "showaddpvlan.cgi", ssphandler_showaddpvlan_cgi, sspfile_addpvlan_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
    { "delpvlan.cgi", ssphandler_delpvlan_cgi, sspfile_pvlanview_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },
#endif    
    { "trunk.htm", NULL, sspfile_trunk_htm, 0, SSP_BANK5 },
    { "trunkhelp1.htm", NULL, sspfile_trunkhelp1_htm, 0, SSP_BANK5 },
#if defined(CFG_SWITCH_STAT_INCLUDED)    
    { "statist.htm", NULL, sspfile_statist_htm, 0, SSP_BANK5 },
    { "statist_help.htm", NULL, sspfile_statist_help_htm, 0, SSP_BANK5 },
    { "clear_counter.cgi", ssphandler_clear_counter_cgi, sspfile_statist_htm, SSPF_QUERY_STRINGS_H, SSP_BANK5 },    
#endif

    { "errormsg.htm", NULL, sspfile_errormsg_htm, 0, SSP_BANK5 },
    { "mirror.htm", NULL, sspfile_mirror_htm, 0, SSP_BANK5 },
    { "mirror_help.htm", NULL, sspfile_mirror_help_htm, 0, SSP_BANK5 },
    { "qos.htm", NULL, sspfile_qos_htm, 0, SSP_BANK5 },
    { "qoshelp1.htm", NULL, sspfile_qoshelp1_htm, 0, SSP_BANK5 },
    { "pqos.htm", NULL, sspfile_pqos_htm, 0, SSP_BANK5 },
    { "qos_redirect.htm", NULL, sspfile_qos_redirect_htm, 0, SSP_BANK5 },
    { "qoshelp1.htm", NULL, sspfile_qoshelp1_htm, 0, SSP_BANK5 },
    { "pqoshelp.htm", NULL, sspfile_pqoshelp_htm, 0, SSP_BANK5 },
    { "restore.htm", NULL, sspfile_restore_htm, SSPF_NO_CACHE, SSP_BANK5 },
    { "restore.cgi", ssphandler_config_data_restore_cgi, sspfile_reset2_htm, SSPF_FILE_UPLOAD_H|SSPF_OPEN_CLOSE_H|SSPF_NO_CACHE, SSP_BANK5 },
    { "upload_vc.htm", NULL, sspfile_upload_vc_htm, SSPF_NO_CACHE, SSP_BANK5 },
    { "upload_vc.cgi", ssphandler_vendor_config_upload_cgi, sspfile_reset2_htm, SSPF_FILE_UPLOAD_H|SSPF_OPEN_CLOSE_H|SSPF_NO_CACHE, SSP_BANK5 },
    { "um.cfg", ssphandler_config_data_save_handler, (RES_CONST_DECL SSP_DATA_ENTRY *) sspfile_switch_config, SSPF_PUBLIC_RESOURCE | SSPF_SET_HEADER_H | SSPF_OPEN_CLOSE_H, SSP_BANK5 },
    { "b.gif", NULL, sspfile_b_gif, SSPF_PUBLIC_RESOURCE|SSPF_FORCE_CACHE, SSP_BANK5 },
    { "t.gif", NULL, sspfile_t_gif, SSPF_PUBLIC_RESOURCE|SSPF_FORCE_CACHE, SSP_BANK5 },
    { "u.gif", NULL, sspfile_u_gif, SSPF_PUBLIC_RESOURCE|SSPF_FORCE_CACHE, SSP_BANK5 },
#endif  /*  __BOOTLOADER__ */
    { NULL, NULL, NULL, 0 },
};

