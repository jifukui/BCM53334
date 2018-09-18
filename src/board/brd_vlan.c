/*
 * $Id: brd_vlan.c,v 1.39 Broadcom SDK $
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

#ifdef __C51__
#ifdef CODE_USERCLASS
#pragma userclass (code = brdimpl)
#endif /* CODE_USERCLASS */
#endif /* __C51__ */

#include "system.h"
#include "brdimpl/vlan.h"
#include "utils/ports.h"
#include "appl/igmpsnoop.h"

/*
 * Typical implementation of VLAN board functions
 *
 * Note: Only applicable for single-unit board.
 */

#ifdef CFG_SWITCH_VLAN_INCLUDED

/* the SW VLAN database */
vlan_info_t  vlan_info;

/* the global variable to represent each port's snapshot on belonging to which
 *  PVLAN or PVLANs.
 */
static pvlan_bmp_t member_of_pvlan_bmp[BOARD_MAX_NUM_OF_PORTS + 1];

/* ------- Internal function ------ */

/*
 * _brdimpl_vlan_sw_vlan_group_get()
 *  - direct to the indicated VLAN entry (vlan_list_t)
 *
 *  Parameter :
 *      group_id - (IN) vlan groud id
 *      vlan_list - (OUT) vlan list pointer
 *      delete_node - (IN) delete node if found
 */
STATICFN sys_error_t
_brdimpl_vlan_sw_vlan_group_get(uint16 group_id,
                                vlan_list_t **vlan_list,
                                BOOL delete_node) REENTRANT
{
   vlan_list_t *this_vlan = NULL, *prev = NULL;

    this_vlan = vlan_info.head;

    /* go to this VLAN group */
    while(this_vlan) {
        if (this_vlan->vlan_id == group_id){
            if (delete_node) {
                /* Update head if necessary */
                if (prev != NULL) {
                    prev->next = this_vlan->next;
                } else {
                    vlan_info.head = this_vlan->next;
                }
                /* Update tail if necessary */
                if (this_vlan == vlan_info.tail) {
                    vlan_info.tail = prev;
                }
                vlan_info.count--;
            }
            *vlan_list = this_vlan;
            return SYS_OK;
        }
        prev = this_vlan;
        this_vlan = this_vlan->next;
    }

    return SYS_ERR_NOT_FOUND;
}

/* _brdimpl_uplist_allport_set()
 *
 */
STATICFN void
_brdimpl_uplist_allport_set(uint8 *uplist) REENTRANT
{
    uint16 uport;
    
    SAL_UPORT_ITER(uport) {
        uplist_port_add(uplist, uport);
    }
}

/*
 * _brdimpl_vlan_init() :
 *  - user for SW VLAN database init only.
 *  - will be called at the device bootup process to ensure
 *      all items' value in the vlan_info is assigned properly.
 */
sys_error_t
_brdimpl_vlan_init(void) REENTRANT
{
    vlan_info.head = NULL;
    vlan_info.tail = NULL;
#if (CFG_XGS_CHIP && CFG_PERSISTENCE_SUPPORT_ENABLED)
    /* Assign invalid vlan type so it could be updated later */
    vlan_info.type = VT_COUNT;
    return SYS_OK;
#else
    return brdimpl_vlan_reset();
#endif /* CFG_XGS_CHIP */
}

#ifdef BRD_VLAN_DEBUG
void _brdimpl_dump_vlan_info(void) REENTRANT
{
    uint16 i;
    soc_switch_t    *soc;
    vlan_list_t *this_vlan;
    pbmp_t  lpbmp,tag_lpbmp;

    soc = board_get_soc_by_unit(0);
    sal_printf("\nDUMP SW VLAN:type=%d,max_cnt=%d,cnt=%d,vlist=%p\n\t vlist:",
            (int)vlan_info.type,(int)vlan_info.max_count,
            (int)vlan_info.count,vlan_info.head);
    this_vlan = vlan_info.head;
    for (i=0;i<vlan_info.count;i++){
        SAL_ASSERT(this_vlan);
        sal_printf("\n\t VLAN_%d-> vid=%d,uplist=0x%x,next=%p", i,
                (int)this_vlan->vlan_id,(int)(*(uint8 *)this_vlan->uplist),
                this_vlan->next);

        if (soc->qvlan_port_get(0, this_vlan->vlan_id, &lpbmp, &tag_lpbmp) ==
                SYS_OK) {
         //   sal_printf(" (HW:lpbmp=0x%02x,tag_lpbmp=0x%02x)",
           //         (int)lpbmp, (int)tag_lpbmp);
        }
        this_vlan = this_vlan->next;
    }
    sal_printf("\n %d VLAN groups dumpped!\n",i);
}
#endif /* BRD_VLAN_DEBUG */

