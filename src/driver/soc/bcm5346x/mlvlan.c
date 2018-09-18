/*
 * $Id: mlvlan.c,v 1.6 Broadcom SDK $
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
#include "soc/bcm5346x.h"
#include "soc/port.h"
#include "brdimpl/vlan.h"

#ifdef CFG_SWITCH_VLAN_INCLUDED

extern vlan_info_t  vlan_info;

static vlan_type_t ml_vlan_type = VT_COUNT;
/*
 *  Function : bcm5346x_pvlan_egress_set
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
bcm5346x_pvlan_egress_set(uint8 unit, uint8 lport, pbmp_t lpbmp)
{
    sys_error_t rv = SYS_OK;
    uint32 entry[2], all_mask;
#ifdef CFG_SWITCH_LOOPDETECT_INCLUDED
    uint32 redirection_entry[2];
#endif
    all_mask = BCM5346X_ALL_PORTS_MASK;

    entry[0] = lpbmp;
    entry[1] = 0x0;

#ifdef CFG_SWITCH_LOOPDETECT_INCLUDED
    /* In the same time, update redirect profile for the port */
    bcm5346x_mem_get(0, M_IFP_REDIRECTION_PROFILE(lport), redirection_entry, 2);
    redirection_entry[0] &= (entry[0] & all_mask);
    bcm5346x_mem_set(0, M_IFP_REDIRECTION_PROFILE(lport), redirection_entry, 2);
#endif /* CFG_SWITCH_LOOPDETECT_INCLUDED */

    entry[0] = ~(entry[0] & all_mask);
    rv = bcm5346x_mem_set(0, M_EGR_MASK(lport), entry, 2);

    return rv;
}


/*
 *  Function : bcm5346x_pvlan_egress_get
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
bcm5346x_pvlan_egress_get(uint8 unit, uint8 lport, pbmp_t *lpbmp)
{
    sys_error_t rv = SYS_OK;
    uint32 entry[2];

    rv = bcm5346x_mem_get(0, M_EGR_MASK(lport), entry, 2);
    *lpbmp = (~entry[0]);

    return rv;
}

sys_error_t 
bcm5346x_qvlan_port_set(uint8 unit, uint16  vlan_id, pbmp_t lpbmp, pbmp_t tag_lpbmp)
{
    sys_error_t rv = SYS_OK;

    uint32 entry[7];
    pbmp_t untag_lpbmp;
    /* check if no exists */
    /* VLAN.VALID[56] */
    rv = bcm5346x_mem_get(0, M_VLAN(vlan_id), entry, 7);
    if((entry[1] & (1 << 24)) == 0) {
        return SYS_ERR;
    }
    entry[0] = lpbmp;
    rv = bcm5346x_mem_set(0, M_VLAN(vlan_id), entry, 7);

    untag_lpbmp = lpbmp & (~tag_lpbmp);
    /* EGR_VLAN.UT_PORT_BITMAP<190:159>*/
    rv = bcm5346x_mem_get(0, M_EGR_VLAN(vlan_id), entry, 7);
    entry[4] &= 0x7FFFFFFF;
    entry[4] |= (untag_lpbmp << 31);
    entry[5] &= 0x80000000;
    entry[5] |= (untag_lpbmp >> 1);
    /* EGR_VLAN.PORT_BITMAP<31:0>*/
    entry[0] = lpbmp;
    /* EGR_VLAN.VALID[56] */
    entry[1] |= (1 << 24);
    rv = bcm5346x_mem_set(0, M_EGR_VLAN(vlan_id), entry, 7);

    return rv;
}

 sys_error_t
bcm5346x_qvlan_port_get(uint8 unit, uint16  vlan_id, pbmp_t *lpbmp, pbmp_t *tag_lpbmp)
{
    sys_error_t rv = SYS_OK;

    uint32 entry[7];

    /* Check VALID[Bit 56] filed for existence */
    rv = bcm5346x_mem_get(0, M_VLAN(vlan_id), entry, 7);
    if((entry[1] & (1 << 24)) == 0) {
        return SYS_ERR;
    }

    rv = bcm5346x_mem_get(0, M_EGR_VLAN(vlan_id), entry, 7);
    *lpbmp = entry[0];
    *tag_lpbmp = entry[0] & ~((entry[4] >> 31) | (entry[5] << 1));

    return rv;
}

