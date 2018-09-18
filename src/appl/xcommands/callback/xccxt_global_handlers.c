/*
 * $Id: xccxt_global_handlers.c,v 1.11 Broadcom SDK $
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

#ifdef CFG_XCOMMAND_INCLUDED

#include "appl/xcmd/xcmd_public.h"
#include "generated/xccxt_global_enums.h"
#include "xccvt_converters.h"

#include "appl/persistence.h"
#include "appl/igmpsnoop.h"
#include "utils/system.h"
#include "utils/net.h"
#include "utils/ports.h"

#ifdef CFG_SWITCH_MIRROR_INCLUDED
/*
  *
  *
  *    Helper function 
  *
  *
  */

typedef struct {
    uint8 enable;
    uint16 dstnzport;
    uplist_t uplist;
} MIRROR_CFG;;


static MIRROR_CFG mirror_cfg = {
      0, 
      0,
      {{0}}
};

/*
 * Function:
 *      xchandler_global_mirror_gather_information
 * Purpose:
 *      gather mirror setting and pack it into a struct , MIRROR_CFG  
 * Parameters:
 *      xch : command handler , key to access command related information             
 * Returns:
 *      XCMD_ERR_xxx
 */

static XCMD_ERROR 
xchandler_global_mirror_gather_information(XCMD_HANDLE xch) 
{

 uint16 uport;
 uint8 enable;
 
 SAL_UPORT_ITER(uport) {
      enable = FALSE;
      board_mirror_port_get(uport, &enable);
      if (enable == TRUE) {
           uplist_port_add(mirror_cfg.uplist.bytes, uport);
           mirror_cfg.enable = TRUE;
      }
 }

 board_mirror_to_get(&mirror_cfg.dstnzport);
 
 mirror_cfg.dstnzport = SAL_UPORT_TO_NZUPORT(mirror_cfg.dstnzport);
 
 return XCMD_ERR_OK;
}
#endif /* CFG_SWITCH_MIRROR_INCLUDED */

/*
 * Function:
 *      xchandler_global_mirror_config
 * Purpose:
 *      config detail mirror setting  
 * Parameters:
 *      xch : command handler , key to access command related information    
 * 
 *      dstnzport : mirror destination port 
 *      src_port_list: is mirror source port list 
 *                          where is ASCII port list string, each port separate by ","
 * Returns:
 *      XCMD_ERR_xxx
 */
static XCMD_ERROR 
xchandler_global_mirror_config(XCMD_HANDLE xch, const uint16 dstnzport, const uplist_t *puplist) {

#ifdef CFG_SWITCH_MIRROR_INCLUDED
   uint8 store_setting = 0;
   uint16 uport, dstuport;   
   xchandler_global_mirror_gather_information(xch);

   if (dstnzport != 0) {
       dstuport = SAL_NZUPORT_TO_UPORT(dstnzport);
       // Setting mirror destination port according previous record   
       if (!SAL_UPORT_IS_NOT_VALID(dstuport)) {  
       /* check setting */
       if (uplist_port_matched(mirror_cfg.uplist.bytes, dstuport) == SYS_OK) {
           sal_printf("Destination port can not be one of source port \n");
           return XCMD_ERR_FAILED_TO_OPERATE;
       }
       /* apply setting */
       board_mirror_to_set(dstuport);      
       store_setting = 1;
       } else {
          sal_printf("check port range fail 1-%d\n",  board_uport_count());
       }
   }
   xchandler_global_mirror_gather_information(xch);

   if (puplist != NULL) {   

       SAL_UPORT_ITER(uport) {
            if ((uplist_port_matched(puplist->bytes, uport) == SYS_OK) && 
                (uport == (dstuport))) {
                sal_printf("Any mirror source port can not be the same with destination port \n");
                return XCMD_ERR_FAILED_TO_OPERATE;                
            } 
       }

       SAL_UPORT_ITER(uport) {
            if ((uplist_port_matched(puplist->bytes, uport) == SYS_OK)) {
                board_mirror_port_set(uport ,TRUE);
            } else {
                board_mirror_port_set(uport ,FALSE);
            }
       }
       store_setting = 1;

   } 
#if CFG_PERSISTENCE_SUPPORT_ENABLED
   if (store_setting) {
       if (persistence_save_current_settings("mirror") != TRUE) {  
           sal_printf("xchandler_config_multicast: Save block unknown MCAST to persistence failed!\n");    
       }  

   }
#endif

   xchandler_global_mirror_gather_information(xch);

#endif /* CFG_SWITCH_MIRROR_INCLUDED */

   return XCMD_ERR_OK;

}

