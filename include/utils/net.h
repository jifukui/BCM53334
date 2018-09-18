/*
 * $Id: net.h,v 1.27 Broadcom SDK $
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

#ifndef _UTILS_NET_H_
#define _UTILS_NET_H_

#include "uip.h"

#if ((CFG_BIG_ENDIAN + CFG_LITTLE_ENDIAN) != 1)
#error "system endian not set"
#endif

/* Byte swap a 16 bit value */
#define BCMSWAP16(val) \
    ((uint16)( \
        (((uint16)(val) & (uint16)0x00ffU) << 8) | \
        (((uint16)(val) & (uint16)0xff00U) >> 8) ))

/* Byte swap a 32 bit value */
#define BCMSWAP32(val) \
    ((uint32)( \
        (((uint32)(val) & (uint32)0x000000ffUL) << 24) | \
        (((uint32)(val) & (uint32)0x0000ff00UL) <<  8) | \
        (((uint32)(val) & (uint32)0x00ff0000UL) >>  8) | \
        (((uint32)(val) & (uint32)0xff000000UL) >> 24) ))

#ifndef hton16
#if CFG_BIG_ENDIAN
#define HTON16(i) (i)
#define hton16(i) (i)
#define HTONL(i)  (i)
#define hton32(i) (i)
#define ntoh16(i) (i)
#define ntoh32(i) (i)
#define ltoh16(i) BCMSWAP16(i)
#define ltoh32(i) BCMSWAP32(i)
#define htol16(i) BCMSWAP16(i)
#define htol32(i) BCMSWAP32(i)
#else
#define HTON16(i) BCMSWAP16(i)
#define hton16(i) BCMSWAP16(i)
#define HTONL(i)  BCMSWAP32(i)
#define hton32(i) BCMSWAP32(i)
#define ntoh16(i) BCMSWAP16(i)
#define ntoh32(i) BCMSWAP32(i)
#define ltoh16(i) (i)
#define ltoh32(i) (i)
#define htol16(i) (i)
#define htol32(i) (i)
#endif
#endif

/*
 * Username / password
 */
#define MAX_USERNAME_LEN    (20)
#define MAX_PASSWORD_LEN    (20)
#define DEFAULT_PASSWORD    "password"
#define MAX_SSS_USER_COUNT  (1)

/*
 * Default IP, mask and gateway
 */
extern CODE const uint8 default_ip_addr[4];
extern CODE const uint8 default_netmask[4];
extern CODE const uint8 default_gateway[4];

#define DEFAULT_IP_ADDR     { 192, 168, 0, 239 }
#define DEFAULT_NETMASK     { 255, 255, 255, 0 }
#define DEFAULT_GATEWAY     { 192, 168, 0, 254 }

/*
 * Interface configurations
 */

typedef enum {
    ETH_STATE_DHCP_ENABLED = 0,
    ETH_STATE_DHCP_IN_PROGRESS,
    ETH_STATE_DHCP_BOUND,
    ETH_STATE_DHCP_FAILED
} ETH_IF_STATE;

typedef enum {
    INET_CONFIG_OFF = 0,
    INET_CONFIG_STATIC,
    INET_CONFIG_DHCP,
    INET_CONFIG_DHCP_FALLBACK     /* Fallback to static IP if DHCP fails */
} INET_CONFIG;

#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
/* Auto Link-Local IP configuration */
#define MAX_SERIAL_AUTOIP_LEN           (5) /* Valid + IP */
#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */

#ifdef CFG_ZEROCONF_MDNS_INCLUDED
#define MAX_SERIAL_BONJOUR_LEN          (1) /* Enable */
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

#define MAX_ACCESSCONTROL_IP            (5)

extern sys_error_t set_network_interface_config(
                        INET_CONFIG config,             /* Configuration */
                        const uint8 *ip,                 /* IP address */
                        const uint8 *netmask,            /* network submask */
                        const uint8 *gateway             /* gateway */
                        );
