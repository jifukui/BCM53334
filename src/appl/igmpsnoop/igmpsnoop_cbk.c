/*
 * $Id: igmpsnoop_cbk.c,v 1.14 Broadcom SDK $
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
#include "appl/igmpsnoop.h"
#include "utils/ports.h"
#include "utils/net.h"
#include "igmpsnoop_int.h"

#if defined(CFG_SWITCH_MCAST_INCLUDED)

#define IGMPSNOOP_DEBUG   0

#if IGMPSNOOP_DEBUG
#define IGMPSNOOP_DBG(x)  do { sal_printf("IGMPSNOOP: "); sal_printf x; \
                          sal_printf("\n"); } while(0);
#else
#define IGMPSNOOP_DBG(x)
#endif

void
compose_mc_mac(uint32 gda, uint8 *mac)
{
    mac[0] = 0x01;
    mac[1] = 0x00;
    mac[2] = 0x5e;
    sal_memcpy(&mac[3], ((uint8*)&gda) + 1, 3);
    mac[3] &= 0x7f;
}

uint8 *
locate_igmp_pkt(uint8 *eth_pkt)
{
    uint8 ip_header_len, index;
    enet_hdr_t  *th;        /* Tagged header pointers */

    th = (enet_hdr_t *)eth_pkt;

    if (ENET_TAGGED(th)) {
        index = 18;
    } else {
        index = 14;
    }

    ip_header_len = (eth_pkt[index] & 0x0f) << 2;
    return (eth_pkt + index + ip_header_len);
}

igmpsnoop_ip_hdr_t *
locate_ip_pkt(uint8 *eth_pkt)
{
    if (ENET_TAGGED((enet_hdr_t *)eth_pkt)) {
        return (igmpsnoop_ip_hdr_t *)(eth_pkt + 18);
    } else {
        return (igmpsnoop_ip_hdr_t *)(eth_pkt + 14);
    }
}

static BOOL
is_igmp_packet(uint8 *eth_pkt)
{
    const uint8 CODE mc_mac[5] = {0x01, 0x00, 0x5E, 0x00, 0x00};

    /* Don't bother if it's not an IP packet */
    if (ENET_TAGGED((enet_hdr_t *)eth_pkt)) {
        if (eth_pkt[16] != 0x08 || eth_pkt[17] != 0x00) {
            return FALSE;
        }
    } else {
        if (eth_pkt[12] != 0x08 || eth_pkt[13] != 0x00) {
            return FALSE;
        }
    }

    /* whether  (DA==01:00:5e:xx:xx:xx  &&  IP_header(protocol)==IGMP) ||
        (DA==01:00:5e:00:00:01) || (DA==01:00:5e:00:00:02) || (DA==01:00:5e:00:00:16) */
    if (!sal_memcmp(eth_pkt, mc_mac, 3) &&
        (locate_ip_pkt(eth_pkt))->protocol == IGMP_PROTOCOL_NUMBER) {
        return (TRUE);
    }
    return (FALSE);      /* not an valid IGMP packet */
}

