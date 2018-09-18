/*
 * $Id: hr3port.c,v 1.9 Broadcom SDK $
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
#include "soc/bcm5343x.h"
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
static uint8 eee_state[BCM5343X_LPORT_MAX+1];
#endif /* CFG_SWITCH_EEE_INCLUDED */


#ifdef CFG_SWITCH_RATE_INCLUDED
/* Ingress rate limit: FP slice 1 */
void
bcm5343x_rate_init(void)
{
    uint8 pport;
    uint32 port_field_sel_entry[6];
    uint32 tcam_entry[15] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000, 0x00000000,
                              0x0027ff80, 0x00000000, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000 };

    uint32 policy_entry[8] = { 0x00000000, 0x00000000, 0x00000000, 0x00000060,
                               0x00100000, 0x00000000, 0x00000000, 0x00000000 };
    int idx = 0;
    uint32 xy_entry[15];
    uint32 global_tcam_mask_entry[3] = { 0x1, 0, 0 };

    /* Ingress rate limit: FP slice 1 */

    for (pport = BCM5343X_LPORT_MIN; pport <= BCM5343X_LPORT_MAX; pport++) {
        bcm5343x_mem_get(0, M_FP_PORT_FIELD_SEL(pport), port_field_sel_entry, 6);
        /* SrcPort: F3(11) in b'26:23 */
        port_field_sel_entry[0] = (port_field_sel_entry[0] & 0xc3ffffff)
                                  | (11 << 26);
	    bcm5343x_mem_set(0, M_FP_PORT_FIELD_SEL(pport), port_field_sel_entry, 6);
        tcam_entry[0] &= 0x1fffffff;
        tcam_entry[0] = (pport & 0x7) << 29;
        tcam_entry[1] &= 0xfffff000;
        tcam_entry[1] |= (pport >> 3);
        bcm5343x_dm_to_xy(tcam_entry, xy_entry, 15, 480);
        bcm5343x_mem_set(0, M_FP_TCAM(RATE_IGR_IDX + pport - BCM5343X_LPORT_MIN),
                                                                xy_entry, 15);
        bcm5343x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(RATE_IGR_IDX + pport - BCM5343X_LPORT_MIN),
                                                global_tcam_mask_entry, 3);

        policy_entry[3] &= 0xc00ffdff;
        if (pport % 2) {
            policy_entry[3] |= (idx << 20);
        } else {
            policy_entry[3] |= (idx << 20) | 0x00000200;
            idx ++;
        }
        bcm5343x_mem_set(0, M_FP_POLICY_TABLE(RATE_IGR_IDX
                            + (pport - BCM5343X_LPORT_MIN)), policy_entry, 8);
	}

}
#endif /* CFG_SWITCH_RATE_INCLUDED */

