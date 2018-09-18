/*
 * $Id: hr3vlan.c,v 1.13 Broadcom SDK $
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
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$f
 *
 */

#include "system.h"
#include "soc/bcm5343x.h"
#include "soc/port.h"
#include "brdimpl/vlan.h"

#ifdef CFG_SWITCH_VLAN_INCLUDED

extern vlan_info_t  vlan_info;

static vlan_type_t hr3_vlan_type = VT_COUNT;
/*
 *  Function : bcm5343x_pvlan_egress_set
 *
 *  Purpose :
 *      Set egr_mask in EGR_VLAN.
 *
 *  Parameters : 
 *
 *  Return :
 *
 *  Note :
 *
 */
 
sys_error_t 
bcm5343x_pvlan_egress_set(uint8 unit, uint8 lport, pbmp_t lpbmp)
{
    sys_error_t rv = SYS_OK;
    uint32 entry[2], all_mask;
#ifdef CFG_SWITCH_LOOPDETECT_INCLUDED
    uint32 redirection_entry[2];
#endif
    all_mask = BCM5343X_ALL_PORTS_MASK;

    entry[0] = lpbmp;
    entry[1] = 0x0;

#ifdef CFG_SWITCH_LOOPDETECT_INCLUDED
    /* In the same time, update redirect profile for the port */
    bcm5343x_mem_get(0, M_IFP_REDIRECTION_PROFILE(lport), redirection_entry, 2);
    redirection_entry[0] &= (entry[0] & all_mask);
    bcm5343x_mem_set(0, M_IFP_REDIRECTION_PROFILE(lport), redirection_entry, 2);
#endif /* CFG_SWITCH_LOOPDETECT_INCLUDED */

    entry[0] = ~(entry[0] & all_mask);
    rv = bcm5343x_mem_set(0, M_EGR_MASK(lport), entry, 2);

    return rv;
}


/*
 *  Function : bcm5343x_pvlan_egress_get
 *
 *  Purpose :
 *      Get egr_mask in EGR_VLAN.
 *
 *  Parameters : 
 *
 *  Return :
 *
 *  Note :
 *
 */
sys_error_t
bcm5343x_pvlan_egress_get(uint8 unit, uint8 lport, pbmp_t *lpbmp)
{
    sys_error_t rv = SYS_OK;
    uint32 entry[2];

    rv = bcm5343x_mem_get(0, M_EGR_MASK(lport), entry, 2);
    *lpbmp = (~entry[0]);

    return rv;
}

sys_error_t 
bcm5343x_qvlan_port_set(uint8 unit, uint16  vlan_id, pbmp_t lpbmp, pbmp_t tag_lpbmp)
{
    sys_error_t rv = SYS_OK;

    uint32 entry[4];
    pbmp_t untag_lpbmp;

    /* check if no exists */
    rv = bcm5343x_mem_get(0, M_VLAN(vlan_id), entry, 4);
    if((entry[1] & 0x100) != 0x100) {
        return SYS_ERR;
    }

    entry[0] = lpbmp;
    rv = bcm5343x_mem_set(0, M_VLAN(vlan_id), entry, 4);

    untag_lpbmp = lpbmp & (~tag_lpbmp);
    rv = bcm5343x_mem_get(0, M_EGR_VLAN(vlan_id), entry, 3);
    entry[0] = untag_lpbmp;
    entry[1] = lpbmp;
    rv = bcm5343x_mem_set(0, M_EGR_VLAN(vlan_id), entry, 3);

    return rv;
}

 sys_error_t
bcm5343x_qvlan_port_get(uint8 unit, uint16  vlan_id, pbmp_t *lpbmp, pbmp_t *tag_lpbmp)
{
    sys_error_t rv = SYS_OK;

    uint32 entry[4];

    /* Check VALID[Bit 40] filed for existence */
    rv = bcm5343x_mem_get(0, M_VLAN(vlan_id), entry, 4);
    if((entry[1] & 0x100) != 0x100) {
        return SYS_ERR;
    }

    rv = bcm5343x_mem_get(0, M_EGR_VLAN(vlan_id), entry, 3);
    *lpbmp = entry[1];
    *tag_lpbmp = entry[1] & (~entry[0]);

    return rv;
}