/*
 *  Function : bcm5346x_vlan_create
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
bcm5346x_vlan_create(uint8 unit, vlan_type_t type, uint16  vlan_id)
{
    sys_error_t rv = SYS_OK;
    uint32 entry[7];

    switch (type) {
        case VT_DOT1Q:
            /* Create vlan in VLAN_TAB 
                       * set VALID[Bit 56]=1,PORT_BITMAP<31:0>=0, VLAN_PROFILE_PTR<75:72>=0, STG<39:32>=1
                       */
            sal_memset(entry, 0, sizeof(entry));
            entry[1] = 0x01000001;
            rv = bcm5346x_mem_set(0, M_VLAN(vlan_id), entry, 7);

            /* Create vlan in EGR_VLAN with empty pbmp STG<39:32>=1 VALID[56]=1*/
            sal_memset(entry, 0, sizeof(entry));
            entry[1] = 0x01000001;
            rv = bcm5346x_mem_set(0, M_EGR_VLAN(vlan_id), entry, 7);
            break;
        default:
              /* VT_PORT_BASED */
            break;
    }

    return rv;
}


/*
 *  Function : _bcm5346x_vlan_destroy
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
bcm5346x_vlan_destroy(uint8 unit, uint16  vlan_id)
{

    sys_error_t rv;
    uint32 entry[7];

    sal_memset(entry, 0, sizeof(entry));
    /* destroy vlan in VLAN_TAB */
    rv = bcm5346x_mem_set(0, M_VLAN(vlan_id), entry, 7);

    /* destroy vlan in EGR_VLAN */
    rv = bcm5346x_mem_set(0, M_EGR_VLAN(vlan_id), entry, 7);

    return rv;
}


