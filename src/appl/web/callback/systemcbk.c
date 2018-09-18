/*
 * $Id: systemcbk.c,v 1.26 Broadcom SDK $
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
#include "appl/httpd.h"
#include "appl/ssp.h"
#include "utils/net.h"
#include "appl/mdns.h"
#include "appl/persistence.h"
#include "../content/sspmacro_system.h"
#include "../content/sspmacro_system_name.h"
#ifndef __BOOTLOADER__
#include "uip.h"
#endif /* __BOOTLOADER__ */

#define WEB_SYSTEM_NAME_LEN     30

#define SYSCBK_DEBUG 0

#if SYSCBK_DEBUG
#define SYSCBK_LOG(x)    do { sal_printf("SYS-CBK: "); sal_printf x; sal_printf("\n"); } while(0);
#else
#define SYSCBK_LOG(x)
#endif

#ifdef __BOOTLOADER__

void 
sspvar_system_tag_net(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT
{
    uint8 addr[4];

    UNREFERENCED_PARAMETER(psmem);
    addr[0] = 0;
    addr[1] = 0;
    addr[2] = 0;
    addr[3] = 0;

    sal_strcpy(ssputil_shared_buffer, "");
    ret->type = SSPVAR_RET_STRING;
    switch (params[0]) {
    case SSPMACRO_SYSTEM_IPADDR:
        sal_sprintf(ssputil_shared_buffer,"%u.%u.%u.%u",
                (int)addr[0], (int)addr[1], (int)addr[2], (int)addr[3]);
        ret->val_data.string = ssputil_shared_buffer;
        break;
        
    case SSPMACRO_SYSTEM_HTTP_PROTO:
        ret->val_data.string = "http";
        break;
    default :
        ret->val_data.string = "-- Internal Error --";
        break;
        
    };

}

#else /* __BOOTLOADER__ */

#ifdef HTTPD_TIMER_SUPPORT

STATICCBK void 
systemcbk_timer_reboot(void *in_data) REENTRANT
{
    httpd_delete_timers_by_callback(systemcbk_timer_reboot);
    
    board_reset(in_data);

}
#endif /* #ifdef HTTPD_TIMER_SUPPORT */

SSP_HANDLER_RETVAL 
ssphandler_reset_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT
{
    bookkeeping_t bk_data;
    BOOL hard_reset;
    
    UNREFERENCED_PARAMETER(psmem);

    if (cxt->type != SSP_HANDLER_QUERY_STRINGS) {
        return SSP_HANDLER_RET_INTACT;
    }

    if (sal_strcmp(cxt->pairs[0].name, "reset_op")) {
        /* not 'reset_op' */
        return SSP_HANDLER_RET_INTACT;
    }
    
    if (!sal_strcmp(cxt->pairs[0].value, "fwupgrade")) {
        
        uint8 addr[4], netmask[4], gateway[4];
        
        SYSCBK_LOG(("Restarting system by web request.."));
        get_network_interface_config(addr, netmask, gateway);
        
        bk_data.magic = UM_BOOKKEEPING_SEAL;
        sal_memcpy(bk_data.agent_ip, addr, 4);
        sal_memcpy(bk_data.agent_netmask, netmask, 4);
        sal_memcpy(bk_data.agent_gateway, gateway, 4);
        
#if CFG_UIP_IPV6_ENABLED
        /* Get the manual IPv6 address */
        {
            uint8 count, i;
            uip_ip6addr_t ip6;
            ip6addr_type type;
            count = get_ipv6_address_count();
            for(i=0; i<count; i++) {
                get_ipv6_address(i, &ip6, &type);
                if (type == IP6ADDR_TYPE_MANUAL) {
                    sal_memcpy(&bk_data.agent_ipv6, &ip6, sizeof(ip6));
                    break;
                }
            }
            if (i == count) {
                /* not found, fill it as unspecified */
                sal_memset(&bk_data.agent_ipv6, 0, sizeof(ip6));
            }
        }
#endif /* CFG_UIP_IPV6_ENABLED */

        board_loader_mode_set(LM_UPGRADE_FIRMWARE, &bk_data);
        hard_reset = FALSE;
#ifdef HTTPD_TIMER_SUPPORT
        httpd_create_timer(2, systemcbk_timer_reboot, (void *)&hard_reset);
#endif  /* HTTPD_TIMER_SUPPORT */
    } else if (!sal_strcmp(cxt->pairs[0].value, "factory")) {
#if CFG_PERSISTENCE_SUPPORT_ENABLED
        persistence_restore_factory_defaults();
        persistence_save_all_current_settings();
#endif        
        hard_reset = TRUE;
#ifdef HTTPD_TIMER_SUPPORT
        httpd_create_timer(3, systemcbk_timer_reboot, (void *)&hard_reset);
#endif  /* HTTPD_TIMER_SUPPORT */
    } else if (!sal_strcmp(cxt->pairs[0].value, "reboot")) {
        hard_reset = TRUE;
#ifdef HTTPD_TIMER_SUPPORT
        httpd_create_timer(1, systemcbk_timer_reboot, (void *)&hard_reset);
#endif  /* HTTPD_TIMER_SUPPORT */
    }

    return SSP_HANDLER_RET_INTACT;
}

SSP_HANDLER_RETVAL 
ssphandler_sys_action_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT
{
    UNREFERENCED_PARAMETER(psmem);
    UNREFERENCED_PARAMETER(cxt);

    if (cxt->type != SSP_HANDLER_QUERY_STRINGS) {
        return SSP_HANDLER_RET_INTACT;
    }

    if (sal_strcmp(cxt->pairs[0].name, "action_op")){
        /* not 'action_op' */
    }

    if (!sal_strcmp(cxt->pairs[0].name, "action_op")){
        if (!sal_strcmp(cxt->pairs[0].value, "save")){
#if (CFG_PERSISTENCE_SUPPORT_ENABLED)
            persistence_save_all_current_settings();
#else /* CFG_PERSISTENCE_SUPPORT_ENABLED */
            /* code for save configuraiton with non-persistence_save desgin. */
#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */
        }
    }

    return SSP_HANDLER_RET_INTACT;
}

SSP_HANDLER_RETVAL 
ssphandler_bonjour_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT
{
#ifdef CFG_ZEROCONF_MDNS_INCLUDED
    BOOL    enable;
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

    UNREFERENCED_PARAMETER(psmem);
    UNREFERENCED_PARAMETER(cxt);

#ifdef CFG_ZEROCONF_MDNS_INCLUDED
    mdns_bonjour_enable_get(&enable);
    mdns_bonjour_enable_set(!enable);
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

#if CFG_PERSISTENCE_SUPPORT_ENABLED
    persistence_save_current_settings("mdns");
#endif
    return SSP_HANDLER_RET_INTACT;
}

void 
sspvar_system_name_tag_info(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT 
{

    UNREFERENCED_PARAMETER(psmem);
    
    sal_memset(ssputil_shared_buffer, 0, SSPUTIL_CBK_SHARED_SIZE);
    switch (params[0]) 
    {
        case SSPMACRO_SYSTEM_NAME_NAME:
            ret->type = SSPVAR_RET_STRING;
            if (get_system_name(ssputil_shared_buffer, WEB_SYSTEM_NAME_LEN + 1) != SYS_OK) {
                SYSCBK_LOG(("Report sytem name failed."));
                sal_strcpy(ssputil_shared_buffer, "");
            }
            ret->val_data.string = ssputil_shared_buffer;
        
            break;
        case SSPMACRO_SYSTEM_NAME_MAX_LENGTH:
            ret->type = SSPVAR_RET_INTEGER;
            ret->val_data.integer = WEB_SYSTEM_NAME_LEN;
        
            break;
        default:
            break;
    }
    
    return;
}

SSP_HANDLER_RETVAL 
ssphandler_system_name_set_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT
{

    UNREFERENCED_PARAMETER(psmem);

    if (cxt->type != SSP_HANDLER_QUERY_STRINGS) 
    {
        return SSP_HANDLER_RET_INTACT;
    }

    sal_strncpy(ssputil_shared_buffer, (char *)cxt->pairs[0].value, WEB_SYSTEM_NAME_LEN);
    if (set_system_name(ssputil_shared_buffer) != SYS_OK) {
        SYSCBK_LOG(("Set sytem name failed."));
    }
#if CFG_PERSISTENCE_SUPPORT_ENABLED
    persistence_save_current_settings("name");
#endif    
    return SSP_HANDLER_RET_INTACT;
}

void 
sspvar_system_tag_info(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT
{

    UNREFERENCED_PARAMETER(psmem);

    ret->type = SSPVAR_RET_STRING;
    sal_memset(ssputil_shared_buffer, 0, SSPUTIL_CBK_SHARED_SIZE);
    switch (params[0]) {
    case SSPMACRO_SYSTEM_DEVNAME:

        if (get_system_name(ssputil_shared_buffer, WEB_SYSTEM_NAME_LEN + 1) != SYS_OK) {
            SYSCBK_LOG(("Report sytem name failed."));
            sal_strcpy(ssputil_shared_buffer, "-Unknown-");
        }
        ret->val_data.string = ssputil_shared_buffer;
        break;
        
    case SSPMACRO_SYSTEM_LINKPORTS:
        {
            uint16 uport;
            int serial_id0, serial_cnt,link_cnt;
            port_mode_t mode;
            
            serial_id0 = -1;
            serial_cnt = link_cnt = 0;
            sal_strcpy(ssputil_shared_buffer, "");
            
            SAL_UPORT_ITER(uport) {
                board_port_mode_get(uport, &mode);
                if(mode != PM_LINKDOWN) {
                    /* linked up */
                    link_cnt++;
                    serial_cnt++;
                    if (serial_id0 == -1){
                        serial_id0 = (int) uport;
                        sal_sprintf(ssputil_shared_buffer + sal_strlen(ssputil_shared_buffer), 
                                "%sP%d", ((link_cnt == 1) ? "" : ","), serial_id0);
                    } else {
                        serial_id0 = (int) uport;
                    }
                    
                    /* print on last port if linked */
                    if (SAL_UPORT_IS_NOT_VALID(uport+1) && (serial_cnt > 1)) {
                        sal_sprintf(ssputil_shared_buffer + sal_strlen(ssputil_shared_buffer),"%sP%d",
                                (serial_cnt == 2) ? "," : "..",
                                serial_id0);
                    }
                } else {
                    /* linked down */
                    if (serial_cnt > 1){
                        sal_sprintf(ssputil_shared_buffer + sal_strlen(ssputil_shared_buffer),"%sP%d",
                                (serial_cnt == 2) ? "," : "..",
                                serial_id0);
                    }
                    serial_id0 = -1;
                    serial_cnt = 0;
                }
            }
            ret->val_data.string = ssputil_shared_buffer;
        }
        break;
        
    }
}

void 
sspvar_system_tag_version(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT
{
    uint8   ver[4];
    
    UNREFERENCED_PARAMETER(psmem);

    ret->type = SSPVAR_RET_STRING;
    sal_strcpy(ssputil_shared_buffer, "");
    switch (params[0]) {
    case SSPMACRO_SYSTEM_UMPLUS:
        board_firmware_version_get(ver, ver+1, ver+2, ver+3);
        sal_sprintf(ssputil_shared_buffer, "Ver %u.%02u.%02u",
                (int)ver[0], (int)ver[1], (int)ver[2]);
        break;
        
    case SSPMACRO_SYSTEM_DATE:
        sal_sprintf(ssputil_shared_buffer, "%s", __DATE__);
        ret->val_data.string = __DATE__;
        break;
        
    default:
        sal_sprintf(ssputil_shared_buffer, "--Unknown--");
    }
    ret->val_data.string = ssputil_shared_buffer;
}

void 
sspvar_system_tag_net(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT
{
    uint8 addr[6], netmask[4], gateway[4];
    BOOL    enable;
#if CFG_UIP_IPV6_ENABLED
    uint8   ipv6_addr_cnt, i;
    ip6addr_type    ipv6type;
    uip_ip6addr_t   ipv6addr;
#endif  /* CFG_UIP_IPV6_ENABLED */

    UNREFERENCED_PARAMETER(psmem);

    get_network_interface_config(addr, netmask, gateway);
        
    sal_strcpy(ssputil_shared_buffer, "");
    ret->type = SSPVAR_RET_STRING;
    switch (params[0]) {
    case SSPMACRO_SYSTEM_MACADDR:
        get_system_mac(addr);
        sal_sprintf(ssputil_shared_buffer,"%02x:%02x:%02x:%02x:%02x:%02x",
                (int)addr[0], (int)addr[1], (int)addr[2], 
                (int)addr[3], (int)addr[4], (int)addr[5]);
        ret->val_data.string = ssputil_shared_buffer;
        break;
        
    case SSPMACRO_SYSTEM_IPV6ADDR:
#if CFG_UIP_IPV6_ENABLED
        ipv6_addr_cnt = get_ipv6_address_count();
        for (i = 0; i < ipv6_addr_cnt; i++){
            if (get_ipv6_address(i, &ipv6addr, &ipv6type) == SYS_OK) {
                sal_sprintf(ssputil_shared_buffer, "%s%x:%x:%x:%x:%x:%x:%x:%x (%s)%s",
                        ssputil_shared_buffer,
                        (int)UIP_HTONS(((uint16 *)(&ipv6addr))[0]),
                        (int)UIP_HTONS(((uint16 *)(&ipv6addr))[1]),
                        (int)UIP_HTONS(((uint16 *)(&ipv6addr))[2]),
                        (int)UIP_HTONS(((uint16 *)(&ipv6addr))[3]),
                        (int)UIP_HTONS(((uint16 *)(&ipv6addr))[4]),
                        (int)UIP_HTONS(((uint16 *)(&ipv6addr))[5]),
                        (int)UIP_HTONS(((uint16 *)(&ipv6addr))[6]),
                        (int)UIP_HTONS(((uint16 *)(&ipv6addr))[7]),
                        ((ipv6type == IP6ADDR_TYPE_AUTO_IP) ? "Auto-IP" : 
                        ((ipv6type == IP6ADDR_TYPE_MANUAL) ? "Manual" : "Link-Local")),
                        (i == (ipv6_addr_cnt - 1)) ? "&nbsp;" : "<BR>&nbsp;&nbsp;"); 
            }                                                                                                                        
        }
#else  /* CFG_UIP_IPV6_ENABLED */
        sal_sprintf(ssputil_shared_buffer,"N/A"); 
#endif  /* CFG_UIP_IPV6_ENABLED */
        ret->val_data.string = ssputil_shared_buffer;
        break;
        
    case SSPMACRO_SYSTEM_IPADDR:
        sal_sprintf(ssputil_shared_buffer,"%u.%u.%u.%u",
                (int)addr[0], (int)addr[1], (int)addr[2],(int)addr[3]); 
        ret->val_data.string = ssputil_shared_buffer;
        break;
        
    case SSPMACRO_SYSTEM_NETMASK:
        sal_sprintf(ssputil_shared_buffer,"%u.%u.%u.%u",
                (int)netmask[0], (int)netmask[1], (int)netmask[2], 
                (int)netmask[3]);
        ret->val_data.string = ssputil_shared_buffer;
        break;
        
    case SSPMACRO_SYSTEM_GATEWAY:
        sal_sprintf(ssputil_shared_buffer,"%u.%u.%u.%u",
                (int)gateway[0], (int)gateway[1], (int)gateway[2], 
                (int)gateway[3]);
        ret->val_data.string = ssputil_shared_buffer;
        break;
        
    case SSPMACRO_SYSTEM_BONJOUR:
        /* need bonjour API to report the enable status  */
#ifdef CFG_ZEROCONF_MDNS_INCLUDED
        mdns_bonjour_enable_get(&enable);
#else /* CFG_ZEROCONF_MDNS_INCLUDED */
        enable = FALSE;
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

        ret->val_data.string = (enable) ? "Enabled" : "Disabled";
        break;
        
    case SSPMACRO_SYSTEM_HTTP_PROTO:
        ret->val_data.string = "http";
        break;
    default :
        ret->val_data.string = "-- Internal Error --";
        break;
        
    };

}

/* Send correct MIME type "text/css" in HTTP header for CSS files */
SSP_HANDLER_RETVAL 
ssphandler_text_css(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT
{
    SSP_HANDLER_CONTEXT_EXT *pcxt = (SSP_HANDLER_CONTEXT_EXT *)cxt;

    UNREFERENCED_PARAMETER(psmem);
    
    if (cxt->type != SSP_HANDLER_SET_HEADER) {
        return SSP_HANDLER_RET_INTACT;
    }
    
    /* Copy strings to buffer to avoid banking issues */
    sal_strcpy(ssputil_shared_buffer, "Content-Type");
    sal_strcpy(ssputil_shared_buffer + 20, "text/css");
    pcxt->url_data.string_pair.name = ssputil_shared_buffer;
    pcxt->url_data.string_pair.value = ssputil_shared_buffer + 20;
    pcxt->flags &= ~SSPF_SET_HEADER_H;

    return SSP_HANDLER_RET_MODIFIED;
}

#endif /*  !__BOOTLOADER__ */