/*
 * Function:
 *      xchandler_global_mirror_enable
 * Purpose:
 *      enable mirror setting according previous setting   
 * Parameters:
 *      xch : command handler , key to access command related information    
 * Returns:
 *      XCMD_ERR_xxx
 */

XCMD_ERROR 
xchandler_global_mirror_enable(XCMD_HANDLE xch) {

#ifdef CFG_SWITCH_MIRROR_INCLUDED
    int i;
    char *port_list;
    if (mirror_cfg.enable) {

         xchandler_global_mirror_gather_information(xch);
         sal_printf("mirror has been enabled");

    } else {
    
         SAL_UPORT_ITER(i) {
             if ((uplist_port_matched(mirror_cfg.uplist.bytes, i) == SYS_OK)) {
                 board_mirror_port_set(i, TRUE);
                 mirror_cfg.enable = 1;
             } else {
                 board_mirror_port_set(i, FALSE);
             }
         }

         board_mirror_to_set(SAL_NZUPORT_TO_UPORT(mirror_cfg.dstnzport));
         sal_printf("re-enable mirror %s according previous setting\n", mirror_cfg.enable ? "success" : "fail");
#if CFG_PERSISTENCE_SUPPORT_ENABLED         
         if (persistence_save_current_settings("mirror") != TRUE) {  
             sal_printf("xchandler_config_multicast: Save block unknown MCAST to persistence failed!\n");    
         }  
#endif 

    }    

    xc_uplist_to_string(&mirror_cfg.uplist, NULL, &port_list);
    sal_printf("    dstination port :%d\n", mirror_cfg.dstnzport); 
    sal_printf("    port_list : %s\n", port_list); 
    xccvt_common_free_string(port_list, NULL);
   
#endif /* CFG_SWITCH_MIRROR_INCLUDED */

    return XCMD_ERR_OK;
}

/*
 * Function:
 *      xchandler_global_mirror_disable
 * Purpose:
 *      record current mirror setting and then disable mirror    
 * Parameters:
 *      xch : command handler , key to access command related information    
 * Returns:
 *      XCMD_ERR_xxx
 */

XCMD_ERROR 
xchandler_global_mirror_disable(XCMD_HANDLE xch) {

#ifdef CFG_SWITCH_MIRROR_INCLUDED

    int i;

    xchandler_global_mirror_gather_information(xch);
     
    SAL_UPORT_ITER(i) {
        board_mirror_port_set(i, FALSE);
    }
    board_mirror_to_set(0);
    
#if CFG_PERSISTENCE_SUPPORT_ENABLED
    if (persistence_save_current_settings("mirror") != TRUE) {  
        sal_printf("xchandler_config_multicast: Save block unknown MCAST to persistence failed!\n");    
    }  
#endif
    mirror_cfg.enable = 0;
    
#endif /* CFG_SWITCH_MIRROR_INCLUDED */

    return XCMD_ERR_OK;
}


/*
  *
  *
  *
  */
/* 
 * TOP-LEVEL-COMMAND: enable
 */
XCMD_ERROR
xchandler_global_enable(int path, XCMD_HANDLE xch)
{

    sys_error_t rv = SYS_OK;    
    BOOL bool_rv = TRUE;


    UNREFERENCED_PARAMETER(rv);
    UNREFERENCED_PARAMETER(bool_rv);

    switch(path) {

    /*
     * COMMAND: enable mirror 
     */
    case XCPATH_GLOBAL_H2_ENABLE_MIRROR: {

#ifdef CFG_SWITCH_MIRROR_INCLUDED

        /* ... Call underlying API ... */
        xchandler_global_mirror_enable(xch);

#endif /* CFG_SWITCH_MIRROR_INCLUDED */

        return XCMD_ERR_OK;
            
      }
#ifdef CFG_SWITCH_MCAST_INCLUDED

    /*
     * COMMAND: enable igmpsnoop vlan <VID:integer> 
     */
    case XCPATH_GLOBAL_H2_ENABLE_IGMPSNOOP: {

        unsigned int vid; /* integer */

        XCMD_GET_NUMBER(XCID_GLOBAL_ENABLE_VID, &vid);
        
        /* ... Use these values to call API ... */
        bool_rv = igmpsnoop_enable_set(TRUE);   

        if (bool_rv != TRUE) {  
            sal_printf("xchandler_config_multicast: Enable IGMPSNOOP failed!\n");   
        }   

        rv = igmpsnoop_vid_set((uint16)vid);    

        if (rv != SYS_OK) {     
            sal_printf("xchandler_config_multicast: Set VLAN ID for IGMPSNOOP failed!\n");  
        }   
#if CFG_PERSISTENCE_SUPPORT_ENABLED
        bool_rv = persistence_save_current_settings("igmpsnoop");   

        if (bool_rv != TRUE) {  
            sal_printf("xchandler_config_multicast: Save IGMPSNOOP to persistence failed!\n");  
        }
#endif
        return XCMD_ERR_OK;
      }

    /*
     * COMMAND: enable block_unknown_multicast 
     */
    case XCPATH_GLOBAL_H2_ENABLE_BLOCK_UNKNOWN_MULTICAST: {

        /* ... Call underlying API ... */
        rv = board_block_unknown_mcast_set(TRUE);   

        if (rv != SYS_OK) {     
            sal_printf("xchandler_config_multicast: Enable block unknown MCAST failed!\n");     
        }   
#if CFG_PERSISTENCE_SUPPORT_ENABLED

        bool_rv = persistence_save_current_settings("mcast");   

        if (bool_rv != TRUE) {  
            sal_printf("xchandler_config_multicast: Save block unknown MCAST to persistence failed!\n");    
        }   
#endif        
        return XCMD_ERR_OK;     

      }
#endif /* CFG_SWITCH_MIRROR_INCLUDED  */
    default:
        return XCMD_ERR_INTERNAL_INVALID_PATH;
    }

    return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
}


/* 
 * TOP-LEVEL-COMMAND: disable
 */
XCMD_ERROR
xchandler_global_disable(int path, XCMD_HANDLE xch)
{
    sys_error_t rv = SYS_OK;    
    BOOL bool_rv = TRUE;

    UNREFERENCED_PARAMETER(rv);
    UNREFERENCED_PARAMETER(bool_rv);
    
    switch(path) {

    /*
     * COMMAND: disable mirror 
     */
    case XCPATH_GLOBAL_H2_DISABLE_MIRROR: {

#ifdef CFG_SWITCH_MIRROR_INCLUDED

        /* ... Call underlying API ... */
        xchandler_global_mirror_disable(xch);

#endif /* CFG_SWITCH_MIRROR_INCLUDED */

        return XCMD_ERR_OK;
      }

#ifdef CFG_SWITCH_MCAST_INCLUDED

    /*
     * COMMAND: disable igmpsnoop 
     */
    case XCPATH_GLOBAL_H2_DISABLE_IGMPSNOOP: {

        /* ... Call underlying API ... */
        bool_rv = igmpsnoop_enable_set(FALSE);  

        if (bool_rv != TRUE) {  
            sal_printf("xchandler_config_multicast: Disable IGMPSNOOP failed!\n");  
        }   
#if CFG_PERSISTENCE_SUPPORT_ENABLED
        bool_rv = persistence_save_current_settings("igmpsnoop");   

        if (bool_rv != TRUE) {  
            sal_printf("xchandler_config_multicast: Save IGMPSNOOP to persistence failed!\n");  
        }
#endif
        return XCMD_ERR_OK;
      }

    /*
     * COMMAND: disable block_unknown_multicast 
     */
    case XCPATH_GLOBAL_H2_DISABLE_BLOCK_UNKNOWN_MULTICAST: {

        /* ... Call underlying API ... */
        rv = board_block_unknown_mcast_set(FALSE);  

        if (rv != SYS_OK) {     
            sal_printf("xchandler_config_multicast: Disable block unknown MCAST failed!\n");    
        }   
#if CFG_PERSISTENCE_SUPPORT_ENABLED
        bool_rv = persistence_save_current_settings("mcast");   

        if (bool_rv != TRUE) {  
            sal_printf("xchandler_config_multicast: Save block unknown MCAST to persistence failed!\n");    
        }
#endif
        return XCMD_ERR_OK;
    }
#endif /* CFG_SWITCH_MCAST_INCLUDED */
    default:
        return XCMD_ERR_INTERNAL_INVALID_PATH;
    }

    return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
}


