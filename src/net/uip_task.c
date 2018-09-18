/*
 * $Id: uip_task.c,v 1.17 Broadcom SDK $
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
#include "uip_arp.h"
#include "uip_timers.h"
#include "uip_task.h"
#include "appl/dhcpc.h"

#if CFG_UIP_IPV6_ENABLED
#define uip_ipaddr_t uip_ip6addr_t
#include "uip-ds6.h"
#undef uip_ipaddr_t
#endif /* CFG_UIP_IPV6_ENABLED */

#if CFG_HTTPD_ENABLED
#include "appl/httpd.h"
#include "appl/ssp.h"
#endif  /* CFG_HTTPD_ENABLED */
#ifdef CFG_ZEROCONF_MDNS_INCLUDED
#include "appl/mdns.h"
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

#include "utils/nvram.h"
#include "utils/system.h"
#include "utils/net.h"

#ifdef CFG_FACTORY_CONFIG_INCLUDED
#include "utils/factory.h"
#endif /* CFG_FACTORY_CONFIG_INCLUDED */

#ifdef __BOOTLOADER__
#include BOOT_SOC_INCLUDE_FILE
#endif /* __BOOTLOADER__ */

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define UDPBUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

#if CFG_UIP_IPV6_ENABLED
#define UIP6_IP_BUF ((struct uip6_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#if UIP_CONF_IPV6_REASSEMBLY
extern struct etimer uip_reass_timer;
#endif /* UIP_CONF_IPV6_REASSEMBLY */

#endif /* CFG_UIP_IPV6_ENABLED */

#ifdef UIP_TASK_VERBOSE
#define UIP_TASK_LOG(x)    do { sal_printf x; } while(0)
#else /* !UIP_TASK_VERBOSE */
#define UIP_TASK_LOG(x)    do { } while(0)
#endif /* !UIP_TASK_VERBOSE */

STATIC tick_t uip_task_prev_ticks;

#if CFG_XGS_CHIP
#define CRC_PADDING             4
#define PRE_IP_HDR_AND_CRC_LEN  22 /* Length before IP header plus 4 CRC bytes. */
#else /* ROBO */
#define CRC_PADDING             0
#define ETYPE_LEN               2 /* Length of EtherType */
#endif
#define LLH_ETYPE_OFFSET        16 /* Ether type offset for XGS. */
#define DA_SA_MAC_LEN           12 /* DA + SA */
#define VLAN_TAG_PROTOCOL       0x8100
#define VLAN_TAG_LEN            4

#define PERIODIC_TIMER_INTERVAL 500000   /* 500000 us = 0.5 second */
#define ARP_TIMER_INTERVAL    10000000   /* 10000000 us = 10 second */

STATIC struct timer periodic_timer;
#ifndef __BOOTLOADER__
STATIC struct timer arp_timer;
#endif /* !__BOOTLOADER__ */

void uip_task(void *param) REENTRANT;

#ifdef __BOOTLOADER__
STATICCBK void loader_rx_handler(soc_rx_packet_t *pkt) REENTRANT;
#if defined(__ARM__)
STATIC uint8 rx_buffers[DEFAULT_RX_BUFFER_COUNT][DEFAULT_RX_BUFFER_SIZE] __attribute__ ((section(".packet_buf"), aligned (32)));
#else
STATIC uint8 rx_buffers[DEFAULT_RX_BUFFER_COUNT][DEFAULT_RX_BUFFER_SIZE];
#endif

/*
 * uIP rx handler for loader.
 */
