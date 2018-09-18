/*
 * $Id: mdns.h,v 1.10 Broadcom SDK $
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
 
#ifndef _MDNS_H_
#define _MDNS_H_

#include "types.h"
#include "mcu.h"
#include "uip.h"
#include "utils/system.h"



/*
 **********************************************************
 * DNS/MDNS Protocol
 **********************************************************
 */
/* Unicast/Multicast DNS Port */
#define DNS_UNICAST_PORT            (53)
#define DNS_MULTICAST_PORT          (5353)

#define DNS_HDR_LEN                 (12)

#define MDNS_SRC_MAC_OFFSET         (6)

/* DNS Record Types */
#define DNS_TYPE_A                  (1) /* IPv4 address */
#define DNS_TYPE_CNAME              (5) /* Canonical name */
#define DNS_TYPE_PTR                (12) /* Domain name pointer */
#define DNS_TYPE_HINFO              (13) /* host information */
#define DNS_TYPE_TXT                (16) /* text string */
#define DNS_TYPE_AAAA               (28) /* IPv6 address */
#define DNS_TYPE_SRV                (33) /* Service record */
#define DNS_TYPE_NSEC               (47) /* Next-Secure record */
#define DNS_TYPE_ANY                (255) /* Any */


/* DNS Record Classes */
#define DNS_CLASS_IN                (1) /* Internet */
#define DNS_CLASS_ANY               (255) /* Any */

/* Unicast Response bit in Class field of Queries */
#define DNS_UNICAST_RESPONSE        (0x8000)
/* Cache flash bit in Class field of Answers */
#define DNS_CACHE_FLASH             (0x8000)

/* Domain Name Compression */
#define DNS_COMPRESS_FLAG           (0xc0)


/* MDNS timing parameters */
#define MDNS_PROBE_DELAY            (250000) /* 250 ms */
#define MDNS_RESPONSE_DELAY         (500000) /* 500 ms */
#define MDNS_PROBE_NUM              (3)
#define MDNS_MAX_RPOBE_CONFLICT     (15)
#define MDNS_PROBE_PERIOD           (10000000) /* 10 seconds */
#define MDNS_PROBE_CONFLICT_WAIT    (5000000) /* 5 second */
#define MDNS_ANNOUNCE_NUM           (2)
#define MDNS_ANNOUNCE_WAIT          (1000000) /* 1 second */
#define MDNS_TC_DELAY               (400000) /* 400 ms */


/* MDNS timer interval */
#define MDNS_TIMER_INTERVAL         (100000) /* 100000 us */


/* Send Packet Type */
#define MDNS_SEND_NONE_PKT              (0) /* No packet to send out */
#define MDNS_SEND_QUERY_PKT             (1) /* Send Probe packet */
#define MDNS_SEND_RESP_PKT              (2) /* Send Response or Advertisement */
#define MDNS_SEND_GOODBYE_PKT           (3) /* Send Goodbye packet */



/* Resource Record format */
#define DNS_INSTANCE_NAME_MAX_LEN       (64)
#define DNS_SUB_PROTO_NAME_MAX_LEN      (69)
#define DNS_PROTO_NAME_MAX_LEN          (22)
#define DNS_RR_NAME_MAX_LEN             (256)
#define DNS_RR_RDATA_MAX_LEN            (256)


/* DNS record TTL value */
#define DNS_RR_TTL_ADDR                 (120) /* 120 seconds */
#define DNS_RR_TTL_SRV                  (120) /* 120 second */
#define DNS_RR_TTL_PTR                  (4500) /* 1 hour 15 min */
#define DNS_RR_TTL_TXT                  (4500) /* 1 hour 15 min */



/* DNS Header Flags */

#define DNS_HDR_FLAG_QR_MASK            (0x8000) /* query (0) response (1) */
#define DNS_HDR_FLAG_QR_SHIFT           (15)
#define DNS_HDR_FLAG_OPCODE_MASK        (0x7800)  /* opcode */
#define DNS_HDR_FLAG_OPCODE_SHIFT       (11) 
#define DNS_HDR_FLAG_AA_MASK            (0x0400)  /* Authoritative answer */
#define DNS_HDR_FLAG_AA_SHIFT           (10)
#define DNS_HDR_FLAG_TC_MASK            (0x0200)   /* Trucated */
#define DNS_HDR_FLAG_TC_SHIFT           (9)
#define DNS_HDR_FLAG_RC_MASK            (0x000f) /* Response code */
#define DNS_HDR_FLAG_RC_SHIFT           (0)


