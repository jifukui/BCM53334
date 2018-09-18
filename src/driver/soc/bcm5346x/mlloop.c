/*
 * $Id: mlloop.c,v 1.8 Broadcom SDK $
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
#include "utils/system.h"
#include "utils/net.h"

#define ___constant_swab32(x) ((uint32)(				\
	(((uint32)(x) & (uint32)0x000000ffUL) << 24) |		\
	(((uint32)(x) & (uint32)0x0000ff00UL) <<  8) |		\
	(((uint32)(x) & (uint32)0x00ff0000UL) >>  8) |		\
	(((uint32)(x) & (uint32)0xff000000UL) >> 24)))


#ifdef CFG_SWITCH_LOOPDETECT_INCLUDED

/* Interval in us for timer callback routines. */
#define LOOP_DETECT_INTERVAL     (1000000UL)   /* 1 second in us. */

/* Interval in system tick to check if loop is removed. */
#define LOOP_REMOVED_INTERVAL    (2000UL)     /* 2 seconds in ms. */

#define DISCOVERY_PKTBUF_LEN      (68)

/* Discoery packet format. */
typedef struct discovery_packet_s {
    uint32 DA_upper;
    uint16 DA_lower;
    uint16 SA_upper;
    uint32 SA_lower;
    uint16 ether_type;
    uint16 pading;
    uint32 module_id_0;
    uint32 module_id_1;
    uint32 module_id_2;
} discovery_packet_t;

static uint8* packet_buffer = NULL;
static uint8 discovery_indicator;
static soc_tx_packet_t *spkt;
static uint32 loop_timer[BCM5346X_PORT_MAX+1];
static uint32 loop_counter[BCM5346X_PORT_MAX+1][3];
static BOOL loop_status[BCM5346X_PORT_MAX+1];
static uint8 loopdetect_status = FALSE;
extern uint32 _getticks(void);

/*
 *  Function : bcm533xx_loopdetect_task
 *
 *  Purpose :
 *      Check per-port discovery packet counter.
 *
 */
STATICFN void
bcm5346x_loopdetect_task(void *param)
{
    uint8 i, link;
    uint32 counter[3];
    uint32 policy[FP_POLICY_T_SIZE], counter_idx;
    uint32 val;

    if (FALSE == loopdetect_status) {
        return;
    }

    discovery_indicator++;

    if (discovery_indicator & 0x1) {
        /* Send discovery packet every 2 seconds */
        bcm5346x_tx(0, spkt);
    } else {
        /* Check discovery FP counter for each port */
        for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX ; i++) {
            bcm5346x_link_status(0, i, &link);
            bcm5346x_mem_get(0, M_FP_POLICY_TABLE(LOOP_COUNT_IDX - BCM5346X_LPORT_MIN +(i)), policy, FP_POLICY_T_SIZE);
            counter_idx = (policy[FP_POLICY_TABLE__COUNTER_INDEX_MINBIT/32] & FP_POLICY_TABLE__COUNTER_INDEX_MASK) 
        						>> (FP_POLICY_TABLE__COUNTER_INDEX_MINBIT%32);
        										
            if (link) {
                /* link up */
                if ((policy[FP_POLICY_TABLE__COUNTER_MODE_MINBIT/32] & FP_POLICY_TABLE__COUNTER_MODE_MASK) == 0x70000) {
                    bcm5346x_mem_get(0, M_FP_COUNTER_TABLE
                            (LOOP_COUNT_IDX + (2 * counter_idx)), counter, 3);
                } else {
                    bcm5346x_mem_get(0, M_FP_COUNTER_TABLE
                            (LOOP_COUNT_IDX + (2 * counter_idx + 1)), counter, 3);
                }

                /* Compared counter[0] is enough for packet/byte */
                if (counter[0] != loop_counter[i][0]) {

                    if (!loop_status[i]) {
                        loop_status[i] = TRUE;
                        /* Set bicolor selector 0 and 1 as Blink LED (0xA) */
                        phy_reg_write(i, 0x17, 0xF04);
                        phy_reg_write(i, 0x15, 0xAA);
                        /* Set LED Modes and Control, LED1 = LED2 = bicolor */
                        phy_reg_write(i, 0x1C, 0xB4AA);
                        /* Update LED status */
                        val = READCSR(LED_PORT_STATUS_OFFSET(i));
                        val |= 0x02;
                        WRITECSR(LED_PORT_STATUS_OFFSET(i), val);
                        sal_printf("Loop found on port %d\n", i);
                    }
                    loop_counter[i][0] = counter[0];
                    loop_counter[i][1] = counter[1];
                    loop_counter[i][2] = counter[2];
                    loop_timer[i] = sal_get_ticks();
                } else {
                    /*
                     * Assume loop has been removed if counter does not increment
                     * after LOOP_REMOVED_INTERVAL.
                     */
                    if (loop_status[i] &&
                        SAL_TIME_EXPIRED(loop_timer[i], LOOP_REMOVED_INTERVAL)) {
                        loop_status[i] = FALSE;
                        /* Set bicolor selector 0 as Link LED (0x8) */
                        phy_reg_write(i, 0x17, 0xF04);
                        phy_reg_write(i, 0x15, 0x8);
                        /* Set LED Modes and Control, LED1 = bicolor & LED2 = ACT */
                        phy_reg_write(i, 0x1C, 0xB43A);
                        val = READCSR(LED_PORT_STATUS_OFFSET(i));
                        val &= 0xFD;
                        WRITECSR(LED_PORT_STATUS_OFFSET(i), val);
                        sal_printf("Loop removed on port %d\n", i);
                    }
                } /* counter changed */
            } else {
                /* link down */
                if (loop_status[i] == TRUE) {
                    /* Clear loop status when link up -> down */
                    loop_status[i] = FALSE;
                    /* Set bicolor selector 0 as Link LED (0x8) */
                    phy_reg_write(i, 0x17, 0xF04);
                    phy_reg_write(i, 0x15, 0x8);
                    /* Set LED Modes and Control, LED1 = bicolor & LED2 = ACT */
                    phy_reg_write(i, 0x1C, 0xB43A);
                    sal_printf("Loop removed on port %d\n", i);
                }
            }
        } /* for each port */
    }
}