/*
 *  Function : bcm5346x_vlan_type_set
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
bcm5346x_vlan_type_set(uint8 unit, vlan_type_t type)
{
    int i;
    uint32 entry[7], port_entry[15];
    uint32 val;
    vlan_list_t     *this_vlan;
    
    sys_error_t rv = SYS_OK;
    if ((type == VT_NONE) || (type == VT_PORT_BASED)) 
        {

        /* Don't have to do this again if it was port based vlan */
        if ((ml_vlan_type == VT_NONE) || (ml_vlan_type == VT_PORT_BASED)) { 
             ml_vlan_type = type;
             return SYS_OK;
        }

        /* Enable USE_LEARN_VID and set LEARN_VID as '1' */
        bcm5346x_reg_get(0, R_VLAN_CTRL, &val);        
        val = (val & 0xffffe000) | 0x00001001;
        bcm5346x_reg_set(0, R_VLAN_CTRL, val);   

        /* clear EN_IFILTER[2] in PORT_TAB */ 
        for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
            bcm5346x_mem_get(0, M_PORT(i), port_entry, 15);
            port_entry[0] &= 0xfffffffb;
            bcm5346x_mem_set(0, M_PORT(i), port_entry, 15);
        }

        /* STG<39:32>=1, VALID[Bit 56]=1, PORT_BITMAP<31:0>=all except CPU, VLAN_PROFILE_PTR<75:72>=0 */
        sal_memset(entry, 0, sizeof(entry));
        entry[0] = BCM5346X_ALL_PORTS_MASK;
        entry[1] = 0x01000001;
        entry[2] = 0x0;
		entry[3] = 0x0;
        /* create 2-4094 vlans */
        for (i = 2; i <= 4094; i++) {
            bcm5346x_mem_set(0, M_VLAN(i), entry,  7);
        }

        /* All ports(exclude cpu) in vlan 2-4094 are tagged, STG=1 */
        sal_memset(entry, 0, sizeof(entry));
        entry[0] = BCM5346X_ALL_PORTS_MASK;
        entry[1] = 0x01000001;
        entry[2] = 0x0;
		entry[3] = 0x0;

        for (i = 2; i <= 4094; i++) {
            /* create vlan in EGR_VLAN */
            bcm5346x_mem_set(0, M_EGR_VLAN(i), entry,  7);
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
                sal_memset(entry, 0, sizeof(entry));
                entry[0] = BCM5346X_ALL_PORTS_MASK;
                entry[1] = 0x01000001;
                entry[2] = 0x0;
                entry[3] = 0x0;
                entry[4] &= 0x7FFFFFFF;	
            	entry[4] = (BCM5346X_ALL_PORTS_MASK << 31);
                entry[5] &= 0x80000000;	
            	entry[5] = BCM5346X_ALL_PORTS_MASK >> 1;	
            	entry[6] = 0;
                bcm5346x_mem_set(0, M_VLAN(1), entry, 7);
            
                /* Setup EGR_VLAN, all ports untagged(exclude cpu) */
                sal_memset(entry, 0, sizeof(entry));
                entry[0] = BCM5346X_ALL_PORTS_MASK;
                entry[1] = 0x01000001;
                entry[2] = 0x0;
                entry[3] = 0x0;
                entry[4] &= 0x7FFFFFFF;
                entry[4] |= (BCM5346X_ALL_PORTS_MASK << 31);
                entry[5] &= 0x80000000;
                entry[5] |= (BCM5346X_ALL_PORTS_MASK >> 1);    
                bcm5346x_mem_set(0, M_EGR_VLAN(1), entry, 7);
                
                /* Reset PVID to default vlan PORT_VID[32:21]*/
                for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
                    bcm5346x_mem_get(0, M_PORT(i), port_entry, 15);
                    port_entry[0] = (port_entry[0] & 0x001FFFFF) | ((this_vlan->vlan_id & 0xFFF)<< 21);
                    port_entry[1] = (port_entry[1] & 0xFFFFFFFE) | ((this_vlan->vlan_id >> 11) & 0x01);
                    rv = bcm5346x_mem_set(0, M_PORT(i), port_entry, 15);
                }
    
            }
        }

    } else if (type == VT_DOT1Q)
    {

        /* Don't have to do this again if it was 802.1Q vlan */
        if (ml_vlan_type == VT_DOT1Q) { 
             ml_vlan_type = type;
             return SYS_OK;
        }

        /* Disable USE_LEARN_VID and set LEARN_VID as '0' */
        bcm5346x_reg_get(0, R_VLAN_CTRL, &val);        
        val = val & 0xffffe000;
        bcm5346x_reg_set(0, R_VLAN_CTRL, val);        

        /* Enable EN_IFILTER[2] to check VLAN membership */ 
        for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
            bcm5346x_mem_get(0, M_PORT(i), port_entry,  15);
            port_entry[0] |= 0x00000004;
            bcm5346x_mem_set(0, M_PORT(i), port_entry,  15);
        }

        /* Clear VLAN and EGR_VLAN tables */
        sal_memset(entry, 0, sizeof(entry));
        for (i = 1; i <= 4094; i++) {
            if (i==VLAN_DEFAULT)
                continue;
                
            /* destroy vlan in VLAN_TAB */
            rv = bcm5346x_mem_set(0, M_VLAN(i), entry,  7);
            rv = bcm5346x_mem_set(0, M_EGR_VLAN(i), entry,  7);
        }
        /* end of egress_mask change */
    }

    ml_vlan_type = type;
    return rv;
}

/*
 *  Function : bcm5346x_vlan_reset
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
bcm5346x_vlan_reset(uint8 unit)
{
    sys_error_t rv = SYS_OK;
    int i;
    uint32 entry[15];
#if defined(CFG_SWITCH_LAG_INCLUDED) || defined(CFG_SWITCH_LOOPDETECT_INCLUDED)
    uint32 all_mask = BCM5346X_ALL_PORTS_MASK;
#endif /* CFG_SWITCH_LAG_INCLUDED || CFG_SWITCH_LOOPDETECT_INCLUDED */
#ifdef CFG_SWITCH_LAG_INCLUDED
    int j, k;
    uint32 lag_pbmp[BOARD_MAX_NUM_OF_LAG];