/*
 * VLAN reset to device default condition.
 *  1. VLAN type is NONE (actually at basic port based VLAN)
 *      - set to Basic PVLAN and all port at the same VLAN group #1
 *      - Disable 1Q VLAN and reset QVLAN table.
 *  2. VLAN related SW database init process.
 */
sys_error_t
brdimpl_vlan_reset(void) REENTRANT
{
    uint8  i;
    sys_error_t     rv;
    soc_switch_t    *soc;
    vlan_list_t     *this_vlan, *next_vlan;

    soc = board_get_soc_by_unit(0);
    /* init SW VLAN database
     *
     *  1. all ports in group 1
     *  2. all other existed VLAN group will be clear.
     *  3. count = 1
     *  4. type = NONE
     */
    this_vlan = vlan_info.head;
    if (this_vlan == NULL) {
        this_vlan = sal_malloc(sizeof(vlan_list_t));
        if (this_vlan == NULL) {
            return SYS_ERR_OUT_OF_RESOURCE;
        }
        vlan_info.head = this_vlan;
        this_vlan->next = NULL;
    }

    /* Default vlan id */
	this_vlan->vlan_id = VLAN_DEFAULT;

    /* retrieve all port uplist and copy to vlan->uplist */
    _brdimpl_uplist_allport_set(this_vlan->uplist);

    /* Destroy other vlan groups */
    this_vlan = this_vlan->next;
    while(this_vlan) {

        next_vlan = this_vlan->next;

#if CFG_XGS_CHIP
        /* Destroy vlan if it was QVLAN to save some work for XGS chip */
        if (vlan_info.type == VT_DOT1Q){
            (*soc->vlan_destroy)(0, this_vlan->vlan_id);
        }
#endif /* CFG_XGS_CHIP */

        /* free list */
        sal_free(this_vlan);

        this_vlan = next_vlan;
    }

    vlan_info.max_count = 1;
    vlan_info.count = 1;
    vlan_info.type = VT_NONE;
    vlan_info.head->next = NULL;
    vlan_info.tail = vlan_info.head;

    /* clear for port-base vlan */
    SAL_UPORT_ITER(i){
        /* each port include in vlan 1 */
        PVLAN_BMP_VLAN_SET(member_of_pvlan_bmp[i], 1);
    }

    /* SOC VLAN reset:
     *  1. VLAN_TYPE = NONE
     *  2. only one VLAN group created and all ports in this VLAN.
     */
    rv = (*soc->vlan_reset)(0);

    return rv;
}

/*
 * Select vlan type
 */
sys_error_t
brdimpl_vlan_type_set(vlan_type_t type) REENTRANT
{
    sys_error_t     rv;
    soc_switch_t    *soc;

    if (type >= VT_COUNT){
        return SYS_ERR_PARAMETER;
    }

    /* Still have to reset again if it stays in the same basic vlan type */
    if (vlan_info.type == type)
        return SYS_OK;

#if defined(CFG_SWITCH_MCAST_INCLUDED)
    /* Clean IGMPSNOOP database when vlan type change */
    igmpsnoop_database_init();
#endif /* defined(CFG_SWITCH_MCAST_INCLUDED) */

    rv = brdimpl_vlan_reset();
    if (rv){
        return rv;
    }

    soc = board_get_soc_by_unit(0);
    rv = (*soc->vlan_type_set)(0, type);
    if (rv){
        return rv;
    }
    
    vlan_info.type = type;
    if (type == VT_PORT_BASED)
    {
        vlan_info.max_count = board_uport_count();
        /* Basice port based vlan use one more VLAN for uplink port */
        vlan_info.max_count++;
        /*
         * Create all port-based vlans when changing types.
         * Vlan 1 is already created when doing vlan_reset.
         */
    } else if (type == VT_DOT1Q){
        vlan_info.max_count = BOARD_MAX_NUM_OF_QVLANS;
    }

    return rv;
}

sys_error_t
brdimpl_vlan_type_get(vlan_type_t *type) REENTRANT
{
    if (type == NULL){
        return SYS_ERR_PARAMETER;
    }

    *type = vlan_info.type;

    return SYS_OK;
}

/*
 * Port-based and 1Q-based VLAN creation
 */