STATICCBK void
loader_rx_handler(soc_rx_packet_t *pkt) REENTRANT
{
    uint16 etype;

#if CFG_XGS_CHIP

    /* Accept only IP or ARP packets */
    etype = *(uint16 *)(pkt->buffer + LLH_ETYPE_OFFSET);
    if (etype == uip_htons(UIP_ETHTYPE_IP) ||
#if CFG_UIP_IPV6_ENABLED
        etype == uip_htons(UIP_ETHTYPE_IP6) ||
#endif /* CFG_UIP_IPV6_ENABLED */
        etype == uip_htons(UIP_ETHTYPE_ARP)) {
        sal_memcpy(uip_buf, pkt->buffer, DA_SA_MAC_LEN);

        sal_memcpy(uip_buf + DA_SA_MAC_LEN,
                   pkt->buffer + LLH_ETYPE_OFFSET,
                   pkt->pktlen - DA_SA_MAC_LEN);
        uip_len = pkt->pktlen - PRE_IP_HDR_AND_CRC_LEN;
    }

#else /* !CFG_XGS_CHIP */

    /* Ethertype or VLAN tag */
    etype = *(uint16 *)(pkt->buffer + DA_SA_MAC_LEN);

    /* Check and strip off VLAN tag*/
    if (etype == uip_htons(VLAN_TAG_PROTOCOL)) {

        /* Accept only IP or ARP packets */
        etype = *(uint16 *)(pkt->buffer + LLH_ETYPE_OFFSET);
        if (etype == uip_htons(UIP_ETHTYPE_IP) ||
#if CFG_UIP_IPV6_ENABLED
            etype == uip_htons(UIP_ETHTYPE_IP6) ||
#endif /* CFG_UIP_IPV6_ENABLED */
            etype == uip_htons(UIP_ETHTYPE_ARP)) {
            sal_memcpy(uip_buf, pkt->buffer, DA_SA_MAC_LEN);
            sal_memcpy(uip_buf+DA_SA_MAC_LEN,
                       pkt->buffer + DA_SA_MAC_LEN + VLAN_TAG_LEN,
                       pkt->pktlen - DA_SA_MAC_LEN - VLAN_TAG_LEN);
            uip_len = pkt->pktlen - DA_SA_MAC_LEN - ETYPE_LEN - VLAN_TAG_LEN;
        }

    } else {

        /* Accept only IP or ARP packets */
        if (etype == uip_htons(UIP_ETHTYPE_IP) ||
#if CFG_UIP_IPV6_ENABLED
            etype == uip_htons(UIP_ETHTYPE_IP6) ||
#endif /* CFG_UIP_IPV6_ENABLED */
            etype == uip_htons(UIP_ETHTYPE_ARP)) {
            sal_memcpy(uip_buf, pkt->buffer, pkt->pktlen);
            uip_len = pkt->pktlen - DA_SA_MAC_LEN - ETYPE_LEN;
        }
    }

#endif

    BOOT_FUNC_RX_FILL_BUFFER(0, pkt);
}
/*
 * Main uIP tx callback routine for firmware.
 */
STATICCBK void
loader_uip_tx_async_cbk(struct soc_tx_packet_s *pkt) REENTRANT
{
    if (pkt) {
        if (pkt->buffer) {
            sal_dma_free(pkt->buffer);
        }
        sal_free(pkt);
    }
}
/*
 * Main uIP tx routine for loader.
 */
static sys_error_t
uip_task_send(void)
{
    /* Send packet in uip_buf */
    soc_tx_packet_t *spkt;

#if CFG_ROBO_CHIP
    if (uip_len < MIN_PACKET_LENGTH) {
        uip_len = MIN_PACKET_LENGTH;
    }
#endif

    spkt = (soc_tx_packet_t *)sal_malloc(sizeof(soc_tx_packet_t));
    if (spkt == NULL) {
        return SYS_ERR_OUT_OF_RESOURCE;
    }
    sal_memset(spkt, 0, sizeof(soc_tx_packet_t));

    spkt->buffer = (uint8 *)sal_dma_malloc(uip_len + CRC_PADDING);
    if (spkt->buffer == NULL) {
        sal_free(spkt);
        return SYS_ERR_OUT_OF_RESOURCE;
    }
    spkt->pktlen = uip_len + CRC_PADDING;
    spkt->callback = loader_uip_tx_async_cbk;
    sal_memcpy(spkt->buffer, BUF, uip_len);

    return BOOT_FUNC_TX(0, spkt);
}

/*
 * Init routine for loader.
 */
