/*
 * $Id: mlport.c,v 1.12 Broadcom SDK $
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
#include "soc/bcm5346x.h"
#include "soc/port.h"

#ifdef CFG_SWITCH_QOS_INCLUDED
static uint32 dscp_table[64] = {
    0x00000040, 0x00000041, 0x00000042, 0x00000043, 0x00000044, 0x00000045, 0x00000046, 0x00000047,
    0x00000048, 0x00000049, 0x0000004a, 0x0000004b, 0x0000004c, 0x0000004d, 0x0000004e, 0x0000004f,
    0x000000d0, 0x000000d1, 0x000000d2, 0x000000d3, 0x000000d4, 0x000000d5, 0x000000d6, 0x000000d7,
    0x000000d8, 0x000000d9, 0x000000da, 0x000000db, 0x000000dc, 0x000000dd, 0x000000de, 0x000000df,
    0x00000160, 0x00000161, 0x00000162, 0x00000163, 0x00000164, 0x00000165, 0x00000166, 0x00000167,
    0x00000168, 0x00000169, 0x0000016a, 0x0000016b, 0x0000016c, 0x0000016d, 0x0000016e, 0x0000016f,
    0x000001f0, 0x000001f1, 0x000001f2, 0x000001f3, 0x000001f4, 0x000001f5, 0x000001f6, 0x000001f7,
    0x000001f8, 0x000001f9, 0x000001fa, 0x000001fb, 0x000001fc, 0x000001fd, 0x000001fe, 0x000001ff
};

static uint32 dot1pmap[16] = {
    0x00000000, 0x00000001, 0x00000004, 0x00000005, 0x00000008, 0x00000009, 0x0000000c, 0x0000000d,
    0x00000010, 0x00000011, 0x00000014, 0x00000015, 0x00000018, 0x00000019, 0x0000001c, 0x0000001d
};

#endif /* CFG_SWITCH_QOS_INCLUDED */

#ifdef CFG_SWITCH_EEE_INCLUDED
static uint8 eee_state[BCM5346X_LPORT_MAX+1];
#endif /* CFG_SWITCH_EEE_INCLUDED */


#ifdef CFG_SWITCH_RATE_INCLUDED
/* Ingress rate limit: FP slice 1 */
void
bcm5346x_rate_init(void)
{
    uint8 pport;
    uint32 port_field_sel_entry[FP_PORT_FIELD_SEL_T_SIZE];
    uint32 tcam_entry[FP_TCAM_T_SIZE] = 
    		{ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	        0x00000000, 0x00000000, 0x00000000, 0x00000000,
	        0x0023ffc0, 0x00000000, 0x00000000, 0x00000000,
	        0x00000000, 0x00000000, 0x00000000 };

    uint32 policy_entry[FP_POLICY_T_SIZE] = 
    		{ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
					0x00000018, 0x00000200, 0x00000000, 0x00000000,
					0x00000000 };
					
    uint32 xy_entry[FP_TCAM_T_SIZE];
    uint32 global_tcam_mask_entry[FP_GLOBAL_TCAM_MASK_T_SIZE] = { 0x1, 0, 0, 0 };

    /* Ingress rate limit: FP slice 1 */
    for (pport = BCM5346X_LPORT_MIN; pport <= BCM5346X_LPORT_MAX; pport++) {
        bcm5346x_mem_get(0, M_FP_PORT_FIELD_SEL(pport), port_field_sel_entry, FP_PORT_FIELD_SEL_T_SIZE);
        /* SrcPort: F3(11) in b'26:23 */
        port_field_sel_entry[FP_PORT_FIELD_SEL__SLICE1_F3_MINBIT/32] &= FP_PORT_FIELD_SEL__SLICE1_F3_UNSET;
				port_field_sel_entry[FP_PORT_FIELD_SEL__SLICE1_F3_MINBIT/32] |= (11 << FP_PORT_FIELD_SEL__SLICE1_F3_MINBIT);
	    	bcm5346x_mem_set(0, M_FP_PORT_FIELD_SEL(pport), port_field_sel_entry, FP_PORT_FIELD_SEL_T_SIZE);
	    	
	    	tcam_entry[IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT/32] &= IFP_SINGLE_WIDE_F3_11__SGLP_UNSET_LOWER;
	    	tcam_entry[IFP_SINGLE_WIDE_F3_11__SGLP_MAXBIT/32] &= IFP_SINGLE_WIDE_F3_11__SGLP_UNSET_HIGHER;	    	
				tcam_entry[IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT/32] |= (pport << (IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT%32) );
				tcam_entry[IFP_SINGLE_WIDE_F3_11__SGLP_MAXBIT/32] |= (pport >> (32 - (IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT%32) ) );
				
        bcm5346x_dm_to_xy(tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));
        bcm5346x_mem_set(0, M_FP_TCAM(RATE_IGR_IDX + pport - BCM5346X_LPORT_MIN),
                                                                xy_entry, FP_TCAM_T_SIZE);
        bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(RATE_IGR_IDX + pport - BCM5346X_LPORT_MIN),
                                                global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);

        policy_entry[FP_POLICY_TABLE__METER_PAIR_MODE_MODIFIER_BIT/32] &= FP_POLICY_TABLE__METER_PAIR_MODE_MODIFIER_UNSET;
        policy_entry[FP_POLICY_TABLE__METER_PAIR_MODE_MODIFIER_BIT/32] &= FP_POLICY_TABLE__METER_PAIR_INDEX_UNSET;
        if (pport & 0x1) {
        		policy_entry[FP_POLICY_TABLE__METER_PAIR_MODE_MODIFIER_BIT/32] |= FP_POLICY_TABLE__METER_PAIR_MODE_MODIFIER_MASK;
      	}
        policy_entry[FP_POLICY_TABLE__METER_PAIR_INDEX_MINBIT/32] |= ( (pport/2) << (FP_POLICY_TABLE__METER_PAIR_INDEX_MINBIT%32) );
        
        bcm5346x_mem_set(0, M_FP_POLICY_TABLE(RATE_IGR_IDX
                            + (pport - BCM5346X_LPORT_MIN)), policy_entry, FP_POLICY_T_SIZE);
	}

}
#endif /* CFG_SWITCH_RATE_INCLUDED */

