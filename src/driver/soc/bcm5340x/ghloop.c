/*
 * $Id: ghloop.c,v 1.13 Broadcom SDK $
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
#include "soc/bcm5340x.h"
#include "soc/port.h"
#include "utils/system.h"
#include "utils/net.h"

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

static uint8 packet_buffer[DISCOVERY_PKTBUF_LEN]  __attribute__ ((section(".packet_buf"), aligned (32)));

static uint8 discovery_indicator;
static soc_tx_packet_t *spkt;
static uint32 loop_timer[BCM5340X_LPORT_MAX+1];
static uint32 loop_counter[BCM5340X_LPORT_MAX+1][3];
static BOOL loop_status[BCM5340X_LPORT_MAX+1];
static uint8 loopdetect_status = FALSE;
static uint8 loopdetect_phy_led1_mode = 0x3;
static uint8 loopdetect_phy_led2_mode = 0xA;
static uint16 loopdetect_phy_led_select = 0x180;
extern uint32 _getticks(void);

/*
 *  Function : bcm533xx_loopdetect_task
 *
 *  Purpose :
 *      Check per-port discovery packet counter.
 *
 */
STATICFN void
bcm5340x_loopdetect_task(void *param)
{
    uint8 i, link;
    uint32 counter[3];
    uint32 policy[8], counter_idx;
    uint32 val;

    if (FALSE == loopdetect_status) {
        return;
    }

    discovery_indicator++;

    if (discovery_indicator & 0x1) {
        /* Send discovery packet every 2 seconds */
        bcm5340x_tx(0, spkt);
    } else {
        /* Check discovery FP counter for each port */
        for (i = BCM5340X_LPORT_MIN; i <= BCM5340X_LPORT_MAX ; i++) {
            bcm5340x_link_status(0, i, &link);
            bcm5340x_mem_get(0, M_FP_POLICY_TABLE(LOOP_COUNT_IDX - BCM5340X_LPORT_MIN +(i)), policy, 8);
            counter_idx = policy[4] & 0x1f;
            if (link) {
                /* link up */
                if ((policy[3] & 0xf0000000) == 0x20000000) {
                    bcm5340x_mem_get(0, M_FP_COUNTER_TABLE
                            (LOOP_COUNT_IDX + (2 * counter_idx)), counter, 3);
                } else {
                    bcm5340x_mem_get(0, M_FP_COUNTER_TABLE
                            (LOOP_COUNT_IDX + (2 * counter_idx + 1)), counter, 3);
                }

                /* Compared counter[0] is enough for packet/byte */
                if (counter[0] != loop_counter[i][0]) {

                    if (!loop_status[i]) {
                        loop_status[i] = TRUE;
                        /* Set bicolor selector 0 and 1 as Blink LED (0xA) */
                        phy_reg_write(i, 0x17, 0xF04);
                        phy_reg_write(i, 0x15, 0xAA);
                        phy_reg_write(i, 0x17, 0x0);
                        /* Set LED Modes and Control, LED1 = LED2 = bicolor */
                        phy_reg_write(i, 0x1C, 0xB4AA);
                        /* Update LED status */
                        val = READCSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(i)));
                        val |= 0x02;
                        WRITECSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(i)), val);
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
                        /* Set bicolor selector 1 as Link LED (default) */
                        phy_reg_write(i, 0x17, 0xF04);
                        phy_reg_write(i, 0x15, loopdetect_phy_led_select);
                        phy_reg_write(i, 0x17, 0x0);
                        /* Set LED Modes and Control, LED2 = bicolor & LED1 = ACT (default) */
                        val = 0xB400 | 
                              (loopdetect_phy_led1_mode & 0xf) |
                              ((loopdetect_phy_led2_mode & 0xf) << 4) ;
                        phy_reg_write(i, 0x1C, val);
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
                    /* Set bicolor selector 1 as Link LED (default) */
                    phy_reg_write(i, 0x17, 0xF04);
                    phy_reg_write(i, 0x15, loopdetect_phy_led_select);
                    phy_reg_write(i, 0x17, 0x0);
                    /* Set LED Modes and Control, LED2 = bicolor & LED1 = ACT (default) */
                    val = 0xB400 | 
                          (loopdetect_phy_led1_mode & 0xf) |
                          ((loopdetect_phy_led2_mode & 0xf) << 4) ;
                    phy_reg_write(i, 0x1C, val);
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
_bcm5340x_loop_detect_fp_init(void)
{
#ifdef CFG_SWITCH_LAG_INCLUDED
    uint32 lag_pbmp[BOARD_MAX_NUM_OF_LAG];
    int k, j;
    uint32 val;
#endif /* CFG_SWITCH_LAG_INCLUDED */
    uint8 port_based_vlan = FALSE;
    uint32 egress_mask_entry;
    int i;
    discovery_packet_t *p;
    uint32 all_mask;
    uint32 xy_entry[15];
	/*
	 * Ether type, SrcPort and UDF :
	 * SLICEX_F3=2,SLICEX_F2=8,SLICEX_F3=4
	 */

    uint32 port_field_sel_entry[5];

	/* offset = 16: 0x00e18284 */
	uint32 fp_udf_offset_entry[3] = { 0x00e18284, 0x00000000, 0x00000000 };

    /* for offset = 16, srcport = 1 and UDF=0x112233445566778899aabbcc */

    uint32 tcam_entry[15] = { 0x00000003, 0x00000000, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x88740000, 0x00000000,
                              0x0007ffe0, 0xfc000000, 0xffffffff, 0xffffffff,
                              0x03ffffff, 0xfc000000, 0x000003ff};

    uint32 policy_entry[8] = { 0x00000000, 0x00000000, 0x00000000, 0x00000110,
                               0x00080400, 0x00200010, 0x00000000, 0x00000000};
    uint32 temp_val;

    /* ether type = 0x8874 */
    uint32 ethertype_tcam_entry[15] = {  0x00000003, 0x00000000, 0x00000000,
                                         0x00000000, 0x00000000, 0x00000000,
                                         0x88740000, 0x00000000, 0x0007ffe0,
                                         0x00000000, 0x00000000, 0x00000000,
                                         0x00000000, 0xfc000000, 0x000003ff};

    /*
     * Use FP redirect to forward multicast SA dicovery packet otherwise it will
     * be dropped.
     */

    /* Action: update counter, new int. priority and redirectpbmp=0x3ffffffc */
    uint32 ethertype_policy_entry[8] = { 0x00000000, 0x00000000, 0x00000000,
                                         0x00000110, 0x02800400, 0x00000500,
                                         0x00000005, 0x00000000 };

    uint32 redirection_entry[2] = {0x0, 0x0};
    uint32 global_tcam_mask_entry[3] = { 0x1, 0, 0 };

    p = (discovery_packet_t *)packet_buffer;

    tcam_entry[2] = ((p->module_id_2 & 0x00ff0000) << 8) | ((p->module_id_2 & 0xff000000) >> 8);
    tcam_entry[3] = ((p->module_id_1 & 0x00ff0000) << 8) | ((p->module_id_1 & 0xff000000) >> 8)
                  | ((p->module_id_2 & 0x000000ff) << 8) | ((p->module_id_2 & 0x0000ff00) >> 8);
    tcam_entry[4] = ((p->module_id_0 & 0x00ff0000) << 8) | ((p->module_id_0 & 0xff000000) >> 8)
                  | ((p->module_id_1 & 0x000000ff) << 8) | ((p->module_id_1 & 0x0000ff00) >> 8);
    tcam_entry[5] = ((p->module_id_0 & 0x000000ff) << 8) | ((p->module_id_0 & 0x0000ff00) >> 8);
   
    for (i = 0; i <= BCM5340X_LPORT_MAX; i++) {

        bcm5340x_mem_get(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, 5);
        /* Slice 3, F1/F2/F3 */
        port_field_sel_entry[1] |= (0xb << 21) | (0x8 << 17) | (0x4 << 13);
        bcm5340x_mem_set(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, 5);
    }

#ifdef CFG_SWITCH_LAG_INCLUDED
    /* The USE_LEARN_VID and LEARN_VID will be set to '1' in VLAN_CTRL
     * for port based vlan */
    bcm5340x_reg_get(0, R_VLAN_CTRL, &val);
    if (val & 0x1001) {
        port_based_vlan = TRUE;
    }

    for (i = 0; i < BOARD_MAX_NUM_OF_LAG; i++) {
        lag_pbmp[i] = 0;
        bcm5340x_mem_get(0, M_TRUNK_BITMAP(i), &(lag_pbmp[i]), 1);
    }
#endif /* CFG_SWITCH_LAG_INCLUDED */

    for (i = BCM5340X_LPORT_MIN; i <= BCM5340X_LPORT_MAX; i++) {
        /* Update port id. */
        tcam_entry[0] &= 0x07ffffff;
	    tcam_entry[0] |= (i << 27);

#ifdef CFG_SWITCH_LAG_INCLUDED
        /*  Revise the source tgid qualify if the port is trunk port */
        for (k = 0; k < BOARD_MAX_NUM_OF_LAG; k++) {
            if (lag_pbmp[k] & (1 << (i))) {
                tcam_entry[0] &= 0x07ffffff;
                tcam_entry[0] |= (k << 27);
                tcam_entry[1] |= 0x100;
            }
        }
#endif /* CFG_SWITCH_LAG_INCLUDED */
        bcm5340x_dm_to_xy(tcam_entry, xy_entry, 15, 480);

        bcm5340x_mem_set(0, M_FP_TCAM(LOOP_COUNT_IDX - BCM5340X_LPORT_MIN +(i)), xy_entry, 15);
        bcm5340x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(LOOP_COUNT_IDX - BCM5340X_LPORT_MIN +(i)), global_tcam_mask_entry, 3);
	}
    temp_val = 0;
    for (i = BCM5340X_LPORT_MIN; i <= BCM5340X_LPORT_MAX; i++) {

        if (!(i % 2)) {
            policy_entry[3] = (policy_entry[3] & 0x1fffffff) | (0x1 << 29);
            policy_entry[4] = (policy_entry[4] & 0xffffff80) | temp_val;
        } else {
            policy_entry[3] = (policy_entry[3] & 0x1fffffff) | (0x1 << 30);
            policy_entry[4] = (policy_entry[4] & 0xffffff80) | temp_val;
            temp_val++;
        }
        /* fp action is Drop, no need to assign redirection entry  */
	    bcm5340x_mem_set(0, M_FP_POLICY_TABLE(LOOP_COUNT_IDX - BCM5340X_LPORT_MIN +(i)), policy_entry, 8);
	}

	/* starting from CPU port */
	for (i = 0; i <= BCM5340X_LPORT_MAX; i++) {
		if ((i > 0) && (i < BCM5340X_LPORT_MIN)) {
        	continue;
    	}
    
		/* Update port id. */
        ethertype_tcam_entry[0] &= 0x07ffffff;
	    ethertype_tcam_entry[0] |= (i << 27);
        
#ifdef CFG_SWITCH_LAG_INCLUDED
        /*  Revise the source tgid qualify if the port is trunk port */
        for (k = 0; k < BOARD_MAX_NUM_OF_LAG; k++) {
            if (lag_pbmp[k] & (1 << (i))) {
                ethertype_tcam_entry[0] &= 0x07ffffff;
                ethertype_tcam_entry[0] |= (k << 27);
                ethertype_tcam_entry[1] |= 0x100;
            }
            
        }
#endif /* CFG_SWITCH_LAG_INCLUDED */
        bcm5340x_dm_to_xy(ethertype_tcam_entry, xy_entry, 15, 480);
        bcm5340x_mem_set(0, M_FP_TCAM(LOOP_REDIRECT_IDX +(i)), xy_entry, 15);
        bcm5340x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(LOOP_REDIRECT_IDX +(i)), 
                                                    global_tcam_mask_entry, 3);

        all_mask = BCM5340X_ALL_PORTS_MASK;
        /* Remove ports for PVLAN setting */
        if (port_based_vlan == TRUE) {
            bcm5340x_mem_get(0, M_EGR_MASK(i), &egress_mask_entry, 1);
            all_mask &= ~egress_mask_entry;
        }
        
#ifdef CFG_SWITCH_LAG_INCLUDED
        /*  Revise the all_mask based on trunk port bitmap */
        for (k = 0; k < BOARD_MAX_NUM_OF_LAG; k++) {
			if (lag_pbmp[k] != 0) {
				all_mask &= ~(lag_pbmp[k]);
				if (!(lag_pbmp[k] & (1 << i))) {
				    for (j = BCM5340X_LPORT_MIN; j <= BCM5340X_LPORT_MAX; j++){
				        if (lag_pbmp[k] & (1 << j)) {
				            all_mask |= (1 << j);
				            break;
				        }
				    }
				}
			}
		}
#endif /* CFG_SWITCH_LAG_INCLUDED */

        /* Update redirect pbmp to exclude src port. */
        redirection_entry[0] = all_mask & ~(1 << i);
		bcm5340x_mem_set(0, M_IFP_REDIRECTION_PROFILE(i), redirection_entry, 2);

        /* starting from index 0 */
        ethertype_policy_entry[1] &= ~(0xFF<<10);
		ethertype_policy_entry[1] |= (i)<<10;
		
		ethertype_policy_entry[5] |= 0x3 << 23;
				

		bcm5340x_mem_set(0, M_FP_POLICY_TABLE(LOOP_REDIRECT_IDX +(i)), ethertype_policy_entry, 8);

    }
    discovery_indicator = 0;
    /* Offset for ETHER II packets (N0 Tags/One Tag) */
    fp_udf_offset_entry[0] = 0x00e18284; /* No Tag*/
    bcm5340x_mem_set(0, M_FP_UDF_OFFSET(12), fp_udf_offset_entry, 3);
    /* One Tag */
    bcm5340x_mem_set(0, M_FP_UDF_OFFSET(140), fp_udf_offset_entry, 3);

}

