/*
 * $Id: xccxt_global_builders.c,v 1.7 Broadcom SDK $
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
#include "utils/ports.h"

uint8 xcbuilder_is_mirror_enable(void) {

#ifdef CFG_SWITCH_MIRROR_INCLUDED
    uint16 uport;
    uint8 enable;

    enable = FALSE;
    
    SAL_UPORT_ITER(uport) {
          board_mirror_port_get((uint16)uport, &enable);
          if (TRUE == enable) {
              break;
          }
    }

    return enable;
#else
    
    return FALSE;
    
#endif /* CFG_SWITCH_MIRROR_INCLUDED */

}

#ifdef CFG_SWITCH_MCAST_INCLUDED

uint8 xcbuilder_is_igmpsnoop_enable(void) {

   uint8 enable;

   enable = FALSE;

   igmpsnoop_enable_get(&enable);

   return enable;
}

uint8 xcbuilder_is_block_unknow_multicast_enable(void) {

   uint8 enable;

   enable = FALSE;

   board_block_unknown_mcast_get(&enable);

   return enable;
}

#endif
/* 
 * TOP-LEVEL-COMMAND: help
 */
XCMD_ACTION
xcbuilder_global_help(int path, unsigned int index, XCMD_HANDLE xch)
{
    switch(path) {

    default:
        return XCMD_ERR_INTERNAL_INVALID_PATH;
    }

    return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
}


/* 
 * TOP-LEVEL-COMMAND: enable
 */
XCMD_ACTION
xcbuilder_global_enable(int path, unsigned int index, XCMD_HANDLE xch)
{

    switch(path) {

    /*
     * COMMAND: enable mirror 
     */
    case XCPATH_GLOBAL_H2_ENABLE_MIRROR: {

#ifdef CFG_SWITCH_MIRROR_INCLUDED

        /* ... Retrieve values from API ... */
        if (xcbuilder_is_mirror_enable()) {
             return XCMD_ACTION_SET_AND_STOP;
        } else {
             return XCMD_ACTION_SKIP_AND_STOP;
        }

#else
        
        return XCMD_ACTION_SKIP_AND_STOP;
        
#endif /* CFG_SWITCH_MIRROR_INCLUDED */

    }

#ifdef CFG_SWITCH_MCAST_INCLUDED
    /*
     * COMMAND: enable igmpsnoop vlan <VID:integer> 
     */
    case XCPATH_GLOBAL_H2_ENABLE_IGMPSNOOP: {

        int vid; /* integer */

        uint16 vid16;
        
        /* ... Retrieve values from API ... */
        if (xcbuilder_is_igmpsnoop_enable()) {

            igmpsnoop_vid_get(&vid16);

            vid = vid16;

            XCMD_SET_NUMBER(XCID_GLOBAL_ENABLE_VID, vid);

            return XCMD_ACTION_SET_AND_STOP;


        } else {

            return XCMD_ACTION_SKIP_AND_STOP;


        } 
      }

    /*
     * COMMAND: enable block_unknown_multicast 
     */
    case XCPATH_GLOBAL_H2_ENABLE_BLOCK_UNKNOWN_MULTICAST: {

        /* ... Retrieve values from API ... */
        if (xcbuilder_is_block_unknow_multicast_enable()) {
            return XCMD_ACTION_SET_AND_STOP;
        } else {
            return XCMD_ACTION_SKIP_AND_STOP;
        }
      }
#endif
    default:
        return XCMD_ERR_INTERNAL_INVALID_PATH;
    }

    return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
}


/* 
 * TOP-LEVEL-COMMAND: disable
 */
