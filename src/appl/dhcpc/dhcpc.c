/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 */

/*
 * $Id: dhcpc.c,v 1.23 Broadcom SDK $
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
 */

#include "system.h"

#ifdef CFG_DHCPC_INCLUDED

#include "uip.h"
#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
#include "uip_arp.h"
#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */
#include "appl/dhcpc.h"
#include "utils/net.h"

#define DHCPC_DEBUG   0

#if DHCPC_DEBUG
#define DHCPC_DBG(x)  do { sal_printf("DHCPC: "); sal_printf x; \
                          sal_printf("\n"); } while(0);
#else
#define DHCPC_DBG(x)
#endif /* DHCPC_DEBUG */

#define STATE_INITIAL          0
#define STATE_SELECTING        1
#define STATE_REQUESTING       2
#define STATE_BOUND            3
#define STATE_RENEWING         4
#define STATE_REBINDING        5
#define STATE_DEFAULT_CONFIG   6

static struct dhcpc_state s;

struct dhcp_msg {
  u8_t op, htype, hlen, hops;
  u8_t xid[4];
  u16_t secs, flags;
  u8_t ciaddr[4];
  u8_t yiaddr[4];
  u8_t siaddr[4];
  u8_t giaddr[4];
  u8_t chaddr[16];
#ifndef UIP_CONF_DHCP_LIGHT
  u8_t sname[64];
  u8_t file[128];
#endif
  u8_t options[312];
};

#define BOOTP_BROADCAST 0x8000

#define DHCP_REQUEST        1
#define DHCP_REPLY          2
#define DHCP_HTYPE_ETHERNET 1
#define DHCP_HLEN_ETHERNET  6
#define DHCP_MSG_LEN      236

#define DHCPDISCOVER  1
#define DHCPOFFER     2
#define DHCPREQUEST   3
#define DHCPDECLINE   4
#define DHCPACK       5
#define DHCPNAK       6
#define DHCPRELEASE   7

#define DHCP_OPTION_SUBNET_MASK   1
#define DHCP_OPTION_ROUTER        3
#define DHCP_OPTION_DNS_SERVER    6
#define DHCP_OPTION_REQ_IPADDR   50
#define DHCP_OPTION_LEASE_TIME   51
#define DHCP_OPTION_MSG_TYPE     53
#define DHCP_OPTION_SERVER_ID    54
#define DHCP_OPTION_REQ_LIST     55
#define DHCP_OPTION_RENEWAL_TIME    58
#define DHCP_OPTION_REBINDING_TIME  59
#define DHCP_OPTION_END         255

#define DHCP_RETRY_MAX           15
#define DHCP_MAC_ADDR_LENGTH      6

/* For code size optimization */
#define m ((struct dhcp_msg *)uip_appdata)

static const u8_t CODE xid[4] = {0xad, 0xde, 0x12, 0x23};
static const u8_t CODE magic_cookie[4] = {99, 130, 83, 99};
static u8_t  retry_count;
/*---------------------------------------------------------------------------*/
#define add_msg_type(optptr, type) do {         \
    *optptr++ = DHCP_OPTION_MSG_TYPE;           \
    *optptr++ = 1;                              \
    *optptr++ = type;                           \
} while(0)
/*---------------------------------------------------------------------------*/
#define add_option(optptr, opt, buf, len) do {  \
    *optptr++ = opt;                            \
    *optptr++ = len;                            \
    sal_memcpy(optptr, buf, len);               \
    optptr += len;                              \
} while(0)
/*---------------------------------------------------------------------------*/
#define add_req_options(optptr) do {            \
    *optptr++ = DHCP_OPTION_REQ_LIST;           \
    *optptr++ = 3;                              \
    *optptr++ = DHCP_OPTION_SUBNET_MASK;        \
    *optptr++ = DHCP_OPTION_ROUTER;             \
    *optptr++ = DHCP_OPTION_DNS_SERVER;         \
} while(0)
/*---------------------------------------------------------------------------*/
#define add_end(optptr) do {                    \
    *optptr++ = DHCP_OPTION_END;                \
} while(0)
/*---------------------------------------------------------------------------*/
static void
create_msg(void)
{
    static const uint8 CODE template[] = {
        DHCP_REQUEST,
        DHCP_HTYPE_ETHERNET,
        DHCP_MAC_ADDR_LENGTH,
        0,
        0xad, 0xde, 0x12, 0x23,
        0x00, 0x00,
    };

    sal_memcpy(m, template, sizeof(template));
    sal_memset(((uint8 *)m) + sizeof(template),
               0,
               sizeof(struct dhcp_msg) - sizeof(template)
               );
    if (s.state != STATE_RENEWING) {
        m->flags = UIP_HTONS(BOOTP_BROADCAST); /*  Broadcast bit. */
    } else {
        m->flags = 0x0; /* Let server unicasts DHCPACK message to client */
    }
    sal_memcpy(m->ciaddr, &uip_hostaddr, sizeof(m->ciaddr));
    sal_memcpy(m->chaddr, s.mac_addr, DHCP_MAC_ADDR_LENGTH);
    sal_memcpy(m->options, magic_cookie, sizeof(magic_cookie));
}

