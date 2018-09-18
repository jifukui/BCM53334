/*
 * $Id: igmpsnoop.c,v 1.55 Broadcom SDK $
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
#include "appl/persistence.h"
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

#define IGMP_PRIO   5
#define IGMP_POLLING_INTERVAL   (1000000UL)  /* 1 seconds in us. */

#define IGMP_LEAVE_TIMEOUT      (IGMP_SpecQUERY_TIMEOUT)

/*----------------------------------------------- */
/* function phototype */
/*----------------------------------------------- */
static void reset_igmpsnoop_tick(void);

IPMCuplist_t IPMCuplist[NUM_IGMPgrp_PER_SYSTEM];

BOOL IGMPsnoop_enable = FALSE;

uint16 IGMPsnoop_vid = 0x1;

/* IGMP_TIMER_PARAMTER_UNCHANGEABLE definition is used to reduce the
 *  usage of the public variables(i.e. XDATA in 8051 solution).
 */
#ifndef IGMP_TIMER_PARAMTER_UNCHANGEABLE
/* IGMP timer parameters */
uint8 IGMPparm_robustness = IGMP_DFT_ROBUSTNESS;
uint16 IGMPparm_query_interval = IGMP_DFT_QUERY_INTERVAL;
uint8 IGMPparm_query_response = IGMP_DFT_QUERY_RESPONSE_INTERVAL;
uint8 IGMPparm_last_member_query_interval = IGMP_DFT_LAST_MEMBER_QUERY_INTERVAL;
uint8 IGMPparm_last_member_query_count    = IGMP_DFT_LAST_MEMBER_QUERY_COUNT;
#endif  /* IGMP_TIMER_PARAMTER_UNCHANGEABLE */


uint16 IGMPparm_router_port_interval = IGMP_DFT_QUERY_INTERVAL;
igmpsnoop_router_port_t dynamic_router[BOARD_MAX_NUM_OF_QVLANS];
uint8 summary_uplist[MAX_UPLIST_WIDTH];
/* database and counters */
const uint8 CODE all_router_ip[4] = {0xe0, 0x00, 0x00, 0x02};
igmpdb_t igmpDB[NUM_IGMPgrp_PER_SYSTEM];
uint16 igmpLeaveQ;
igmpLeave_t igmpLV[MAX_IGMPleave];
uint16 igmpsnoop_tick;
uint16 router_port_tick;
igmp_dbg_count_t igmp_dbg_count;
uint16 igmpsnoop_get_vid_by_index(int16 index) {

    vlan_type_t vlan_type;
    uint8 uplist[MAX_UPLIST_WIDTH];
    int16 i;
    uint16 vid;
    if (board_vlan_type_get(&vlan_type) != SYS_OK) {
        return 0;

    }
    if (vlan_type == VT_PORT_BASED) {
        i = 0;
        vid = 0;

        do {
           vid ++;
           if (board_pvlan_port_get(vid,uplist) == SYS_OK) {
               i++;
           }
        } while((i <= index) && (vid < 4095));

        return vid;

    } else if (vlan_type == VT_DOT1Q) {
        if (board_qvlan_get_by_index(index,&vid,uplist,uplist) == SYS_OK) {
            return vid;
        }

    }
    return 0;
}

void
igmpsnoop_dbg_timer_get(uint16 *tick_timer,
                               uint16 *router_port_tick_timer)
{
    *tick_timer = igmpsnoop_tick;
    *router_port_tick_timer = router_port_tick;
}

void
igmpsnoop_dbg_count_get(igmp_dbg_count_t *count)
{
    sal_memcpy(count, &igmp_dbg_count, sizeof(igmp_dbg_count_t));
}

void
igmpsnoop_dbg_count_reset(void)
{
    sal_memset(&igmp_dbg_count, 0, sizeof(igmp_dbg_count_t));
}

BOOL
igmpsnoop_enable_set(uint8 enable)
{
    if (enable == TRUE) {
        /* enabling IGMPsnooping */
        if (!IGMPsnoop_enable) {
            if(sys_rx_register(igmpsnoop_rx, IGMP_PRIO, NULL, 0) != SYS_OK) {
                IGMPSNOOP_DBG(("sys_rx_register failed in \
                               igmpsnoop_enable_set"));
                return FALSE;
            }
            if(timer_add(igmpsnoop_one_sec_timer, NULL, IGMP_POLLING_INTERVAL)
                != TRUE) {
                IGMPSNOOP_DBG(("timer_add failed in igmpsnoop_enable_set"));
                return FALSE;
            }
            reset_igmpsnoop_tick();
            /* default vid for IGMP snoop */
            IGMPsnoop_vid = igmpsnoop_get_vid_by_index(0);

        }
        if(board_igmp_snoop_enable_set(TRUE) != SYS_OK) {
            IGMPSNOOP_DBG(("board_igmp_snoop_enable_set failed in \
                           igmpsnoop_enable_set"));
            return FALSE;
        }
        IGMPsnoop_enable = TRUE;
    } else {
        /* disabling IGMPsnooping, unregister handlers */
        board_igmp_snoop_enable_set(FALSE);
        timer_remove(igmpsnoop_one_sec_timer);
        sys_rx_unregister(igmpsnoop_rx);
        /* remove all multicast entries from ARL and clear the igmpDB */
        igmpsnoop_database_init();
        IGMPsnoop_enable = FALSE;
    }
    return TRUE;
}