typedef struct _eth_if_config {
//    const char *if_name;
//    volatile ETH_IF_STATE state;
    char ip_addr[16];
    char ip_netmask[16];
    char ip_gateway[16];
//    unsigned char dhcpstate;
//    struct dhcp_lease lease;
//    struct bootp bootp_data;
//    sal_mutex_t mutex;
//    struct _eth_if_config *peer;
//    void *callback;
//    void *cbkarg;
} ETH_IF_CONFIG;

extern INET_CONFIG get_network_interface_config(uint8 *ip,
                                                uint8 *netmask,
                                                uint8 *gateway);

/* Parse ip address nvram format to array */
extern sys_error_t parse_ip(const char *str, uint8 *ipaddr);

/* Set/Get login password */
extern sys_error_t set_login_password(const char *password);

extern sys_error_t get_login_password(char *buf, uint8 len);

#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
/* Set/Get Auto IP configuration address */
extern sys_error_t get_autoip_addr(BOOL *valid, uint8 *autoip);
extern sys_error_t set_autoip_addr(BOOL valid, uint8 *autoip);
#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */
extern sys_error_t set_accessip_addr(BOOL valid, int ip_idx, uint8 accip[MAX_ACCESSCONTROL_IP][4], uint8 maskip[MAX_ACCESSCONTROL_IP][4]);
extern sys_error_t get_accessip_addr(BOOL *valid, int *ip_idx, uint8 accip[MAX_ACCESSCONTROL_IP][4], uint8 maskip[MAX_ACCESSCONTROL_IP][4]);
extern sys_error_t get_adminpv(BOOL *valid);
extern sys_error_t set_adminpv(BOOL valid);

/* Used by DHCPC task to configure system IP */
extern void
net_dhcp_configured(const uint8 *ip,
                    const uint8 *netmask,
                    const uint8 *gateway);

extern void net_utils_init(void);
#pragma pack(1)
typedef struct enet_hdr_s {
    uint8       en_dhost[6];    /* Destination */
    uint8       en_shost[6];    /* Source */
    union {
    uint16  _tpid;          /* Tag Prot ID */
    uint16  _len;           /* Length */
    }           en_f3;      /* Field 3 */
#   define  en_untagged_len en_f3._len
#   define  en_tag_tpid en_f3._tpid

    /* Untagged Format ends here */

    uint16      en_tag_ctrl;    /* Tag control */
    uint16      en_tag_len; /* Tagged length */
} enet_hdr_t;
#pragma pack()

#define ENET_TAGGED(e) (HTON16((e)->en_untagged_len) == (0x8100))

#if CFG_UIP_IPV6_ENABLED

typedef enum {
    IP6ADDR_TYPE_LINK_LOCAL,        /* Link-Local address */
    IP6ADDR_TYPE_AUTO_IP,           /* Auto IP from Router Advertisement */
    IP6ADDR_TYPE_MANUAL             /* Manually set */
} ip6addr_type;

extern uint8 get_ipv6_address_count(void);
extern sys_error_t get_ipv6_address(uint8 index,
                                    uip_ip6addr_t *addr, ip6addr_type *type);
extern sys_error_t set_manual_ipv6_address(uip_ip6addr_t *addr);

#endif /* CFG_UIP_IPV6_ENABLED */

#ifdef CFG_NET_LINKCHANGE_NOTIFY_INCLUDED
/*
 * Nofitication for network interface link change
 */
typedef void (*NET_LINKCHANGE_FUNC)(BOOL link);
extern void net_enable_linkchange(BOOL enable);
extern BOOL net_register_linkchange(NET_LINKCHANGE_FUNC func);
extern void net_unregister_linkchange(NET_LINKCHANGE_FUNC func);

#endif /* CFG_NET_LINKCHANGE_NOTIFY_INCLUDED */

#endif /* _UTILS_NET_H_ */