STATICFN void
loopdetect_cbk(struct soc_tx_packet_s *pkt)
{

}

STATICFN void
_bcm5346x_loop_detect_fp_init(void)
{
#ifdef CFG_SWITCH_LAG_INCLUDED
    /* We use real lagid(1~4)(the same as lagid on web page) instead of (0~3) to set the HW.
    *   lag_pbmp[0] is not not used so we use lag_pbmp[1]~lag_pbmp[BOARD_MAX_NUM_OF_LAG+1]
    */
    uint32 lag_pbmp[BOARD_MAX_NUM_OF_LAG+1][2];
    int k, j;
    uint32 val;
#endif /* CFG_SWITCH_LAG_INCLUDED */
    uint8 port_based_vlan = FALSE;
    uint32 egress_mask_entry[2];
    int i;
    discovery_packet_t *p;
    uint32 all_mask;
    uint32 xy_entry[FP_TCAM_T_SIZE];
    
	/*
	 * Ether type, SrcPort and UDF :
	 * SLICEX_F3=2,SLICEX_F2=8,SLICEX_F3=4
	 */
  	uint32 port_field_sel_entry[FP_PORT_FIELD_SEL_T_SIZE];

    /* table index UDF_TABLE_INDEX_LOOPDETECT_ONE_TAG(0) for single tag */
    uint32 fp_udf_offset_one_tag_entry[FP_UDF_OFFSET_T_SIZE] = 
			{   0x00000000, 0x00000000, 0x058a0000, 0x0078e1a3,
				0x00924900, 0x00000000, 0x00000000 };
    uint32 fp_udf_tcam_one_tag_entry[FP_UDF_TCAM_T_SIZE] = 
			{   0x00000025, 0x00008000, 0x007e0000, 0x80000000, 
				0x0000000d, 0x00000000 };

    
    /* table index UDF_TABLE_INDEX_LOOPDETECT_NO_TAG(1) for notag */
    uint32 fp_udf_offset_no_tag_entry[FP_UDF_OFFSET_T_SIZE] = 
			{   0x00000000, 0x00000000, 0x84880000, 0x0068c162,
				0x00924900, 0x00000000, 0x00000000};
    uint32 fp_udf_tcam_no_tag_entry[FP_UDF_TCAM_T_SIZE] = 
			{   0x00000025, 0x00000000, 0x007e0000, 0x80000000, 
				0x0000000d, 0x00000000 };
    					
    uint32 udf_xy_entry[FP_UDF_TCAM_T_SIZE]; 
	
    /* for offset = 16, srcport = 1 and UDF=0x112233445566778899aabbcc */
    uint32 tcam_entry[FP_TCAM_T_SIZE] = 
			{   0x00000003, 0x0221d000, 0x00000000, 0x00000000, 
				0x00000000, 0x00000000, 0x00000000, 0x00000000, 
				0xffc00000, 0x0000003f, 0xffffc000, 0xffffffff, 
				0xffffffff, 0x2fffffff, 0x00000000 };

    uint32 policy_entry[FP_POLICY_T_SIZE] = 
			{   0x00000000, 0x00000000, 0x00000000, 0x00000000, 
				0x00000088, 0x10000201, 0x00004000, 0x00000000, 
				0x00000000 };

    uint32 temp_val, temp[3];

    /* ether type = 0x8874, egress to front port */
    uint32 ethertype_tcam_entry[FP_TCAM_T_SIZE] = 
			{	0x00000003, 0x0221d000, 0x00000000, 0x00000000, 
				0x00000000, 0x00000000, 0x00000000, 0x00000000, 
				0xffc00000, 0x0000003f, 0x00000000, 0x00000000, 
				0x00000000, 0x2fffc000, 0x00000000 };
    /*
     * Use FP redirect to forward multicast SA dicovery packet otherwise it will
     * be dropped.
     */

    /* Action: update counter, new int. priority and redirectpbmp=0x3ffffffc */
    uint32 ethertype_policy_entry[FP_POLICY_T_SIZE] = 
			{   0x00000000, 0x00000000, 0x00000000, 0x00000000, 
				0x00000088, 0x0000a001, 0x0a000005, 0x00000000, 
				0x00000000 };

    uint32 redirection_entry[2] = {0x0, 0x0};
    uint32 global_tcam_mask_entry[FP_GLOBAL_TCAM_MASK_T_SIZE] = { 0x1, 0, 0, 0 };

    p = (discovery_packet_t *)packet_buffer;

#if (CFG_BIG_ENDIAN == 1)
    /* For GTR board */
    temp[0] = (p->module_id_0);
	temp[1] = (p->module_id_1);
	temp[2] = (p->module_id_2);
#else
    temp[0] = ___constant_swab32(p->module_id_0);
	temp[1] = ___constant_swab32(p->module_id_1);
	temp[2] = ___constant_swab32(p->module_id_2);
#endif
		
    tcam_entry[3] |= (temp[2] << 2);  
    tcam_entry[4] |= (temp[1] << 2) | (temp[2] >> 30);  
    tcam_entry[5] |= (temp[0] << 2) | (temp[1] >> 30);  
    tcam_entry[6] |= (temp[0] >> 30);  
    
    for (i = 0; i <= BCM5346X_LPORT_MAX; i++) {

        bcm5346x_mem_get(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, FP_PORT_FIELD_SEL_T_SIZE);
        /* Slice 3, F1/F2/F3 */
		port_field_sel_entry[FP_PORT_FIELD_SEL__SLICE3_F1_MINBIT/32] |= 
                (   (0xb << (FP_PORT_FIELD_SEL__SLICE3_F1_MINBIT%32)) | 
					(0x9 << (FP_PORT_FIELD_SEL__SLICE3_F2_MINBIT%32) ) | 
					(0x5 << (FP_PORT_FIELD_SEL__SLICE3_F3_MINBIT%32) ) );

        bcm5346x_mem_set(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, FP_PORT_FIELD_SEL_T_SIZE);
    }

#ifdef CFG_SWITCH_LAG_INCLUDED
    /* The USE_LEARN_VID and LEARN_VID will be set to '1' in VLAN_CTRL
     * for port based vlan */
    bcm5346x_reg_get(0, R_VLAN_CTRL, &val);
    if (val & 0x1001) {
        port_based_vlan = TRUE;
    }

    for (i = 1; i < (BOARD_MAX_NUM_OF_LAG+1); i++) {
        lag_pbmp[i][0] = 0;
        lag_pbmp[i][1] = 0;
        bcm5346x_mem_get(0, M_TRUNK_BITMAP(i), lag_pbmp[i], 2);
    }
#endif /* CFG_SWITCH_LAG_INCLUDED */
    for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
        /* Update port id. */
        tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] &= IFP_SINGLE_WIDE_F1_11__SGLP_UNSET;
        tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] |= (i << (IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT%32) ); 
        