void
igmpsnoop_enable_get(uint8 *enable)
{
    *enable = IGMPsnoop_enable;
}

void
igmpsnoop_database_init(void)
{
    int16 i;
    uint8 mc_mac[6];
    /* Clear IGMP Snoop member port database */
    for (i = 0 ; i < NUM_IGMPgrp_PER_SYSTEM ; i++) {
        if (is_pioneer(i)) {
            compose_mc_mac(igmpDB[i].gda, mc_mac);
            board_mcast_addr_remove(mc_mac, (igmpDB[i].vid) & VID_BITS_MASK);
        }
    }
    sal_memset(igmpDB, 0, sizeof(igmpDB));
    sal_memset(IPMCuplist, 0, sizeof(IPMCuplist));

    /* Clear dynamic router port database */
    for (i = 0 ; i < BOARD_MAX_NUM_OF_QVLANS ; i++) {
        if (dynamic_router[i].vid != 0) {
            dynamic_router[i].vid = 0x0;
            uplist_clear(dynamic_router[i].age);
            uplist_clear(dynamic_router[i].uplist);
        }
    }

    /* Clear Leave Queue */
    sal_memset(igmpLV, 0, sizeof(igmpLV));
    for (i = 0 ; i < MAX_IGMPleave ; i++) {
        igmpLV[i].gda = 0;
        igmpLV[i].next = MAX_IGMPleave;
    }
    igmpLeaveQ = MAX_IGMPleave;

    /* reset igmp debug count */
    sal_memset(&igmp_dbg_count, 0, sizeof(igmp_dbg_count_t));


     /* default vid for IGMP snoop */
    IGMPsnoop_vid = igmpsnoop_get_vid_by_index(0);

}

#if (CFG_CLI_ENABLED)
uint16
igmpsnoop_group_count_get(void)
{
    uint16 i, count = 0;

    for (i = 0 ; i < NUM_IGMPgrp_PER_SYSTEM ; i++) {
        if(is_valid(i)) {
            count++;
        }
    }
    return count;
}

BOOL
igmpsnoop_group_member_get(uint16 index,
              uint32 *gip, uint16 *vid, uint8 *uplist)
{
    uint16 i, count = 0, last_valid = 0;

    if(index >= igmpsnoop_group_count_get()) {
        return FALSE;
    }

    for (i = 0 ; i < NUM_IGMPgrp_PER_SYSTEM ; i++) {
        if(is_valid(i)) {
            count++;
            last_valid=i;
        }
        if(count > index) {
            break;
        }
    }

    *gip = igmpDB[last_valid].gda;
    *vid = igmpDB[last_valid].vid;

    if (uplist_clear(uplist) != SYS_OK) {
        return FALSE;
    }
    if (uplist_manipulate(uplist, IPMCuplist[last_valid].uplist,
                                        UPLIST_OP_COPY) != SYS_OK) {
        return FALSE;
    }
    /* OR dynamic router port */
    for (i = 0 ; i < BOARD_MAX_NUM_OF_QVLANS ; i++) {
        if(dynamic_router[i].vid == igmpDB[last_valid].vid) {
            if(uplist_manipulate(uplist, dynamic_router[i].uplist,
                UPLIST_OP_OR) != SYS_OK) {
                return FALSE;
            }
            break;
        }
    }

    return TRUE;
}

uint16
igmpsnoop_dynamic_router_port_count_get(void)
{
    uint16 i, count = 0;

    for (i = 0 ; i < BOARD_MAX_NUM_OF_QVLANS ; i++) {
        if(dynamic_router[i].vid != 0) {
            count++;
        }
    }
    return count;
}

BOOL
igmpsnoop_dynamic_router_port_get(uint8 index,
                  uint16 *vid, uint8 *uplist)
{
    int i, count=0, last_valid=0;

    if(index >= igmpsnoop_dynamic_router_port_count_get()){
        return FALSE;
    }

    for(i = 0 ; i < BOARD_MAX_NUM_OF_QVLANS ; i++){
        if(dynamic_router[i].vid != 0){
            count++;
            last_valid=i;
        }

        if(count > index){
            break;
        }
    }

    *vid = dynamic_router[last_valid].vid;

    if (uplist_manipulate(uplist, dynamic_router[last_valid].uplist,
                                            UPLIST_OP_COPY) != SYS_OK) {
        return FALSE;
    }

    return TRUE;
}
#endif /* (CFG_CLI_ENABLED) */

uint8
igmpDB_new(void)
{
    uint16 i;

    for (i = 0 ; i < NUM_IGMPgrp_PER_SYSTEM ; i++) {
        if (0 == (*((uint8*)&igmpDB[i]) & 0x80)) {
            break;
        }
    }
    return (i);
}