#ifdef CFG_SWITCH_QOS_INCLUDED
/* QoS: FP slice 2 */
void
bcm5343x_qos_init(void)
{
    int i, j;
    uint32 port_entry[12];
    uint32 cos_map[8] = { 1, 0, 0, 1, 2, 2, 3, 3} ;
    uint32 tcam_entry[15] = { 0x00000003, 0x00000000, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000, 0x00000000,
                              0x01f00000, 0x00000000, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000 };

    uint32 policy_entry[8] = { 0x00000000, 0x00000000, 0x00000000, 0x00000220,
                               0x05000000, 0x00000a00, 0x0000000a, 0x00000000};
    uint32 tcam_entry_port_based_qos[15], xy_entry[15], dm_entry[15];
    uint32 global_tcam_mask_entry[3] = { 0x1, 0, 0 };
    uint32 priority_val = 0x0;

    /* Using FP slice 2, entry 30~37, to make 1p priority take
     * precedence over DSCP
     */
    for (i = 0; i < 8; i++) {
        tcam_entry[1] = 0x00004000 | (i << 10);
        bcm5343x_dm_to_xy(tcam_entry, xy_entry, 15, 480);

        bcm5343x_mem_set(0, M_FP_TCAM(DOT1P_BASE_IDX + i), xy_entry, 15);
        bcm5343x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(DOT1P_BASE_IDX + i),
                                                global_tcam_mask_entry, 3);

        priority_val = (i << 10) | (i << 5) | (i << 0);
        policy_entry[0] = (policy_entry[0] & 0x001fffff)
                        | ((priority_val & 0x7ff) << 21);
        policy_entry[1] = (policy_entry[1] & 0xfffffff0)
                        | ((priority_val & 0xfffff800) >> 11);

        bcm5343x_mem_set(0, M_FP_POLICY_TABLE(DOT1P_BASE_IDX + i),
                                                            policy_entry, 8);
    }

    /*
     * Having 1p priority take precedence over DSCP:
     * Disable port base QoS via setting invalid bit in FP_TCAM for entry
     * 0_23 for port based QoS
     */
    for (i = BCM5343X_LPORT_MIN; i <= BCM5343X_LPORT_MAX; i++) {
        bcm5343x_mem_get(0, M_FP_TCAM(QOS_BASE_IDX + (i - BCM5343X_LPORT_MIN)),
                                                tcam_entry_port_based_qos, 15);
        bcm5343x_xy_to_dm(tcam_entry_port_based_qos, dm_entry, 15, 480);
        dm_entry[0] &= 0xfffffffc;
        bcm5343x_dm_to_xy(dm_entry, xy_entry, 15, 480);

        bcm5343x_mem_set(0, M_FP_TCAM(QOS_BASE_IDX + (i - BCM5343X_LPORT_MIN)),
                                                xy_entry, 15);
        bcm5343x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(QOS_BASE_IDX + (i - BCM5343X_LPORT_MIN)),
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
        bcm5343x_mem_set(0, M_COS_MAP(i), &cos_map[i], 1);
    }

    for (j = BCM5343X_LPORT_MIN; j <= BCM5343X_LPORT_MAX; j++) {
        /* Assign DSCP priority mapping */
        for (i = 0; i < 64; i++) {
            bcm5343x_mem_set(0, M_DSCP_TB(j * 64 + i), &dscp_table[i], 1);
        }
        /* Assign dot1p mapping to port_tab */
        for (i = 0; i < 16; i++) {
            bcm5343x_mem_set(0, M_ING_PRI_CNG_MAP(j * 16 + i), &dot1pmap[i], 1);
        }
    }

    /* Enable Trust_outer_dot1p/USE_INCOMING_DOT1P, mapping to ING_PRI_CNG_MAP. */
    for (i = BCM5343X_LPORT_MIN; i <= BCM5343X_LPORT_MAX; i++) {
	    bcm5343x_mem_get(0, M_PORT(i), port_entry, 12);
        port_entry[1] |= (0x1 << 28);
		port_entry[2] |= (0x1 << 23);
	    bcm5343x_mem_set(0, M_PORT(i), port_entry, 12);
	}
}

void
bcm5343x_dscp_map_enable(BOOL enable)
{
    int i;
    uint32 dscp_port_tab[12];

    for (i = BCM5343X_LPORT_MIN; i <= BCM5343X_LPORT_MAX; i++) {
        bcm5343x_mem_get(0, M_PORT(i), dscp_port_tab, 12);
        if (enable) {
            /* Enable TRUST_DSCP_V6 and TRUST_DSCP_V4 */
            dscp_port_tab[0] |= 0x18;
        } else {
            /* Disable TRUST_DSCP_V6 and TRUST_DSCP_V4 */
            dscp_port_tab[0] &= 0xffffffe7;
        }
        bcm5343x_mem_set(0, M_PORT(i), dscp_port_tab, 12);
    }
}
#endif /* CFG_SWITCH_QOS_INCLUDED */

#ifdef CFG_SWITCH_EEE_INCLUDED
void
bcm5343x_eee_init(void)
{
    uint8 i;

    for (i = BCM5343X_LPORT_MIN; i <= BCM5343X_LPORT_MAX; i++) {
        bmd_phy_eee_set(0, i, BMD_PHY_M_EEE_802_3);
        eee_state[i] = TRUE;
    }
}

/*
 *  Function : bcm5343x_port_eee_enable_set
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
bcm5343x_port_eee_enable_set(uint8 unit, uint8 port, uint8 enable, uint8 save)
{
    uint32 regval;

    bcm5343x_reg_get(unit, SOC_PORT_BLOCK(port),
                     R_UMAC_EEE_CTRL(SOC_PORT_BLOCK_INDEX(port)), &regval);
    /* EEE_EN is bit 3 of register UMAC_EEE_CTRL */
    if(enable == TRUE) {
        regval |= 0x8;
    } else {
        regval &= ~0x8;
    }
    bcm5343x_reg_set(unit, SOC_PORT_BLOCK(port),
                     R_UMAC_EEE_CTRL(SOC_PORT_BLOCK_INDEX(port)), regval);

    if(save == TRUE) {
        eee_state[port] = enable;
    }

    return SYS_OK;
}

/*
 *  Function : bcm5343x_port_eee_enable_get
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
bcm5343x_port_eee_enable_get(uint8 unit, uint8 port, uint8 *enable)
{
    *enable  = eee_state[port];
}
#endif /* CFG_SWITCH_EEE_INCLUDED */