sys_error_t
brdimpl_vlan_create(uint16 vlan_id) REENTRANT
{
    soc_switch_t    *soc;
    sys_error_t     rv;
    vlan_list_t     *this_vlan, *vlan;

    if (vlan_info.type == VT_DOT1Q){
        if (!SOC_1QVLAN_ID_IS_VALID(vlan_id)){
            return SYS_ERR_PARAMETER;
        }
    }
    else if (vlan_info.type == VT_PORT_BASED)
    {
        if (vlan_id > vlan_info.max_count){
            return SYS_ERR_PARAMETER;
        }
    } else {
        return SYS_ERR_STATE;
    }

    if (vlan_info.count == vlan_info.max_count) {
        return SYS_ERR_FULL;
    }

    /* check if no existed */
    rv = _brdimpl_vlan_sw_vlan_group_get(vlan_id, &this_vlan, FALSE);
    if (rv != SYS_ERR_NOT_FOUND){
        return SYS_ERR_EXISTS;
    }

    /* not existed, create vlan */
    soc = board_get_soc_by_unit(0);
    rv = (*soc->vlan_create)(0, vlan_info.type, vlan_id);
    if (rv) {
        return rv;
    }

    /* update SW VLAN database */
    vlan = sal_malloc(sizeof(vlan_list_t));
    if (vlan == NULL) {
        return SYS_ERR_OUT_OF_RESOURCE;
    }
    sal_memset(vlan->uplist, 0, MAX_UPLIST_WIDTH);

    vlan->vlan_id = vlan_id;
    vlan->next = NULL;
    if (vlan_info.tail != NULL) {
        vlan_info.tail->next = vlan;
    }
    vlan_info.tail = vlan;
    if (vlan_info.head == NULL) {
        vlan_info.head = vlan_info.tail;
    }

    vlan_info.count++;

    return rv;
}


/*
 * Port-based and 1Q-based VLAN destroy
 */
sys_error_t
brdimpl_vlan_destroy(uint16 vlan_id) REENTRANT
{
    sys_error_t     rv;
    soc_switch_t    *soc;
    vlan_list_t     *this_vlan;
    uint8      tmp_uplist[MAX_UPLIST_WIDTH];


    /* Clear physical port bit map of this VLAN to zero */
    if (vlan_info.type == VT_PORT_BASED) {

        sal_memset(tmp_uplist, 0 , sizeof(tmp_uplist));
        rv = brdimpl_pvlan_port_set(vlan_id, tmp_uplist);
        if (rv != SYS_OK) {
               return rv;
        }
    }

    /* check if no existed */
    rv = _brdimpl_vlan_sw_vlan_group_get(vlan_id, &this_vlan, TRUE);
    if (rv){
        /* rv is expected at SYS_OK if vlan is existed */
        return rv;
    }

    /* free list */
    sal_free(this_vlan);

    soc = board_get_soc_by_unit(0);
    if (vlan_info.type == VT_DOT1Q){
        /* soc->vlan_destroy only handle qvlan_destroy */
        rv = (*soc->vlan_destroy)(0, vlan_id);
    }
    return rv ;
}