#ifdef CFG_SWITCH_LAG_INCLUDED
        /*  Revise the source tgid qualify if the port is trunk port */
        for (k = 1; k < (BOARD_MAX_NUM_OF_LAG+1); k++) {
    		if (lag_pbmp[k][0] & (1 << (i))) {
                tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] &= IFP_SINGLE_WIDE_F1_11__SGLP_UNSET;
                tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] |= (k << (IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT%32) ); 
                tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] |= 0x20000;	
            }
        }
#endif /* CFG_SWITCH_LAG_INCLUDED */

        bcm5346x_dm_to_xy(tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

        bcm5346x_mem_set(0, M_FP_TCAM(LOOP_COUNT_IDX - BCM5346X_LPORT_MIN +(i)), xy_entry, FP_TCAM_T_SIZE);
        bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(LOOP_COUNT_IDX - BCM5346X_LPORT_MIN +(i)), global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);
        
		}
		
    temp_val = 0;
    for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
		if ((i % 2)) {
            policy_entry[FP_POLICY_TABLE__COUNTER_MODE_MINBIT/32] &= FP_POLICY_TABLE__COUNTER_MODE_UNSET;
            policy_entry[FP_POLICY_TABLE__COUNTER_MODE_MINBIT/32] |= (0x7 << (FP_POLICY_TABLE__COUNTER_MODE_MINBIT%32) );
            
            policy_entry[FP_POLICY_TABLE__COUNTER_INDEX_MINBIT/32] &= FP_POLICY_TABLE__COUNTER_INDEX_UNSET;
            policy_entry[FP_POLICY_TABLE__COUNTER_INDEX_MINBIT/32] |= (temp_val << (FP_POLICY_TABLE__COUNTER_INDEX_MINBIT%32) );
        } else {
            policy_entry[FP_POLICY_TABLE__COUNTER_MODE_MINBIT/32] &= FP_POLICY_TABLE__COUNTER_MODE_UNSET;
            policy_entry[FP_POLICY_TABLE__COUNTER_MODE_MINBIT/32] |= (0x38 << (FP_POLICY_TABLE__COUNTER_MODE_MINBIT%32) );
            
            policy_entry[FP_POLICY_TABLE__COUNTER_INDEX_MINBIT/32] &= FP_POLICY_TABLE__COUNTER_INDEX_UNSET;
            policy_entry[FP_POLICY_TABLE__COUNTER_INDEX_MINBIT/32] |= (temp_val << (FP_POLICY_TABLE__COUNTER_INDEX_MINBIT%32) );
            temp_val++;
        }
        /* fp action is Drop, no need to assign redirection entry  */
        bcm5346x_mem_set(0, M_FP_POLICY_TABLE(LOOP_COUNT_IDX - BCM5346X_LPORT_MIN +(i)), policy_entry, FP_POLICY_T_SIZE);
    }

    /* starting from CPU port */
    for (i = 0; i <= BCM5346X_LPORT_MAX; i++) {
		if ((i > 0) && (i < BCM5346X_LPORT_MIN)) {
            continue;
        }
        
        /* Update port id. */
        ethertype_tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] &= IFP_SINGLE_WIDE_F1_11__SGLP_UNSET;
        ethertype_tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] |= (i << (IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT%32) ); 

