/*
 * $Id: mlswitch.c,v 1.44.2.4 Broadcom SDK $
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
#include "utils/net.h"

#define LINKSCAN_INTERVAL        (100000UL)   /* 100 ms */

#define PORT_LINK_UP                    (1)
#define PORT_LINK_DOWN                  (0)
#define COS_MOD_SP                       0
#define COS_MOD_WRR                      1  

bcm5346x_sw_info_t ml_sw_info;

/* Flow control is enabled on COSQ1 by default */
#ifdef CFG_FLOW_CONTROL_ENABLED_COSQ_OVERRIDE
#define CFG_FLOW_CONTROL_ENABLED_COSQ CFG_FLOW_CONTROL_ENABLED_COSQ_OVERRIDE
#else
#define CFG_FLOW_CONTROL_ENABLED_COSQ 1
#endif /* CFG_FLOW_CONTROL_ENABLED_COSQ_OVERRIDE */


/*TDM_A : 0:180MHz: 209 (19 * 11) Cycles Used */
#define ML_TDM_A_SIZE    204
#define ML_TDM_A_FREQ   180 
#define ML_TDM_A_ROW    19
#define ML_TDM_A_COL    11 
const static uint32 ml_tdm_56270_A_ref[ML_TDM_A_SIZE] = {
		/*Col:1*/		/*Col:2*/		/*Col:3*/		/*Col:4*/	/*Col:5*/	/*Col:6*/	 /*Col:7*/	  /*Col:8*/ /*Col:9*/	 /*Col:10*/ /*Col:11*/
	/*1*/	5,				6,				7,				8,		1,				9,				10, 	11, 		12, 		 5, 		2,
	/*2*/	6,				7,				8,				9,		3,				10, 			11, 	12, 		5,			 6, 		4,
	/*3*/	7,				8,				9,				10, 	ML_IDLE,		ML_IDLE,		11, 	12, 		5,			 6, 		7,
	/*4*/	8,				9,				10, 			11, 	1,				12, 			5,		6,			7,			 8, 		2,
	/*5*/	9,				10, 			11, 			12, 	3,				5,				6,		7,			8,			 9, 		4,
	/*6*/	10, 			11, 			12, 			5,		ML_LPBK,		6,				7,		8,			9,			 10,		ML_CMIC,
	/*7*/	11, 			12, 			5,				6,		ML_LPBK,		7,				8,		9,			10, 		 11,		ML_CMIC,
	/*8*/	12, 			5,				6,				7,		1,				8,				9,		10, 		11, 		 12,		2,
	/*9*/	5,				6,				7,				8,		3,				9,				10, 	11, 		12, 		 5, 		4,
	/*10*/	6,				7,				8,				9,		ML_IDLE,		ML_IDLE,		10, 	11, 		12, 		 5, 		6,
	/*11*/	7,				8,				9,				10, 	1,				11, 			12, 	5,			6,			 7, 		2,
	/*12*/	8,				9,				10, 			11, 	3,				12, 			5,		6,			7,			 8, 		4,
	/*13*/	9,				10, 			11, 			12, 	ML_LPBK,		5,				6,		7,			8,			 9, 		ML_CMIC,
	/*14*/	10, 			11, 			12, 			5,		ML_LPBK,		6,				7,		8,			9,			 10,		ML_CMIC,
	/*15*/	11, 			12, 			5,				6,		1,				7,				8,		9,			10, 		 11,		2,
	/*16*/	12, 			5,				6,				7,		3,				8,				9,		10, 		11, 		 12,		4,
	/*17*/	5,				6,				7,				8,		ML_IDLE,		ML_IDLE,		9,		10, 		11, 		 12,		5,
	/*18*/	6,				7,				8,				9,		ML_LPBK,		10, 			11, 	12, 		5,			 6, 		ML_IDLE,
	/*19*/	7,				8,				9,				10, 	11, 			12				/* ------------------ EMPTY ----------------------*/
};



/*TDM_C : 0:125MHz: 147 (21 * 7) Cycles Used */
#define ML_TDM_C_SIZE    140
#define ML_TDM_C_FREQ   125 
#define ML_TDM_C_ROW    21
#define ML_TDM_C_COL    7 
const static uint32 ml_tdm_56271_C_ref[ML_TDM_C_SIZE] = {
    /*Col:1*/       /*Col:2*/       /*Col:3*/       /*Col:4*/   /*Col:5*/   /*Col:6*/    /*Col:7*/   
/*1*/	5,		6,		7,		8,	1,		9,	     	ML_CMIC, 
/*2*/	5, 		6,		7,		8,	2,		10,		ML_LPBK,
/*3*/	5,		6,		7,		8,	3,       	11,		ML_IDLE,
/*4*/	5,		6,		7,		8,	4,		12,		/* EMPTY */
/*5*/	5,		6,		7,		8,	1,  		9,		/* EMPTY */
/*6*/	5,		6,		7,		8,	2,      	10,		ML_LPBK,
/*7*/	5,		6,		7,		8,	3,		11,		ML_CMIC,
/*8*/	5,		6,		7,		8,	4,		12,		ML_IDLE,
/*9*/	5,		6,		7,		8,	1,		9,		/* EMPTY */
/*10*/	5,		6,		7,		8,	2,		10,		ML_LPBK,
/*11*/	5,		6,		7,		8,	3,		11,		ML_IDLE,
/*12*/	5,		6,		7,		8,	4,		12,		/* EMPTY */
/*13*/	5,		6,		7,		8,	1,		9,		ML_CMIC,
/*14*/	5,		6,		7,		8,	2,		10,		ML_LPBK,
/*15*/	5,		6,		7,		8,	3,		11,		ML_IDLE,
/*16*/	5,		6,		7,		8,	4,		12,		/* EMPTY */	
/*17*/	5,		6,		7,		8,	1,		9,		/* EMPTY */
/*18*/	5,		6,		7,		8,	2,		10,		ML_LPBK,
/*19*/	5,		6,		7,		8,	3,		11,		ML_CMIC,
/*20*/	5,		6,		7,		8,	4,		12,		ML_IDLE,
/*21*/	5,		6,		7,		8,	ML_IDLE,	ML_IDLE		/* EMPTY */
};

/*
 * Viper : 1 ~ 4, Eagle #0 : 5 ~ 8, Eagle #1 : 9 ~ 12
 */
static const int p2l_mapping_53460[] = {
           0,  1,  2,  3,  4,  5,  6,  7,
           8,  9, 10, 11, 12
};

static const int port_speed_max_53460[] = {
      -1,  25,  25,  25,  25,  100,  100,  100,
      100,  100,  100,  100,  100
};

static const int port_speed_max_53461[] = {
      -1,  25,  25,  25,  25,  100,  100,  100,
      100,  25,  25,  25,  25
};

uint8 tsce_interface = CFG_TSCE_INTERFACE;
uint8 viper_interface = CFG_VIPER_INTERFACE;

#ifdef CFG_LED_MICROCODE_INCLUDED
/* Left LED : Link,   Right LED : TX/RX activity */
const unsigned char ledproc_ml_12p_1[] = {
 0x02, 0x01, 0x28, 0x60, 0xE3, 0x67, 0x70, 0x75,
 0x0D, 0x67, 0x50, 0x77, 0x0F, 0x67, 0x22, 0x06,
 0xE3, 0x80, 0xD2, 0x0D, 0x74, 0x02, 0x12, 0xE0,
 0x85, 0x05, 0xD2, 0x03, 0x71, 0x20, 0x52, 0x00,
 0x3A, 0x30, 0x32, 0x00, 0x32, 0x01, 0xB7, 0x97,
 0x75, 0x31, 0x12, 0xE4, 0xFE, 0xE3, 0x02, 0x01,
 0x50, 0x12, 0xE4, 0xFE, 0xE3, 0x95, 0x75, 0x41,
 0x85, 0x06, 0xE3, 0x67, 0x49, 0x75, 0xAB, 0x77,
 0x91, 0x16, 0xE0, 0xDA, 0x01, 0x71, 0x91, 0x77,
 0x9E, 0x12, 0xA0, 0xF8, 0x15, 0x1A, 0x00, 0x57,
 0x12, 0xA0, 0xF8, 0x15, 0x1A, 0x01, 0x75, 0x5F,
 0x12, 0xE4, 0xFE, 0xE3, 0x02, 0x01, 0x50, 0x12,
 0xE4, 0xFE, 0xE3, 0x95, 0x75, 0x68, 0x85, 0x57,
 0x16, 0xE0, 0xDA, 0x01, 0x71, 0x77, 0x77, 0x84,
 0x12, 0xA0, 0xF8, 0x15, 0x1A, 0x01, 0x57, 0x32,
 0x0E, 0x87, 0x32, 0x0E, 0x87, 0x32, 0x0E, 0x87,
 0x32, 0x0E, 0x87, 0x57, 0x32, 0x0F, 0x87, 0x32,
 0x0F, 0x87, 0x32, 0x0E, 0x87, 0x32, 0x0E, 0x87,
 0x57, 0x32, 0x0F, 0x87, 0x32, 0x0E, 0x87, 0x32,
 0x0E, 0x87, 0x32, 0x0E, 0x87, 0x57, 0x32, 0x0F,
 0x87, 0x32, 0x0F, 0x87, 0x32, 0x0E, 0x87, 0x32,
 0x0E, 0x87, 0x57, 0x32, 0x0E, 0x87, 0x32, 0x0E,
 0x87, 0x32, 0x0E, 0x87, 0x32, 0x0E, 0x87, 0x57,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


/* Left LED : TX/RX activity,   Right LED : Link */
const unsigned char ledproc_ml_12p_2[] = {
 0x02, 0x01, 0x28, 0x60, 0xE3, 0x67, 0x70, 0x75,
 0x0D, 0x67, 0x50, 0x77, 0x0F, 0x67, 0x22, 0x06,
 0xE3, 0x80, 0xD2, 0x0D, 0x74, 0x02, 0x12, 0xE0,
 0x85, 0x05, 0xD2, 0x03, 0x71, 0x20, 0x52, 0x00,
 0x3A, 0x30, 0x32, 0x00, 0x32, 0x01, 0xB7, 0x97,
 0x75, 0x31, 0x12, 0xE4, 0xFE, 0xE3, 0x02, 0x01,
 0x50, 0x12, 0xE4, 0xFE, 0xE3, 0x95, 0x75, 0x41,
 0x85, 0x06, 0xE3, 0x67, 0x49, 0x75, 0xAB, 0x77,
 0x91, 0x16, 0xE0, 0xDA, 0x01, 0x71, 0x91, 0x77,
 0x9E, 0x12, 0xA0, 0xF8, 0x15, 0x1A, 0x00, 0x57,
 0x12, 0xA0, 0xF8, 0x15, 0x1A, 0x01, 0x75, 0x5F,
 0x12, 0xE4, 0xFE, 0xE3, 0x02, 0x01, 0x50, 0x12,
 0xE4, 0xFE, 0xE3, 0x95, 0x75, 0x68, 0x85, 0x57,
 0x16, 0xE0, 0xDA, 0x01, 0x71, 0x77, 0x77, 0x84,
 0x12, 0xA0, 0xF8, 0x15, 0x1A, 0x01, 0x57, 0x32,
 0x0E, 0x87, 0x32, 0x0E, 0x87, 0x32, 0x0E, 0x87,
 0x32, 0x0E, 0x87, 0x57, 0x32, 0x0F, 0x87, 0x32,
 0x0F, 0x87, 0x32, 0x0E, 0x87, 0x32, 0x0E, 0x87,
 0x57, 0x32, 0x0E, 0x87, 0x32, 0x0F, 0x87, 0x32,
 0x0E, 0x87, 0x32, 0x0E, 0x87, 0x57, 0x32, 0x0F,
 0x87, 0x32, 0x0F, 0x87, 0x32, 0x0E, 0x87, 0x32,
 0x0E, 0x87, 0x57, 0x32, 0x0E, 0x87, 0x32, 0x0E,
 0x87, 0x32, 0x0E, 0x87, 0x32, 0x0E, 0x87, 0x57,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};



#endif /* CFG_LED_MICROCODE_INCLUDED */

#if CONFIG_METROLITE_EMULATION
int link_qt[BCM5346X_LPORT_MAX+1];

void
bcm5346x_link_change(uint8 port)
{
    link_qt[port] ^= 1;
}
#endif /* CONFIG_METROLITE_EMULATION */
static void
bcm5346x_handle_link_up(uint8 unit, uint8 lport, int changed, uint32 *flags)
{
    BOOL   tx_pause, rx_pause;
    int    duplex;
    uint32 speed;
    int an = 0;

    if (1 == changed) {
        /* Port changes to link up from link down */
#if CONFIG_METROLITE_EMULATION
            speed = 1000;
            duplex = TRUE;
			tx_pause = rx_pause = TRUE;
#else
        if (ml_sw_info.loopback[lport] == PORT_LOOPBACK_MAC) {
            /* Force link up in MAC loopback mode */
            speed = SOC_PORT_SPEED_MAX(lport);
            duplex = TRUE;
            an = tx_pause = rx_pause = FALSE;
        } else {
            int rv = 0;
            rv |= PHY_SPEED_GET(BMD_PORT_PHY_CTRL(unit, lport), &speed);
            rv |= PHY_DUPLEX_GET(BMD_PORT_PHY_CTRL(unit, lport), &duplex);
            rv |= PHY_AUTONEG_GET(BMD_PORT_PHY_CTRL(unit, lport), &an);
            if (an) {
                rv |= phy_pause_get(unit, lport, &tx_pause, &rx_pause);
                if (SOC_FAILURE(rv)) {
                    return;
                }
            } else {
                tx_pause = rx_pause = TRUE;
            }
        }
#endif /* CONFIG_METROLITE_EMULATION */

        SOC_PORT_LINK_STATUS(lport) = TRUE;
        SOC_PORT_SPEED_STATUS(lport) = speed;
        SOC_PORT_DUPLEX_STATUS(lport) = duplex ? TRUE : FALSE;
        SOC_PORT_AN_STATUS(lport) = an ? TRUE : FALSE;
        SOC_PORT_TX_PAUSE_STATUS(lport) = tx_pause;
        SOC_PORT_RX_PAUSE_STATUS(lport) = rx_pause;

#if CFG_CONSOLE_ENABLED
        sal_printf("\nlport %d (P:%d), speed = %d, duplex = %d, tx_pause = %d, rx_pause = %d, an = %d\n",
                   lport, SOC_PORT_L2P_MAPPING(lport), speed, duplex, tx_pause, rx_pause, an);
#endif /* CFG_CONSOLE_ENABLED */

        MAC_SPEED_SET(ml_sw_info.p_mac[lport], unit, lport, speed);

        MAC_DUPLEX_SET(ml_sw_info.p_mac[lport], unit, lport, duplex);

        /* Interface? */

        MAC_PAUSE_SET(ml_sw_info.p_mac[lport], unit, lport, tx_pause, rx_pause);

        if (ml_sw_info.loopback[lport] == PORT_LOOPBACK_MAC) {
            MAC_LOOPBACK_SET(ml_sw_info.p_mac[lport], unit, lport, TRUE);
        } else {
            MAC_LOOPBACK_SET(ml_sw_info.p_mac[lport], unit, lport, FALSE);
        }

        soc_phyctrl_notify(BMD_PORT_PHY_CTRL(unit, lport), PhyEvent_Speed, speed);
        soc_phyctrl_notify(BMD_PORT_PHY_CTRL(unit, lport), PhyEvent_Duplex, duplex);

        /* Enable the MAC. */
        MAC_ENABLE_SET(ml_sw_info.p_mac[lport], unit, lport, TRUE);

#if defined(CFG_SWITCH_EEE_INCLUDED)
        {
            uint8 eee_state;
            int eee_support;
            uint32 remote_eee = 0x0;
            int rv;
            /* check if the port is auto-neg, need to use remote eee */
            /* use bit 12 to show the remote eee is enable(1) or not(0) */
            bmd_phy_eee_get(unit,lport, &eee_support);
            if ((an) && (eee_support)) {
                rv = PHY_CONFIG_GET(BMD_PORT_PHY_CTRL(unit, lport),
                            PhyConfig_AdvEEERemote, &remote_eee, NULL);
                if ((rv == CDK_E_NONE) && (remote_eee & 0x7)) {
                    eee_state = TRUE;
                }
            } else {
                /* get local */
                bcm5346x_port_eee_enable_get(unit, lport, &eee_state);
            }

            if (eee_state == TRUE) {
                /* Enable EEE in UMAC_EEE_CTRL register after one second
                 * if EEE is enabled in S/W database
                 */
                ml_sw_info.link_up_time[lport] = sal_get_ticks();
                ml_sw_info.need_process_for_eee_1s[lport] = TRUE;
            }
        }
#endif /* CFG_SWITCH_EEE_INCLUDED */

        /* If everything has been completed */
        ml_sw_info.link[lport] = PORT_LINK_UP;
    } else {
        /* Port stays in link up state */
#if defined(CFG_SWITCH_EEE_INCLUDED)
        int eee_support;
        /* EEE one second delay for link up timer check */
        if ((ml_sw_info.need_process_for_eee_1s[lport]) &&
            (SAL_TIME_EXPIRED(ml_sw_info.link_up_time[lport], 1000))) {
#if CFG_CONSOLE_ENABLED
            SAL_DEBUGF(("EEE : enable eee for port %d\n", lport));
#endif /* CFG_CONSOLE_ENABLED */
            bmd_phy_eee_get(unit,lport, &eee_support);
            if (eee_support) {
                bcm5346x_port_eee_enable_set(unit, lport, TRUE, FALSE);
            } else {
                bcm5346x_port_eee_enable_set(unit, lport, FALSE, FALSE);
            }
            ml_sw_info.need_process_for_eee_1s[lport] = FALSE;
        }
#endif /* CFG_SWITCH_EEE_INCLUDED */
    }

#ifdef CFG_LED_MICROCODE_INCLUDED
        uint32 val;
        if ((changed == 1) && (ml_sw_info.link[lport] == PORT_LINK_UP)) {
            /* Update LED status */
            val = READCSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)));
            val |= 0x01;
            WRITECSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)), val);
        }
#endif /* CFG_LED_MICROCODE_INCLUDED */

}

void
bcm5346x_handle_link_down(uint8 unit, uint8 lport, int changed)
{
#if defined(CFG_SWITCH_EEE_INCLUDED)
    uint8 eee_state;
#endif /* CFG_SWITCH_EEE_INCLUDED */

#ifdef CFG_LOOPBACK_TEST_ENABLED
    if (1 == changed) {
#if CFG_CONSOLE_ENABLED
        if (board_linkdown_message) {
            sal_printf("port %d goes down!\n", lport);
        }
#endif /* CFG_CONSOLE_ENABLED */
	    ml_sw_info.link[lport] = PORT_LINK_DOWN;
    }
#else

    if (1 == changed) {
        SOC_PORT_LINK_STATUS(lport) = FALSE;
#if CFG_CONSOLE_ENABLED
        if (board_linkdown_message) {
            sal_printf("port %d goes down!\n", lport);
        }
#endif /* CFG_CONSOLE_ENABLED */

        /* Port changes to link down from link up */
        MAC_ENABLE_SET(ml_sw_info.p_mac[lport], unit, lport, FALSE);

#if defined(CFG_SWITCH_EEE_INCLUDED)
        bcm5346x_port_eee_enable_get(unit, lport, &eee_state);
        if (eee_state == TRUE) {
            /* Disable EEE in UMAC_EEE_CTRL register if EEE is enabled in S/W database */
#if CFG_CONSOLE_ENABLED
            SAL_DEBUGF(("EEE : disable eee for lport %d\n", lport));
#endif /* CFG_CONSOLE_ENABLED */
            bcm5346x_port_eee_enable_set(unit, lport, FALSE, FALSE);
            ml_sw_info.need_process_for_eee_1s[lport] = FALSE;
        }
#endif /* CFG_SWITCH_EEE_INCLUDED */

        ml_sw_info.link[lport] = PORT_LINK_DOWN;
    } else {
        /* Port stays in link down state */
    }
#endif /* CFG_LOOPBACK_TEST_ENABLED */

#ifdef CFG_LED_MICROCODE_INCLUDED
        uint32 val;
        if ((changed == 1) && (ml_sw_info.link[lport] == PORT_LINK_DOWN)) {
            /* Update LED status */
            val = READCSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)));
            val &= (~0x01);
            WRITECSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)), val);
        }
#endif /* CFG_LED_MICROCODE_INCLUDED */

}

/*
 *  Function : bcm5346x_linkscan_task
 *
 *  Purpose :
 *      Update the link status of switch ports.
 *
 *  Parameters :
 *
 *  Return :
 *
 *  Note :
 *
 */
void
bcm5346x_linkscan_task(void *param)
{
    uint8 unit = 0, lport;
    uint32 flags;
    int link;

    if (board_linkscan_disable) 
        return;

    SOC_LPORT_ITER(lport) {
#if CONFIG_METROLITE_EMULATION
        link = (int)link_qt[lport];
#else
        if (ml_sw_info.loopback[lport] == PORT_LOOPBACK_MAC) {
            /* Force link up in mac loopback mode */
            link = PORT_LINK_UP;
        } else {
            int rv = 0;
            int andone;
            rv = PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit, lport), &link, &andone);
            if (rv < 0) {
                continue;
            }
        }
#endif /* CONFIG_METROLITE_EMULATION */

        if (link == PORT_LINK_UP) {
            /* Link up */
            flags = 0;
            bcm5346x_handle_link_up(unit, lport,
                (ml_sw_info.link[lport] == PORT_LINK_DOWN) ? TRUE : FALSE, &flags);
        } else {
            bcm5346x_handle_link_down(unit, lport,
                (ml_sw_info.link[lport] == PORT_LINK_UP) ? TRUE : FALSE);
        }
    }
}

static void soc_hw_reset_control_init(uint8 unit) {
#ifndef __SIM__	
    uint32 val;
#endif
    /*
     * Reset the IPIPE and EPIPE block
     */
    bcm5346x_reg_set(unit, R_ING_HW_RESET_CONTROL_1, 0x0);
    /*
     * Set COUNT[Bit 17-0] to # entries in largest IPIPE table, L2_ENTRYm 0x4000
     * RESET_ALL [Bit 18] = 1, VALID [Bit 19] = 1
     */
    bcm5346x_reg_set(unit, R_ING_HW_RESET_CONTROL_2, 0xC4000);

    bcm5346x_reg_set(unit, R_EGR_HW_RESET_CONTROL_0, 0x0);
    /*
     * Set COUNT[Bit 15-0] to # entries in largest EPIPE table, EGR_VLAN 0x4000
     * RESET_ALL [Bit 16] = 1, VALID [Bit 17] = 1
     */
    bcm5346x_reg_set(unit, R_EGR_HW_RESET_CONTROL_1, 0x34000);

#ifndef __SIM__
    /* Wait for IPIPE memory initialization done. */
    do {
        /* Polling DONE[Bit 20] */
        bcm5346x_reg_get(unit, R_ING_HW_RESET_CONTROL_2, &val);
        if (val & (0x1 << 20)) {
            break;
        }
        
    } while (1);
#endif

#ifndef __SIM__

    /* Wait for EPIPE memory initialization done. */
    do {
        /* Polling DONE[Bit 18] */
        bcm5346x_reg_get(unit, R_EGR_HW_RESET_CONTROL_1, &val);
        if (val & (0x1 << 18)) {
            break;
        }

        
    } while (1);
#endif

    /*
     * Modify RESET_ALL [Bit 18] = 0, CMIC_REQ_ENABLE [Bit 21] = 1
     */
    bcm5346x_reg_get(unit, R_ING_HW_RESET_CONTROL_2, &val);
    val |= (0x1 << 21);
    val &= ~(0x1 << 18);
    bcm5346x_reg_set(unit, R_ING_HW_RESET_CONTROL_2, val);

    /*
     * Modify RESET_ALL [Bit 16] = 0, DONE [Bit 18] = 0;
     */
    bcm5346x_reg_get(unit, R_EGR_HW_RESET_CONTROL_1, &val);
    val &= ~(0x1 << 16);
    val &= ~(0x1 << 18);
    bcm5346x_reg_set(unit, R_EGR_HW_RESET_CONTROL_1, val);
}

static void
soc_xgxs_reset(uint8 unit, uint8 port)
{
    uint32      val;
#if CONFIG_METROLITE_EMULATION
    int         sleep_usec = 50000;
    int         lcpll = 0;
#else
    int         sleep_usec = 1100;
    int         lcpll = 1;
#endif /* CONFIG_METROLITE_EMULATION */

    if (IS_GX_PORT(port)) {
        /* XPORT_XGXS_CTRL
         * Txd10g_FIFO_RstB [Bit 12]
         * Txd1g_FIFO_RstB [Bit 11:8] = 1
         * REFSEL [Bit 7:5] = 1
         * RstB_PLL [Bit 4] = 1
         * RstB_MDIOREGS [Bit 3] = 1
         * RstB_HW [Bit 2] = 1
         * IDDQ [Bit 1] = 1
         * PwrDwn [Bit 0] = 1
         */
        
        /* XGXS MAC initialization steps. */
        /* Release reset (if asserted) to allow bigmac to initialize */
        bcm5346x_reg_get(unit, SOC_PORT_BLOCK(port), R_XPORT_XGXS_CTRL, &val);
        val &= ~(0x1 << 1); /* IDDQ */
        val &= ~(0x1 << 0); /* PWRDWN */
        val |= (0x1 << 2); /* RstB_HW */
        bcm5346x_reg_set(unit, SOC_PORT_BLOCK(port), R_XPORT_XGXS_CTRL, val);
        sal_usleep(sleep_usec);

        /* Power down and reset */
        bcm5346x_reg_get(unit, SOC_PORT_BLOCK(port), R_XPORT_XGXS_CTRL, &val);
        val |= (0x1 << 1); /* IDDQ */
        val |= (0x1 << 0); /* PWRDWN */
        val &= ~(0x1 << 2); /* RstB_HW */
        val &= ~(0xf << 8); /* TXD1G_FIFO_RSTB */
        val &= ~(0x1 << 12); /* TXD10G_FIFO_RSTB */
        val &= ~(0x1 << 3); /* RSTB_MDIOREGS */
        val &= ~(0x1 << 4); /* RSTB_PLL */
        bcm5346x_reg_set(unit, SOC_PORT_BLOCK(port), R_XPORT_XGXS_CTRL, val);
        sal_usleep(sleep_usec);
        
        /* Bring up both digital and analog clocks */
        bcm5346x_reg_get(unit, SOC_PORT_BLOCK(port), R_XPORT_XGXS_CTRL, &val);
        val &= ~(0x1 << 1); /* IDDQ */
        val &= ~(0x1 << 0); /* PWRDWN */
        bcm5346x_reg_set(unit, SOC_PORT_BLOCK(port), R_XPORT_XGXS_CTRL, val);
        sal_usleep(sleep_usec);
                
        /* Bring XGXS out of reset */
        bcm5346x_reg_get(unit, SOC_PORT_BLOCK(port), R_XPORT_XGXS_CTRL, &val);
        val |= (0x1 << 2); /* RstB_HW */
        bcm5346x_reg_set(unit, SOC_PORT_BLOCK(port), R_XPORT_XGXS_CTRL, val);
        sal_usleep(sleep_usec);
        
        /* Bring MDIO registers out of reset */
        bcm5346x_reg_get(unit, SOC_PORT_BLOCK(port), R_XPORT_XGXS_CTRL, &val);
        val |= (0x1 << 3); /* RSTB_MDIOREGS */
        bcm5346x_reg_set(unit, SOC_PORT_BLOCK(port), R_XPORT_XGXS_CTRL, val);
        
        /* Activate all clocks */
        bcm5346x_reg_get(unit, SOC_PORT_BLOCK(port), R_XPORT_XGXS_CTRL, &val);
        val |= (0x1 << 4); /* RSTB_PLL */
        bcm5346x_reg_set(unit, SOC_PORT_BLOCK(port), R_XPORT_XGXS_CTRL, val);

        bcm5346x_reg_get(unit, SOC_PORT_BLOCK(port), R_XPORT_XGXS_CTRL, &val);
        val |= (0xf << 8); /* TXD1G_FIFO_RSTB */
        val |= (0x1 << 12); /* TXD10G_FIFO_RSTB */
        bcm5346x_reg_set(unit, SOC_PORT_BLOCK(port), R_XPORT_XGXS_CTRL, val);
    } else if (IS_XL_PORT(port)) {
        /*
         * Reference clock selection: REFIN_ENf [Bit 2]
         */
        bcm5346x_reg_get(unit, SOC_PORT_BLOCK(port), R_XLPORT_XGXS0_CTRL_REG, &val);
        val &= ~(0x1 << 2);
        val |= (lcpll << 2);
		val |= 1;
        bcm5346x_reg_set(unit, SOC_PORT_BLOCK(port), R_XLPORT_XGXS0_CTRL_REG, val);

        /* Deassert power down [Bit 3]*/
        val &= ~(0x1 << 3);
        bcm5346x_reg_set(unit, SOC_PORT_BLOCK(port), R_XLPORT_XGXS0_CTRL_REG, val);
        sal_usleep(sleep_usec);

        /* Bring XGXS out of reset: RstB_HW[Bit 0] */
        val |= 0x1;
        bcm5346x_reg_set(unit, SOC_PORT_BLOCK(port), R_XLPORT_XGXS0_CTRL_REG, val);
        sal_usleep(sleep_usec);
    }
}

static void
soc_reset(uint8 unit)
{
    uint32 val, to_usec;
    uint8 lport;    

#if CONFIG_METROLITE_EMULATION
    to_usec = 250000;
#else
    to_usec = 10000;
#endif /* CONFIG_METROLITE_EMULATION */

    WRITECSR(CMIC_SBUS_RING_MAP_0_7, 0x62034000);
    WRITECSR(CMIC_SBUS_RING_MAP_8_15, 0x60377531);
    WRITECSR(CMIC_SBUS_RING_MAP_16_23, 0x00000031);
    WRITECSR(CMIC_SBUS_RING_MAP_24_31, 0x0);
    WRITECSR(CMIC_SBUS_RING_MAP_32_39, 0x0);
    WRITECSR(CMIC_SBUS_RING_MAP_40_47, 0x0);
    WRITECSR(CMIC_SBUS_RING_MAP_48_55, 0x0);
    WRITECSR(CMIC_SBUS_RING_MAP_56_63, 0x0);
    WRITECSR(CMIC_SBUS_TIMEOUT, 0x7d0);

    sal_usleep(to_usec);

    /* Bring LCPLL out of reset */
    bcm5346x_reg_get(unit, R_TOP_SOFT_RESET_REG_3, &val);
    /* TOP_XG0_PLL_RST_L [Bit 0] = 1 */
    val |= 0x1;
    bcm5346x_reg_set(unit, R_TOP_SOFT_RESET_REG_3, val);

    sal_usleep(to_usec);

    /* Bring network sync out of reset */
    bcm5346x_reg_get(unit, R_TOP_SOFT_RESET_REG, &val);
    /* 
     * TOP_AVS_RST_L [Bit 10] = 1
     * TOP_NS_RST_L [Bit 9] = 1
     * TOP_EP_RST_L [Bit 8] = 1
     * TOP_IP_RST_L [Bit 7] = 1
     * TOP_MMU_RST_L [Bit 6] = 1
     * TOP_PM1_HOTSWAP_RST_L [Bit 5] = 1
     * TOP_PM0_HOTSWAP_RST_L [Bit 4] = 1
     * TOP_MXQ_HOTSWAP_RST_L [Bit 3] = 1
     * TOP_PM1_RST_L [Bit 2] = 1
     * TOP_PM0_RST_L [Bit 1] = 1
     * TOP_MXQ_RST_L [Bit 0] = 1
     */
    val |= 0x7FF;
    bcm5346x_reg_set(unit, R_TOP_SOFT_RESET_REG, val);

    /* IPIPE and EPIPE configurations */
    soc_hw_reset_control_init(unit);
    sal_usleep(to_usec);

    /* TSC reset */
    SOC_LPORT_ITER(lport) {
        if (SOC_PORT_BLOCK_INDEX(lport) == 0) {
            soc_xgxs_reset(unit, lport);
        }
    }
    
    /* MAC reset */
    SOC_LPORT_ITER(lport) {
        if (IS_GX_PORT(lport) && (SOC_PORT_BLOCK_INDEX(lport) == 0)) {
            /* XMAC_RESETf: [Bit 0] */
            bcm5346x_reg_get(unit, SOC_PORT_BLOCK(lport), R_XPORT_XMAC_CONTROL, &val);
            val |= 0x1;
            bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XPORT_XMAC_CONTROL, val);
            sal_usleep(10);
            val &= ~0x1;
            bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XPORT_XMAC_CONTROL, val);
        } else if (IS_XL_PORT(lport) && (SOC_PORT_BLOCK_INDEX(lport) == 0)) {
            bcm5346x_reg_get(unit, SOC_PORT_BLOCK(lport), R_XLPORT_MAC_CONTROL, &val);
            /* XMAC0_RESETf: [Bit 0] */
            val |= 0x1;
            bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_MAC_CONTROL, val);
            sal_usleep(10);
            val &= ~0x1;
            bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_MAC_CONTROL, val);
        }
    }
}

soc_chip_type_t
bcm5346x_chip_type(void)
{
    return SOC_TYPE_SWITCH_XGS;
}

uint8
bcm5346x_port_count(uint8 unit)
{
    if (unit > 0) {
        return 0;
    }
    return SOC_PORT_COUNT(unit);
}

sys_error_t
bcm5346x_link_status(uint8 unit, uint8 lport, BOOL *link)
{
    if (link == NULL || unit > 0 || lport > BCM5346X_LPORT_MAX) {
        return SYS_ERR_PARAMETER;
    }

    *link = ml_sw_info.link[lport];

    return SYS_OK;
}

static int
bcm5346x_sw_op(uint8 unit,
               uint32 op,
               uint8 block_id,
               uint32 addr,
               uint32 *buf,
               int len)
{
    uint32 msg_hdr;
    uint32 ctrl;
    int i;

    if (buf == NULL || unit > 0) {
        return -1;
    }

    msg_hdr = (V_SMHDR_OP_CODE(op) | V_SMHDR_DEST_BLOCK(block_id));

    msg_hdr |= V_SMHDR_DATA_LEN(len*4);

    SCHAN_LOCK(unit);

    WRITECSR(R_CMIC_SCHAN_D(0), msg_hdr);
    WRITECSR(R_CMIC_SCHAN_D(1), addr);

    if (op == SC_OP_WR_REG_CMD || op == SC_OP_WR_MEM_CMD) {
        for (i = 0; i < len; i++) {
            WRITECSR(R_CMIC_SCHAN_D(2+i), buf[i]);
        }
    }

    WRITECSR(CMIC_CMC1_SCHAN_CTRL, SC_CMCx_MSG_START);

    for (i = 0; i < 100; i++) {
        sal_usleep(2);
        ctrl = READCSR(CMIC_CMC1_SCHAN_CTRL);
        if (ctrl & SC_CMCx_MSG_DONE) {
            break;
        }
    }

#if CFG_CONSOLE_ENABLED
    if ((i == 100) || ((ctrl & SC_CMCx_MSG_ERROR_MASK) != 0)) {
        sal_printf("S-CHAN op=0x%x, %d:0x%x, error(%d-0x%08x)\n", op, block_id, addr, i, ctrl);
    }
#endif /* CFG_CONSOLE_ENABLED */

    ctrl = READCSR(CMIC_CMC1_SCHAN_CTRL);
    ctrl &= ~SC_CMCx_MSG_DONE;
    WRITECSR(CMIC_CMC1_SCHAN_CTRL, ctrl);

    if (op == SC_OP_RD_REG_CMD || op == SC_OP_RD_MEM_CMD) {
        for (i = 0; i < len; i++) {
            buf[i] = READCSR(R_CMIC_SCHAN_D(1+i));
        }
    }

    SCHAN_UNLOCK(unit);
    return 0;
}