/*
 *  Function : bcm5343x_vlan_create
 *
 *  Purpose :
 *      Create vlan.
 *
 *  Parameters : 
 *
 *  Return :
 *
 *  Note :
 *
 */
sys_error_t
bcm5343x_vlan_create(uint8 unit, vlan_type_t type, uint16  vlan_id)
{
    sys_error_t rv = SYS_OK;
    uint32 entry[4];

    switch (type) {
        case VT_DOT1Q:
            /* Create vlan in VLAN_TAB
             * set VALID=1,PBMP=0, VLAN_PROFILE_PTR=0, STG=1
             */
            entry[0] = 0x0;
            entry[1] = 0x300;
            entry[2] = 0x0;
            entry[3] = 0x0;
            rv = bcm5343x_mem_set(0, M_VLAN(vlan_id), entry, 4);

            /* Create vlan in EGR_VLAN with empty pbmp */
            entry[0] = 0x0;
            entry[1] = 0x0;
            entry[2] = 0x101;
            rv = bcm5343x_mem_set(0, M_EGR_VLAN(vlan_id), entry, 3);
            break;
        default:
              /* VT_PORT_BASED */
            break;
    }

    return rv;
}


/*
 *  Function : _bcm5343x_vlan_destroy
 *
 *  Purpose :
 *      Destroy vlan.
 *
 *  Parameters : 
 *
 *  Return :
 *
 *  Note :
 *
 */
sys_error_t
bcm5343x_vlan_destroy(uint8 unit, uint16  vlan_id)
{

    sys_error_t rv;
    uint32 entry[4] = { 0x0, 0x0, 0x0, 0x0 };
    
    /* destroy vlan in VLAN_TAB */
    rv = bcm5343x_mem_set(0, M_VLAN(vlan_id), entry, 4);

    /* destroy vlan in EGR_VLAN */
    rv = bcm5343x_mem_set(0, M_EGR_VLAN(vlan_id), entry, 3);

    return rv;
}


/*
 *  Function : bcm5343x_vlan_type_set
 *
 *  Purpose :
 *      Set current vlan type.
 *
 *  Parameters : 
 *
 *  Return :
 *
 *  Note :
 *
 */