/* DNS local doamin label length */
#define MDNS_LOCAL_DOMAIN_LEN           (6)
#define MDNS_IPV4_REV_DOMAIN_LEN        (21)
#define MDNS_IPV6_REV_DOMAIN_LEN        (15)
#define MDNS_GENERAL_QUERY_LEN          (30)

#define MDNS_ADVERTISE_IPV4             (1)
#define MDNS_ADVERTISE_IPV6             (2)



/* Service Advertisement State */
typedef enum
{
    MDNS_SRV_STATE_INIT,
    MDNS_SRV_STATE_PROBE,
    MDNS_SRV_STATE_ANNOUNCE,
    MDNS_SRV_STATE_CONFLICT,
    MDNS_SRV_STATE_WAIT,
    MDNS_SRV_STATE_DISABLED,
    MDNS_SRV_STATE_GOODBYE,
    MDNS_SRV_STATE_FAILED
} mdns_srv_state_t;


/* Section Types of DNS Resource Record */
typedef enum
{
    MDNS_PKT_SEC_TYPE_QUEST,
    MDNS_PKT_SEC_TYPE_ANSWER,
    MDNS_PKT_SEC_TYPE_AUTH,
    MDNS_PKT_SEC_TYPE_ADDI
}mdns_pkt_sec_type_t;


/* Resource record format */
typedef struct dns_rr_info_s dns_rr_info_t;
struct dns_rr_info_s
{
    /* basic infomation */
    uint8   *rname;
    uint8   rname_len;
    uint16  rtype;
    uint16  rclass;
    uint32  ttl;
    uint16  rdlength;
    uint8   *rdata;

    /* state diagram */
    BOOL    unique;   
    uint8   send_flags;
    BOOL    enabled; /* indicate this record is valid or not */
    BOOL    peer_is_ip6;
    uint8   peer_ip4[4];
#if CFG_UIP_IPV6_ENABLED    
    uint8   peer_ip6[16];
#endif /* CFG_UIP_IPV6_ENABLED */

    
};

/* Resource name status */
typedef struct mdns_naming_state_s mdns_naming_state_t;
struct mdns_naming_state_s
{
    mdns_srv_state_t    state;
    uint8               conflict_count;
    tick_t              first_probe;
    tick_t              last_probe;
    tick_t              probe_interval;
    tick_t              last_ad;
    tick_t              ad_interval;
    uint8               probe_count;
    uint8               ad_count;
    
};



typedef struct dns_service_info_s dns_service_info_t;
struct dns_service_info_s
{
    uint8               name[DNS_RR_NAME_MAX_LEN];
    uint8               name_len;
    uint8               instance_len;
    uint8               sub_proto_len;
    uint8               proto_len;
    uint8               domain_len;

    mdns_naming_state_t     name_ipv4;
#if CFG_UIP_IPV6_ENABLED
    mdns_naming_state_t     name_ipv6;
#endif /* CFG_UIP_IPV6_ENABLED */

    dns_rr_info_t           *srv_rr;
    dns_rr_info_t           *ptr_rr;
    dns_rr_info_t           *txt_rr;
    dns_rr_info_t           *dns_rr;

    dns_service_info_t      *next;
    
};



typedef struct mdns_reply_answer_s mdns_reply_answer_t;
struct mdns_reply_answer_s
{
    BOOL                queried; /* queried by current query packet */
    
    /* record info */
    dns_rr_info_t       *asnwer_rr;
    uint8               send_flags;
    tick_t              start;
    tick_t              delay;
    uint8               peer_ip[16];

    mdns_reply_answer_t      *next;
    
};



typedef struct mdns_state mdns_state_t;
struct mdns_state
{
    BOOL        enable;
    uint8       state;
    uint8       iconfig;
    uint8       sys_ip[4];
#if CFG_UIP_IPV6_ENABLED
    uint8       sys_ip6[16];
#endif /* CFG_UIP_IPV6_ENABLED */
    uint8       sys_netmask[4];
    uint8       sys_gateway[4];
    uint8       sys_mac[6];
#if CFG_UIP_IPV6_ENABLED
#endif /* CFG_UIP_IPV6_ENABLED */
    uint8       sys_name[MAX_SYSTEM_NAME_LEN];
    uint8       sys_name_len;
    struct      uip_udp_conn *conn;
#if CFG_UIP_IPV6_ENABLED
    struct      uip_udp_conn *conn_v6;
#endif /* CFG_UIP_IPV6_ENABLED */

    dns_service_info_t    *srv_list;
    

    /* Device address record */
    uint8       host_dn_len; /* total length inlucude length bytes */
    uint8       host_name_len; /* only host name length include length byte */
    uint8       domain_len;
    uint8       host_dn[DNS_RR_NAME_MAX_LEN];