/* 
 * TOP-LEVEL-COMMAND: config
 */
XCMD_ERROR
xchandler_global_config(int path, XCMD_HANDLE xch)
{
    switch(path) {

    /*
     * COMMAND: config port <PORT_NAME:line> 
     */
    case XCPATH_GLOBAL_H2_CONFIG_PORT: {

        unsigned int nzuport; /* integer */
        const char * port_name; /* line */

        XCMD_GET_NUMBER(XCID_GLOBAL_CONFIG_PORT_ID, &nzuport);
        XCMD_GET_STRING(XCID_GLOBAL_CONFIG_PORT_NAME, &port_name);

        /* ... Use these values to call API ... */
        set_port_desc((uint16)SAL_NZUPORT_TO_UPORT(nzuport), port_name);
#if CFG_PERSISTENCE_SUPPORT_ENABLED        
        persistence_save_current_settings("portdesc");
#endif
        return XCMD_ERR_OK;
      }

#ifdef CFG_SWITCH_MCAST_INCLUDED

    /*
     * COMMAND: config igmpsnoop version <VER_ID:integer> 
     */
    case XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_VERSION: {

        unsigned int ver_id; /* integer */

        /* ... Use these values to call API ... */
        XCMD_GET_NUMBER(XCID_GLOBAL_CONFIG_VER_ID, &ver_id);
        sal_printf("ver_id = %d\n", ver_id);

        return XCMD_ERR_OK;
      }

    /*
     * COMMAND: config igmpsnoop query_interval <SECS:integer> 
     */
    case XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_QUERY_INTERVAL: {

        unsigned int secs; /* integer */
        
        /* ... Use these values to call API ... */
        XCMD_GET_NUMBER(XCID_GLOBAL_CONFIG_SECS, &secs);
        sal_printf("secs = %d\n", secs);

        return XCMD_ERR_OK;
      }

    /*
     * COMMAND: config igmpsnoop robustness_variable <TIMES:integer> 
     */
    case XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_ROBUSTNESS_VARIABLE: {

        unsigned int times; /* integer */
        
        /* ... Use these values to call API ... */
        XCMD_GET_NUMBER(XCID_GLOBAL_CONFIG_TIMES, &times);
        sal_printf("times = %d\n", times);

        return XCMD_ERR_OK;
      }

    /*
     * COMMAND: config igmpsnoop all 
     */
    case XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_ALL: {

        /* Reserve ALL path for customer to set all parameters by one API */
        /* ... Use these values to call API ... */
        return XCMD_ERR_OK;
    }
#endif /* CFG_SWITCH_MCAST_INCLUDED */
    /*
     * COMMAND: config mirror destination_port <PORTNAME:integer> 
     */
    case XCPATH_GLOBAL_H3_CONFIG_MIRROR_DESTINATION_PORT: {

        unsigned int port_id; /* integer */

        XCMD_GET_NUMBER(XCID_GLOBAL_CONFIG_PORT_ID, &port_id);

        /* ... Use these values to call API ... */
        xchandler_global_mirror_config(xch, port_id, NULL);
               
        return XCMD_ERR_OK;
      }

    /*
     * COMMAND: config mirror source_port_list <PORT_LIST:line> 
     */
    case XCPATH_GLOBAL_H3_CONFIG_MIRROR_SOURCE_PORT_LIST: {

        uplist_t port_list; /* port_list */
        
        XCMD_GET_VALUE(XCID_GLOBAL_CONFIG_PORT_LIST, &port_list);


        /* ... Use these values to call API ... */
        xchandler_global_mirror_config(xch, 0, &port_list);
        
        return XCMD_ERR_OK;
      }

    default:
        return XCMD_ERR_INTERNAL_INVALID_PATH;
    }

    return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
}


