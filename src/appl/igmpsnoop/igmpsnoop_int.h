/*
 * $Id: igmpsnoop_int.h,v 1.6 Broadcom SDK $
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

#ifndef _IGMPSNOOP_INT_H_
#define _IGMPSNOOP_INT_H_

/*
 * Constant definitions
 */
#define IGMPv2_QUERY       0x11
#define IGMPv1_REPORT      0x12
#define IGMPv2_REPORT      0x16
#define IGMPv2_LEAVE       0x17
#define IGMP_GeneralQUERY_GDA  0x0  /* GDA for General Query is all zero */

#define IGMP_PROTOCOL_NUMBER    2  /* Internet Protocol Numbers */
#define NUM_IGMPgrp_PER_SYSTEM  128
#define MAX_IGMPleave           128
#define VID_BITS_MASK      0x0fff
#if CFG_BIG_ENDIAN
#define GDA_MASK       0x007fffffL     /* bid endian */
#define IGMPDB_MASK        0xbfffffffL
#else
#define GDA_MASK       0xffff7f00L     /* little endian */
#define IGMPDB_MASK        0xffffffbfL
#endif /* ENDIAN_BIG */

#define IGMP_DFT_ROBUSTNESS            2
#define IGMP_DFT_QUERY_INTERVAL        125
#define IGMP_DFT_QUERY_RESPONSE_INTERVAL   10
#define IGMP_DFT_LAST_MEMBER_QUERY_INTERVAL    1
#define IGMP_DFT_LAST_MEMBER_QUERY_COUNT   IGMP_DFT_ROBUSTNESS

/* IGMP_TIMER_PARAMTER_UNCHANGEABLE definition is used to reduce the 
 *  usage of the public variables(i.e. XDATA in 8051 solution).
 */
#define IGMP_TIMER_PARAMTER_UNCHANGEABLE
#ifndef IGMP_TIMER_PARAMTER_UNCHANGEABLE
#define IGMP_group_membership_interval \
             (IGMPparm_robustness * IGMPparm_query_interval + \
              IGMPparm_query_response)
#define IGMP_SpecQUERY_TIMEOUT \
             (IGMPparm_last_member_query_count * \
              IGMPparm_last_member_query_interval)
#else   /* IGMP_TIMER_PARAMTER_UNCHANGEABLE */
#define IGMP_group_membership_interval \
             (IGMP_DFT_ROBUSTNESS * IGMP_DFT_QUERY_INTERVAL + \
              IGMP_DFT_QUERY_RESPONSE_INTERVAL)
#define IGMP_SpecQUERY_TIMEOUT \
             (IGMP_DFT_LAST_MEMBER_QUERY_COUNT * \
              IGMP_DFT_LAST_MEMBER_QUERY_INTERVAL)
#endif  /* IGMP_TIMER_PARAMTER_UNCHANGEABLE */

/*
 * Type definitions
 */

/* The IGMPv2 and IP headers. */
typedef struct igmpsnoop_igmp_hdr_s{
    /* IP header. */
    /*
        uint8 vhl, tos, len[2], ipid[2], ipoffset[2], ttl, proto;
        uint16 ipchksum;
        uint16 srcipaddr[2], destipaddr[2];
    */
    /* IGMP header. */
    uint8   type;
    uint8   mrt;
    uint16  chksum;
    uint32  group;
} igmpsnoop_igmp_hdr_t;

typedef struct igmpsnoop_ip_hdr_s {
    uint8             verHlen;        /* IP packet start */
    uint8             serviceType;
    uint16            totalLen;
    uint16            identification;
    uint16            flagFragOffset;
    uint8             ttl;
    uint8             protocol;
    uint16            headerChksum;
    uint8             srcIp[4];
    uint8             desIp[4];    
} igmpsnoop_ip_hdr_t;

typedef struct igmpdb_s {
    uint32  gda;
    uint16  vid;
} igmpdb_t;

typedef  struct igmpLeave_s {
    uint32  gda;
    uint16  vid;
    uint16  uport;
    uint8   timeout;
    uint8   next;
} igmpLeave_t;

typedef struct igmpsnoop_router_port_s{
    uint16  vid;
    uint8   age[MAX_UPLIST_WIDTH];
    uint8   uplist[MAX_UPLIST_WIDTH];
} igmpsnoop_router_port_t;

typedef struct {
    uint8   age[MAX_UPLIST_WIDTH];
    uint8   uplist[MAX_UPLIST_WIDTH];
} IPMCuplist_t;

/*
 * Macros
 */