sys_error_t
bcm5343x_vlan_type_set(uint8 unit, vlan_type_t type)
{
    int i;
    uint32 entry[4], port_entry[12];
    uint32 val;
    vlan_list_t     *this_vlan;
    
    sys_error_t rv = SYS_OK;
    if ((type == VT_NONE) || (type == VT_PORT_BASED)) 
        {

        /* Don't have to do this again if it was port based vlan */
        if ((hr3_vlan_type == VT_NONE) || (hr3_vlan_type == VT_PORT_BASED)) { 
             hr3_vlan_type = type;
             return SYS_OK;
        }

        /* Enable USE_LEARN_VID and set LEARN_VID as '1' */
        bcm5343x_reg_get(0, R_VLAN_CTRL, &val);        
        val = (val & 0xffffe000) | 0x00001001;
        bcm5343x_reg_set(0, R_VLAN_CTRL, val);   

        /* clear EN_IFILTER in PORT_TAB */ 
        for (i = BCM5343X_LPORT_MIN; i <= BCM5343X_LPORT_MAX; i++) {
            bcm5343x_mem_get(0, M_PORT(i), port_entry, 12);
            port_entry[0] &= 0xffffffdf;
            bcm5343x_mem_set(0, M_PORT(i), port_entry, 12);
        }

        /* STG=1, VALID=1, PBMP=all except CPU, VLAN_PROFILE_PTR=0 */
        entry[0] = BCM5343X_ALL_PORTS_MASK;
        entry[1] = 0x300;
        entry[2] = 0x0;
		entry[3] = 0x0;
        /* create 2-4094 vlans */
        for (i = 2; i <= 4094; i++) {
            bcm5343x_mem_set(0, M_VLAN(i), entry, 4);
        }

        /* All ports(exclude cpu) in vlan 2-4094 are tagged, STG=1 */
        entry[0] = 0x0;
        entry[1] = BCM5343X_ALL_PORTS_MASK;
        entry[2] = 0x101;

        for (i = 2; i <= 4094; i++) {
            /* create vlan in EGR_VLAN */
            bcm5343x_mem_set(0, M_EGR_VLAN(i), entry, 3);
        }
        
        if (type == VT_PORT_BASED)
        {
            if (VLAN_DEFAULT != 1){
                this_vlan = vlan_info.head;
                if (this_vlan == NULL) {
                    sal_printf("%s..:%s..:this_vlan == NULL. Should not happen after brdimpl_vlan_reset()\n", __FILE__, __func__);
                    return SYS_ERR_OUT_OF_RESOURCE;
                }
                this_vlan->vlan_id = 1;
                            
                /* Re-overwrite the default VLAN to 1 for VT_PORT_BASED */
                /* Setup default vlan */
                /* STG=1, VALID=1, PBMP=all except CPU, VLAN_PROFILE_PTR=0 */
                entry[0] = BCM5343X_ALL_PORTS_MASK;
                entry[1] = 0x300;
                entry[2] = 0x0;
            	entry[3] = 0x0;
                bcm5343x_mem_set(0, M_VLAN(1), entry, 4);
            
                /* Setup EGR_VLAN, all ports untagged(exclude cpu) */
                entry[0] = BCM5343X_ALL_PORTS_MASK;
                entry[1] = BCM5343X_ALL_PORTS_MASK;
                entry[2] = 0x101;
                bcm5343x_mem_set(0, M_EGR_VLAN(1), entry, 3);
                
                /* Reset PVID to default vlan */
                for (i = BCM5343X_LPORT_MIN; i <= BCM5343X_LPORT_MAX; i++) {
                    bcm5343x_mem_get(0, M_PORT(i), port_entry, 12);
                    port_entry[0] = (port_entry[0] & 0x00ffffff) | (this_vlan->vlan_id << 24);
            		port_entry[1] = (port_entry[1] & 0xfffffff0) | ((this_vlan->vlan_id & 0xf00) >> 8);   
            
                    rv = bcm5343x_mem_set(0, M_PORT(i), port_entry, 12);
                }
    
            }
        }

    } else if (type == VT_DOT1Q)
    {

        /* Don't have to do this again if it was 802.1Q vlan */
        if (hr3_vlan_type == VT_DOT1Q) { 
             hr3_vlan_type = type;
             return SYS_OK;
        }

        /* Disable USE_LEARN_VID and set LEARN_VID as '0' */
        bcm5343x_reg_get(0, R_VLAN_CTRL, &val);        
        val = val & 0xffffe000;
        bcm5343x_reg_set(0, R_VLAN_CTRL, val);        

        /* Enable EN_IFILTER to check VLAN membership */ 
        for (i = BCM5343X_LPORT_MIN; i <= BCM5343X_LPORT_MAX; i++) {
            bcm5343x_mem_get(0, M_PORT(i), port_entry, 12);
            port_entry[0] |= 0x00000020;
            bcm5343x_mem_set(0, M_PORT(i), port_entry, 12);
        }

        /* Clear VLAN and EGR_VLAN tables */
        entry[0] = entry[1] = entry[2] = entry[3] = 0;
        for (i = 1; i <= 4094; i++) {
            if (i==VLAN_DEFAULT)
                continue;
                
            /* destroy vlan in VLAN_TAB */
            rv = bcm5343x_mem_set(0, M_VLAN(i), entry, 4);
            rv = bcm5343x_mem_set(0, M_EGR_VLAN(i), entry, 3);
        }
        /* end of egress_mask change */
    }

    hr3_vlan_type = type;
    return rv;
}

/*
 *  Function : bcm5343x_vlan_reset
 *
 *  Purpose :
 *      Clear all vlan related tables..
 *
 *  Parameters : 
 *
 *  Return :
 *
 *  Note :
 *
 */