static void
send_discover(void)
{
    const uint8 CODE discover_options[] = {
        /* msg type */
        DHCP_OPTION_MSG_TYPE,
        1,
        DHCPDISCOVER,

        /* request list */
        DHCP_OPTION_REQ_LIST, 3,
        DHCP_OPTION_SUBNET_MASK,
        DHCP_OPTION_ROUTER,
        DHCP_OPTION_DNS_SERVER,

        /* end of options */
        DHCP_OPTION_END
    };

    create_msg();
    sal_memcpy(&m->options[4], discover_options, sizeof(discover_options));

    uip_send(uip_appdata,
             &m->options[4] + sizeof(discover_options) - (u8_t *)uip_appdata);
}

static void
send_request(void)
{
    u8_t *end = &m->options[4];

    create_msg();

    add_msg_type(end, DHCPREQUEST);
    if (s.state == STATE_REQUESTING) {
        /* Server ID and requested IP MUST NOT in request packet
           for RENEWING and REBINDING state */
        add_option(end, DHCP_OPTION_SERVER_ID, s.serverid, 4);
        add_option(end, DHCP_OPTION_REQ_IPADDR, s.ipaddr, 4);
    }
    add_end(end);

    uip_send(uip_appdata, end - (u8_t *)uip_appdata);
}

static u8_t
parse_options(u8_t *optptr, int len)
{
    u8_t *end = optptr + len;
    u8_t type = 0;

    while(optptr < end) {
        switch(*optptr) {
        case DHCP_OPTION_SUBNET_MASK:
          sal_memcpy(s.netmask, optptr + 2, 4);
          break;
        case DHCP_OPTION_ROUTER:
          sal_memcpy(s.default_router, optptr + 2, 4);
          break;
        case DHCP_OPTION_DNS_SERVER:
          sal_memcpy(s.dnsaddr, optptr + 2, 4);
          break;
        case DHCP_OPTION_MSG_TYPE:
          type = *(optptr + 2);
          break;
        case DHCP_OPTION_SERVER_ID:
          sal_memcpy(s.serverid, optptr + 2, 4);
          break;
        case DHCP_OPTION_LEASE_TIME:
          sal_memcpy(s.lease_time, optptr + 2, 4);
          break;
        case DHCP_OPTION_RENEWAL_TIME:
          sal_memcpy(s.t1_time, optptr + 2, 4);
          break;
        case DHCP_OPTION_REBINDING_TIME:
          sal_memcpy(s.t2_time, optptr + 2, 4);
          break;
        case DHCP_OPTION_END:
#if DHCPC_DEBUG
          /* Re-define time in second for debug only */
          s.t1_time[0] = 0;
          s.t1_time[1] = 0;
          s.t2_time[0] = 0;
          s.t2_time[1] = 0;
          s.lease_time[0] = 0;
          s.lease_time[1] = UIP_HTONS(240);
#endif /* DHCPC_DEBUG */
          return type;
        }

        optptr += optptr[1] + 2;
    }
    return type;
}

static u8_t
parse_msg(void)
{
    if(m->op == DHCP_REPLY &&
        sal_memcmp(m->xid, xid, sizeof(xid)) == 0 &&
        sal_memcmp(m->chaddr, s.mac_addr, DHCP_MAC_ADDR_LENGTH) == 0) {
        sal_memcpy(s.ipaddr, m->yiaddr, 4);
        return parse_options(&m->options[4], uip_datalen());
    }
    return 0;
}