    mdns_naming_state_t     name_ipv4;
#if CFG_UIP_IPV6_ENABLED
    mdns_naming_state_t     name_ipv6;
#endif /* CFG_UIP_IPV6_ENABLED */
    
    dns_rr_info_t     addr_rr; /* IPv4 */
    dns_rr_info_t     addr6_rr; /* IPv6 */

    
    /* schedule the query and response time */
    uint8       send_pkt_type;
    tick_t      send_delay;


    /* send packet info */
    BOOL        unicast_pkt_send;
    BOOL        multicast_pkt_send;
    uint8       dst_mac[6];
    uint8       dst_ip4[4];
    uint8       dst_ip6[16];

    /* MDNS probe timer */
    BOOL        probe_timer_enable;

    mdns_reply_answer_t     *reply_list_ipv4;
#if CFG_UIP_IPV6_ENABLED
    mdns_reply_answer_t     *reply_list_ipv6;
#endif /* CFG_UIP_IPV6_ENABLED */
    
};



/* DNS header */
struct dns_hdr {  
  uint16    dns_id;
  uint16    dns_flag;
  uint16    qdcount;
  uint16    ancount;
  uint16    nscount; 
  uint16    arcount;
};


#define MDNS_IS_UPPER_CASE(a)       ((a) >= 'A' && (a) <= 'Z')
#define MDNS_IS_LOWER_CASE(a)       ((a) >= 'a' && (a) <= 'z')
#define MDNS_IS_DIGIT(a)            ((a) >= '0' && (a) <= '9')


#define MDNS_RR_IS_UNIQUE(rr)       (rr->unique == FALSE)

/* MDNS packet flags */
#define MDNS_PKT_FLAG_UNICAST       (0x1) /* bit 0 : 0->mcast 1->unicast */
#define MDNS_PKT_FLAG_RESPONSE      (0x2) /* bit 1 : 0->query 1->response */
#define MDNS_PKT_FLAG_ADDR_RECORD   (0x4) /* bit 2 : 0->address 1->service */    
#define MDNS_PKT_FLAG_DNS_GQUERY    (0x8) /* bit3 : indicates general dns query */
#define MDNS_PKT_FLAG_NONEXIST      (0x10) /* bit4 : indicates record nonexist */


/* DNS Resource Record Send Flags */
#define DNS_RR_SEND_FLAG_UNICAST            (0x1)
#define DNS_RR_SEND_FLAG_IN_QUEST           (0x2) /* Put in question section */
#define DNS_RR_SEND_FLAG_IN_ANSWER          (0x4) /* Put in answer section */
#define DNS_RR_SEND_FLAG_IN_AUTH            (0x8) /* Put in auth section */
#define DNS_RR_SEND_FLAG_IN_ADDI            (0x10) /* Put in additional section */
#define DNS_RR_SEND_FLAG_NSEC               (0x20) /* Put NSEC record */
#define DNS_RR_SEND_FLAG_REPLY_MASK         ( \
    DNS_RR_SEND_FLAG_IN_ANSWER | DNS_RR_SEND_FLAG_IN_AUTH | \
    DNS_RR_SEND_FLAG_IN_ADDI | DNS_RR_SEND_FLAG_NSEC)




/* Prototypes */
extern void mdns_packet_out(void);
extern void mdns_packet_in(void);

extern void mdns_init(void);
extern void mdns_start(void);
#if CFG_UIP_IPV6_ENABLED
extern void mdns_start_ipv6(uip_ip6addr_t *ipv6_addr);
#endif /* CFG_UIP_IPV6_ENABLED */
extern void mdns_appcall(void);

extern void mdns_host_domain_name_set(void);
extern void mdns_http_service_txt_rdata_build(dns_rr_info_t *txt_rr);



/* Utilities */
extern int8 mdns_rdata_cmp(uint16 rd1_len, uint8 *rdata1, uint16 rd2_len, uint8 *rdata2);
extern BOOL mdns_same_domain(const uint8 *name1, const uint8 *name2, uint16 len);
extern BOOL mdns_local_domain_check(uint8 *name);
extern sys_error_t mdns_string_shift_down(uint8 *str, uint8 name_start, 
            uint8 name_len, uint8 movement);
extern sys_error_t mdns_string_shift_up(uint8 *str, uint8 name_start, 
            uint8 name_len, uint8 movement);

extern sys_error_t mdns_convert_string2label(const char *name, uint8 *domain_label);

#ifndef __BOOTLOADER__
extern void mdns_bonjour_enable_set(BOOL enable);
extern void mdns_bonjour_enable_get(BOOL *enable);
#endif /* __BOOTLOADER__ */



#endif /* _MDNS_H_ */
