/*
 * $Id: hr2port.c,v 1.27 Broadcom SDK $
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
#include "soc/bcm5333x.h"
#include "soc/port.h"
#include "soc/soc.h"
#include "utils/ui.h"


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
/* save in Lport port index */
static uint8 eee_state[BCM5333X_LPORT_MAX+1];
#endif /* CFG_SWITCH_EEE_INCLUDED */

#define QOS_BASE_IDX               (2 * ENTRIES_PER_SLICE)
/* 802.1p higher than DSCP fp entries index */
#define DOT1P_BASE_IDX             (2 * ENTRIES_PER_SLICE + BCM5333X_LPORT_MAX + BCM5333X_LPORT_MIN + 1)
#define RATE_IGR_IDX               (1 * ENTRIES_PER_SLICE + 3)

#ifdef CFG_SWITCH_RATE_INCLUDED
/* Ingress rate limit: FP slice 1 */
void
bcm5333x_rate_init(void)
{
    uint8 pport;
    uint32 port_field_sel_entry[5];
    uint32 tcam_entry[15] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000, 0x00000000,
                              0xffc00000, 0x0000000b, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000 };

    uint32 policy_entry[8] = { 0x00000000, 0x00000000, 0x00000000, 0x00000030,
                               0x00080000, 0x00000000, 0x00000000, 0x00000000 };
    int idx = 0;
    uint32 xy_entry[15];
    uint32 global_tcam_mask_entry[3] = { 0x1, 0xffffffc0, 0x7ff };

    /* Ingress rate limit: FP slice 1 */

    for (pport = BCM5333X_PORT_MIN; pport <= BCM5333X_PORT_MAX; pport++) {
        bcm5333x_mem_get(0, M_FP_PORT_FIELD_SEL(pport), port_field_sel_entry, 5);
        port_field_sel_entry[0] |= 0x01000000;
        bcm5333x_mem_set(0, M_FP_PORT_FIELD_SEL(pport), port_field_sel_entry, 5);

        tcam_entry[1] = (pport << 12);
        bcm5333x_dm_to_xy(tcam_entry, xy_entry, 15, 234);

        bcm5333x_mem_set(0, M_FP_TCAM(RATE_IGR_IDX + pport - BCM5333X_PORT_MIN),
                                                                xy_entry, 15);
        bcm5333x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(RATE_IGR_IDX + pport - BCM5333X_PORT_MIN),
                                                global_tcam_mask_entry, 3);

        policy_entry[3] &= 0xe007feff;
        if (pport % 2) {
            policy_entry[3] |= (idx << 19);
        } else {
            policy_entry[3] |= (idx << 19) | 0x00000100;
            idx ++;
        }
        bcm5333x_mem_set(0, M_FP_POLICY_TABLE(RATE_IGR_IDX
                            + (pport - BCM5333X_PORT_MIN)), policy_entry, 8);
    }

}
#endif /* CFG_SWITCH_RATE_INCLUDED */

