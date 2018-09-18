/*
 * $Id: serializers.c,v 1.24 Broadcom SDK $
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
#include "appl/persistence.h"

#if CFG_PERSISTENCE_SUPPORT_ENABLED

/* Forwards */
extern void serializers_init(void);

#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
/*
 * Convenient macro for string serializer
 */
#define DEF_STRING_SERL(name,group,func) do { \
    extern int32 func(SERIALIZE_OP op, SERL_MEDIUM_STR *medium) REENTRANT; \
    register_string_serializer(name, group, func, FALSE); \
} while(0)
#define DEF_STRING_SERL_WITH_DEFAULTS(name,group,func) do { \
    extern int32 func(SERIALIZE_OP op, SERL_MEDIUM_STR *medium) REENTRANT; \
    register_string_serializer(name, group, func, TRUE); \
} while(0)
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */

/*
 * Convenient macro for binary serializer
 */
#define DEF_BINARY_SERL(name,group,func) do { \
    extern int32 func(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT; \
    register_binary_serializer(name, group, func, FALSE); \
} while(0)
#define DEF_BINARY_SERL_WITH_DEFAULTS(name,group,func) do { \
    extern int32 func(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT; \
    register_binary_serializer(name, group, func, TRUE); \
} while(0)

/*
 * REGISTER YOUR SERIALIZERS HERE!
 */
void
serializers_init(void)
{
    /*
     * Switch features 
     */

#ifdef CFG_SWITCH_QOS_INCLUDED
    DEF_BINARY_SERL_WITH_DEFAULTS("qos", SERL_GRP_SWITCH, qos_serializer);
#endif /* CFG_SWITCH_QOS_INCLUDED */

#ifdef CFG_SWITCH_RATE_INCLUDED
    DEF_BINARY_SERL_WITH_DEFAULTS("rate", SERL_GRP_SWITCH, rate_serializer);
    DEF_BINARY_SERL_WITH_DEFAULTS("storm", SERL_GRP_SWITCH, storm_serializer);
#endif /* CFG_SWITCH_RATE_INCLUDED */

#ifdef CFG_SWITCH_MIRROR_INCLUDED
    DEF_BINARY_SERL_WITH_DEFAULTS("mirror", SERL_GRP_SWITCH, mirror_serializer);
#endif /* CFG_SWITCH_MIRROR_INCLUDED */

#ifdef CFG_SWITCH_VLAN_INCLUDED
    DEF_BINARY_SERL_WITH_DEFAULTS("vlan", SERL_GRP_SWITCH, vlan_serializer);
    DEF_BINARY_SERL_WITH_DEFAULTS("pvid", SERL_GRP_SWITCH, pvid_serializer);
#endif /* CFG_SWITCH_VLAN_INCLUDED */

#ifdef CFG_SWITCH_MCAST_INCLUDED
    DEF_BINARY_SERL_WITH_DEFAULTS("mcast", SERL_GRP_SWITCH, mcast_serializer);
/*
  * IGMP snoop features 
  */
    DEF_BINARY_SERL_WITH_DEFAULTS("igmpsnoop", SERL_GRP_SWITCH, igmpsnoop_serializer);

#endif /* CFG_SWITCH_MCAST_INCLUDED */

#ifdef CFG_SWITCH_LAG_INCLUDED
    DEF_BINARY_SERL_WITH_DEFAULTS("lag", SERL_GRP_SWITCH, lag_serializer);
#endif /* CFG_SWITCH_LAG_INCLUDED */

#if CFG_UIP_STACK_ENABLED
    /* 
     * Networking 
     */
    DEF_BINARY_SERL_WITH_DEFAULTS("ethconfig", SERL_GRP_NETWORK_IP, eth_serializer);
#endif /* CFG_UIP_STACK_ENABLED */

    /*
     * Other system configuration
     */
#ifdef CFG_SYSTEM_PASSWORD_INCLUDED
    DEF_BINARY_SERL_WITH_DEFAULTS("password", SERL_GRP_NETWORK, password_serializer);
#endif /* CFG_SYSTEM_PASSWORD_INCLUDED */
    DEF_BINARY_SERL_WITH_DEFAULTS("name", SERL_GRP_NETWORK, system_name_serializer);
#ifdef CFG_PRODUCT_REGISTRATION_INCLUDED    
    DEF_BINARY_SERL_WITH_DEFAULTS("registration", SERL_GRP_NETWORK, 
        registration_status_serializer);
#endif /* CFG_PRODUCT_REGISTRATION_INCLUDED */

#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
    DEF_BINARY_SERL_WITH_DEFAULTS("autoip", SERL_GRP_NETWORK, 
        autoip_serializer);
#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */

#ifdef CFG_ZEROCONF_MDNS_INCLUDED
    DEF_BINARY_SERL_WITH_DEFAULTS("mdns", SERL_GRP_PROTOCOL, 
        mdns_serializer);
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

#if CFG_UIP_IPV6_ENABLED 
    DEF_BINARY_SERL_WITH_DEFAULTS("ipv6", SERL_GRP_NETWORK, 
        ipv6_serializer);
#endif /* CFG_UIP_IPV6_ENABLED */

#ifdef CFG_SWITCH_LOOPDETECT_INCLUDED
    DEF_BINARY_SERL_WITH_DEFAULTS("loopdetect", SERL_GRP_SWITCH, 
        loopdetect_serializer);
#endif
    DEF_BINARY_SERL_WITH_DEFAULTS("portdesc", SERL_GRP_NETWORK, port_desc_serializer);
    DEF_BINARY_SERL_WITH_DEFAULTS("portenable", SERL_GRP_NETWORK, port_enable_serializer);
#if CFG_UIP_STACK_ENABLED
    DEF_BINARY_SERL_WITH_DEFAULTS("accesscontrol", SERL_GRP_NETWORK, access_control_serializer);
    DEF_BINARY_SERL_WITH_DEFAULTS("adminpriv", SERL_GRP_NETWORK, admin_privilege_serializer);
#endif /* CFG_UIP_STACK_ENABLED */
}
#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */

