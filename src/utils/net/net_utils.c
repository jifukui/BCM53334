/*
 * $Id: net_utils.c,v 1.46 Broadcom SDK $
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
#include "uip.h"
#include "utils/net.h"
#include "appl/dhcpc.h"

#if CFG_UIP_STACK_ENABLED
#ifdef CFG_ZEROCONF_MDNS_INCLUDED
#include "appl/mdns.h"
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

#if CFG_UIP_IPV6_ENABLED
#define uip_ipaddr_t uip_ip6addr_t
#include "uip-ds6.h"
#undef uip_ipaddr_t
#endif /* CFG_UIP_IPV6_ENABLED */

CODE const uint8 default_ip_addr[4] = DEFAULT_IP_ADDR;
CODE const uint8 default_netmask[4] = DEFAULT_NETMASK;
CODE const uint8 default_gateway[4] = DEFAULT_GATEWAY;

/* Default mode: fall back to default IP if DHCP failed */
static INET_CONFIG inet_config = INET_CONFIG_DHCP_FALLBACK;

static uint8 inet_ipaddr[4];
static uint8 inet_netmask[4];
static uint8 inet_gateway[4];
#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
static BOOL  autoip_valid;
static uint8 autoip_addr[4];
#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */
static void net_ip_configured(const uint8 *ip, const uint8 *netmask, const uint8 *gateway);
static CODE const uint8 zero_ip_addr[] = { 0, 0, 0, 0 };
static BOOL  accip_valid = FALSE;
static int valid_accip_num = 0;
static uint8 accip_addr[MAX_ACCESSCONTROL_IP][4], maskip_addr[MAX_ACCESSCONTROL_IP][4];
static BOOL  adminpv_valid;

#ifdef CFG_SYSTEM_PASSWORD_INCLUDED
/*
 * Username / password
 */
static char login_password[MAX_PASSWORD_LEN + 1] = DEFAULT_PASSWORD;
#endif /* CFG_SYSTEM_PASSWORD_INCLUDED */

#ifndef __BOOTLOADER__
extern BOOL logined_entries_valid[1];
extern uint32 logined_IPs[1];
extern unsigned int logined_accessed[1];
#endif
#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

#ifdef CFG_SYSTEM_PASSWORD_INCLUDED
sys_error_t
set_login_password(const char *password)
{
    if (password == NULL) {
        return SYS_ERR_PARAMETER;
    }

    if (sal_strlen(password) > MAX_PASSWORD_LEN) {
        return SYS_ERR_PARAMETER;
    }

    sal_strcpy(login_password, password);

    return SYS_OK;
}

sys_error_t
get_login_password(char *buf, uint8 len)
{
    if (buf == NULL || len == 0) {
        return SYS_ERR_PARAMETER;
    }

    if (sal_strlen(login_password) >= len) {
        return SYS_ERR_PARAMETER;
    }

    sal_strcpy(buf, login_password);

    return SYS_OK;
}
#endif /* CFG_SYSTEM_PASSWORD_INCLUDED */

sys_error_t
parse_ip(const char *str, uint8 *ipaddr)
{
    uint8 idx = 0, i;
    uint32 cur = 0;
    char ch, last_ch = '.';
    uint8 ip[4];

    if (str == NULL) {
        return SYS_ERR_PARAMETER;
    }

    for(i = 0;; i++) {
        ch = str[i];
        if (ch == 0 || ch == '.' || ch == '/') {
            if (last_ch == '.') /* for ".3.2.1" or "3..2" or "1.2.3." */
                return SYS_ERR_PARAMETER;

            if (cur > 255)
                return SYS_ERR_PARAMETER;

            if (idx >= 4)
                return SYS_ERR_PARAMETER;

            ip[idx] = cur;
            idx++;
            cur = 0;

            if (ch == 0 || ch == '/')
                break;
        } else if (isdigit(ch)) {
            cur *= 10;
            cur += ch - '0';
            if (cur > 255) {
                return SYS_ERR_PARAMETER;
            }
        } else
            return SYS_ERR_PARAMETER;

        last_ch = ch;
    }

    if (idx != 4)
        return SYS_ERR_PARAMETER;

    if (ipaddr) {
        sal_memcpy(ipaddr, ip, 4);
    }
    return SYS_OK;
}

sys_error_t
set_network_interface_config(
            INET_CONFIG config,
            const uint8 *ip,
            const uint8 *netmask,
            const uint8 *gateway)
{
#ifndef CFG_DHCPC_INCLUDED
    UNREFERENCED_PARAMETER(config);
    net_ip_configured(ip, netmask, gateway);
    return SYS_OK;
#else /* CFG_DHCPC_INCLUDED */

    BOOL changed = FALSE;

    /* Check parameters */
    if (config < INET_CONFIG_OFF || config > INET_CONFIG_DHCP_FALLBACK) {
        return SYS_ERR_PARAMETER;
    }

    /* Check if config or parameters changed */
    if (inet_config != config) {
        changed = TRUE;
    } else if (config == INET_CONFIG_STATIC) {
        if (sal_memcmp(inet_ipaddr, ip, 4) ||
            sal_memcmp(inet_netmask, netmask, 4) ||
            sal_memcmp(inet_gateway, gateway, 4)) {
            changed = TRUE;
        }
    } else if (config == INET_CONFIG_DHCP_FALLBACK) {
        /* Re-aquire IP if it's DHCP again */
        changed = TRUE;
    }

    /* Something has changed, update and apply it */
    if (changed) {
        inet_config = config;

        if (config == INET_CONFIG_STATIC) {
            net_ip_configured(ip, netmask, gateway);
        } else if (config == INET_CONFIG_DHCP_FALLBACK) {
            /* Clear current IP info */
            net_ip_configured(zero_ip_addr, zero_ip_addr, zero_ip_addr);
#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
            sal_memcpy(autoip_addr, zero_ip_addr, 4);
#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */
            dhcpc_renew();
        }
    }

    return SYS_OK;
#endif /* CFG_DHCPC_INCLUDED */
}

INET_CONFIG
get_network_interface_config(uint8 *ip, uint8 *netmask, uint8 *gateway)
{
    if (ip) {
        sal_memcpy(ip, inet_ipaddr, 4);
    }

    if (netmask) {
        sal_memcpy(netmask, inet_netmask, 4);
    }

    if (gateway) {
        sal_memcpy(gateway, inet_gateway, 4);
    }

    return inet_config;
}

static void
net_ip_configured(const uint8 *ip, const uint8 *netmask, const uint8 *gateway)
{
#if CFG_CONSOLE_ENABLED
    char buf[16];
#endif /* CFG_CONSOLE_ENABLED */\

    uip_sethostaddr((uip_ipaddr_t *)ip);

    uip_setnetmask((uip_ipaddr_t *)netmask);

    uip_setdraddr((uip_ipaddr_t *)gateway);

    sal_memcpy(inet_ipaddr, ip, 4);
    sal_memcpy(inet_netmask, netmask, 4);
    sal_memcpy(inet_gateway, gateway, 4);

#if CFG_CONSOLE_ENABLED
    /* Don't show it if it's all zeroes */
    if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0) {
        return;
    }
    sal_sprintf(buf, "%u.%u.%u.%u", (int)uip_ipaddr1((uip_ipaddr_t *)ip),
                                    (int)uip_ipaddr2((uip_ipaddr_t *)ip),
                                    (int)uip_ipaddr3((uip_ipaddr_t *)ip),
                                    (int)uip_ipaddr4((uip_ipaddr_t *)ip));
    sal_printf("System IP : %s ", buf);
    sal_sprintf(buf, "%u.%u.%u.%u", (int)uip_ipaddr1((uip_ipaddr_t *)netmask),
                                    (int)uip_ipaddr2((uip_ipaddr_t *)netmask),
                                    (int)uip_ipaddr3((uip_ipaddr_t *)netmask),
                                    (int)uip_ipaddr4((uip_ipaddr_t *)netmask));
    sal_printf("netmask : %s ", buf);
    sal_sprintf(buf, "%u.%u.%u.%u", (int)uip_ipaddr1((uip_ipaddr_t *)gateway),
                                    (int)uip_ipaddr2((uip_ipaddr_t *)gateway),
                                    (int)uip_ipaddr3((uip_ipaddr_t *)gateway),
                                    (int)uip_ipaddr4((uip_ipaddr_t *)gateway));
    sal_printf("gateway : %s\n", buf);