void
uip_task_init()
{
    uint8 i;
    soc_rx_packet_t *spkt;
    struct uip_eth_addr mac_addr;
    bookkeeping_t bk_data;

    /* Update mac address info in uIP. */
    get_system_mac((uint8 *)&mac_addr);
    uip_setethaddr(mac_addr);

    timer_set(&periodic_timer, SAL_USEC_TO_TICKS(PERIODIC_TIMER_INTERVAL));

    sal_srand(sal_get_ticks());

    uip_init();
    uip_arp_init(FALSE);
#ifdef CFG_NET_LINKCHANGE_NOTIFY_INCLUDED
    net_register_linkchange(uip_arp_init);
#endif /* CFG_NET_LINKCHANGE_NOTIFY_INCLUDED */

#if CFG_UIP_IPV6_ENABLED
    uip6_init();
#ifdef CFG_NET_LINKCHANGE_NOTIFY_INCLUDED
    net_register_linkchange(uip_ds6_init);
#endif /* CFG_NET_LINKCHANGE_NOTIFY_INCLUDED */
#endif /* CFG_UIP_IPV6_ENABLED */

    BOOT_FUNC_RX_SET_HANDLER(0, loader_rx_handler, FALSE);

    for(i=0; i<DEFAULT_RX_BUFFER_COUNT; i++) {
        spkt = (soc_rx_packet_t *)sal_malloc(sizeof(soc_rx_packet_t));
        if (spkt == NULL) {
            return;
        }
        spkt->buffer = rx_buffers[i];
        spkt->buflen = DEFAULT_RX_BUFFER_SIZE;
        BOOT_FUNC_RX_FILL_BUFFER(0, spkt);
    }
#ifdef CFG_ZEROCONF_MDNS_INCLUDED
    mdns_init();
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

#if  CFG_HTTPD_ENABLED
    httpd_init();
#endif  /* CFG_HTTPD_ENABLED */

    /* Register a background task for RX handling */
    task_add(uip_task, (void *)NULL);

    if ((board_loader_mode_get(&bk_data, FALSE) == LM_UPGRADE_FIRMWARE)) {

        /* Pick up bookkeeping data from firmware. */
        set_network_interface_config(
            INET_CONFIG_STATIC,
            (const uint8 *)bk_data.agent_ip,
            (const uint8 *)bk_data.agent_netmask,
            (const uint8 *)bk_data.agent_gateway);

#if CFG_UIP_IPV6_ENABLED
        if (!uip_is_addr_unspecified((uip_ip6addr_t *)&bk_data.agent_ipv6)) {
            uip_ds6_addr_add(
                (uip_ip6addr_t *)&bk_data.agent_ipv6, 0, ADDR_MANUAL);
        }
#endif /* CFG_UIP_IPV6_ENABLED */

    } else {
        /*
         *  Found invalid firmware if we got here. Loader always uses default IP.
         */
#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
        /* Start auto ip configuration */
        uip_autoip_start();
#else /* !CFG_ZEROCONF_AUTOIP_INCLUDED */
        set_network_interface_config(
            INET_CONFIG_STATIC,
            (const uint8 *)default_ip_addr,
            (const uint8 *)default_netmask,
            (const uint8 *)default_gateway);
#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */

    }
}

#else /* !__BOOTLOADER__ */

/*
 * uIP rx handler for firmware.
 */