/* 
 * TOP-LEVEL-COMMAND: show
 */
XCMD_ERROR
xchandler_global_show(int path, XCMD_HANDLE xch)
{
    switch(path) {

    /*
     * COMMAND: show counter 
     */
    case XCPATH_GLOBAL_H2_SHOW_COUNTER: {
#if defined(CFG_SWITCH_STAT_INCLUDED)
        uint16 uport; 
        port_stat_t stat;
        /* ... Call underlying API ... */
        XC_DISPLAY_PAGES_DO(xch) {
        
            SAL_UPORT_ITER(uport) {
                 board_port_stat_get(uport, &stat);
                 xc_display_pages(xch, " --------------------------------------------------------------------- \r\n");
                 xc_display_pages(xch, "|                                                                     |\r\n");
                 xc_display_pages(xch, "|                  Boardcom port %02d statistics                        |\r\n", uport);
                 xc_display_pages(xch, "|                                                                     |\r\n");
                 xc_display_pages(xch, " --------------------------------------------------------------------- \r\n");
                 xc_display_pages(xch, "                                                                       \r\n");
                 xc_display_pages(xch, "  Tx bytes   %08x%08x                                         \r\n", stat.TxOctets_hi, stat.TxOctets_lo);
                 xc_display_pages(xch, "  Tx packets %08x%08x                                         \r\n", stat.TxPkts_hi, stat.TxPkts_lo);
                 xc_display_pages(xch, "  Tx unicast packets %08x%08x                                 \r\n", stat.TxUnicastPkts_hi, stat.TxUnicastPkts_lo);
                 xc_display_pages(xch, "  Tx multicast packets %08x%08x                               \r\n", stat.TxMulticastPkts_hi, stat.TxMulticastPkts_lo);
                 xc_display_pages(xch, "  Tx broadcast packets %08x%08x                               \r\n", stat.TxBroadcastPkts_hi, stat.TxBroadcastPkts_lo);
                 xc_display_pages(xch, "  Tx pause frames    %08x%08x                                 \r\n", stat.TxPauseFramePkts_hi, stat.TxPauseFramePkts_lo);
                 xc_display_pages(xch, "  Tx oversize frames %08x%08x                                 \r\n", stat.TxOversizePkts_hi, stat.TxOversizePkts_lo);
                 xc_display_pages(xch, "  Tx LPI events   %08x                                        \r\n", stat.TxLPIPkts_lo);
                 xc_display_pages(xch, "  Tx LPI duration %08x                                        \r\n", stat.TxLPIDuration_lo);
                 xc_display_pages(xch, "  Rx bytes   %08x%08x                                         \r\n", stat.RxOctets_hi, stat.RxOctets_lo);
                 xc_display_pages(xch, "  Rx packets %08x%08x                                         \r\n", stat.RxPkts_hi, stat.RxPkts_lo);
                 xc_display_pages(xch, "  Rx unicast packets %08x%08x                                 \r\n", stat.RxUnicastPkts_hi, stat.RxUnicastPkts_lo);
                 xc_display_pages(xch, "  Rx multicast packets %08x%08x                               \r\n", stat.RxMulticastPkts_hi, stat.RxMulticastPkts_lo);
                 xc_display_pages(xch, "  Rx broadcast packets %08x%08x                               \r\n", stat.RxBroadcastPkts_hi, stat.RxBroadcastPkts_lo);
                 xc_display_pages(xch, "  Rx pause frames    %08x%08x                                 \r\n", stat.RxPauseFramePkts_hi, stat.RxPauseFramePkts_lo);
                 xc_display_pages(xch, "  Rx oversize frames %08x%08x                                 \r\n", stat.RxOversizePkts_hi, stat.RxOversizePkts_lo);
                 xc_display_pages(xch, "  Rx LPI events   %08x                                        \r\n", stat.RxLPIPkts_lo);
                 xc_display_pages(xch, "  Rx LPI duration %08x                                        \r\n", stat.RxLPIDuration_lo);
                 xc_display_pages(xch, "  Rx FCS Error Frames %08x%08x                                \r\n", stat.CRCErrors_hi, stat.CRCErrors_lo);
                 xc_display_pages(xch, "  Egress hold drop packets %08x                         \r\n", stat.EgressDropPkts_lo);
                 xc_display_pages(xch, "                                                                      \r\n");
                 xc_display_pages(xch, "                                                                      \r\n");
                 xc_display_pages(xch, "                                                                      \r\n");
                 xc_display_pages(xch, "                                                                      \r\n");
            }

        } XC_DISPLAY_PAGES_WHILE(xch);

        return XCMD_ERR_OK;
#else /* defined(CFG_SWITCH_STAT_INCLUDED) */
        return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
#endif /* defined(CFG_SWITCH_STAT_INCLUDED) */

      }

    /*
     * COMMAND: show system 
     */
    case XCPATH_GLOBAL_H2_SHOW_SYSTEM: {
        uint8   ver[4];
        char device_name[MAX_SYSTEM_NAME_LEN+1];
        uint8 addr[6];
#if CFG_UIP_STACK_ENABLED
        uint8 netmask[4], gateway[4];
#endif /* CFG_UIP_STACK_ENABLED */

        if (get_system_name(device_name, MAX_SYSTEM_NAME_LEN+1) != SYS_OK) {
            sal_strcpy(device_name, "-Unknown-");
        }

        sal_printf("\n --------------------------------------------------------------------- \n");
        sal_printf(" Device Name       :  %s\n", device_name);

        board_firmware_version_get(ver, ver+1, ver+2, ver+3);
        sal_printf(" Firmware Version  :  Ver %u.%02u.%02u\n",
                (int)ver[0], (int)ver[1], (int)ver[2]);

        sal_printf(" Build Date        :  %s\n", __DATE__);

        get_system_mac(addr);
        sal_printf(" MAC Address       :  %02x:%02x:%02x:%02x:%02x:%02x\n",
                (int)addr[0], (int)addr[1], (int)addr[2], 
                (int)addr[3], (int)addr[4], (int)addr[5]);

#if CFG_UIP_STACK_ENABLED
        get_network_interface_config(addr, netmask, gateway);

        sal_printf(" IPv4 Address      :  %u.%u.%u.%u\n",
                (int)addr[0], (int)addr[1], (int)addr[2],(int)addr[3]); 

        sal_printf(" Subnet Mask       :  %u.%u.%u.%u\n",
                (int)netmask[0], (int)netmask[1], (int)netmask[2], 
                (int)netmask[3]);
        
        sal_printf(" Gateway           :  %u.%u.%u.%u\n",
                (int)gateway[0], (int)gateway[1], (int)gateway[2], 
                (int)gateway[3]);
#endif /* CFG_UIP_STACK_ENABLED */

        sal_printf(" --------------------------------------------------------------------- \n");
        sal_printf("\n");

        return XCMD_ERR_OK;
      }

    default:
        return XCMD_ERR_INTERNAL_INVALID_PATH;
    }

    return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
}


/* 
 * TOP-LEVEL-COMMAND: exit
 */
XCMD_ERROR
xchandler_global_exit(int path, XCMD_HANDLE xch)
{
    switch(path) {

    /*
     * COMMAND: exit 
     */
    case XCPATH_GLOBAL_H1_EXIT: {

        /* ... Call underlying API ... */

        return XCMD_ERR_EXIT;
      }

    default:
        return XCMD_ERR_INTERNAL_INVALID_PATH;
    }

    return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
}



#endif /* CFG_XCOMMAND_INCLUDED */