uint16
igmpDB_search(uint16 vid, uint32 gda)
{
    uint16 i;

    for (i = 0 ; i < NUM_IGMPgrp_PER_SYSTEM ; i++) {
        if (((gda & IGMPDB_MASK) == (igmpDB[i].gda & IGMPDB_MASK)) &&
            ((vid & VID_BITS_MASK) == (igmpDB[i].vid & VID_BITS_MASK))) {
            return (i);
        }
    }
    return (i);
}

BOOL
igmpDB_update(uint16 vid, uint32 gda, uint8 *uplist)
{
    uint16 i, j;
    uint8 mc_mac[6];
    uint16 count;
    uint8 uplist_temp[MAX_UPLIST_WIDTH];

    if (NUM_IGMPgrp_PER_SYSTEM <= (i = igmpDB_search(vid, gda))) {
        /* (1)  create new IGMP group entry */
        if (NUM_IGMPgrp_PER_SYSTEM <= (i = igmpDB_new())) {
            /* igmpDB full !!  no more entry available */
            return FALSE;
        }
        compose_mc_mac(gda, mc_mac);
        /* (2)  add new ARL entry */

        count = board_vlan_count();
        if(uplist_manipulate(uplist_temp, uplist, UPLIST_OP_COPY) != SYS_OK) {
            return FALSE;
        }

        /* OR dynamic router port */
        for (j = 0 ; j < count ; j++) {
            if(dynamic_router[j].vid == vid) {
                if(uplist_manipulate(uplist_temp,
                       dynamic_router[j].uplist, UPLIST_OP_OR) != SYS_OK) {
                    return FALSE;
                }
                break;
            }
        }

        if (board_mcast_addr_add(mc_mac, vid, uplist_temp) != SYS_OK) {
             /* invalidate (free) this igmpDB entry */
             /* no more ARL entry available */
            igmpDB[i].gda = 0;
            return FALSE;
        }
        igmpDB[i].vid = vid;
        igmpDB[i].gda = gda;

        /* valid bit was set since  GDA == (E0.0.0.0 ~ Ef.ff.ff.ff) */
        /* now determine it's a pioneer or follower, by checking whether
         * a pal exist
         */
        for (j = 0 ; j < NUM_IGMPgrp_PER_SYSTEM ; ++j) {
            if (are_pals(i, j)) {
                break;
            }
        }
        if (j < NUM_IGMPgrp_PER_SYSTEM) {
            /* a pal found, igmpDB[i] is a follower */
            *((uint8*)&(igmpDB[i])) &=  0xbf;
        }
    } else {
        /* igmpDB entry exist, new port joint, update L2MC multicast
         * port bitmap
         */
        if (is_follower(i)) {
            /*-- find out the pioneer pal (j)  (GDAs cast to the same
             *      multicast MAC address)
             */
            for (j = 0 ; j < NUM_IGMPgrp_PER_SYSTEM ; ++j) {
                if (are_pals(i, j)  &&  is_pioneer(j)) {
                    break;
                }
            }
        } else {
            j = i;
        }
        L2MCportbmp_update(j, uplist, L2MCportbmp_ADD);
    }
    IPMCuplist_add(i, uplist);

    return TRUE;
}

void
igmpsnoop_IPMCentry_remove(uint16 entry_num)
{
    uint16 j, pal;
    uint8 mc_mac[6];

    pal = NUM_IGMPgrp_PER_SYSTEM;

    if((*((uint8*) &igmpDB[entry_num]) & 0xc0) == 0xc0) { /* entry is a valid pioneer */
        /* entry is valid and need to be cast */
        /* summarize all pals' port bitmap    */
        IPMCuplist_summ_reset();
        for (j = 0 ; j < NUM_IGMPgrp_PER_SYSTEM ; j++) {
            if (are_pals(entry_num,j) && is_valid(j)) {
                /* valid follower with active ports */
                /* GIP of entry entry_num and j cast to the same Multicast MAC address */
                IPMCuplist_summ_uplist(j); /* entry j's age bits have been cleared */
                if (pal == NUM_IGMPgrp_PER_SYSTEM) { /* all entry_num's age bits are zero */
                    pal = j;               /* pal j will take over pioneer */
                }
            }
        }
        L2MCportbmp_update(entry_num, NULL, L2MCportbmp_CAST);
    }

    /* (pal <  NUM_IGMPgrp_PER_SYSTEM) :  pioneer with all zero age bits, and a pal exist  */
    if (pal == NUM_IGMPgrp_PER_SYSTEM) {
        /* no more pals (with same L2MC MAC addr) are active */
        /* delete corresponding ARL L2 entry */
        compose_mc_mac(igmpDB[entry_num].gda, mc_mac);
        if (is_pioneer(entry_num)) {   /* followers don't erase L2 entries */
            board_mcast_addr_remove(mc_mac,
                  (igmpDB[entry_num].vid) & VID_BITS_MASK);
        }
        igmpDB[entry_num].gda &= 0x0L;     /* invalidate pal igmpDB entry */
        IPMCuplist_wipe(entry_num);
    } else {
        if (pal <  NUM_IGMPgrp_PER_SYSTEM) {
            /* all age bits are 0, no more IGMP report came from this group */
            /* it's a pioneer and an pal exist, let the pal takes over the pioneer */
            sal_memcpy(&igmpDB[entry_num], &igmpDB[pal], sizeof(igmpdb_t));
            IPMCuplist_copy(entry_num, pal);    /* copy age and bmp bits from pal */
            *((uint8*) &igmpDB[entry_num]) |= 0xc0; /* set valid and pioneer      */
            igmpDB[pal].gda &= 0x0L;       /* invalidate pal igmpDB entr */
            IPMCuplist_wipe(pal);
        }
        /* this IGMP group is still active,  clear aging bits to perform aging */
        /* utilized bits for certain ports will be set next round while receiving IGMP report */
    }
    return;
}

