/*
 * $Id: xccxt_global_enums.h,v 1.2 Broadcom SDK $
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

#ifndef _XCCXT_GLOBAL_ENUMS_H_
#define _XCCXT_GLOBAL_ENUMS_H_

#include "system.h"

#ifdef CFG_XCOMMAND_INCLUDED

/*
 * All possible command paths
 */
enum {

    /* show system  */
    XCPATH_GLOBAL_H2_SHOW_SYSTEM,

    /* enable block_unknown_multicast  */
    XCPATH_GLOBAL_H2_ENABLE_BLOCK_UNKNOWN_MULTICAST,

    /* disable igmpsnoop  */
    XCPATH_GLOBAL_H2_DISABLE_IGMPSNOOP,

    /* config igmpsnoop version <VER_ID:integer>  */
    XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_VERSION,

    /* disable mirror  */
    XCPATH_GLOBAL_H2_DISABLE_MIRROR,

    /* config mirror destination_port <PORT_ID:integer>  */
    XCPATH_GLOBAL_H3_CONFIG_MIRROR_DESTINATION_PORT,

    /* enable mirror  */
    XCPATH_GLOBAL_H2_ENABLE_MIRROR,

    /* config mirror source_port_list <PORT_LIST>  */
    XCPATH_GLOBAL_H3_CONFIG_MIRROR_SOURCE_PORT_LIST,

    /* enable igmpsnoop vlan <VID:integer>  */
    XCPATH_GLOBAL_H2_ENABLE_IGMPSNOOP,

    /* exit  */
    XCPATH_GLOBAL_H1_EXIT,

    /* config port <PORT_ID:integer> <PORT_NAME:line>  */
    XCPATH_GLOBAL_H2_CONFIG_PORT,

    /* disable block_unknown_multicast  */
    XCPATH_GLOBAL_H2_DISABLE_BLOCK_UNKNOWN_MULTICAST,

    /* config igmpsnoop all  */
    XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_ALL,

    /* config igmpsnoop query_interval <SECS:integer>  */
    XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_QUERY_INTERVAL,

    /* show counter  */
    XCPATH_GLOBAL_H2_SHOW_COUNTER,

    /* config igmpsnoop robustness_variable <TIMES:integer>  */
    XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_ROBUSTNESS_VARIABLE,

};


/*
 * Variant parameter IDs for command "enable"
 */
enum {
    XCID_GLOBAL_ENABLE_VID,
};


/*
 * Variant parameter IDs for command "config"
 */
enum {
    XCID_GLOBAL_CONFIG_PORT_ID,
    XCID_GLOBAL_CONFIG_PORT_NAME,
    XCID_GLOBAL_CONFIG_VER_ID,
    XCID_GLOBAL_CONFIG_SECS,
    XCID_GLOBAL_CONFIG_TIMES,
    XCID_GLOBAL_CONFIG_PORT_LIST,
};


/*
 * All callbacks function that should be implemented
 */
extern XCMD_ERROR xchandler_global_exit(int, XCMD_HANDLE);
extern XCMD_ERROR xchandler_global_disable(int, XCMD_HANDLE);
extern XCMD_ERROR xchandler_global_enable(int, XCMD_HANDLE);
extern XCMD_ERROR xchandler_global_show(int, XCMD_HANDLE);
extern XCMD_ERROR xchandler_global_config(int, XCMD_HANDLE);
extern XCMD_ACTION xcbuilder_global_disable(int, unsigned int, XCMD_HANDLE);
extern XCMD_ACTION xcbuilder_global_enable(int, unsigned int, XCMD_HANDLE);
extern XCMD_ACTION xcbuilder_global_config(int, unsigned int, XCMD_HANDLE);

#endif /* CFG_XCOMMAND_INCLUDED */

#endif /* _XCCXT_GLOBAL_ENUMS_H_ */