#ifdef CFG_SWITCH_LAG_INCLUDED
        /*  Revise the source tgid qualify if the port is trunk port */
        for (k = 1; k < (BOARD_MAX_NUM_OF_LAG+1); k++) {
    		if (lag_pbmp[k][0] & (1 << (i))) {
                ethertype_tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] &= IFP_SINGLE_WIDE_F1_11__SGLP_UNSET;
                ethertype_tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] |= (k << (IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT%32) ); 
                ethertype_tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] |= 0x20000;	
            }
        }
#endif /* CFG_SWITCH_LAG_INCLUDED */

        bcm5346x_dm_to_xy(ethertype_tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));
        bcm5346x_mem_set(0, M_FP_TCAM(LOOP_REDIRECT_IDX +(i)), xy_entry, FP_TCAM_T_SIZE);
        bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(LOOP_REDIRECT_IDX +(i)),
                                                    global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);

        all_mask = BCM5346X_ALL_PORTS_MASK;
        /* Remove ports for PVLAN setting */
        if (port_based_vlan == TRUE) {
            bcm5346x_mem_get(0, M_EGR_MASK(i), egress_mask_entry, 2);
            all_mask &= ~egress_mask_entry[0];
        }
        
#ifdef CFG_SWITCH_LAG_INCLUDED
        if(0){
            /*
            * When i==0, the source is CPU port. The all_mask turns out to be 0x1fe2 when port(1~4) are in trunk.
            * But redirection_entry[0] == 0x1fe2 makes no redirection when sending loop detection packet from CPU port. 
            * So I keep redirection_entry[0] == 0x1ffe for M_IFP_REDIRECTION_PROFILE(0) to let HW determine which port to forward when sending loop detection packet from CPU port.
            */
            
            /*  Revise the all_mask based on trunk port bitmap */
            for (k = 1; k < (BOARD_MAX_NUM_OF_LAG+1); k++) {
                
                if (lag_pbmp[k][0] != 0) {
                    all_mask &= ~(lag_pbmp[k][0]);
                    if (!(lag_pbmp[k][0] & (1 << i))) {
                        for (j = BCM5346X_LPORT_MIN; j <= BCM5346X_LPORT_MAX; j++) {
                            if (lag_pbmp[k][0] & (1 << j)) {
                                all_mask |= (1 << j);
                                break;
                            }
                        }
                    }
                }
            }
        }