BOOL
igmpsnoop_router_port_add(uint16 vid, uint16 uport)
{
    uint16 i, j;

    if (SAL_UPORT_IS_NOT_VALID(uport)) {
        return FALSE;
    }

    for (i = 0 ; i < BOARD_MAX_NUM_OF_QVLANS ; i++) {
        if (dynamic_router[i].vid == vid) {
            break;
        }
    }

    if (i == BOARD_MAX_NUM_OF_QVLANS) {
        for (i = 0 ; i < BOARD_MAX_NUM_OF_QVLANS ; i++) {
            if (dynamic_router[i].vid == 0x0) {
                break;
            }
        }
        if (i == BOARD_MAX_NUM_OF_QVLANS) {
            IGMPSNOOP_DBG(("igmpsnoop_router_port_add failed"));
            return FALSE;
        } else {
            dynamic_router[i].vid = vid;
        }
    }

    if (uplist_port_matched(dynamic_router[i].uplist, uport) == SYS_OK) {
        if (uplist_port_add(dynamic_router[i].age, uport) != SYS_OK) {
            return FALSE;
        }
        return TRUE;
    } else {
        if (uplist_port_add(dynamic_router[i].uplist, uport) != SYS_OK) {
            return FALSE;
        }
        if (uplist_port_add(dynamic_router[i].age, uport) != SYS_OK) {
            return FALSE;
        }
    }

    /* re-calculate the l2mc entry */
    for (i = 0 ; i < NUM_IGMPgrp_PER_SYSTEM ; i++) {
        if (! is_pioneer(i)) {
            continue;
        }
        IPMCuplist_summ_reset();
        for (j = 0 ; j < NUM_IGMPgrp_PER_SYSTEM ; j++) {
            if (are_pals2(i,j) && is_valid(j)) {
                IPMCuplist_summ_uplist(j);
            }
        }
        L2MCportbmp_update(i, 0, router_uplist_UPDATE);
    }

    return TRUE;
}

BOOL
igmpsnoop_router_port_age(void)
{
    uint16 i, j;
    uint16 count;

    count = board_vlan_count();

    for (i = 0 ; i < count ; i++) {
        if (dynamic_router[i].vid != 0) {
            if (uplist_manipulate(dynamic_router[i].uplist,
                dynamic_router[i].age, UPLIST_OP_COPY) != SYS_OK) {
                return FALSE;
            }
            if (uplist_is_empty(dynamic_router[i].uplist) == SYS_OK) {
                dynamic_router[i].vid = 0x0;
            }

            if (uplist_clear(dynamic_router[i].age) != SYS_OK) {
                return FALSE;
            }
        }
    }

    /* re-calculate the l2mc entry */
    for (i = 0 ; i < NUM_IGMPgrp_PER_SYSTEM ; i++) {
        if (! is_pioneer(i)) {
            continue;
        }
        IPMCuplist_summ_reset();
        for (j = 0 ; j < NUM_IGMPgrp_PER_SYSTEM ; j++) {
            if (are_pals2(i,j) && is_valid(j)) {
                IPMCuplist_summ_uplist(j);
            }
        }
        if (L2MCportbmp_update(i, 0, router_uplist_UPDATE) == FALSE) {
            return FALSE;
        }
    }

    return TRUE;
}

static void
reset_igmpsnoop_tick(void)
{
    igmpsnoop_tick = IGMP_group_membership_interval;
    router_port_tick = IGMPparm_router_port_interval;
}

uint8
igmpLeaveP_new(void)
{
    uint16 i;

    for (i = 0 ; i < MAX_IGMPleave ; i++) {
        if (igmpLV[i].gda == 0) {
            return i;
        }
    }
    return MAX_IGMPleave;
}

void
igmpLeaveP_free(uint16 p)
{
    igmpLV[p].gda = 0;
    igmpLV[p].next = MAX_IGMPleave;
}