static void
igmpsnoop(sys_pkt_t *pkt) REENTRANT
{
    igmpsnoop_igmp_hdr_t  *igmppkt;
    igmpsnoop_ip_hdr_t *ippkt;

    uint8 vlan_t_uplist[MAX_UPLIST_WIDTH], vlan_uplist[MAX_UPLIST_WIDTH];
    uint8 *eth_pkt = pkt->pkt_data;
    uint32 igmp_group;
    uint8 uplist[MAX_UPLIST_WIDTH];
    uint16 vid;
    enet_hdr_t  *th;        /* Tagged header pointers */
    vlan_type_t vlan_type;

    igmp_dbg_count.total_receive++;

    th = (enet_hdr_t *)eth_pkt;
    static uint8 igmpaddr[4]={224,0,0,22};
    if (board_vlan_type_get(&vlan_type) != SYS_OK) {
        return;
    }

    if (vlan_type == VT_PORT_BASED) {
        /* PVLAN */
        /* Save VLAN group member ports in vlan_uplist.
                * And the vlan_uplist will be used in API irrigate also.
                * However the vlan_t_uplist will be got in API irrigate.
              */
        if (board_pvlan_port_get(IGMPsnoop_vid, vlan_uplist) != SYS_OK) {
            IGMPSNOOP_DBG(("board_pvlan_port_get failed in igmpsnoop"));
            return;
        }
        /*  check whether pkt->rx_src_uport belong valid VLAN group */
        if (uplist_port_matched(vlan_uplist, pkt->rx_src_uport) != SYS_OK) {
            igmp_dbg_count.invalid_vlan_group++;
            return;
        }
        /* IGMP valid VLAN ID 0xFFF for PVLAN */
        vid = 0xFFF;
    } else {
        /*  QVLAN */
        if (ENET_TAGGED(th)) {
            vid = BCM_PKT_VLAN_ID(th);
        }
        if (vid == 0x0) {
            /* get PVID for untag packet and priority tag packet */
            if (board_untagged_vlan_get(pkt->rx_src_uport, &vid) != SYS_OK) {
                return;
            }
        }
        /* Check whether the VID is a valid VLAN ID */
        if (vid != IGMPsnoop_vid) {
              igmp_dbg_count.invalid_vlan_group++;
              return;
        }

        /* Save VLAN tagged & untagged member ports in vlan_t_uplist and vlan_uplist
               * And the both of vlan_t_uplist and vlan_uplist will be used in API irrigate also.
               */
        if (board_qvlan_port_get(vid, vlan_uplist, vlan_t_uplist) != SYS_OK) {
            IGMPSNOOP_DBG(("board_qvlan_port_get failed in igmpsnoop"));
            return;
        }

    }

    /* point to IP header*/
    ippkt = locate_ip_pkt(eth_pkt);

    /* point to IP payload (IGMP message) */
    igmppkt = (igmpsnoop_igmp_hdr_t*) locate_igmp_pkt(eth_pkt);

    if (uplist_clear(uplist) != SYS_OK) {
        return;
    }

    if(uplist_port_add(uplist, pkt->rx_src_uport) != SYS_OK) {
        IGMPSNOOP_DBG(("uplist_port_add failed in igmpsnoop"));
        return;
    }

    if(((uint8*)&(igmppkt->group) + 3) == NULL) {
        IGMPSNOOP_DBG(("((uint8*)&(igmppkt->group) + 3) == NULL\n"));
        return;
    }

    sal_memcpy(&igmp_group, &(igmppkt->group), 4);

    /* Relay the IGMP control packet to other necessary ports  */
    if(irrigate(pkt, igmp_group, vid, igmppkt->type,
                     vlan_uplist, vlan_t_uplist) != TRUE) {
        igmp_dbg_count.relay_fail++;
        return;
    }

    IGMPSNOOP_DBG(("igmp packet with protocol val %d type %d port %d", \
                   (int)ippkt->protocol, (int)igmppkt->type, \
                   (int)pkt->rx_src_uport));
    if (ippkt->protocol == IGMP_PROTOCOL_NUMBER) {

        switch (igmppkt->type) {
	case 0x22:
	    igmp_dbg_count.report_v3++;
        case (IGMPv2_REPORT) :
            igmp_dbg_count.report_v2++;
            igmpLeaveQ_delete(vid, igmp_group, uplist);
            /*  !! no 'break;' here !!!! */
        case (IGMPv1_REPORT) :
            if(igmppkt->type == IGMPv1_REPORT) {
                igmp_dbg_count.report_v1++;
            }
            if(!sal_memcmp(ippkt->desIp, all_router_ip, 3)) {
                /* do nothing for 224.0.0.X */
		if(!sal_memcmp(ippkt->desIp,igmpaddr,4))
		{
			
		}
            } else {
                if (igmpDB_update(vid, igmp_group, uplist) == FALSE) {
                    igmp_dbg_count.database_update_fail++;
                }
            }
            break;
        case (IGMPv2_LEAVE) :
            igmp_dbg_count.leave++;
            if (igmpLeaveQ_insert(vid, igmp_group, uplist) == FALSE) {
               igmp_dbg_count.leave_q_operation_fail++;
            }
            break;
        case (IGMPv2_QUERY) :
            /* update dynamic router port */
            igmp_dbg_count.query++;
            if(igmpsnoop_router_port_add(vid, pkt->rx_src_uport) == FALSE) {
               ;
            }

            if (igmpLeaveQ_grant(vid, igmp_group) == FALSE) {
               igmp_dbg_count.leave_q_operation_fail++;
            }
            break;
        default :
            igmp_dbg_count.unkonwn_igmp_type++;
        }
    } else {
        igmp_dbg_count.non_igmp++;
    }
}