#ifdef CFG_SWITCH_QOS_INCLUDED
/* QoS: FP slice 2 */
void
bcm5333x_qos_init(void)
{
    int i, j;
    uint32 port_entry[8];
    uint32 cos_map[8] = { 1, 0, 0, 1, 2, 2, 3, 3} ;
    uint32 tcam_entry[15] = { 0x00000003, 0x00000000, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000, 0x00000000,
                              0x007c0000, 0x00000000, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000 };

    uint32 policy_entry[8] = { 0x00000000, 0x00000000, 0x00000000, 0x00000110,
                               0x02800000, 0x00000500, 0x00000005, 0x00000000};
    uint32 tcam_entry_port_based_qos[15], xy_entry[15], dm_entry[15];
    uint32 global_tcam_mask_entry[3] = { 0x1, 0xffffffc0, 0x7ff };
    uint32 priority_val = 0x0;

    /* Using FP slice 2, entry 24~31, to make 1p priority take precedence over DSCP */
    for (i = 0; i < 8; i++) {
        tcam_entry[1] = 0x00001000 | (i << 8);
        bcm5333x_dm_to_xy(tcam_entry, xy_entry, 15, 234);

        bcm5333x_mem_set(0, M_FP_TCAM(DOT1P_BASE_IDX + i), xy_entry, 15);
        bcm5333x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(DOT1P_BASE_IDX + i),
                                                global_tcam_mask_entry, 3);

        priority_val = (i << 10) | (i << 5) | (i << 0);
        policy_entry[0] = (policy_entry[0] & 0x001fffff)
                        | ((priority_val & 0x7ff) << 21);
        policy_entry[1] = (policy_entry[1] & 0xfffffff0)
                        | ((priority_val & 0xfffff800) >> 11);

        policy_entry[7] = 0x0;
        if ((i == 1) || (i == 2) || (i == 4) || (i == 7)) {
            policy_entry[7] = 0x20;
        }
        bcm5333x_mem_set(0, M_FP_POLICY_TABLE(DOT1P_BASE_IDX + i),
                                                            policy_entry, 8);
    }

    /*
     * Having 1p priority take precedence over DSCP:
     * Disable port base QoS via setting invalid bit in FP_TCAM for entry
     * 0_23 for port based QoS
     */
    for (i = BCM5333X_LPORT_MIN; i <= BCM5333X_LPORT_MAX; i++) {
        bcm5333x_mem_get(0, M_FP_TCAM(QOS_BASE_IDX + i), 
                                                tcam_entry_port_based_qos, 15);
        bcm5333x_xy_to_dm(tcam_entry_port_based_qos, dm_entry, 15, 234);
        dm_entry[0] &= 0xfffffffc;
        bcm5333x_dm_to_xy(dm_entry, xy_entry, 15, 234);

        bcm5333x_mem_set(0, M_FP_TCAM(QOS_BASE_IDX + i),
                                                xy_entry, 15);
        bcm5333x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(QOS_BASE_IDX + i),
                                                global_tcam_mask_entry, 3);

    }

    /*
     * Assign 1p priority mapping:
     *  pri_1/2 ==> low    (COS0)
     *  pri_0/3 ==> normal (COS1)
     *  pri_4/5 ==> medium (COS2)
     *  pri_6/7 ==> high   (COS3)
     */
    for (i = 0; i < 8; i++) {
        bcm5333x_mem_set(0, M_COS_MAP(i), &cos_map[i], 1);
    }

    for (j = BCM5333X_PORT_MIN; j <= BCM5333X_PORT_MAX; j++) {
        /* Assign DSCP priority mapping */
        for (i = 0; i < 64; i++) {
            bcm5333x_mem_set(0, M_DSCP_TB(j * 64 + i), &dscp_table[i], 1);
        }
        /* Assign dot1p mapping to port_tab */
        for (i = 0; i < 16; i++) {
            bcm5333x_mem_set(0, M_ING_PRI_CNG_MAP(j * 16 + i), &dot1pmap[i], 1);
        }
    }

    /* Enable Trust_outer_dot1p/USE_INCOMING_DOT1P, mapping to ING_PRI_CNG_MAP. */
    for (i = BCM5333X_LPORT_MIN; i <= BCM5333X_LPORT_MAX; i++) {
        bcm5333x_mem_get(0, M_PORT(i), port_entry, 8);
        port_entry[1] |= (0x1 << 25);
        port_entry[2] |= (0x1 << 26);
        bcm5333x_mem_set(0, M_PORT(i), port_entry, 8);
    }
}

void
bcm5333x_dscp_map_enable(BOOL enable)
{
    int i;
    uint32 dscp_port_tab[8];

    for (i = BCM5333X_LPORT_MIN; i <= BCM5333X_LPORT_MAX; i++) {
        bcm5333x_mem_get(0, M_PORT(i), dscp_port_tab, 8);
        if (enable) {
            /* Enable TRUST_DSCP_V6 and TRUST_DSCP_V4 */
            dscp_port_tab[0] |= 0x18;
        } else {
            /* Disable TRUST_DSCP_V6 and TRUST_DSCP_V4 */
            dscp_port_tab[0] &= 0xffffffe7;
        }
        bcm5333x_mem_set(0, M_PORT(i), dscp_port_tab, 8);
    }
}
#endif /* CFG_SWITCH_QOS_INCLUDED */

#ifdef CFG_SWITCH_EEE_INCLUDED
void
bcm5333x_eee_init(void)
{
    int block_idx, port_idx;
    uint32 lport;



    SOC_LPORT_ITER(lport) {
        /* Configure has been done in XLMAC init for XLPORT */
        if (!IS_XL_PORT(lport)) {

            block_idx = SOC_PORT_BLOCK(lport);
            port_idx = SOC_PORT_BLOCK_INDEX(lport);

            /* Initialize EEE wake and delay entry timer */
            bcm5333x_reg_set(0, block_idx, R_MII_EEE_WAKE_TIMER(port_idx), BCM5333X_PHY_EEE_100MB_MIN_WAKE_TIME);
            bcm5333x_reg_set(0, block_idx, R_GMII_EEE_WAKE_TIMER(port_idx), BCM5333X_PHY_EEE_1GB_MIN_WAKE_TIME);
            bcm5333x_reg_set(0, block_idx, R_MII_EEE_DELAY_ENTRY_TIMER(port_idx), 0x3C);
            bcm5333x_reg_set(0, block_idx, R_GMII_EEE_DELAY_ENTRY_TIMER(port_idx), 0x22);
            /* To derive 1usec clock ticks for EEE timers  for 250MHz ts_clk */
            bcm5333x_reg_set(0, block_idx, R_UMAC_EEE_REF_COUNT(port_idx), 0xFA);
            bmd_phy_eee_set(0,lport,BMD_PHY_M_EEE_802_3);
            eee_state[lport] = TRUE;
        }
    }
}