uint16
igmpLeaveP_search(uint16 vid, uint32 gda, uint16 uport, uint16 *prev)
{
    uint16 p;
    uint16 qlast;

    for (qlast = p = igmpLeaveQ; p < MAX_IGMPleave; p = igmpLV[p].next) {
        if ((igmpLV[p].vid == vid) &&
            (igmpLV[p].gda  == gda) &&
            (igmpLV[p].uport == uport)) {
            break;
        }
        qlast = p;
    }

    if (prev != NULL) {
        *prev = qlast;
    }
    return (p);
}

BOOL
igmpLeaveQ_insert(uint16 vid, uint32 gda, uint8 *uplist)
{
    uint16 uport;
    uint16 index, qlast;
    uint8 timeout_val;
    uint8 existed = FALSE;

    if (NUM_IGMPgrp_PER_SYSTEM <= igmpDB_search(vid, gda)) {
        return (FALSE);
    }

    SAL_UPORT_ITER(uport) {
        if (uplist_port_matched(uplist, uport) == SYS_OK) {
            if (MAX_IGMPleave <=
                (index = igmpLeaveP_search(vid, gda, uport, &qlast))) {
                if (MAX_IGMPleave <= (index = igmpLeaveP_new())) {
                    return (FALSE);
                }
                igmpLV[index].gda  = gda;
                igmpLV[index].vid  = vid;
                igmpLV[index].uport = uport;
                igmpLV[index].next = igmpLeaveQ;
                igmpLeaveQ     = index;
            } else {
                existed = TRUE;
            }

            timeout_val = IGMP_LEAVE_TIMEOUT;

            /* for the existed leave Q, keep the small timeout value */
            if (existed) {
                if (igmpLV[index].timeout < timeout_val) {
                    timeout_val = igmpLV[index].timeout;
                }
            }

            igmpLV[index].timeout = timeout_val;
        }
    }
    return (TRUE);
}

BOOL
igmpLeaveQ_grant(uint16 vid, uint32 gda)
{
    uint16 p;

    for (p = igmpLeaveQ; p < MAX_IGMPleave; p = igmpLV[p].next) {
        if ((igmpLV[p].vid == vid) && (igmpLV[p].gda  == gda)) {
            /*-- trigger the timer */
            igmpLV[p].timeout = IGMP_SpecQUERY_TIMEOUT;
        }
    }
    return (TRUE);
}

BOOL
igmpLeaveQ_delete(uint16 vid, uint32 gda, uint8 *uplist)
{
    uint16 uport;
    uint16 index, qlast;
    SAL_UPORT_ITER(uport){
        if (uplist_port_matched(uplist, uport) == SYS_OK) {
            if (MAX_IGMPleave <=
                (index = igmpLeaveP_search(vid, gda, uport, &qlast))) {
                return (FALSE);
            }
            if (index == igmpLeaveQ) {     /* hit at the first node in the queue ? */
                igmpLeaveQ = igmpLV[index].next;
            } else {
                igmpLV[qlast].next = igmpLV[index].next;
            }
            igmpLeaveP_free(index);
        }
    }

    return (TRUE);
}

void
IPMCuplist_summ_reset(void)
{
    uplist_clear(summary_uplist);
}

void
IPMCuplist_summ_uplist(uint16 index)
{
    uint16 i;
    uint16 count;

    count = board_vlan_count();

    uplist_manipulate(summary_uplist, IPMCuplist[index].uplist, UPLIST_OP_OR);

    /* OR dynamic router port */
    for (i = 0 ; i < count ; i++) {
        if(dynamic_router[i].vid == igmpDB[index].vid) {
            uplist_manipulate(summary_uplist,
                   dynamic_router[i].uplist, UPLIST_OP_OR);
            break;
        }
    }
}

void
IPMCuplist_summ_age(uint16 index)
{
    uint16 i;
    uint16 count;

    count = board_vlan_count();

    uplist_manipulate(summary_uplist, IPMCuplist[index].age, UPLIST_OP_OR);

    /* OR dynamic router port */
    for (i = 0 ; i < count ; i++) {
        if(dynamic_router[i].vid == igmpDB[index].vid) {
            uplist_manipulate(summary_uplist,
                   dynamic_router[i].uplist, UPLIST_OP_OR);
            break;
        }
    }
}

/* set corresponding bit to 1 */
void
IPMCuplist_add(uint16 index, uint8 *uplist)
{
    uint16 uport;

    SAL_UPORT_ITER(uport) {
        if (uplist_port_matched(uplist, uport) == SYS_OK) {
            uplist_port_add(IPMCuplist[index].age, uport);
            uplist_port_add(IPMCuplist[index].uplist, uport);
        }
    }
}

/* clear corresponding bit to 0 */
void
IPMCuplist_remove(uint16 index, uint16 uport)
{
    uplist_port_remove(IPMCuplist[index].age, uport);
    uplist_port_remove(IPMCuplist[index].uplist, uport);
}

void
IPMCuplist_wipe(uint16 index)
{
    uplist_clear(IPMCuplist[index].age);
    uplist_clear(IPMCuplist[index].uplist);
}