STATICFN void
_bcm5340x_loop_detect_init(uint16 port)
{
    uint8 sa[6] = { 0x01, 0x80, 0xC2, 0x0, 0x0, 0x1 };
    uint8 mac_addr[6];
    discovery_packet_t *pd;
    uint8 *p;
    int i;
    uint32 policy[8], counter_idx;
    uint32 (*funcptr)(void) = (uint32 (*)(void))_getticks;
    
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
    sal_memcpy(&packet_buffer[22], &port, 2);
 
    pd->module_id_2 = (*funcptr)();
#ifdef SP_LOOP_DETECT_DEBUG
    sal_printf("module id 0 = 0x%08X\n", pd->module_id_0);
    sal_printf("module id 1 = 0x%08X\n", pd->module_id_1);
    sal_printf("module id 2 = 0x%08X\n", pd->module_id_2);
#endif

    spkt = (soc_tx_packet_t *)sal_malloc(sizeof(soc_tx_packet_t));
    if (spkt == NULL) {
        sal_printf("_bcm5340x_loop_detect_init: malloc failed!\n");
        return;
    }

    sal_memset(spkt, 0, sizeof(soc_tx_packet_t));
    spkt->buffer = packet_buffer;
    spkt->pktlen = DISCOVERY_PKTBUF_LEN;
    spkt->callback = loopdetect_cbk;
    _bcm5340x_loop_detect_fp_init();

    for (i = BCM5340X_LPORT_MIN; i <= BCM5340X_LPORT_MAX; i++) {
        loop_counter[i][0] = 0;
        loop_counter[i][1] = 0;
        loop_counter[i][2] = 0;
        loop_status[i] = FALSE;

        bcm5340x_mem_get(0, M_FP_POLICY_TABLE(LOOP_COUNT_IDX+(i)), policy, 8);
        counter_idx = (policy[4] & 0x3f);

        if ((policy[3] & 0xf0000000) == 0x20000000) {
            bcm5340x_mem_set(0, M_FP_COUNTER_TABLE
                (LOOP_COUNT_IDX + (2 * counter_idx)), loop_counter[i], 3);
        } else {
            bcm5340x_mem_set(0, M_FP_COUNTER_TABLE
                (LOOP_COUNT_IDX + (2 * counter_idx + 1)), loop_counter[i], 3);
        }
    }

    /* Register background process to detect loop */
    timer_add(bcm5340x_loopdetect_task, NULL, LOOP_DETECT_INTERVAL);
}

void
bcm5340x_loop_detect_init(void)
{
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
    sal_config_uint8_get(SAL_CONFIG_PHY_LED1_MODE, &loopdetect_phy_led1_mode);
    sal_config_uint8_get(SAL_CONFIG_PHY_LED2_MODE, &loopdetect_phy_led2_mode);
    sal_config_uint16_get(SAL_CONFIG_PHY_LED_SELECT, &loopdetect_phy_led_select);
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
    _bcm5340x_loop_detect_init(0xA);
}

void
bcm5340x_loop_detect_enable(BOOL enable)
{
    if (loopdetect_status == enable) {
        return;
    }

	if (!enable) {
        timer_remove(bcm5340x_loopdetect_task);
        if (spkt) {
            sal_free(spkt);
            spkt = (soc_tx_packet_t *)NULL;
        }
    } else {
        bcm5340x_loop_detect_init();
    }
    loopdetect_status = enable;
}

uint8
bcm5340x_loop_detect_status_get(void)
{
    return loopdetect_status;
}

#endif /* CFG_SWITCH_LOOPDETECT_INCLUDED */