sys_error_t
brdimpl_pvlan_port_set(uint16  vlan_id, uint8 *uplist) REENTRANT
{
    pbmp_t      this_lpbmp;
    uint8       unit, lport;
    sys_error_t rv = SYS_OK;
    soc_switch_t *soc;

    vlan_list_t *this_vlan, *tmp_vlist, *all_vlan;
    pbmp_t      egr[BOARD_MAX_NUM_OF_PORTS+1];
    uint8       uport, tmp_uplist[MAX_UPLIST_WIDTH];

    sal_memset(&this_lpbmp, 0, sizeof(pbmp_t));
    sal_memset(egr, 0, sizeof(egr));
    rv = _brdimpl_vlan_sw_vlan_group_get(vlan_id, &this_vlan, FALSE);
    if (rv){
        /* if this vlan is not found */
        return rv;
    } else {
        /* early return while VLAN member is the same with currnt setting. */
        if (sal_memcmp(this_vlan->uplist, uplist, MAX_UPLIST_WIDTH) == 0) {
            return SYS_OK;
        }
    }
    soc = board_get_soc_by_unit(0);
    /* rebuild the member_of_pvlan_bmp information :
         *
         *  - vlan_id here is the group id for PVLAN and it is one basis not zero
         *     basis value.
         */
    SAL_UPORT_ITER(uport)
    {
        /* Assign current value to egress mask for this port. */
        rv |= board_uplist_to_lpbmp(uplist, 0, &this_lpbmp);

        /* uplist_port_matched, return SYS_OK (=0) while matched. */
        if (uplist_port_matched(uplist, uport) == SYS_OK) {
            PVLAN_BMP_VLAN_ADD(member_of_pvlan_bmp[uport],vlan_id);
            /* if the port in this vlan, the uplist is the egr lpbmp in this vlan */
            BCM_PBMP_ASSIGN(egr[uport], this_lpbmp);

        } else {
            PVLAN_BMP_VLAN_REMOVE(member_of_pvlan_bmp[uport], vlan_id);
            /* if the port is not in this vlan, set block alll lpbmp in this vlan */
            BCM_PBMP_CLEAR(egr[uport]);
        }
        if (vlan_info.type == VT_PORT_BASED)
        {
            /* Update egress mask for each vlan this port belongs to */
            tmp_vlist = vlan_info.head;
            while(tmp_vlist) {

                if (tmp_vlist->vlan_id != vlan_id)
                {
                    if (PVLAN_BMP_MEMBER(member_of_pvlan_bmp[uport], tmp_vlist->vlan_id)) {
                        rv |= board_uplist_to_lpbmp(tmp_vlist->uplist, 0, &this_lpbmp);
                        BCM_PBMP_OR(egr[uport], this_lpbmp);
                    }
                }
                tmp_vlist = tmp_vlist->next;
            }
            rv |= board_uport_to_lport(uport, &unit, &lport);

            rv |= (*soc->pvlan_egress_set)(unit, lport, egr[uport]);
        } else {
            /* basic PORT_BASED, only updated egress_mask while exists in
             *  member_of_pvlan_bmp
             *
             * Porcess :
             *  1. Only member port of this VLAn need update the egress lpbmp.
             *  2. If this VLAN == VLAN_ALL(for Basic Port based VLAN only),
             *      the egress lpbmp is "ALL ports".
             *  3. If this VLAN != VLAN_ALL, and this port is not in VLAN_ALL
             *      the egress lpbmp is "this_vlan_member | vlan_all_member"
             */
            if (PVLAN_BMP_MEMBER(member_of_pvlan_bmp[uport], vlan_id)) {
                rv |= board_uport_to_lport(uport, &unit, &lport);
                if (vlan_id == (board_uport_count() + 1)) {
                    /* if this is ALL_VLAN */
                    _brdimpl_uplist_allport_set(tmp_uplist);
                    rv |= board_uplist_to_lpbmp(tmp_uplist, 0, &this_lpbmp);
                    BCM_PBMP_ASSIGN(egr[uport],this_lpbmp);
                    rv |= (*soc->pvlan_egress_set)(unit, lport, egr[uport]);
                } else {
                    /* if this is non-ALL_VLAN */
                    rv |= _brdimpl_vlan_sw_vlan_group_get(board_uport_count()+1, &all_vlan, FALSE);
                    if (!rv) {
                        if (uplist_port_matched(all_vlan->uplist, uport) != SYS_OK) {
                            /* if this port is not the member of VLAN_ALL */
                            rv |= board_uplist_to_lpbmp(all_vlan->uplist, 0, &this_lpbmp);
                            BCM_PBMP_OR(egr[uport],this_lpbmp);
                            rv |= (*soc->pvlan_egress_set)(unit, lport, egr[uport]);
                        }
                    } else {
                        /* means VLAN_ALL is not found!
                         *
                         * This is an unpxpected condition due to all possible VLANs
                         *  were created already while Basic PVLAN type was assigned.
                         */
                        return SYS_ERR_NOT_FOUND;
                    }
                }
            }
        }
    }
    /*
     *  - set to VLAN SW database of the new member ports in this Port
     *      based VLAN group
     */
    sal_memcpy(this_vlan->uplist, uplist, MAX_UPLIST_WIDTH);
    return rv;
}

sys_error_t
brdimpl_pvlan_port_get(uint16  vlan_id, uint8 *uplist) REENTRANT
{
    sys_error_t rv;
    vlan_list_t *this_vlan;

    /* go to this VLAN group */
    rv = _brdimpl_vlan_sw_vlan_group_get(vlan_id, &this_vlan, FALSE);
    if (rv){
        return rv;
    }
    sal_memcpy(uplist, this_vlan->uplist, MAX_UPLIST_WIDTH);
    return SYS_OK;
}