sys_error_t
bcm5346x_phy_reg_get(uint8 unit, uint8 lport,
                           uint16 reg_addr, uint16 *p_value)
{
    return SYS_OK;
}

sys_error_t
bcm5346x_phy_reg_set(uint8 unit, uint8 lport,
                           uint16 reg_addr, uint16 value)
{
    return SYS_OK;
}

sys_error_t
bcm5346x_reg_get(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *val)
{
	return bcm5346x_sw_op(unit, SC_OP_RD_REG_CMD, block_id, addr, val, 1);
}

sys_error_t
bcm5346x_reg_set(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 val)
{
	return bcm5346x_sw_op(unit, SC_OP_WR_REG_CMD, block_id, addr, &val, 1);
}

sys_error_t
bcm5346x_reg64_get(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *val,
                 int len)
{
	return bcm5346x_sw_op(unit, SC_OP_RD_REG_CMD, block_id, addr, val, len);
}

sys_error_t
bcm5346x_reg64_set(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *buf,
                 int len)
{
	return bcm5346x_sw_op(unit, SC_OP_WR_REG_CMD, block_id, addr, buf, len);
}

sys_error_t
bcm5346x_mem_get(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *buf,
                 int len)
{
	return bcm5346x_sw_op(unit, SC_OP_RD_MEM_CMD, block_id, addr, buf, len);
}

sys_error_t
bcm5346x_mem_set(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *buf,
                 int len)
{
    return bcm5346x_sw_op(unit, SC_OP_WR_MEM_CMD, block_id, addr, buf, len);
}

sys_error_t
bcm5346x_chip_revision(uint8 unit, uint16 *dev, uint16 *rev)
{
    uint32 val;

    if (unit > 0) {
        return -1;
    }

    val = READCSR(CMIC_DEV_REV_ID);
    ml_sw_info.devid = val & 0xFFFF;
    ml_sw_info.revid = val >> 16;

    return 0;
}

static void
bcm5346x_load_led_program(uint8 unit)
{
#ifdef CFG_LED_MICROCODE_INCLUDED
    const uint8 *led_program;
    int i, offset, led_code_size;
    uint32 addr, val;
    int byte_count = 0;
    uint8 led_option = 1;    
    uint8 led_program_3[256];
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED       
    sys_error_t sal_config_rv = SYS_OK;
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */ 

    uint32 port_remap[9] = { 0x0030b289, 0x00207185, 0x00103081, 0x0,
                             0x0, 0x0, 0x0, 0x0, 0x0 };

    for (i = 0, addr = CMIC_LEDUP0_PORT_ORDER_REMAP_0_3; i < 9; i++, addr += 4) {
        WRITECSR(addr, port_remap[i]);
    }

#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED       
    sal_config_rv = sal_config_uint8_get(SAL_CONFIG_LED_OPTION, &led_option);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Overwrite serial led option with value %d.\n", led_option);
    }

    byte_count = sal_config_bytes_get(SAL_CONFIG_LED_PROGRAM, led_program_3, 256);
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED   */
    if (led_option == 2) {
        led_program = ledproc_ml_12p_2;
        led_code_size = sizeof(ledproc_ml_12p_2);
    } else if ((led_option == 3) && byte_count) {
         sal_printf("Vendor Config : Load customer LED ucdoe with length %d.\n", byte_count);
         led_program = led_program_3;
         led_code_size = sizeof(led_program_3);
    } else {
         led_program = ledproc_ml_12p_1;
         led_code_size = sizeof(ledproc_ml_12p_1);
    }


#define LED_RAM_SIZE     0x100
    for (offset = 0; offset < LED_RAM_SIZE; offset++) {
        WRITECSR(CMIC_LEDUP_PROGRAM_RAM_D(offset),
                      (offset >= led_code_size) ? 0 : *(led_program + offset));

        WRITECSR(CMIC_LEDUP_DATA_RAM_D(offset), 0);
    }

    /* enable LED processor */
    val = READCSR(CMIC_LEDUP0_CTRL);
    val |= 0x1;
    WRITECSR(CMIC_LEDUP0_CTRL, val);
#endif /* CFG_LED_MICROCODE_INCLUDED */
}

sys_error_t
bcm5346x_l2_op(uint8 unit,
               l2x_entry_t *entry,
               uint8 op_code
               )

{
    uint32 l2_entry[4];
    uint32 msg_hdr[2] = {0x90080800, 0x18000000};
    uint32 ctrl;
    int i;
     /* L2::MAC_ADDR 62:15 */
     /* L2::VLAN_ID 14:3 */
	l2_entry[0] = ((entry->vlan_id << 3) |
                   (entry->mac_addr[5] << 15) |
                   (entry->mac_addr[4] << 23) |
                   (entry->mac_addr[3] << 31));

	l2_entry[1] = (((entry->mac_addr[3] >> 1) & 0x0000007F) |
                   (entry->mac_addr[2] << 7) |
                   (entry->mac_addr[1] << 15) |
                   (entry->mac_addr[0] << 23)
                   );
    /* L2::PORT_NUM 71:65*/
    l2_entry[2] = (((entry->port) & 0x0000003F) << 1); 
    /* set static and valid bit bit 94/102*/
    l2_entry[2] |= (1 << 30);
    l2_entry[3] = (1 << 6);

    if (op_code == SC_OP_L2_DEL_CMD) {
         msg_hdr[0] = 0x98080800;
    }

    WRITECSR(R_CMIC_SCHAN_D(0), msg_hdr[0]);
    WRITECSR(R_CMIC_SCHAN_D(1), msg_hdr[1]);
    WRITECSR(R_CMIC_SCHAN_D(2), l2_entry[0]);
    WRITECSR(R_CMIC_SCHAN_D(3), l2_entry[1]);
    WRITECSR(R_CMIC_SCHAN_D(4), l2_entry[2]);
    WRITECSR(R_CMIC_SCHAN_D(5), l2_entry[3]);

    WRITECSR(CMIC_CMC1_SCHAN_CTRL, SC_CMCx_MSG_START);

    for (i = 0; i < 100; i++) {
        sal_usleep(2);
        ctrl = READCSR(CMIC_CMC1_SCHAN_CTRL);
        if (ctrl & SC_CMCx_MSG_DONE)
            break;
    }

    return SYS_OK;
}

static void
soc_port_block_info_get(uint8 unit, uint8 pport, int *block_type, int *block_idx, int *port_idx)
{
    *block_type = PORT_BLOCK_TYPE_XLPORT;
    if ((pport >= PHY_XLPORT1_BASE) && (pport <= BCM5346X_PORT_MAX)) {
        *block_idx = XLPORT1_BLOCK_ID;
        *port_idx = (pport - PHY_XLPORT1_BASE) & 0x3;
    } else if (pport >= PHY_XLPORT0_BASE) {
        *block_idx = XLPORT0_BLOCK_ID;
        *port_idx = (pport - PHY_XLPORT0_BASE) & 0x3;
    } else if (pport >= PHY_MXQPORT0_BASE) {
        *block_idx = MXQPORT0_BLOCK_ID;
        *port_idx = (pport - PHY_MXQPORT0_BASE) & 0x3;
        *block_type = PORT_BLOCK_TYPE_MXQPORT;
    } else {
        sal_printf("soc_port_block_info_get : invalid pport %d\n", pport);
    }
}

static void
soc_init_port_mapping(uint8 unit)
{
    int i;
    const int *p2l_mapping = 0;
    const int *speed_max = 0;
    int block_start_port;

    p2l_mapping = p2l_mapping_53460;
    if ((ml_sw_info.devid == BCM53460_DEVICE_ID) ||
        (ml_sw_info.devid == BCM56270_DEVICE_ID)) {
        speed_max = port_speed_max_53460;
    } else {
        speed_max = port_speed_max_53461;
    }

    for (i = 0; i <= BCM5346X_PORT_MAX ; i++) {
        ml_sw_info.port_p2l_mapping[i] = p2l_mapping[i];
    }

    for (i = 0; i <= BCM5346X_LPORT_MAX ; i++) {
        ml_sw_info.port_l2p_mapping[i] = p2l_mapping[i];
    }

    for (i = 0; i <= BCM5346X_PORT_MAX; i++) {
        if (speed_max[i] != -1) {
            if (speed_max[i] < 100) {
                ml_sw_info.port_speed_max[p2l_mapping[i]] = 2500;
            } else {
                ml_sw_info.port_speed_max[p2l_mapping[i]] = 10000;
            }
        }
    }

    SOC_LPORT_ITER(i) {
        soc_port_block_info_get(unit, SOC_PORT_L2P_MAPPING(i),
                                &SOC_PORT_BLOCK_TYPE(i),
                                &SOC_PORT_BLOCK(i), &SOC_PORT_BLOCK_INDEX(i));

        BCM5346X_ALL_PORTS_MASK |= (0x1 << i);
    }

    SOC_LPORT_ITER(i) {
        block_start_port = SOC_PORT_L2P_MAPPING(i) - SOC_PORT_BLOCK_INDEX(i);
        SOC_PORT_LANE_NUMBER(i) = 4;
        if (p2l_mapping[block_start_port + 1] != -1) {
            SOC_PORT_LANE_NUMBER(i) = 1;
        } else if (p2l_mapping[block_start_port + 2] != -1) {
                SOC_PORT_LANE_NUMBER(i) = 2;
        }
    }
}

static void
soc_misc_init(uint8 unit)
{
    int lport;
    uint32 val, mode;
    uint32 cpu_pbm = 0x1, entry, entry2[2]; /* CPU is port 0 */


    /* program CPU_PBMm and CPU_PBM_2m */
    bcm5346x_mem_set(unit, M_CPU_PBM, &cpu_pbm, 1);
    bcm5346x_mem_set(unit, M_CPU_PBM_2, &cpu_pbm, 1);
	entry = 0;	
	bcm5346x_mem_set(unit, M_ING_PHYSICAL_PORT_TABLE(0), &entry, 1);
	bcm5346x_mem_set(unit, M_EGR_PHYSICAL_PORT(0), &entry, 1); 
	bcm5346x_mem_set(unit, M_PP_PORT_TO_PHYSICAL_PORT_MAP(0), &entry, 1);

	entry = (2<<22) | (0xd << 16);	
	bcm5346x_mem_set(unit, M_ING_PHYSICAL_PORT_TABLE(13), &entry, 1);
	entry = (2);		
	bcm5346x_mem_set(unit, M_EGR_PHYSICAL_PORT(13), &entry, 1); 
	entry = 0xd;
	bcm5346x_mem_set(unit, M_PP_PORT_TO_PHYSICAL_PORT_MAP(13), &entry, 1);
	
	entry = (1<<22) | (0x0 << 16);	
	bcm5346x_mem_set(unit, M_ING_PHYSICAL_PORT_TABLE(14), &entry, 1);	
	entry = (1);		
	bcm5346x_mem_set(unit, M_EGR_PHYSICAL_PORT(14), &entry, 1); 
	entry = 0;
	bcm5346x_mem_set(unit, M_PP_PORT_TO_PHYSICAL_PORT_MAP(14), &entry, 1);
	
    SOC_LPORT_ITER(lport) {
        entry = 0;
        bcm5346x_mem_set(unit, M_EGR_PHYSICAL_PORT(lport), &entry, 1); 
        entry = lport << 16; /* PP_PORT<Bit 20:16> */
        bcm5346x_mem_set(unit, M_ING_PHYSICAL_PORT_TABLE(lport), &entry, 1);
        entry = lport; /* DESTINATION<Bit 3:0> */
        bcm5346x_mem_set(unit, M_PP_PORT_TO_PHYSICAL_PORT_MAP(lport), &entry, 1);
        
    }

    /* MODID_0_VALID */	
    entry2[0] = 0x0;
    entry2[1] = 0x80000000; /* set MODID_0_VALID <Bit 63> to 1*/
    bcm5346x_mem_set(unit, M_EGR_PP_PORT_GPP_TRANSLATION_1, entry2, 2);
    bcm5346x_mem_set(unit, M_EGR_PP_PORT_GPP_TRANSLATION_2, entry2, 2);

    entry2[0] = 0x0;
    entry2[1] = 0x00800000; /* set MODID_0_VALID <Bit 55> to 1*/
    bcm5346x_mem_set(unit, M_PP_PORT_GPP_TRANSLATION_1, entry2, 2);
    bcm5346x_mem_set(unit, M_PP_PORT_GPP_TRANSLATION_2, entry2, 2);
    bcm5346x_mem_set(unit, M_PP_PORT_GPP_TRANSLATION_3, entry2, 2);
    bcm5346x_mem_set(unit, M_PP_PORT_GPP_TRANSLATION_4, entry2, 2);

#if !CONFIG_METROLITE_ROMCODE
    bcm5346x_reg_get(unit, R_MISCCONFIG, &val);
    /* Metering Clock [Bit 0] */
    val |= 0x1;
    bcm5346x_reg_set(unit, R_MISCCONFIG, val);

    /* Enable dual hash on L2 and L3 tables */
    /* HASH_SELECT[Bit3:1] = FB_HASH_CRC32_LOWER(2), INSERT_LEAST_FULL_HALF[Bit 0] = 1 */
    bcm5346x_reg_set(unit, R_L2_AUX_HASH_CONTROL, 0x15);
    bcm5346x_reg_set(unit, R_L3_AUX_HASH_CONTROL, 0x15);
#endif

    /* Egress Disable? */

    /* ING_CONFIG_64r and EGR_CONFIG_1r? */

    /* The HW defaults for EGR_VLAN_CONTROL_1.VT_MISS_UNTAG == 1, which
     * causes the outer tag to be removed from packets that don't have
     * a hit in the egress vlan tranlation table. Set to 0 to disable this.
     */
    SOC_LPORT_ITER(lport) {
        bcm5346x_reg_set(unit, R_EGR_VLAN_CONTROL_1(lport), 0x0);
    }

    val = (BCM5346X_ALL_PORTS_MASK | 0x1); /* CPU is port 0 */
    bcm5346x_mem_set(unit, M_ING_EN_EFILTER_BITMAP, &val, 1);

    val = READCSR(CMIC_TXBUF_CONFIG);    
    /* FIRST_SERVE_BUFFERS_WITH_EOP_CELLS [Bit 2] */
    val &= ~(0x1 << 2);
    WRITECSR(CMIC_TXBUF_CONFIG, val);

    /* soc_metrolite_port_init_config */
    SOC_LPORT_ITER(lport) {
     
        if (IS_GX_PORT(lport)) {
            /* XPORT_MODE_REG 
             * PORT_GMII_MII_ENABLE<2>
             * PHY_PORT_MODE<1:0>
             */
            mode = 0x2; /* 0x2 : Quad Port Mode,  0x1 : Dual Port Mode, 0x0 : Single Port Mode */
            val = mode;
            val |= 0x1 << 2;
            bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XPORT_MODE_REG, val); 
            bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XPORT_MIB_RESET, 0xf);
            bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XPORT_MIB_RESET, 0x0);
            bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_X_GPORT_CNTMAXSIZE(SOC_PORT_BLOCK_INDEX(lport)), 0x5f2);
            /* Move to mac driver Enable XPORT */
            // bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XPORT_PORT_ENABLE, 0xf);
        } else if (IS_XL_PORT(lport)) {
            bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), 0);
            mode = 0x0; /* 0x0 : Quad Port Mode */
            val = (mode << 3) | mode;
            bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_MODE_REG, val);
			bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_SOFT_RESET, 0xf);
			val |= 0x00000200; /* EGR_1588_TIMESTAMPING_CMIC_48_EN */
			bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_MODE_REG, val);
			bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_SOFT_RESET, 0x0);		
            bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_MIB_RESET, 0xf);
            bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_MIB_RESET, 0x0);
            bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_ECC_CONTROL, 0x1);
            bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_CNTMAXSIZE(SOC_PORT_BLOCK_INDEX(lport)), 0x5f2);
			bcm5346x_reg_get(unit, SOC_PORT_BLOCK(lport), R_XLPORT_FLOW_CONTROL_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), &val);
			val |= (1<<1);
			bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_FLOW_CONTROL_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), val);

        }
    }

    bcm5346x_reg_get(unit, R_AUX_ARB_CONTROL_2, &val);
    /* TCAM_ATOMIC_WRITE_ENABLE<28> */
    val |= (0x1 << 28);
    bcm5346x_reg_set(unit, R_AUX_ARB_CONTROL_2, val);

#if !CONFIG_METROLITE_ROMCODE
    /* Enable SRC_REMOVAL_EN[Bit 0] and LAG_RES_EN[Bit2] */
    bcm5346x_reg_set(unit, R_SW2_FP_DST_ACTION_CONTROL, 0x5);
#endif /* !CONFIG_METROLITE_ROMCODE */

    /*
     * Set reference clock (based on 200MHz core clock)
     * to be 200MHz * (1/40) = 5MHz
     */
    WRITECSR(CMIC_RATE_ADJUST_EXT_MDIO, 0x10028);
    /* Match the Internal MDC freq with above for External MDC */
    WRITECSR(CMIC_RATE_ADJUST_INT_MDIO, 0x10028);
}

static void
soc_mmu_tdm_init(uint8 unit){
    int i;
    uint32 val;
    int tdm_size;
    const uint32 *arr = NULL;
    uint32 port_enable_value=0;

    switch (ml_sw_info.devid) {
        case BCM53460_DEVICE_ID:
        case BCM56270_DEVICE_ID:
            tdm_size = ML_TDM_A_SIZE;
            arr = ml_tdm_56270_A_ref;
        break;
        case BCM53461_DEVICE_ID:
        case BCM56271_DEVICE_ID:
            tdm_size = ML_TDM_C_SIZE;
            arr = ml_tdm_56271_C_ref;
        break;
    }

    /* Disable IARB TDM before programming... */
    /* DISABLE [Bit 0] = 1, TDM_WRAP_PTR[Bit 9:1] = TDM_SIZE-1 */
    bcm5346x_reg_get(0, R_IARB_TDM_CONTROL, &val);
    val &= ~0x3FF;
    val |= (((tdm_size-1) << 1) | 1);
    bcm5346x_reg_set(0, R_IARB_TDM_CONTROL, val);

    for (i = 0; i < tdm_size; i++) {
        if (arr[i] < ML_IDLE) { /* Non Idle Slots */
            port_enable_value = 1;
        } else {
            port_enable_value = 0;
        }
          
        bcm5346x_mem_set(unit, M_IARB_TDM_TABLE(i), (uint32 *)&arr[i], 1);

        if((i % 2) == 0) {
            /* Two entries per mem entry */
            /* port_id_0_enable [Bit 9], port_id_0 [Bit 8:5] */
            val = (((arr[i] & 0x1f) << 5) | (port_enable_value << 9));
        } else {
            /* port_id_1_enable [Bit 4], port_id_1 [Bit 3:0] */
            val |= (((arr[i] & 0x1f) << 0) | (port_enable_value << 4));
            bcm5346x_mem_set(unit, M_LLS_PORT_TDM(i/2), &val, 1);
        }
    }

    if (tdm_size % 2) {
        sal_printf("Info:Odd TDM Size%d \n",tdm_size);
        bcm5346x_mem_set(unit, M_LLS_PORT_TDM(i/2), &val, 1);
    }

    /* DISABLE [Bit 0] = 0, TDM_WRAP_PTR[Bit 9:1] = TDM_SIZE-1, IDLE_PORT_NUM_SEL[Bit 10] = 1*/
    bcm5346x_reg_get(0, R_IARB_TDM_CONTROL, &val);
    val &= ~0x7FF;
    val |= ((tdm_size-1) << 1) | (1 << 10);
    bcm5346x_reg_set(unit, R_IARB_TDM_CONTROL, val);

    /* END_B [Bit 8:0] = TDM_SIZE-1, END_A[Bit 17:9] = TDM_SIZE-1, ENABLE[Bit 25] = 1*/
    val = ((tdm_size-1) << 0) | ((tdm_size-1) << 9) | (1 << 25);
    bcm5346x_reg_set(unit, R_LLS_TDM_CAL_CFG, val);
}

/* *************************************************************** */
/* MetroLiteMMU HELPER  --- START(As per Metrolite_MMU_Settings.xls */
/* *************************************************************** */
const static uint32 ml_port_mapping[BCM5346X_LPORT_MAX + 2][2] =
{ {0,0}, {1,15}, {2,23}, {3, 31}, {4, 39}, {5,47}, {6, 55},
  {7,63}, {8, 71}, {9, 79}, {21, 175}, {22, 183}, {23, 191}, {29,239}};

#define ML_GET_THDI_PORT(p) ml_port_mapping[p][0]
#define ML_GET_THDI_PORT_PG(p) ml_port_mapping[p][1]
#define RQEI_QUEUE_SIZE 12
#define RDEI_QUEUE_SIZE 8
#define RDEQ_QUEUE_SIZE 8
#define OTHER_QUEUE_SIZE 12

/* Dynamic/SetLater Parameter */
typedef struct _soc_ml_mmu_params {
    uint32 mmu_min_pkt_size;                   /*C5*/
    uint32 mmu_ethernet_mtu_bytes;             /*C6*/
    uint32 mmu_max_pkt_size;                   /*C7*/
    uint32 mmu_jumbo_frame_size;               /*C8*/
    uint32 mmu_int_buf_cell_size;              /*C9*/
    uint32 mmu_pkt_header_size;                /*C10*/
    uint32 lossless_mode_d_c;                  /*C11*/
    uint32 pfc_pause_mode_d_c;                 /*C12*/
    uint32 mmu_lossless_pg_num;                /*C13*/
    uint32 num_ge_ports_d;                     /*C14*/
    uint32 mmu_line_rate_ge;                   /*C15*/
    uint32 num_ge_int_ports_d;                 /*C14*/
    uint32 num_egr_queue_per_int_ge_port_d;    /*C16*/
    uint32 num_hg_ports_d;                     /*C17*/
    uint32 mmu_line_rate_hg;                   /*C18*/
    uint32 num_hg_int_ports_d;                 /*C17*/
    uint32 num_egr_queue_per_int_hg_port_d;    /*C19 */
    uint32 mmu_num_cpu_port;                   /*C20*/
    uint32 mmu_num_cpu_queue;                  /*C21*/
    uint32 mmu_num_loopback_port;              /*C22*/
    uint32 mmu_num_loopback_queue;             /*C35*/
    uint32 mmu_num_ep_redirection_queue;       /*C36*/
    uint32 mmu_num_olp_port_d;                 /*C37*/
    uint32 mmu_num_olp_queue;                  /*C38*/
    uint32 olp_port_int_ext_bounding_d_c;      /*C39:Dynamic*/
    uint32 mmu_total_egress_queue;             /*C40*/
    uint32 mmu_pipeline_lat_cpap_admission;    /*C41*/
    uint32 mmu_int_buf_size;                   /*C42:2048KB=2MB*/
    uint32 mmu_available_int_buf_size_d;       /*C43:1434KB */
    uint32 mmu_reserved_int_buf_cells;         /*C44:100 Cells */
    uint32 mmu_reserved_int_buf_ema_pool_size_d; /*C45:614KB */
    uint32 internal_buffer_reduction_d_c;        /* C46:Dynamic */
    uint32 mmu_ext_buf_size;                   /*C47:737280KB=720MB*/
    uint32 mmu_reserved_ext_buf_space_cfap;    /*C48:737280KB=720MB*/
    uint32 mmu_egress_queue_entries;           /*C49*/
    uint32 mmu_ep_redirect_queue_entries;      /*C50*/
    uint32 mmu_exp_num_of_repl_per_pkt;        /*C51*/
    uint32 mmu_repl_engine_work_queue_entries; /*C52*/
    uint32 mmu_resv_repl_engine_work_queue_entries; /*C53*/
    uint32 mmu_repl_engine_work_queue_in_device;/*C54*/
    uint32 mmu_ema_queues;                     /*C55*/
    uint32 num_cells_rsrvd_ing_ext_buf;        /* C56:Dynamic */
    uint32 per_cos_res_cells_for_int_buff_d;   /* C58:Dynamic */
    uint32 per_cos_res_cells_for_ext_buff_d;   /* C59:Dynamic */
    uint32 mmu_ing_port_dyn_thd_param;         /*C60*/
    uint32 mmu_ing_pg_dyn_thd_param;           /*C61*/
    uint32 mmu_egr_queue_dyn_thd_param_baf;        /*C62*/
    uint32 mmu_egr_queue_dyn_thd_param_baf_profile_lossy;           /*C63*/
    uint32 mmu_ing_cell_buf_reduction;         /*C64 */
    uint32 mmu_ing_pkt_buf_reduction;          /*C65 */
} _soc_ml_mmu_params_t;

typedef struct _general_info {
    uint32 max_packet_size_in_cells;                         /* C72 */
    uint32 jumbo_frame_for_int_buff;                         /* C73 */
    uint32 jumbo_frame_for_ext_buff;                         /* C74 */
    uint32 ether_mtu_cells_for_int_buff;                     /* C76 */
    uint32 ether_mtu_cells_for_ext_buff;                     /* C77 */
    uint32 total_num_of_ports;                               /* C79 */
    uint32 total_num_of_ports_excl_lpbk;                     /* C80 */
    uint32 total_num_of_ports_excl_lpbk_olp;                 /* C81 */
    uint32 total_num_of_ports_excl_lpbk_olp_cpu;             /* C82 */
    uint32 port_bw_bound_to_ext_buff;                        /* C83 */
    uint32 total_egr_queues_for_a_int_ge_ports;              /* C84 */
    uint32 total_egr_queues_for_a_ext_ge_ports;              /* C85 */
    uint32 total_egr_queues_for_a_int_hg_ports;              /* C86 */
    uint32 total_egr_queues_for_a_ext_hg_ports;              /* C87 */
    uint32 total_cpu_queues;                                 /* C88 */
    uint32 total_base_queue_int_buff;                       /* C89 */
    uint32 total_base_queue_ext_buff;                        /* C90 */
    uint32 total_ema_queues;                                 /* C91 */
    uint32 total_egr_base_queues_in_device;                  /* C93 */
    uint32 total_egr_queues_in_device;                       /* C94 */
    uint32 max_int_cell_buff_size;                           /* C95 */
    uint32 int_cell_buff_size_after_limitation;              /* C96 */
    uint32 src_packing_fifo;                                 /* C97 */
    uint32 int_buff_pool;                                    /* C98 */
    uint32 ema_pool;                                         /* C99 */
    uint32 max_ext_cell_buff_size;                           /* C100 */
    uint32 repl_engine_work_queue_entries;                   /* C101 */
    uint32 ratio_of_ext_buff_to_int_buff_size;               /* C103 */
    uint32 int_buff_cells_per_avg_size_pkt;                  /* C104 */
    uint32 ext_buff_cells_per_avg_size_pkt;                  /* C105 */
    uint32 max_prop_of_buff_used_by_one_queue_port;          /* C106 */
}_general_info_t;

typedef struct _input_port_threshold_t {
    uint32 global_hdrm_cells_for_int_buff_ingress_pool;      /* C110 */
    uint32 global_hdrm_cells_for_int_buff_ema_pool;          /* C111 */
    uint32 global_hdrm_cells_for_ext_buff_pool;              /* C112 */
    uint32 global_hdrm_cells_for_RE_WQEs;                    /* C113 */
    uint32 global_hdrm_cells_for_EQEs;                       /* C114 */

    uint32 hdrm_int_buff_cells_for_10G_PG;                   /* C115 */
    uint32 hdrm_ema_buff_cells_for_10G_PG;                   /* C116 */
    uint32 hdrm_ext_buff_cells_for_10G_PG;                   /* C117 */
    uint32 hdrm_RE_WQEs_pkts_for_10G_PG;                     /* C118 */
    uint32 hdrm_EQEs_pkts_for_10G_PG;                        /* C119 */

    uint32 hdrm_int_buff_cells_for_10G_total_PG;             /* C120 */
    uint32 hdrm_ema_buff_cells_for_10G_total_PG;             /* C121 */
    uint32 hdrm_ext_buff_cells_for_10G_total_PG;             /* C122 */
    uint32 hdrm_RE_WQEs_pkts_for_10G_total_PG;               /* C123 */
    uint32 hdrm_EQEs_pkts_for_10G_total_PG;                  /* C124 */

    uint32 hdrm_int_buff_cells_for_1G_PG;                    /* C125 */
    uint32 hdrm_ema_buff_cells_for_1G_PG;                    /* C126 */
    uint32 hdrm_ext_buff_cells_for_1G_PG;                    /* C127 */
    uint32 hdrm_RE_WQEs_pkts_for_1G_PG;                      /* C128 */
    uint32 hdrm_EQEs_pkts_for_1G_PG;                         /* C129 */

    uint32 hdrm_int_buff_cells_for_1G_total_PG;              /* C130 */
    uint32 hdrm_ema_buff_cells_for_1G_total_PG;              /* C131 */
    uint32 hdrm_ext_buff_cells_for_1G_total_PG;              /* C132 */
    uint32 hdrm_RE_WQEs_pkts_for_1G_total_PG;                /* C133 */
    uint32 hdrm_EQEs_pkts_for_1G_total_PG;                   /* C134 */

    uint32 hdrm_int_buff_cells_for_olp_port;                 /* C135 */
    uint32 hdrm_ema_buff_cells_for_olp_port;                 /* C136 */
    uint32 hdrm_ext_buff_cells_for_olp_port;                 /* C137 */
    uint32 hdrm_RE_WQEs_pkts_for_olp_port;                   /* C138 */
    uint32 hdrm_EQEs_pkts_for_olp_port;                      /* C139 */

    uint32 hdrm_int_buff_cells_for_lpbk_port;                /* C140 */
    uint32 hdrm_ema_buff_cells_for_lpbk_port;                /* C141 */
    uint32 hdrm_ext_buff_cells_for_lpbk_port;                /* C142 */
    uint32 hdrm_RE_WQEs_pkts_for_lpbk_port;                  /* C143 */
    uint32 hdrm_EQEs_pkts_for_lpbk_port;                     /* C144 */

    uint32 hdrm_int_buff_cells_for_cpu_port;                 /* C145 */
    uint32 hdrm_ema_buff_cells_for_cpu_port;                 /* C146 */
    uint32 hdrm_ext_buff_cells_for_cpu_port;                 /* C147 */
    uint32 hdrm_RE_WQEs_pkts_for_cpu_port;                   /* C148 */
    uint32 hdrm_EQEs_pkts_for_cpu_port;                      /* C149 */

    uint32 total_hdrm_int_buff_cells;                        /* C150 */
    uint32 total_hdrm_int_buff_ema_pool_cells;               /* C151 */
    uint32 total_hdrm_ext_buff_cells;                        /* C152 */
    uint32 total_hdrm_RE_WQEs_pkts;                          /* C153 */
    uint32 total_hdrm_EQEs_pkts;                             /* C154 */

    uint32 min_int_buff_cells_per_PG;                        /* C156 */
    uint32 min_int_buff_ema_pool_cells_per_PG;               /* C157 */
    uint32 min_ext_buff_cells_per_PG;                        /* C158 */
    uint32 min_RE_WQEs_pkt_per_PG;                           /* C159 */
    uint32 min_EQEs_pkt_per_PG;                              /* C160 */

    uint32 min_int_buff_cells_for_total_PG;                  /* C162 */
    uint32 min_int_buff_ema_pool_cells_for_total_PG;         /* C163 */
    uint32 min_ext_buff_cells_for_total_PG;                  /* C164 */
    uint32 min_RE_WQEs_pkts_for_total_PG;                    /* C165 */
    uint32 min_EQEs_pkts_for_total_PG;                       /* C166 */

    uint32 min_int_buff_cells_for_a_port;                    /* C168 */
    uint32 min_int_buff_ema_pool_cells_for_a_port;           /* C169 */
    uint32 min_ext_buff_cells_for_a_port;                    /* C170 */
    uint32 min_RE_WQEs_pkts_for_a_port;                      /* C171 */
    uint32 min_EQEs_pkts_for_a_port;                         /* C172 */

    uint32 min_int_buff_cells_for_total_port;                /* C174 */
    uint32 min_int_buff_ema_pool_cells_for_total_port;       /* C175 */
    uint32 min_ext_buff_cells_for_total_port;                /* C176 */
    uint32 min_RE_WQEs_pkts_for_total_port;                  /* C177 */
    uint32 min_EQEs_pkts_for_total_port;                     /* C178 */

    uint32 total_min_int_buff_cells;                         /* C180 */
    uint32 total_min_int_buff_ema_pool_cells;                /* C181 */
    uint32 total_min_ext_buff_cells;                         /* C182 */
    uint32 total_min_RE_WQEs_pkts;                           /* C183 */
    uint32 total_min_EQEs_pkts;                              /* C184 */

    uint32 total_shared_ing_buff_pool;                       /* C186 */
    uint32 total_shared_EMA_buff;                            /* C187 */
    uint32 total_shared_ext_buff;                            /* C188 */
    uint32 total_shared_RE_WQEs_buff;                        /* C189 */
    uint32 total_shared_EQEs_buff;                           /* C190 */

    uint32 ingress_burst_cells_size_for_one_port;            /* C192 */
    uint32 ingress_burst_pkts_size_for_one_port;             /* C193 */
    uint32 ingress_burst_cells_size_for_all_ports;           /* C194 */
    uint32 ingress_total_shared_cells_use_for_all_port;      /* C195 */
    uint32 ingress_burst_pkts_size_for_all_port;             /* C196 */
    uint32 ingress_total_shared_pkts_use_for_all_port;       /* C197 */
    uint32 ingress_total_shared_hdrm_cells_use_for_all_port; /* C198 */
    uint32 ingress_total_shared_hdrm_pkts_use_for_all_port;  /* C199 */

}_input_port_threshold_t;