#endif /* CFG_SWITCH_LAG_INCLUDED */

        /* Update redirect pbmp to exclude src port. */
        redirection_entry[0] = all_mask & ~(1 << i);
        bcm5346x_mem_set(0, M_IFP_REDIRECTION_PROFILE(i), redirection_entry, 2);
        
        /* starting from index 0 */
        ethertype_policy_entry[FP_POLICY_TABLE__G_PACKET_REDIRECTION_MINBIT/32] &= FP_POLICY_TABLE__G_PACKET_REDIRECTION_UNSET;
        ethertype_policy_entry[FP_POLICY_TABLE__REDIRECTION_PROFILE_INDEX_MINBIT/32] &= FP_POLICY_TABLE__REDIRECTION_PROFILE_INDEX_UNSET; 
        ethertype_policy_entry[FP_POLICY_TABLE__G_PACKET_REDIRECTION_MINBIT/32] |= 
        		(0x3 << (FP_POLICY_TABLE__G_PACKET_REDIRECTION_MINBIT%32) );
        ethertype_policy_entry[FP_POLICY_TABLE__REDIRECTION_PROFILE_INDEX_MINBIT/32] |= 
        		(i << (FP_POLICY_TABLE__REDIRECTION_PROFILE_INDEX_MINBIT%32) );
        

        
        bcm5346x_mem_set(0, M_FP_POLICY_TABLE(LOOP_REDIRECT_IDX +(i)),
                                                   ethertype_policy_entry, FP_POLICY_T_SIZE);
    }
		
    discovery_indicator = 0;
    
    /* UDF: single tag */
    bcm5346x_udf_dm_to_xy(fp_udf_tcam_one_tag_entry, udf_xy_entry, FP_UDF_TCAM_T_SIZE, (32*FP_UDF_TCAM_T_SIZE) );
    bcm5346x_mem_set(0, M_FP_UDF_OFFSET(UDF_TABLE_INDEX_LOOPDETECT_ONE_TAG), fp_udf_offset_one_tag_entry, FP_UDF_OFFSET_T_SIZE);
    bcm5346x_mem_set(0, M_FP_UDF_TCAM(UDF_TABLE_INDEX_LOOPDETECT_ONE_TAG), udf_xy_entry, FP_UDF_TCAM_T_SIZE);
    

    /* UDF: no tag */
    bcm5346x_udf_dm_to_xy(fp_udf_tcam_no_tag_entry, udf_xy_entry, FP_UDF_TCAM_T_SIZE, (32*FP_UDF_TCAM_T_SIZE) );
    bcm5346x_mem_set(0, M_FP_UDF_OFFSET(UDF_TABLE_INDEX_LOOPDETECT_NO_TAG), fp_udf_offset_no_tag_entry, FP_UDF_OFFSET_T_SIZE);
    bcm5346x_mem_set(0, M_FP_UDF_TCAM(UDF_TABLE_INDEX_LOOPDETECT_NO_TAG), udf_xy_entry, FP_UDF_TCAM_T_SIZE);
    
    
}