#endif /* CFG_SWITCH_LAG_INCLUDED */

    /* Setup default vlan */
    /* STG=1, VALID=1, PBMP=all except CPU, VLAN_PROFILE_PTR=0 */
    sal_memset(entry, 0, sizeof(entry));
    entry[0] = BCM5346X_ALL_PORTS_MASK;
    entry[1] = 0x01000001;
    entry[2] = 0x0;
    entry[3] = 0x0;
    entry[4] &= 0x7FFFFFFF;	
	entry[4] = (BCM5346X_ALL_PORTS_MASK << 31);
    entry[5] &= 0x80000000;	
	entry[5] = BCM5346X_ALL_PORTS_MASK >> 1;	
	entry[6] = 0;
    bcm5346x_mem_set(0, M_VLAN(VLAN_DEFAULT), entry, 7);

    /* Setup EGR_VLAN, all ports untagged(exclude cpu) */
    sal_memset(entry, 0, sizeof(entry));
    entry[0] = BCM5346X_ALL_PORTS_MASK;
    entry[1] = 0x01000001;
    entry[2] = 0x0;
    entry[3] = 0x0;
    entry[4] &= 0x7FFFFFFF;
    entry[4] |= (BCM5346X_ALL_PORTS_MASK << 31);
    entry[5] &= 0x80000000;
    entry[5] |= (BCM5346X_ALL_PORTS_MASK >> 1);    
    bcm5346x_mem_set(0, M_EGR_VLAN(VLAN_DEFAULT), entry, 7);

    /* Set VLAN_STG and EGR_VLAN_STG to forwading state*/
    sal_memset(entry, 0, sizeof(entry));
    entry[0] = 0x3fffffc;
    entry[1] = 0x0;
    entry[2] = 0x0;
    bcm5346x_mem_set(unit, M_VLAN_STG(1), entry, 3);
    bcm5346x_mem_set(unit, M_EGR_VLAN_STG(1), entry, 3);

    /* Clear egr_mask for reconstruct */
    entry[0] = 0x0;
    entry[1] = 0x0;
    for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
        rv = bcm5346x_mem_set(0, M_EGR_MASK(i), &entry[0], 2);
    }

    /* Recover egress_mask change in pvlan_port_set */
    for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
#ifdef CFG_SWITCH_LAG_INCLUDED
        /*  Revise the all_mask based on trunk port bitmap */
        all_mask = BCM5346X_ALL_PORTS_MASK;
        for (j = 0; j < BOARD_MAX_NUM_OF_LAG; j++) {
            if (lag_pbmp[j] != 0) {
                all_mask &= ~(lag_pbmp[j]);
                if (!(lag_pbmp[j] & (1 << i))) {
                    for (k = BCM5346X_LPORT_MIN; k <= BCM5346X_LPORT_MAX; k++) {
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
        bcm5346x_mem_get(0, M_IFP_REDIRECTION_PROFILE(i), entry, 2);
        entry[0] = all_mask;
        bcm5346x_mem_set(0, M_IFP_REDIRECTION_PROFILE(i), entry, 2);
#endif /* CFG_SWITCH_LOOPDETECT_INCLUDED */
    }

    /* Reset PVID to default vlan PORT_VID[32:21]*/
    for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
        bcm5346x_mem_get(0, M_PORT(i), entry, 15);
        entry[0] = (entry[0] & 0x001FFFFF) | ((VLAN_DEFAULT & 0xFFF)<< 21);
        entry[1] = (entry[1] & 0xFFFFFFFE) | ((VLAN_DEFAULT >> 11) & 0x01);
        rv = bcm5346x_mem_set(0, M_PORT(i), entry, 15);
    }

    return rv;
}
#endif /* CFG_SWITCH_VLAN_INCLUDED */