void
dhcpc_init(const void *mac_addr)
{
    uip_ipaddr_t addr;

    s.mac_len  = DHCP_MAC_ADDR_LENGTH;
    sal_memcpy(s.mac_addr, mac_addr, DHCP_MAC_ADDR_LENGTH);
    /* Set interval to 4 seconds in ticks */
    s.interval_tick = (1000000UL / sal_get_us_per_tick()) * 4;
    s.state = STATE_INITIAL;
    s.renew = FALSE;
    uip_ipaddr(&addr, 255, 255, 255, 255);
    s.conn = uip_udp_new(&addr, UIP_HTONS(DHCPC_SERVER_PORT));
    if(s.conn != NULL) {
        uip_udp_bind(s.conn, UIP_HTONS(DHCPC_CLIENT_PORT));
    }
}

void
dhcpc_appcall(void)
{
    uip_ipaddr_t addr;

    if (!s.renew) {
        if ((s.state == STATE_DEFAULT_CONFIG) || (s.state == STATE_INITIAL)) {
            return;
        }

        if ((s.state == STATE_BOUND) &&
            (SAL_TIME_EXPIRED_IN_SECOND(s.start_lease, s.lease_interval))) {
            s.state = STATE_RENEWING;
            uip_ipaddr_copy((uip_ipaddr_t *)&s.conn->ripaddr, (uip_ipaddr_t *)s.server_ip);
            send_request();
            s.send_request = sal_get_seconds();
            s.start_tick = sal_get_ticks();
            retry_count = 1;
            s.renew = TRUE;
            DHCPC_DBG(("STATE_BOUND -> STATE_RENEWING : send request "
                       "at %d seconds",
                       s.send_request));
        } else if ((s.state == STATE_RENEWING) &&
            (SAL_TIME_EXPIRED_IN_SECOND(s.start_lease, s.lease_interval))) {
            s.state = STATE_REBINDING;
            uip_ipaddr(&addr, 255,255,255,255);
            uip_ipaddr_copy((uip_ipaddr_t *)&s.conn->ripaddr, &addr);
            send_request();
            s.send_request = sal_get_seconds();
            s.start_tick = sal_get_ticks();
            retry_count = 1;
            s.renew = TRUE;
            DHCPC_DBG(("STATE_RENEWING -> STATE_REBINDING : send request "
                       "at %d seconds",
                       s.send_request));
        } else if ((s.state == STATE_REBINDING) &&
            (SAL_TIME_EXPIRED_IN_SECOND(s.start_lease, s.lease_interval))) {
            s.state = STATE_DEFAULT_CONFIG;
#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
            /* Start auto ip configuration */
            uip_autoip_start();
#else /* !CFG_ZEROCONF_AUTOIP_INCLUDED */
            net_dhcp_configured((const uint8 *)default_ip_addr,
                                (const uint8 *)default_netmask,
                                (const uint8 *)default_gateway);
#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */
            DHCPC_DBG(("STATE_REBINDING -> STATE_DEFAULT_CONFIG at %d seconds",
                       sal_get_seconds()));
        }
        return;
    }

    if (s.state == STATE_INITIAL) {
        uip_ipaddr(&addr, 255,255,255,255);
        uip_ipaddr_copy((uip_ipaddr_t *)&s.conn->ripaddr, &addr);
        send_discover();
        s.start_tick = sal_get_ticks();
        retry_count = 1;
        s.state = STATE_SELECTING;
        DHCPC_DBG(("STATE_INITIAL -> STATE_SELECTING : send discover"));
        return;
    }

    if (uip_newdata()) {
        if (s.state == STATE_SELECTING && parse_msg() == DHCPOFFER) {
            s.state = STATE_REQUESTING;
            send_request();
            s.send_request = sal_get_seconds();
            s.start_tick = sal_get_ticks();
            retry_count = 1;
            DHCPC_DBG(("STATE_SELECTING -> STATE_REQUESTING : "
                       "receive offer and send request"));
        } else if (((s.state == STATE_REQUESTING) ||
                    (s.state == STATE_RENEWING) ||
                    (s.state == STATE_REBINDING)) &&
                   (parse_msg() == DHCPACK)) {
            if (s.state == STATE_REQUESTING) {
                /* Update DHCP server IP for sending unicast renewal */
                sal_memcpy(s.server_ip, s.serverid, 4);
                DHCPC_DBG(("STATE_REQUESTING -> STATE_BOUND : receive ACK "
                           "from server_ip[0]=0x%x server_ip[1]=0x%x",
                           s.server_ip[0], s.server_ip[1]));
                net_dhcp_configured((const uint8 *)s.ipaddr,
                            (const uint8 *)s.netmask,
                            (const uint8 *)s.default_router);
            }
            s.state = STATE_BOUND;
            s.renew = FALSE;
            if ((s.t1_time[0] == 0x0) && (s.t1_time[1] == 0x0)) {
                /* If DHCP server doesn't provide T1 time,
                   use 1/2 * lease time as T1 */
                s.lease_interval = (UIP_HTONS(s.lease_time[0]) * 65536UL +
                                    UIP_HTONS(s.lease_time[1])) / 2;
            } else {
                s.lease_interval = UIP_HTONS(s.t1_time[0]) * 65536UL +
                                   UIP_HTONS(s.t1_time[1]);
            }
            s.start_lease = s.send_request;
            DHCPC_DBG(("-> STATE_BOUND : receive ACK with lease_time[0]=0x%x "
                       "lease_time[1]=0x%x t1_time[0]=0x%x t1_time[1]=0x%x "
                       "t2_time[0]=0x%x t2_time[1]=0x%x",
                       s.lease_time[0], s.lease_time[1], s.t1_time[0],
                       s.t1_time[1], s.t2_time[0], s.t2_time[1]));
            DHCPC_DBG(("-> STATE_BOUND : start lease at %d seconds with "
                       "t1 %d seconds",
                       s.start_lease, s.lease_interval));
        }
    } else {
        /* Timer */
        if (SAL_TIME_EXPIRED(s.start_tick, s.interval_tick)) {
            if ((s.state == STATE_SELECTING) ||
                (s.state == STATE_REQUESTING)) {
                if (retry_count >= DHCP_RETRY_MAX) {
                    s.state = STATE_DEFAULT_CONFIG;
#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
                    /* Start auto ip configuration */
                    uip_autoip_start();
#else /* !CFG_ZEROCONF_AUTOIP_INCLUDED */
                    net_dhcp_configured((const uint8 *)default_ip_addr,
                                        (const uint8 *)default_netmask,
                                        (const uint8 *)default_gateway);
#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */
                    s.renew = FALSE;
                    DHCPC_DBG(("-> STATE_DEFAULT_CONFIG"));
                    return;
                }

                if(s.state == STATE_SELECTING) {
                    send_discover();
                    DHCPC_DBG(("STATE_SELECTING : re-send discover %d retry",
                               retry_count));
                } else {
                    send_request();
                    s.send_request = sal_get_seconds();
                    DHCPC_DBG(("STATE_REQUESTING : re-send request %d retry",
                                retry_count));
                }
            } else if (s.state == STATE_RENEWING) {
                if (retry_count >= DHCP_RETRY_MAX) {
                    if ((s.t2_time[0] == 0x0) && (s.t2_time[1] == 0x0)) {
                        /* If DHCP server doesn't provide T2 time,
                           use 7/8 * lease time as T2 */
                        s.lease_interval = (UIP_HTONS(s.lease_time[0]) * 65536UL +
                                            UIP_HTONS(s.lease_time[1])) / 8 * 7;
                    } else {
                        s.lease_interval = UIP_HTONS(s.t2_time[0]) * 65536UL +
                                           UIP_HTONS(s.t2_time[1]);
                    }
                    s.renew = FALSE;
                    return;
                }

                send_request();
                s.send_request = sal_get_seconds();
                DHCPC_DBG(("STATE_RENEWING : re-send request %d retry",
                            retry_count));
            } else if (s.state == STATE_REBINDING) {
                if (retry_count >= DHCP_RETRY_MAX) {
                    s.lease_interval = UIP_HTONS(s.lease_time[0]) * 65536UL +
                                       UIP_HTONS(s.lease_time[1]);
                    s.renew = FALSE;
                    return;
                }

                send_request();
                s.send_request = sal_get_seconds();
                DHCPC_DBG(("STATE_REBINDING : re-send request %d retry",
                            retry_count));
            }

            s.start_tick = sal_get_ticks();
            retry_count++;
        }
    }
}

void dhcpc_renew(void)
{
    s.renew = TRUE;
    s.state = STATE_INITIAL;
    sal_memset(s.t1_time, 0x0, sizeof(u16_t) * 2);
    sal_memset(s.t2_time, 0x0, sizeof(u16_t) * 2);
}

#endif /* CFG_DHCPC_INCLUDED */