STATICFN void
_bcm5346x_loop_detect_init(uint16 port)
{
    uint8 sa[6] = { 0x01, 0x80, 0xC2, 0x0, 0x0, 0x1 };
    uint8 mac_addr[6];
    discovery_packet_t *pd;
    uint8 *p;
    int i;
    uint32 policy[FP_POLICY_T_SIZE], counter_idx;
    uint32 (*funcptr)(void) = (uint32 (*)(void))_getticks;

    packet_buffer = sal_dma_malloc(DISCOVERY_PKTBUF_LEN);
		
    sal_memset(packet_buffer, 0x0, DISCOVERY_PKTBUF_LEN);

    p = packet_buffer;
    /* Broadcast DA */
    for (i = 0; i < 6; i++) {
        *p++ = 0xFF;
    }
    for (i = 0; i < 6; i++) {
        *p++ = sa[i];
    }
    /* Ether type = 0x8874 */
    *(uint16 *)p = HTON16(0x8874);

    pd = (discovery_packet_t *)packet_buffer;
    get_system_mac(mac_addr);
    sal_memcpy(&pd->module_id_0, mac_addr, 6);
#if (CFG_BIG_ENDIAN == 1)
    /* For GTR board */
    packet_buffer[22] = 0x0A;
    packet_buffer[23] = 0;
#else
    sal_memcpy(&packet_buffer[22], &port, 2);
#endif

    pd->module_id_2 = (*funcptr)();
#ifdef SP_LOOP_DETECT_DEBUG
    sal_printf("module id 0 = 0x%08X\n", pd->module_id_0);
    sal_printf("module id 1 = 0x%08X\n", pd->module_id_1);
    sal_printf("module id 2 = 0x%08X\n", pd->module_id_2);
#endif

    spkt = (soc_tx_packet_t *)sal_malloc(sizeof(soc_tx_packet_t));
    if (spkt == NULL) {
        sal_printf("_bcm5346x_loop_detect_init: malloc failed!\n");
        return;
    }

    sal_memset(spkt, 0, sizeof(soc_tx_packet_t));
    spkt->buffer = packet_buffer;
    spkt->pktlen = DISCOVERY_PKTBUF_LEN;
    spkt->callback = loopdetect_cbk;
    _bcm5346x_loop_detect_fp_init();

    for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
        loop_counter[i][0] = 0;
        loop_counter[i][1] = 0;
        loop_counter[i][2] = 0;
        loop_status[i] = FALSE;

        bcm5346x_mem_get(0, M_FP_POLICY_TABLE(LOOP_COUNT_IDX - BCM5346X_LPORT_MIN +(i)), policy, FP_POLICY_T_SIZE);
        counter_idx = (policy[FP_POLICY_TABLE__COUNTER_INDEX_MINBIT/32] & FP_POLICY_TABLE__COUNTER_INDEX_MASK) 
        						>> (FP_POLICY_TABLE__COUNTER_INDEX_MINBIT%32);

        if ((policy[FP_POLICY_TABLE__COUNTER_MODE_MINBIT/32] & FP_POLICY_TABLE__COUNTER_MODE_MASK) == 0x70000) {
            bcm5346x_mem_set(0, M_FP_COUNTER_TABLE
                (LOOP_COUNT_IDX + (2 * counter_idx)), loop_counter[i], 3);
        } else {
            bcm5346x_mem_set(0, M_FP_COUNTER_TABLE
                (LOOP_COUNT_IDX + (2 * counter_idx + 1)), loop_counter[i], 3);
        }
    }

    /* Register background process to detect loop */
    timer_add(bcm5346x_loopdetect_task, NULL, LOOP_DETECT_INTERVAL);
}

void
bcm5346x_loop_detect_init(void)
{
    _bcm5346x_loop_detect_init(0xA);
}

void
bcm5346x_loop_detect_enable(BOOL enable)
{
    if (loopdetect_status == enable) {
        return;
    }

	if (!enable) {
        timer_remove(bcm5346x_loopdetect_task);
        if (spkt) {
            sal_free(spkt);
            spkt = (soc_tx_packet_t *)NULL;
        }
    } else {
        bcm5346x_loop_detect_init();
    }
    loopdetect_status = enable;
}

uint8
bcm5346x_loop_detect_status_get(void)
{
    return loopdetect_status;
}

#endif /* CFG_SWITCH_LOOPDETECT_INCLUDED */