STATICCBK sys_rx_t
uip_task_rx_handler(sys_pkt_t *pkt, void *cookie) REENTRANT
{
    /*
     * Note: Currently the uIP process handles packet in one shot
     * (i.e. without yielding execution in the middle) so it's not
     * possible to have another packet come in when a packet is being
     * processed by uIP stack.
     *
     * Therefore, there is no need (for now) to handle such situation.
     *
     * However, if we later modify uIP processing and POLL() is involved
     * in uIP stack, we would need to do buffer flow control to prevent
     * uip_buf corruption.
     */

    uint16 etype;

    UNREFERENCED_PARAMETER(cookie);
    SAL_ASSERT(pkt != NULL);
    SAL_ASSERT(pkt->pkt_data != NULL);

#if CFG_XGS_CHIP

    /* Accept only IP or ARP packets */
    etype = *(uint16 *)(pkt->pkt_data + LLH_ETYPE_OFFSET);
    if (etype != uip_htons(UIP_ETHTYPE_IP) &&
#if CFG_UIP_IPV6_ENABLED
        etype != uip_htons(UIP_ETHTYPE_IP6) &&
#endif /* CFG_UIP_IPV6_ENABLED */
        etype != uip_htons(UIP_ETHTYPE_ARP)) {
        return SYS_RX_NOT_HANDLED;
    }

    /* Copy packet into uip_buf. */
    sal_memcpy(uip_buf, pkt->pkt_data, DA_SA_MAC_LEN);

    sal_memcpy(uip_buf + DA_SA_MAC_LEN,
               pkt->pkt_data + LLH_ETYPE_OFFSET,
               pkt->pkt_len - DA_SA_MAC_LEN);
    uip_len = pkt->pkt_len - PRE_IP_HDR_AND_CRC_LEN;

#else /* !CFG_XGS_CHIP */

    /* Ethertype or VLAN tag */
    etype = *(uint16 *)(pkt->pkt_data + DA_SA_MAC_LEN);

    /* Check and strip off VLAN tag*/
    if (etype == uip_htons(VLAN_TAG_PROTOCOL)) {

        /* Accept only IP or ARP packets */
        etype = *(uint16 *)(pkt->pkt_data + LLH_ETYPE_OFFSET);
        if (etype != uip_htons(UIP_ETHTYPE_IP) &&
#if CFG_UIP_IPV6_ENABLED
            etype != uip_htons(UIP_ETHTYPE_IP6) &&
#endif /* CFG_UIP_IPV6_ENABLED */
            etype != uip_htons(UIP_ETHTYPE_ARP)) {
            return SYS_RX_NOT_HANDLED;
        }

        /* Copy packet into uip_buf. */
        sal_memcpy(uip_buf, pkt->pkt_data, DA_SA_MAC_LEN);
        sal_memcpy(uip_buf + DA_SA_MAC_LEN,
               pkt->pkt_data + DA_SA_MAC_LEN + VLAN_TAG_LEN,
               pkt->pkt_len - DA_SA_MAC_LEN - VLAN_TAG_LEN);
        uip_len = pkt->pkt_len - DA_SA_MAC_LEN - ETYPE_LEN - VLAN_TAG_LEN;

    } else {

        /* Accept only IP or ARP packets */
        if (etype != uip_htons(UIP_ETHTYPE_IP) &&
#if CFG_UIP_IPV6_ENABLED
            etype != uip_htons(UIP_ETHTYPE_IP6) &&
#endif /* CFG_UIP_IPV6_ENABLED */
            etype != uip_htons(UIP_ETHTYPE_ARP)) {
            return SYS_RX_NOT_HANDLED;
        }

        /* Copy packet into uip_buf. */
        sal_memcpy(uip_buf, pkt->pkt_data, pkt->pkt_len);
        uip_len = pkt->pkt_len - DA_SA_MAC_LEN - ETYPE_LEN;
    }
#endif /* CFG_XGS_CHIP */

    return SYS_RX_HANDLED;
}
/*
 * Main uIP tx callback routine for firmware.
 */
STATICCBK void
uip_tx_async_cbk(sys_pkt_t *pkt, sys_error_t status) REENTRANT
{
    if (status != SYS_OK) {
        UIP_TASK_LOG(("\n uIP TX cbk ERROR(%d)!\n", (int16)status));
    }
    if (pkt) {
        if (pkt->pkt_data) {
            sal_dma_free(pkt->pkt_data);
        }
        sal_free(pkt);
    }
}
/*
 * Main uIP tx routine for firmware.
 */