BOOL
IPMCuplist_nil_uplist(uint16 index)
{
    if (uplist_is_empty(IPMCuplist[index].uplist) == SYS_OK) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
IPMCuplist_nil_age(uint16 index)
{
    if(uplist_is_empty(IPMCuplist[index].age) == SYS_OK) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void
IPMCuplist_copy(uint16 i, uint16 j)
{
    /*-- copy entry j to entry i */
    uplist_manipulate(IPMCuplist[i].uplist,
           IPMCuplist[j].uplist, UPLIST_OP_COPY);
    uplist_manipulate(IPMCuplist[i].age, IPMCuplist[j].age, UPLIST_OP_COPY);
}

void
IPMCuplist_aging(uint16 index)
{
    uplist_manipulate(IPMCuplist[index].uplist,
            IPMCuplist[index].age, UPLIST_OP_COPY);
    uplist_clear(IPMCuplist[index].age);
}

/*  the port is a member */
BOOL
IPMCuplist_mob(uint16 i, uint16 j, uint16 uport)
{
    if ((uplist_port_matched(IPMCuplist[i].age, uport) == SYS_OK) &&
        (uplist_port_matched(IPMCuplist[j].age, uport) == SYS_OK)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
L2MCportbmp_update(uint16 index, uint8 *uplist, uint8 op)
{
    uint8 exist_uplist[MAX_UPLIST_WIDTH];
    uint16 uport;
    uint16 vid = igmpDB[index].vid;
    uint8 mc_mac[6];

    compose_mc_mac(igmpDB[index].gda, mc_mac);

    switch (op) {
    case (router_uplist_UPDATE):
    case (L2MCportbmp_CAST):
        if (board_mcast_port_get(mc_mac, vid, exist_uplist) != SYS_OK) {
            return FALSE;
        }
        /* MCT port members not in summary_list must leave */
        SAL_UPORT_ITER(uport) { 
            if (uplist_port_matched(exist_uplist, uport) == SYS_OK) {
                if (uplist_port_matched(summary_uplist, uport) != SYS_OK) {
                    if (board_mcast_port_remove(mc_mac, vid, uport) != SYS_OK) {
                        return FALSE;
                    }
                }
            }
        }

        if (op == router_uplist_UPDATE) {  /* no join port in this case */
            /* summary port members not in MCT ports list must join */
            SAL_UPORT_ITER(uport) {
                if (uplist_port_matched(summary_uplist, uport) == SYS_OK) {
                    if (uplist_port_matched(exist_uplist, uport) != SYS_OK) {
                        if (board_mcast_port_add(mc_mac,
                                        vid, uport) != SYS_OK) {
                            return FALSE;
                        }
                    }
                }
            }
        }
        break;
    case (L2MCportbmp_ADD):
          SAL_UPORT_ITER(uport){
            if (uplist_port_matched(uplist, uport) == SYS_OK) {
                if (board_mcast_port_add(mc_mac, vid, uport) != SYS_OK) {
                    return FALSE;
                }
            }
        }
        break;
    }
    return (TRUE);
}

void
L2MCportbmp_remove(uint16 index, uint16 uport)
{
    uint16 i;
    uint16 vid = igmpDB[index].vid;
    uint8 mc_mac[6];
    BOOL isDynamicRouterPort = FALSE;

    compose_mc_mac(igmpDB[index].gda, mc_mac);

    /* check the port is dynamic router port or not */
    for (i = 0 ; i < BOARD_MAX_NUM_OF_QVLANS ; i++) {
        if (dynamic_router[i].vid == vid) {
            if (uplist_port_matched(dynamic_router[i].uplist, uport) == SYS_OK) {
                 isDynamicRouterPort = TRUE;
            }
            break;
        }
    }

    if(isDynamicRouterPort == FALSE) {
        board_mcast_port_remove(mc_mac, vid, uport);
    }
}

/*== IRRIGATE within the VLAN, compare to 'flood',      */
/*== selectively forward IGMP messages to necessary ports   */
BOOL
irrigate(sys_pkt_t *pkt, uint32 igmp_group, uint16 vid,
             uint8 igmp_type, uint8 *uplist, uint8 *t_uplist) REENTRANT
{
    uint16 s = 0;
    uint16 uport;
    uint8 *eth_pkt = pkt->pkt_data;
    uint16 i;
    sys_pkt_t *tx_pkt;
    uint8 *txpkt_data;
    uint16 txpkt_len;
    enet_hdr_t  *th;        /* Tagged header pointers */
    vlan_type_t vlan_type;
    uint8 uplist_temp[MAX_UPLIST_WIDTH];
    sys_error_t rv;
    BOOL isDynamicRouterPort = FALSE;
    port_mode_t mode;
#ifdef CFG_SWITCH_LAG_INCLUDED
    uint8 enable;
    uint8 lag_uplist[MAX_UPLIST_WIDTH];
    uint8 lag_first_member[BOARD_MAX_NUM_OF_LAG];
#endif /* CFG_SWITCH_LAG_INCLUDED */

    th = (enet_hdr_t *)eth_pkt;

    if (board_vlan_type_get(&vlan_type) != SYS_OK) {
        return FALSE;
    }

    /* Remove ports from forwarding list for REPORT/LEAVE packet */
    if ((igmp_type == IGMPv1_REPORT) ||
        (igmp_type == IGMPv2_REPORT) ||
        (igmp_type == IGMPv2_LEAVE))
    {
        igmp_dbg_count.relay_report_and_leave++;
        /* Remove this port from uplist if this port is not a dynamic router port */
        SAL_UPORT_ITER(uport) {
            if (uplist_port_matched(uplist, uport) != SYS_OK) {
                continue;
            }
            isDynamicRouterPort = FALSE;
            for (i = 0 ; i < BOARD_MAX_NUM_OF_QVLANS ; i++) {
                if (dynamic_router[i].vid == vid) {
                    if(uplist_port_matched(dynamic_router[i].uplist, uport) == SYS_OK) {
                        isDynamicRouterPort = TRUE;
                    }
                    break;
                }
            }

            if(isDynamicRouterPort == FALSE) {
                if (uplist_port_remove(uplist, uport) != SYS_OK) {
                    IGMPSNOOP_DBG(("uplist_port_remove failed \
                                   in irrigate"));
                    return FALSE;
                }
            }
        }
    }

    /* Remove ports from forwarding list for specific query packet */
    if (igmp_type == IGMPv2_QUERY){
        if(igmp_group != IGMP_GeneralQUERY_GDA) {
            /* Forward specific Query to memberPorts     */
            igmp_dbg_count.relay_specific_query++;
            s = igmpDB_search(vid, igmp_group);
            if (s < NUM_IGMPgrp_PER_SYSTEM) {
                /* Remove this port from uplist if this port is not a member port */
                SAL_UPORT_ITER(uport) {
                    if ((uplist_port_matched(uplist, uport) == SYS_OK) &&
                        (uplist_port_matched(IPMCuplist[s].uplist, uport) != SYS_OK)) {
                        if (uplist_port_remove(uplist, uport) != SYS_OK) {
                            IGMPSNOOP_DBG(("uplist_port_remove failed \
                                           in irrigate"));
                            return FALSE;
                        }
                    }
                }
            }
        } else {
            igmp_dbg_count.relay_query++;
        }
    }

    if (uplist_is_empty(uplist) != SYS_OK) {     /* if not empty port list */
        /* remove link-down port from uplist */
            SAL_UPORT_ITER(uport) {
            if (uplist_port_matched(uplist, uport) == SYS_OK) {
                if (board_port_mode_get(uport, &mode) != SYS_OK) {
                    return FALSE;
                }

                if (mode == PM_LINKDOWN){
                    if (uplist_port_remove(uplist, uport) != SYS_OK) {
                        IGMPSNOOP_DBG(("uplist_port_remove failed in irrigate"));
                        return FALSE;
                    }
                }
            }
        }

#ifdef CFG_SWITCH_LAG_INCLUDED
        for (i = 1; i <= BOARD_MAX_NUM_OF_LAG; i++) {
            lag_first_member[i-1] = TRUE;
        }

        /* Remove redundant ports based on lag group member */
        SAL_UPORT_ITER(uport) {
            if (uplist_port_matched(uplist, uport) == SYS_OK) {
                for (i = 1; i <= BOARD_MAX_NUM_OF_LAG; i++) {
                    if (board_lag_group_get(i, &enable, lag_uplist) != SYS_OK) {
                        return FALSE;
                    }
                    if ((enable == TRUE) && (uplist_port_matched(lag_uplist, uport) == SYS_OK)) {
                        if (uplist_port_matched(lag_uplist, pkt->rx_src_uport) == SYS_OK) {
                            if (uplist_port_remove(uplist, uport) != SYS_OK) {
                                IGMPSNOOP_DBG(("uplist_port_remove failed in irrigate"));
                                return FALSE;
                            }
                        } else {
                            if (lag_first_member[i-1] == TRUE) {
                                lag_first_member[i-1] = FALSE;
                            } else {
                                if (uplist_port_remove(uplist, uport) != SYS_OK) {
                                    IGMPSNOOP_DBG(("uplist_port_remove failed in irrigate"));
                                    return FALSE;
                                }
                            }
                        }
                    }
                }
            }
        }
#endif /* CFG_SWITCH_LAG_INCLUDED */

        /* exclude source port */
        if (uplist_port_remove(uplist, pkt->rx_src_uport) != SYS_OK) {
            IGMPSNOOP_DBG(("uplist_port_remove failed in irrigate"));
            return FALSE;
        }

        /* allocate and fill packet parameters */
        tx_pkt = (sys_pkt_t *)sal_malloc(sizeof(sys_pkt_t));
        if (tx_pkt == NULL) {
            IGMPSNOOP_DBG(("Out of memory!\n"));
            return FALSE;
        }
        sal_memset(tx_pkt, 0, sizeof(sys_pkt_t));

        tx_pkt->pkt_data = pkt->pkt_data;
        tx_pkt->pkt_len = pkt->pkt_len;
        tx_pkt->buf_len = pkt->buf_len;
        tx_pkt->flags = 0;

        /* Have to meet minimal packet length */
        if (ENET_TAGGED(th)) {
            if (tx_pkt->pkt_len < 64) {
                tx_pkt->pkt_len = 64;
            }
        } else {
            if (tx_pkt->pkt_len < 60) {
                tx_pkt->pkt_len = 60;
            }
        }

        if (vlan_type == VT_PORT_BASED) {
            if (ENET_TAGGED(th)) {
                /* Get t_uplist based on packet's VID */
                rv = board_qvlan_port_get(BCM_PKT_VLAN_ID(th),
                                           uplist_temp, t_uplist);
                if(rv == SYS_ERR_NOT_FOUND) {
                    /* Do nothing for ROBO chip */
                } else if (rv != SYS_OK) {
                    IGMPSNOOP_DBG(("board_qvlan_port_get failed in irrigate"));
                    return FALSE;
                }

            } else {
                /* Do nothing for ROBO chip */
            }
        }

        /* AND t_uplist with uplist */
        if (uplist_manipulate(t_uplist, uplist, UPLIST_OP_AND) != SYS_OK) {
            IGMPSNOOP_DBG(("uplist_manipulate with AND failed in irrigate"));
            return FALSE;
        }

        /* forward IGMP messages */
        txpkt_data = (uint8 *)sal_dma_malloc(DEFAULT_RX_BUFFER_SIZE);
        if (txpkt_data == NULL) {
            IGMPSNOOP_DBG(("Out of memory!\n"));
            return FALSE;
        }
        /* backup tx_pkt->pkt_data and tx_pkt->pkt_len
        because they could be revised in sys_tx for untag member */
        txpkt_len = tx_pkt->pkt_len;
        sal_memcpy(txpkt_data, pkt->pkt_data, txpkt_len);

        
        igmp_dbg_count.relay_success++;
        SAL_UPORT_ITER(uport) {
            if (uplist_port_matched(uplist, uport) == SYS_OK) {
                if (uplist_clear(tx_pkt->tx_uplist) != SYS_OK) {
                    return FALSE;
                }
                if (uplist_clear(tx_pkt->tx_untag_uplist) != SYS_OK) {
                    return FALSE;
                }
                if (uplist_port_add(tx_pkt->tx_uplist, uport) != SYS_OK) {
                    return FALSE;
                }

                tx_pkt->flags &= ~SYS_TX_FLAG_USE_UNTAG_PORT_LIST;
                if (uplist_port_matched(t_uplist, uport) != SYS_OK) {
                    /* untag */
                    tx_pkt->flags |= SYS_TX_FLAG_USE_UNTAG_PORT_LIST;
                    if (uplist_port_add(tx_pkt->tx_untag_uplist, uport) != SYS_OK) {
                        return FALSE;
                    }
                }

                if (sys_tx(tx_pkt, NULL)) {
                    IGMPSNOOP_DBG(("sys_tx tagged members failed in irrigate"));
                    return FALSE;
                }

                /* recover tx_pkt->pkt_data and tx_pkt->pkt_len
                because they could be revised in sys_tx for untag member */
                sal_memcpy(tx_pkt->pkt_data, txpkt_data, txpkt_len);
                tx_pkt->pkt_len = txpkt_len;
            }
        }

        /* free tx_pkt */
        sal_free(tx_pkt);
        sal_dma_free(txpkt_data);

        return TRUE;
    } else {
        IGMPSNOOP_DBG(("empty uplist in irrigate"));
        return TRUE;
    }
}


sys_error_t
igmpsnoop_vid_set(uint16 vid) REENTRANT
{
    vlan_type_t vlan_type;
    uint8 uplist[MAX_UPLIST_WIDTH];

    if (IGMPsnoop_enable == 0) {
        return SYS_ERR_STATE;
    }
    if (board_vlan_type_get(&vlan_type) != SYS_OK) {
        return SYS_ERR_STATE;
    }

    if (vlan_type == VT_PORT_BASED) {
       if (board_pvlan_port_get(vid,uplist) != SYS_OK) {
           return SYS_ERR_NOT_FOUND;
       }
    } else if (vlan_type == VT_DOT1Q) {
        if (board_qvlan_port_get(vid,uplist,uplist) != SYS_OK) {
           return SYS_ERR_NOT_FOUND;
        }
    }
    if(vid != IGMPsnoop_vid) {
        /* remove all multicast entries from ARL and clear the igmpDB */
        igmpsnoop_database_init();
        IGMPsnoop_vid = vid;
    }
    return SYS_OK;
}

void
igmpsnoop_vid_get(uint16 *vid) REENTRANT
{

    *vid = IGMPsnoop_vid;
}




#endif /* defined(CFG_SWITCH_MCAST_INCLUDED) */