/*
 *  Function : bcm5333x_port_eee_enable_set
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
bcm5333x_port_eee_enable_set(uint8 unit, uint8 lport, uint8 enable, uint8 save)
{
    uint32 regval;
    int block_idx, port_idx;
    uint32 entry[2];

    block_idx = SOC_PORT_BLOCK(lport);
    port_idx = SOC_PORT_BLOCK_INDEX(lport);

    /* Configure has been done in XLMAC init for XLPORT */
    if (IS_XL_PORT(lport)) {
        bcm5333x_reg64_get(0, block_idx, R_XLMAC_EEE_CTRL(port_idx), entry, 2);
        /* EEE_EN is bit 0 of register XLMAC_EEE_CTRL */
        if(enable == TRUE) {
            entry[0] |= 0x1;
        } else {
            entry[0] &= ~0x1;
        }
        bcm5333x_reg64_set(0, block_idx, R_XLMAC_EEE_CTRL(port_idx), entry, 2);
    } else {
        bcm5333x_reg_get(0, block_idx, R_UMAC_EEE_CTRL(port_idx), &regval);
        /* EEE_EN is bit 3 of register UMAC_EEE_CTRL */
        if(enable == TRUE) {
            regval |= 0x8;
        } else {
            regval &= ~0x8;
        }
        bcm5333x_reg_set(0, block_idx, R_UMAC_EEE_CTRL(port_idx), regval);
    }

    if(save == TRUE) {
        eee_state[lport] = enable;
    }

    return SYS_OK;
}

/*
 *  Function : bcm5333x_port_eee_enable_get
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
bcm5333x_port_eee_enable_get(uint8 unit, uint8 port, uint8 *enable)
{
    *enable  = eee_state[port];
}
#endif /* CFG_SWITCH_EEE_INCLUDED */


void
bcm5333x_port_disacrd_mod_get(uint8 unit, uint8 lport,
                        uint32* disard)
{
    uint32 val = 0;
	soc_mem_info_t * tab_prt;
	soc_field_info_t *fieldinfo;
    if (disard == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }
    uint32 data[8];
	tab_prt = mem_arr[PORT_TAB_M];
	if (tab_prt == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }
	fieldinfo = &tab_prt->fields[PORT_DIS_TAGf];
	if (tab_prt == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }
	sal_memset(data, 0, 8 * sizeof(uint32));
	bcm5333x_mem_get(0, tab_prt->block_id, tab_prt->base + lport, data, 8);
	val = data[0] >> fieldinfo->bp;
	val &= 0x3;
	if (val == 3) {
        *disard = BCM_PORT_DISCARD_ALL;
	} else if (val == 2) {
        *disard = BCM_PORT_DISCARD_UNTAG;
	} else if (val == 1) {
        *disard = BCM_PORT_DISCARD_TAG;
	} else {
        *disard = BCM_PORT_DISCARD_NONE;
	}    
}

void
bcm5333x_port_stg_state_get(uint8 unit, uint8 lport,
                        uint32* stg)
{
	soc_mem_info_t * tab_prt;
	soc_field_info_t *fieldinfo;
	uint32 data[8];
	uint16 vlan_id = 0;
	uint16 stg_id = 0;
	uint16 stg_data = 0;
    if (stg == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }
	tab_prt = mem_arr[PORT_TAB_M];
    if (tab_prt == NULL) {		
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }
	fieldinfo = &tab_prt->fields[PORT_OVIDf];
	if (tab_prt == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }
	sal_memset(data, 0, 8 * sizeof(uint32));
	bcm5333x_mem_get(0, tab_prt->block_id, tab_prt->base + lport, data, 8);
	
	vlan_id = soc_memacc_field32_get(fieldinfo, data);

	tab_prt = mem_arr[VLAN_TAB_M];
    if (tab_prt == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }
    sal_memset(data, 0, 8 * sizeof(uint32));
	bcm5333x_mem_get(0, tab_prt->block_id, tab_prt->base + vlan_id, data, 8);
	fieldinfo = &tab_prt->fields[VLAN_STGf];
	if (tab_prt == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }
	stg_id = soc_memacc_field32_get(fieldinfo, data);

	tab_prt = mem_arr[VLAN_STG_TAB_M];
    if (tab_prt == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }
    sal_memset(data, 0, 8 * sizeof(uint32));
	bcm5333x_mem_get(0, tab_prt->block_id, tab_prt->base + stg_id, data, 8);
	if (lport > tab_prt->nFields) {
	    sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
		return;
	}
	fieldinfo = &tab_prt->fields[lport];
	if (tab_prt == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }
	stg_data = soc_memacc_field32_get(fieldinfo, data);	
	
	if (stg_data == 0) {
        *stg = BCM_STG_STP_DISABLE;
	} else if (stg_data == 1) {
        *stg = BCM_STG_STP_BLOCK;
	} else if (stg_data == 2) {
        *stg = BCM_STG_STP_LEARN;
	} else {
        *stg = BCM_STG_STP_FORWARD;
	}    
}



