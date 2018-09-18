
/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 * Header file for the uIP TCP/IP stack.
 * \author  Adam Dunkels <adam@dunkels.com>
 * \author  Julien Abeille <jabeille@cisco.com> (IPv6 related code)
 * \author  Mathilde Durvy <mdurvy@cisco.com> (IPv6 related code)
 *
 * The uIP TCP/IP stack header file contains definitions for a number
 * of C macros that are used by uIP programs as well as internal uIP
 * structures, TCP/IP header structures and function declarations.
 *
 */

/*
 * Copyright (c) 2001-2003, Adam Dunkels.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: uip6.h,v 1.2 Broadcom SDK $
 *
 */

#if CFG_UIP_IPV6_ENABLED

#if !UIP_CONF_IPV6

/*---------------------------------------------------------------------------*/
/* #if !UIP_CONF_IPV6:
 * This file is not included by a C file for IPv6 TCP/IP stack.
 * In this case, IPv6 definitions required for user applications are provided
 * here.
 */
 
#ifdef __UIP6_H__
/* A trick to source the definitions only when included for the 2nd time */
#ifndef __UIP6_H2__
#define __UIP6_H2__

/* Header sizes. */
#define UIP6_IPH_LEN     40
#define UIP6_LLIPH_LEN   (UIP_LLH_LEN + UIP6_IPH_LEN)    /* L2 + IP header */
#define UIP6_IPICMPH_LEN (UIP6_IPH_LEN + UIP_ICMPH_LEN)  /* ICMP + IP header */
#define UIP6_IPUDPH_LEN  (UIP_UDPH_LEN + UIP6_IPH_LEN)   /* IP + UDP header */
#define UIP6_IPTCPH_LEN  (UIP_TCPH_LEN + UIP6_IPH_LEN)   /* IP + TCP header */
#define UIP6_TCPIP_HLEN  UIP_IPTCPH_LEN                  /* IP + TCP header */

/* The IP header */
struct uip6_ip_hdr {
  /* IPV6 header */
  u8_t vtc;
  u8_t tcflow;
  u16_t flow;
  u8_t len[2];
  u8_t proto, ttl;
  uip_ip6addr_t srcipaddr, destipaddr;
};

/* The TCP and IP headers. */
struct uip6_tcpip_hdr {
  /* IPv6 header. */
  u8_t vtc,
    tcflow;
  u16_t flow;
  u8_t len[2];
  u8_t proto, ttl;
  uip_ip6addr_t srcipaddr, destipaddr;
  
  /* TCP header. */
  u16_t srcport,
    destport;
  u8_t seqno[4],
    ackno[4],
    tcpoffset,
    flags,
    wnd[2];
  u16_t tcpchksum;
  u8_t urgp[2];
  u8_t optdata[4];
};

/* The UDP and IP headers. */
struct uip6_udpip_hdr {
  /* IPv6 header. */
  u8_t vtc,
    tcf;
  u16_t flow;
  u8_t len[2];
  u8_t proto, ttl;
  uip_ip6addr_t srcipaddr, destipaddr;
  
  /* UDP header. */
  u16_t srcport,
    destport;
  u16_t udplen;
  u16_t udpchksum;
};

/**
 * Send data on the current connection.
 */
extern void uip6_send(const void *data_p, int len);

/**
 * Set up a new UDP connection.
 */
extern struct uip_udp_conn *uip6_udp_new(const uip_ip6addr_t *ripaddr, u16_t rport);

/**
 * uIP initialization function.
 *
 * This function should be called at boot up to initilize the uIP
 * TCP/IP stack.
 */
extern void uip6_init(void);

/* uip6_process(flag):
 *
 * The actual uIP function which does all the work.
 */
extern void uip6_process(u8_t flag);
#define uip6_input()        uip6_process(UIP_DATA)

/* *
 * Resolve link layer address by IPv6 address (before sending out)
 */
/** \brief 802.3 address */
extern u16_t uip_ds6_resolve_lladdr(uip_ip6addr_t *ipaddr, uip_lladdr_t *lladdr);

#endif /* !__UIP6_H2__ */
#else /* __UIP6_H__ */
#define __UIP6_H__
#endif /* !__UIP6_H__ */

#else /* UIP_CONF_IPV6 */

/*---------------------------------------------------------------------------*/
/* #if UIP_CONF_IPV6:
 * This file is included by a C file for IPv6 TCP/IP stack.
 * In this case, shared symbols are re-defined for IPv6 stack to avoid linking
 * problems (of duplicated symbols) for dual stack.
 */

#ifndef __UIP6_H__
#define __UIP6_H__

/* Functions */
#define uip_init            uip6_init
#define uip_process         uip6_process
#define uip_send            uip6_send
#define uip_udp_new         uip6_udp_new

#endif /* __UIP6_H__ */

#endif /* UIP_CONF_IPV6 */
#endif /* CFG_UIP_IPV6_ENABLED */

/** @} */