#endif /* CFG_CONSOLE_ENABLED */

#ifdef CFG_ZEROCONF_MDNS_INCLUDED
    if (sal_memcmp(zero_ip_addr, ip, 4)) {
        /* If non-zero IP configured, start the MDNS process */
        mdns_start();
    }
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */
}

#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
sys_error_t
get_autoip_addr(BOOL *valid, uint8 *autoip)
{
    if (autoip == NULL) {
        return SYS_ERR_PARAMETER;
    }
    *valid = autoip_valid;
    sal_memcpy(autoip, autoip_addr, 4);
    return SYS_OK;
}

sys_error_t
set_autoip_addr(BOOL valid, uint8 *autoip)
{

    if (autoip == NULL) {
        return SYS_ERR_PARAMETER;
    }

    autoip_valid = valid;
    sal_memcpy(autoip_addr, autoip, 4);

    return SYS_OK;
}

#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */

sys_error_t
set_accessip_addr(BOOL valid, int ip_num, uint8 accip[MAX_ACCESSCONTROL_IP][4], uint8 maskip[MAX_ACCESSCONTROL_IP][4])
{
    int i, j;
    int idx = 0;

    if (ip_num > MAX_ACCESSCONTROL_IP) {
        return SYS_ERR_PARAMETER;
    } else if (ip_num == 0) {
        /* no valid entry, treat as valid=FALSE */
        valid = FALSE;
    }
    valid_accip_num = ip_num;

    accip_valid = valid;
    if (valid == FALSE) {
        return SYS_OK;
    }
    if ((accip != NULL) && (maskip != NULL)) {
        for (i = 0; i < MAX_ACCESSCONTROL_IP; i++ ) {   
            if (idx >= ip_num) {
                break;
            }
            idx++;
            for (j = 0; j < 4; j++) {
                accip_addr[i][j] = accip[i][j];
                maskip_addr[i][j] = maskip[i][j];
            }

        }
    }
    return SYS_OK;
}

sys_error_t
get_accessip_addr(BOOL *valid, int *ip_num, uint8 accip[MAX_ACCESSCONTROL_IP][4], uint8 maskip[MAX_ACCESSCONTROL_IP][4])
{
    int i, j;

    *valid = accip_valid;
    *ip_num = valid_accip_num;

    if (*valid == TRUE) {
        for (i = 0; i < MAX_ACCESSCONTROL_IP; i++ ) {       
            for (j = 0; j < 4; j++) {
                accip[i][j] = accip_addr[i][j];
                maskip[i][j] = maskip_addr[i][j];
            }
        }
    }

    return SYS_OK;
}


sys_error_t
get_adminpv(BOOL *valid)
{

    *valid = adminpv_valid;

    return SYS_OK;
}

sys_error_t
set_adminpv(BOOL valid)
{
//    uint32 ip;
    
    adminpv_valid = valid;

#if !defined(__BOOTLOADER__) && CFG_WEB 

    if (valid) {
//        logined_entries_valid[0] = TRUE;

//        ip = (BUF->srcipaddr.u8[0] & 0xFF) | ((BUF->srcipaddr.u8[1]) << 8) 
//            | ((BUF->srcipaddr.u8[2]) << 16) | ((BUF->srcipaddr.u8[3]) << 24);
//        logined_IPs[0] = ip;
//        logined_accessed[0] = sal_get_ticks();
    
    } else {
        logined_entries_valid[0] = FALSE;
        logined_IPs[0] = 0;
        logined_accessed[0] = 0;
    }
#endif
    return SYS_OK;
}