typedef struct _output_port_threshold_t {
    uint32 min_grntd_res_queue_cells_int_buff;               /* C204 */
    uint32 min_grntd_res_queue_cells_ext_buff;               /* C205 */
    uint32 min_grntd_res_queue_cells_EQEs;                   /* C206 */
    uint32 min_grntd_res_EMA_queue_cells;                    /* C207 */
    uint32 min_grntd_res_RE_WQs_cells;                       /* C208 */
    uint32 min_grntd_res_RE_WQs_queue_cells_for_int_buff;    /* C209 */
    uint32 min_grntd_res_RE_WQs_queue_cells_for_ext_buff;    /* C210 */
    uint32 min_grntd_res_EP_redirect_queue_entry_cells;      /* C211 */

    uint32 min_grntd_tot_res_queue_cells_int_buff;           /* C213 */
    uint32 min_grntd_tot_res_queue_cells_ext_buff;           /* C214 */
    uint32 min_grntd_tot_res_queue_cells_EQEs;               /* C215 */
    uint32 min_grntd_tot_res_EMA_queue_cells;                /* C216 */
    uint32 min_grntd_tot_res_RE_WQs_cells;                   /* C217 */
    uint32 min_grntd_tot_res_RE_WQs_queue_cells_for_int_buff;/* C218 */
    uint32 min_grntd_tot_res_RE_WQs_queue_cells_for_ext_buff;/* C219 */
    uint32 min_grntd_tot_res_EP_redirect_queue_entry_cells;  /* C220 */

    uint32 min_grntd_tot_shr_queue_cells_int_buff;           /* C222 */
    uint32 min_grntd_tot_shr_queue_cells_ext_buff;           /* C223 */
    uint32 min_grntd_tot_shr_queue_cells_EQEs;               /* C224 */
    uint32 min_grntd_tot_shr_EMA_queue_cells;                /* C225 */
    uint32 min_grntd_tot_shr_RE_WQs_cells;                   /* C226 */
    uint32 min_grntd_tot_shr_RE_WQs_queue_cells_for_int_buff;/* C227 */
    uint32 min_grntd_tot_shr_RE_WQs_queue_cells_for_ext_buff;/* C228 */
    uint32 min_grntd_tot_shr_EP_redirect_queue_entry_cells;  /* C229 */

    uint32 egress_queue_dynamic_threshold_parameter;         /* C230 */
    uint32 egress_burst_cells_size_for_one_queue;            /* C231 */
    uint32 egress_burst_pkts_size_for_one_queue;             /* C232 */
    uint32 egress_burst_cells_size_for_all_ports;            /* C233 */
    uint32 egress_burst_pkts_size_for_all_ports;             /* C234 */
    uint32 egress_burst_cells_size_for_all_queues;           /* C235 */
    uint32 egress_burst_pkts_size_for_all_queues;            /* C236 */
    uint32 egress_total_use_in_cells_for_all_queues;         /* C237 */
    uint32 egress_total_use_in_pkts_for_all_queues;          /* C238 */
    uint32 egress_remaining_cells_for_all_queues;            /* C239 */
    uint32 egress_remaining_pkts_for_all_queues;             /* C240 */

}_output_port_threshold_t;

typedef struct _soc_ml_mmu_intermediate_results {
    _general_info_t           general_info;
    _input_port_threshold_t   input_port_threshold;
    _output_port_threshold_t  output_port_threshold;
}_soc_ml_mmu_intermediate_results_t;

/* Move sal_cell_func and sal_floor_func to sal_lib ? */
static uint32 sal_ceil_func(uint32 numerators , uint32 denominator)
{
    uint32  result;
    if (denominator == 0) {
        return 0xFFFFFFFF;
    }
    result = numerators / denominator;
    if (numerators % denominator != 0) {
        result++;
    }
    return result;
}

static uint32 sal_floor_func(uint32 numerators , uint32 denominator)
{
    uint32  result;
    if (denominator == 0) {
        return 0xFFFFFFFF;
    }
    result = numerators / denominator;
    return result;
}

STATIC
void _soc_ml_mmu_config_extra_queue(int unit, uint32 queue,
                          _soc_ml_mmu_params_t *_soc_ml_mmu_params,
                          _output_port_threshold_t *output_port_threshold)
{
    
    uint32        entry2[2];
    uint32        temp_val;

    /* 1. MMU_THDO_QCONFIG_CELL */

    /* MMU_THDO_QCONFIG_CELL
     * LIMIT_YELLOW_CELL<49:37>
     * Q_COLOR_LIMIT_DYNAMIC_CELL<36>
     * Q_COLOR_ENABLE_CELL<35>
     * Q_LIMIT_DYNAMIC_CELL<33>
     * Q_LIMIT_ENABLE_CELL<32>
     * Q_MIN_CELL<31:16>
     * Q_SHARED_LIMIT_CELL<15:0>
     */
    sal_memset(entry2,0,sizeof(uint32) * 2);
    bcm5346x_mem_get(unit, M_MMU_THDO_QCONFIG_CELL(queue), entry2, 2);

    /* THDO_QCONFIG_CELL.Q_MIN_CELL */
    entry2[0] &= ~(0xFFFF << 16);
    entry2[0] |= ((output_port_threshold->min_grntd_res_queue_cells_int_buff) << 16);

    /* THDO_QCONFIG_CELL.Q_SHARED_LIMIT_CELL */
    if (_soc_ml_mmu_params->lossless_mode_d_c) {
        temp_val = output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff;
    } else {
        temp_val = _soc_ml_mmu_params->mmu_egr_queue_dyn_thd_param_baf_profile_lossy;
    }
    entry2[0] &= ~0xFFFF;
    entry2[0] |= temp_val;

    /* THDO_QCONFIG_CELL.Q_LIMIT_DYNAMIC_CELL */
    entry2[1] &= ~(0x1 << (33-32));
    entry2[1] |= ((_soc_ml_mmu_params->lossless_mode_d_c ? 0 : 1) << (33-32));

    /* THDO_QCONFIG_CELL.Q_LIMIT_ENABLE_CELL */
    entry2[1] &= ~(0x1 << (32-32));
    entry2[1] |= ((_soc_ml_mmu_params->lossless_mode_d_c ? 0 : 1) << (32-32));

    /* THDO_QCONFIG_CELL.Q_COLOR_ENABLE_CELL = 0 */
    entry2[1] &= ~(0x1 << (35-32));

    /* THDO_QCONFIG_CELL.Q_COLOR_LIMIT_DYNAMIC_CELL */
    entry2[1] &= ~(0x1 << (36-32));
    entry2[1] |= ((_soc_ml_mmu_params->lossless_mode_d_c ? 0 : 1) << (36-32));

    /* THDO_QCONFIG_CELL.LIMIT_YELLOW_CELL = 0 */
    entry2[1] &= ~(0x1FFF << (37-32));

    bcm5346x_mem_set(unit, M_MMU_THDO_QCONFIG_CELL(queue), entry2, 2);

    /* 2. MMU_THDO_QOFFSET_CELL */

    /* MMU_THDO_QOFFSET_CELL
     * LIMIT_RED_CELL<51:39>
     * RESET_OFFSET_RED_CELL<38:26>
     * RESET_OFFSET_YELLOW_CELL<25:13>
     * RESET_OFFSET_CELL<12:0>
     */
    sal_memset(entry2,0,sizeof(uint32) * 2);
    bcm5346x_mem_get(unit, M_MMU_THDO_QOFFSET_CELL(queue), entry2, 2);

    /* THDO_QOFFSET_CELL.RESET_OFFSET_CELL = 2 */
    entry2[0] &= ~0x1FFF;
    entry2[0] |= 0x2;

    /* THDO_QOFFSET_CELL.LIMIT_RED_CELL = 0 */ 
    entry2[1] &= ~(0x1FFF << (39-32));

    /* THDO_QOFFSET_CELL.RESET_OFFSET_YELLOW_CELL = 2 */
    entry2[0] &= ~(0x1FFF << 13);
    entry2[0] |= (0x2 << 13);

    /* THDO_QOFFSET_CELL.RESET_OFFSET_RED_CELL = 2 */
    entry2[0] &= ~(0x3F << 26);
    entry2[1] &= ~(0x7F << (32-32));
    entry2[0] |= (0x2 << 26);

    bcm5346x_mem_set(unit, M_MMU_THDO_QOFFSET_CELL(queue), entry2, 2);

    /* 3. MMU_THDO_QCONFIG_QENTRY */

    /* MMU_THDO_QCONFIG_QENTRY
     * LIMIT_YELLOW_QENTRY<49:37>
     * Q_COLOR_LIMIT_DYNAMIC_QENTRY<36>
     * Q_COLOR_ENABLE_QENTRY<35>
     * Q_LIMIT_DYNAMIC_QENTRY<33>
     * Q_LIMIT_ENABLE_QENTRY<32>
     * Q_MIN_QENTRY<31:16>
     * Q_SHARED_LIMIT_QENTRY<15:0>
     */
    sal_memset(entry2,0,sizeof(uint32) * 2);
    bcm5346x_mem_get(unit, M_MMU_THDO_QCONFIG_QENTRY(queue), entry2, 2);

    /* THDO_QCONFIG_QENTRY.Q_MIN_QENTRY */
    entry2[0] &= ~(0xFFFF << 16);
    entry2[0] |= ((output_port_threshold->min_grntd_res_queue_cells_EQEs) << 16);
 
    /* THDO_QCONFIG_QENTRY.Q_SHARED_LIMIT_QENTRY */
    if (_soc_ml_mmu_params->lossless_mode_d_c) {
        temp_val = output_port_threshold->min_grntd_tot_shr_queue_cells_EQEs;
    } else {
        temp_val = _soc_ml_mmu_params->mmu_egr_queue_dyn_thd_param_baf_profile_lossy;
    }
    entry2[0] &= ~0xFFFF;
    entry2[0] |= temp_val;

    /* THDO_QCONFIG_QENTRY.Q_LIMIT_DYNAMIC_QENTRY */
    entry2[1] &= ~(0x1 << (33-32));
    entry2[1] |= ((_soc_ml_mmu_params->lossless_mode_d_c ? 0 : 1) << (33-32));

    /* THDO_QCONFIG_QENTRY.Q_LIMIT_ENABLE_QENTRY */
    entry2[1] &= ~(0x1 << (32-32));
    entry2[1] |= ((_soc_ml_mmu_params->lossless_mode_d_c ? 0 : 1) << (32-32));

    /* THDO_QCONFIG_QENTRY.Q_COLOR_ENABLE_QENTRY = 0 */
    entry2[1] &= ~(0x1 << (35-32));

    /* THDO_QCONFIG_QENTRY.Q_COLOR_LIMIT_DYNAMIC_QENTRY */
    entry2[1] &= ~(0x1 << (36-32));
    entry2[1] |= ((_soc_ml_mmu_params->lossless_mode_d_c ? 0 : 1) << (36-32));

    /* THDO_QCONFIG_QENTRY.LIMIT_YELLOW_QENTRY= 0 */
    entry2[1] &= ~(0x1FFF << (37-32));
    bcm5346x_mem_set(unit, M_MMU_THDO_QCONFIG_QENTRY(queue), entry2, 2);

    /* 4. MMU_THDO_QOFFSET_QENTRY */

    /* MMU_THDO_QOFFSET_QENTRY
     * LIMIT_RED_QENTRY<51:39>
     * RESET_OFFSET_RED_QENTRY<38:26>
     * RESET_OFFSET_YELLOW_QENTRY<25:13>
     * RESET_OFFSET_QENTRY<12:0>
     */
    sal_memset(entry2,0,sizeof(uint32) * 2);
    bcm5346x_mem_get(unit, M_MMU_THDO_QOFFSET_QENTRY(queue), entry2, 2);

    /* THDO_QOFFSET_QENTRY.RESET_OFFSET_QENTRY = 1 */
    entry2[0] &= ~0x1FFF;
    entry2[0] |= 0x1;

    /* THDO_QOFFSET_QENTRY.LIMIT_RED_QENTRY  = 0 */ 
    entry2[1] &= ~(0x1FFF << (39-32));

    /* THDO_QOFFSET_QENTRY.RESET_OFFSET_YELLOW_QENTRY = 1 */
    entry2[0] &= ~(0x1FFF << 13);
    entry2[0] |=( 0x1 << 13);

    /* THDO_QOFFSET_QENTRY.RESET_OFFSET_RED_QENTRY = 1 */
    entry2[0] &= ~(0x3F << 26);
    entry2[1] &= ~(0x7F << (32-32));
    entry2[0] |= (0x1 << 26);

    bcm5346x_mem_set(unit, M_MMU_THDO_QOFFSET_QENTRY(queue), entry2, 2);
}