#define BCM_PKT_VLAN_ID(e) (HTON16((e)->en_tag_ctrl) & VID_BITS_MASK)
#define is_valid(x)      (*((uint8*)&igmpDB[(x)]) & 0x80)
#define is_invalid(x)    (!(*((uint8*)&igmpDB[(x)]) & 0x80))
#define is_pioneer(x)    (*((uint8*)&igmpDB[(x)]) & 0x40)
#define is_follower(x)   (!(*((uint8*)&igmpDB[(x)]) & 0x40))
#define are_pals(x, y) \
         (((igmpDB[(x)].gda & GDA_MASK) == (igmpDB[(y)].gda & GDA_MASK)) && \
           ((igmpDB[(x)].vid & VID_BITS_MASK) == \
            (igmpDB[(y)].vid & VID_BITS_MASK))) && \
          ((x) != (y))
#define are_pals2(x, y) \
         (((igmpDB[(x)].gda & GDA_MASK) == (igmpDB[(y)].gda & GDA_MASK)) && \
          ((igmpDB[(x)].vid & VID_BITS_MASK) == \
           (igmpDB[(y)].vid & VID_BITS_MASK)))


/*
 * Shared variables
 */
extern const uint8 CODE all_router_ip[4];
extern BOOL IGMPsnoop_enable;
extern igmpdb_t igmpDB[NUM_IGMPgrp_PER_SYSTEM];
extern uint16 igmpLeaveQ;
extern igmpLeave_t igmpLV[MAX_IGMPleave];
extern uint16 igmpsnoop_tick;
extern uint16 router_port_tick;
extern uint16 IGMPparm_router_port_interval;
extern igmp_dbg_count_t igmp_dbg_count;
extern igmpsnoop_router_port_t dynamic_router[BOARD_MAX_NUM_OF_QVLANS];
extern IPMCuplist_t IPMCuplist[NUM_IGMPgrp_PER_SYSTEM];

/*
 * Functions
 */
extern igmpsnoop_ip_hdr_t *locate_ip_pkt(uint8 *eth_pkt);
extern uint8 *locate_igmp_pkt(uint8 *eth_pkt);
extern uint16 igmpDB_search(uint16 vid, uint32 gda);
extern uint8 igmpDB_new(void);
extern BOOL igmpDB_update(uint16 vid, uint32 gda, uint8 *uplist);
extern void IPMCuplist_summ_reset(void);
extern void IPMCuplist_summ_uplist(uint16 index);
extern void IPMCuplist_summ_age(uint16 index);
extern void IPMCuplist_add(uint16 index, uint8 *uplist);
extern void IPMCuplist_remove(uint16 index, uint16 uport);
extern void IPMCuplist_wipe(uint16 index);
extern BOOL IPMCuplist_nil_uplist(uint16 index);
extern BOOL IPMCuplist_nil_age(uint16 index);
extern void IPMCuplist_copy(uint16 i, uint16 j);
extern void IPMCuplist_aging(uint16 index);
extern BOOL IPMCuplist_mob(uint16 i, uint16 j, uint16 uport);
#define     L2MCportbmp_NEW         1       
#define     L2MCportbmp_ADD         2
#define     L2MCportbmp_DEL         3
#define     L2MCportbmp_CAST        4
#define     router_uplist_UPDATE    8
extern BOOL L2MCportbmp_update(uint16 index, uint8 *uplist, uint8 op);
extern void L2MCportbmp_remove(uint16 index, uint16 uport);
extern uint8 igmpLeaveP_new(void);
extern void igmpLeaveP_free(uint16 p);
extern uint16 igmpLeaveP_search(uint16 vid, uint32 gda, 
                                uint16 uport, uint16 *prev);
extern BOOL igmpLeaveQ_grant(uint16 vid, uint32 gda);
extern BOOL igmpLeaveQ_insert(uint16 vid, uint32 gda, uint8 *uplist);
extern BOOL igmpLeaveQ_delete(uint16 vid, uint32 gda, uint8 *uplist);
extern void igmpsnoop_IPMCentry_remove(uint16 entry_num);
extern BOOL igmpsnoop_router_port_add(uint16 vid, uint16 uport);
extern BOOL igmpsnoop_router_port_age(void);
extern void compose_mc_mac(uint32 gda, uint8 *mac);
extern BOOL irrigate(sys_pkt_t *pkt, uint32 igmp_group, uint16 vid,
             uint8 igmp_type, uint8 *uplist, uint8 *t_uplist) REENTRANT;
extern sys_rx_t igmpsnoop_rx(sys_pkt_t *pkt, void *cookie) REENTRANT;
extern void igmpsnoop_one_sec_timer(void *arg) REENTRANT;

#endif /* _IGMPSNOOP_INT_H_ */