void
net_dhcp_configured(const uint8 *ip, const uint8 *netmask, const uint8 *gateway)
{
    if (inet_config == INET_CONFIG_STATIC) {
        /* Has changed to static IP */
        return;
    }
    net_ip_configured(ip, netmask, gateway);
    return;
}

#if CFG_UIP_IPV6_ENABLED

uint8
get_ipv6_address_count(void)
{
    uint8 i, count;
    for(i=0, count=0; i<UIP_DS6_ADDR_NB; i++) {
        if (uip_ds6_if.addr_list[i].isused) {
            count++;
        }
    }
    return count;
}

sys_error_t
get_ipv6_address(uint8 index, uip_ip6addr_t *addr, ip6addr_type *type)
{
    uint8 i, count;

    for(i=0, count=0; i<UIP_DS6_ADDR_NB; i++) {
        if (uip_ds6_if.addr_list[i].isused) {
            if (index == count) {
                break;
            }
            count++;
        }
    }

    if (i == UIP_DS6_ADDR_NB) {
        return SYS_ERR_NOT_FOUND;
    }

    if (addr != NULL) {
        sal_memcpy(
            addr, &uip_ds6_if.addr_list[i].ipaddr, sizeof(uip_ip6addr_t));
    }

    if (type != NULL) {
        switch(uip_ds6_if.addr_list[i].type) {

        case ADDR_AUTOCONF:
            if (uip_is_addr_link_local(&uip_ds6_if.addr_list[i].ipaddr)) {
                *type = IP6ADDR_TYPE_LINK_LOCAL;
            } else {
                *type = IP6ADDR_TYPE_AUTO_IP;
            }
            break;

        default:
            *type = IP6ADDR_TYPE_MANUAL;
            break;

        }
    }

    return SYS_OK;
}

sys_error_t
set_manual_ipv6_address(uip_ip6addr_t *addr)
{
    uint8 i;
    uip_ds6_addr_t *orgaddr;

    if (addr == NULL) {
        return SYS_ERR_PARAMETER;
    }

    /* Found original manual IP if any */
    for(i=0; i<UIP_DS6_ADDR_NB; i++) {
        if (uip_ds6_if.addr_list[i].isused) {
            if (uip_ds6_if.addr_list[i].type == ADDR_MANUAL) {
                orgaddr = &uip_ds6_if.addr_list[i];
                break;
            }
        }
    }
    if (i < UIP_DS6_ADDR_NB) {

        /* If it's the same, just leave as is */
        if (!sal_memcmp(addr, &orgaddr->ipaddr, sizeof(uip_ip6addr_t))) {
            return SYS_OK;
        }

        /* Otherwise, delete the original ipaddr */
        uip_ds6_addr_rm(orgaddr);
    }

    /* If it's an unspecified address (::), no manual address will be set */
    if (uip_is_addr_unspecified(addr)) {
        return SYS_OK;
    }

    /*
     * The address won't be available right away since it has to perform DAD
     * (Duplicated Address Detection) before it can be used.
     * In case of DAD failure (address conflict), this address won't be
     * usable.
     */
    uip_ds6_addr_add(addr, 0, ADDR_MANUAL);

    return SYS_OK;
}

#endif /* CFG_UIP_IPV6_ENABLED */

#ifdef CFG_NET_LINKCHANGE_NOTIFY_INCLUDED

#if !CFG_LINKCHANGE_CALLBACK_SUPPORT
#error CFG_LINKCHANGE_CALLBACK_SUPPORT is required!
#endif /* !CFG_LINKCHANGE_CALLBACK_SUPPORT */