static void
soc_mmu_init_helper(uint8 unit){
    uint32        pbmp_int_ge_count=0;
    uint32        pbmp_int_hg_count=0;
    uint32        mmu_num_olp_port=0;
    
    uint32        port=0;
    uint32        available_internal_buffer=100;
    uint32        reserve_internal_buffer=0;
    uint32        temp_val;
    uint32        rval=0;
    uint32        service_pool=0;
    uint32        queue;
    uint32        op_node=0;
    uint32        op_node_offset=0;
    uint32        cos;
    uint32        idx;

    uint32        mem_idx=0;
    uint32        entry2[2], entry3[3];

    _soc_ml_mmu_intermediate_results_t _soc_ml_mmu_intermediate_results;
    _general_info_t *general_info = 
        &_soc_ml_mmu_intermediate_results.general_info;
    _input_port_threshold_t *input_port_threshold =
        &_soc_ml_mmu_intermediate_results.input_port_threshold;
    _output_port_threshold_t *output_port_threshold = 
        &_soc_ml_mmu_intermediate_results.output_port_threshold;

    _soc_ml_mmu_params_t _soc_ml_mmu_params;

    sal_memset(&_soc_ml_mmu_intermediate_results, 0, sizeof(_soc_ml_mmu_intermediate_results_t));
    sal_memset(&_soc_ml_mmu_params, 0, sizeof(_soc_ml_mmu_params_t));

    /********************************************************************/
    /***************** Fixed Parameter **********************************/
    /********************************************************************/
    _soc_ml_mmu_params.mmu_min_pkt_size = 64;              /*C5*/
    _soc_ml_mmu_params.mmu_ethernet_mtu_bytes=1536;        /*C6*/
    _soc_ml_mmu_params.mmu_max_pkt_size = 12288;           /*C7*/
    _soc_ml_mmu_params.mmu_jumbo_frame_size = 9216;        /*C8*/
    _soc_ml_mmu_params.mmu_int_buf_cell_size = 190;        /*C9*/
    _soc_ml_mmu_params.mmu_pkt_header_size = 62;           /*C10*/
    _soc_ml_mmu_params.mmu_lossless_pg_num = 1;            /*C13*/
    _soc_ml_mmu_params.mmu_line_rate_ge = 1;               /*C15*/
    _soc_ml_mmu_params.mmu_line_rate_hg = 10;              /*C18*/
    _soc_ml_mmu_params.mmu_num_cpu_port = 1;               /*C20*/
    _soc_ml_mmu_params.mmu_num_cpu_queue = 48;             /*C21*/
    _soc_ml_mmu_params.mmu_num_loopback_port = 1;          /*C22*/
    _soc_ml_mmu_params.mmu_num_loopback_queue = 8;
    _soc_ml_mmu_params.mmu_num_ep_redirection_queue = 16;   /**/
    _soc_ml_mmu_params.mmu_num_olp_queue = 8;               /*C26*/
    _soc_ml_mmu_params.mmu_pipeline_lat_cpap_admission = 30;/*C28*/
    _soc_ml_mmu_params.mmu_reserved_int_buf_cells = 42;     /*C30*/
    _soc_ml_mmu_params.mmu_ep_redirect_queue_entries = 480; /*C32*/
    _soc_ml_mmu_params.mmu_repl_engine_work_queue_entries = 1;/*C34*/
    _soc_ml_mmu_params.mmu_resv_repl_engine_work_queue_entries = 0;    /**/
    _soc_ml_mmu_params.mmu_repl_engine_work_queue_in_device = 12;    /*C36*/
    _soc_ml_mmu_params.mmu_ing_port_dyn_thd_param = 2; /*C39*/
    _soc_ml_mmu_params.mmu_ing_pg_dyn_thd_param = 7;  /*C40*/
    _soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf = 2; /*C41*/
    _soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf_profile_lossy = 7;  /*C42*/
    _soc_ml_mmu_params.mmu_ing_cell_buf_reduction = 0; /*C43*/
    _soc_ml_mmu_params.mmu_ing_pkt_buf_reduction = 0; /*C44*/

    /* C11 */
    _soc_ml_mmu_params.lossless_mode_d_c =  1;
    /* C12 */
    _soc_ml_mmu_params.pfc_pause_mode_d_c =  0;

    /* C29 */
    _soc_ml_mmu_params.mmu_int_buf_size = 10922;

    pbmp_int_ge_count = 4;
    pbmp_int_hg_count = 8;
	mmu_num_olp_port = 1;

    /* C14 */
    _soc_ml_mmu_params.num_ge_ports_d = pbmp_int_ge_count ;

    /* C14 */
    _soc_ml_mmu_params.num_ge_int_ports_d = pbmp_int_ge_count;
    /* C16 */
    _soc_ml_mmu_params.num_egr_queue_per_int_ge_port_d = pbmp_int_ge_count ? 8:0;
    /* C17 */
    _soc_ml_mmu_params.num_hg_ports_d = pbmp_int_hg_count ;
    /* C17 */
    _soc_ml_mmu_params.num_hg_int_ports_d = pbmp_int_hg_count ;
    /* C19 */
    _soc_ml_mmu_params.num_egr_queue_per_int_hg_port_d = pbmp_int_hg_count ? 8:0;
    /* C25 */
    _soc_ml_mmu_params.mmu_num_olp_port_d = mmu_num_olp_port;
    /* */
    _soc_ml_mmu_params.olp_port_int_ext_bounding_d_c = 0;

    /* C27 */
    _soc_ml_mmu_params.mmu_total_egress_queue = 512;

    /* C33 */
    /* C51 = C14+C17-1 */
    _soc_ml_mmu_params.mmu_exp_num_of_repl_per_pkt =
        _soc_ml_mmu_params.num_ge_ports_d +
        _soc_ml_mmu_params.num_hg_ports_d - 1 ;

    available_internal_buffer=100;
    reserve_internal_buffer=0;

    /*  */
    _soc_ml_mmu_params.mmu_available_int_buf_size_d=available_internal_buffer;
    /* (_soc_ml_mmu_params.mmu_int_buf_size*available_internal_buffer)/100; */
    /*  */
    _soc_ml_mmu_params.mmu_reserved_int_buf_ema_pool_size_d=
        reserve_internal_buffer;
    /* (_soc_ml_mmu_params.mmu_int_buf_size*reserve_internal_buffer)/100; */

    /*  */
    _soc_ml_mmu_params.internal_buffer_reduction_d_c = 0;

    /* C36 */
    _soc_ml_mmu_params.mmu_egress_queue_entries = 12;

    /* C38 = CEILING((C6*1024+C10)/C9, 1) */
    _soc_ml_mmu_params.per_cos_res_cells_for_int_buff_d =
        sal_ceil_func((_soc_ml_mmu_params.mmu_ethernet_mtu_bytes +
                    _soc_ml_mmu_params.mmu_pkt_header_size),
                _soc_ml_mmu_params.mmu_int_buf_cell_size);


    /***********************************************************************************/
    /*********************** Intermediate Results Processing****************************/
    /***********************************************************************************/

    /* C62 = CEILING((C7+C10)/C9,1) */
    general_info->max_packet_size_in_cells = sal_ceil_func(
            (_soc_ml_mmu_params.mmu_max_pkt_size +
             _soc_ml_mmu_params.mmu_pkt_header_size),
            _soc_ml_mmu_params.mmu_int_buf_cell_size);

    /* C63 = CEILING((C8+C10)/C9,1) */
    general_info->jumbo_frame_for_int_buff = sal_ceil_func(
            (_soc_ml_mmu_params.mmu_jumbo_frame_size +
             _soc_ml_mmu_params.mmu_pkt_header_size),
            _soc_ml_mmu_params.mmu_int_buf_cell_size);

    /* C64 =CEILING((C6*1024+C10)/C9, 1) */
    general_info->ether_mtu_cells_for_int_buff = sal_ceil_func(
            (_soc_ml_mmu_params.mmu_ethernet_mtu_bytes) +
            _soc_ml_mmu_params.mmu_pkt_header_size,
            _soc_ml_mmu_params.mmu_int_buf_cell_size);


    /* C66 = C14+C17+C20+C22+C25 */
    general_info->total_num_of_ports =
        _soc_ml_mmu_params.num_ge_ports_d +
        _soc_ml_mmu_params.num_hg_ports_d +
        _soc_ml_mmu_params.mmu_num_cpu_port +
        _soc_ml_mmu_params.mmu_num_loopback_port +
        _soc_ml_mmu_params.mmu_num_olp_port_d;

    /* C67 = C14+C17+C20+C25  */
    general_info->total_num_of_ports_excl_lpbk =
        _soc_ml_mmu_params.num_ge_ports_d +
        _soc_ml_mmu_params.num_hg_ports_d +
        _soc_ml_mmu_params.mmu_num_cpu_port +
        _soc_ml_mmu_params.mmu_num_olp_port_d;

    /* C68 = C14+C17+C20 */
    general_info->total_num_of_ports_excl_lpbk_olp =
        _soc_ml_mmu_params.num_ge_ports_d +
        _soc_ml_mmu_params.num_hg_ports_d +
        _soc_ml_mmu_params.mmu_num_cpu_port;

    /* C69 = C14+C17 */
    general_info->total_num_of_ports_excl_lpbk_olp_cpu =
        _soc_ml_mmu_params.num_ge_ports_d +
        _soc_ml_mmu_params.num_hg_ports_d;

    /* C71 = C16*C14 */
    general_info->total_egr_queues_for_a_int_ge_ports =
        (_soc_ml_mmu_params.num_egr_queue_per_int_ge_port_d *
         _soc_ml_mmu_params.num_ge_int_ports_d);

    /* C72 = C19*C17 */
    general_info->total_egr_queues_for_a_int_hg_ports =
        (_soc_ml_mmu_params.num_egr_queue_per_int_hg_port_d *
         _soc_ml_mmu_params.num_hg_int_ports_d);

    /* C73 = C20*C21 */
    general_info->total_cpu_queues =
        (_soc_ml_mmu_params.mmu_num_cpu_port *
         _soc_ml_mmu_params.mmu_num_cpu_queue);

    /* C74 = $C$71+$C$72+C73+C26*C25+C22*C23+C22*C24 */
    general_info->total_base_queue_int_buff =
        (general_info->total_egr_queues_for_a_int_ge_ports +
         general_info->total_egr_queues_for_a_int_hg_ports +
         general_info->total_cpu_queues);
    if (_soc_ml_mmu_params.olp_port_int_ext_bounding_d_c == 0) {
        general_info->total_base_queue_int_buff +=
            (_soc_ml_mmu_params.mmu_num_olp_queue *
             _soc_ml_mmu_params.mmu_num_olp_port_d);
    }
    general_info->total_base_queue_int_buff +=
        (_soc_ml_mmu_params.mmu_num_loopback_queue *
         _soc_ml_mmu_params.mmu_num_loopback_port);
    general_info->total_base_queue_int_buff +=
        ((_soc_ml_mmu_params.mmu_num_ep_redirection_queue *
          _soc_ml_mmu_params.mmu_num_loopback_port)/ 2);

    if (_soc_ml_mmu_params.olp_port_int_ext_bounding_d_c) {
        general_info->total_base_queue_ext_buff +=
            (_soc_ml_mmu_params.mmu_num_olp_queue *
             _soc_ml_mmu_params.mmu_num_olp_port_d);
    }

    /* C74 */
    general_info->total_egr_base_queues_in_device =
        general_info->total_base_queue_int_buff +
        general_info->total_base_queue_ext_buff;
    /* C75 = C27 */
    general_info->total_egr_queues_in_device =
        _soc_ml_mmu_params.mmu_total_egress_queue ;

    /* C95 = C42-C44*/
    general_info->max_int_cell_buff_size =
        _soc_ml_mmu_params.mmu_int_buf_size -
        _soc_ml_mmu_params.mmu_reserved_int_buf_cells ;

    /* C96 = C95 */
    general_info->int_cell_buff_size_after_limitation =
        general_info->max_int_cell_buff_size;

    general_info->int_buff_pool = general_info->max_int_cell_buff_size;

    /* C99 =IF(C17=1,FLOOR($C$45*(IF($C$46,$C$96,$C$95))-C41,1),0) */
    if (_soc_ml_mmu_params.internal_buffer_reduction_d_c) {
        temp_val = general_info->int_cell_buff_size_after_limitation;
    } else {
        temp_val = general_info->max_int_cell_buff_size;;
    }
    temp_val = (temp_val * _soc_ml_mmu_params.mmu_reserved_int_buf_ema_pool_size_d)/100;
    temp_val = temp_val - _soc_ml_mmu_params.mmu_pipeline_lat_cpap_admission;

    /* C101 =C52*1024-C53*/
    general_info->repl_engine_work_queue_entries =
        _soc_ml_mmu_params.mmu_repl_engine_work_queue_entries*1024 -
        _soc_ml_mmu_params.mmu_resv_repl_engine_work_queue_entries;

    /* C104 = 1 */
    general_info->int_buff_cells_per_avg_size_pkt = 1;

    /* C106 = 0.75 but normalized to 1 */
    general_info->max_prop_of_buff_used_by_one_queue_port = 75;

    /* Fill up input threshold info */
    /* C110 =CEILING(C7*2/C9, 1)  */
    input_port_threshold->global_hdrm_cells_for_int_buff_ingress_pool = sal_ceil_func
        (_soc_ml_mmu_params.mmu_max_pkt_size * 2 ,
         _soc_ml_mmu_params.mmu_int_buf_cell_size);

    /* C113 */
    input_port_threshold->global_hdrm_cells_for_RE_WQEs = 0 ;
    /* C114 */
    input_port_threshold->global_hdrm_cells_for_EQEs = 0 ;
    if (_soc_ml_mmu_params.lossless_mode_d_c) {
        /* C115*/
         input_port_threshold->hdrm_int_buff_cells_for_10G_PG = 178;
        /* C116*/
        input_port_threshold->hdrm_ema_buff_cells_for_10G_PG = 0;
        /* C117*/
        input_port_threshold->hdrm_RE_WQEs_pkts_for_10G_PG = 66;
        /* C119 =IF(C14=0, 0, C118*C51)*/
        input_port_threshold->hdrm_EQEs_pkts_for_10G_PG =
            input_port_threshold->hdrm_RE_WQEs_pkts_for_10G_PG *
            _soc_ml_mmu_params.mmu_exp_num_of_repl_per_pkt ;
    } else {
        /* C115*/
        input_port_threshold->hdrm_int_buff_cells_for_10G_PG = 0;
        /* C116*/
        input_port_threshold->hdrm_ema_buff_cells_for_10G_PG = 0;
        /* C118*/
        input_port_threshold->hdrm_RE_WQEs_pkts_for_10G_PG = 0;
        /* C119*/
        input_port_threshold->hdrm_EQEs_pkts_for_10G_PG = 0;
    }

    /* C120 = =C25*C16*$C$115 */
    input_port_threshold->hdrm_int_buff_cells_for_10G_total_PG =
        (_soc_ml_mmu_params.num_hg_ports_d *
         _soc_ml_mmu_params.mmu_lossless_pg_num *
         input_port_threshold->hdrm_int_buff_cells_for_10G_PG);

    /* C121 = C25*C16*C116 */
    input_port_threshold->hdrm_ema_buff_cells_for_10G_total_PG =
        (_soc_ml_mmu_params.num_hg_ports_d *
         _soc_ml_mmu_params.mmu_lossless_pg_num *
         input_port_threshold->hdrm_ema_buff_cells_for_10G_PG);

    /* C123 = C25*C16*C118 */
    input_port_threshold->hdrm_RE_WQEs_pkts_for_10G_total_PG =
        (_soc_ml_mmu_params.num_hg_ports_d *
         _soc_ml_mmu_params.mmu_lossless_pg_num *
         input_port_threshold->hdrm_RE_WQEs_pkts_for_10G_PG);

    /* C124 = C25*C16*C119 */
    input_port_threshold->hdrm_EQEs_pkts_for_10G_total_PG =
        (_soc_ml_mmu_params.num_hg_ports_d *
         _soc_ml_mmu_params.mmu_lossless_pg_num *
         input_port_threshold->hdrm_EQEs_pkts_for_10G_PG);

    if (_soc_ml_mmu_params.lossless_mode_d_c) {
        /* C125 */
        input_port_threshold->hdrm_int_buff_cells_for_1G_PG = 130;
        /* C126 */
        input_port_threshold->hdrm_ema_buff_cells_for_1G_PG = 0;
        /* C128*/
        input_port_threshold->hdrm_RE_WQEs_pkts_for_1G_PG = 23;
        /* C129 =IF(C14=0, 0, C128*C51) */
        input_port_threshold->hdrm_EQEs_pkts_for_1G_PG =
            input_port_threshold->hdrm_RE_WQEs_pkts_for_1G_PG *
            _soc_ml_mmu_params.mmu_exp_num_of_repl_per_pkt;
    } else {
        /* C125 */
        input_port_threshold->hdrm_int_buff_cells_for_1G_PG = 0;
        /* C126 */
        input_port_threshold->hdrm_ema_buff_cells_for_1G_PG = 0;
        /* C128*/
        input_port_threshold->hdrm_RE_WQEs_pkts_for_1G_PG = 0;
        /* C129*/
        input_port_threshold->hdrm_EQEs_pkts_for_1G_PG = 0;
    }

    /* C130 =C19*C16*$C$125 */
    input_port_threshold->hdrm_int_buff_cells_for_1G_total_PG =
        (_soc_ml_mmu_params.num_ge_ports_d *
         _soc_ml_mmu_params.mmu_lossless_pg_num *
         input_port_threshold->hdrm_int_buff_cells_for_1G_PG);

    /* C131 = C19*C16*$C$126 */
    input_port_threshold->hdrm_ema_buff_cells_for_1G_total_PG =
        (_soc_ml_mmu_params.num_ge_ports_d *
         _soc_ml_mmu_params.mmu_lossless_pg_num *
         input_port_threshold->hdrm_ema_buff_cells_for_1G_PG);

    /* C133 = C19*C16*$C$128 */
    input_port_threshold->hdrm_RE_WQEs_pkts_for_1G_total_PG =
        (_soc_ml_mmu_params.num_ge_ports_d *
         _soc_ml_mmu_params.mmu_lossless_pg_num *
         input_port_threshold->hdrm_RE_WQEs_pkts_for_1G_PG);

    /* C134 = C19*C16*$C$129 */
    input_port_threshold->hdrm_EQEs_pkts_for_1G_total_PG =
        (_soc_ml_mmu_params.num_ge_ports_d *
         _soc_ml_mmu_params.mmu_lossless_pg_num *
         input_port_threshold->hdrm_EQEs_pkts_for_1G_PG);

    /* C132-C135 values are zero for lossy mode */
    if (_soc_ml_mmu_params.lossless_mode_d_c) {
        /* C135 */
        input_port_threshold->hdrm_int_buff_cells_for_olp_port = 180;
        /* C136 */
        input_port_threshold->hdrm_ema_buff_cells_for_olp_port = 0;
        /* C138 */
        input_port_threshold->hdrm_RE_WQEs_pkts_for_olp_port = 7;
        /* C139 = C138*C51 */
        input_port_threshold->hdrm_EQEs_pkts_for_olp_port =
            input_port_threshold->hdrm_RE_WQEs_pkts_for_olp_port *
            _soc_ml_mmu_params.mmu_exp_num_of_repl_per_pkt;
    } else {
        /* C135 */
        input_port_threshold->hdrm_int_buff_cells_for_olp_port = 0;
        /* C136 */
        input_port_threshold->hdrm_int_buff_cells_for_olp_port = 0;
        /* C138 */
        input_port_threshold->hdrm_RE_WQEs_pkts_for_olp_port = 0;
        /* C139 */
        input_port_threshold->hdrm_EQEs_pkts_for_olp_port = 0;
    }

    /* C140 = 44 */
    input_port_threshold->hdrm_int_buff_cells_for_lpbk_port =  44;

    /* C143 = =IF(C14=0, 23, 23) */
    input_port_threshold->hdrm_RE_WQEs_pkts_for_lpbk_port =  23;

    /* C144 = C143*C51 */
    input_port_threshold->hdrm_EQEs_pkts_for_lpbk_port =
        input_port_threshold->hdrm_RE_WQEs_pkts_for_lpbk_port *
        _soc_ml_mmu_params.mmu_exp_num_of_repl_per_pkt;

    /* C145 = 44 */
    input_port_threshold->hdrm_int_buff_cells_for_cpu_port =  44;

    /* C148 =IF(C14=0, 23, 23) */
    input_port_threshold->hdrm_RE_WQEs_pkts_for_cpu_port = 23;

    /* C149 = C148*C51 */
    input_port_threshold->hdrm_EQEs_pkts_for_cpu_port =
        input_port_threshold->hdrm_RE_WQEs_pkts_for_cpu_port *
        _soc_ml_mmu_params.mmu_exp_num_of_repl_per_pkt;

    /* C150 =$C$110+$C$120+$C$130+C135+C140+C145*/
    input_port_threshold->total_hdrm_int_buff_cells =
        input_port_threshold->global_hdrm_cells_for_int_buff_ingress_pool +
        input_port_threshold->hdrm_int_buff_cells_for_10G_total_PG +
        input_port_threshold->hdrm_int_buff_cells_for_1G_total_PG +
        input_port_threshold->hdrm_int_buff_cells_for_olp_port +
        input_port_threshold->hdrm_int_buff_cells_for_lpbk_port +
        input_port_threshold->hdrm_int_buff_cells_for_cpu_port;

    /* C151 = =C111+C121+C131+C136+C141+C146*/
    input_port_threshold->total_hdrm_int_buff_ema_pool_cells =
        input_port_threshold->global_hdrm_cells_for_int_buff_ema_pool +
        input_port_threshold->hdrm_ema_buff_cells_for_10G_total_PG +
        input_port_threshold->hdrm_ema_buff_cells_for_1G_total_PG +
        input_port_threshold->hdrm_ema_buff_cells_for_olp_port +
        input_port_threshold->hdrm_ema_buff_cells_for_lpbk_port +
        input_port_threshold->hdrm_ema_buff_cells_for_cpu_port;


    /* C153 = =$C$113+$C$123+$C$133+C138+C143+C148 */
    input_port_threshold->total_hdrm_RE_WQEs_pkts =
        input_port_threshold->global_hdrm_cells_for_RE_WQEs+
        input_port_threshold->hdrm_RE_WQEs_pkts_for_10G_total_PG+
        input_port_threshold->hdrm_RE_WQEs_pkts_for_1G_total_PG+
        input_port_threshold->hdrm_RE_WQEs_pkts_for_olp_port+
        input_port_threshold->hdrm_RE_WQEs_pkts_for_lpbk_port+
        input_port_threshold->hdrm_RE_WQEs_pkts_for_cpu_port;

    /* C154 =$C$114+$C$124+$C$134+C139+C144+C149*/
    input_port_threshold->total_hdrm_EQEs_pkts =
        input_port_threshold->global_hdrm_cells_for_EQEs+
        input_port_threshold->hdrm_EQEs_pkts_for_10G_total_PG+
        input_port_threshold->hdrm_EQEs_pkts_for_1G_total_PG+
        input_port_threshold->hdrm_EQEs_pkts_for_olp_port+
        input_port_threshold->hdrm_EQEs_pkts_for_lpbk_port+
        input_port_threshold->hdrm_EQEs_pkts_for_cpu_port;

    /* C156 = $C$73 */
    input_port_threshold->min_int_buff_cells_per_PG =
        general_info->jumbo_frame_for_int_buff;

    /* C159 */
    input_port_threshold->min_RE_WQEs_pkt_per_PG = 9;

    /* C160 = =$C$159*C51 */
    input_port_threshold->min_EQEs_pkt_per_PG =
        input_port_threshold->min_RE_WQEs_pkt_per_PG *
        _soc_ml_mmu_params.mmu_exp_num_of_repl_per_pkt;

    /* C162 = =C79*C16*$C$156 */
    input_port_threshold->min_int_buff_cells_for_total_PG =
        (general_info->total_num_of_ports *
         _soc_ml_mmu_params.mmu_lossless_pg_num *
         input_port_threshold->min_int_buff_cells_per_PG) ;

    /* C163 = =C79*C16*$C$157 */
    input_port_threshold->min_int_buff_ema_pool_cells_for_total_PG =
        (general_info->total_num_of_ports *
         _soc_ml_mmu_params.mmu_lossless_pg_num *
         input_port_threshold->min_int_buff_ema_pool_cells_per_PG);

    /* C165 = C75*C16*$C$159 */
    input_port_threshold->min_RE_WQEs_pkts_for_total_PG =
        (general_info->total_num_of_ports *
         _soc_ml_mmu_params.mmu_lossless_pg_num *
         input_port_threshold->min_RE_WQEs_pkt_per_PG) ;

    /* C166 = C75*C16*$C$160 */
    input_port_threshold->min_EQEs_pkts_for_total_PG =
        (general_info->total_num_of_ports *
         _soc_ml_mmu_params.mmu_lossless_pg_num *
         input_port_threshold->min_EQEs_pkt_per_PG) ;

    /* C168 C169 C170 C171 C172= 0 */
    input_port_threshold->min_int_buff_cells_for_a_port = 0;
    input_port_threshold->min_int_buff_ema_pool_cells_for_a_port = 0;
    input_port_threshold->min_RE_WQEs_pkts_for_a_port = 0;
    input_port_threshold->min_EQEs_pkts_for_a_port = 0;

    /* C174 =$C$79*$C$168 */
    input_port_threshold->min_int_buff_cells_for_total_port =
        (general_info->total_num_of_ports *
         input_port_threshold->min_int_buff_cells_for_a_port);

    /* C175 = $C$75*$C$169 */
    input_port_threshold->min_int_buff_ema_pool_cells_for_total_port =
        (general_info->total_num_of_ports *
         input_port_threshold->min_int_buff_ema_pool_cells_for_a_port);


    /* C177 = $C$75*$C$171 */
    input_port_threshold->min_RE_WQEs_pkts_for_total_port =
        (general_info->total_num_of_ports *
         input_port_threshold->min_RE_WQEs_pkts_for_a_port);

    /* C179 = $C$75*$C$172 */
    input_port_threshold->min_EQEs_pkts_for_total_port =
        (general_info->total_num_of_ports *
         input_port_threshold->min_EQEs_pkts_for_a_port);

    /* C180 =$C$162+$C$174 */
    input_port_threshold->total_min_int_buff_cells =
        input_port_threshold->min_int_buff_cells_for_total_PG +
        input_port_threshold->min_int_buff_cells_for_total_port;

    /* C181 = $C$163+$C$175 */
    input_port_threshold->total_min_int_buff_ema_pool_cells =
        input_port_threshold->min_int_buff_ema_pool_cells_for_total_PG +
        input_port_threshold->min_int_buff_ema_pool_cells_for_total_port;


    /* C183 = $C$165+$C$177 */
    input_port_threshold->total_min_RE_WQEs_pkts =
        input_port_threshold->min_RE_WQEs_pkts_for_total_PG +
        input_port_threshold->min_RE_WQEs_pkts_for_total_port;

    /* C184 = $C$166+$C$178 */
    input_port_threshold->total_min_EQEs_pkts =
        input_port_threshold->min_EQEs_pkts_for_total_PG +
        input_port_threshold->min_EQEs_pkts_for_total_port;


    /* Fill up output threshold info */
    /* C204 = =IF(C14=0, C58, 0) */
    if (_soc_ml_mmu_params.lossless_mode_d_c == 0) {
        output_port_threshold->min_grntd_res_queue_cells_int_buff =
            _soc_ml_mmu_params.per_cos_res_cells_for_int_buff_d;
    }
    /* C206 = =IF(C14=0, $C$58/$C$105, 0) */
    if (_soc_ml_mmu_params.lossless_mode_d_c == 0) {
        output_port_threshold->min_grntd_res_queue_cells_EQEs =
            _soc_ml_mmu_params.per_cos_res_cells_for_int_buff_d;
    }
    /* C208 = C204 */
    output_port_threshold->min_grntd_res_RE_WQs_cells =
        output_port_threshold->min_grntd_res_queue_cells_int_buff;

    /* C209 = C204 */
    output_port_threshold->min_grntd_res_RE_WQs_queue_cells_for_int_buff =
        output_port_threshold->min_grntd_res_queue_cells_int_buff;


    /* C211 = 4 */
    output_port_threshold->min_grntd_res_EP_redirect_queue_entry_cells = 4;


    /* C213 =C89*$C$204 */
    output_port_threshold->min_grntd_tot_res_queue_cells_int_buff =
        general_info->total_base_queue_int_buff *
        output_port_threshold->min_grntd_res_queue_cells_int_buff;


    /* C215  =C94*$C$206 */
    output_port_threshold->min_grntd_tot_res_queue_cells_EQEs =
        general_info->total_egr_base_queues_in_device *
        output_port_threshold->min_grntd_res_queue_cells_EQEs;

    /* C216 =$C$55*$C$207 */
    output_port_threshold->min_grntd_tot_res_EMA_queue_cells =
        _soc_ml_mmu_params.mmu_ema_queues *
        output_port_threshold->min_grntd_res_EMA_queue_cells;

    /* C217 =$C$54*$C$208 */
    output_port_threshold->min_grntd_tot_res_RE_WQs_cells =
        _soc_ml_mmu_params.mmu_repl_engine_work_queue_in_device *
        output_port_threshold->min_grntd_res_RE_WQs_cells;

    /* C218 =$C$54*$C$209 */
    output_port_threshold->min_grntd_tot_res_RE_WQs_queue_cells_for_int_buff =
        _soc_ml_mmu_params.mmu_repl_engine_work_queue_in_device *
        output_port_threshold->min_grntd_res_RE_WQs_queue_cells_for_int_buff;


    /* C220 =C211*(C36) */
    output_port_threshold->min_grntd_tot_res_EP_redirect_queue_entry_cells =
        (_soc_ml_mmu_params.mmu_num_ep_redirection_queue *
        output_port_threshold->min_grntd_res_EP_redirect_queue_entry_cells)/2;


    /* C186 =$C$98-$C$150-$C$180-IF(C14=0, 0, C213)-IF(C14=0, 0, C218) */

    input_port_threshold->total_shared_ing_buff_pool =
        general_info->int_buff_pool -
        input_port_threshold->total_min_int_buff_cells -
        input_port_threshold->total_hdrm_int_buff_cells;
    if (_soc_ml_mmu_params.lossless_mode_d_c) {
        input_port_threshold->total_shared_ing_buff_pool -=
            output_port_threshold->min_grntd_tot_res_queue_cells_int_buff;
        input_port_threshold->total_shared_ing_buff_pool -=
            output_port_threshold->min_grntd_tot_res_RE_WQs_queue_cells_for_int_buff;
    }



    /* C189 =$C$101-$C$153-$C$183-IF(C14=0, 0, C217) */
    input_port_threshold->total_shared_RE_WQEs_buff =
        general_info->repl_engine_work_queue_entries -
        input_port_threshold->total_hdrm_RE_WQEs_pkts -
        input_port_threshold->total_min_RE_WQEs_pkts;
    if (_soc_ml_mmu_params.lossless_mode_d_c) {
        input_port_threshold->total_shared_RE_WQEs_buff -=
            output_port_threshold->min_grntd_tot_res_RE_WQs_cells;
    }

    /* C190 =($C$49*1024)-$C$154-$C$184-IF(C14=0, 0, C215)-1 */
    input_port_threshold->total_shared_EQEs_buff =
        (_soc_ml_mmu_params.mmu_egress_queue_entries * 1024) -
        input_port_threshold->total_hdrm_EQEs_pkts -
        input_port_threshold->total_min_EQEs_pkts;
    if (_soc_ml_mmu_params.lossless_mode_d_c) {
        input_port_threshold->total_shared_EQEs_buff -=
            output_port_threshold->min_grntd_tot_res_queue_cells_EQEs;
    }
    input_port_threshold->total_shared_EQEs_buff -= 1;

    /* C192 =FLOOR(($C$60*$C$186)/(1+($C$60*1)),1) */
    input_port_threshold->ingress_burst_cells_size_for_one_port = sal_floor_func(
            (_soc_ml_mmu_params.mmu_ing_port_dyn_thd_param *
             input_port_threshold->total_shared_ing_buff_pool),
            (1+(_soc_ml_mmu_params.mmu_ing_port_dyn_thd_param*1)));

    /* C193 =FLOOR(($C$60*$C$190)/(1+($C$60*1)),1) */
    input_port_threshold->ingress_burst_pkts_size_for_one_port = sal_floor_func(
            (_soc_ml_mmu_params.mmu_ing_port_dyn_thd_param *
             input_port_threshold->total_shared_EQEs_buff),
            (1+(_soc_ml_mmu_params.mmu_ing_port_dyn_thd_param*1)));

    /* C194 =FLOOR(($C$60*$C$186)/(1+($C$60*$C$79)),1) */
    input_port_threshold->ingress_burst_cells_size_for_all_ports = sal_floor_func(
            (_soc_ml_mmu_params.mmu_ing_port_dyn_thd_param *
             input_port_threshold->total_shared_ing_buff_pool),
            (1+(_soc_ml_mmu_params.mmu_ing_port_dyn_thd_param*
                general_info->total_num_of_ports)));

    /* C195 =($C$194+$C$168+C156)*$C$79 */
    input_port_threshold->ingress_total_shared_cells_use_for_all_port =
        (input_port_threshold->ingress_burst_cells_size_for_all_ports +
         input_port_threshold->min_int_buff_cells_for_a_port +
         input_port_threshold->min_int_buff_cells_per_PG) *
        general_info->total_num_of_ports;

    /* C196 =FLOOR(($C$60*$C$190)/(1+($C$60*$C$79)),1) */
    input_port_threshold->ingress_burst_pkts_size_for_all_port = sal_floor_func(
            _soc_ml_mmu_params.mmu_ing_port_dyn_thd_param *
            input_port_threshold->total_shared_EQEs_buff,
            (1+(_soc_ml_mmu_params.mmu_ing_port_dyn_thd_param*
                general_info->total_num_of_ports)));

    /* C197 =($C$196+$C$172+C160)*$C$79 */
    input_port_threshold->ingress_total_shared_pkts_use_for_all_port =
        (input_port_threshold->ingress_burst_pkts_size_for_all_port+
         input_port_threshold->min_EQEs_pkts_for_a_port +
         input_port_threshold->min_EQEs_pkt_per_PG) *
        general_info->total_num_of_ports;

    /* C198 = =$C$195+C150 */
    input_port_threshold->ingress_total_shared_hdrm_cells_use_for_all_port =
        input_port_threshold->ingress_total_shared_cells_use_for_all_port +
        input_port_threshold->total_hdrm_int_buff_cells;

    /* C199 = =$C$197+C154 */
    input_port_threshold->ingress_total_shared_hdrm_pkts_use_for_all_port =
        input_port_threshold->ingress_total_shared_pkts_use_for_all_port +
        input_port_threshold->total_hdrm_EQEs_pkts;

    /* C222 =C98-$C$213-C218 */
    output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff =
        general_info->int_buff_pool -
        output_port_threshold->
        min_grntd_tot_res_queue_cells_int_buff-
        output_port_threshold->
        min_grntd_tot_res_RE_WQs_queue_cells_for_int_buff;


    /* C224 =$C$49*1024-$C$215-1 */
    output_port_threshold->min_grntd_tot_shr_queue_cells_EQEs =
        (_soc_ml_mmu_params.mmu_egress_queue_entries*1024) -
        output_port_threshold->min_grntd_tot_res_queue_cells_EQEs - 1;

    /* C225 =$C$99-$C$216 */
    output_port_threshold->min_grntd_tot_shr_EMA_queue_cells =
        general_info->ema_pool -
        output_port_threshold->min_grntd_tot_res_EMA_queue_cells;

    /* C226 =$C$101-$C$217 */
    output_port_threshold->min_grntd_tot_shr_RE_WQs_cells =
        general_info->repl_engine_work_queue_entries -
        output_port_threshold->min_grntd_tot_res_RE_WQs_cells ;

    /* C227 =C222 */
    output_port_threshold->min_grntd_tot_shr_RE_WQs_queue_cells_for_int_buff =
        output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff ;


    /* C229 =C50-C220 */
    output_port_threshold->min_grntd_tot_shr_EP_redirect_queue_entry_cells =
        _soc_ml_mmu_params.mmu_ep_redirect_queue_entries -
        output_port_threshold->min_grntd_tot_res_EP_redirect_queue_entry_cells;

    /* C230 = 2 */
    output_port_threshold->egress_queue_dynamic_threshold_parameter = 2;

    /* C231 =FLOOR( ($C$62*$C$222)/(1+($C$62*1)), 1) */
    output_port_threshold->egress_burst_cells_size_for_one_queue = sal_floor_func(
            (_soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf *
             output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff),
            (1 + (_soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf*1)));

    /* C232 =FLOOR( ($C$62*$C$224)/(1+($C$62*1)), 1) */
    output_port_threshold->egress_burst_pkts_size_for_one_queue = sal_floor_func(
            (_soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf *
             output_port_threshold->min_grntd_tot_shr_queue_cells_EQEs),
            (1 + (_soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf * 1)));

    /* C233 = =FLOOR( ($C$62*$C$222)/(1+($C$62*$C$79)), 1)*/
    output_port_threshold->egress_burst_cells_size_for_all_ports = sal_floor_func(
            (_soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf *
             output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff),
            (1 + (_soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf *
                  general_info->total_num_of_ports)));

    /* C234 =FLOOR( ($C$62*$C$224)/(1+($C$62*$C$79)), 1) */
    output_port_threshold->egress_burst_pkts_size_for_all_ports = sal_floor_func(
            (_soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf *
             output_port_threshold->min_grntd_tot_shr_queue_cells_EQEs),
            (1 + (_soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf *
                  general_info->total_num_of_ports)));

    /* C235 =FLOOR( ($C$62*$C$222)/(1+($C$62*C93)), 1) */
    output_port_threshold->egress_burst_cells_size_for_all_queues = sal_floor_func(
            (_soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf *
             output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff),
            (1 + (_soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf *
                  general_info->total_egr_base_queues_in_device)));

    /* C236 =FLOOR( ($C$62*$C$224)/(1+($C$62*C93)), 1) */
    output_port_threshold->egress_burst_pkts_size_for_all_queues = sal_floor_func(
            (_soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf *
             output_port_threshold->min_grntd_tot_shr_queue_cells_EQEs),
            (1 + (_soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf *
                  general_info->total_egr_base_queues_in_device)));

    /* C237 =C93*$C$235 */
    output_port_threshold->egress_total_use_in_cells_for_all_queues =
        general_info->total_egr_base_queues_in_device *
        output_port_threshold->egress_burst_cells_size_for_all_queues;

    /* C238 =C93*$C$236 */
    output_port_threshold->egress_total_use_in_pkts_for_all_queues =
        general_info->total_egr_base_queues_in_device *
        output_port_threshold->egress_burst_pkts_size_for_all_queues;

    /* C239 =$C$222-C237*/
    output_port_threshold->egress_remaining_cells_for_all_queues =
        output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff -
        output_port_threshold->egress_total_use_in_cells_for_all_queues;

    /* C240 =$C$224-C238*/
    output_port_threshold->egress_remaining_pkts_for_all_queues =
        output_port_threshold->min_grntd_tot_shr_queue_cells_EQEs -
        output_port_threshold->egress_total_use_in_pkts_for_all_queues;


    /*======================================================================== */
    /*                        Device Wide Registers                            */
    /* ======================================================================= */
    /* C244: CFAPICONFIG.CFAPIPOOLSIZE =C42+22 */
    bcm5346x_reg_get(unit, R_CFAPICONFIG, &rval);
    /* CFAPIPOOLSIZE<13:0> */
    rval &= ~0x3FFF;
    rval |= (_soc_ml_mmu_params.mmu_int_buf_size + 22);
    bcm5346x_reg_set(unit, R_CFAPICONFIG, rval);

    /* C245: CFAPIFULLSETPOINT.CFAPIFULLSETPOINT = =C96 */
    bcm5346x_reg_get(unit, R_CFAPIFULLSETPOINT, &rval);
    /* CFAPIFULLSETPOINT<13:0> */
    rval &= ~0x3FFF;
    rval |= general_info->int_cell_buff_size_after_limitation;
    bcm5346x_reg_set(unit, R_CFAPIFULLSETPOINT, rval);

    /* C246: CFAPIFULLRESETPOINT.CFAPIFULLRESETPOINT =C244-128 */
    bcm5346x_reg_get(unit, R_CFAPIFULLRESETPOINT, &rval);
    /* CFAPIFULLRESETPOINT<13:0> */
    rval &= ~0x3FFF;
    rval |= (_soc_ml_mmu_params.mmu_int_buf_size + 22 - 128);
    bcm5346x_reg_set(unit, R_CFAPIFULLRESETPOINT, rval);

    /* C250: MMU_ENQ_FAPCONFIG_0.FAPPOOLSIZE =C52*1024-1*/
    bcm5346x_reg_get(unit, R_MMU_ENQ_FAPCONFIG_0, &rval);
    /* FAPPOOLSIZE<11:0> */
    rval &= ~0xFFF;
    /* rval |= _soc_ml_mmu_params.mmu_repl_engine_work_queue_entries*1024 - 1; */
    rval |= 1344;
    bcm5346x_reg_set(unit, R_MMU_ENQ_FAPCONFIG_0, rval);
    /* temp_val = _soc_ml_mmu_params.mmu_repl_engine_work_queue_entries*1024 - 1; */
     temp_val = 1344;

    /* C251 MMU_ENQ_FAPFULLSETPOINT_0.FAPFULLSETPOINT =C250-64 */
    bcm5346x_reg_get(unit, R_MMU_ENQ_FAPFULLSETPOINT_0, &rval);
    /* FAPFULLSETPOINT<11:0> */
    rval &= ~0xFFF;
    rval |= (temp_val - 64);
    bcm5346x_reg_set(unit, R_MMU_ENQ_FAPFULLSETPOINT_0, rval);
    
    /* C252 MMU_ENQ_FAPFULLRESETPOINT_0.FAPFULLRESETPOINT =C250-128 */
    bcm5346x_reg_get(unit, R_MMU_ENQ_FAPFULLRESETPOINT_0, &rval);
    /* FAPFULLRESETPOINT<11:0> */
    rval &= ~0xFFF;
    rval |= (temp_val - 128);
    bcm5346x_reg_set(unit, R_MMU_ENQ_FAPFULLRESETPOINT_0, rval);

    /* C253: QSTRUCT_FAPCONFIG.FAPPOOLSIZE =(C49*1024+1024*3)/4 */
    temp_val = (((_soc_ml_mmu_params.mmu_egress_queue_entries * 1024 ) + (1024 *3))/4); 
    for (idx =0 ; idx < 1; idx++) {
        bcm5346x_reg_get(unit, R_QSTRUCT_FAPCONFIG(idx), &rval);
        /* FAPPOOLSIZE<11:0> */
        rval &= ~0xFFF;
        rval |= temp_val;
        bcm5346x_reg_set(unit, R_QSTRUCT_FAPCONFIG(idx), rval);
    }

    /* C254 QSTRUCT_FAPFULLSETPOINT.FAPFULLSETPOINT  = C253 - 8 */
    temp_val = temp_val - 8;
    for (idx =0 ; idx < 1; idx++) {
        bcm5346x_reg_get(unit, R_QSTRUCT_FAPFULLSETPOINT(idx), &rval);
        /* FAPFULLSETPOINT<11:0> */
        rval &= ~0xFFF;
        rval |= temp_val;
        bcm5346x_reg_set(unit, R_QSTRUCT_FAPFULLSETPOINT(idx), rval);
    }

    /* C255 QSTRUCT_FAPFULLRESETPOINT.FAPFULLRESETPOINT C254 - 16*/
    temp_val = temp_val -16;
    for (idx =0 ; idx < 1; idx++) {
        bcm5346x_reg_get(unit, R_QSTRUCT_FAPFULLRESETPOINT(idx), &rval);
        /* FAPFULLRESETPOINT<11:0> */
        rval &= ~0xFFF;
        rval |= temp_val;
        bcm5346x_reg_set(unit, R_QSTRUCT_FAPFULLRESETPOINT(idx), rval);
    }

    /* C256: QSTRUCT_FAPOTPCONFIG.MAXFULLSET = C253*/
    temp_val = (((_soc_ml_mmu_params.mmu_egress_queue_entries * 1024 ) + (1024 *3))/4);
    for (idx =0 ; idx < 1; idx++) {
        bcm5346x_reg_get(unit, R_QSTRUCT_FAPOTPCONFIG(idx), &rval);
        /* MAXFULLSET<11:0> */
        rval &= ~0xFFF;
        rval |= temp_val;
        /* Its failing on Emulator */       
        /* bcm5346x_reg_set(unit, R_QSTRUCT_FAPOTPCONFIG(idx), rval); */
    }

    /* C261: THDO_MISCCONFIG.UCMC_SEPARATION = 0 */
    bcm5346x_reg_get(unit, R_THDO_MISCCONFIG, &rval);
    /*UCMC_SEPARATION<3> */
    rval &= ~(0x1 << 3);
    bcm5346x_reg_set(unit, R_THDO_MISCCONFIG, rval);

    /* C262: WRED_MISCCONFIG.UCMC_SEPARATION = 0 */
    bcm5346x_reg_get(unit, R_WRED_MISCCONFIG, &rval);
    /* UCMC_SEPARATION<16> */
    rval &= ~(0x1 << 16);
    bcm5346x_reg_set(unit, R_WRED_MISCCONFIG, rval);

    /* C266: COLOR_AWARE.ENABLE = 0 */
    bcm5346x_reg_get(unit, R_COLOR_AWARE, &rval);
    /* ENABLE<3:0> */
    rval &= ~0xF;
    bcm5346x_reg_set(unit, R_COLOR_AWARE, rval);

    /* C268: GLOBAL_HDRM_LIMIT.GLOBAL_HDRM_LIMIT = =$C$110 */
    bcm5346x_reg_get(unit, R_GLOBAL_HDRM_LIMIT, &rval);
    /* GLOBAL_HDRM_LIMIT<15:0> */
    rval &= ~0xFFFF;
    rval |= input_port_threshold->global_hdrm_cells_for_int_buff_ingress_pool;
    bcm5346x_reg_set(unit, R_GLOBAL_HDRM_LIMIT, rval);

    /* C269: BUFFER_CELL_LIMIT_SP.LIMIT =  =$C$186 */
    bcm5346x_reg_get(unit, R_BUFFER_CELL_LIMIT_SP(service_pool), &rval);
    /* LIMIT<15:0> */
    rval &= ~0xFFFF;
    rval |= input_port_threshold->total_shared_ing_buff_pool;
    bcm5346x_reg_set(unit, R_BUFFER_CELL_LIMIT_SP(service_pool), rval);

    /* C270: CELL_RESET_LIMIT_OFFSET_SP.OFFSET =CEILING(C82/4, 1)*C76 */
    bcm5346x_reg_get(unit, R_CELL_RESET_LIMIT_OFFSET_SP(service_pool), &rval);
    temp_val = sal_ceil_func(general_info->total_num_of_ports_excl_lpbk_olp_cpu,4) *
        general_info->ether_mtu_cells_for_int_buff;
    /* OFFSET<15:0> */
    rval &= ~0xFFFF;
    rval |= temp_val;
	
    bcm5346x_reg_set(unit, R_CELL_RESET_LIMIT_OFFSET_SP(service_pool), rval);

    /* C280: THDIRQE_GLOBAL_HDRM_LIMIT.GLOBAL_HDRM_LIMIT = C113 */
    bcm5346x_reg_get(unit, R_THDIRQE_GLOBAL_HDRM_LIMIT, &rval);
    /* GLOBAL_HDRM_LIMIT<15:0> */
    rval &= ~0xFFFF;
    rval |= input_port_threshold->global_hdrm_cells_for_RE_WQEs;
    bcm5346x_reg_set(unit, R_THDIRQE_GLOBAL_HDRM_LIMIT, rval);

    /* C281:  THDIRQE_BUFFER_CELL_LIMIT_SP.LIMIT =$C$189  */
    bcm5346x_reg_get(unit, R_THDIRQE_BUFFER_CELL_LIMIT_SP(service_pool), &rval);
    /* LIMIT<15:0> */
    rval &= ~0xFFFF;
    rval |= input_port_threshold->total_shared_RE_WQEs_buff;
    bcm5346x_reg_set(unit, R_THDIRQE_BUFFER_CELL_LIMIT_SP(service_pool), rval);

    /* C282: = THDIRQE_CELL_RESET_LIMIT_OFFSET_SP.OFFSET = =CEILING(C82/4, 1) */
    bcm5346x_reg_get(unit, R_THDIRQE_CELL_RESET_LIMIT_OFFSET_SP(service_pool), &rval);
    /* OFFSET<15:0> */
    rval &= ~0xFFFF;
    rval |= sal_ceil_func(general_info->total_num_of_ports_excl_lpbk_olp_cpu , 4);
    bcm5346x_reg_set(unit, R_THDIRQE_CELL_RESET_LIMIT_OFFSET_SP(service_pool), rval);

    /* C284: THDIQEN_GLOBAL_HDRM_LIMIT.GLOBAL_HDRM_LIMIT = C114 */
    bcm5346x_reg_get(unit, R_THDIQEN_GLOBAL_HDRM_LIMIT, &rval);
    /* GLOBAL_HDRM_LIMIT<15:0> */
    rval &= ~0xFFFF;
    rval |= input_port_threshold->global_hdrm_cells_for_EQEs;

    bcm5346x_reg_set(unit, R_THDIQEN_GLOBAL_HDRM_LIMIT, rval);

    /* C285:  THDIQEN_BUFFER_CELL_LIMIT_SP.LIMIT =$C$190  */
    bcm5346x_reg_get(unit, R_THDIQEN_BUFFER_CELL_LIMIT_SP(service_pool), &rval);
    /* LIMIT<15:0> */
    rval &= ~0xFFFF;
    rval |= input_port_threshold->total_shared_EQEs_buff;

    bcm5346x_reg_set(unit, R_THDIQEN_BUFFER_CELL_LIMIT_SP(service_pool), rval);

    /* C286: THDIQEN_CELL_RESET_LIMIT_OFFSET_SP.OFFSET= 
       =CEILING(C82/4, 1)*C51*/
    bcm5346x_reg_get(unit, R_THDIQEN_CELL_RESET_LIMIT_OFFSET_SP(service_pool), &rval);
    /* OFFSET<15:0> */
    temp_val = sal_ceil_func(general_info->total_num_of_ports_excl_lpbk_olp_cpu, 4)  * 
        _soc_ml_mmu_params.mmu_exp_num_of_repl_per_pkt;
    rval &= ~0xFFFF;
    rval |= temp_val;
    bcm5346x_reg_set(unit, R_THDIQEN_CELL_RESET_LIMIT_OFFSET_SP(service_pool), rval);

    /* C289: OP_BUFFER_SHARED_LIMIT_CELLI.OP_BUFFER_SHARED_LIMIT_CELLI = C222 */
    temp_val = output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff;

    bcm5346x_reg_get(unit, R_OP_BUFFER_SHARED_LIMIT_CELLI, &rval);
    /* OP_BUFFER_SHARED_LIMIT_CELLI<13:0> */
    rval &= ~0x3FFF;
    rval |= temp_val;
    bcm5346x_reg_set(unit, R_OP_BUFFER_SHARED_LIMIT_CELLI, rval);

    /* C290 OP_BUFFER_SHARED_LIMIT_RESUME_CELLI.OP_BUFFER_SHARED_LIMIT_RESUME_CELLI
       =C289-CEILING(C82/4, 1)*C76 */        
    temp_val = temp_val - (sal_ceil_func(general_info->total_num_of_ports_excl_lpbk_olp_cpu,4) 
            * general_info->ether_mtu_cells_for_int_buff);

    bcm5346x_reg_get(unit, R_OP_BUFFER_SHARED_LIMIT_RESUME_CELLI, &rval);
    /* OP_BUFFER_SHARED_LIMIT_RESUME_CELLI<13:0> */
    rval &= ~0x3FFF;
    rval |= temp_val;
    bcm5346x_reg_set(unit, R_OP_BUFFER_SHARED_LIMIT_RESUME_CELLI, rval);
    
    /* C292 OP_BUFFER_LIMIT_RESUME_YELLOW_CELLI.OP_BUFFER_LIMIT_RESUME_YELLOW_CELLI =
       =CEILING(C290/8, 1) */
    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_RESUME_YELLOW_CELLI, &rval);
    /* OP_BUFFER_LIMIT_RESUME_YELLOW_CELLI<10:0> */
    rval &= ~0x7FF;
    rval |= sal_ceil_func(temp_val,8);
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_RESUME_YELLOW_CELLI, rval);

    /* C294 OP_BUFFER_LIMIT_RESUME_RED_CELLI.OP_BUFFER_LIMIT_RESUME_RED_CELLI
       =CEILING(C290/8, 1) */
    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_RESUME_RED_CELLI, &rval);
    /* OP_BUFFER_LIMIT_RESUME_RED_CELLI<10:0> */
    rval &= ~0x7FF;
    rval |= sal_ceil_func(temp_val,8);
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_RESUME_RED_CELLI, rval);

    /* C291: OP_BUFFER_LIMIT_YELLOW_CELLI.OP_BUFFER_LIMIT_YELLOW_CELLI =
       =CEILING(C289/8, 1) */
    temp_val = output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff;

    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_YELLOW_CELLI, &rval);
    /* OP_BUFFER_LIMIT_YELLOW_CELLI<10:0> */
    rval &= ~0x7FF;
    rval |= sal_ceil_func(temp_val,8);
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_YELLOW_CELLI, rval);

    /* C293 OP_BUFFER_LIMIT_RED_CELLI.OP_BUFFER_LIMIT_RED_CELLI 
       =CEILING(C289/8, 1) */
    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_RED_CELLI, &rval);
    /* OP_BUFFER_LIMIT_RED_CELLI<10:0> */
    rval &= ~0x7FF;
    rval |= sal_ceil_func( temp_val,8);
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_RED_CELLI, rval);

    /* C310: 
       OP_BUFFER_SHARED_LIMIT_THDORQEQ.OP_BUFFER_SHARED_LIMIT_CELLE = C226 */
    temp_val = output_port_threshold->min_grntd_tot_shr_RE_WQs_cells; 

    bcm5346x_reg_get(unit, R_OP_BUFFER_SHARED_LIMIT_THDORQEQ, &rval);
    /* OP_BUFFER_SHARED_LIMIT<11:0> */
    rval &= ~0xFFF;
    rval |= temp_val;
    bcm5346x_reg_set(unit, R_OP_BUFFER_SHARED_LIMIT_THDORQEQ, rval);

    /* C312: OP_BUFFER_LIMIT_YELLOW_THDORQEQ.OP_BUFFER_LIMIT_YELLOW =
       =CEILING(C310/8, 1) */
    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_YELLOW_THDORQEQ, &rval);
    /* OP_BUFFER_LIMIT_YELLOW<8:0> */
    rval &= ~0x1FF;
    rval |= sal_ceil_func(temp_val,8);
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_YELLOW_THDORQEQ, rval);

    /* C314: OP_BUFFER_LIMIT_RED_THDORQEQ.OP_BUFFER_LIMIT_RED =
       =CEILING(C310/8, 1) */
    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_RED_THDORQEQ, &rval);
    /* OP_BUFFER_LIMIT_RED<8:0> */
    rval &= ~0x1FF;
    rval |= sal_ceil_func(temp_val,8);
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_RED_THDORQEQ, rval);

    /* C311: OP_BUFFER_SHARED_LIMIT_RESUME_THDORQEQ.
       OP_BUFFER_SHARED_LIMIT_RESUME =C226-CEILING(C82/4, 1) */
    temp_val -=  sal_ceil_func(general_info->total_num_of_ports_excl_lpbk_olp_cpu,4);

    bcm5346x_reg_get(unit, R_OP_BUFFER_SHARED_LIMIT_RESUME_THDORQEQ, &rval);
    /* OP_BUFFER_SHARED_LIMIT_RESUME<11:0> */
    rval &= ~0xFFF;
    rval |= (temp_val);
    bcm5346x_reg_set(unit, R_OP_BUFFER_SHARED_LIMIT_RESUME_THDORQEQ, rval);

    /* C313: OP_BUFFER_LIMIT_RESUME_YELLOW_THDORQEQ.
       OP_BUFFER_LIMIT_RESUME_YELLOW = C312-1 */
    temp_val = sal_ceil_func(output_port_threshold->min_grntd_tot_shr_RE_WQs_cells , 8); 

    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_RESUME_YELLOW_THDORQEQ, &rval);
    /* OP_BUFFER_LIMIT_RESUME_YELLOW<8:0> */
    rval &= ~0x1FF;
    rval |= (temp_val-1);
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_RESUME_YELLOW_THDORQEQ, rval);

    /* C315: OP_BUFFER_LIMIT_RESUME_RED_THDORQEQ.
       OP_BUFFER_LIMIT_RESUME_RED==C314 -1 */
    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_RESUME_RED_THDORQEQ, &rval);
    /* OP_BUFFER_LIMIT_RESUME_RED<8:0> */
    rval &= ~0x1FF;
    rval |= (temp_val-1);
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_RESUME_RED_THDORQEQ, rval);

    /* C317: OP_BUFFER_SHARED_LIMIT_QENTRY.OP_BUFFER_SHARED_LIMIT_QENTRY =C221*/
    bcm5346x_reg_get(unit, R_OP_BUFFER_SHARED_LIMIT_QENTRY, &rval);
    /* OP_BUFFER_SHARED_LIMIT_QENTRY<15:0> */
    rval &= ~0xFFFF;
    rval |= output_port_threshold->min_grntd_tot_shr_queue_cells_EQEs;
    bcm5346x_reg_set(unit, R_OP_BUFFER_SHARED_LIMIT_QENTRY, rval);

    temp_val = output_port_threshold->min_grntd_tot_shr_queue_cells_EQEs;

    /* C319: OP_BUFFER_LIMIT_YELLOW_QENTRY.OP_BUFFER_LIMIT_YELLOW_QENTRY
       = CEILING(C317/8, 1)  */
    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_YELLOW_QENTRY, &rval);
    /* OP_BUFFER_LIMIT_YELLOW_QENTRY<12:0> */
    rval &= ~0x1FFF;
    rval |= sal_ceil_func(temp_val,8); 
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_YELLOW_QENTRY, rval);

    /* C321: OP_BUFFER_LIMIT_RED_QENTRY.OP_BUFFER_LIMIT_RED_QENTRY
       = CEILING(C317/8, 1)  */
    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_RED_QENTRY, &rval);
    /* OP_BUFFER_LIMIT_RED_QENTRY<12:0> */
    rval &= ~0x1FFF;
    rval |= sal_ceil_func(temp_val,8);
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_RED_QENTRY, rval);

    /* C318: OP_BUFFER_SHARED_LIMIT_RESUME_QENTRY.OP_BUFFER_SHARED_LIMIT_RESUME_QENTRY
       =C224-CEILING(C82/4, 1)*C51 */
    temp_val -= (sal_ceil_func(general_info->total_num_of_ports_excl_lpbk_olp_cpu,4) * 
            _soc_ml_mmu_params.mmu_exp_num_of_repl_per_pkt);
    bcm5346x_reg_get(unit, R_OP_BUFFER_SHARED_LIMIT_RESUME_QENTRY, &rval);
    /* OP_BUFFER_SHARED_LIMIT_RESUME_QENTRY<15:0> */
    rval &= ~0xFFFF;
    rval |= temp_val;
    bcm5346x_reg_set(unit, R_OP_BUFFER_SHARED_LIMIT_RESUME_QENTRY, rval);

    /* C320 OP_BUFFER_LIMIT_RESUME_YELLOW_QENTRY.OP_BUFFER_LIMIT_RESUME_YELLOW_QENTRY
       =CEILING(C318/8, 1) */
    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_RESUME_YELLOW_QENTRY, &rval);
    /* OP_BUFFER_LIMIT_RESUME_YELLOW_QENTRY<12:0> */
    rval &= ~0x1FFF;
    rval |= sal_ceil_func(temp_val,8);
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_RESUME_YELLOW_QENTRY, rval);

    /* C322 OP_BUFFER_LIMIT_RESUME_RED_QENTRY.OP_BUFFER_LIMIT_RESUME_RED_QENTRY
       =CEILING(C318/8, 1) */
    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_RESUME_RED_QENTRY, &rval);
    /* OP_BUFFER_LIMIT_RESUME_RED_QENTRY<12:0> */
    rval &= ~0x1FFF;
    rval |= sal_ceil_func(temp_val,8);
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_RESUME_RED_QENTRY, rval);

    /* C324 : OP_BUFFER_SHARED_LIMIT_THDORDEQ.OP_BUFFER_SHARED_LIMIT = C229 */
    bcm5346x_reg_get(unit, R_OP_BUFFER_SHARED_LIMIT_THDORDEQ, &rval);
    /* OP_BUFFER_SHARED_LIMIT<8:0> */
    rval &= ~0x1FF;
    rval |= output_port_threshold->min_grntd_tot_shr_EP_redirect_queue_entry_cells;
    bcm5346x_reg_set(unit, R_OP_BUFFER_SHARED_LIMIT_THDORDEQ, rval);
    temp_val= output_port_threshold->
        min_grntd_tot_shr_EP_redirect_queue_entry_cells;

    /*C325: OP_BUFFER_SHARED_LIMIT_TRESUME_THDORDEQr.
      OP_BUFFER_SHARED_LIMIT_RESUMEf = =C229-4 */
    bcm5346x_reg_get(unit, R_OP_BUFFER_SHARED_LIMIT_RESUME_THDORDEQ, &rval);
    /* OP_BUFFER_SHARED_LIMIT_RESUME<8:0> */
    rval &= ~0x1FF;
    rval |= (temp_val - 4);
    bcm5346x_reg_set(unit, R_OP_BUFFER_SHARED_LIMIT_RESUME_THDORDEQ, rval);

    /* C326: OP_BUFFER_LIMIT_YELLOW_THDORDEQ.OP_BUFFER_LIMIT_YELLOW
       = CEILING(C324/8, 1) */
    temp_val =  sal_ceil_func(temp_val , 8);
    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_YELLOW_THDORDEQ, &rval);
    /* OP_BUFFER_LIMIT_YELLOW<5:0> */
    rval &= ~0x3F;
    rval |= temp_val;
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_YELLOW_THDORDEQ, rval);

    /* C328: OP_BUFFER_LIMIT_RED_THDORDEQ.OP_BUFFER_LIMIT_RED
       = CEILING(C324/8, 1) */
    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_RED_THDORDEQ, &rval);
    /* OP_BUFFER_LIMIT_RED<5:0> */
    rval &= ~0x3F;
    rval |= temp_val;
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_RED_THDORDEQ, rval);

    /* C327: OP_BUFFER_LIMIT_RESUME_YELLOW_THDORDEQ.
       OP_BUFFER_LIMIT_RESUME_YELLOW = =C326-1 */
    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_RESUME_YELLOW_THDORDEQ, &rval);
    /* OP_BUFFER_LIMIT_RESUME_YELLOW<5:0> */
    rval &= ~0x3F;
    rval |= (temp_val-1);
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_RESUME_YELLOW_THDORDEQ, rval);

    /* C329: OP_BUFFER_LIMIT_RESUME_RED_THDORDEQ.
       OP_BUFFER_LIMIT_RESUME_RED = =C328-1 */
    bcm5346x_reg_get(unit, R_OP_BUFFER_LIMIT_RESUME_RED_THDORDEQ, &rval);
    /* OP_BUFFER_LIMIT_RESUME_RED<5:0> */
    rval &= ~0x3F;
    rval |= (temp_val-1);
    bcm5346x_reg_set(unit, R_OP_BUFFER_LIMIT_RESUME_RED_THDORDEQ, rval);

	/* remove loop back port? */
    for (port = ML_CMIC ; port <= (BCM5346X_LPORT_MAX + 1); port++) {
        /* THDI_PORT_SP_CONFIG
         * DATAWIDTH<48>
         * PORT_SP_RESUME_LIMIT<47:32>
         * PORT_SP_MIN_LIMIT<31:16>
         * PORT_SP_MAX_LIMIT<15:0>
         */
        sal_memset(entry2,0,sizeof(uint32) * 2);
        mem_idx= ML_GET_THDI_PORT(port) * ML_MAX_SERVICE_POOLS;
        
        /* C335 : THDI_PORT_SP_CONFIG.PORT_SP_MIN_LIMIT = =$C$168 */
        entry2[0] |= ((input_port_threshold->min_int_buff_cells_for_a_port) << 16);

        /* C336 : THDI_PORT_SP_CONFIG.PORT_SP_MAX_LIMIT = =$C$269 i.e. =$C$186 */
		
        entry2[0] |= input_port_threshold->total_shared_ing_buff_pool;

        /* C337 : PORT_MAX_PKT_SIZE.PORT_MAX_PKT_SIZE = =$C$72 */

		bcm5346x_reg_get(unit, R_PORT_MAX_PKT_SIZE(ML_GET_THDI_PORT(port)), &rval);
		/* PORT_MAX_PKT_SIZE<7:0> */
		rval &= ~0xFF;
		rval |= general_info->max_packet_size_in_cells;
		bcm5346x_reg_set(unit, R_PORT_MAX_PKT_SIZE(ML_GET_THDI_PORT(port)), rval);

        /* C338: THDI_PORT_SP_CONFIG.PORT_SP_RESUME_LIMIT = =C336-2*$C$76 */
        entry2[1] |= ((input_port_threshold->total_shared_ing_buff_pool - 
                (2 *  general_info->ether_mtu_cells_for_int_buff)) << (32-32));
        bcm5346x_mem_set(unit, M_THDI_PORT_SP_CONFIG(mem_idx), entry2, 2);


        /* THDI_PORT_PG_CONFIG
         * DATAWIDTH<88>
         * RSVD<87:85>
         * FDR_MODE_ENABLE<84>
         * SP_SHARED_MAX_ENABLE<83>
         * SP_MIN_PG_ENABLE<82>
         * PG_MIN_LIMIT<81:66>
         * PG_SHARED_DYNAMIC<65>
         * PG_SHARED_LIMIT<64:49>
         * PG_GBL_HDRM_EN<48>
         * PG_HDRM_LIMIT<47:32>
         * PG_RESET_OFFSET<31:16>
         * PG_RESET_FLOOR<15:0>
         */
        sal_memset(entry3,0,sizeof(uint32) * 3);
        mem_idx= ML_GET_THDI_PORT_PG(port);

        /* C339: THDI_PORT_PG_CONFIG.PG_MIN_LIMIT (PG0) = =$C$156 */
        entry3[2] |= ((input_port_threshold->min_int_buff_cells_per_PG) << (66-64));

        /* C340: THDI_PORT_PG_CONFIG.PG_SHARED_LIMIT
           =IF($C$14, $C$61, $C$269)*/
        if (_soc_ml_mmu_params.lossless_mode_d_c) {
            temp_val = _soc_ml_mmu_params.mmu_ing_pg_dyn_thd_param;
        } else {
            temp_val = input_port_threshold->total_shared_ing_buff_pool;
        } 

        entry3[1] |= (temp_val << (49-32));


        /* C341: THDI_PORT_PG_CONFIG.PG_RESET_OFFSET = 2*$C$76 */
        entry3[0] |= ((2 * general_info->ether_mtu_cells_for_int_buff) << 16);

        /* C342: THDI_PORT_PG_CONFIG.PG_RESET_FLOOR = 0 */
        /* Do nothing with entry3 */

        /* C343: THDI_PORT_PG_CONFIG.PG_SHARED_DYNAMIC= =IF($C$14, 1, 0) */
        if (_soc_ml_mmu_params.lossless_mode_d_c) {
            temp_val = 1;
        } else {
            temp_val = 0;
        }

        entry3[2] |= (temp_val << (65-64));

        /* C344: THDI_PORT_PG_CONFIG.PG_HDRM_LIMIT=
           For Ge:$C125 For Hg:$C115 For Cpu:$C145 */
        if (port == ML_CMIC) {
            /* CPU port */
            temp_val = input_port_threshold->hdrm_int_buff_cells_for_cpu_port;
        } else if (SOC_PORT_SPEED_MAX(port) == 10000) {
            /* XE port */
            temp_val = input_port_threshold->hdrm_int_buff_cells_for_10G_PG;
        } else {
            /* GE port */
            temp_val = input_port_threshold->hdrm_int_buff_cells_for_1G_PG;
        }

        entry3[1] |= (temp_val << (32-32));

        /* C345 : THDI_PORT_PG_CONFIG.PG_GBL_HDRM_EN = for lossy=1 else 0 */

         entry3[1] |= (0x1 << (48-32));

        /* C346: THDI_PORT_PG_CONFIG.SP_SHARED_MAX_ENABLE = 
           For Ge:1 For Hg,Cpu: Ge Value only!!! */

        entry3[2] |= (0x1 << (83-64));

        /* C347: THDI_PORT_PG_CONFIG.SP_MIN_PG_ENABLE =
           For Ge:1 For Hg,Cpu: Ge Value only!!! */
        entry3[2] |= (0x1 << (82-64));

        bcm5346x_mem_set(unit, M_THDI_PORT_PG_CONFIG(mem_idx), entry3, 3);
        
        /* THDIRQE_THDI_PORT_SP_CONFIG
         * DATAWIDTH<48>
         * PORT_SP_RESUME_LIMIT<47:32>
         * PORT_SP_MIN_LIMIT<31:16>
         * PORT_SP_MAX_LIMIT<15:0>
         */
        sal_memset(entry2,0,sizeof(uint32) * 2);
        mem_idx= ML_GET_THDI_PORT(port) * ML_MAX_SERVICE_POOLS;
        
        /* C380: THDIRQE_THDI_PORT_SP_CONFIG.PORT_SP_MIN_LIMIT  = $C$171 */
        entry2[0] |= ((input_port_threshold->min_RE_WQEs_pkts_for_a_port) << 16);

        /* C381: THDIRQE_THDI_PORT_SP_CONFIG.PORT_SP_MAX_LIMIT = $C$189 */
        entry2[0] |= input_port_threshold->total_shared_RE_WQEs_buff;

        /* C382: THDIRQE_THDI_PORT_SP_CONFIG.PORT_SP_RESUME_LIMIT =
           =C381-2 */
        entry2[1] |= ((input_port_threshold->total_shared_RE_WQEs_buff - 2) << (32-32));
        
        bcm5346x_mem_set(unit, M_THDIRQE_THDI_PORT_SP_CONFIG(mem_idx), entry2, 2);

        /* THDIRQE_THDI_PORT_PG_CONFIG
         * DATAWIDTH<88>
         * RSVD<87:85>
         * FDR_MODE_ENABLE<84>
         * SP_SHARED_MAX_ENABLE<83>
         * SP_MIN_PG_ENABLE<82>
         * PG_MIN_LIMIT<81:66>
         * PG_SHARED_DYNAMIC<65>
         * PG_SHARED_LIMIT<64:49>
         * PG_GBL_HDRM_EN<48>
         * PG_HDRM_LIMIT<47:32>
         * PG_RESET_OFFSET<31:16>
         * PG_RESET_FLOOR<15:0>
         */
        sal_memset(entry3,0,sizeof(uint32) * 3);        
        mem_idx= ML_GET_THDI_PORT_PG(port);
        
        /* C383: THDIRQE_THDI_PORT_PG_CONFIG.PG_MIN_LIMIT(PG0) = =$C$159 */
        temp_val = input_port_threshold->min_RE_WQEs_pkt_per_PG;
        entry3[2] |= (temp_val << (66-64));

        /* C384: THDIRQE_THDI_PORT_PG_CONFIG.PG_SHARED_LIMIT =
           =IF($C$14, $C$61, $C$189) */
        if (_soc_ml_mmu_params.lossless_mode_d_c) {
            temp_val = _soc_ml_mmu_params.mmu_ing_pg_dyn_thd_param;
        } else {
            temp_val = input_port_threshold->total_shared_RE_WQEs_buff;
        }
        entry3[1] |= (temp_val << (49-32));

        /* C385: THDIRQE_THDI_PORT_PG_CONFIG.PG_RESET_OFFSET = 2 */
        entry3[0] |= (0x2 << 16);

        /* C386: THDIRQE_THDI_PORT_PG_CONFIG.PG_RESET_FLOOR = 0 */
        /* Do nothing with entry3 */

        /* C387: THDIRQE_THDI_PORT_PG_CONFIG.PG_SHARED_DYNAMIC= 
           =IF($C$14, 1, 0) */
        if (_soc_ml_mmu_params.lossless_mode_d_c) {
            temp_val = 1;
        } else {
            temp_val = 0;
        }
        entry3[2] |= (temp_val << (65-64));

        /* C388: THDIRQE_THDI_PORT_PG_CONFIG.PG_HDRM_LIMIT
           For Ge:$C128 For Hg:$C118 For Cpu:$C148 */
        if (port == ML_CMIC) {
            /* CPU port */
            temp_val = input_port_threshold->hdrm_RE_WQEs_pkts_for_cpu_port;
        } else if (SOC_PORT_SPEED_MAX(port) == 10000) {
            /* XE port */
            temp_val= input_port_threshold->hdrm_RE_WQEs_pkts_for_10G_PG;
        } else {
            /* GE port */
            temp_val = input_port_threshold->hdrm_RE_WQEs_pkts_for_1G_PG;
        }
        
        entry3[1] |= (temp_val << (32-32));

        /* C389: THDIRQE_THDI_PORT_PG_CONFIG.PG_GBL_HDRM_EN = 0 */ 
        /* Do nothing with entry3 */

        /* C390: THDIRQE_THDI_PORT_PG_CONFIG.SP_SHARED_MAX_ENABLE =
           For Ge:1 For Hg,Cpu: Ge Value only!!! */
        entry3[2] |= (0x1 << (83-64));

        /* C391: THDIRQE_THDI_PORT_PG_CONFIG.SP_MIN_PG_ENABLE =
           For Ge:1 For Hg,Cpu: Ge Value only!!! */
        entry3[2] |= (0x1 << (82-64));

        bcm5346x_mem_set(unit, M_THDIRQE_THDI_PORT_PG_CONFIG(mem_idx), entry3, 3);

        /* THDIQEN_THDI_PORT_SP_CONFIG
         * DATAWIDTH<48>
         * PORT_SP_RESUME_LIMIT<47:32>
         * PORT_SP_MIN_LIMIT<31:16>
         * PORT_SP_MAX_LIMIT<15:0>
         */
        sal_memset(entry2,0,sizeof(uint32) * 2);
        mem_idx= ML_GET_THDI_PORT(port) * ML_MAX_SERVICE_POOLS;

        /* C394: THDIQEN_THDI_PORT_SP_CONFIG.PORT_SP_MIN_LIMIT  = $C$172 */
        entry2[0] |= ((input_port_threshold->min_EQEs_pkts_for_a_port) << 16);

        /* C395: THDIQEN_THDI_PORT_SP_CONFIG.PORT_SP_MAX_LIMIT = $C$190 */
        entry2[0] |= input_port_threshold->total_shared_EQEs_buff;

        /* C396: THDIQEN_THDI_PORT_SP_CONFIG.PORT_SP_RESUME_LIMIT =
           ==C395-2*C51 */
        entry2[1] |= ((input_port_threshold->total_shared_EQEs_buff - 
                (2 * _soc_ml_mmu_params.mmu_exp_num_of_repl_per_pkt)) << (32-32));
        bcm5346x_mem_set(unit, M_THDIQEN_THDI_PORT_SP_CONFIG(mem_idx), entry2, 2);

        /* THDIQEN_THDI_PORT_PG_CONFIG
         * DATAWIDTH<88>
         * RSVD<87:85>
         * FDR_MODE_ENABLE<84>
         * SP_SHARED_MAX_ENABLE<83>
         * SP_MIN_PG_ENABLE<82>
         * PG_MIN_LIMIT<81:66>
         * PG_SHARED_DYNAMIC<65>
         * PG_SHARED_LIMIT<64:49>
         * PG_GBL_HDRM_EN<48>
         * PG_HDRM_LIMIT<47:32>
         * PG_RESET_OFFSET<31:16>
         * PG_RESET_FLOOR<15:0>
         */
		
        sal_memset(entry3,0,sizeof(uint32) * 3);
        mem_idx= ML_GET_THDI_PORT_PG(port);

        /* C397: THDIQEN_THDI_PORT_PG_CONFIG.PG_MIN_LIMIT(PG0) = 
           =$C$160 */
        entry3[2] |= (input_port_threshold->min_EQEs_pkt_per_PG << (66-64));

        /* C398: THDIQEN_THDI_PORT_PG_CONFIG.PG_SHARED_LIMIT =
           =IF($C$14, $C$61, $C$190) */
        if (_soc_ml_mmu_params.lossless_mode_d_c) {
            temp_val = _soc_ml_mmu_params.mmu_ing_pg_dyn_thd_param;
        } else {
            temp_val = input_port_threshold->total_shared_EQEs_buff;
        }
        entry3[1] |= (temp_val << (49-32));

        /* C399: THDIQEN_THDI_PORT_PG_CONFIG.PG_RESET_OFFSET = =2*C51 */
        entry3[0] |= ((2 * _soc_ml_mmu_params.mmu_exp_num_of_repl_per_pkt) << 16);

        /* C400: THDIQEN_THDI_PORT_PG_CONFIG.PG_RESET_FLOOR = 0 */
        /* Do nothing with entry3 */

        /* C401: THDIQEN_THDI_PORT_PG_CONFIG.PG_SHARED_DYNAMIC= 0 */
        if (_soc_ml_mmu_params.lossless_mode_d_c) {
            temp_val = 1;
        } else {
            temp_val = 0;
        }
        entry3[2] |= (temp_val << (65-64));

        /* C402: THDIQEN_THDI_PORT_PG_CONFIG.PG_HDRM_LIMIT
           For Ge:$C129 For Hg:$C119 For Cpu:$C149 */
        if (port == ML_CMIC) {
            /* CPU port */
            temp_val = input_port_threshold->hdrm_EQEs_pkts_for_cpu_port;
        } else if (SOC_PORT_SPEED_MAX(port) == 10000) {
            /* XE port */
            temp_val= input_port_threshold->hdrm_EQEs_pkts_for_10G_PG;
        } else {
            /* GE port */
            temp_val = input_port_threshold->hdrm_EQEs_pkts_for_1G_PG;
        }
        
        entry3[1] |= (temp_val << (32-32));

        /* C403: THDIQEN_THDI_PORT_PG_CONFIG.PG_GBL_HDRM_EN =0 */
        /* Do nothing with entry3 */

        /* C404: THDIQEN_THDI_PORT_PG_CONFIG.SP_SHARED_MAX_ENABLE =
           For Ge:1 For Hg,Cpu: Ge Value only!!! */
        entry3[2] |= (0x1 << (83-64));

        /* C405: THDIQEN_THDI_PORT_PG_CONFIG.SP_MIN_PG_ENABLE =
           For Ge:1 For Hg,Cpu: Ge Value only!!! */
        entry3[2] |= (0x1 << (82-64));

        bcm5346x_mem_set(unit, M_THDIQEN_THDI_PORT_PG_CONFIG(mem_idx), entry3, 3);


        op_node = ( SOC_PORT_COSQ_BASE(port) / 8);
        for (op_node_offset = 0; 
                op_node_offset < sal_ceil_func(SOC_PORT_NUM_UC_COSQ(port),8); 
                op_node_offset++) {

            /* MMU_THDO_OPNCONFIG_CELL
             * PID<49:46>
             * LIMIT_YELLOW_CELL<45:33>
             * PORT_LIMIT_ENABLE_CELL<32>
             * OPN_SHARED_RESET_VALUE_CELL<31:16>
             * OPN_SHARED_LIMIT_CELL<15:0>
             */

            sal_memset(entry2,0,sizeof(uint32) * 2);
            bcm5346x_mem_get(unit, M_MMU_THDO_OPNCONFIG_CELL(op_node + op_node_offset), entry2, 2);

            /* C409:THDO_OPNCONFIG_CELL.OPN_SHARED_LIMIT_CELL = =$C$222 */
            entry2[0] &= ~0xFFFF;
            entry2[0] |= output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff;

            /* C410:THDO_OPNCONFIG_CELL.OP_SHARED_RESET_VALUE_CELL
               = =C409-2*$C$76 */
            temp_val = output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff -
                (2 * general_info->ether_mtu_cells_for_int_buff);
            entry2[0] &= ~(0xFFFF << 16);
            entry2[0] |= (temp_val << 16);

            /* C411:THDO_OPNCONFIG_CELL.PORT_LIMIT_ENABLE_CELL
               = 0 */
            entry2[1] &= ~(0x1 << (32-32));

            /* THDO_OPNCONFIG_CELL.PID = port*/
            entry2[1] &= ~(0xF << (46-32));
            entry2[1] |= (port << (46-32));
			
            bcm5346x_mem_set(unit, M_MMU_THDO_OPNCONFIG_CELL(op_node + op_node_offset), entry2, 2);
        }

        for (queue = SOC_PORT_UC_COSQ_BASE(port);
                queue < SOC_PORT_UC_COSQ_BASE(port) + 
                SOC_PORT_NUM_UC_COSQ(port);
                queue++) {
            /* MMU_THDO_QCONFIG_CELL
             * LIMIT_YELLOW_CELL<49:37>
             * Q_COLOR_LIMIT_DYNAMIC_CELL<36>
             * Q_COLOR_ENABLE_CELL<35>
             * Q_LIMIT_DYNAMIC_CELL<33>
             * Q_LIMIT_ENABLE_CELL<32>
             * Q_MIN_CELL<31:16>
             * Q_SHARED_LIMIT_CELL<15:0>
             */
            sal_memset(entry2,0,sizeof(uint32) * 2);
            bcm5346x_mem_get(unit, M_MMU_THDO_QCONFIG_CELL(queue), entry2, 2);

            /* C412:THDO_QCONFIG_CELL.Q_MIN_CELL
               IF(C11=0, C38, 0)  for base queue*/
            /* C412:THDO_QCONFIG_CELL.Q_MIN_CELL
               = =$C$204 */
            entry2[0] &= ~(0xFFFF << 16);
            if (queue < general_info->total_egr_base_queues_in_device) {
                entry2[0] |= ((output_port_threshold->min_grntd_res_queue_cells_int_buff) << 16);
            }

            /* C414:THDO_QCONFIG_CELL.Q_SHARED_LIMIT_CELL
               =IF($C$14,$C$222,$C$63) */
            if (_soc_ml_mmu_params.lossless_mode_d_c) {
                temp_val = output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff;
            } else {
                temp_val = _soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf_profile_lossy;
            }
            entry2[0] &= ~0xFFFF;
            entry2[0] |= temp_val;
            
            /* C416:THDO_QCONFIG_CELL.Q_LIMIT_DYNAMIC_CELL
               =IF($C$14, 0, 1) */
            entry2[1] &= ~(0x1 << (33-32));
            entry2[1] |= ((_soc_ml_mmu_params.lossless_mode_d_c ? 0 : 1) << (33-32));

            /* C417:THDO_QCONFIG_CELL.Q_LIMIT_ENABLE_CELL
               =IF($C$14, 0, 1) */
            entry2[1] &= ~(0x1 << (32-32));
            entry2[1] |= ((_soc_ml_mmu_params.lossless_mode_d_c ? 0 : 1) << (32-32));

            /* C418:THDO_QCONFIG_CELL.Q_COLOR_ENABLE_CELL = 0 */
            entry2[1] &= ~(0x1 << (35-32));

            /* C419: THDO_QCONFIG_CELL.Q_COLOR_LIMIT_DYNAMIC_CELL
               =IF($C$14, 0, 1) */
            entry2[1] &= ~(0x1 << (36-32));
            entry2[1] |= ((_soc_ml_mmu_params.lossless_mode_d_c ? 0 : 1) << (36-32));

            /* C420: THDO_QCONFIG_CELL.LIMIT_YELLOW_CELL= 0 */
            entry2[1] &= ~(0x1FFF << (37-32));

            bcm5346x_mem_set(unit, M_MMU_THDO_QCONFIG_CELL(queue), entry2, 2);

            /* MMU_THDO_QOFFSET_CELL
             * LIMIT_RED_CELL<51:39>
             * RESET_OFFSET_RED_CELL<38:26>
             * RESET_OFFSET_YELLOW_CELL<25:13>
             * RESET_OFFSET_CELL<12:0>
             */
            sal_memset(entry2,0,sizeof(uint32) * 2);
            bcm5346x_mem_get(unit, M_MMU_THDO_QOFFSET_CELL(queue), entry2, 2);

            /* C415:THDO_QOFFSET_CELL.RESET_OFFSET_CELL =2 */
            entry2[0] &= ~0x1FFF;
            entry2[0] |= 0x2;

            /* C421: THDO_QOFFSET_CELL.LIMIT_RED_CELL  = 0 */ 
            entry2[1] &= ~(0x1FFF << (39-32));

            /* C422: THDO_QOFFSET_CELL.RESET_OFFSET_YELLOW_CELL =2 */
            entry2[0] &= ~(0x1FFF << 13);
            entry2[0] |= (0x2 << 13);

            /* C423: THDO_QOFFSET_CELL.RESET_OFFSET_RED_CELL =2 */
            entry2[0] &= ~(0x3F << 26);
            entry2[1] &= ~(0x7F << (32-32));
            entry2[0] |= (0x2 << 26);

            bcm5346x_mem_set(unit, M_MMU_THDO_QOFFSET_CELL(queue), entry2, 2);

        }


        for (op_node_offset = 0; 
                op_node_offset < sal_ceil_func(SOC_PORT_NUM_UC_COSQ(port),8);
                op_node_offset++) {

            
            /* MMU_THDO_OPNCONFIG_QENTRY
             * PID<49:46>
             * LIMIT_YELLOW_QENTRY<45:33>
             * PORT_LIMIT_ENABLE_QENTRY<32>
             * OPN_SHARED_RESET_VALUE_QENTRY<31:16>
             * OPN_SHARED_LIMIT_QENTRY<15:0>
             */                
            sal_memset(entry2,0,sizeof(uint32) * 2);
            bcm5346x_mem_get(unit, M_MMU_THDO_OPNCONFIG_QENTRY(op_node + op_node_offset), entry2, 2);

            /* C452:THDO_OPNCONFIG_QENTRY.OPN_SHARED_LIMIT_QENTRY = =$C$224 */
            entry2[0] &= ~0xFFFF;
            entry2[0] |= output_port_threshold->min_grntd_tot_shr_queue_cells_EQEs;

            /* C453:THDO_OPNCONFIG_QENTRY.OP_SHARED_RESET_VALUE_QENTRY
               =C452-2 */
            temp_val = output_port_threshold->min_grntd_tot_shr_queue_cells_EQEs - 2;
            entry2[0] &= ~(0xFFFF << 16);
            entry2[0] |= (temp_val << 16);

            /* C454:THDO_OPNCONFIG_QENTRY.PORT_LIMIT_ENABLE_QENTRY
               = 0 */
            entry2[1] &= ~(0x1 << (32-32));

            /* THDO_OPNCONFIG_QENTRY.PID = port */
            entry2[1] &= ~(0xF << (46-32));
            entry2[1] |= (port << (46-32));

            bcm5346x_mem_set(unit, M_MMU_THDO_OPNCONFIG_QENTRY(op_node + op_node_offset), entry2, 2);
        }
        for (queue = SOC_PORT_UC_COSQ_BASE(port);
                queue < SOC_PORT_UC_COSQ_BASE(port) + SOC_PORT_NUM_UC_COSQ(port);
                queue++) {

            /* MMU_THDO_QCONFIG_QENTRY
             * LIMIT_YELLOW_QENTRY<49:37>
             * Q_COLOR_LIMIT_DYNAMIC_QENTRY<36>
             * Q_COLOR_ENABLE_QENTRY<35>
             * Q_LIMIT_DYNAMIC_QENTRY<33>
             * Q_LIMIT_ENABLE_QENTRY<32>
             * Q_MIN_QENTRY<31:16>
             * Q_SHARED_LIMIT_QENTRY<15:0>
             */
            sal_memset(entry2,0,sizeof(uint32) * 2);
            bcm5346x_mem_get(unit, M_MMU_THDO_QCONFIG_QENTRY(queue), entry2, 2);

            /* C455:THDO_QCONFIG_QENTRY.Q_MIN_QENTRY = =$C$206 */
            entry2[0] &= ~(0xFFFF << 16);
            if (queue < general_info->total_egr_base_queues_in_device) {
                entry2[0] |= (output_port_threshold->min_grntd_res_queue_cells_EQEs) << 16;
            }

            /* C456:THDO_QCONFIG_QENTRY.Q_SHARED_LIMIT_QENTRY
               =IF($C$14,$C$224,$C$63) */
            if (_soc_ml_mmu_params.lossless_mode_d_c) {
                temp_val = output_port_threshold->min_grntd_tot_shr_queue_cells_EQEs;
            } else {
                temp_val = _soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf_profile_lossy;
            }
            entry2[0] &= ~0xFFFF;
            entry2[0] |= temp_val;

            /* C458:THDO_QCONFIG_QENTRY.Q_LIMIT_DYNAMIC_QENTRY
               =IF($C$14, 0, 1) */
            entry2[1] &= ~(0x1 << (33-32));
            entry2[1] |= ((_soc_ml_mmu_params.lossless_mode_d_c ? 0 : 1) << (33-32));

            /* C459:THDO_QCONFIG_QENTRY.Q_LIMIT_ENABLE_QENTRY
               =IF($C$14, 0, 1) */
            entry2[1] &= ~(0x1 << (32-32));
            entry2[1] |= ((_soc_ml_mmu_params.lossless_mode_d_c ? 0 : 1) << (32-32));

            /* C460:THDO_QCONFIG_QENTRY.Q_COLOR_ENABLE_QENTRY = 0 */
            entry2[1] &= ~(0x1 << (35-32));

            /* C461: THDO_QCONFIG_QENTRY.Q_COLOR_LIMIT_DYNAMIC_QENTRY
               =IF($C$14, 0, 1) */
            entry2[1] &= ~(0x1 << (36-32));
            entry2[1] |= ((_soc_ml_mmu_params.lossless_mode_d_c ? 0 : 1) << (36-32));

            /* C462: THDO_QCONFIG_QENTRY.LIMIT_YELLOW_QENTRY= 0 */
            entry2[1] &= ~(0x1FFF << (37-32));
            bcm5346x_mem_set(unit, M_MMU_THDO_QCONFIG_QENTRY(queue), entry2, 2);

            /* MMU_THDO_QOFFSET_QENTRY
             * LIMIT_RED_QENTRY<51:39>
             * RESET_OFFSET_RED_QENTRY<38:26>
             * RESET_OFFSET_YELLOW_QENTRY<25:13>
             * RESET_OFFSET_QENTRY<12:0>
             */
            sal_memset(entry2,0,sizeof(uint32) * 2);
            bcm5346x_mem_get(unit, M_MMU_THDO_QOFFSET_QENTRY(queue), entry2, 2);

            /* C457:THDO_QOFFSET_QENTRY.RESET_OFFSET_QENTRY =1 */
            entry2[0] &= ~0x1FFF;
            entry2[0] |= 0x1;

            /* C463: THDO_QOFFSET_QENTRY.LIMIT_RED_QENTRY  = 0 */ 
            entry2[1] &= ~(0x1FFF << (39-32));

            /* C464: THDO_QOFFSET_QENTRY.RESET_OFFSET_YELLOW_QENTRY =1 */
            entry2[0] &= ~(0x1FFF << 13);
            entry2[0] |= (0x1 << 13);

            /* C465: THDO_QOFFSET_QENTRY.RESET_OFFSET_RED_QENTRY =1 */
            entry2[0] &= ~(0x3F << 26);
            entry2[1] &= ~(0x7F << (32-32));
            entry2[0] |= (0x1 << 26);

            bcm5346x_mem_set(unit, M_MMU_THDO_QOFFSET_QENTRY(queue), entry2, 2);
        }
			
    }

    for(cos=0; cos< OTHER_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_CONFIG1_THDORQEQ
         * Q_COLOR_DYNAMIC<13>
         * Q_COLOR_ENABLE<12>
         * Q_MIN<11:0>
         */                
        bcm5346x_reg_get(unit, R_OP_QUEUE_CONFIG1_THDORQEQ(cos), &rval);
        /* C467:OP_QUEUE_CONFIG1_THDORQEQ.Q_MIN = C208 */
        rval &= ~0xFFF;
        rval |= output_port_threshold->min_grntd_res_RE_WQs_cells;

        /* C472:OP_QUEUE_CONFIG1_THDORQEQ.Q_COLOR_ENABLE  = 0 */        
        rval &= ~(0x1 << 12);

        /* C473:OP_QUEUE_CONFIG1_THDORQEQ.Q_COLOR_DYNAMIC lossless=0 else 1
           =IF(C14=0, 1, 0) */
        rval &= ~(0x1 << 13);
        rval |= ((_soc_ml_mmu_params.lossless_mode_d_c? 0 : 1) << 13);
        bcm5346x_reg_set(unit, R_OP_QUEUE_CONFIG1_THDORQEQ(cos), rval);
    }

    for(cos=0; cos<OTHER_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_CONFIG_THDORQEQ
         * Q_LIMIT_ENABLE<13>
         * Q_LIMIT_DYNAMIC<12>
         * Q_SHARED_LIMIT<11:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_CONFIG_THDORQEQ(cos), &rval);

        /* C468:OP_QUEUE_CONFIG_THDORQEQ.Q_SHARED_LIMIT
           ==IF($C$14,$C$226,$C$63) */
        if (_soc_ml_mmu_params.lossless_mode_d_c) {
            temp_val = output_port_threshold->
                min_grntd_tot_shr_RE_WQs_cells ;
        } else {
            temp_val =
                _soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf_profile_lossy; 
        }   

        rval &= ~0xFFF;
        rval |= temp_val;

        /*C470: OP_QUEUE_CONFIG_THDORQEQ.Q_LIMIT_DYNAMIC:=lossless=0 else 1
          =IF(C14=0, 1, 0) */

        rval &= ~(0x1 << 12);
        rval |= ((_soc_ml_mmu_params.lossless_mode_d_c? 0 : 1) << 12);

        /*C471:OP_QUEUE_CONFIG_THDORQEQ.Q_LIMIT_ENABLE:=lossless=0 else 1*/
        rval &= ~(0x1 << 13);
        rval |= ((_soc_ml_mmu_params.lossless_mode_d_c? 0 : 1) << 13);
        bcm5346x_reg_set(unit, R_OP_QUEUE_CONFIG_THDORQEQ(cos), rval);
    }

    for(cos=0; cos<OTHER_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_RESET_OFFSET_THDORQEQ
         * Q_RESET_OFFSET<11:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_RESET_OFFSET_THDORQEQ(cos), &rval);
        /*C469: OP_QUEUE_RESET_OFFSET_THDORQEQ.Q_RESET_OFFSET == 1 */
        rval &= ~0xFFF;
        rval |= 0x1;
        bcm5346x_reg_set(unit, R_OP_QUEUE_RESET_OFFSET_THDORQEQ(cos), rval);
    }
    for(cos=0; cos<OTHER_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_LIMIT_YELLOW_THDORQEQ
         * Q_LIMIT_YELLOW<8:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_LIMIT_YELLOW_THDORQEQ(cos), &rval);
        /* C474: OP_QUEUE_LIMIT_YELLOW_THDORQEQ.Q_LIMIT_YELLOW 0 */
        rval &= ~0x1FF;
        bcm5346x_reg_set(unit, R_OP_QUEUE_LIMIT_YELLOW_THDORQEQ(cos), rval);
    }
    for(cos=0; cos<OTHER_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_LIMIT_RED_THDORQEQ
         * Q_LIMIT_RED<8:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_LIMIT_RED_THDORQEQ(cos), &rval);
        /* C475: OP_QUEUE_LIMIT_RED_THDORQEQ.Q_LIMIT_RED =0 */
        rval &= ~0x1FF;
        bcm5346x_reg_set(unit, R_OP_QUEUE_LIMIT_RED_THDORQEQ(cos), rval);
    }

    for(cos=0; cos<OTHER_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_RESET_OFFSET_YELLOW_THDORQEQ
         * RESUME_OFFSET_YELLOW<8:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_RESET_OFFSET_YELLOW_THDORQEQ(cos), &rval);
        /*C476:OP_QUEUE_RESET_OFFSET_YELLOW_THDORQEQ.RESUME_OFFSET_YELLOW=1*/
        rval &= ~0x1FF;
        rval |= 0x1;
        bcm5346x_reg_set(unit, R_OP_QUEUE_RESET_OFFSET_YELLOW_THDORQEQ(cos), rval);
    }

    for(cos=0; cos<OTHER_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_RESET_OFFSET_RED_THDORQEQ
         * RESUME_OFFSET_RED<8:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_RESET_OFFSET_RED_THDORQEQ(cos), &rval);
        /*C477:OP_QUEUE_RESET_OFFSET_RED_THDORQEQ.RESUME_OFFSET_RED=1*/
        rval &= ~0x1FF;
        rval |= 0x1;
        bcm5346x_reg_set(unit, R_OP_QUEUE_RESET_OFFSET_RED_THDORQEQ(cos), rval);
    }

    /* 2.4.2 RQEI */
    for(cos=0; cos<RQEI_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_CONFIG1_THDORQEI
         * Q_COLOR_DYNAMIC<15>
         * Q_COLOR_ENABLE<14>
         * Q_MIN<13:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_CONFIG1_THDORQEI(cos), &rval);
        /* C478:OP_QUEUE_CONFIG1_THDORQEI.Q_MIN = C209 */
        rval &= ~0x3FFF;
        rval |= output_port_threshold->min_grntd_res_RE_WQs_queue_cells_for_int_buff;
        /* C483:OP_QUEUE_CONFIG1_THDORQEI.Q_COLOR_ENABLE  = 0 */
        rval &= ~(0x1 << 14);
        /* C484:OP_QUEUE_CONFIG1_THDORQEI.Q_COLOR_DYNAMIC lossless=0 else 1
           =IF($C$14, 0, 1)*/
        rval &= ~(0x1 << 15);
        rval |= (_soc_ml_mmu_params.lossless_mode_d_c? 0 : 1) << 15;
        bcm5346x_reg_set(unit, R_OP_QUEUE_CONFIG1_THDORQEI(cos), rval);
    }

    for(cos=0; cos<RQEI_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_CONFIG_THDORQEI
         * Q_LIMIT_ENABLE<15>
         * Q_LIMIT_DYNAMIC<14>
         * Q_SHARED_LIMIT<13:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_CONFIG_THDORQEI(cos), &rval);
        /* C479:OP_QUEUE_CONFIG_THDORQEI.Q_SHARED_LIMIT
           =IF($C$14,$C$227,$C$63)..Can be put outside loop */
        if (_soc_ml_mmu_params.lossless_mode_d_c) {
            temp_val = output_port_threshold->min_grntd_tot_shr_RE_WQs_queue_cells_for_int_buff;
        } else {
            temp_val = _soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf_profile_lossy;
        }   
        rval &= ~0x3FFF;
        rval |= temp_val;
        
        /*C481: OP_QUEUE_CONFIG_THDORQEI.Q_LIMIT_DYNAMIC:=lossless=0 else 1
                  =IF($C$14, 0, 1)*/        
        rval &= ~(0x1 << 14);
        rval |= ((_soc_ml_mmu_params.lossless_mode_d_c? 0 : 1) << 14);
        
        /*C482:OP_QUEUE_CONFIG_THDORQEI.Q_LIMIT_ENABLE:=lossless=0 else 1
          =IF($C$14, 0, 1) */
        rval &= ~(0x1 << 15);
        rval |= ((_soc_ml_mmu_params.lossless_mode_d_c? 0 : 1) << 15);
        bcm5346x_reg_set(unit, R_OP_QUEUE_CONFIG_THDORQEI(cos), rval);
    }

    for(cos=0; cos<RQEI_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_RESET_OFFSET_THDORQEI
         * Q_RESET_OFFSET<13:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_RESET_OFFSET_THDORQEI(cos), &rval);
        /*C480: OP_QUEUE_RESET_OFFSET_THDORQEI.Q_RESET_OFFSET == 2 */
        rval &= ~0x3FFF;
        rval |= 0x2;
        bcm5346x_reg_set(unit, R_OP_QUEUE_RESET_OFFSET_THDORQEI(cos), rval);
    }

    for(cos=0; cos<RQEI_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_LIMIT_YELLOW_THDORQEI
         *  Q_LIMIT_YELLOW<10:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_LIMIT_YELLOW_THDORQEI(cos), &rval);
        /* C485: OP_QUEUE_LIMIT_YELLOW_THDORQEI.Q_LIMIT_YELLOW =0 */
        rval &= ~0x7FF;
        bcm5346x_reg_set(unit, R_OP_QUEUE_LIMIT_YELLOW_THDORQEI(cos), rval);
    }

    for(cos=0; cos<RQEI_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_LIMIT_RED_THDORQEI
         * Q_LIMIT_RED<10:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_LIMIT_RED_THDORQEI(cos), &rval);
        /* C486: OP_QUEUE_LIMIT_RED_THDORQEI.Q_LIMIT_RED = 0*/
        rval &= ~0x7FF;
        bcm5346x_reg_set(unit, R_OP_QUEUE_LIMIT_RED_THDORQEI(cos), rval);
    }

    for(cos=0; cos<RQEI_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_RESET_OFFSET_YELLOW_THDORQEI
         * RESUME_OFFSET_YELLOW<10:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_RESET_OFFSET_YELLOW_THDORQEI(cos), &rval);
        /*C487:OP_QUEUE_RESET_OFFSET_YELLOW_THDORQEI.RESUME_OFFSET_YELLOW=2*/
        rval &= ~0x7FF;
        rval |= 0x2;
        bcm5346x_reg_set(unit, R_OP_QUEUE_RESET_OFFSET_YELLOW_THDORQEI(cos), rval);
    }

    for(cos=0; cos<RQEI_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_RESET_OFFSET_RED_THDORQEI
         * RESUME_OFFSET_RED<10:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_RESET_OFFSET_RED_THDORQEI(cos), &rval);
        /*C488:OP_QUEUE_RESET_OFFSET_RED_THDORQEI.RESUME_OFFSET_RED=2*/
        rval &= ~0x7FF;
        rval |= 0x2;
        bcm5346x_reg_set(unit, R_OP_QUEUE_RESET_OFFSET_RED_THDORQEI(cos), rval);
    }

    for(cos=0; cos<RDEQ_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_CONFIG1_THDORDEQ
         * Q_COLOR_DYNAMIC<10>
         * Q_COLOR_ENABLE<9>
         * Q_MIN<8:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_CONFIG1_THDORDEQ(cos), &rval);
        /* C501:OP_QUEUE_CONFIG1_THDORDEQ.Q_MIN = C211 */
        rval &= ~0x1FF;
        rval |= output_port_threshold->min_grntd_res_EP_redirect_queue_entry_cells;
        
        /* C506:OP_QUEUE_CONFIG1_THDORDEQ.Q_COLOR_ENABLE  = 0 */        
        rval &= ~(0x1 << 9);
        
        /* C507:OP_QUEUE_CONFIG1_THDORDEQ.Q_COLOR_DYNAMIC lossless=0 else 1
           =IF(C14=0, 1, 0)*/
        rval &= ~(0x1 << 10);
        rval |= ((_soc_ml_mmu_params.lossless_mode_d_c? 0 : 1) << 10);
        bcm5346x_reg_set(unit, R_OP_QUEUE_CONFIG1_THDORDEQ(cos), rval);
    }

    for(cos=0; cos<RDEQ_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_CONFIG_THDORDEQ
         * Q_LIMIT_ENABLE<10>
         * Q_LIMIT_DYNAMIC<9>
         * Q_SHARED_LIMIT<8:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_CONFIG_THDORDEQ(cos), &rval);
        /* C502:OP_QUEUE_CONFIG_THDORDEQ.Q_SHARED_LIMIT
           =IF(C14=0, C63, C229)..Can be put outside loop */
        if (_soc_ml_mmu_params.lossless_mode_d_c) {
            temp_val = output_port_threshold->min_grntd_tot_shr_EP_redirect_queue_entry_cells;
        } else {
            temp_val = _soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf_profile_lossy;
        }   
        rval &= ~0x1FF;
        rval |= temp_val;
        
        /*C504: OP_QUEUE_CONFIG_THDORDEQ.Q_LIMIT_DYNAMIC:=lossless=0 else 1i
          =IF(C14=0, 1, 0)*/
        rval &= ~(0x1 << 9);
        rval |= ((_soc_ml_mmu_params.lossless_mode_d_c? 0 : 1) << 9);
        
        /*C505:OP_QUEUE_CONFIG_THDORDEQ.Q_LIMIT_ENABLE:=lossless=0 else 1
          =IF(C14=0, 1, 0)*/
        rval &= ~(0x1 << 10);
        rval |= ((_soc_ml_mmu_params.lossless_mode_d_c? 0 : 1) << 10);
        bcm5346x_reg_set(unit, R_OP_QUEUE_CONFIG_THDORDEQ(cos), rval);
    }

    for(cos=0; cos<RDEQ_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_RESET_OFFSET_THDORDEQ
         * Q_RESET_OFFSET<8:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_RESET_OFFSET_THDORDEQ(cos), &rval);
        /*C503: OP_QUEUE_RESET_OFFSET_THDORDEQ.Q_RESET_OFFSET == 1 */
        rval &= ~0x1FF;
        rval |= 0x1;
        bcm5346x_reg_set(unit, R_OP_QUEUE_RESET_OFFSET_THDORDEQ(cos), rval);
    }

    for(cos=0; cos<RDEQ_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_LIMIT_YELLOW_THDORDEQ
         * Q_LIMIT_YELLOW<5:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_LIMIT_YELLOW_THDORDEQ(cos), &rval);
        /* C508: OP_QUEUE_LIMIT_YELLOW_THDORDEQ.Q_LIMIT_YELLOW = 0 */
        rval &= ~0x3F;
        bcm5346x_reg_set(unit, R_OP_QUEUE_LIMIT_YELLOW_THDORDEQ(cos), rval);
    }

    for(cos=0; cos<RDEQ_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_LIMIT_RED_THDORDEQ
         * Q_LIMIT_RED<5:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_LIMIT_RED_THDORDEQ(cos), &rval);
        /* C509: OP_QUEUE_LIMIT_RED_THDORDEQ.Q_LIMIT_RED = 0 */
        rval &= ~0x3F;
        bcm5346x_reg_set(unit, R_OP_QUEUE_LIMIT_RED_THDORDEQ(cos), rval);
    }

    for(cos=0; cos<RDEQ_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_RESET_OFFSET_YELLOW_THDORDEQ
         * RESUME_OFFSET_YELLOW<5:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_RESET_OFFSET_YELLOW_THDORDEQ(cos), &rval);
        /*C510:OP_QUEUE_RESET_OFFSET_YELLOW_THDORDEQ.RESUME_OFFSET_YELLOW=1*/
        rval &= ~0x3F;
        rval |= 0x1;
        bcm5346x_reg_set(unit, R_OP_QUEUE_RESET_OFFSET_YELLOW_THDORDEQ(cos), rval);
    }

    for(cos=0; cos<RDEQ_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_RESET_OFFSET_RED_THDORDEQ
         * RESUME_OFFSET_RED<5:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_RESET_OFFSET_RED_THDORDEQ(cos), &rval);
        /*C511:OP_QUEUE_RESET_OFFSET_RED_THDORDEQ.RESUME_OFFSET_RED=1*/
        rval &= ~0x3F;
        rval |= 0x1;
        bcm5346x_reg_set(unit, R_OP_QUEUE_RESET_OFFSET_RED_THDORDEQ(cos), rval);
    }

    for(cos=0; cos<RDEI_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_CONFIG1_THDORDEI
         * Q_COLOR_DYNAMIC<15>
         * Q_COLOR_ENABLE<14>
         * Q_MIN<13:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_CONFIG1_THDORDEI(cos), &rval);
        /* C512:OP_QUEUE_CONFIG1_THDORDEI.Q_MIN = 9 (C204) */
        rval &= ~0x3FFF;
        rval |= output_port_threshold->min_grntd_res_queue_cells_int_buff;
        
        /* C517:OP_QUEUE_CONFIG1_THDORDEI.Q_COLOR_ENABLE  = 0 */
        rval &= ~(0x1 << 14);
        
        /* C518:OP_QUEUE_CONFIG1_THDORDEI.Q_COLOR_DYNAMIC = 1*/
        rval &= ~(0x1 << 15);
        rval |= ((_soc_ml_mmu_params.lossless_mode_d_c? 0 :1) << 15);
        bcm5346x_reg_set(unit, R_OP_QUEUE_CONFIG1_THDORDEI(cos), rval);
    }

    for(cos=0; cos<RDEI_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_CONFIG_THDORDEI
         * Q_LIMIT_ENABLE<15>
         * Q_LIMIT_DYNAMIC<14>
         * Q_SHARED_LIMIT<13:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_CONFIG_THDORDEI(cos), &rval);
         /* C513:OP_QUEUE_CONFIG_THDORDEI.Q_SHARED_LIMIT = 
           =IF(C14=0, C63, C222)*/
        if (_soc_ml_mmu_params.lossless_mode_d_c) {
            temp_val = output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff;
        } else {
            temp_val = _soc_ml_mmu_params.mmu_egr_queue_dyn_thd_param_baf_profile_lossy;
        } 
        rval &= ~0x3FFF;
        rval |= temp_val;
        
        /*C515: OP_QUEUE_CONFIG_THDORDEI.Q_LIMIT_DYNAMIC:=1 
          =IF(C14=0, 1, 0)*/
        rval &= ~(0x1 << 14);
        rval |= ((_soc_ml_mmu_params.lossless_mode_d_c ? 0 :1) << 14);
        
         /*C516:OP_QUEUE_CONFIG_THDORDEI.Q_LIMIT_ENABLE:
          =IF(C14=0, 1, 0)*/
        rval &= ~(0x1 << 15);
        rval |= ((_soc_ml_mmu_params.lossless_mode_d_c ? 0 :1) << 15);
        bcm5346x_reg_set(unit, R_OP_QUEUE_CONFIG_THDORDEI(cos), rval);
    }

    for(cos=0; cos<RDEI_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_RESET_OFFSET_THDORDEI
         * RESUME_OFFSET_RED<5:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_RESET_OFFSET_THDORDEI(cos), &rval);
        /*C514: OP_QUEUE_RESET_OFFSET_THDORDEI.Q_RESET_OFFSET == 2 */
        rval &= ~0x3F;
        rval |= 0x2;
        bcm5346x_reg_set(unit, R_OP_QUEUE_RESET_OFFSET_THDORDEI(cos), rval);
    }

    for(cos=0; cos<RDEI_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_LIMIT_YELLOW_THDORDEI
         * Q_LIMIT_YELLOW<10:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_LIMIT_YELLOW_THDORDEI(cos), &rval);
        /* C519: OP_QUEUE_LIMIT_YELLOW_THDORDEI.Q_LIMIT_YELLOW = 0 */
        rval &= ~0x7FF;
        bcm5346x_reg_set(unit, R_OP_QUEUE_LIMIT_YELLOW_THDORDEI(cos), rval);
    }

    for(cos=0; cos<RDEI_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_LIMIT_RED_THDORDEI
         * Q_LIMIT_RED<10:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_LIMIT_RED_THDORDEI(cos), &rval);
        /* C520: OP_QUEUE_LIMIT_RED_THDORDEI.Q_LIMIT_RED = C486 = 0*/
        rval &= ~0x7FF;
        bcm5346x_reg_set(unit, R_OP_QUEUE_LIMIT_RED_THDORDEI(cos), rval);
    }

    for(cos=0; cos<RDEI_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_RESET_OFFSET_YELLOW_THDORDEI
         * RESUME_OFFSET_YELLOW<10:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_RESET_OFFSET_YELLOW_THDORDEI(cos), &rval);
        /*C521:OP_QUEUE_RESET_OFFSET_YELLOW_THDORDEI.RESUME_OFFSET_YELLOW=2*/
        rval &= ~0x7FF;
        rval |= 0x2;
        bcm5346x_reg_set(unit, R_OP_QUEUE_RESET_OFFSET_YELLOW_THDORDEI(cos), rval);
    }

    for(cos=0; cos<RDEI_QUEUE_SIZE; cos++) {
        /* OP_QUEUE_RESET_OFFSET_RED_THDORDEI
         * RESUME_OFFSET_RED<10:0>
         */
        bcm5346x_reg_get(unit, R_OP_QUEUE_RESET_OFFSET_RED_THDORDEI(cos), &rval);
        /*C522:OP_QUEUE_RESET_OFFSET_RED_THDORDEI.RESUME_OFFSET_RED=2*/
        rval &= ~0x7FF;
        rval |= 0x2;
        bcm5346x_reg_set(unit, R_OP_QUEUE_RESET_OFFSET_RED_THDORDEI(cos), rval);
    }