#ifdef CFG_SWITCH_QOS_INCLUDED
/* QoS: FP slice 2 */
void
bcm5346x_qos_init(void)
{
    int i, j;
//    uint32 port_entry[FP_PORT_T_SIZE];
    uint32 cos_map[8] = { 1, 0, 0, 1, 2, 2, 3, 3} ;
    uint32 tcam_entry[FP_TCAM_T_SIZE] = 
    		{ 0x00000003, 0x00000000, 0x00000000, 0x00000000,
          0x00000000, 0x00000000, 0x00000000, 0x00000000,
          0x00f80000, 0x00000000, 0x00000000, 0x00000000,
          0x00000000, 0x00000000, 0x00000000 };

    uint32 policy_entry[FP_POLICY_T_SIZE] = 
				{ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
					0x00000088, 0x0000a000, 0x0a000005, 0x00000000,
					0x00000000 };

    uint32 tcam_entry_port_based_qos[FP_TCAM_T_SIZE], xy_entry[FP_TCAM_T_SIZE], dm_entry[FP_TCAM_T_SIZE];
    uint32 global_tcam_mask_entry[FP_GLOBAL_TCAM_MASK_T_SIZE] = { 0x1, 0, 0, 0 };
    uint32 priority_val = 0x0;

    /* Using FP slice 2, entry 30~37, to make 1p priority take
     * precedence over DSCP
     */
    for (i = 0; i < 8; i++) {
        tcam_entry[IFP_SINGLE_WIDE_F3_3__OUTER_VLAN_TAG_PRI_MINBIT/32] = 
        		0x00000800 | (i << (IFP_SINGLE_WIDE_F3_3__OUTER_VLAN_TAG_PRI_MINBIT%32) );
        bcm5346x_dm_to_xy(tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

        bcm5346x_mem_set(0, M_FP_TCAM(DOT1P_BASE_IDX + i), xy_entry, FP_TCAM_T_SIZE);
        bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(DOT1P_BASE_IDX + i),
                                                global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);

				priority_val = i & 0x7;
        policy_entry[FP_POLICY_TABLE__R_COS_INT_PRI_MINBIT/32] &= (FP_POLICY_TABLE__R_COS_INT_PRI_UNSET_LOWER); 
        policy_entry[FP_POLICY_TABLE__R_COS_INT_PRI_MAXBIT/32] &= 
        		(FP_POLICY_TABLE__R_COS_INT_PRI_UNSET_HIGHER & 
        		 FP_POLICY_TABLE__Y_COS_INT_PRI_UNSET & 
        		 FP_POLICY_TABLE__G_COS_INT_PRI_UNSET);
        
        policy_entry[FP_POLICY_TABLE__R_COS_INT_PRI_MINBIT/32] |= 
        		(priority_val << FP_POLICY_TABLE__R_COS_INT_PRI_MINBIT);
				policy_entry[FP_POLICY_TABLE__R_COS_INT_PRI_MAXBIT/32] |= 
        		 ( (priority_val >> (32-FP_POLICY_TABLE__R_COS_INT_PRI_MINBIT) ) | 
        		 (priority_val << (FP_POLICY_TABLE__Y_COS_INT_PRI_MINBIT%32) ) | 
        		 (priority_val << (FP_POLICY_TABLE__G_COS_INT_PRI_MINBIT%32) ) );
                        
        bcm5346x_mem_set(0, M_FP_POLICY_TABLE(DOT1P_BASE_IDX + i),
                                                            policy_entry, FP_POLICY_T_SIZE);
    }

    /*
     * Having 1p priority take precedence over DSCP:
     * Disable port base QoS via setting invalid bit in FP_TCAM for entry
     * 0_23 for port based QoS
     */
    for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
        bcm5346x_mem_get(0, M_FP_TCAM(QOS_BASE_IDX + (i - BCM5346X_LPORT_MIN)),
                                                tcam_entry_port_based_qos, FP_TCAM_T_SIZE);
        bcm5346x_xy_to_dm(tcam_entry_port_based_qos, dm_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));
        dm_entry[0] &= 0xfffffffc;
        bcm5346x_dm_to_xy(dm_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

        bcm5346x_mem_set(0, M_FP_TCAM(QOS_BASE_IDX + (i - BCM5346X_LPORT_MIN)),
                                                xy_entry, FP_TCAM_T_SIZE);
        bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(QOS_BASE_IDX + (i - BCM5346X_LPORT_MIN)),
                                                global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);

    }

    /*
     * Assign 1p priority mapping:
     *  pri_1/2 ==> low    (COS0)
     *  pri_0/3 ==> normal (COS1)
     *  pri_4/5 ==> medium (COS2)
     *  pri_6/7 ==> high   (COS3)
     */
    for (i = 0; i < 8; i++) {
        bcm5346x_mem_set(0, M_COS_MAP(i), &cos_map[i], 1);
    }

    for (j = BCM5346X_LPORT_MIN; j <= BCM5346X_LPORT_MAX; j++) {
        /* Assign DSCP priority mapping */
        for (i = 0; i < 64; i++) {
            bcm5346x_mem_set(0, M_DSCP_TB(j * 64 + i), &dscp_table[i], 1);
        }
        /* Assign dot1p mapping to port_tab */
        for (i = 0; i < 16; i++) {
            bcm5346x_mem_set(0, M_ING_PRI_CNG_MAP(j * 16 + i), &dot1pmap[i], 1);
        }
    }

}