static sys_error_t
uip_task_send(void)
{
    sys_error_t r;
    sys_pkt_t *pkt;

    /* Send packet in uip_buf */
    pkt = (sys_pkt_t *)sal_malloc(sizeof(sys_pkt_t));
    if (pkt == NULL) {
        UIP_TASK_LOG(("Out of memory!\n"));
        return SYS_ERR_OUT_OF_RESOURCE;
    }
    sal_memset(pkt, 0, sizeof(sys_pkt_t));
#if CFG_ROBO_CHIP
    if (uip_len < MIN_PACKET_LENGTH) {
        uip_len = MIN_PACKET_LENGTH;
    }
#endif

#ifdef CFG_IP_FRAGMENT_INCLUDED
    if (uip_len > MAX_PACKET_LENGTH) {
        uint16 remain_len = uip_len - MAX_PACKET_LENGTH;
        pkt->pkt_data = (uint8 *)sal_dma_malloc(MAX_PACKET_LENGTH + CRC_PADDING);
        if (pkt->pkt_data == NULL) {
            UIP_TASK_LOG(("Out of memory!\n"));
            sal_free(pkt);
            return SYS_ERR_OUT_OF_RESOURCE;
        }

        UDPBUF->len[0] = (MAX_IP_TOTAL_LENGTH >> 8);
        UDPBUF->len[1] = (MAX_IP_TOTAL_LENGTH & 0xff);

        /* More fragment */
        UDPBUF->ipoffset[0] = 0x20;
        UDPBUF->ipoffset[1] = 0;

        UDPBUF->ipchksum = 0;
        UDPBUF->ipchksum = uip_ipchksum();

        sal_memcpy(pkt->pkt_data, BUF, MAX_PACKET_LENGTH);
        pkt->pkt_len = MAX_PACKET_LENGTH + CRC_PADDING;

        r = sys_tx(pkt, uip_tx_async_cbk);
        if (r != SYS_OK) {
            UIP_TASK_LOG(("\n uIP TX ERROR(%d)!\n", (int16)r));
            return r;
        }

        /* Second packet */
        pkt = (sys_pkt_t *)sal_malloc(sizeof(sys_pkt_t));
        if (pkt == NULL) {
            UIP_TASK_LOG(("Out of memory!\n"));
            return SYS_ERR_OUT_OF_RESOURCE;
        }
        sal_memset(pkt, 0, sizeof(sys_pkt_t));

        /* Copy remaining data to begining of buffer */
        sal_memcpy((void *)&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN],
                   (void *)&uip_buf[MAX_PACKET_LENGTH], remain_len);

        remain_len += UIP_IPH_LEN;
        pkt->pkt_data = (uint8 *)sal_dma_malloc(remain_len + UIP_LLH_LEN + CRC_PADDING);
        if (pkt->pkt_data == NULL) {
            UIP_TASK_LOG(("Out of memory!\n"));
            sal_free(pkt);
            return SYS_ERR_OUT_OF_RESOURCE;
        }

        UDPBUF->len[0] = (remain_len >> 8);
        UDPBUF->len[1] = (remain_len & 0xff);

        /* Fragment offset = 1480/8 */
        UDPBUF->ipoffset[0] = 0x0;
        UDPBUF->ipoffset[1] = 0xB9;

        UDPBUF->ipchksum = 0;
        UDPBUF->ipchksum = uip_ipchksum();

        sal_memcpy(pkt->pkt_data, BUF, remain_len + UIP_LLH_LEN + CRC_PADDING);
        pkt->pkt_len = remain_len + UIP_LLH_LEN + CRC_PADDING;

        r = sys_tx(pkt, uip_tx_async_cbk);
        if (r != SYS_OK) {
            UIP_TASK_LOG(("\n uIP TX ERROR(%d)!\n", (int16)r));
        }
    } else
#endif /* CFG_IP_FRAGMENT_INCLUDED */
    {
        pkt->pkt_data = (uint8 *)sal_dma_malloc(uip_len + CRC_PADDING);
        if (pkt->pkt_data == NULL) {
            UIP_TASK_LOG(("Out of memory!\n"));
            sal_free(pkt);
            return SYS_ERR_OUT_OF_RESOURCE;
        }
        sal_memcpy(pkt->pkt_data, BUF, uip_len);
        pkt->pkt_len = uip_len + CRC_PADDING;

        r = sys_tx(pkt, uip_tx_async_cbk);
        if (r != SYS_OK) {
            UIP_TASK_LOG(("\n uIP TX ERROR(%d)!\n", (int16)r));
        }
    }
    return r;
}
/*
 * Init routine for firmware.
 */