/* Enable back pressure status from MMU for lossless mode */
    if (_soc_ml_mmu_params.lossless_mode_d_c) {
        /* rval = 0x1ffe; */   /* 1 to 12 ports */
        rval = 0x3ffffffe;

        bcm5346x_reg_set(unit, R_THDIQEN_PORT_PAUSE_ENABLE_64, rval);
        bcm5346x_reg_set(unit, R_THDIRQE_PORT_PAUSE_ENABLE_64, rval);
        bcm5346x_reg_set(unit, R_PORT_PAUSE_ENABLE_64, rval);
    }
	
    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* OpNode related config for remaining OpNodes                 */
    /* (assuming internal memory settings)                         */
    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    for (op_node = 0; op_node <= 127; op_node++) {
        temp_val = 0;

        /* MMU_THDO_OPNCONFIG_CELL
         * PID<49:46>
         * LIMIT_YELLOW_CELL<45:33>
         * PORT_LIMIT_ENABLE_CELL<32>
         * OPN_SHARED_RESET_VALUE_CELL<31:16>
         * OPN_SHARED_LIMIT_CELL<15:0>
         */
        sal_memset(entry2,0,sizeof(uint32) * 2);
        bcm5346x_mem_get(unit, M_MMU_THDO_OPNCONFIG_CELL(op_node), entry2, 2);
        
        temp_val = entry2[0] & 0xFFFF;
        
        if (temp_val == 0) {
            /* THDO_OPNCONFIG_CELL.OPN_SHARED_LIMIT_CELL*/
            entry2[0] &= ~0xFFFF;
            entry2[0] |= output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff;

            /* THDO_OPNCONFIG_CELL.OP_SHARED_RESET_VALUE_CELL */
            temp_val = output_port_threshold->min_grntd_tot_shr_queue_cells_int_buff -
                (2 * general_info->ether_mtu_cells_for_int_buff);
            entry2[0] &= ~(0xFFFF << 16);
            entry2[0] |= (temp_val << 16);
            
            /* THDO_OPNCONFIG_CELL.PORT_LIMIT_ENABLE_CELL */
            entry2[1] &= ~(0x1 << (32-32));
            
            bcm5346x_mem_set(unit, M_MMU_THDO_OPNCONFIG_CELL(op_node), entry2, 2);
        }
    }
    
    for (op_node = 0; op_node <= 127; op_node++) {
         temp_val = 0 ;

         /* MMU_THDO_OPNCONFIG_QENTRY
          * PID<49:46>
          * LIMIT_YELLOW_QENTRY<45:33>
          * PORT_LIMIT_ENABLE_QENTRY<32>
          * OPN_SHARED_RESET_VALUE_QENTRY<31:16>
          * OPN_SHARED_LIMIT_QENTRY<15:0>
          */

         sal_memset(entry2,0,sizeof(uint32) * 2);
         bcm5346x_mem_get(unit, M_MMU_THDO_OPNCONFIG_QENTRY(op_node), entry2, 2);

         temp_val = entry2[0] & 0xFFFF;
         
         if (temp_val == 0) {
             /* THDO_OPNCONFIG_QENTRY.OPN_SHARED_LIMIT_QENTRY */
             entry2[0] &= ~0xFFFF;
             entry2[0] |= output_port_threshold->min_grntd_tot_shr_queue_cells_EQEs;
             
             /* THDO_OPNCONFIG_QENTRY.OP_SHARED_RESET_VALUE_QENTRY */
             temp_val = output_port_threshold->min_grntd_tot_shr_queue_cells_EQEs - 2;
             
             entry2[0] &= ~(0xFFFF << 16);
             entry2[0] |= (temp_val << 16);
             
             /* THDO_OPNCONFIG_QENTRY.PORT_LIMIT_ENABLE_QENTRY */
             entry2[1] &= ~(0x1 << (32-32));
             
             bcm5346x_mem_set(unit, M_MMU_THDO_OPNCONFIG_QENTRY(op_node), entry2, 2);
        }
    }

    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* Subport queue Settings ?                                    */
    /* (Assuming lossless-internal memory operation)               */
    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* Extended Queue(BCM_COSQ_GPORT_SUBSCRIBE) Settings           */
    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    for (queue = SOC_PORT_UC_COSQ_BASE(BCM5346X_LPORT_MAX) +
                 SOC_PORT_NUM_UC_COSQ(BCM5346X_LPORT_MAX);
         queue <= 511;
         queue++) {
            _soc_ml_mmu_config_extra_queue(unit, queue,
                                               &_soc_ml_mmu_params,
                                               output_port_threshold);
    }
}