sys_rx_t
igmpsnoop_rx(sys_pkt_t *pkt, void *cookie) REENTRANT
{
    UNREFERENCED_PARAMETER(cookie);

    if (!IGMPsnoop_enable || !is_igmp_packet(pkt->pkt_data)) {
	return (SYS_RX_NOT_HANDLED);
    }

    igmpsnoop(pkt);
    return SYS_RX_HANDLED;
}

static void
igmpsnoop_one_sec(void)
{
    uint16 i, j, index, pal, my_pioneer, next = MAX_IGMPleave;
    uint16 qlast;
    uint8 mc_mac[6];

    for (qlast = index = igmpLeaveQ ; index < MAX_IGMPleave ; index = next) {
        if (0 == (--(igmpLV[index].timeout))) {
            pal = 0;
            /* get the igmpDB entry of this igmpLeave node */
            if (NUM_IGMPgrp_PER_SYSTEM <=
                (i = my_pioneer =
                     igmpDB_search(igmpLV[index].vid, igmpLV[index].gda))) {
                continue;
            }
            /* no IGMP report received after Specific Query */
            /* check is any pal (GDAs with same L2MC MAC address) still use this port */
            for (j = 0 ; j < NUM_IGMPgrp_PER_SYSTEM ; ++j) {
                if ((j == i) || (is_invalid(j))) {
                    continue;
                }
                if (are_pals2(i, j)) {
                    if (is_pioneer(j)) {
                        my_pioneer = j;
                    }
                    if (IPMCuplist_mob(i, j, igmpLV[index].uport)) {
                        ++pal;     /* these two pals both forward to the same port  */
                    }
                }
            }
            if (pal == 0) {      /* no pal is using this port, stop forward to this port */
                L2MCportbmp_remove(my_pioneer, igmpLV[index].uport);
            }
            /* update the corresponding IPMC port bitmap,
                         clear the bit corresponding to this port */
            IPMCuplist_remove(i, igmpLV[index].uport);

            if(IPMCuplist_nil_uplist(i)) {
                /* port is null */
                igmpsnoop_IPMCentry_remove(i);
            }

            /* delete this node form igmpLeave queue */
            if (index == igmpLeaveQ) {      /* hit at the first node in the queue ? */
                next = igmpLeaveQ = qlast = igmpLV[index].next;
            } else {
                next = igmpLV[qlast].next = igmpLV[index].next;
            }
            igmpLeaveP_free(index);
        } else {   /* timeout > 0 */
            qlast = index;
            next = igmpLV[index].next;
        }
    } /* for index in igmpLeaveQ */

    if (!(--router_port_tick)) {
        /* age out dynamic router port */
        igmpsnoop_router_port_age();

        router_port_tick = IGMPparm_router_port_interval;
    }

    /*-- tick countdown */
    if (--igmpsnoop_tick) {
        /* query interval not reached */
        return;
    }

    /* cast igmpDB to physical L2MC multicast port bitmap */
    index = 0;
    for (i = 0 ; i < NUM_IGMPgrp_PER_SYSTEM ; i++) {
        if (is_invalid(i)) {
            continue;      /* skip invalid entry */
        }
        pal = NUM_IGMPgrp_PER_SYSTEM;
        if (!IPMCuplist_nil_age(i)) {
            /* age bits are not nil ? */
            /* pal == 1 + NUM_IGMPgrp_PER_SYSTEM; */
            pal++;
        }

        /* !! notice there is no 'break' inside this switch{} structure !! */
        switch (*((uint8*) &igmpDB[i]) & 0xc0) {
        case (0xc0) :     /* -- entry is a valid pioneer */
            /* entry is valid and need to be cast */
            /* summarize all pals' port bitmap    */
            IPMCuplist_summ_reset();
            for (j = 0 ; j < i ; j++) {
                if (are_pals2(i,j) && is_valid(j)) {
                    /* valid follower with active ports */
                    /* GDA of entry i and j cast to the same Multicast MAC address */
                    IPMCuplist_summ_uplist(j); /* entry j's age bits have been cleared */
                    if (pal == NUM_IGMPgrp_PER_SYSTEM) { /* all i's age bits are zero */
                        pal = j;               /* pal j will take over pioneer */
                    }
                }
            }
            IPMCuplist_summ_age(i);
            for (++j ; j < NUM_IGMPgrp_PER_SYSTEM ; j++) {
                if (are_pals2(i,j) && is_valid(j)) {
                    IPMCuplist_summ_age(j);
                    if (pal == NUM_IGMPgrp_PER_SYSTEM) {   /* all age bits are zero */
                        pal = j;               /* pal j will take over pioneer */
                    }
                }
            }
            L2MCportbmp_update(i, NULL, L2MCportbmp_CAST);
            /*!!!! NO 'break' here !!!! */
        case (0x80) :     /* entry is valid  (both pioneer and follower) */
            /* (pal == NUM_IGMPgrp_PER_SYSTEM) :
                          all age bits are zero, invalidate this entry */
            /* (pal <  NUM_IGMPgrp_PER_SYSTEM) :
                          pioneer with all zero age bits, and a pal exist */
            /* (pal >  NUM_IGMPgrp_PER_SYSTEM) :
                          normal case, an valid entry with non-zero age bits */
            if (pal == NUM_IGMPgrp_PER_SYSTEM) {
                /* no more pals (with same L2MC MAC addr) are active */
                /* delete corresponding ARL L2 entry */
                compose_mc_mac(igmpDB[i].gda, mc_mac);
                if (is_pioneer(i)) {
                    /* followers don't erase L2 entries */
                    board_mcast_addr_remove(mc_mac,
                        (igmpDB[i].vid) & VID_BITS_MASK);
                }
                /* invalidate this igmpDB entry */
                igmpDB[i].gda &= 0x0L;
                IPMCuplist_wipe(i);
            } else {
                if (pal <  NUM_IGMPgrp_PER_SYSTEM) {
                    /* all age bits are 0, no more IGMP report came from this group */
                    /* it's a pioneer and an pal exist, let the pal takes over the pioneer */
                    sal_memcpy(&igmpDB[i], &igmpDB[pal], sizeof(igmpdb_t));
                    /* copy age and bmp bits from pal */
                    IPMCuplist_copy(i, pal);
                    /* set valid and pioneer */
                    *((uint8*) &igmpDB[i]) |= 0xc0;
                    /* invalidate pal igmpDB entry */
                    igmpDB[pal].gda &= 0x0L;
                    IPMCuplist_wipe(pal);
                }
                /* this IGMP group is still active,  clear aging bits to perform aging */
                /* utilized bits for certain ports will be set next round
                                 while receiving IGMP report */
                IPMCuplist_aging(i);
            }
        }
    }

    /* resume the countdown tick */
    igmpsnoop_tick = IGMP_group_membership_interval;
}

void
igmpsnoop_one_sec_timer(void *arg) REENTRANT
{
    UNREFERENCED_PARAMETER(arg);

    if (!IGMPsnoop_enable) {
        return;
    }

    igmpsnoop_one_sec();
}

#endif /* defined(CFG_SWITCH_MCAST_INCLUDED) */