void
uip_task_init()
{
    uip_ipaddr_t ipaddr;
    struct uip_eth_addr mac_addr;

    /* Update mac address info in uIP. */
    get_system_mac((uint8 *)&mac_addr);
    uip_setethaddr(mac_addr);

    timer_set(&periodic_timer, SAL_USEC_TO_TICKS(PERIODIC_TIMER_INTERVAL));
    timer_set(&arp_timer, SAL_USEC_TO_TICKS(ARP_TIMER_INTERVAL));

    sal_srand(sal_get_ticks());

    uip_init();
    uip_arp_init(FALSE);
#ifdef CFG_NET_LINKCHANGE_NOTIFY_INCLUDED
    net_register_linkchange(uip_arp_init);
#endif /* CFG_NET_LINKCHANGE_NOTIFY_INCLUDED */

#if CFG_UIP_IPV6_ENABLED
    uip6_init();
#ifdef CFG_NET_LINKCHANGE_NOTIFY_INCLUDED
    net_register_linkchange(uip_ds6_init);
#endif /* CFG_NET_LINKCHANGE_NOTIFY_INCLUDED */
#endif /* CFG_UIP_IPV6_ENABLED */

    uip_ipaddr(&ipaddr, 0,0,0,0);
    uip_sethostaddr(&ipaddr);

#ifdef CFG_DHCPC_INCLUDED
    /* DHCP is not actually started until serializer enables it */
    dhcpc_init(&mac_addr);
#endif /* CFG_DHCPC_INCLUDED */

#ifdef CFG_ZEROCONF_MDNS_INCLUDED
    mdns_init();
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

#if  CFG_HTTPD_ENABLED
    httpd_init();
#endif  /* CFG_HTTPD_ENABLED */

    sys_rx_register(
        uip_task_rx_handler,
        CFG_UIP_RX_PRIORITY,
        NULL,
        0);

    /* Register a background task for RX handling */
    task_add(uip_task, (void *)NULL);
}
#endif /* __BOOTLOADER__ */

/*
 *  Function : tcpip_out
 *
 *  Purpose :
 *      Send out the packet if uip_len > 0
 *
 *  Parameters :
 *
 *  Return :
 *
 *  Note :
 *
 */
STATICFN void
tcpip_out(void)
{
    /*
     * If previous function invocation resulted in data that
     *  should be sent out on the network, the global variable
     *  uip_len is set to a value > 0.
     */
    if(uip_len == 0) {
        return;
    }

#if CFG_UIP_IPV6_ENABLED
    if (uip_ipv6) {

        if (uip_ds6_resolve_lladdr(&UIP6_IP_BUF->destipaddr, &BUF->dest) > 0) {
            memcpy(&BUF->src, &uip_lladdr, UIP_LLADDR_LEN);
            BUF->type = UIP_HTONS(UIP_ETHTYPE_IP6);
            uip_len += sizeof(struct uip_eth_hdr);
            uip_task_send();
        }

        uip_len = 0;
        uip_ext_len = 0;
        return;
    }
#endif /* !CFG_UIP_IPV6_ENABLED */

    /* IPv4 */
    uip_arp_out();
    uip_task_send();
    uip_len = 0;
}

#if UIP_TCP
/*
 *  Function : tcp_conns_periodic
 *
 *  Purpose :
 *      Checking TCP connections (called by timer)
 *
 *  Parameters :
 *
 *  Return :
 *
 *  Note :
 *
 */
void
tcp_conns_periodic(void)
{
    register uint8 i;

    for(i = 0; i < UIP_CONNS; i++) {
        uip_periodic(i);
        tcpip_out();
    }
}
#endif /* UIP_TCP */

#if UIP_UDP
/*
 *  Function : udp_conns_periodic
 *
 *  Purpose :
 *      Checking UDP connections (called by timer)
 *
 *  Parameters :
 *
 *  Return :
 *
 *  Note :
 *
 */
void
udp_conns_periodic(void)
{
    register uint8 i;

    for(i = 0; i < UIP_UDP_CONNS; i++) {
        uip_udp_periodic(i);
        tcpip_out();
    }
}
#endif /* UIP_UDP */

/*
 *  Function : uip_task
 *
 *  Purpose :
 *      Main control loop for uIP TCP/IP stack.
 *
 *  Parameters :
 *
 *  Return :
 *
 *  Note :
 *
 */