static void
soc_post_mmu_init(uint8 unit){
    uint32 val;
#ifdef __SIM__
    uint32 entry2[2];    
#endif /* __SIM__ */

    /* 
     * TXLP_HW_RESET_CONTROL_1 :
     * COUNT<15:0>
     * RESET_ALL<16>
     * VALID<17>
     * DONE<18>
     */
     
    /* COUNT[Bit 15:0] = 0x84, VALID[Bit 17] = 1 */
    bcm5346x_reg_get(unit, R_TXLP_HW_RESET_CONTROL_1, &val);    
    val = (0x1 << 17) | 0x84;
    bcm5346x_reg_set(unit, R_TXLP_HW_RESET_CONTROL_1, val);

#ifdef __SIM__
    entry2[0] = 0x3fff;
    entry2[1] = 0;
    bcm5346x_mem_set(unit, M_EPC_LINK_BMAP, entry2, 2);    
#endif /* __SIM__ */

    /* TXLP_PORT_ADDR_MAP_TABLE? */

    val = 0 | (0x1F << 12);
    bcm5346x_mem_set(unit, M_TXLP_PORT_ADDR_MAP_TABLE(0), &val, 1); 
    val = 0x20 | (0x3f << 12);
    bcm5346x_mem_set(unit, M_TXLP_PORT_ADDR_MAP_TABLE(1), &val, 1);
    val = 0x40 |  (0x5f << 12);
    bcm5346x_mem_set(unit, M_TXLP_PORT_ADDR_MAP_TABLE(2), &val, 1);
    val = 0x60 |  (0x7f << 12);
    bcm5346x_mem_set(unit, M_TXLP_PORT_ADDR_MAP_TABLE(3), &val, 1);   
}

static void
soc_cosq_init(uint8 unit)
{
    uint32        lport=0;

    int numq = 0;
    /* set SOC_PORT_COSQ_BASE, SOC_PORT_UC_COSQ_BASE and SOC_PORT_NUM_UC_COSQ */
    /* 
    * CPU : queue 0 ~ 47 (48 queues)
    * LB port : queue 48 ~ 71 (24 queues)
    * port 1 : queue 72 ~ 79 (8 queues)
    * port 2 : queue 80 ~ 87 (8 queues)
    * ......
    */

    /* first front panel port needs to start with 72 */
    numq = 72;    

    SOC_PORT_COSQ_BASE(0) = 0x0;
    SOC_PORT_UC_COSQ_BASE(0) = 0x0;
    SOC_PORT_NUM_UC_COSQ(0) = 48;	
    /* For Loopback port it is hardcoded as 48 as a base queue*/
    SOC_PORT_COSQ_BASE(BCM5346X_LPORT_MAX + 1) = 48;
    SOC_PORT_UC_COSQ_BASE(BCM5346X_LPORT_MAX + 1) = 48;
    SOC_PORT_NUM_UC_COSQ(BCM5346X_LPORT_MAX + 1) = 24;

    for (lport = 1 ; lport <= BCM5346X_LPORT_MAX ; lport++) {
          SOC_PORT_UC_COSQ_BASE(lport) = SOC_PORT_UC_COSQ_BASE(lport-1) + numq;;
            SOC_PORT_COSQ_BASE(lport) = SOC_PORT_UC_COSQ_BASE(lport);
            SOC_PORT_NUM_UC_COSQ(lport) = 8;
			numq = SOC_PORT_NUM_UC_COSQ(lport);

    }


    return;
}
void soc_cosq_post_init(uint8 unit, int mode) {

     int idx, idx2;
     uint32 val;
     uint32 entry_tot[4];
	 int lport;
	 int queue_num;
    uint32 dot1pmap[16] = {
    0x00000000, 0x00000001, 0x00000004, 0x00000005, 0x00000008, 0x00000009, 0x0000000c, 0x0000000d,
    0x00000010, 0x00000011, 0x00000014, 0x00000015, 0x00000018, 0x00000019, 0x0000001c, 0x0000001d };	 
	 entry_tot[0] = 0;
	 entry_tot[1] = 0;

	  for (idx=0; idx<64; idx++) {
			entry_tot[0] = 0;
			bcm5346x_mem_set(unit, M_LLS_L0_CONFIG(idx), entry_tot, 1); 		
      } 

      for (idx = 0; idx < 128; idx++) {
   			bcm5346x_mem_set(unit, M_LLS_L0_PARENT(idx), entry_tot, 1); 
		    bcm5346x_mem_set(unit, M_LLS_L1_CHILD_WEIGHT_CFG_CNT(idx), entry_tot, 2);
			bcm5346x_mem_set(unit, M_LLS_L1_PARENT(idx), entry_tot, 1);
   		    bcm5346x_mem_set(unit, M_LLS_L1_CONFIG(idx), entry_tot, 2); 		  
	  }
      for (idx = 0; idx < 512; idx++) {
           bcm5346x_mem_set(unit, M_MMU_RQE_QUEUE_OP_NODE_MAP(idx), entry_tot, 1);
           bcm5346x_mem_set(unit, M_MMU_WRED_QUEUE_OP_NODE_MAP(idx), entry_tot, 1);
           bcm5346x_mem_set(unit, M_LLS_L2_CHILD_WEIGHT_CFG_CNT(idx), entry_tot, 2);	
		   bcm5346x_mem_set(unit, M_EGR_QUEUE_TO_PP_PORT_MAP(idx), entry_tot, 1);
      }

	  /* Base queue number assignment */
      for (idx = 0; idx <= (BCM5346X_LPORT_MAX+1); idx++) {
           /* 
                     * ING_COS_MODE
                     * COS_MODE<1:0>
                     * BASE_QUEUE_NUM<10:2>
                     * SERVICE_BASE_QUEUE_NUM<19:11>
                     */
     	    bcm5346x_reg_get(unit, R_ING_COS_MODE(idx), &val);
            /* BASE_QUEUE_NUM<10:2> = base queue number on this egress port */
     	    val &= ~(0x1ff << 2);
	        val |= (SOC_PORT_UC_COSQ_BASE(idx) << 2);
     	    //val &= ~(0x3);	
	        //val |= (0);
         	bcm5346x_reg_set(unit, R_ING_COS_MODE(idx), val);

			bcm5346x_reg_get(unit, R_RQE_PP_PORT_CONFIG(idx), &val);
			val &= ~(0x1ff << 0);
			val |= (SOC_PORT_UC_COSQ_BASE(idx) << 0);
			bcm5346x_reg_set(unit, R_RQE_PP_PORT_CONFIG(idx), val);

      }	  

	  for (idx = 0; idx <= 0x14; idx++) {
		entry_tot[0] = idx;
		for (idx2 = 0; idx2 < 8; idx2++) {
			 if (idx >= 6 && idx2 >= 4) {
				 entry_tot[0] = 0;
			 }
			 bcm5346x_mem_set(unit, M_MMU_RQE_QUEUE_OP_NODE_MAP(idx * 8 + idx2), entry_tot, 1);
			 bcm5346x_mem_set(unit, M_MMU_WRED_QUEUE_OP_NODE_MAP(idx * 8 + idx2), entry_tot, 1);		 			 
		}
	  }

      /*          PORT           L0           L1          L2
               CPU    0                (0-2)       (0-5)     (0~47)
               LBP     13              (3)           (6~8)    (48 ~ 71)
               ge/xe  (1-12)        (4-15)     (9~20)   (72 ~ 96)
               ge0     1                 4            9            (72 ~ 75)             
          */

	  /****************** CPU lls setting ***********************************/ 
      entry_tot[0] = (1 << 6) | (0);	  
	  bcm5346x_mem_set(unit, M_LLS_PORT_CONFIG(0), entry_tot, 1);
      /* c_type[5] c_parent[4:0] */
	  entry_tot[0] = (1 << 5) | (0);
      bcm5346x_mem_set(unit, M_LLS_L0_PARENT(0), entry_tot, 1);
	  /* reserve for s1 range 0 ~ 23 */
	  entry_tot[0] = (0);
      bcm5346x_mem_set(unit, M_LLS_L0_PARENT(1), entry_tot, 1);
      bcm5346x_mem_set(unit, M_LLS_L0_PARENT(2), entry_tot, 1);	 

	  /* p_start_spri[6:0]=0, p_num_spri[10:7]=0xF, p_wrr_in_use[11] = 0, p_cfg_ef_propagate[12] = 0, p_vect_spri_7_4[16:13]=0x3 */
	  entry_tot[0] = 0x00006780;
      bcm5346x_mem_set(unit, M_LLS_L0_CONFIG(0), entry_tot, 1);
	  /* p_start_spri[6:0]=0, p_num_spri[10:7]=0, p_wrr_in_use[11] = 0, p_cfg_ef_propagate[12] = 0, p_vect_spri_7_4[16:13]=0 */
	  entry_tot[0] = 0x00000000;
      bcm5346x_mem_set(unit, M_LLS_L0_CONFIG(1), entry_tot, 1);		
      bcm5346x_mem_set(unit, M_LLS_L0_CONFIG(2), entry_tot, 1);
	  /* c_parent[5:0] */
	  entry_tot[0] = (0);
	  bcm5346x_mem_set(unit, M_LLS_L1_PARENT(0), entry_tot, 1);	  		   
      for (idx = 0; idx < (SOC_PORT_NUM_UC_COSQ(0)/8); idx++) {
		  /* p_start_uc_spri[8:0] = SOC_PORT_COSQ_BASE(lport) + idx * 8, p_start_mc_spri[17:9] = 0, p_num_spri[21:18] = 0xF, 
			   p_spri_select[29:22] = 0, p_wrr_in_use[30] = 0, p_cfg_ef_propagate[31] = 0, p_vect_spri_7_4[35:32] = 0xF */
      	  entry_tot[0] = (SOC_PORT_COSQ_BASE(0) + idx * 8) | (0xF << 18);
   	      entry_tot[1] = 0x0000000f;
     	  bcm5346x_mem_set(unit, M_LLS_L1_CONFIG(idx), entry_tot, 2);  
      } 
      for (queue_num = 0; queue_num < (SOC_PORT_NUM_UC_COSQ(0)); queue_num++) {
		  /* c_parent[6:0], c_ef[7] */
      	  entry_tot[0] = (0 + queue_num/8);
   	      entry_tot[1] = 0x00000000; // reserved
     	  bcm5346x_mem_set(unit, M_LLS_L2_PARENT(SOC_PORT_UC_COSQ_BASE(0) + queue_num), entry_tot, 2);
 	      entry_tot[0] = (0x0) << 12;
		  bcm5346x_mem_set(unit, M_EGR_QUEUE_TO_PP_PORT_MAP(queue_num), entry_tot, 1);
      } 	  

      /***************** Loop back port setting ***********************************/
	  
      /* LLS_PORT_CONFIG: p_vect_spri_7_4[15:12] , packet_mode_shaper_accounting_enable[11], 
               packet_mode_wrr_accounting_enable[10] = 1, p_num_spri[9:6]=1, p_start_spri[5:0]=3 */        
      entry_tot[0] = (1 << 10) | (1 << 6) | (3);	  
	  bcm5346x_mem_set(unit, M_LLS_PORT_CONFIG(BCM5346X_LPORT_MAX+1), entry_tot, 1);
	  
	  /* p_start_spri[6:0] = 6, p_num_spri[10:7] = 0xF, p_wrr_in_use[11]=0, p_cfg_ef_propagate[12]=0 , p_vect_spri_7_4[16:13] =0 */
	  entry_tot[0] = (0xF << 7) | (6);
	  bcm5346x_mem_set(unit, M_LLS_L0_CONFIG(3), entry_tot, 1);

	  /* c_type[5], c_parent[4:0] */
	  entry_tot[0] = (1 << 5) | (BCM5346X_LPORT_MAX+1);
	  bcm5346x_mem_set(unit, M_LLS_L0_PARENT(3), entry_tot, 1);

	  /* c_parent[5:0] */
      for (idx = 0; idx < (SOC_PORT_NUM_UC_COSQ(BCM5346X_LPORT_MAX+1)/8); idx++) {	  
	       entry_tot[0] = (3);
           bcm5346x_mem_set(unit, M_LLS_L1_PARENT(6+idx), entry_tot, 1);
           /* p_start_uc_spri[8:0], p_start_mc_spri[17:9], p_num_spri[21:18], 
		       p_spri_select[29:22], p_wrr_in_use[30], p_cfg_ef_propagate[31], p_vect_spri_7_4[35:32] */
           entry_tot[0] = SOC_PORT_COSQ_BASE(BCM5346X_LPORT_MAX+1) + idx * 8;
	       bcm5346x_mem_set(unit, M_LLS_L1_CONFIG(6+idx), entry_tot, 1);
      }
      for (queue_num = SOC_PORT_UC_COSQ_BASE(BCM5346X_LPORT_MAX+1); 
	       queue_num < (SOC_PORT_UC_COSQ_BASE(BCM5346X_LPORT_MAX+1) + SOC_PORT_NUM_UC_COSQ(BCM5346X_LPORT_MAX+1)); queue_num++) {
		  /* c_parent[6:0], c_ef[7] */
      	  entry_tot[0] = (6 + (queue_num - SOC_PORT_UC_COSQ_BASE(BCM5346X_LPORT_MAX+1))/8);
     	  bcm5346x_mem_set(unit, M_LLS_L2_PARENT(queue_num), entry_tot, 1); 
 	      entry_tot[0] = (BCM5346X_LPORT_MAX+1) << 12;
		  bcm5346x_mem_set(unit, M_EGR_QUEUE_TO_PP_PORT_MAP(queue_num), entry_tot, 1);		  
      } 

	  /***************** ge/xe port setting ***********************************/

      SOC_LPORT_ITER(lport) {
             /* LLS_PORT_CONFIG: p_vect_spri_7_4[15:12] , packet_mode_shaper_accounting_enable[11], 
                           packet_mode_wrr_accounting_enable[10], p_num_spri[9:6], p_start_spri[5:0] */ 
             entry_tot[0] = (1 << 6) | (3 + lport);
             bcm5346x_mem_set(unit, M_LLS_PORT_CONFIG(lport), entry_tot, 1);
			 /* c_type[5], c_parent[4:0] */
			 entry_tot[0] = (1 << 5) | (lport);
			 bcm5346x_mem_set(unit, M_LLS_L0_PARENT((3 + lport)), entry_tot, 1);
			 /* p_start_spri[6:0], p_num_spri[10:7], p_wrr_in_use[11], p_cfg_ef_propagate[12], p_vect_spri_7_4[16:13] */
 			 entry_tot[0] = (1 << 7) | (8 + lport);
       		 bcm5346x_mem_set(unit, M_LLS_L0_CONFIG((3 + lport)), entry_tot, 1);
			 if (mode == COS_MOD_SP) {
			     /* c_parent[5:0] */
			     entry_tot[0] = (3 + lport);
			     bcm5346x_mem_set(unit, M_LLS_L1_PARENT((8 + lport)), entry_tot, 1);
			     /* p_start_uc_spri[8:0], p_start_mc_spri[17:9], p_num_spri[21:18], 
			             p_spri_select[29:22], p_wrr_in_use[30], p_cfg_ef_propagate[31], p_vect_spri_7_4[35:32] */
  			     entry_tot[0] = SOC_PORT_COSQ_BASE(lport) | (0xF << 18);
			     bcm5346x_mem_set(unit, M_LLS_L1_CONFIG((8 + lport)), entry_tot, 2);
             } else {
			     /* c_parent[5:0] */
			     entry_tot[0] = (3 + lport);
			     bcm5346x_mem_set(unit, M_LLS_L1_PARENT((8 + lport)), entry_tot, 1);
			     /* p_start_uc_spri[8:0], p_start_mc_spri[17:9], p_num_spri[21:18] = 0, 
			             p_spri_select[29:22], p_wrr_in_use[30], p_cfg_ef_propagate[31], p_vect_spri_7_4[35:32] */
  			     entry_tot[0] = SOC_PORT_COSQ_BASE(lport) | (0x0 << 18) | (1 << 30);
			     bcm5346x_mem_set(unit, M_LLS_L1_CONFIG((8 + lport)), entry_tot, 2);
				 /* Assign queue weight : 1:2:4:8 */
				 for (queue_num = SOC_PORT_UC_COSQ_BASE(lport); queue_num < (SOC_PORT_UC_COSQ_BASE(lport) + 4); queue_num++) {
                  	  entry_tot[0] = (1 << (queue_num - SOC_PORT_UC_COSQ_BASE(lport)));				   
            	      bcm5346x_mem_set(unit, M_LLS_L2_CHILD_WEIGHT_CFG_CNT(queue_num), entry_tot, 1); 
				 }
             }
			 for (queue_num = SOC_PORT_UC_COSQ_BASE(lport); queue_num < (SOC_PORT_UC_COSQ_BASE(lport) + 4); queue_num++) {
			      /* c_parent[6:0], c_ef[7] */			 
			      entry_tot[0] = (8 + lport);			 	
				  bcm5346x_mem_set(unit, M_LLS_L2_PARENT(queue_num), entry_tot, 1);
				  entry_tot[0] = lport << 12;
     	  		  bcm5346x_mem_set(unit, M_EGR_QUEUE_TO_PP_PORT_MAP(queue_num), entry_tot, 1);	
			 } 		          
	               
	  }            

      #define INT_PRI_MAX  16
      /* COS_MAP: unity mapping */
      for (idx = 0; idx < INT_PRI_MAX; idx++) {
          if ((idx/2) < 4) {
			   val = ((idx/2));
          }
		  bcm5346x_mem_set(unit, M_COS_MAP(idx + INT_PRI_MAX), &val, 1);	
          bcm5346x_mem_set(unit, M_COS_MAP(idx), &val, 1);
      }		

	  for (idx = 0; idx <= BCM5346X_LPORT_MAX; idx++) {
         if (-1 == SOC_PORT_L2P_MAPPING(idx)) {
            continue;
         }

        /*
                * ING_PRI_CNG_MAP: Unity priority mapping and CNG = 0 or 1
                */
        for (idx2 = 0; idx2 < 16; idx2++) {
            bcm5346x_mem_set(unit, M_ING_PRI_CNG_MAP(idx*16+idx2), &dot1pmap[idx2], 1);
        }
      }
  
}