sys_error_t
bcm5343x_vlan_reset(uint8 unit)
{
    sys_error_t rv = SYS_OK;
    int i;
    uint32 entry[12];
#if defined(CFG_SWITCH_LAG_INCLUDED) || defined(CFG_SWITCH_LOOPDETECT_INCLUDED)
    uint32 all_mask = BCM5343X_ALL_PORTS_MASK;
#endif /* CFG_SWITCH_LAG_INCLUDED || CFG_SWITCH_LOOPDETECT_INCLUDED */
#ifdef CFG_SWITCH_LAG_INCLUDED
    int j, k;
    uint32 lag_pbmp[BOARD_MAX_NUM_OF_LAG];
#endif /* CFG_SWITCH_LAG_INCLUDED */

    /* Setup default vlan */
    /* STG=1, VALID=1, PBMP=all except CPU, VLAN_PROFILE_PTR=0 */
    entry[0] = BCM5343X_ALL_PORTS_MASK;
    entry[1] = 0x300;
    entry[2] = 0x0;
	entry[3] = 0x0;
    bcm5343x_mem_set(0, M_VLAN(VLAN_DEFAULT), entry, 4);

    /* Setup EGR_VLAN, all ports untagged(exclude cpu) */
    entry[0] = BCM5343X_ALL_PORTS_MASK;
    entry[1] = BCM5343X_ALL_PORTS_MASK;
    entry[2] = 0x101;
    bcm5343x_mem_set(0, M_EGR_VLAN(VLAN_DEFAULT), entry, 3);

    /* Set VLAN_STG and EGR_VLAN_STG */
    entry[0] = 0xfffffff0;
    entry[1] = 0xffffffff;
    entry[2] = 0x0;
    bcm5343x_mem_set(unit, M_VLAN_STG(1), entry, 3);
    bcm5343x_mem_set(unit, M_EGR_VLAN_STG(1), entry, 3);

    /* Clear egr_mask for reconstruct */
    entry[0] = 0x0;
    entry[1] = 0x0;
    for (i = BCM5343X_LPORT_MIN; i <= BCM5343X_LPORT_MAX; i++) {
        rv = bcm5343x_mem_set(0, M_EGR_MASK(i), &entry[0], 2);
    }

    /* Recover egress_mask change in pvlan_port_set */
    for (i = BCM5343X_LPORT_MIN; i <= BCM5343X_LPORT_MAX; i++) {
#ifdef CFG_SWITCH_LAG_INCLUDED
        /*  Revise the all_mask based on trunk port bitmap */
        all_mask = BCM5343X_ALL_PORTS_MASK;
        for (j = 0; j < BOARD_MAX_NUM_OF_LAG; j++) {
            if (lag_pbmp[j] != 0) {
                all_mask &= ~(lag_pbmp[j]);
                if (!(lag_pbmp[j] & (1 << i))) {
                    for (k = BCM5343X_LPORT_MIN; k <= BCM5343X_LPORT_MAX; k++) {
                        if (lag_pbmp[j] & (1 << k)) {
                            all_mask |= (1 << k);
                            break;
                        }
                    }
                }
            }
        }
#endif /* CFG_SWITCH_LAG_INCLUDED */

#ifdef CFG_SWITCH_LOOPDETECT_INCLUDED
        bcm5343x_mem_get(0, M_IFP_REDIRECTION_PROFILE(i), entry, 2);
        entry[0] = all_mask;
        bcm5343x_mem_set(0, M_IFP_REDIRECTION_PROFILE(i), entry, 2);
#endif /* CFG_SWITCH_LOOPDETECT_INCLUDED */
    }

    /* Reset PVID to default vlan */
    for (i = BCM5343X_LPORT_MIN; i <= BCM5343X_LPORT_MAX; i++) {
        bcm5343x_mem_get(0, M_PORT(i), entry, 12);
        entry[0] = (entry[0] & 0x00ffffff) | (VLAN_DEFAULT << 24);
		entry[1] = (entry[1] & 0xfffffff0) | ((VLAN_DEFAULT & 0xf00) >> 8);   

        rv = bcm5343x_mem_set(0, M_PORT(i), entry, 12);
    }

    return rv;
}
#endif /* CFG_SWITCH_VLAN_INCLUDED */