void
bcm5346x_dscp_map_enable(BOOL enable)
{
    int i;
    uint32 dscp_port_tab[15];

    for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
        bcm5346x_mem_get(0, M_PORT(i), dscp_port_tab, 15);
        /* TRUST_DSCP_V4[71] TRUST_DSCP_V6[72] */
        if (enable) {
            /* Enable TRUST_DSCP_V6 and TRUST_DSCP_V4 */
            dscp_port_tab[0] |= (PORT_TABLE__V4V6_DSCP_ENABLE);
        } else {
            /* Disable TRUST_DSCP_V6 and TRUST_DSCP_V4 */
            dscp_port_tab[0] &= ~(PORT_TABLE__V4V6_DSCP_ENABLE);
        }
        bcm5346x_mem_set(0, M_PORT(i), dscp_port_tab, 15);
    }
}
#endif /* CFG_SWITCH_QOS_INCLUDED */

#ifdef CFG_SWITCH_EEE_INCLUDED
void
bcm5346x_eee_init(void)
{
    uint8 i;

    for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
        bmd_phy_eee_set(0, i, BMD_PHY_M_EEE_802_3);
        eee_state[i] = TRUE;
    }
}

/*
 *  Function : bcm5346x_port_eee_enable_set
 *
 *  Purpose :
 *      Set the EEE enable control of the specific port.
 *
 *  Parameters :
 *
 *  Return :
 *      SYS_OK : success
 *
 *  Note :
 *
 */
sys_error_t
bcm5346x_port_eee_enable_set(uint8 unit, uint8 port, uint8 enable, uint8 save)
{
    uint32 entry[2];

    bcm5346x_reg64_get(0, SOC_PORT_BLOCK(port), 
                     R_XLMAC_EEE_CTRL(SOC_PORT_BLOCK_INDEX(port)), entry, 2);
    /* EEE_EN is bit 0 of register XLMAC_EEE_CTRL */
    if(enable == TRUE) {
        entry[0] |= 0x1;
    } else {
        entry[0] &= ~0x1;
    }
    bcm5346x_reg64_set(0, SOC_PORT_BLOCK(port), 
                    R_XLMAC_EEE_CTRL(SOC_PORT_BLOCK_INDEX(port)), entry, 2);
    
    /* XLMAC EEE wake timer setting need to depend on the speed */
    /* Set these value in the linkscan link-up run time stage */

    if(save == TRUE) {
        eee_state[port] = enable;
    }

    return SYS_OK;
}

/*
 *  Function : bcm5346x_port_eee_enable_get
 *
 *  Purpose :
 *      Get the current EEE enable status of the specific port.
 *
 *  Parameters :
 *
 *  Return :
 *      SYS_OK : success
 *
 *  Note :
 *
 */
void
bcm5346x_port_eee_enable_get(uint8 unit, uint8 port, uint8 *enable)
{
    *enable  = eee_state[port];
}
#endif /* CFG_SWITCH_EEE_INCLUDED */