XCMD_ACTION
xcbuilder_global_disable(int path, unsigned int index, XCMD_HANDLE xch)
{
    switch(path) {

    /*
     * COMMAND: disable mirror 
     */
    case XCPATH_GLOBAL_H2_DISABLE_MIRROR: {

#ifdef CFG_SWITCH_MIRROR_INCLUDED

        /* ... Retrieve values from API ... */
        if (xcbuilder_is_mirror_enable()) {
             return XCMD_ACTION_SKIP_AND_STOP;
        } else {
             return XCMD_ACTION_SET_AND_STOP;
        }

#else
            
        return XCMD_ACTION_SKIP_AND_STOP;
            
#endif /* CFG_SWITCH_MIRROR_INCLUDED */

    }
#ifdef CFG_SWITCH_MCAST_INCLUDED

    /*
     * COMMAND: disable igmpsnoop 
     */
    case XCPATH_GLOBAL_H2_DISABLE_IGMPSNOOP: {

        /* ... Retrieve values from API ... */
        if (xcbuilder_is_igmpsnoop_enable()) {            
            return XCMD_ACTION_SKIP_AND_STOP;
        } else {
            return XCMD_ACTION_SET_AND_STOP;
        }
    }

    /*
     * COMMAND: disable block_unknown_multicast 
     */
    case XCPATH_GLOBAL_H2_DISABLE_BLOCK_UNKNOWN_MULTICAST: {

        /* ... Retrieve values from API ... */
        if (xcbuilder_is_block_unknow_multicast_enable()) {
            return XCMD_ACTION_SKIP_AND_STOP;
        } else {
            return XCMD_ACTION_SET_AND_STOP;
        }

        
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
XCMD_ACTION
xcbuilder_global_config(int path, unsigned int index, XCMD_HANDLE xch)
{

    switch(path) {

    /*
     * COMMAND: config port <PORT_NAME:line> 
     */
    case XCPATH_GLOBAL_H2_CONFIG_PORT: {

        uint16 uport; /* integer */
        char port_name[WEB_PORT_DESC_LEN + 1]; /* line */

        /* ... Retrieve values from API ... */
        uport = (int)index;

        get_port_desc(uport, 
                port_name, WEB_PORT_DESC_LEN + 1);

        XCMD_SET_NUMBER(XCID_GLOBAL_CONFIG_PORT_ID, SAL_UPORT_TO_NZUPORT(uport));
        
        XCMD_SET_STRING(XCID_GLOBAL_CONFIG_PORT_NAME, port_name);

        if(index == (board_uport_count() - 1)) {
            if(sal_strlen(port_name) == 0) {
                return XCMD_ACTION_SKIP_AND_STOP;
            } else {
                return XCMD_ACTION_SET_AND_STOP;
            }
        } else {
            if(sal_strlen(port_name) == 0) {
                return XCMD_ACTION_SKIP_AND_CONTINUE;
            } else {
                return XCMD_ACTION_SET_AND_CONTINUE;
            }
        }
      }
#ifdef CFG_SWITCH_MCAST_INCLUDED
    /*
     * COMMAND: config igmpsnoop version <VER_ID:integer> 
     */
    case XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_VERSION: {

        int ver_id; /* integer */

        /* ... Retrieve values from API ... */
        if (xcbuilder_is_igmpsnoop_enable() == FALSE) {
            return XCMD_ACTION_SKIP_AND_STOP;

        } 

        ver_id = 2;
        
        XCMD_SET_NUMBER(XCID_GLOBAL_CONFIG_VER_ID, ver_id);

        return XCMD_ACTION_SET_AND_STOP;
            // XCMD_ACTION_SET_AND_CONTINUE;
            // XCMD_ACTION_SKIP_AND_STOP;
            // XCMD_ACTION_SKIP_AND_CONTINUE;
      }

    /*
     * COMMAND: config igmpsnoop query_interval <SECS:integer> 
     */
    case XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_QUERY_INTERVAL: {

        int secs; /* integer */

        /* ... Retrieve values from API ... */
        if (xcbuilder_is_igmpsnoop_enable() == FALSE) {
            return XCMD_ACTION_SKIP_AND_STOP;

        } 

        secs = 125;
        
        XCMD_SET_NUMBER(XCID_GLOBAL_CONFIG_SECS, secs);

        return XCMD_ACTION_SET_AND_STOP;
            // XCMD_ACTION_SET_AND_CONTINUE;
            // XCMD_ACTION_SKIP_AND_STOP;
            // XCMD_ACTION_SKIP_AND_CONTINUE;
      }

    /*
     * COMMAND: config igmpsnoop robustness_variable <TIMES:integer> 
     */
    case XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_ROBUSTNESS_VARIABLE: {

        int times; /* integer */

        /* ... Retrieve values from API ... */

        times = 2;
        
        XCMD_SET_NUMBER(XCID_GLOBAL_CONFIG_TIMES, times);

        return XCMD_ACTION_SET_AND_STOP;
            // XCMD_ACTION_SET_AND_CONTINUE;
            // XCMD_ACTION_SKIP_AND_STOP;
            // XCMD_ACTION_SKIP_AND_CONTINUE;
      }

    /*
     * COMMAND: config igmpsnoop all 
     */
    case XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_ALL: {

        /* ... Retrieve values from API ... */

        return XCMD_ACTION_SKIP_AND_STOP;
            // XCMD_ACTION_SET_AND_CONTINUE;
            // XCMD_ACTION_SKIP_AND_STOP;
            // XCMD_ACTION_SKIP_AND_CONTINUE;
      }

#endif /* CFG_SWITCH_MCAST_INCLUDED */
    /*
     * COMMAND: config mirror destination_port <PORTNAME:integer> 
     */
    case XCPATH_GLOBAL_H3_CONFIG_MIRROR_DESTINATION_PORT: {

#ifdef CFG_SWITCH_MIRROR_INCLUDED
        int port_id; /* integer */
        uint16 port_id16; 
        /* ... Retrieve values from API ... */

        if (xcbuilder_is_mirror_enable()) {

             board_mirror_to_get(&port_id16);

             port_id = port_id16 + 1;

             XCMD_SET_NUMBER(XCID_GLOBAL_CONFIG_PORT_ID, port_id);

             return XCMD_ACTION_SET_AND_STOP;
        } else {
        
             return XCMD_ACTION_SKIP_AND_STOP;
        }
#else

        return XCMD_ACTION_SKIP_AND_STOP;

#endif /* CFG_SWITCH_MIRROR_INCLUDED */
      }

    /*
     * COMMAND: config mirror source_port_list <PORT_LIST:line> 
     */
    case XCPATH_GLOBAL_H3_CONFIG_MIRROR_SOURCE_PORT_LIST: {

#ifdef CFG_SWITCH_MIRROR_INCLUDED
        int i;
        uplist_t port_list; /* port_list */
        uint8 enable = FALSE;

        /* ... Retrieve values from API ... */

        sal_memset(&port_list, 0, sizeof(port_list));
        
        if (xcbuilder_is_mirror_enable()) { 
             sal_memset(&port_list, 0, sizeof(port_list));
             SAL_UPORT_ITER(i) {
                  enable = FALSE;
                
                  board_mirror_port_get(i, &enable);
                  if (enable == TRUE) {
                      uplist_port_add(port_list.bytes, i);
                  }
             }

             XCMD_SET_VALUE(XCID_GLOBAL_CONFIG_PORT_LIST, port_list);

             return XCMD_ACTION_SET_AND_STOP;
             
        } else {
             return XCMD_ACTION_SKIP_AND_STOP;
        }
#else
            
        return XCMD_ACTION_SKIP_AND_STOP;
            
#endif /* CFG_SWITCH_MIRROR_INCLUDED */
      }

    default:
        return XCMD_ERR_INTERNAL_INVALID_PATH;
    }

    return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
}


/* 
 * TOP-LEVEL-COMMAND: show
 */
XCMD_ACTION
xcbuilder_global_show(int path, unsigned int index, XCMD_HANDLE xch)
{
    sal_printf("%s\n", __FUNCTION__);
    switch(path) {

    default:
        return XCMD_ERR_INTERNAL_INVALID_PATH;
    }

    return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
}


/* 
 * TOP-LEVEL-COMMAND: exit
 */
XCMD_ACTION
xcbuilder_global_exit(int path, unsigned int index, XCMD_HANDLE xch)
{

    switch(path) {

    default:
        return XCMD_ERR_INTERNAL_INVALID_PATH;
    }

    return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
}


#endif /* CFG_XCOMMAND_INCLUDED */