STATIC BOOL net_enable_link_funcs;
STATIC NET_LINKCHANGE_FUNC net_link_funcs[CFG_NET_MAX_LINKCHANGE_HANDLER];
STATIC BOOL net_intf_link;

void
net_enable_linkchange(BOOL enable)
{
    net_enable_link_funcs = enable;
}

BOOL
net_register_linkchange(NET_LINKCHANGE_FUNC func)
{
    uint8 i;

    SAL_ASSERT(func != NULL);

    for(i=0; i<CFG_NET_MAX_LINKCHANGE_HANDLER; i++) {
        if (net_link_funcs[i] == NULL) {
            net_link_funcs[i] = func;
            return TRUE;
        }
    }

    return FALSE;
}

void
net_unregister_linkchange(NET_LINKCHANGE_FUNC func)
{
    uint8 i;

    SAL_ASSERT(func != NULL);

    for(i=0; i<CFG_NET_MAX_LINKCHANGE_HANDLER; i++) {
        if (net_link_funcs[i] == func) {
            net_link_funcs[i] = NULL;
            return;
        }
    }
}

STATICCBK void
net_port_linkchange_handler(uint16 uport, BOOL link, void *arg) REENTRANT
{
    uint8 i;

    UNREFERENCED_PARAMETER(arg);

    /* Check if any front port's link is up */
    SAL_UPORT_ITER(uport) {
        if (board_get_port_link_status(uport, &link) != SYS_OK) {
            SAL_ASSERT(FALSE);
            continue;
        }
        if (link == TRUE) {
            break;
        }
    }

    /* Check for interface link change */
    if ((SAL_UPORT_IS_NOT_VALID(uport) && net_intf_link) ||
        (!SAL_UPORT_IS_NOT_VALID(uport) && !net_intf_link)) {
        net_intf_link = (SAL_UPORT_IS_NOT_VALID(uport)) ? FALSE : TRUE;
       
        /*
                * Some application, for example  Cable Diag, would cause
                * interface down->up and get a different system IP address.
                * For these applications, the interface change handling is not
                * necessary.
                * So we add net_enable_link_funcs to disable/enable it.
                */
        if (net_enable_link_funcs == FALSE) {
            return;
        }

        for(i=0; i<CFG_NET_MAX_LINKCHANGE_HANDLER; i++) {
            if (net_link_funcs[i] != NULL) {
                (*net_link_funcs[i])(net_intf_link);
            }
        }
    }
}

#ifdef CFG_DHCPC_INCLUDED
STATICCBK void
net_utils_linkchange(BOOL link)
{
    if (link) {
        if (inet_config == INET_CONFIG_DHCP_FALLBACK ||
            inet_config == INET_CONFIG_DHCP) {

            net_ip_configured(zero_ip_addr, zero_ip_addr, zero_ip_addr);
#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
            sal_memcpy(autoip_addr, zero_ip_addr, 4);
#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */
            dhcpc_renew();
        }
    }
}
#endif /* CFG_DHCPC_INCLUDED */

#endif /* CFG_NET_LINKCHANGE_NOTIFY_INCLUDED */

void
net_utils_init()
{
#ifdef CFG_NET_LINKCHANGE_NOTIFY_INCLUDED
    {
        uint8 i;
        net_enable_link_funcs = TRUE;
        net_intf_link = FALSE;
        for(i=0; i<CFG_NET_MAX_LINKCHANGE_HANDLER; i++) {
            net_link_funcs[i] = NULL;
        }
        sys_register_linkchange(net_port_linkchange_handler, NULL);
#ifdef CFG_DHCPC_INCLUDED
        net_register_linkchange(net_utils_linkchange);
#endif /* CFG_DHCPC_INCLUDED */
    }
#endif /* CFG_NET_LINKCHANGE_NOTIFY_INCLUDED */
}
#endif /* CFG_UIP_STACK_ENABLED */