sys_error_t
brdimpl_pvlan_egress_get(uint16 uport, uint8 *uplist) REENTRANT
{
    pbmp_t          lpbmp;
    uint8           unit, lport;
    sys_error_t     rv = SYS_OK;
    soc_switch_t    *soc;
    vlan_type_t     type;

    if (uplist == NULL){
        return SYS_ERR_PARAMETER;
    }

    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    /* Only Pvlan will be checked here */
    rv = brdimpl_vlan_type_get(&type);
    if (rv){
        return rv;
    } else if (type != VT_PORT_BASED) {
        return SYS_ERR_PARAMETER;
    }

    soc = board_get_soc_by_unit(0);
    rv = soc->pvlan_egress_get(0, lport, &lpbmp);
    if (rv){
        return rv;
    }

    rv = board_lpbmp_to_uplist(0, lpbmp, uplist);

    return rv;
}

sys_error_t
brdimpl_qvlan_port_set(uint16  vlan_id, uint8 *uplist,
                                uint8 *taguplist) REENTRANT
{
    pbmp_t      lpbmp, tag_lpbmp;
    sys_error_t rv= SYS_OK;
    soc_switch_t    *soc;
    vlan_list_t     *this_vlan;

    if (uplist == NULL || taguplist == NULL){
        return SYS_ERR_PARAMETER;
    }
    if (!(vlan_info.type == VT_DOT1Q)){
        return SYS_ERR_STATE;
    }
    /* NOT FOUND error will be checked here */
    rv = _brdimpl_vlan_sw_vlan_group_get(vlan_id, &this_vlan, FALSE);
    if (rv){
        return rv;
    }

    soc = board_get_soc_by_unit(0);
    board_uplist_to_lpbmp(uplist, 0, &lpbmp);
    board_uplist_to_lpbmp(taguplist, 0, &tag_lpbmp);

    rv = soc->qvlan_port_set(0, vlan_id, lpbmp, tag_lpbmp);

    if (rv){
        return rv;
    }
    /* update SW database */
    sal_memcpy(this_vlan->uplist, uplist, MAX_UPLIST_WIDTH);

    return rv;
}

sys_error_t
brdimpl_qvlan_port_get(uint16  vlan_id, uint8 *uplist,
                                uint8 *taguplist) REENTRANT
{
    pbmp_t          lpbmp, tag_lpbmp;
    sys_error_t     rv;
    soc_switch_t    *soc;
    vlan_list_t     *this_vlan = NULL;

    if (uplist == NULL || taguplist == NULL){
        return SYS_ERR_PARAMETER;
    }
    /* NOT FOUND error will be checked here */
    rv = _brdimpl_vlan_sw_vlan_group_get(vlan_id, &this_vlan, FALSE);
    if (rv){
        return rv;
    }
    soc = board_get_soc_by_unit(0);

    rv = soc->qvlan_port_get(0, vlan_id, &lpbmp, &tag_lpbmp);

    if (rv){
        return rv;
    }

    rv = board_lpbmp_to_uplist(0, lpbmp, uplist);
    rv = board_lpbmp_to_uplist(0, tag_lpbmp, taguplist);

    return rv;
}

uint16
brdimpl_vlan_count(void) REENTRANT
{
    return vlan_info.count;
}

sys_error_t
brdimpl_qvlan_get_by_index(uint16  index, uint16 *vlan_id,
                                    uint8 *uplist, uint8 *taguplist) REENTRANT
{
    sys_error_t     rv;
    uint16          this_vid;
    soc_switch_t    *soc;
    vlan_list_t     *this_vlan;
    pbmp_t          lpbmp, tag_lpbmp;
    uint8           i;

    if (uplist == NULL || taguplist == NULL){
        return SYS_ERR_PARAMETER;
    }

    if (vlan_info.head == NULL) {
        return SYS_ERR_NOT_FOUND;
    }

    this_vlan = vlan_info.head;

    for (i = 0; i < index && this_vlan; i++) {
        this_vlan = this_vlan->next;
    }

    if (!this_vlan) {
        return SYS_ERR_NOT_FOUND;
    }

    this_vid = this_vlan->vlan_id;

    soc = board_get_soc_by_unit(0);

    rv = (*soc->qvlan_port_get)(0, this_vid, &lpbmp, &tag_lpbmp);

    if (rv) {
        return rv;
    }

    *vlan_id = this_vid;
    rv = board_lpbmp_to_uplist(0, lpbmp, uplist);
    rv = board_lpbmp_to_uplist(0, tag_lpbmp, taguplist);

    return rv;
}
#endif  /* CFG_SWITCH_VLAN_INCLUDED */