void
uip_task(void *param) REENTRANT
{
    UNREFERENCED_PARAMETER(param);

    if(uip_len > 0) {
        if(BUF->type == uip_htons(UIP_ETHTYPE_IP)) {

            uip_input();
            tcpip_out();

        } else if(BUF->type == uip_htons(UIP_ETHTYPE_ARP)) {

            uip_arp_arpin();
            if(uip_len > 0) {
                uip_task_send();
                uip_len = 0;
            }

#if CFG_UIP_IPV6_ENABLED
        } else if(BUF->type == uip_htons(UIP_ETHTYPE_IP6)) {

            uip6_input();
            tcpip_out();

#endif /* CFG_UIP_IPV6_ENABLED */

        } else {

            uip_len = 0;
        }

    } else if (uip_task_prev_ticks != sal_get_ticks()) {

        uip_task_prev_ticks = sal_get_ticks();

        if (timer_expired(&periodic_timer)) {

            timer_reset(&periodic_timer);

#if UIP_TCP
            tcp_conns_periodic();
#endif /* UIP_TCP */

#if UIP_UDP
            udp_conns_periodic();
#endif /* UIP_UDP */

#ifndef __BOOTLOADER__
            /* Call the ARP timer function every 10 seconds. */
            if(timer_expired(&arp_timer)) {
                timer_reset(&arp_timer);
                uip_arp_timer();
            }
#endif /* !__BOOTLOADER__ */

#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
            uip_autoip_appcall();
            if (uip_len > 0) {
                uip_task_send();
                uip_len = 0;
            }
#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */
        }

#if CFG_UIP_IPV6_ENABLED

        /* Address/neighbor checking and DAD */
        if (etimer_expired(&uip_ds6_timer_periodic)) {
            uip_ipv6 = TRUE;            /* For checksum and tcpip_out() */
            uip_ds6_periodic();
            tcpip_out();
        }

        /* Send Router Solicitation */
        if (etimer_expired(&uip_ds6_timer_rs)) {
            uip_ipv6 = TRUE;            /* For checksum and tcpip_out() */
            uip_ds6_send_rs();
            tcpip_out();
        }

#if UIP_CONF_IPV6_REASSEMBLY
        /* Check reassembly timeout */
        if (etimer_expired(&uip_reass_timer)) {
            uip_ipv6 = TRUE;            /* For checksum and tcpip_out() */
            uip_reass_over();
            tcpip_out();
        }
#endif /* UIP_CONF_IPV6_REASSEMBLY */

#endif /* CFG_UIP_IPV6_ENABLED */

    }
}

#if  UIP_TCP
void uip_appcall(void)
{
    switch (uip_htons(uip_conn->lport)) {
#if CFG_HTTPD_ENABLED
        case HTTP_TCP_PORT:
            httpd_appcall();
            break;
#endif  /* CFG_HTTPD_ENABLED */
        default:
            break;
    }
}
#endif  /* UIP_TCP */

#if UIP_UDP
void
udp_appcall(void)
{
    switch (uip_htons(uip_udp_conn->lport)) {
#ifdef CFG_DHCPC_INCLUDED
        case DHCPC_CLIENT_PORT:
            dhcpc_appcall();
            break;
#endif /* CFG_DHCPC_INCLUDED */

#ifdef CFG_ZEROCONF_MDNS_INCLUDED
        case DNS_MULTICAST_PORT:
            mdns_appcall();
            break;
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */
        default:
            break;
    }
}
#endif /* UIP_UDP */

#if CFG_UIP_IPV6_ENABLED
#if  UIP_TCP
void uip6_appcall(void)
{
    switch (uip_htons(uip_conn->lport)) {
#if CFG_HTTPD_ENABLED
        case HTTP_TCP_PORT:
            httpd_appcall();
            break;
#endif  /* CFG_HTTPD_ENABLED */
        default:
            break;
    }
}
#endif  /* UIP_TCP */

#if UIP_UDP
void
udp6_appcall(void)
{
    switch (uip_htons(uip_udp_conn->lport)) {
#ifdef CFG_ZEROCONF_MDNS_INCLUDED
        case DNS_MULTICAST_PORT:
            mdns_appcall();
            break;
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */
        default:
            break;
    }
}
#endif /* UIP_UDP */
#endif /* CFG_UIP_IPV6_ENABLED */

#if UIP_LOGGING
void
uip_log(char *m)
{
#if CFG_CONSOLE_ENABLED
    sal_printf("uIP log message: %s\n", m);
#endif /* CFG_CONSOLE_ENABLED */
}
#endif /* UIP_LOGGING */