static void
soc_mmu_init(uint8 unit)
{
    int i;
    uint32 val, mmu_aging_lmt_int_entry[2] = {0x0, 0x0};
    uint32 val2[2];    
    /* Init Link List Scheduler */
    bcm5346x_reg_set(unit, R_LLS_SOFT_RESET, 0);

    /* INIT [Bit 0] = 1 */
    bcm5346x_reg_set(unit, R_LLS_INIT, 1);

#ifndef __SIM__
    /* Wait for LLS init done. */
    /* Wait for IPIPE memory initialization done. */
    do {
        /* Polling INIT_DONE[Bit 1] */
        bcm5346x_reg_get(unit, R_LLS_INIT, &val);
        if (val & (0x1 << 1)) {
            break;
        }
        
    } while (1);
#endif

    /* Enable all ports */
    /* val = 0x1FFF; */
    val = 0x3fffffff;
    bcm5346x_reg_set(unit, R_INPUT_PORT_RX_ENABLE_64, val);
    bcm5346x_reg_set(unit, R_THDIQEN_INPUT_PORT_RX_ENABLE_64, val);
    bcm5346x_reg_set(unit, R_THDIRQE_INPUT_PORT_RX_ENABLE_64, val);

    bcm5346x_reg_set(unit, R_THDI_BYPASS, 0);
    bcm5346x_reg_set(unit, R_THDIQEN_THDI_BYPASS, 0);
    bcm5346x_reg_set(unit, R_THDIRQE_THDI_BYPASS, 0);
    bcm5346x_reg_set(unit, R_THDO_BYPASS, 0);

    /* Setup TDM for MMU Arb & LLS */
    soc_mmu_tdm_init(unit);

    /* Enable LLS */
    /* DEQUEUE_ENABLE[Bit 0] = 1, ENQUEUE_ENABLE[Bit 1] = 1, 
     * FC_ENABLE[Bit 2] = 1, MIN_ENABLE[Bit 4] = 1, 
     * PORT_SCHEDULER_ENABLE[Bit 5] = 1, SHAPER_ENABLEE[Bit 3] = 1, 
     * SPRI_VECT_MODE_ENABLE[Bit 6] = 1 */
    bcm5346x_reg_set(unit, R_LLS_CONFIG0, 0x7f);
   
    /* Enable shaper background refresh */
    /* L0_MAX_REFRESH_ENABLE[Bit 2] = 1, L1_MAX_REFRESH_ENABLE[Bit 1] = 1, 
     * L2_MAX_REFRESH_ENABLE[Bit 0] = 1, PORT_MAX_REFRESH_ENABLE[Bit 3] = 1, 
     * S1_MAX_REFRESH_ENABLE[Bit 4] = 1 */
    bcm5346x_reg_set(unit, R_LLS_MAX_REFRESH_ENABLE, 0x1f);

    /* L0_MIN_REFRESH_ENABLE[Bit 2] = 1, L1_MIN_REFRESH_ENABLE[Bit 1] = 1, 
     * L2_MIN_REFRESH_ENABLE[Bit 0] = 1 */
    bcm5346x_reg_set(unit, R_LLS_MIN_REFRESH_ENABLE, 0x7);

    /* RQE configuration */
    /* L0_MCM_MODE[Bit 3:0] = 1, L0_CC_MODE[Bit 7:4] = 1, 
     * L0_UCM_MODE[Bit 11:8] = 1, L1_MODE[Bit 15:12] = 1 */
    /* 0-SP, 1-RR, 2-WRR, 3-EDRR */
    val = (0x1 << 0) | (0x1 << 4) | (0x1 << 8) | (0x1 << 12);
    bcm5346x_reg_set(unit, R_RQE_SCHEDULER_CONFIG, val);

    /* WRR_WEIGHT[Bit 6:0] = 1 */
    for (i = 0; i < 12; i++) {
        bcm5346x_reg_set(unit, R_RQE_SCHEDULER_WEIGHT_L0_QUEUE(i), 0x1);
    }

    /* WRR_WEIGHT[Bit 6:0] = 1 */
    for (i = 0; i < 3; i++) {
        bcm5346x_reg_set(unit, R_RQE_SCHEDULER_WEIGHT_L1_QUEUE(i), 0x1);
    }

    /* WRED Configuration? */

    /* _soc_saber2_perq_flex_counters_init? */

    val = 0xffffffff; /* PRIx_GRP = 0x7 */

    bcm5346x_reg_set(unit, R_MMU_ENQ_PROFILE_0_PRI_GRP0, val);
    bcm5346x_reg_set(unit, R_MMU_ENQ_PROFILE_0_PRI_GRP1, val);

    bcm5346x_reg_set(unit, R_PROFILE0_PRI_GRP0, val);
    bcm5346x_reg_set(unit, R_PROFILE0_PRI_GRP1, val);
    bcm5346x_reg_set(unit, R_THDIRQE_PROFILE0_PRI_GRP0, val);
    bcm5346x_reg_set(unit, R_THDIRQE_PROFILE0_PRI_GRP1, val);
    bcm5346x_reg_set(unit, R_THDIQEN_PROFILE0_PRI_GRP0, val);
    bcm5346x_reg_set(unit, R_THDIQEN_PROFILE0_PRI_GRP1, val);

    val = 0x0;
    /* Using the same rval1 for setting the remaining registers */
    bcm5346x_reg_set(unit, R_MMU_ENQ_IP_PRI_TO_PG_PROFILE_0, val);
    val2[0] = 0;
    val2[1] = 0;
    bcm5346x_reg64_set(unit, R_PORT_PROFILE_MAP, val2, 2);
    bcm5346x_reg64_set(unit, R_THDIRQE_PORT_PROFILE_MAP, val2, 2);
    bcm5346x_reg64_set(unit, R_THDIQEN_PORT_PROFILE_MAP, val2, 2);

    bcm5346x_reg_set(unit, R_BUFFER_CELL_LIMIT_SP_SHARED, val);
    bcm5346x_reg_set(unit, R_THDIRQE_BUFFER_CELL_LIMIT_SP_SHARED, val);
    bcm5346x_reg_set(unit, R_THDIQEN_BUFFER_CELL_LIMIT_SP_SHARED, val);

    /* Input port per-device global headroom */
    /* STAT_CLEAR[Bit 2] = 0, PARITY_CHK_EN[Bit 1] = 1, 
     * PARITY_CHK_EN[Bit 0] = 1 */
    val = 0x1 | (0x1 << 1);
    bcm5346x_reg_set(unit, R_THDO_MISCCONFIG, val);

    /* EARLY_E2E_SELECT [Bit 0] = 0 */
    bcm5346x_reg_get(unit, R_OP_THR_CONFIG, &val);
    val &= ~0x1;
    bcm5346x_reg_set(unit, R_OP_THR_CONFIG, val);

    soc_mmu_init_helper(unit);

    /* Initialize MMU internal aging limit memory */
    for (i = 0; i < 32; i++) {
        bcm5346x_mem_set(unit, M_MMU_AGING_LMT_INT(i), mmu_aging_lmt_int_entry, 2);
    }

    soc_post_mmu_init(unit);
}

#ifdef CFG_SWITCH_LAG_INCLUDED
#if defined(CFG_SWITCH_RATE_INCLUDED) || defined(CFG_SWITCH_QOS_INCLUDED) || defined(CFG_SWITCH_LOOPDETECT_INCLUDED)
static void
_bcm5346x_lag_group_fp_set(uint8 unit, int start_index, uint8 lagid,
                     pbmp_t pbmp, pbmp_t old_pbmp, uint8 revise_redirect_pbmp, uint8 cpu_include)
{
    int i, j;
    uint32 tcam_entry[FP_TCAM_T_SIZE], xy_entry[FP_TCAM_T_SIZE], dm_entry[FP_TCAM_T_SIZE];
    uint32 global_tcam_mask_entry[FP_GLOBAL_TCAM_MASK_T_SIZE] = { 0x1, 0, 0, 0 };

    /* The entry (pbmp) bit 0 is cpu port.
     * The policy[0] redirect entry, bit 5 is cpu port.
     */

    if (cpu_include == TRUE) {
        j = 0;
    } else {
        j = BCM5346X_LPORT_MIN;
    }
    for (i = start_index; j <= BCM5346X_LPORT_MAX; i++, j++) {
        if ((j > 0) && (j < BCM5346X_LPORT_MIN)) {
            continue;
        }
        bcm5346x_mem_get(unit, M_FP_TCAM(i), dm_entry, FP_TCAM_T_SIZE);
        bcm5346x_xy_to_dm(dm_entry, tcam_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

		if (start_index== LOOP_REDIRECT_IDX){
            /*  Revise the source tgid qualify if the port is trunk port */
            if (old_pbmp & (0x1 << j)) {
                tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] &= IFP_SINGLE_WIDE_F1_11__SGLP_UNSET;
                tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] |= (j << (IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT%32) ); 
            }
            if (pbmp & (0x1 << j)) {
                tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] &= IFP_SINGLE_WIDE_F1_11__SGLP_UNSET;
                tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] |= (lagid << (IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT%32) ); 
                tcam_entry[IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT/32] |= 0x20000;
            }
            
		}else{
            /*  Revise the source tgid qualify if the port is trunk port */
            if (old_pbmp & (0x1 << j)) {
                tcam_entry[IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT/32] &= IFP_SINGLE_WIDE_F3_11__SGLP_UNSET_LOWER;
                tcam_entry[IFP_SINGLE_WIDE_F3_11__SGLP_MAXBIT/32] &= IFP_SINGLE_WIDE_F3_11__SGLP_UNSET_HIGHER;
                tcam_entry[IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT/32] |= (j << (IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT%32) );
                tcam_entry[IFP_SINGLE_WIDE_F3_11__SGLP_MAXBIT/32] |= (j >> (32 - (IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT%32) ) );
            }
            if (pbmp & (0x1 << j)) {
                tcam_entry[IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT/32] &= IFP_SINGLE_WIDE_F3_11__SGLP_UNSET_LOWER;
                tcam_entry[IFP_SINGLE_WIDE_F3_11__SGLP_MAXBIT/32] &= IFP_SINGLE_WIDE_F3_11__SGLP_UNSET_HIGHER;	    	
                tcam_entry[IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT/32] |= (lagid << (IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT%32) );
                tcam_entry[IFP_SINGLE_WIDE_F3_11__SGLP_MAXBIT/32] |= (lagid >> (32 - (IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT%32) ) );
                tcam_entry[IFP_SINGLE_WIDE_F3_11__SGLP_MAXBIT/32] |= 0x200;
                
                tcam_entry[8] = 0x00203fc0;     //update the SGLP mask for trunk
                
            }
      	
      	}
      	
        bcm5346x_dm_to_xy(tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

        bcm5346x_mem_set(unit, M_FP_TCAM(i), xy_entry, FP_TCAM_T_SIZE);
        bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(i), global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);
    }

}
#endif /* CFG_SWITCH_RATE_INCLUDED || CFG_SWITCH_QOS_INCLUDED || CFG_SWITCH_LOOPDETECT_INCLUDED */

/*
 *  Function : bcm5346x_lag_group_set
 *  Purpose :
 *      Set lag group membership.
 *  Parameters :
 *  Return :
 *  Note :
 *
 */
sys_error_t
bcm5346x_lag_group_set(uint8 unit, uint8 lagid, pbmp_t pbmp)
{
    uint32 bitmap_entry[2] = {(uint32)pbmp, 0x0};
    uint32 old_bitmap_entry[2];
    uint32 entry[3] = {0, 0, 0};
    uint8 i, j, count = 0;
    uint8 trunk_port[BOARD_MAX_PORT_PER_LAG];
    uint32 modbase;
    uint32 trunk_group;
    uint32 trunk_member;
    uint32 reset_bitmap_entry[2] = {0x0, 0x0};
    uint32 reset_val=0;
    
    /* In ML, we use real lagid(1~4) instead of (0~3) for table setting */
    lagid += 1;
    
    bcm5346x_mem_get(0, M_TRUNK_BITMAP(lagid), old_bitmap_entry, 2);

    if (bitmap_entry[0] != old_bitmap_entry[0]) {
        /* Need to update source port qualifier in FP TCAM entry  */
#ifdef CFG_SWITCH_RATE_INCLUDED
        /*
         * Slice 1 Entry 0~23 (one entry for each port):
         * Rate ingress
         */
        _bcm5346x_lag_group_fp_set(unit, RATE_IGR_IDX, lagid,
                                   bitmap_entry[0], old_bitmap_entry[0], FALSE, FALSE);
#endif /* CFG_SWITCH_RATE_INCLUDED */

#ifdef CFG_SWITCH_QOS_INCLUDED
        /*
         * Slice 2 Entry 0~23 (one entry for each port):
         * Port based QoS
         */
        _bcm5346x_lag_group_fp_set(unit, QOS_BASE_IDX, lagid,
                                   bitmap_entry[0], old_bitmap_entry[0], FALSE, FALSE);
#endif /* CFG_SWITCH_QOS_INCLUDED */

#ifdef CFG_SWITCH_LOOPDETECT_INCLUDED
        /*
         * Slice 3 Entry BCM5346X_PORT_MIN~BCM5346X_PORT_MAX (one entry for each port):
         * Loop detect counter
         */
        _bcm5346x_lag_group_fp_set(unit, LOOP_COUNT_IDX, lagid,
                                   bitmap_entry[0], old_bitmap_entry[0], FALSE, FALSE);

			
        /*
         * Slice 3, #define LOOP_REDIRECT_IDX              ( 3 * ENTRIES_PER_SLICE + 35)
         * (one entry for each port, including CPU):
         * Update both source port qualifier 
         */
        _bcm5346x_lag_group_fp_set(unit, LOOP_REDIRECT_IDX, lagid,
                                   bitmap_entry[0], old_bitmap_entry[0], TRUE, TRUE);
                                   
#endif /* CFG_SWITCH_LOOPDETECT_INCLUDED */

    }

		/* Calculate the trunk port members mapping to L.port
		* (eg.)	L.port 1 ~ 3 are trunk ports, then count=4
		*	trunk_port[0] = 1
		*	trunk_port[1] = 2
		*	trunk_port[2] = 3
		*	trunk_port[3] = 4
		*/
    for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
        if(bitmap_entry[0] & (0x1 << i)) {
            count ++;
            trunk_port[count-1] = i;
        }
    }

    /* Keep the same setting as SDK:
    * SOURCE_TRUNK_MAP_MODBASE[0]=0x20, Indexed using MODID, whcih is "0" by default. 
    */
    modbase = 0x20;
    bcm5346x_mem_set(0, M_SOURCE_TRUNK_MAP_MODBASE(0),
                                          &modbase, 1);
                                        
	/* Reset all trunk stuff for this TGID */
	{
        entry[0] = 0x0;
			
			/* set M_SOURCE_TRUNK_MAP[33]~ M_SOURCE_TRUNK_MAP[44] */
	    for (i = (modbase+BCM5346X_PORT_MIN); i <= (modbase+BCM5346X_PORT_MAX); i++) {
	        if(old_bitmap_entry[0] & (0x1 << (i%modbase) )) {
	            bcm5346x_mem_set(unit, M_SOURCE_TRUNK_MAP(i), entry, 3);
	        }
	    }
	    
	    bcm5346x_mem_set(unit, M_TRUNK_BITMAP(lagid), reset_bitmap_entry, 2);
	    
	    trunk_group = 0;
	    bcm5346x_mem_set(unit, M_TRUNK_GROUP(lagid), &trunk_group, 1);
	    
	    for (i = 0; i < BOARD_MAX_PORT_PER_LAG; i++) {
            j = 1+ (8*(lagid-1))+ i;
            bcm5346x_mem_set(unit, M_TRUNK_MEMBER(j), &reset_val, 1);
        }

		}

    if(bitmap_entry[0] != 0){
        /* set M_SOURCE_TRUNK_MAP[33]~ M_SOURCE_TRUNK_MAP[44] */
        for (i = (modbase+BCM5346X_PORT_MIN); i <= (modbase+BCM5346X_PORT_MAX); i++) {
            if(bitmap_entry[0] & (0x1 << (i%modbase) )) {
                entry[0] = 0x1 /*0x1 = TRUNK - Trunk port. */ | (lagid << 2);
                bcm5346x_mem_set(unit, M_SOURCE_TRUNK_MAP(i), entry, 3);
            }
        }
				
        bcm5346x_mem_set(unit, M_TRUNK_BITMAP(lagid), bitmap_entry, 2);
				
				
				
		trunk_group = 0x3; 					//0x3 = SA_DA - If Draco 1.5 hashing, based on SA/DA, otherwise based on SA/DA, VLAN, Ethertype, source module ID/port.
		trunk_group |= ((count-1) << 3); 	//TG_SIZE: Number of members in this LAG. If set to 0, size is 1. If set to 1, size is 2, and so on.
		/* We use fixed tgid(1~4), so make fixed mapping 
		* TRUNK_GROUP[1] -> TRUNK_MEMBER[1]~[8]
		* TRUNK_GROUP[2] -> TRUNK_MEMBER[9]~[16]
		* TRUNK_GROUP[3] -> TRUNK_MEMBER[17]~[24]
		* TRUNK_GROUP[4] -> TRUNK_MEMBER[25]~[32]
		*/
		trunk_group |= ( ( (1 + 8*(lagid-1) )<<5) );	//BASE_PTR
		bcm5346x_mem_set(unit, M_TRUNK_GROUP(lagid), &trunk_group, 1);

				
		for (i = 0; i < BOARD_MAX_PORT_PER_LAG; i++) {
            j = 1+ (8*(lagid-1))+ i;
            trunk_member = trunk_port[i%count];
            bcm5346x_mem_set(unit, M_TRUNK_MEMBER(j), &trunk_member, 1);
        }
	}

    for (i = 0; i < 64; i++) {
        bcm5346x_mem_get(0, M_NONUCAST_TRUNK_BLOCK_MASK(i),
                                      entry, 2);
        entry[0] &= ~old_bitmap_entry[0];
        entry[0] |= bitmap_entry[0];
        if (count != 0) {
            entry[0] &= ~(0x1 << trunk_port[i%count]);
        }
        bcm5346x_mem_set(0, M_NONUCAST_TRUNK_BLOCK_MASK(i),
                                      entry, 2);
    }

    return SYS_OK;
}

/*
 *  Function : bcm5346x_lag_group_get
 *  Purpose :
 *      Get lag group membership.
 *  Parameters :
 *  Return :
 *  Note :
 *
 */
void
bcm5346x_lag_group_get(uint8 unit, uint8 lagid, pbmp_t *pbmp) {
    uint32 bitmap_entry[2];
    
    /* In ML, we use real lagid(1~4) instead of (0~3) for table setting */
    lagid += 1;
    
    bcm5346x_mem_get(unit, M_TRUNK_BITMAP(lagid), bitmap_entry, 2);

    *pbmp = (pbmp_t)bitmap_entry[0];
}
#endif /* CFG_SWITCH_LAG_INCLUDED */

static void
bcm5346x_system_init(uint8 unit)
{
    int i, j;
    uint32 entry[9];
    uint32 val;

    /* Default port_entry :
    * bit 32 - 21 : PORT_VID = 0x1
    * bit 67 - 64 : OUTER_TPID_ENABLE = 0x1
    * bit 68 : TRUST_INCOMING_VID = 0x1
    * bit 124 - 121 : CML_FLAGS_NEW = 0x8
    * bit 84 - 81 : CML_FLAGS_MOVE = 0x8
    */
    uint32 port_entry[15] = { 0x00200000, 0x00000000, 0x00000011,
                              0x10000000, 0x00000001, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000};

    uint32 dot1pmap[16] = {
            0x00000000, 0x00000001, 0x00000004, 0x00000005, 0x00000008, 0x00000009, 0x0000000c, 0x0000000d,
            0x00000010, 0x00000011, 0x00000014, 0x00000015, 0x00000018, 0x00000019, 0x0000001c, 0x0000001d };
    
#ifdef CFG_SWITCH_QOS_INCLUDED 
    soc_cosq_post_init(unit, COS_MOD_WRR);
#else
    soc_cosq_post_init(unit, COS_MOD_SP);
#endif

    /* Configurations to guarantee no packet modifications */
    SOC_LPORT_ITER(i) {
        /* ING_OUTER_TPID[0] is allowed outer TPID values */
        entry[0] = 0x1;
        bcm5346x_mem_set(unit, M_SYSTEM_CONFIG_TABLE(i), entry, 1);

        entry[0] = 0x0;
#ifdef CFG_SWITCH_VLAN_UNAWARE_INCLUDED
        /* DISABLE_VLAN_CHECKS[Bit 63] = 1, PACKET_MODIFICATION_DISABLE[Bit 62] = 1 */
        entry[1] = 0xC0000000;
#else
        entry[1] = 0x0;
#endif /* CFG_SWITCH_VLAN_UNAWARE_INCLUDED */
        entry[2] = 0x0;
        bcm5346x_mem_set(unit, M_SOURCE_TRUNK_MAP(i), entry, 3);

        bcm5346x_mem_set(unit, M_PORT(i), port_entry, 15);

        /* Clear Unknown Unicast Block Mask. */
        bcm5346x_mem_get(0, M_UNKNOWN_UCAST_BLOCK_MASK(i), entry, 2);
        entry[0] = 0x0;
        entry[1] = 0x0;		
        bcm5346x_mem_set(0, M_UNKNOWN_UCAST_BLOCK_MASK(i), entry, 2);

        /* Clear ingress block mask. */
        bcm5346x_mem_get(0, M_ING_EGRMSKBMAP(i), entry, 2);
        entry[0] = 0x0;
        entry[1] = 0x0;		
        bcm5346x_mem_set(0, M_ING_EGRMSKBMAP(i), entry, 2);
    }

    for (i = 0; i <= BCM5346X_LPORT_MAX; i++) {
        if (-1 == SOC_PORT_L2P_MAPPING(i)) {
            continue;
        }

        /*
         * ING_PRI_CNG_MAP: Unity priority mapping and CNG = 0 or 1
         */
        for (j = 0; j < 16; j++) {
            bcm5346x_mem_set(unit, M_ING_PRI_CNG_MAP(i*16+j), &dot1pmap[j], 1);
        }
    }

    /* TRUNK32_CONFIG_TABLE: OUTER_TPID_ENABLE[3:0] (Bit 0-3) = 0x1 */
    val = 0x1;
    /*
     * TRUNK32_PORT_TABLE:
     * DISABLE_VLAN_CHECKS[Bit 31] = 1, PACKET_MODIFICATION_DISABLE[Bit 30] = 1
     */
#ifdef CFG_SWITCH_VLAN_UNAWARE_INCLUDED
    entry[0] = 0xC0000000;
#else
    entry[0] = 0x0;
#endif /* CFG_SWITCH_VLAN_UNAWARE_INCLUDED */
    entry[1] = 0x0;

    /* 
     * ING_VLAN_TAG_ACTION_PROFILE:
     * UT_OTAG_ACTION (Bit 2-3) = 0x1           add internal OVID
     * SIT_OTAG_ACTION (Bit 8-10) = 0x0        do not modify
     * SOT_POTAG_ACTION (Bit 15-14) = 0x2   replace incoming OVID with internal OVID
     * SOT_OTAG_ACTION (Bit 17-16) = 0x0     do not modify 
     * DT_POTAG_ACTION (Bit 26-24) = 0x2     replace incoming OVID with internal OVID
     */
    val = (0x2 << 24) | (0x0 << 16) | (0x2 << 14) | (0x0 << 8) | (0x1 << 2);
    bcm5346x_mem_set(unit, M_ING_VLAN_TAG_ACTION_PROFILE(0), &val, 1);

    /*
     * Program l2 user entry table to drop below MAC addresses:
     * 0x0180C2000001, 0x0180C2000002 and 0x0180C200000E
     */

    /* VALID[Bit 0~Bit1] = 1 MAC_ADDR[Bit 2 ~ Bit 49] KEY_TPYE=0:VLAN*/
    entry[0] = (0xC2000001 << 2) | 0x1;
    entry[1] = (0x0180 << 2) | (0xC2000001 >> (32-2));
    entry[2] = 0;
    /* Encoded MASK */
    entry[3] = 0xf8000000;
    entry[4] = 0xfcf7ffff;
    entry[5] = 0x10003f9;
    entry[6] = 0x0;
    entry[7] = 0x0;

    /* DST_DISCARD[Bit 248] = 1 */
    entry[7] |= (1 << (24));
    /* BPDU[Bit 265] = 1 */
    entry[8] |= (0x1 << (9));

    bcm5346x_mem_set(unit, M_L2_USER_ENTRY(0), entry, 9);

    entry[0] = (0xC2000002 << 2) | 0x1;
    entry[3] = 0xf4000000;
    bcm5346x_mem_set(unit, M_L2_USER_ENTRY(1), entry, 9);

    entry[0] = (0xC200000E << 2) | 0x1;
    entry[3] = 0xc4000000;
    bcm5346x_mem_set(unit, M_L2_USER_ENTRY(2), entry, 9);

#if !CONFIG_METROLITE_ROMCODE
#ifdef CFG_SWITCH_DOS_INCLUDED
    /*
     * Enable following Denial of Service protections:
     * DROP_IF_SIP_EQUALS_DIP (R_DOS_CONTROL Bit 0)  = 1
     * IP_FIRST_FRAG_CHECK_ENABLE (R_DOS_CONTROL Bit 4) = 1
     * IPV4_FIRST_FRAG_CHECK_ENABLE =1 (R_DOS_CONTROL Bit 4) =1
     * TCP_HDR_OFFSET_EQ1_ENABLE (DOS_CONTROL_2 Bit 5) =1 
     * TCP_HDR_PARTIAL_ENABLE (DOS_CONTROL_2 Bit 4) =1 
     * ICMP_FRAG_PKTS_ENABLE (DOS_CONTROL_2 Bit 8)=1 
     * 
     * In ML, we also need to add 
     * ICMP_V4_PING_SIZE_ENABLE (DOS_CONTROL_2 Bit 7) = 1
     * ICMP_V6_PING_SIZE_ENABLE (DOS_CONTROL_2 Bit 6) = 1
     */
    
    bcm5346x_reg_set(unit, R_DOS_CONTROL, (1 << 4) | (1 << 0));
    bcm5346x_reg_set(unit, R_DOS_CONTROL_2, (1 << 8) | (1 << 5) | (1<<4) | (3<<6) );
#endif /* CFG_SWITCH_DOS_INCLUDED */

    /* MIN_TCPHDR_SIZE = 0x14 (Default)*/
    entry[1]= 0x14;
    /* BIG_ICMPV6_PKT_SIZE = BIG_ICMP_PKT_SIZE = 0x200 (Default)*/    
    entry[0]= (0x200 << 16) | (0x200);
    bcm5346x_reg64_set(unit, R_DOS_CONTROL_3, entry, 2);

    /* enable FP_REFRESH_ENABLE [Bit 21] */
    bcm5346x_reg_get(unit, R_AUX_ARB_CONTROL_2, &val);
    val |= (1 << 21);
    bcm5346x_reg_set(unit, R_AUX_ARB_CONTROL_2, val);

    /*
     * Enable IPV4_RESERVED_MC_ADDR_IGMP_ENABLE[Bit 31], APPLY_EGR_MASK_ON_L3[Bit 13]
     * and APPLY_EGR_MASK_ON_L2[Bit 12]
     * Disable L2DST_HIT_ENABLE[Bit 2]
     */
    bcm5346x_reg64_get(unit, R_ING_CONFIG_64, entry, 2);
    entry[0] |= 0x80003000;
    entry[0] &= ~0x4;
    bcm5346x_reg64_set(unit, R_ING_CONFIG_64, entry, 2);

    /*
     * IPV4L3_ENABLE[17]=1
     * IPV6L3_ENABLE[16]=1,          
     * IPMCV4_L2_ENABLE[15]=1, 
     * IPMCV6_L2_ENABLE[14]=1, 
     * IPMCV4_ENABLE[13]=1
     * IPMCV6_ENABLE[12]=1, 
     * L3_IPV6_PFM[5:4]=1, 
     * L3_IPV4_PFM[3:2]=1, 
     * L2_PFM[1:0]=1,          
     * EVEN_PARITY[41]
     */    
    entry[0] = 0x0003f015;
    entry[1] = 0x0;
    bcm5346x_mem_set(unit, M_VLAN_PROFILE(0), entry, 2);
#endif /* !CONFIG_METROLITE_ROMCODE */

    /* Do VLAN Membership check EN_EFILTER[Bit 3] for the outgoing port */
    SOC_LPORT_ITER(i) {
        bcm5346x_mem_get(0, M_EGR_PORT(i), entry, 6);
        entry[0] |= (0x1 << 3);
        bcm5346x_mem_set(0, M_EGR_PORT(i), entry, 6);    
    }

#if CFG_RXTX_SUPPORT_ENABLED
    /*
     * Use VLAN 0 for CPU to transmit packets
     * All ports are untagged members, with VALID[56]=1 STG[Bit 32~39]=1 and VLAN_PROFILE_PTR[Bit 72~75]=0
     */
    sal_memset(entry, 0, sizeof(entry));
    entry[0] = 0xfffffffe;
    entry[1] = 0x00000001 | (1<< (56-32));
    entry[2] = 0x0;
    entry[3] = 0x0;
    bcm5346x_mem_set(unit, M_VLAN(0), entry, 7);

    sal_memset(entry, 0, sizeof(entry));
    /* EGR_VLAN.UT_PORT_BITMAP<190:159>*/
    bcm5346x_mem_get(0, M_EGR_VLAN(0), entry, 7);
    entry[4] &= 0x7FFFFFFF;
    entry[4] |= (0xfffffffe << 31);
    entry[5] &= 0x80000000;
    entry[5] |= (0xfffffffe >> 1);
    /* EGR_VLAN.PORT_BITMAP<31:0>*/
    entry[0] = 0xfffffffe;
    /* EGR_VLAN.VALID[56] STG[Bit 32~39]=1*/
    entry[1] |= ((1 << 24) | 1); 
    bcm5346x_mem_set(0, M_EGR_VLAN(0), entry, 7);
    

#ifdef __BOOTLOADER__
    /* Default VLAN 1 with  VALID[56]=1 STG[Bit 32~39]=1 and VLAN_PROFILE_PTR[Bit 72~75]=0 for bootloader */
    entry[0] = 0xfffffffe;
    entry[1] = 0x00000001 | (1<< (56-32));
    entry[2] = 0x0;
    entry[3] = 0x0;
    bcm5346x_mem_set(unit, M_VLAN(1), entry, 7);


    sal_memset(entry, 0, sizeof(entry));
    /* EGR_VLAN.UT_PORT_BITMAP<190:159>*/
    bcm5346x_mem_get(0, M_EGR_VLAN(1), entry, 7);
    entry[4] &= 0x7FFFFFFF;
    entry[4] |= (0xfffffffe << 31);
    entry[5] &= 0x80000000;
    entry[5] |= (0xfffffffe >> 1);
    /* EGR_VLAN.PORT_BITMAP<31:0>*/
    entry[0] = 0xfffffffe;
    /* EGR_VLAN.VALID[56] */
    entry[1] |= ((1 << 24) | 1);
    bcm5346x_mem_set(0, M_EGR_VLAN(1), entry, 7);

#endif /* __BOOTLOADER__ */

    /* Set VLAN_STG and EGR_VLAN_STG to forwarding*/
    entry[0] = 0xfffffffc;
    entry[1] = 0xffffffff;
    entry[2] = 0x0;
    bcm5346x_mem_set(unit, M_VLAN_STG(1), entry, 3);
    bcm5346x_mem_set(unit, M_EGR_VLAN_STG(1), entry, 3);

    /* Make PORT_VID[Bit 32:21] = 0 for CPU port */
    bcm5346x_mem_get(unit, M_PORT(0), port_entry, 15);
    port_entry[0] = (port_entry[0] & 0x001FFFFF) | (0 << 21);
    port_entry[1] = (port_entry[1] & 0xfffffffe) | (((0) >> 11) & 0x1);
    bcm5346x_mem_set(unit, M_PORT(0), port_entry, 15);

#if !CONFIG_METROLITE_ROMCODE
    /*
     * Trap DHCP[Bit 0] and ARP packets[Bit 4, 6] to CPU.
     * Note ARP reply is copied to CPU ONLY when l2 dst is hit.
     */
    /* ML need to chck if it is per port register */
    SOC_LPORT_ITER(i) {
        bcm5346x_reg_set(unit, R_PROTOCOL_PKT_CONTROL(i), 0x51);
    }
    
	/*
	*	In ML, we also need to enable PROTOCOL_PKT_CONTROL in CPU port
	*/
	bcm5346x_reg_set(unit, R_PROTOCOL_PKT_CONTROL(0), 0x51);
		
#endif /* !CONFIG_METROLITE_ROMCODE */
#endif /* CFG_RXTX_SUPPORT_ENABLED */

    /* Enable aging timer */
    bcm5346x_reg_get(unit, R_L2_AGE_TIMER, &val);
    val |= (0x1 << 20);
    bcm5346x_reg_set(unit, R_L2_AGE_TIMER, val);

}

