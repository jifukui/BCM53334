/*
 * $Id: xccxt_table_global.c,v 1.2 Broadcom SDK $
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

#include "appl/xcmd/xcmd_internal.h"
#include "xccxt_global_enums.h"

extern XCMD_TYPE_CONVERTER xccvt_port_list_converters;
static const XCNODE_PARAM_EOL _node0000 = {
    XCPATH_GLOBAL_H2_ENABLE_MIRROR,
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { xchandler_global_enable, xcbuilder_global_enable },
    NULL,
};

static const XCNODE_CMD_KEYWORD _node0001 = {
    "mirror",
    "Enable mirror",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_param_eol, (void *)&_node0000 }
};

static const XCNODE_PARAM_EOL _node0002 = {
    XCPATH_GLOBAL_H2_ENABLE_IGMPSNOOP,
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { xchandler_global_enable, xcbuilder_global_enable },
    NULL,
};

static const XCNODE_PARAM_INTEGER _node0003 = {
    XCID_GLOBAL_ENABLE_VID,
    "<1..4094>",
    "Value of VLAN ID for igmpsnoop",
    1UL,
    4094UL,
    0,
    10,
    { &xcnode_handler_param_eol, (void *)&_node0002 }
};

static const XCNODE_PARAM_KEYWORD _node0004 = {
    "vlan",
    "VLAN ID for igmpsnoop",
    { &xcnode_handler_param_integer, (void *)&_node0003 }
};

static const XCNODE_CMD_KEYWORD _node0005 = {
    "igmpsnoop",
    "Enable igmpsnoop",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_param_keyword, (void *)&_node0004 }
};

static const XCNODE_PARAM_EOL _node0006 = {
    XCPATH_GLOBAL_H2_ENABLE_BLOCK_UNKNOWN_MULTICAST,
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { xchandler_global_enable, xcbuilder_global_enable },
    NULL,
};

static const XCNODE_CMD_KEYWORD _node0007 = {
    "block_unknown_multicast",
    "Enable block unknown multicast",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_param_eol, (void *)&_node0006 }
};

static const XCMD_NODE_PTR _node0008[] = {
    { &xcnode_handler_cmd_keyword, (void *)&_node0001 },
    { &xcnode_handler_cmd_keyword, (void *)&_node0005 },
    { &xcnode_handler_cmd_keyword, (void *)&_node0007 },
};

static const XCNODE_SELECT _node0009 = {
    (void *)_node0008,
    3
};

static const XCNODE_CMD_KEYWORD _node000A = {
    "enable",
    "Enable command",
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_select, (void *)&_node0009 }
};

static const XCNODE_PARAM_EOL _node000B = {
    XCPATH_GLOBAL_H2_DISABLE_MIRROR,
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { xchandler_global_disable, xcbuilder_global_disable },
    NULL,
};

static const XCNODE_CMD_KEYWORD _node000C = {
    "mirror",
    "Disable IGMPSNOOP of multicast",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_param_eol, (void *)&_node000B }
};

static const XCNODE_PARAM_EOL _node000D = {
    XCPATH_GLOBAL_H2_DISABLE_IGMPSNOOP,
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { xchandler_global_disable, xcbuilder_global_disable },
    NULL,
};

static const XCNODE_CMD_KEYWORD _node000E = {
    "igmpsnoop",
    "Disable IGMPSNOOP of multicast",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_param_eol, (void *)&_node000D }
};

static const XCNODE_PARAM_EOL _node000F = {
    XCPATH_GLOBAL_H2_DISABLE_BLOCK_UNKNOWN_MULTICAST,
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { xchandler_global_disable, xcbuilder_global_disable },
    NULL,
};

static const XCNODE_CMD_KEYWORD _node0010 = {
    "block_unknown_multicast",
    "Disable block unknown multicast",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_param_eol, (void *)&_node000F }
};

static const XCMD_NODE_PTR _node0011[] = {
    { &xcnode_handler_cmd_keyword, (void *)&_node000C },
    { &xcnode_handler_cmd_keyword, (void *)&_node000E },
    { &xcnode_handler_cmd_keyword, (void *)&_node0010 },
};

static const XCNODE_SELECT _node0012 = {
    (void *)_node0011,
    3
};

static const XCNODE_CMD_KEYWORD _node0013 = {
    "disable",
    "Disable command",
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_select, (void *)&_node0012 }
};

static const XCNODE_PARAM_EOL _node0014 = {
    XCPATH_GLOBAL_H2_CONFIG_PORT,
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { xchandler_global_config, xcbuilder_global_config },
    NULL,
};

static const XCNODE_PARAM_LINE _node0015 = {
    XCID_GLOBAL_CONFIG_PORT_NAME,
    "port_name",
    "Port Name",
    0,
    20,
    0,
    { &xcnode_handler_param_eol, (void *)&_node0014 }
};

static const XCNODE_PARAM_INTEGER _node0016 = {
    XCID_GLOBAL_CONFIG_PORT_ID,
    "port_id",
    "Specify port number. The index starts from 1",
    1UL,
    24UL,
    0,
    10,
    { &xcnode_handler_param_line, (void *)&_node0015 }
};

static const XCNODE_CMD_KEYWORD _node0017 = {
    "port",
    "configure port name",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_param_integer, (void *)&_node0016 }
};

static const XCNODE_PARAM_EOL _node0018 = {
    XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_VERSION,
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { xchandler_global_config, xcbuilder_global_config },
    NULL,
};

static const XCNODE_PARAM_INTEGER _node0019 = {
    XCID_GLOBAL_CONFIG_VER_ID,
    "<1..3>",
    "Configure IGMPSNOOP version",
    1UL,
    3UL,
    0,
    10,
    { &xcnode_handler_param_eol, (void *)&_node0018 }
};

static const XCNODE_CMD_KEYWORD _node001A = {
    "version",
    "Config IGMPSNOOP version",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_param_integer, (void *)&_node0019 }
};

static const XCNODE_PARAM_EOL _node001B = {
    XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_QUERY_INTERVAL,
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { xchandler_global_config, xcbuilder_global_config },
    NULL,
};

static const XCNODE_PARAM_INTEGER _node001C = {
    XCID_GLOBAL_CONFIG_SECS,
    "<1..125>",
    "Configure IGMPSNOOP version",
    1UL,
    125UL,
    0,
    10,
    { &xcnode_handler_param_eol, (void *)&_node001B }
};

static const XCNODE_CMD_KEYWORD _node001D = {
    "query_interval",
    "Config IGMPSNOOP query_interval",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_param_integer, (void *)&_node001C }
};

static const XCNODE_PARAM_EOL _node001E = {
    XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_ROBUSTNESS_VARIABLE,
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { xchandler_global_config, xcbuilder_global_config },
    NULL,
};

static const XCNODE_PARAM_INTEGER _node001F = {
    XCID_GLOBAL_CONFIG_TIMES,
    "<2..10>",
    "Configure IGMPSNOOP robustness variable",
    2UL,
    10UL,
    0,
    10,
    { &xcnode_handler_param_eol, (void *)&_node001E }
};

static const XCNODE_CMD_KEYWORD _node0020 = {
    "robustness_variable",
    "Config IGMPSNOOP robustness variable",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_param_integer, (void *)&_node001F }
};

static const XCNODE_PARAM_EOL _node0021 = {
    XCPATH_GLOBAL_H3_CONFIG_IGMPSNOOP_ALL,
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { xchandler_global_config, xcbuilder_global_config },
    NULL,
};

static const XCNODE_CMD_KEYWORD _node0022 = {
    "all",
    "",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_param_eol, (void *)&_node0021 }
};

static const XCMD_NODE_PTR _node0023[] = {
    { &xcnode_handler_cmd_keyword, (void *)&_node001A },
    { &xcnode_handler_cmd_keyword, (void *)&_node001D },
    { &xcnode_handler_cmd_keyword, (void *)&_node0020 },
    { &xcnode_handler_cmd_keyword, (void *)&_node0022 },
};

static const XCNODE_OPTIONAL _node0024 = {
    (void *)_node0023,
    4
};

static const XCNODE_CMD_KEYWORD _node0025 = {
    "igmpsnoop",
    "Configure igmpsnoop",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_optional, (void *)&_node0024 }
};

static const XCNODE_PARAM_EOL _node0026 = {
    XCPATH_GLOBAL_H3_CONFIG_MIRROR_DESTINATION_PORT,
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { xchandler_global_config, xcbuilder_global_config },
    NULL,
};

static const XCNODE_PARAM_INTEGER _node0027 = {
    XCID_GLOBAL_CONFIG_PORT_ID,
    "port_id",
    "Specify port number. The index starts from 1",
    1UL,
    24UL,
    0,
    10,
    { &xcnode_handler_param_eol, (void *)&_node0026 }
};

static const XCNODE_CMD_KEYWORD _node0028 = {
    "destination_port",
    "configure mirror destination port",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_param_integer, (void *)&_node0027 }
};

static const XCNODE_PARAM_EOL _node0029 = {
    XCPATH_GLOBAL_H3_CONFIG_MIRROR_SOURCE_PORT_LIST,
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { xchandler_global_config, xcbuilder_global_config },
    NULL,
};

static const XCNODE_PARAM_CUSTOM _node002A = {
    XCID_GLOBAL_CONFIG_PORT_LIST,
    "WORD",
    "Port list. For example, 5-8,10,12-13",
    &xccvt_port_list_converters,
    0,
    { &xcnode_handler_param_eol, (void *)&_node0029 }
};

static const XCNODE_CMD_KEYWORD _node002B = {
    "source_port_list",
    "configure source port list",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_param_custom, (void *)&_node002A }
};

static const XCMD_NODE_PTR _node002C[] = {
    { &xcnode_handler_cmd_keyword, (void *)&_node0028 },
    { &xcnode_handler_cmd_keyword, (void *)&_node002B },
};

static const XCNODE_SELECT _node002D = {
    (void *)_node002C,
    2
};

static const XCNODE_CMD_KEYWORD _node002E = {
    "mirror",
    "configure mirror",
    XCFLAG_ACCESS_GUEST|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_select, (void *)&_node002D }
};

static const XCMD_NODE_PTR _node002F[] = {
    { &xcnode_handler_cmd_keyword, (void *)&_node0017 },
    { &xcnode_handler_cmd_keyword, (void *)&_node0025 },
    { &xcnode_handler_cmd_keyword, (void *)&_node002E },
};

static const XCNODE_SELECT _node0030 = {
    (void *)_node002F,
    3
};

static const XCNODE_CMD_KEYWORD _node0031 = {
    "config",
    "Config command",
    XCFLAG_ACCESS_PRIVILEGED|XCFLAG_CMDTYPE_CONFIG,
    { &xcnode_handler_select, (void *)&_node0030 }
};

static const XCNODE_PARAM_EOL _node0032 = {
    XCPATH_GLOBAL_H2_SHOW_COUNTER,
    XCFLAG_CMDTYPE_SHOW|XCFLAG_ACCESS_GUEST,
    { xchandler_global_show, NULL },
    NULL,
};

static const XCNODE_CMD_KEYWORD _node0033 = {
    "counter",
    "Show counter",
    XCFLAG_CMDTYPE_SHOW|XCFLAG_ACCESS_GUEST,
    { &xcnode_handler_param_eol, (void *)&_node0032 }
};

static const XCNODE_PARAM_EOL _node0034 = {
    XCPATH_GLOBAL_H2_SHOW_SYSTEM,
    XCFLAG_CMDTYPE_SHOW|XCFLAG_ACCESS_GUEST,
    { xchandler_global_show, NULL },
    NULL,
};

static const XCNODE_CMD_KEYWORD _node0035 = {
    "system",
    "Show system information",
    XCFLAG_CMDTYPE_SHOW|XCFLAG_ACCESS_GUEST,
    { &xcnode_handler_param_eol, (void *)&_node0034 }
};

static const XCMD_NODE_PTR _node0036[] = {
    { &xcnode_handler_cmd_keyword, (void *)&_node0033 },
    { &xcnode_handler_cmd_keyword, (void *)&_node0035 },
};

static const XCNODE_SELECT _node0037 = {
    (void *)_node0036,
    2
};

static const XCNODE_CMD_KEYWORD _node0038 = {
    "show",
    "Show command",
    XCFLAG_CMDTYPE_SHOW|XCFLAG_ACCESS_GUEST,
    { &xcnode_handler_select, (void *)&_node0037 }
};

static const XCNODE_PARAM_EOL _node0039 = {
    XCPATH_GLOBAL_H1_EXIT,
    XCFLAG_CMDTYPE_SHOW|XCFLAG_ACCESS_GUEST,
    { xchandler_global_exit, NULL },
    NULL,
};

static const XCNODE_CMD_KEYWORD _node003A = {
    "exit",
    "Exit current shell",
    XCFLAG_CMDTYPE_SHOW|XCFLAG_ACCESS_GUEST,
    { &xcnode_handler_param_eol, (void *)&_node0039 }
};

static const XCMD_NODE_PTR _node003B[] = {
    { &xcnode_handler_cmd_keyword, (void *)&_node000A },
    { &xcnode_handler_cmd_keyword, (void *)&_node0013 },
    { &xcnode_handler_cmd_keyword, (void *)&_node0031 },
    { &xcnode_handler_cmd_keyword, (void *)&_node0038 },
    { &xcnode_handler_cmd_keyword, (void *)&_node003A },
};

static const XCNODE_SELECT _node003C = {
    (void *)_node003B,
    5
};

const XCNODE_CONTEXT xcmd_context_global = {
    "global",
    { &xcnode_handler_select, (void *)&_node003C }
};

#endif /* CFG_XCOMMAND_INCLUDED */