#define FW_ALIGN_BYTES                  16
#define FW_ALIGN_MASK                   (FW_ALIGN_BYTES - 1)
int
_firmware_helper(void *ctx, uint32 offset, uint32 size, void *data)
{
    uint32 val;
    uint32 wbuf[4], ucmem_data[4];
    uint32 *fw_data;
    uint32 *fw_entry;
    uint32 fw_size;
    uint32 idx, wdx;
    phy_ctrl_t *pc = (phy_ctrl_t *)ctx;
    int lport;


    /* Check if PHY driver requests optimized MDC clock */
    if (data == NULL) {
        uint32 rate_adjust;
        val = 1;

        /* Offset value is MDC clock in kHz (or zero for default) */
        if (offset) {
            val = offset / 1500;
        }

        rate_adjust = READCSR(CMIC_RATE_ADJUST_EXT_MDIO);
        rate_adjust &= ~ (0xFFFF0000);
        rate_adjust |= val << 16;
        WRITECSR(CMIC_RATE_ADJUST_EXT_MDIO, val);

        return SYS_OK;
    }
    if (sal_strcmp(pc->drv->drv_name, "bcmi_tsce_xgxs") != 0) {
        return CDK_E_NONE;
    }
    if (size == 0) {
        return SYS_ERR_PARAMETER;
    }
    /* Aligned firmware size */
    fw_size = (size + FW_ALIGN_MASK) & ~FW_ALIGN_MASK;

    /* Enable parallel bus access, ACCESS_MODE [Bit 0] = 0x1 */
    lport = SOC_PORT_P2L_MAPPING(pc->port);
    bcm5346x_reg_set(pc->unit, SOC_PORT_BLOCK(lport), R_XLPORT_WC_UCMEM_CTRL, 0x1);

    /* DMA buffer needs 32-bit words in little endian order */
    fw_data = (uint32 *)data;
    for (idx = 0; idx < fw_size; idx += 16) {
        if (idx + 15 < size) {
            fw_entry = &fw_data[idx >> 2];
        } else {
            /* Use staging buffer for modulo bytes */
            sal_memset(wbuf, 0, sizeof(wbuf));
            sal_memcpy(wbuf, &fw_data[idx >> 2], 16 - (fw_size - size));
            fw_entry = wbuf;
        }
        for (wdx = 0; wdx < 4; wdx++) {
            ucmem_data[wdx] = htol32(fw_entry[wdx]);
        }
        bcm5346x_mem_set(pc->unit, SOC_PORT_BLOCK(lport),
                         M_XLPORT_WC_UCMEM_DATA(idx >> 4), ucmem_data, 4);
    }

    /* Disable parallel bus access */
    bcm5346x_reg_set(pc->unit, SOC_PORT_BLOCK(lport), R_XLPORT_WC_UCMEM_CTRL, 0x0);

    return SYS_OK;
}

extern int
bmd_phy_fw_helper_set(int unit, int port,
                      int (*fw_helper)(void *, uint32, uint32, void *));

/* Function:
 *   bcm5346x_sw_init
 * Description:
 *   Perform chip specific initialization.
 *   This will be called by board_init()
 * Parameters:
 *   None
 * Returns:
 *   None
 */

sys_error_t
bcm5346x_sw_init(void)
{
    int   rv = SYS_OK;
    uint8 unit = 0, lport;
    int port_speed[BCM5346X_LPORT_MAX+1];
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
    int port_cnt = 0;
    sys_error_t sal_config_rv = SYS_OK, an_rv = SYS_OK, cl73_rv = SYS_OK, cl37_rv = SYS_OK;
    pbmp_t active_pbmp, phy_an_pbmp, phy_cl73_pbmp, phy_cl37_pbmp;
    pbmp_t speed_1000_pbmp, speed_2500_pbmp, speed_10000_pbmp;
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */

    /* Get chip revision */
    bcm5346x_chip_revision(unit, &ml_sw_info.devid, &ml_sw_info.revid);
#if CFG_CONSOLE_ENABLED
    sal_printf("\ndevid = 0x%x, revid = 0x%x\n", ml_sw_info.devid, ml_sw_info.revid);
#endif /* CFG_CONSOLE_ENABLED */

#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
    sal_config_rv = sal_config_uint16_get(SAL_CONFIG_SKU_DEVID, &ml_sw_info.devid);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Overwrite SKU device ID with value 0x%x.\n", ml_sw_info.devid);
    }

    sal_config_rv = sal_config_uint8_get(SAL_CONFIG_TSCE_INTERFACE, &tsce_interface);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set TSCE interface to %d. (1:SGMII, 2:Fiber)\n", tsce_interface);
        if ((tsce_interface != SERDES_INTERFACE_SGMII) && 
            (tsce_interface != SERDES_INTERFACE_FIBER)) {
            sal_printf("The TSCE interface %d is not valid and will change it to %d (1:SGMII, 2:FIBER)\n", tsce_interface, CFG_TSCE_INTERFACE);
            tsce_interface = CFG_TSCE_INTERFACE;
        }
    }

    sal_config_rv = sal_config_uint8_get(SAL_CONFIG_VIPER_INTERFACE, &viper_interface);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set VIPER interface to %d. (1:SGMII, 2:Fiber)\n", viper_interface);
        if ((viper_interface != SERDES_INTERFACE_SGMII) && 
            (viper_interface != SERDES_INTERFACE_FIBER)) {
            sal_printf("The VIPER interface %d is not valid and will change it to %d (1:SGMII, 2:FIBER)\n", viper_interface, CFG_VIPER_INTERFACE);
            viper_interface = CFG_VIPER_INTERFACE;
        }
    }
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */

    if ((ml_sw_info.devid == BCM53460_DEVICE_ID) || (ml_sw_info.devid == BCM53461_DEVICE_ID) || 
        (ml_sw_info.devid == BCM56270_DEVICE_ID) || (ml_sw_info.devid == BCM56271_DEVICE_ID)) {
        SOC_PORT_COUNT(unit) = 12;
    } else {
        sal_printf("\nERROR : devid 0x%x is not supported in UM software !\n", ml_sw_info.devid);
        return SYS_ERR_NOT_FOUND;
    }

    /* CPS reset complete SWITCH and CMICd */
    WRITECSR(CMIC_CPS_RESET, 0x1);
#if CONFIG_METROLITE_EMULATION
    sal_usleep(250000);
#else
    sal_usleep(1000);
#endif /* CONFIG_METROLITE_EMULATION */

    soc_init_port_mapping(unit);
    soc_cosq_init(unit);

    soc_reset(unit);

    soc_misc_init(unit);

    soc_mmu_init(unit);

    bcm5346x_system_init(unit);

    /* Probe PHYs */
    SOC_LPORT_ITER(lport) {
        rv = bmd_phy_probe(unit, lport);
        if (CDK_SUCCESS(rv)) {
            rv = PHY_CONFIG_SET(BMD_PORT_PHY_CTRL(unit, lport), PhyConfig_InitSpeed, SOC_PORT_SPEED_MAX(lport), NULL);

            if (IS_XL_PORT(lport)) {
                /* Configure 2-LANCE/4-LANE TSC if necessary. */
                if (SOC_PORT_LANE_NUMBER(lport) == 4) {
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_tsce_xgxs",
                                      BMD_PHY_MODE_SERDES, 0);
                } else if (SOC_PORT_LANE_NUMBER(lport) == 2) {
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_tsce_xgxs",
                                      BMD_PHY_MODE_SERDES, 1);
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_tsce_xgxs",
                                              BMD_PHY_MODE_2LANE, 1);
                } else {
                    /* lane number = 1 */
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_tsce_xgxs",
                                      BMD_PHY_MODE_SERDES, 1);
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_tsce_xgxs",
                                              BMD_PHY_MODE_2LANE, 0);
                }

                if (CDK_SUCCESS(rv)) {
                    rv = bmd_phy_fw_helper_set(unit, lport, _firmware_helper);
                }
            } else if (IS_GX_PORT(lport)){
                /* Configure 2-LANCE/4-LANE TSC if necessary. */
                if (SOC_PORT_LANE_NUMBER(lport) == 4) {
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_viper_xgxs",
                                      BMD_PHY_MODE_SERDES, 0);
                } else if (SOC_PORT_LANE_NUMBER(lport) == 2) {
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_viper_xgxs",
                                      BMD_PHY_MODE_SERDES, 1);
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_viper_xgxs",
                                              BMD_PHY_MODE_2LANE, 1);
                } else {
                    /* lane number = 1 */
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_viper_xgxs",
                                      BMD_PHY_MODE_SERDES, 1);
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_viper_xgxs",
                                              BMD_PHY_MODE_2LANE, 0);
                }
            }

            if (CDK_SUCCESS(rv)) {
                rv = bmd_phy_init(unit, lport);
            }
        }
    }

    SOC_LPORT_ITER(lport) {
        if (SOC_PORT_SPEED_MAX(lport) < 10000) {
            port_speed[lport] = 1000;
        } else {
            port_speed[lport] = 10000;
        }
    }

#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED 
    sal_config_rv = sal_config_pbmp_get(SAL_CONFIG_VALID_PORTS, &active_pbmp);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set valid logical pbmp with value 0x%x.\n", active_pbmp);
        SOC_LPORT_ITER(lport) {
            if (active_pbmp & (0x1 << lport)) {
               port_cnt++; 
            } else {
                SOC_PORT_L2P_MAPPING(lport) = -1;
            }
        }
        SOC_PORT_COUNT(unit) = port_cnt;
    }

    sal_config_rv = sal_config_pbmp_get(SAL_CONFIG_SPEED_1000_PORTS, &speed_1000_pbmp);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set speed (1G) logical pbmp with value 0x%x.\n", speed_1000_pbmp);
        SOC_LPORT_ITER(lport) {
            if ((speed_1000_pbmp & (0x1 << lport)) && (SOC_PORT_SPEED_MAX(lport) >= 1000)){
               port_speed[lport] = 1000;
            }
        }
    }

    sal_config_rv = sal_config_pbmp_get(SAL_CONFIG_SPEED_2500_PORTS, &speed_2500_pbmp);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set speed (2.5G) logical pbmp with value 0x%x.\n", speed_2500_pbmp);
        SOC_LPORT_ITER(lport) {
            if ((speed_2500_pbmp & (0x1 << lport)) && (SOC_PORT_SPEED_MAX(lport) >= 2500)){
               port_speed[lport] = 2500;
            }
        }
    }

    sal_config_rv = sal_config_pbmp_get(SAL_CONFIG_SPEED_10000_PORTS, &speed_10000_pbmp);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set speed (10G) logical pbmp with value 0x%x.\n", speed_10000_pbmp);
        SOC_LPORT_ITER(lport) {
            if ((speed_10000_pbmp & (0x1 << lport)) && (SOC_PORT_SPEED_MAX(lport) >= 10000)){
               port_speed[lport] = 10000;
            }
        }
    }

    an_rv = sal_config_pbmp_get(SAL_CONFIG_PHY_AN_PORTS, &phy_an_pbmp);
    if (an_rv == SYS_OK) {
        sal_printf("Vendor Config : Set AN logical pbmp with value 0x%x.\n", phy_an_pbmp);
    }

    cl73_rv = sal_config_pbmp_get(SAL_CONFIG_PHY_CL73_PORTS, &phy_cl73_pbmp);
    if (cl73_rv == SYS_OK) {
        sal_printf("Vendor Config : Set CL73 logical pbmp with value 0x%x.\n", phy_cl73_pbmp);
    }

    cl37_rv = sal_config_pbmp_get(SAL_CONFIG_PHY_CL37_PORTS, &phy_cl37_pbmp);
    if (cl37_rv == SYS_OK) {
        sal_printf("Vendor Config : Set CL37 logical pbmp with value 0x%x.\n", phy_cl37_pbmp);
    }
#endif /*  CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */

#if !(CONFIG_METROLITE_EMULATION)
    SOC_LPORT_ITER(lport) {
        int ability, an;
        phy_ctrl_t *pc = BMD_PORT_PHY_CTRL(unit, lport);

        if ((port_speed[lport] == 1000) && 
            (SOC_PORT_LANE_NUMBER(lport) == 1)) {
            if (IS_XL_PORT(lport)) {
                if (tsce_interface == SERDES_INTERFACE_SGMII) {
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_tsce_xgxs",
                              BMD_PHY_MODE_FIBER, 0);
                } else {
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_tsce_xgxs",
                              BMD_PHY_MODE_FIBER, 1);
                }
            } else if (IS_GX_PORT(lport)){
                if (viper_interface == SERDES_INTERFACE_SGMII) {
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_viper_xgxs",
                              BMD_PHY_MODE_FIBER, 0);
                } else {
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_viper_xgxs",
                              BMD_PHY_MODE_FIBER, 1);
                }
            }
        }

        if (!pc) {
            sal_printf("bcm5340x_sw_init pc is NULL on lport %d\n", lport);
            return SYS_ERR;
        }

        /* According the speed to configure the phy ability */
        ability = (BMD_PHY_ABIL_1000MB_FD | BMD_PHY_ABIL_100MB_FD);
        if (port_speed[lport] == 1000) {
            ability |= BMD_PHY_ABIL_10MB_FD;
        } else if (port_speed[lport] == 2500) {
            ability |= BMD_PHY_ABIL_2500MB;
        } else if (port_speed[lport] == 10000) {
            ability |= (BMD_PHY_ABIL_10GB | BMD_PHY_ABIL_2500MB);
        }

        if (IS_XL_PORT(lport)) {
            rv = bmd_phy_ability_set(unit, lport, "bcmi_tsce_xgxs", ability);
        } else if (IS_GX_PORT(lport)){
            rv = bmd_phy_ability_set(unit, lport, "bcmi_viper_xgxs", ability);
        }

        if (!SOC_SUCCESS(rv)) {
            sal_printf("bcm5346x_sw_init set phy ability 0x%x on lport %d failed\n", ability, lport);
        }

        if (port_speed[lport] == 1000) {
            if ((IS_XL_PORT(lport) && (tsce_interface == SERDES_INTERFACE_SGMII)) ||
                (IS_GX_PORT(lport) && (viper_interface == SERDES_INTERFACE_SGMII))) {
                an = 0;
            } else {
                an = CFG_CONFIG_1G_PORT_AN;
                if (an == 2) {
                    PHY_CTRL_FLAGS(pc) |= PHY_F_CLAUSE37;
                    /* ML need to chck for viper interface */
                } else if (an == 1) {
                    PHY_CTRL_FLAGS(pc) |= PHY_F_CLAUSE73;
                }
                
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
                if (an_rv == SYS_OK){
                    if (phy_an_pbmp & (0x1 << lport)) {
                        /* Set or clear CL37 flag */
                        if (cl37_rv == SYS_OK) {
                            if (!(PHY_CTRL_FLAGS(pc) & PHY_F_CLAUSE37)) {
                                if  (phy_cl37_pbmp & (0x1 << lport)) {
                                    /* set cl37 flag*/
                                    PHY_CTRL_FLAGS(pc) |= PHY_F_CLAUSE37;
                                }
                            } else {
                                if  (!(phy_cl37_pbmp & (0x1 << lport))) {
                                    /* clear cl37 flag*/
                                    PHY_CTRL_FLAGS(pc) &= ~PHY_F_CLAUSE37;
                                }
                            }
                        }
                        
                        /* Set or clear CL73 flag */
                        if (cl73_rv == SYS_OK) {
                            if (!(PHY_CTRL_FLAGS(pc) & PHY_F_CLAUSE73)) {
                                if  (phy_cl73_pbmp & (0x1 << lport)) {
                                    /* set cl73 flag*/
                                    PHY_CTRL_FLAGS(pc) |= PHY_F_CLAUSE73;
                                }
                            } else {
                                /* an = 1 */
                                if  (!(phy_cl73_pbmp & (0x1 << lport))) {
                                    /* clear cl73 flag*/
                                    PHY_CTRL_FLAGS(pc) &= ~PHY_F_CLAUSE73;
                                }
                            }
                        }
                        an = 1;
                    } else {
                        an = 0;
                    }
                }
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
            }

            an = an ? TRUE : FALSE;
        } else if (port_speed[lport] == 10000) {
            an = CFG_CONFIG_10G_PORT_AN;
            PHY_CTRL_FLAGS(pc) |= PHY_F_CLAUSE73;
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
             if (an_rv == SYS_OK){
                if (phy_an_pbmp & (0x1 << lport)) {
                    /* Clause 73 */
                    an = 1;
                } else {
                    an = 0;
                }
             }
#endif  /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */          

             /*  The default mode of 10G port is force mode with XFI interface.
                 User can change it to SFI interface by calling  bmd_phy_line_interface_set.
             if (!an) {
                 rv = bmd_phy_line_interface_set(unit, lport, BMD_PHY_IF_SFI);
             } */

             an = an ? TRUE : FALSE;
             
        } else if (port_speed[lport] == 2500) {
            an = FALSE;
        } else {
            sal_printf("bcm5340x_sw_init : wrong max speed %d on port %d\n", port_speed[lport], lport);
            return SYS_ERR;
        }

        if (!bmd_phy_external_mode_get(unit, lport)) {
            rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, lport), an);
            if (!SOC_SUCCESS(rv)) {
                sal_printf("bcm5340x_sw_init set phy autoneg %d on lport %d failed\n", an, lport);
            }
        
            /* only serdes phy, set the autoneg need to use speed set to active this port */
            rv = PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, lport), port_speed[lport]);
        } else {
            /* For current design of external PHY driver, AN needs to be set to diabled before calling phy_speed_set */
            rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, lport), FALSE);
            if (!SOC_SUCCESS(rv)) {
                sal_printf("bcm5340x_sw_init set phy autoneg %d on lport %d failed\n", an, lport);
            }
        
            rv = PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, lport), port_speed[lport]);
            if (!SOC_SUCCESS(rv)) {
                sal_printf("bcm5340x_sw_init set phy speed %d on lport %d failed\n", port_speed[lport], lport);
            }
        
            /* Let AN to be enabled always for copper ports */
            rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, lport), TRUE);
            if (!SOC_SUCCESS(rv)) {
                sal_printf("bcm5340x_sw_init set phy autoneg %d on lport %d failed\n", an, lport);
            }
        }
    }
#endif
    /* Init MACs */
    SOC_LPORT_ITER(lport) {
        ml_sw_info.p_mac[lport] = &soc_mac_xl;

        MAC_INIT(ml_sw_info.p_mac[lport], unit, lport);
        /* Probe function should leave port disabled */
        MAC_ENABLE_SET(ml_sw_info.p_mac[lport], unit, lport, FALSE);

        ml_sw_info.link[lport] = PORT_LINK_DOWN;
#if defined(CFG_SWITCH_EEE_INCLUDED)
        ml_sw_info.need_process_for_eee_1s[lport] = FALSE;
#endif /*  CFG_SWITCH_EEE_INCLUDED */
    }

    bcm5346x_load_led_program(unit);

#if CONFIG_METROLITE_EMULATION
    SOC_LPORT_ITER(lport) {
        link_qt[lport] = PORT_LINK_UP;
    }
    bcm5346x_linkscan_task(NULL);
    sal_printf("all ports up!\n");
#endif /* CONFIG_METROLITE_EMULATION */

    /* Register background process for handling link status */
    timer_add(bcm5346x_linkscan_task, NULL, LINKSCAN_INTERVAL);

    return rv;
}


/*
* 28nm:
*     Encode: K0 = MASK & KEY
*             K1 = MASK & ~KEY
*     Decode: KEY = K0
*             MASK = K0 | K1
*     KEY MASK   K0  K1   KEY MASK
*     --------   ------   --------
*      0   0     0   0     0   0
*      1   0     0   0     0   0  =====> info loss
*      0   1     0   1     0   1
*      1   1     1   0     1   1
*/
void
bcm5346x_udf_dm_to_xy(uint32 *dm_entry, uint32 *xy_entry, int data_words, int bit_length)
{
    int i;
    int data_bytes;
    int word_length = (bit_length + 31) / 32;
    uint32 temp_tcam_entry[data_words];

    uint32 key[word_length];
    uint32 mask[word_length];
    uint32 k0[word_length];
    uint32 k1[word_length];
    
	data_bytes = data_words * sizeof(uint32);
	uint32 mask_shift = UDF_TCAM_MASK_MINBIT%32;	//81%32 = 17

		sal_memcpy(temp_tcam_entry, dm_entry, data_bytes);

		key[0] = (temp_tcam_entry[0] >>1) | (temp_tcam_entry[1] << (32-1) );
		key[1] = (temp_tcam_entry[1] >>1) | (temp_tcam_entry[2] << (32-1) );
		key[2] = (temp_tcam_entry[2] >>1) & 0xFFFF;
		
		
		mask[0] = (temp_tcam_entry[2] >>mask_shift) | (temp_tcam_entry[3] << (32-mask_shift) );
		mask[1] = (temp_tcam_entry[3] >>mask_shift) | (temp_tcam_entry[4] << (32-mask_shift) );
		mask[2] = (temp_tcam_entry[4] >>mask_shift) | ((temp_tcam_entry[5] << (32-mask_shift)) /* 160th bit*/ );


		for (i = 0; i < word_length; i++) {
				k0[i] = mask[i] & key[i];
				k1[i] = mask[i] & (~key[i]);
		}
		temp_tcam_entry[0] = (dm_entry[0] & 0x1) | ((k0[0] << 1) );
		temp_tcam_entry[1] = ((k0[0] >>(32-1)) ) | ((k0[1] << 1) );
		temp_tcam_entry[2] = ((k0[1] >>(32-1)) ) | ((k0[2] << 1) ) | (k1[0]<< mask_shift );
		
		temp_tcam_entry[3] = (k1[0]>> (32-mask_shift) ) | (k1[1]<< mask_shift );
		temp_tcam_entry[4] = (k1[1]>> (32-mask_shift) ) | (k1[2]<< mask_shift );
		temp_tcam_entry[5] = (k1[2]>> (32-mask_shift) );

		sal_memcpy(xy_entry, temp_tcam_entry, data_bytes);


}

/*
* 28nm:
*     Encode: K0 = MASK & KEY
*             K1 = MASK & ~KEY
*     Decode: KEY = K0
*             MASK = K0 | K1
*     KEY MASK   K0  K1   KEY MASK
*     --------   ------   --------
*      0   0     0   0     0   0
*      1   0     0   0     0   0  =====> info loss
*      0   1     0   1     0   1
*      1   1     1   0     1   1
*/
void
bcm5346x_udf_xy_to_dm(uint32 *xy_entry, uint32 *dm_entry, int data_words, int bit_length)
{
    int i;
    int data_bytes;
    int word_length = (bit_length + 31) / 32;
    uint32 temp_tcam_entry[data_words];

    uint32 key[word_length];
    uint32 mask[word_length];
    uint32 k0[word_length];
    uint32 k1[word_length];
    
    data_bytes = data_words * sizeof(uint32);
    uint32 mask_shift = UDF_TCAM_MASK_MINBIT%32;	//81%32 = 17
		
    sal_memcpy(temp_tcam_entry, xy_entry, data_bytes);
    k0[0] = (temp_tcam_entry[0] >>1) | (temp_tcam_entry[1] << (32-1) );
    k0[1] = (temp_tcam_entry[1] >>1) | (temp_tcam_entry[2] << (32-1) );
    k0[2] = (temp_tcam_entry[2] >>1) & 0xFFFF;
    
    k1[0] = (temp_tcam_entry[2] >>mask_shift) | (temp_tcam_entry[3] << (32-mask_shift) );
    k1[1] = (temp_tcam_entry[3] >>mask_shift) | (temp_tcam_entry[4] << (32-mask_shift) );
    k1[2] = (temp_tcam_entry[4] >>mask_shift) | ((temp_tcam_entry[5] << (32-mask_shift)) /* 160th bit*/ );
    
    for (i = 0; i < word_length; i++) {
    		key[i] = k0[i];
    		mask[i] = k0[i] | k1[i];
    }
    temp_tcam_entry[0] = (xy_entry[0] & 0x1) | ((key[0] << 1) );
    temp_tcam_entry[1] = ((key[0] >>(32-1)) ) | ((key[1] << 1) );
    temp_tcam_entry[2] = ((key[1] >>(32-1)) ) | ((key[2] << 1) ) | (mask[0]<< mask_shift );
    
    temp_tcam_entry[3] = (mask[0]>> (32-mask_shift) ) | (mask[1]<< mask_shift );
    temp_tcam_entry[4] = (mask[1]>> (32-mask_shift) ) | (mask[2]<< mask_shift );
    temp_tcam_entry[5] = (mask[2]>> (32-mask_shift) );
    
    sal_memcpy(dm_entry, temp_tcam_entry, data_bytes);
		

}

/* Before FP_TCAM insert, encode FP_TCAM entry.
 * dm_entry: Original data
 * xy_entry: encode data
 * data_word: size of one FP_TCAM entry
 * bit_length: full_key/full_mask bit length
 */
void
bcm5346x_dm_to_xy(uint32 *dm_entry, uint32 *xy_entry, int data_words, int bit_length)
{
    int i;
    int data_bytes;
    int word_length = (bit_length + 31) / 32;
    uint32 temp_tcam_entry[data_words];

    uint32 key[word_length];
    uint32 mask[word_length];
    uint32 converted_key, converted_mask;
    uint32 xor_value;

    xor_value = 0xffffffff;
    data_bytes = data_words * sizeof(uint32);
    sal_memcpy(temp_tcam_entry, dm_entry, data_bytes);
    for (i = 0; i < word_length; i++) {
        key[i] = (temp_tcam_entry[i] >> 2) | ((temp_tcam_entry[i+1] & 0x3) << 30);
        mask[i] = (temp_tcam_entry[i+7] >> 14) | ((temp_tcam_entry[i+8] & 0x3fff) << 18);
        if (i == 7) {
            key[i] = (temp_tcam_entry[i] >> 2) & 0xfff;
            mask[i] = (temp_tcam_entry[i+7] >> 14) & 0xfff;
        }
        
    }
    for (i = 0; i < word_length; i++) {
        converted_key = key[i] & mask[i];
        converted_mask = (key[i] | ~mask[i]) ^ xor_value;
        mask[i] = converted_mask;
        key[i] = converted_key;
    }
    if ((bit_length & 0x1f) != 0) {
        mask[i - 1] &= (1 << (bit_length & 0x1f)) - 1;
    }
    for (i = 0; i < data_words; i++) {
        if (i == 0) {
            temp_tcam_entry[i] = (dm_entry[0] & 0x3) | ((key[0] << 2) & 0xfffffffc);
        } else if (i <= 6) {
            temp_tcam_entry[i] = ((key[i-1] >> 30) & 0x3) | ((key[i] << 2) & 0xfffffffc);
        } else if (i == 7) {
            temp_tcam_entry[i] = ((key[i-1] >> 30) & 0x3) | ((key[i] << 2) & 0x3FFC) |
                          (mask[i-7] << 14);
        } else if (i <= 13) {
            temp_tcam_entry[i] = ((mask[i-7-1] >> 18) & 0x3fff) | ((mask[i-7] << 14) & 0xffffC000);
        } else {
            temp_tcam_entry[i] = ((mask[i-7-1] >> 18) & 0x3fff) | ((mask[i-7] << 14) & 0x3FFC000);
        }
    }
    sal_memcpy(xy_entry, temp_tcam_entry, data_bytes);

}

/* Before Decode FP_TCAM entry.
 * dm_entry: Original data
 * xy_entry: encode data
 * data_word: size of one FP_TCAM entry
 * bit_length: full_key/full_mask bit length
 */
void
bcm5346x_xy_to_dm(uint32 *xy_entry, uint32 *dm_entry, int data_words, int bit_length)
{
    int i;
    int data_bytes;
    int word_length = (bit_length + 31) / 32;;
    uint32 temp_tcam_entry[data_words];

    uint32 key[word_length];
    uint32 mask[word_length];
    uint32 xor_value;

    xor_value = 0;
    data_bytes = data_words * sizeof(uint32);
    sal_memcpy(dm_entry, xy_entry, data_bytes);
    sal_memcpy(temp_tcam_entry, xy_entry, data_bytes);

    for (i = 0; i < word_length; i++) {
        key[i] = (temp_tcam_entry[i] >> 2) | ((temp_tcam_entry[i+1] & 0x3) << 30);
        mask[i] = (temp_tcam_entry[i+7] >> 14) | ((temp_tcam_entry[i+8] & 0x3fff) << 18);
        if (i == 7) {
            key[i] = (temp_tcam_entry[i] >> 2) & 0xfff;
            mask[i] = (temp_tcam_entry[i+7] >> 14) & 0xfff;
        }
    }
    for (i = 0; i < word_length; i++) {
        mask[i] = key[i] | (mask[i] ^ xor_value);
    }
    if ((bit_length & 0x1f) != 0) {
        mask[i - 1] &= (1 << (bit_length & 0x1f)) - 1;
    }

    for (i = 0; i < data_words; i++) {
        if (i == 0) {
            temp_tcam_entry[i] = (temp_tcam_entry[0] & 0x3) | ((key[0] << 2) & 0xfffffffc);
        } else if (i <= 6) {
            temp_tcam_entry[i] = ((key[i-1] >> 30) & 0x3) | ((key[i] << 2) & 0xfffffffc);
        } else if (i == 7) {
            temp_tcam_entry[i] = ((key[i-1] >> 30) & 0x3) | ((key[i] << 2) & 0x3FFC) |
                          (mask[i-7] << 14);
        } else if (i <= 13) {
            temp_tcam_entry[i] = ((mask[i-7-1] >> 18) & 0x3fff) | ((mask[i-7] << 14) & 0xffffC000);
        } else {
            temp_tcam_entry[i] = ((mask[i-7-1] >> 18) & 0x3fff) | ((mask[i-7] << 14) & 0x3FFC000);
        }
    }
    sal_memcpy(dm_entry, temp_tcam_entry, data_bytes);
}



void
bcm5346x_loopback_enable(uint8 unit, uint8 port, int loopback_mode)
{
    int rv = 0;
    int link;
    int andone;
    if (loopback_mode == PORT_LOOPBACK_MAC) {
      uint32 flag;
      bcm5346x_handle_link_down(unit, port, TRUE);
      ml_sw_info.loopback[port] = PORT_LOOPBACK_MAC;
      bcm5346x_handle_link_up(unit, port, TRUE, &flag);
      return;
    } else if (loopback_mode == PORT_LOOPBACK_NONE) {
        if (ml_sw_info.loopback[port] != PORT_LOOPBACK_NONE) {
            if (ml_sw_info.loopback[port] == PORT_LOOPBACK_MAC) {
                bcm5346x_handle_link_down(unit, port, TRUE);
            } else if (ml_sw_info.loopback[port] == PORT_LOOPBACK_PHY) {
                rv = PHY_LOOPBACK_SET(BMD_PORT_PHY_CTRL(unit, port), 0);
                rv = PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit, port), &link, &andone);
            }
            ml_sw_info.loopback[port] = PORT_LOOPBACK_NONE;
        }
        return;
    }

    ml_sw_info.loopback[port] = loopback_mode;

    rv = PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit, port), &link, &andone);
    if (rv < 0) {
#if CFG_CONSOLE_ENABLED
        sal_printf("Failed to get link of port %d\n", (int)port);
#endif /* CFG_CONSOLE_ENABLED */
        return;
    }

    if (link) {
        /* Force link change */
        sal_printf("force port %d link change\n", port);
        ml_sw_info.link[port] = PORT_LINK_DOWN;
    }

    if (loopback_mode == PORT_LOOPBACK_PHY) {
        rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, port), 0);
        rv |= PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, port), SOC_PORT_SPEED_MAX(port));
        rv |= PHY_LOOPBACK_SET(BMD_PORT_PHY_CTRL(unit, port), 1);
        if (rv < 0) {
#if CFG_CONSOLE_ENABLED
            sal_printf("Failed to set phy loopback of port %d\n", (int)port);
#endif /* CFG_CONSOLE_ENABLED */
        }
    }

}

soc_switch_t soc_switch_bcm5346x =
{
    bcm5346x_chip_type,
    NULL,
    bcm5346x_port_count,
    NULL,
    NULL,
#if CFG_RXTX_SUPPORT_ENABLED
    bcm5346x_rx_set_handler,
    bcm5346x_rx_fill_buffer,
    bcm5346x_tx,
#endif /* CFG_RXTX_SUPPORT_ENABLED */

    bcm5346x_link_status,
    bcm5346x_chip_revision,
    bcm5346x_reg_get,
    bcm5346x_reg_set,
    bcm5346x_mem_get,
    bcm5346x_mem_set,
#ifdef CFG_SWITCH_VLAN_INCLUDED
    bcm5346x_pvlan_egress_set,
    bcm5346x_pvlan_egress_get,
    bcm5346x_qvlan_port_set,
    bcm5346x_qvlan_port_get,
    bcm5346x_vlan_create,
    bcm5346x_vlan_destroy,
    bcm5346x_vlan_type_set,
    bcm5346x_vlan_reset,
#endif  /* CFG_SWITCH_VLAN_INCLUDED */
    bcm5346x_phy_reg_get,
    bcm5346x_phy_reg_set,
};

