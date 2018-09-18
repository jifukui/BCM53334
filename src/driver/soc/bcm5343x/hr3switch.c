/*
 * $Id: hr3switch.c,v 1.62.2.1 Broadcom SDK $
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

bcm5343x_sw_info_t hr3_sw_info;

/* Flow control is enabled on COSQ1 by default */
#ifdef CFG_FLOW_CONTROL_ENABLED_COSQ_OVERRIDE
#define CFG_FLOW_CONTROL_ENABLED_COSQ CFG_FLOW_CONTROL_ENABLED_COSQ_OVERRIDE
#else
#define CFG_FLOW_CONTROL_ENABLED_COSQ 1
#endif /* CFG_FLOW_CONTROL_ENABLED_COSQ_OVERRIDE */

static const uint32 hr3_tdm_56162_1[34] = {
         10, 18, 26, 34,
         11, 19, 27, 35,
         12, 20, 28, 36,
         13, 21, 29, 37,
         14, 22, 30, 34,
         15, 23, 31, 35,
         16, 24, 32, 36,
         17, 25, 33, 37,
         0, 63
};

static const int p2l_mapping_56162_1[] = {
           0, -1, -1, -1, -1, -1, -1, -1,
          -1, -1,  2,  3,  4,  5,  6,  7,
           8,  9, 10, 11, 12, 13, 14, 15,
          16, 17, 18, 19, 20, 21, 22, 23,
          24, 25, 26, 27, 28, 29, -1, -1,
          -1, -1

};

/* The mapping mode for QSGMII been forced at SGMII mode */
static const int port_speed_max_53434_SGMII[] = {
      -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      -1,  -1,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  -1,  -1,  -1,  -1,
      -1,  -1,  10,  10,  10,  10,  10,  10,
      10,  10,  -1,  -1,  -1,  -1,  -1,  -1,
      -1,  -1
};

static const int port_speed_max_53434[] = {
      -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      -1,  -1,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  -1,  -1,  -1,  -1,  -1,  -1,
      -1,  -1
};

uint8 qtc_interface = CFG_QTC_INTERFACE;

#ifdef CFG_LED_MICROCODE_INCLUDED

/* Left LED : Link,   Right LED : TX/RX activity */
static const uint8 led_embedded_16x1g_1_B0[] = {

    0x12, 0x0A, 0x61, 0xD1, 0x16, 0xD1, 0xDA, 0x21,
    0x70, 0x3A, 0xDA, 0x11, 0x70, 0x16, 0x02, 0x0A,
    0x12, 0x11, 0x61, 0xD1, 0x77, 0x1E, 0x02, 0x1A,
    0x12, 0x21, 0x61, 0xD1, 0x77, 0x1E, 0x28, 0x60,
    0xD0, 0x67, 0x91, 0x75, 0x29, 0x67, 0x70, 0x77,
    0x2B, 0x67, 0x46, 0x06, 0xD0, 0x16, 0xD1, 0xD1,
    0x70, 0x04, 0x75, 0x37, 0x80, 0x77, 0x1E, 0x90,
    0x77, 0x1E, 0x12, 0xD2, 0x85, 0x05, 0xD2, 0x05,
    0x71, 0x44, 0x52, 0x00, 0x3A, 0x60, 0x32, 0x00,
    0x32, 0x01, 0xB7, 0x97, 0x75, 0x55, 0x12, 0xD4,
    0xFE, 0xD0, 0x02, 0x0A, 0x50, 0x12, 0xD4, 0xFE,
    0xD0, 0x95, 0x75, 0x61, 0x85, 0x06, 0xD0, 0x77,
    0x69, 0x16, 0xD2, 0xDA, 0x02, 0x75, 0xB3, 0x77,
    0x99, 0x32, 0x08, 0x97, 0x75, 0xA6, 0x77, 0xB3,
    0x12, 0xA0, 0xFE, 0xD0, 0x15, 0x1A, 0x01, 0x75,
    0x80, 0x12, 0xD4, 0xFE, 0xD0, 0x02, 0x0A, 0x50,
    0x12, 0xD4, 0xFE, 0xD0, 0x95, 0x75, 0x89, 0x85,
    0x57, 0x16, 0xD2, 0xDA, 0x02, 0x75, 0x99, 0x77,
    0xA6, 0x12, 0xA0, 0xFE, 0xD0, 0x15, 0x1A, 0x01,
    0x57, 0x22, 0x01, 0x87, 0x22, 0x01, 0x87, 0x22,
    0x00, 0x87, 0x22, 0x00, 0x87, 0x57, 0x22, 0x00,
    0x87, 0x22, 0x00, 0x87, 0x22, 0x00, 0x87, 0x22,
    0x00, 0x87, 0x57, 0x22, 0x01, 0x87, 0x22, 0x00,
    0x87, 0x22, 0x00, 0x87, 0x22, 0x00, 0x87, 0x57,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

};
/* Right LED : Link,   Left LED : TX/RX activity */
static const uint8 led_embedded_16x1g_2_B0[] = {
    0x12, 0x0A, 0x61, 0xD1, 0x16, 0xD1, 0xDA, 0x21,
    0x70, 0x3A, 0xDA, 0x11, 0x70, 0x16, 0x02, 0x0A,
    0x12, 0x11, 0x61, 0xD1, 0x77, 0x1E, 0x02, 0x1A,
    0x12, 0x21, 0x61, 0xD1, 0x77, 0x1E, 0x28, 0x60,
    0xD0, 0x67, 0x91, 0x75, 0x29, 0x67, 0x70, 0x77,
    0x2B, 0x67, 0x46, 0x06, 0xD0, 0x16, 0xD1, 0xD1,
    0x70, 0x04, 0x75, 0x37, 0x80, 0x77, 0x1E, 0x90,
    0x77, 0x1E, 0x12, 0xD2, 0x85, 0x05, 0xD2, 0x05,
    0x71, 0x44, 0x52, 0x00, 0x3A, 0x60, 0x32, 0x00,
    0x32, 0x01, 0xB7, 0x97, 0x75, 0x55, 0x12, 0xD4,
    0xFE, 0xD0, 0x02, 0x0A, 0x50, 0x12, 0xD4, 0xFE,
    0xD0, 0x95, 0x75, 0x61, 0x85, 0x06, 0xD0, 0x77,
    0x69, 0x16, 0xD2, 0xDA, 0x02, 0x75, 0xB3, 0x77,
    0x99, 0x32, 0x08, 0x97, 0x75, 0xA6, 0x77, 0xB3,
    0x12, 0xA0, 0xFE, 0xD0, 0x15, 0x1A, 0x01, 0x75,
    0x80, 0x12, 0xD4, 0xFE, 0xD0, 0x02, 0x0A, 0x50,
    0x12, 0xD4, 0xFE, 0xD0, 0x95, 0x75, 0x89, 0x85,
    0x57, 0x16, 0xD2, 0xDA, 0x02, 0x75, 0x99, 0x77,
    0xA6, 0x12, 0xA0, 0xFE, 0xD0, 0x15, 0x1A, 0x01,
    0x57, 0x22, 0x01, 0x87, 0x22, 0x01, 0x87, 0x22,
    0x00, 0x87, 0x22, 0x00, 0x87, 0x57, 0x22, 0x00,
    0x87, 0x22, 0x00, 0x87, 0x22, 0x00, 0x87, 0x22,
    0x00, 0x87, 0x57, 0x22, 0x00, 0x87, 0x22, 0x01,
    0x87, 0x22, 0x00, 0x87, 0x22, 0x00, 0x87, 0x57,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* Left LED : Link,   Right LED : TX/RX activity */
static const uint8 led_embedded_16x1g_1[] = {
    0x12, 0x11, 0x61, 0xD1, 0x16, 0xD1, 0xDA, 0x21,
    0x70, 0x3A, 0xDA, 0x0A, 0x70, 0x16, 0x02, 0x11,
    0x12, 0x0A, 0x61, 0xD1, 0x77, 0x1E, 0x02, 0x1A,
    0x12, 0x21, 0x61, 0xD1, 0x77, 0x1E, 0x28, 0x60,
    0xD0, 0x67, 0x91, 0x75, 0x29, 0x67, 0x70, 0x77,
    0x2B, 0x67, 0x46, 0x06, 0xD0, 0x16, 0xD1, 0xD1,
    0x70, 0x04, 0x75, 0x37, 0x80, 0x77, 0x1E, 0x90,
    0x77, 0x1E, 0x12, 0xD2, 0x85, 0x05, 0xD2, 0x05,
    0x71, 0x44, 0x52, 0x00, 0x3A, 0x60, 0x32, 0x00,
    0x32, 0x01, 0xB7, 0x97, 0x75, 0x55, 0x12, 0xD4,
    0xFE, 0xD0, 0x02, 0x0A, 0x50, 0x12, 0xD4, 0xFE,
    0xD0, 0x95, 0x75, 0x61, 0x85, 0x06, 0xD0, 0x77,
    0x69, 0x16, 0xD2, 0xDA, 0x02, 0x75, 0xB3, 0x77,
    0x99, 0x32, 0x08, 0x97, 0x75, 0xA6, 0x77, 0xB3,
    0x12, 0xA0, 0xFE, 0xD0, 0x15, 0x1A, 0x01, 0x75,
    0x80, 0x12, 0xD4, 0xFE, 0xD0, 0x02, 0x0A, 0x50,
    0x12, 0xD4, 0xFE, 0xD0, 0x95, 0x75, 0x89, 0x85,
    0x57, 0x16, 0xD2, 0xDA, 0x02, 0x75, 0x99, 0x77,
    0xA6, 0x12, 0xA0, 0xFE, 0xD0, 0x15, 0x1A, 0x01,
    0x57, 0x22, 0x01, 0x87, 0x22, 0x01, 0x87, 0x22,
    0x00, 0x87, 0x22, 0x00, 0x87, 0x57, 0x22, 0x00,
    0x87, 0x22, 0x00, 0x87, 0x22, 0x00, 0x87, 0x22,
    0x00, 0x87, 0x57, 0x22, 0x01, 0x87, 0x22, 0x00,
    0x87, 0x22, 0x00, 0x87, 0x22, 0x00, 0x87, 0x57,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,


};
/* Right LED : Link,   Left LED : TX/RX activity */
static const uint8 led_embedded_16x1g_2[] = {
    0x12, 0x11, 0x61, 0xD1, 0x16, 0xD1, 0xDA, 0x21,
    0x70, 0x3A, 0xDA, 0x0A, 0x70, 0x16, 0x02, 0x11,
    0x12, 0x0A, 0x61, 0xD1, 0x77, 0x1E, 0x02, 0x1A,
    0x12, 0x21, 0x61, 0xD1, 0x77, 0x1E, 0x28, 0x60,
    0xD0, 0x67, 0x91, 0x75, 0x29, 0x67, 0x70, 0x77,
    0x2B, 0x67, 0x46, 0x06, 0xD0, 0x16, 0xD1, 0xD1,
    0x70, 0x04, 0x75, 0x37, 0x80, 0x77, 0x1E, 0x90,
    0x77, 0x1E, 0x12, 0xD2, 0x85, 0x05, 0xD2, 0x05,
    0x71, 0x44, 0x52, 0x00, 0x3A, 0x60, 0x32, 0x00,
    0x32, 0x01, 0xB7, 0x97, 0x75, 0x55, 0x12, 0xD4,
    0xFE, 0xD0, 0x02, 0x0A, 0x50, 0x12, 0xD4, 0xFE,
    0xD0, 0x95, 0x75, 0x61, 0x85, 0x06, 0xD0, 0x77,
    0x69, 0x16, 0xD2, 0xDA, 0x02, 0x75, 0xB3, 0x77,
    0x99, 0x32, 0x08, 0x97, 0x75, 0xA6, 0x77, 0xB3,
    0x12, 0xA0, 0xFE, 0xD0, 0x15, 0x1A, 0x01, 0x75,
    0x80, 0x12, 0xD4, 0xFE, 0xD0, 0x02, 0x0A, 0x50,
    0x12, 0xD4, 0xFE, 0xD0, 0x95, 0x75, 0x89, 0x85,
    0x57, 0x16, 0xD2, 0xDA, 0x02, 0x75, 0x99, 0x77,
    0xA6, 0x12, 0xA0, 0xFE, 0xD0, 0x15, 0x1A, 0x01,
    0x57, 0x22, 0x01, 0x87, 0x22, 0x01, 0x87, 0x22,
    0x00, 0x87, 0x22, 0x00, 0x87, 0x57, 0x22, 0x00,
    0x87, 0x22, 0x00, 0x87, 0x22, 0x00, 0x87, 0x22,
    0x00, 0x87, 0x57, 0x22, 0x00, 0x87, 0x22, 0x01,
    0x87, 0x22, 0x00, 0x87, 0x22, 0x00, 0x87, 0x57,
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

#if CONFIG_HURRICANE3_EMULATION
int link_qt[BCM5343X_LPORT_MAX+1];

void
bcm5343x_link_change(uint8 port)
{
    link_qt[port] ^= 1;
}
#endif
static void
bcm5343x_handle_link_up(uint8 unit, uint8 lport, int changed, uint32 *flags)
{
    BOOL   tx_pause, rx_pause;
    int    duplex;
    uint32 speed;
    int an = 0;
    int rv = 0;
    phy_ctrl_t *qtc_pc = NULL;
    uint32 val;
    
    if (1 == changed) {
        /* Port changes to link up from link down */
#if CONFIG_HURRICANE3_EMULATION
            speed = 1000;
            duplex = tx_pause = rx_pause = TRUE;
#else
        if (hr3_sw_info.loopback[lport] == PORT_LOOPBACK_MAC) {
            /* Force link up in MAC loopback mode */
            speed = SOC_PORT_SPEED_MAX(lport);
            duplex = TRUE;
            an = tx_pause = rx_pause = FALSE;
        } else {
            rv = 0;
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
#endif /* CONFIG_HURRICANE3_EMULATION */

        soc_phyctrl_notify(BMD_PORT_PHY_CTRL(unit, lport), PhyEvent_Speed, speed);
        soc_phyctrl_notify(BMD_PORT_PHY_CTRL(unit, lport), PhyEvent_Duplex, duplex);

        /* SDK-80948 patch : Unexpected packet shown during link down */
        if (BMD_PORT_PHY_CTRL(unit,lport)->next != NULL && BMD_PORT_PHY_CTRL(unit,lport)->next->drv != NULL &&
            BMD_PORT_PHY_CTRL(unit,lport)->next->drv->drv_name != NULL && 
            sal_strcmp(BMD_PORT_PHY_CTRL(unit, lport)->next->drv->drv_name, "bcmi_qtc_serdes") == 0) {

            qtc_pc = BMD_PORT_PHY_CTRL(unit,lport)->next;

        } 
        if (BMD_PORT_PHY_CTRL(unit,lport) != NULL && BMD_PORT_PHY_CTRL(unit,lport)->drv != NULL &&
            BMD_PORT_PHY_CTRL(unit,lport)->drv->drv_name != NULL && 
            sal_strcmp(BMD_PORT_PHY_CTRL(unit, lport)->drv->drv_name, "bcmi_qtc_serdes") == 0) {

            qtc_pc = BMD_PORT_PHY_CTRL(unit,lport);

        }
        if (qtc_pc != NULL) {
            /* According to experiment 25ms is good enough for link up, too get the QTC link status too early will led to register access fail */
            sal_usleep(25000);            
        }            


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

        MAC_SPEED_SET(hr3_sw_info.p_mac[lport], unit, lport, speed);

        MAC_DUPLEX_SET(hr3_sw_info.p_mac[lport], unit, lport, duplex);

        /* Interface? */

        MAC_PAUSE_SET(hr3_sw_info.p_mac[lport], unit, lport, tx_pause, rx_pause);

        if (hr3_sw_info.loopback[lport] == PORT_LOOPBACK_MAC) {
            MAC_LOOPBACK_SET(hr3_sw_info.p_mac[lport], unit, lport, TRUE);
        } else {
            MAC_LOOPBACK_SET(hr3_sw_info.p_mac[lport], unit, lport, FALSE);
        }                        

        /* Enable the MAC. */
        MAC_ENABLE_SET(hr3_sw_info.p_mac[lport], unit, lport, TRUE);

        if (qtc_pc != NULL) {
             /* 
                SDK-80948 patch : Unexpected packet shown during link down
                soc_pgw_rx_fifo_reset(unit, port, FALSE)
              */ 
            if (SOC_PORT_L2P_MAPPING(lport) >= 18 && SOC_PORT_L2P_MAPPING(lport) < 34)     {
                bcm5343x_reg_get(unit, PGW_GE1_BLOCK_ID, R_PGW_GE_RXFIFO_SOFT_RESET, &val);
                val = val & ~(1 <<    (SOC_PORT_L2P_MAPPING(lport) - 18));             
                bcm5343x_reg_set(unit, PGW_GE1_BLOCK_ID, R_PGW_GE_RXFIFO_SOFT_RESET, val);
            } else if (SOC_PORT_L2P_MAPPING(lport) >= 2 && SOC_PORT_L2P_MAPPING(lport) < 18)  {
                bcm5343x_reg_get(unit, PGW_GE0_BLOCK_ID, R_PGW_GE_RXFIFO_SOFT_RESET, &val);
                val = val & ~(1 <<    (SOC_PORT_L2P_MAPPING(lport) - 2));              
                bcm5343x_reg_set(unit, PGW_GE0_BLOCK_ID, R_PGW_GE_RXFIFO_SOFT_RESET, val);
            }

        }
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
                bcm5343x_port_eee_enable_get(unit, lport, &eee_state);
            }

            if (eee_state == TRUE) {
                /* Enable EEE in UMAC_EEE_CTRL register after one second
                 * if EEE is enabled in S/W database
                 */
                hr3_sw_info.link_up_time[lport] = sal_get_ticks();
                hr3_sw_info.need_process_for_eee_1s[lport] = TRUE;
            }
        }
#endif /* CFG_SWITCH_EEE_INCLUDED */

        /* If everything has been completed */
        hr3_sw_info.link[lport] = PORT_LINK_UP;
    } else {
        /* Port stays in link up state */
#if defined(CFG_SWITCH_EEE_INCLUDED)
        int eee_support;
        /* EEE one second delay for link up timer check */
        if ((hr3_sw_info.need_process_for_eee_1s[lport]) &&
            (SAL_TIME_EXPIRED(hr3_sw_info.link_up_time[lport], 1000))) {
#if CFG_CONSOLE_ENABLED
            SAL_DEBUGF(("EEE : enable eee for port %d\n", lport));
#endif /* CFG_CONSOLE_ENABLED */
            bmd_phy_eee_get(unit,lport, &eee_support);
            if (eee_support) {
                bcm5343x_port_eee_enable_set(unit, lport, TRUE, FALSE);
            } else {
                bcm5343x_port_eee_enable_set(unit, lport, FALSE, FALSE);
            }
            hr3_sw_info.need_process_for_eee_1s[lport] = FALSE;
        }
#endif /* CFG_SWITCH_EEE_INCLUDED */
    }

#ifdef CFG_LED_MICROCODE_INCLUDED

        if ((changed == 1) && (hr3_sw_info.link[lport] == PORT_LINK_UP)) {
            /* Update LED status */
            val = READCSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)));
            val |= 0x01;
            WRITECSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)), val);
        }
#endif /* CFG_LED_MICROCODE_INCLUDED */

}

void
bcm5343x_handle_link_down(uint8 unit, uint8 lport, int changed)
{

    phy_ctrl_t *qtc_pc = NULL;
    uint32 val;

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
        hr3_sw_info.link[lport] = PORT_LINK_DOWN;
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
        MAC_ENABLE_SET(hr3_sw_info.p_mac[lport], unit, lport, FALSE);
        /* SDK-80948 patch : Unexpected packet shown during link down */
        if (BMD_PORT_PHY_CTRL(unit,lport)->next != NULL && BMD_PORT_PHY_CTRL(unit,lport)->next->drv != NULL &&
            BMD_PORT_PHY_CTRL(unit,lport)->next->drv->drv_name != NULL && 
            sal_strcmp(BMD_PORT_PHY_CTRL(unit, lport)->next->drv->drv_name, "bcmi_qtc_serdes") == 0) {

            qtc_pc = BMD_PORT_PHY_CTRL(unit,lport)->next;

        } 
        if (BMD_PORT_PHY_CTRL(unit,lport) != NULL && BMD_PORT_PHY_CTRL(unit,lport)->drv != NULL &&
            BMD_PORT_PHY_CTRL(unit,lport)->drv->drv_name != NULL && 
            sal_strcmp(BMD_PORT_PHY_CTRL(unit, lport)->drv->drv_name, "bcmi_qtc_serdes") == 0) {

            qtc_pc = BMD_PORT_PHY_CTRL(unit,lport);

        }

        if (qtc_pc != NULL) {
        /* 
              SDK-80948 patch : Unexpected packet shown during link down 
              soc_pgw_rx_fifo_reset(unit, port, TRUE)
              */ 
           if (SOC_PORT_L2P_MAPPING(lport) >= 18 && SOC_PORT_L2P_MAPPING(lport) < 34)    {
               bcm5343x_reg_get(unit,    PGW_GE1_BLOCK_ID, R_PGW_GE_RXFIFO_SOFT_RESET, &val);
                 val = val | (1 <<  (SOC_PORT_L2P_MAPPING(lport) - 18));             
                 bcm5343x_reg_set(unit,    PGW_GE1_BLOCK_ID, R_PGW_GE_RXFIFO_SOFT_RESET, val);
           } else if (SOC_PORT_L2P_MAPPING(lport) >= 2 && SOC_PORT_L2P_MAPPING(lport) < 18)  {
                  bcm5343x_reg_get(unit,    PGW_GE0_BLOCK_ID, R_PGW_GE_RXFIFO_SOFT_RESET, &val);
                val = val | (1 <<  (SOC_PORT_L2P_MAPPING(lport) - 2));                
                 bcm5343x_reg_set(unit,    PGW_GE0_BLOCK_ID, R_PGW_GE_RXFIFO_SOFT_RESET, val);
           }
        }
#if defined(CFG_SWITCH_EEE_INCLUDED)
        bcm5343x_port_eee_enable_get(unit, lport, &eee_state);
        if (eee_state == TRUE) {
            /* Disable EEE in UMAC_EEE_CTRL register if EEE is enabled in S/W database */
#if CFG_CONSOLE_ENABLED
            SAL_DEBUGF(("EEE : disable eee for lport %d\n", lport));
#endif /* CFG_CONSOLE_ENABLED */
            bcm5343x_port_eee_enable_set(unit, lport, FALSE, FALSE);
            hr3_sw_info.need_process_for_eee_1s[lport] = FALSE;
        }
#endif /* CFG_SWITCH_EEE_INCLUDED */

        hr3_sw_info.link[lport] = PORT_LINK_DOWN;
    } else {
        /* Port stays in link down state */
    }
#endif /* CFG_LOOPBACK_TEST_ENABLED */

#ifdef CFG_LED_MICROCODE_INCLUDED
        if ((changed == 1) && (hr3_sw_info.link[lport] == PORT_LINK_DOWN)) {
            /* Update LED status */
            val = READCSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)));
            val &= (~0x01);
            WRITECSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)), val);
        }
#endif /* CFG_LED_MICROCODE_INCLUDED */

}

/*
 *  Function : bcm5343x_linkscan_task
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
bcm5343x_linkscan_task(void *param)
{
    uint8 unit = 0, lport;
    uint32 flags;
    int link;

    if (board_linkscan_disable) {
        return;
    }

    SOC_LPORT_ITER(lport) {
#if CONFIG_HURRICANE3_EMULATION
        link = (int)link_qt[lport];
#else
        if (hr3_sw_info.loopback[lport] == PORT_LOOPBACK_MAC) {
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
#endif /* CONFIG_HURRICANE3_EMULATION */

        if (link == PORT_LINK_UP) {
            /* Link up */
            flags = 0;
            bcm5343x_handle_link_up(unit, lport,
                (hr3_sw_info.link[lport] == PORT_LINK_DOWN) ? TRUE : FALSE, &flags);
        } else {
            bcm5343x_handle_link_down(unit, lport,
                (hr3_sw_info.link[lport] == PORT_LINK_UP) ? TRUE : FALSE);
        }
    }
}

static void
soc_reset(uint8 unit)
{
    uint32 val, to_usec;
#if CONFIG_HURRICANE3_EMULATION
    to_usec = 250000;
#else
    to_usec = 10000;
#endif /* CONFIG_HURRICANE3_EMULATION */

    WRITECSR(CMIC_SBUS_RING_MAP_0_7, 0x00110000); /* block 7  - 0 */
    WRITECSR(CMIC_SBUS_RING_MAP_8_15, 0x00430000); /* block 15 - 8 */
    WRITECSR(CMIC_SBUS_RING_MAP_16_23, 0x00005064); /* block 23 - 16 */
    WRITECSR(CMIC_SBUS_RING_MAP_24_31, 0x00000000); /* block 31 - 24 */
    WRITECSR(CMIC_SBUS_RING_MAP_32_39, 0x77772222); /* block 39 - 32 */
    WRITECSR(CMIC_SBUS_RING_MAP_40_47, 0x00000000); /* block 37 - 40 */

    WRITECSR(CMIC_SBUS_TIMEOUT, 0x7d0);
    sal_usleep(to_usec);

    /* Select LCPLL0 as external PHY reference clock */
    {
        /* Enable GPIO3 as output driver */
        val = READCSR(CMIC_GP_OUT_EN);
        /* OUT_ENABLEf [Bit 3:0] */
        val |= (0x1 << 3);
        WRITECSR(CMIC_GP_OUT_EN, val);

        /* Pull GPIO3 low to reset the ext. PHY */
        val = READCSR(CMIC_GP_DATA_OUT);
        /* DATA_OUTf [Bit 3:0] */
        val &= ~(0x1 << 3);
        WRITECSR(CMIC_GP_DATA_OUT, val);

        bcm5343x_reg_get(unit, R_TOP_XG_PLL0_CTRL_6, &val);
        /* MSC_CTRLf [Bit 15:0] */
        val &= ~0xFFFF;
        val |= 0x71a2;
        bcm5343x_reg_set(unit, R_TOP_XG_PLL0_CTRL_6, val);

        /* Pull  GPIO3 high to leave the reset state */
        val = READCSR(CMIC_GP_DATA_OUT);
        /* DATA_OUTf [Bit 3:0] */
        val |= (0x1 << 3);
        WRITECSR(CMIC_GP_DATA_OUT, val);
    }

    /* don't disable QTC for foxhound2 */
    bcm5343x_reg_get(unit, R_PGW_CTRL_0, &val);
    val &= ~0x3;
    bcm5343x_reg_set(unit, R_PGW_CTRL_0, val);

    /*
     * Bring port blocks out of reset:
     * TOP_GXP0_RST_L [Bit 5], TOP_GXP1_RST_L [Bit 6] and TOP_QGPHY_RST_Lf [Bit 19~16]
     */
    bcm5343x_reg_get(unit, R_TOP_SOFT_RESET_REG, &val);
    val |= 0xF0078;
    bcm5343x_reg_set(unit, R_TOP_SOFT_RESET_REG, val);
    sal_usleep(to_usec);

    /*
     * Bring network sync out of reset
     * TOP_TS_RST_Lf [Bit 15]
     */
    bcm5343x_reg_get(unit, R_TOP_SOFT_RESET_REG, &val);
    val |= 0x8000;
    bcm5343x_reg_set(unit, R_TOP_SOFT_RESET_REG, val);

    /*
         * Bring network sync PLL out of reset
         * TOP_TS_PLL_RST_Lf [Bit 8] and then TOP_TS_PLL_POST_RST_Lf [Bit 9]
       */
    bcm5343x_reg_get(unit, R_TOP_SOFT_RESET_REG_2, &val);
    val |= 0x100;
    bcm5343x_reg_set(unit, R_TOP_SOFT_RESET_REG_2, val);
    val |= 0x200;
    bcm5343x_reg_set(unit, R_TOP_SOFT_RESET_REG_2, val);

    sal_usleep(to_usec);

    /*
     * Bring IP, EP, and MMU blocks out of reset
     * TOP_IP_RST_L [Bit 0], TOP_EP_RST_Lf [Bit 1], TOP_MMU_RST_Lf [Bit 2]
     */
    bcm5343x_reg_get(unit, R_TOP_SOFT_RESET_REG, &val);
    val |= 0x7;
    bcm5343x_reg_set(unit, R_TOP_SOFT_RESET_REG, val);
    sal_usleep(to_usec);

    /* XGXS RESET */
    {
        /*
        * Reference clock selection: REFIN_ENf [Bit 5]
        */
        bcm5343x_reg_get(unit, PMQ1_BLOCK_ID, R_GPORT_XGXS0_CTRL_REG, &val);
        val |= (0x1 << 5);
        bcm5343x_reg_set(unit, PMQ1_BLOCK_ID, R_GPORT_XGXS0_CTRL_REG, val);
        sal_usleep(1100);

        /* Deassert power down [Bit 6]*/
        val &= ~(0x1 << 6);
        bcm5343x_reg_set(unit, PMQ1_BLOCK_ID, R_GPORT_XGXS0_CTRL_REG, val);
        sal_usleep(1100);

        /* Reset XGXS : RstB_HW[Bit 2] */
        val &= ~(0x1 << 2);
        bcm5343x_reg_set(unit, PMQ1_BLOCK_ID, R_GPORT_XGXS0_CTRL_REG, val);
        sal_usleep(1100);

        /* Bring XGXS out of reset: RstB_HW[Bit 2] */
        val |= (0x1 << 2);
        bcm5343x_reg_set(unit, PMQ1_BLOCK_ID, R_GPORT_XGXS0_CTRL_REG, val);
        sal_usleep(1100);

        /* Bring reference clock out of reset : RSTB_REFCLK [Bit 3] */
        val |= (0x1 << 3);
        bcm5343x_reg_set(unit, PMQ1_BLOCK_ID, R_GPORT_XGXS0_CTRL_REG, val);
        sal_usleep(1100);

        /* Activate clocks : RSTB_PLL [Bit 1] */
        val |= (0x1 << 1);
        bcm5343x_reg_set(unit, PMQ1_BLOCK_ID, R_GPORT_XGXS0_CTRL_REG, val);
        sal_usleep(1100);
    }
}

soc_chip_type_t
bcm5343x_chip_type(void)
{
    return SOC_TYPE_SWITCH_XGS;
}

uint8
bcm5343x_port_count(uint8 unit)
{
    if (unit > 0) {
        return 0;
    }
    return SOC_PORT_COUNT(unit);
}

sys_error_t
bcm5343x_link_status(uint8 unit, uint8 lport, BOOL *link)
{
    if (link == NULL || unit > 0 || lport > BCM5343X_LPORT_MAX) {
        return SYS_ERR_PARAMETER;
    }

    *link = hr3_sw_info.link[lport];

    return SYS_OK;
}

static int
bcm5343x_sw_op(uint8 unit,
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

    if (op != SC_OP_RD_MEM_CMD) {
        msg_hdr |= V_SMHDR_DATA_LEN(len*4);
    }

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
bcm5343x_phy_reg_get(uint8 unit, uint8 lport,
                           uint16 reg_addr, uint16 *p_value)
{
    return SYS_OK;
}

sys_error_t
bcm5343x_phy_reg_set(uint8 unit, uint8 lport,
                           uint16 reg_addr, uint16 value)
{
    return SYS_OK;
}

sys_error_t
bcm5343x_reg_get(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *val)
{
    return bcm5343x_sw_op(unit, SC_OP_RD_REG_CMD, block_id, addr, val, 1);
}

sys_error_t
bcm5343x_reg_set(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 val)
{
    return bcm5343x_sw_op(unit, SC_OP_WR_REG_CMD, block_id, addr, &val, 1);
}

sys_error_t
bcm5343x_reg64_get(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *val,
                 int len)
{
    return bcm5343x_sw_op(unit, SC_OP_RD_REG_CMD, block_id, addr, val, len);
}

sys_error_t
bcm5343x_reg64_set(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *buf,
                 int len)
{
    return bcm5343x_sw_op(unit, SC_OP_WR_REG_CMD, block_id, addr, buf, len);
}

sys_error_t
bcm5343x_mem_get(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *buf,
                 int len)
{
    return bcm5343x_sw_op(unit, SC_OP_RD_MEM_CMD, block_id, addr, buf, len);
}

sys_error_t
bcm5343x_mem_set(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *buf,
                 int len)
{
    return bcm5343x_sw_op(unit, SC_OP_WR_MEM_CMD, block_id, addr, buf, len);
}

sys_error_t
bcm5343x_chip_revision(uint8 unit, uint16 *dev, uint16 *rev)
{
    uint32 val;

    if (unit > 0) {
        return -1;
    }

    val = READCSR(CMIC_DEV_REV_ID);
    hr3_sw_info.devid = val & 0xFFFF;
    hr3_sw_info.revid = val >> 16;

    return 0;
}

static void
bcm5343x_load_led_program(uint8 unit)
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
    /* The LED scanning out order is
     *  PGW_XL1(41->38) -> PGW_XL0(37->34) -> PGW_GE1(18->33) -> PGW_GE0(2->17)
     *  If PM4x10 is ON, PGW_XL will output 4 bytes LED status to the LED chain.
     *  If PM4x10 is OFF, PGW_XL will output 1 byte to the LED chain.
     *  PGW_GE always outputs 16 bytes.
     */


   uint32 port_remap[9] = { (0)  + (0 << 6) + (18 << 12) + (19 << 18), 
                      (20) + (21 << 6) + (22 << 12) + (23 << 18),
                      (24) + (25 << 6) + (26 << 12) + (27 << 18),
                      (28) + (29 << 6) + (30 << 12) + (31 << 18),            
                      (32) + (33 << 6) + (2 << 12) + (3 << 18),
                      (4)  + (5 << 6) + (6 << 12) + (7 << 18),
                      (8)  + (9 << 6) + (10 << 12) + (11 << 18),                                                
                      (12)  + (13 << 6) + (14 << 12) + (15 << 18),
                      (16)  + (17 << 6) + (0 << 12) + (0 << 18),
    };

    for (i = 0, addr = CMIC_LEDUP0_PORT_ORDER_REMAP_0_3; i < (sizeof(port_remap)/4); i++, addr += 4) {
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
         if (hr3_sw_info.revid != 0x01) {
              led_program = led_embedded_16x1g_2_B0;
              led_code_size = sizeof(led_embedded_16x1g_2_B0);
         } else {
              led_program = led_embedded_16x1g_2;
              led_code_size = sizeof(led_embedded_16x1g_2);
         }
    } else if ((led_option == 3) && byte_count) {
         sal_printf("Vendor Config : Load customer LED ucdoe with length %d.\n", byte_count);
         led_program = led_program_3;
         led_code_size = sizeof(led_program_3);
    } else {
        if (hr3_sw_info.revid != 0x01) {
             led_program = led_embedded_16x1g_1_B0;
             led_code_size = sizeof(led_embedded_16x1g_1_B0);
        } else {
            led_program = led_embedded_16x1g_1;
            led_code_size = sizeof(led_embedded_16x1g_1);
        }
    }


   
#define LED_RAM_SIZE     0x100

    for (offset = 0; offset < LED_RAM_SIZE; offset++) {
        WRITECSR(CMIC_LEDUP_PROGRAM_RAM_D(offset),
                      (offset >= led_code_size) ? 0 : *(led_program + offset));

        WRITECSR(CMIC_LEDUP_DATA_RAM_D(offset), 0);
    }
#if (CFG_LED_MICROCODE_INCLUDED == 2)
    /* led_current_sel=0x3, led_faultdet_pdwn_l=0x3f */
    /* led_ser2par_sel=1 */
    bcm5343x_reg_set(0, R_TOP_PARALLEL_LED_CTRL, 0x3FB00);
#endif
    /* enable LED processor */
    val = READCSR(CMIC_LEDUP0_CTRL);
    val |= 0x1;
    WRITECSR(CMIC_LEDUP0_CTRL, val);
#endif /* CFG_LED_MICROCODE_INCLUDED */
}

sys_error_t
bcm5343x_l2_op(uint8 unit,
               l2x_entry_t *entry,
               uint8 op_code
               )

{
    uint32 l2_entry[4];
    uint32 msg_hdr[2] = {0x90500800, 0x1c000000};
    uint32 ctrl;
    int i;

    l2_entry[0] = ((entry->vlan_id << 3) |
                   (entry->mac_addr[5] << 15) |
                   (entry->mac_addr[4] << 23) |
                   (entry->mac_addr[3] << 31));

    l2_entry[1] = (((entry->mac_addr[3] >> 1) & 0x0000007F) |
                   (entry->mac_addr[2] << 7) |
                   (entry->mac_addr[1] << 15) |
                   (entry->mac_addr[0] << 23)
                   );

    l2_entry[2] = ((entry->port) & 0x0000003F);
    /* set static and valid bit */
    l2_entry[3] = (1 << 6) | (1 << 4);

    if (op_code == SC_OP_L2_DEL_CMD) {
         msg_hdr[0] = 0x98500800;
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

#define JUMBO_FRM_SIZE (9216)

static void
enable_jumbo_frame(uint8 unit)
{
    int lport;
    SOC_LPORT_ITER(lport) {
        bcm5343x_reg_set(unit, SOC_PORT_BLOCK(lport),
            R_FRM_LENGTH(SOC_PORT_BLOCK_INDEX(lport)), JUMBO_FRM_SIZE);
    }
}

static void
soc_pipe_mem_clear(uint8 unit)
{
    uint32 val;
    /*
     * Reset the IPIPE and EPIPE block
     */
    bcm5343x_reg_set(unit, R_ING_HW_RESET_CONTROL_1, 0x0);
    /*
     * Set COUNT[Bit 15-0] to # entries in largest IPIPE table, L2_ENTRYm 0x4000
     * RESET_ALL [Bit 16] = 1, VALID [Bit 17] = 1
     */
    bcm5343x_reg_set(unit, R_ING_HW_RESET_CONTROL_2, 0x34000);

    bcm5343x_reg_set(unit, R_EGR_HW_RESET_CONTROL_0, 0x0);
    /*
     * Set COUNT[Bit 15-0] to # entries in largest EPIPE table, EGR_VLAN 0x1000
     * RESET_ALL [Bit 16] = 1, VALID [Bit 17] = 1
     */
    bcm5343x_reg_set(unit, R_EGR_HW_RESET_CONTROL_1, 0x31000);

    /* Wait for IPIPE memory initialization done. */
    do {
        /* Polling DONE[Bit 18] */
        bcm5343x_reg_get(unit, R_ING_HW_RESET_CONTROL_2, &val);
        if (val & (0x1 << 18)) {
            break;
        }
        
        /*  soc_cm_debug(DK_WARN, "unit %d : ING_HW_RESET timeout\n", unit); */
    } while (1);

    /* Wait for EPIPE memory initialization done. */
    do {
        /* Polling DONE[Bit 18] */
        bcm5343x_reg_get(unit, R_EGR_HW_RESET_CONTROL_1, &val);
        if (val & (0x1 << 18)) {
            break;
        }

        
        /*  soc_cm_debug(DK_WARN, "unit %d : EGR_HW_RESET timeout\n", unit); */
    } while (1);

    bcm5343x_reg_set(unit, R_ING_HW_RESET_CONTROL_2, 0x0);
    bcm5343x_reg_set(unit, R_EGR_HW_RESET_CONTROL_1, 0x0);

    
}

static void
soc_port_block_info_get(uint8 unit, uint8 pport, int *block_type, int *block_idx, int *port_idx)
{
    *block_type = PORT_BLOCK_TYPE_GXPORT;
    if ((pport >= PHY_GPORT3_BASE) && (pport <= BCM5343X_PORT_MAX)) {
        *block_idx = GPORT3_BLOCK_ID;
        *port_idx = (pport - PHY_GPORT3_BASE) & 0x7;
    } else if (pport >= PHY_GPORT2_BASE) {
        *block_idx = GPORT2_BLOCK_ID;
        *port_idx = (pport - PHY_GPORT2_BASE) & 0x7;
    } else if (pport >= PHY_GPORT1_BASE) {
        *block_idx = GPORT1_BLOCK_ID;
        *port_idx = (pport - PHY_GPORT1_BASE) & 0x7;
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
    uint32 val;
    int block_start_port;

    p2l_mapping = p2l_mapping_56162_1;
    if (qtc_interface == QTC_INTERFACE_QSGMII) {
        speed_max = port_speed_max_53434;
    } else {
         /* SGMII mode or FIBER mode */
        speed_max = port_speed_max_53434_SGMII;
    }
    
    for (i = 0 ; i <= BCM5343X_LPORT_MAX ; i++) {
        hr3_sw_info.port_l2p_mapping[i] = -1;
    }

    for (i = 0; i <= BCM5343X_PORT_MAX ; i++) {
        hr3_sw_info.port_p2l_mapping[i] = p2l_mapping[i];
        if (p2l_mapping[i] != -1) {
            if (speed_max[i] != -1) {
                hr3_sw_info.port_l2p_mapping[p2l_mapping[i]] = i;
            } else if (i == 0) {
                hr3_sw_info.port_l2p_mapping[p2l_mapping[i]] = 0;
            } else {
                hr3_sw_info.port_l2p_mapping[p2l_mapping[i]] = -1;
            }
        }
    }

    /* Ingress physical to logical port mapping */
    for (i = 0; i <= BCM5343X_PORT_MAX; i++) {
        val = (p2l_mapping[i] == -1) ? 0x30: p2l_mapping[i];
        bcm5343x_mem_set(unit, M_ING_PHYSICAL_TO_LOGICAL_PORT_NUMBER_MAPPING_TABLE(i), &val, 1);
        if (speed_max[i] != -1) {
            if (speed_max[i] < 100) {
                hr3_sw_info.port_speed_max[p2l_mapping[i]] = 1000;
            } else {
                hr3_sw_info.port_speed_max[p2l_mapping[i]] = 10000;
            }
        }
    }

    /* Egress logical to physical port mapping, needs a way for maximum logical port? */
    for (i = 0; i <= BCM5343X_LPORT_MAX; i++) {
        val = (hr3_sw_info.port_l2p_mapping[i] == -1) ? 0x3F : hr3_sw_info.port_l2p_mapping[i];
        bcm5343x_reg_set(unit, R_EGR_LOGICAL_TO_PHYSICAL_PORT_NUMBER_MAPPING(i), val);
        /* MMU logical to physical port mapping
         * (Here, Same as Egress logical to physical port mapping)
         */
        if (val != 0x3F) {
            bcm5343x_reg_set(unit, R_LOG_TO_PHY_PORT_MAPPING(i), val);
        }
    }

    SOC_LPORT_ITER(i) {
        soc_port_block_info_get(unit, SOC_PORT_L2P_MAPPING(i),
                                &SOC_PORT_BLOCK_TYPE(i),
                                &SOC_PORT_BLOCK(i), &SOC_PORT_BLOCK_INDEX(i));

        BCM5343X_ALL_PORTS_MASK |= (0x1 << i);
    }

    SOC_LPORT_ITER(i) {
        block_start_port = SOC_PORT_L2P_MAPPING(i) - SOC_PORT_BLOCK_INDEX(i);
        SOC_PORT_LANE_NUMBER(i) = 4;
        if (p2l_mapping[block_start_port + 1] != -1) {
            if (qtc_interface == QTC_INTERFACE_QSGMII) {
                SOC_PORT_LANE_NUMBER(i) = 1;
            } else {
                 /* SGMII mode or FIBER mode */
                SOC_PORT_LANE_NUMBER(i) = 4;
            }
        } else if (p2l_mapping[block_start_port + 2] != -1) {
                SOC_PORT_LANE_NUMBER(i) = 2;
        }
    }
}

static void
soc_misc_init(uint8 unit)
{
    int i, lport;
    uint32 val;

    /* HR3-1325 : enable GPHY1 and GPHY2 */
    bcm5343x_reg_get(unit, R_TOP_MISC_STATUS, &val);
    val &= ~(0x3 << 5);
    bcm5343x_reg_set(unit, R_TOP_MISC_STATUS, val);
    
    soc_pipe_mem_clear(unit);


    

    /* GMAC init */
    SOC_LPORT_ITER(i) {
        if (IS_GX_PORT(i) && (SOC_PORT_BLOCK_INDEX(i) == 0)) {
            /* Clear counter and enable gport */
            bcm5343x_reg_set(unit, SOC_PORT_BLOCK(i), R_GPORT_CONFIG, 0x3);
        }
    }

    SOC_LPORT_ITER(i) {
        if (IS_GX_PORT(i) && (SOC_PORT_BLOCK_INDEX(i) == 0)) {
            /* Enable gport */
            bcm5343x_reg_set(unit, SOC_PORT_BLOCK(i), R_GPORT_CONFIG, 0x1);
        }
    }

#if !CONFIG_HURRICANE3_ROMCODE
    /* Metering Clock [Bit 5] */
    bcm5343x_reg_get(unit, R_MISCCONFIG, &val);
    val |= 0x20;
    bcm5343x_reg_set(unit, R_MISCCONFIG, val);

    /* Enable dual hash on L2 and L3 tables */
    /* HASH_SELECT[Bit3:1] = FB_HASH_CRC32_LOWER(2), INSERT_LEAST_FULL_HALF[Bit 0] = 1 */
    bcm5343x_reg_set(unit, R_L2_AUX_HASH_CONTROL, 0x15);
    bcm5343x_reg_set(unit, R_L3_AUX_HASH_CONTROL, 0x15);
#endif /* !CONFIG_HURRICANE3_ROMCODE */

    /* Egress Enable */
    val = 0x1;
    SOC_LPORT_ITER(lport) {
        bcm5343x_mem_set(unit, M_EGR_ENABLE(SOC_PORT_L2P_MAPPING(lport)), &val, 1);
    }
    

    /*
     * Set reference clock (based on 200MHz core clock)
     * to be 200MHz * (1/40) = 5MHz
     */
    WRITECSR(CMIC_RATE_ADJUST_EXT_MDIO, 0x10028);
    /* Match the Internal MDC freq with above for External MDC */
    WRITECSR(CMIC_RATE_ADJUST_INT_MDIO, 0x10028);


    /* The HW defaults for EGR_VLAN_CONTROL_1.VT_MISS_UNTAG == 1, which
     * causes the outer tag to be removed from packets that don't have
     * a hit in the egress vlan tranlation table. Set to 0 to disable this.
     */
    SOC_LPORT_ITER(i) {
        bcm5343x_reg_set(unit, R_EGR_VLAN_CONTROL_1(i), 0x0);
    }

#if !CONFIG_HURRICANE3_ROMCODE
    /* Enable SRC_REMOVAL_EN[Bit 0] and LAG_RES_EN[Bit2] */
    bcm5343x_reg_set(unit, R_SW2_FP_DST_ACTION_CONTROL, 0x5);
#endif /* !CONFIG_HURRICANE3_ROMCODE */



    
}

static void
soc_mmu_init(uint8 unit)
{
    int i, j;
    uint32 val;
    int tdm_size;
    const uint32 *arr = NULL;

    tdm_size = 34;
    arr = hr3_tdm_56162_1;

    /* DISABLE [Bit 0] = 1, TDM_WRAP_PTR[Bit 7:1] = TDM_SIZE-1 */
    val = ((tdm_size-1) << 1 | 1);
    bcm5343x_reg_set(0, R_IARB_TDM_CONTROL, val);

    for (i = 0; i < tdm_size; i++) {
        bcm5343x_mem_set(unit, M_IARB_TDM_TABLE(i), (uint32 *)&arr[i], 1);
        /* TDM programmed in MMU is in Logical port domain */
        val = (arr[i] != 63) ? hr3_sw_info.port_p2l_mapping[arr[i]] : 63;
        if (i == (tdm_size - 1)) {
            /* WRAP_EN [Bit 6] = 1 */
            val |= 0x40;
        }
        bcm5343x_mem_set(unit, M_MMU_ARB_TDM_TABLE(i), &val, 1);
    }

    /* DISABLE [Bit 0] = 0, TDM_WRAP_PTR[Bit 7:1] = POWERSAVE_MODE_TDM_SIZE-1 */
    val = (tdm_size-1) << 1;
    bcm5343x_reg_set(unit, R_IARB_TDM_CONTROL, val);

    /* MMU initialization */
    for (i = 0; i < COS_QUEUE_NUM; i++) {
        for (j = 0; j <= BCM5343X_LPORT_MAX; j++) {
            if (-1 == SOC_PORT_L2P_MAPPING(j)) {
                continue;
            }
            /*
             * The HOLCOSPKTSETLIMITr register controls BOTH the XQ
             * size per cosq AND the HOL set limit for that cosq.
             */
            bcm5343x_reg_set(unit, R_HOLCOSPKTSETLIMIT(i, j), MMU_R_HOLCOSPKTSETLIMIT);
            bcm5343x_reg_set(unit, R_HOLCOSPKTRESETLIMIT(i, j), MMU_R_HOLCOSPKTRESETLIMIT);
            bcm5343x_reg_set(unit, R_LWMCOSCELLSETLIMIT(i, j), MMU_R_LWMCOSCELLSETLIMIT);

            if (i == CFG_FLOW_CONTROL_ENABLED_COSQ) {
                bcm5343x_reg_set(unit, R_HOLCOSCELLMAXLIMIT(i, j), MMU_R_HOLCOSCELLMAXLIMIT_COS0);
            } else {
                bcm5343x_reg_set(unit, R_HOLCOSCELLMAXLIMIT(i, j), MMU_R_HOLCOSCELLMAXLIMIT_COS1);
            }

            bcm5343x_reg_set(unit, R_HOLCOSMINXQCNT(i, j), MMU_R_HOLCOSMINXQCNT);
            bcm5343x_reg_set(unit, R_PGCELLLIMIT(i, j), MMU_R_PGCELLLIMIT);
            bcm5343x_reg_set(unit, R_PGDISCARDSETLIMIT(i, j), MMU_R_PGDISCARDSETLIMIT);
        }
    }

    for (i = 0; i <= BCM5343X_LPORT_MAX; i++) {
        if (-1 == SOC_PORT_L2P_MAPPING(i)) {
            continue;
        }
        bcm5343x_reg_set(unit, R_DYNCELLLIMIT(i), MMU_R_DYNCELLLIMIT);
        bcm5343x_reg_set(unit, R_DYNXQCNTPORT(i), MMU_R_DYNXQCNTPORT);
        bcm5343x_reg_set(unit, R_DYNRESETLIMPORT(i), MMU_R_DYNRESETLIMPORT);

        bcm5343x_reg_set(unit, R_IBPPKTSETLIMIT(i), MMU_R_IBPPKTSETLIMIT);
    }

    bcm5343x_reg_set(unit, R_TOTALDYNCELLSETLIMIT, MMU_R_TOTALDYNCELLSETLIMIT);
    bcm5343x_reg_set(unit, R_TOTALDYNCELLRESETLIMIT, MMU_R_TOTALDYNCELLRESETLIMIT);

    /* DYN_XQ_EN[Bit8] = 1, HOL_CELL_SOP_DROP_EN[Bit7] = 1, SKIDMARKER[Bit3:2] = 3 */
    bcm5343x_reg_get(unit, R_MISCCONFIG, &val);
    val |= 0x18c;
    bcm5343x_reg_set(unit, R_MISCCONFIG, val);

    /* Port enable */
#if CFG_RXTX_SUPPORT_ENABLED
    /* Add CPU port */
    bcm5343x_reg_set(unit, R_MMUPORTENABLE, BCM5343X_ALL_PORTS_MASK | 0x1);
#else
    bcm5343x_reg_set(unit, R_MMUPORTENABLE, BCM5343X_ALL_PORTS_MASK);
#endif /* CFG_RXTX_SUPPORT_ENABLED */
}

static void
config_schedule_mode(uint8 unit)
{
    int i, j;

    SOC_LPORT_ITER(i) {
#if CONFIG_HURRICANE3_ROMCODE
        /* Strict Priority Mode[Bit 0-1] = 0x0, MTU_Quanta_Select[Bit 2-3]=0x3 */
        bcm5343x_reg_set(unit, R_XQCOSARBSEL(i), 0xC);
#else
        
        bcm5343x_reg_set(unit, R_WRRWEIGHT_COS0(i), 0x81);
        bcm5343x_reg_set(unit, R_WRRWEIGHT_COS1(i), 0x82);
        bcm5343x_reg_set(unit, R_WRRWEIGHT_COS2(i), 0x84);
        bcm5343x_reg_set(unit, R_WRRWEIGHT_COS3(i), 0x88);
        bcm5343x_reg_set(unit, R_XQCOSARBSEL(i), 0xE);
#endif /* CONFIG_HURRICANE3_ROMCODE */
        /* MAX_THD_SEL = 0 : Disable MAX shaper */
        for (j = 0; j < COS_QUEUE_NUM; j++) {
          bcm5343x_reg_set(unit, R_MAXBUCKETCONFIG(j, i), 0x0);
        }
    }
}

#ifdef CFG_SWITCH_LAG_INCLUDED
#if defined(CFG_SWITCH_RATE_INCLUDED) || defined(CFG_SWITCH_QOS_INCLUDED) || defined(CFG_SWITCH_LOOPDETECT_INCLUDED)
static void
_bcm5343x_lag_group_fp_set(uint8 unit, int start_index, uint8 lagid,
                     pbmp_t pbmp, pbmp_t old_pbmp, uint8 revise_redirect_pbmp, uint8 cpu_include)
{
    int i, j;
    uint32 tcam_entry[15], xy_entry[15], dm_entry[15];
    uint32 global_tcam_mask_entry[3] = { 0x1, 0, 0 };

    /* The entry (pbmp) bit 0 is cpu port.
     * The policy[0] redirect entry, bit 5 is cpu port.
     */

    if (cpu_include == TRUE) {
        j = 0;
    } else {
        j = BCM5343X_LPORT_MIN;
    }
    for (i = start_index; j <= BCM5343X_LPORT_MAX; i++, j++) {
        if ((j > 0) && (j < BCM5343X_LPORT_MIN)) {
            continue;
        }
        bcm5343x_mem_get(unit, M_FP_TCAM(i), dm_entry, 15);
        bcm5343x_xy_to_dm(dm_entry, tcam_entry, 15, 480);
        /*  Revise the source tgid qualify if the port is trunk port */
        if (old_pbmp & (0x1 << j)) {
            tcam_entry[0] &= 0x1fffffff;
            tcam_entry[0] |= (j & 0x7) << 29;
            tcam_entry[1] &= 0xfffff000;
            tcam_entry[1] |= (j >> 3) | 0x800;
        }
        if (pbmp & (0x1 << j)) {
            tcam_entry[0] &= 0x1fffffff;
            tcam_entry[0] |= (lagid & 0x7) << 29;
            tcam_entry[1] &= 0xfffff000;
            tcam_entry[1] |= (lagid >> 3) | 0x800;
        }
        bcm5343x_dm_to_xy(tcam_entry, xy_entry, 15, 480);
        bcm5343x_mem_set(unit, M_FP_TCAM(i), xy_entry, 15);
        bcm5343x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(i), global_tcam_mask_entry, 3);
    }

}
#endif /* CFG_SWITCH_RATE_INCLUDED || CFG_SWITCH_QOS_INCLUDED || CFG_SWITCH_LOOPDETECT_INCLUDED */

/*
 *  Function : bcm5343x_lag_group_set
 *  Purpose :
 *      Set lag group membership.
 *  Parameters :
 *  Return :
 *  Note :
 *
 */
sys_error_t
bcm5343x_lag_group_set(uint8 unit, uint8 lagid, pbmp_t pbmp)
{
    uint32 bitmap_entry[2] = {(uint32)pbmp, 0x0};
    uint32 old_bitmap_entry[2];
    uint32 entry[3] = {0, 0, 0};
    uint32 group_entry[4] = {0, 0, 0, 0};
    uint8 i, j, count = 0;
    uint8 trunk_port[BOARD_MAX_PORT_PER_LAG];

    for (i = BCM5343X_LPORT_MIN; i <= BCM5343X_LPORT_MAX; i++) {
        if(bitmap_entry[0] & (0x1 << i)) {
            count ++;
            trunk_port[count-1] = i;
        }
    }

    bcm5343x_mem_get(0, M_TRUNK_BITMAP(lagid), old_bitmap_entry, 2);

    if (bitmap_entry[0] != old_bitmap_entry[0]) {
        /* Need to update source port qualifier in FP TCAM entry  */
#ifdef CFG_SWITCH_RATE_INCLUDED
        /*
         * Slice 1 Entry 0~23 (one entry for each port):
         * Rate ingress
         */
        _bcm5343x_lag_group_fp_set(unit, RATE_IGR_IDX, lagid,
                                   bitmap_entry[0], old_bitmap_entry[0], FALSE, FALSE);
#endif /* CFG_SWITCH_RATE_INCLUDED */

#ifdef CFG_SWITCH_QOS_INCLUDED
        /*
         * Slice 2 Entry 0~23 (one entry for each port):
         * Port based QoS
         */
        _bcm5343x_lag_group_fp_set(unit, QOS_BASE_IDX, lagid,
                                   bitmap_entry[0], old_bitmap_entry[0], FALSE, FALSE);
#endif /* CFG_SWITCH_QOS_INCLUDED */

#ifdef CFG_SWITCH_LOOPDETECT_INCLUDED
        /*
         * Slice 3 #define LOOP_COUNT_IDX                 ( 3 * ENTRIES_PER_SLICE) 
         * (one entry for each port):
         * Loop detect counter
         */
        _bcm5343x_lag_group_fp_set(unit, LOOP_COUNT_IDX, lagid,
                                   bitmap_entry[0], old_bitmap_entry[0], FALSE, FALSE);
                                   
		/*
         * Slice 3, #define LOOP_REDIRECT_IDX              ( 3 * ENTRIES_PER_SLICE + 35)
         * (one entry for each port, including CPU):
         * Update both source port qualifier 
         */
        _bcm5343x_lag_group_fp_set(unit, LOOP_REDIRECT_IDX, lagid,
                                    bitmap_entry[0], old_bitmap_entry[0], TRUE, TRUE);
#endif /* CFG_SWITCH_LOOPDETECT_INCLUDED */

    }

    /* Set TRUNK Bitmap, TRUNK Group, Source TRUNK Map and NonUcast TRUNK Block Mask Table */
    for (i = BCM5343X_LPORT_MIN; i <= BCM5343X_LPORT_MAX; i++) {
        if(old_bitmap_entry[0] & (0x1 << i)) {
            entry[0] = 0x0;
            bcm5343x_mem_set(unit, M_SOURCE_TRUNK_MAP(i), entry, 3);
        }

        if(bitmap_entry[0] & (0x1 << i)) {
            entry[0] = 0x1 | (lagid << 2);
            bcm5343x_mem_set(unit, M_SOURCE_TRUNK_MAP(i), entry, 3);
        }
    }

    for (i = 0; i < BOARD_MAX_PORT_PER_LAG; i++) {
        if (count == 0) {
            j = 0;
        } else {
            j = trunk_port[i%count];
        }
        switch (i) {
            case 0:
                group_entry[0] |= j;
                break;
            case 1:
                group_entry[0] |= (j << 14);
                break;
            case 2:
                group_entry[0] |= (j << 28);
                break;
            case 3:
                group_entry[1] |= (j << 10);
                break;
            case 4:
                group_entry[1] |= (j << 24);
                break;
            case 5:
                group_entry[2] |= (j << 6);
                break;
            case 6:
                group_entry[2] |= (j << 20);
                break;
            case 7:
                group_entry[3] |= (j << 2);
                break;
        }
    }

    /* Set RTAG to 0x3 (SA+DA) */
    group_entry[3] |= (0x3 << 16);

    bcm5343x_mem_set(unit, M_TRUNK_BITMAP(lagid), bitmap_entry, 2);
    bcm5343x_mem_set(unit, M_TRUNK_GROUP(lagid), group_entry, 4);


    for (i = 0; i < 64; i++) {
        bcm5343x_mem_get(0, M_NONUCAST_TRUNK_BLOCK_MASK(i),
                                      entry, 2);
        entry[0] &= ~old_bitmap_entry[0];
        entry[0] |= bitmap_entry[0];
        if (count != 0) {
            entry[0] &= ~(0x1 << trunk_port[i%count]);
        }
        bcm5343x_mem_set(0, M_NONUCAST_TRUNK_BLOCK_MASK(i),
                                      entry, 2);
    }
    return SYS_OK;
}

/*
 *  Function : bcm5343x_lag_group_get
 *  Purpose :
 *      Get lag group membership.
 *  Parameters :
 *  Return :
 *  Note :
 *
 */
void
bcm5343x_lag_group_get(uint8 unit, uint8 lagid, pbmp_t *pbmp) {
    uint32 bitmap_entry[2];

    bcm5343x_mem_get(unit, M_TRUNK_BITMAP(lagid), bitmap_entry, 2);

    *pbmp = (pbmp_t)bitmap_entry[0];
}
#endif /* CFG_SWITCH_LAG_INCLUDED */

#define FW_ALIGN_BYTES                  16
#define FW_ALIGN_MASK                   (FW_ALIGN_BYTES - 1)
int
_firmware_helper(void *ctx, uint32 offset, uint32 size, void *data)
{
    int ioerr = 0;
    uint32 val;
    uint32 wbuf[4], ucmem_data[4];
    uint32 *fw_data;
    uint32 *fw_entry;
    uint32 fw_size;
    uint32 idx, wdx;
    phy_ctrl_t *pc = (phy_ctrl_t *)ctx;
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
        ioerr += WRITECSR(CMIC_RATE_ADJUST_EXT_MDIO, val);

        return ioerr ? SYS_ERR_STATE : SYS_OK;
    }
    if (sal_strcmp(pc->drv->drv_name, "bcmi_qtce_xgxs") != 0) {
        return CDK_E_NONE;
    }
    if (size == 0) {
        return SYS_ERR_PARAMETER;
    }

    /* Aligned firmware size */
    fw_size = (size + FW_ALIGN_MASK) & ~FW_ALIGN_MASK;
    /* Enable parallel bus access, ACCESS_MODE [Bit 0] = 0x1 */
    bcm5343x_reg_set(pc->unit, PMQ1_BLOCK_ID, R_GPORT_WC_UCMEM_CTRL, 0x1);

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
        bcm5343x_mem_set(pc->unit, PMQ1_BLOCK_ID,
                         M_GPORT_WC_UCMEM_DATA(idx >> 4), ucmem_data, 4);
    }

    /* Disable parallel bus access */
    bcm5343x_reg_set(pc->unit, PMQ1_BLOCK_ID, R_GPORT_WC_UCMEM_CTRL, 0x0);

    return ioerr ? SYS_ERR_STATE : SYS_OK;
}

extern int
bmd_phy_fw_helper_set(int unit, int port,
                      int (*fw_helper)(void *, uint32, uint32, void *));

                      
static void
bcm5343x_system_init(uint8 unit)
{
    int i, j;
    uint32 entry[6];
    uint32 val;

    /* Default port_entry :
     * bit 35 - 24 : PORT_VID = 0x1
     * bit 60 : TRUST_OUTER_DOT1P = 0x1
     * bit 99 - 96 : OUTER_TPID_ENABLE = 0x1
     * bit 121 : TRUST_INCOMING_VID = 0x1
     * bit 135 - 132 : CML_FLAGS_NEW = 0x8
     * bit 139 - 136 : CML_FLAGS_MOVE = 0x8
     */
    uint32 port_entry[12] = { 0x01000000, 0x10000000, 0x00000000, 0x02000001,
                              0x00000880, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0  };

#if CONFIG_HURRICANE3_ROMCODE
  uint32 cos_map[8] = { 0, 0, 1, 1, 2, 2, 3, 3} ;
#endif
    uint32 dot1pmap[16] = {
    0x00000000, 0x00000001, 0x00000004, 0x00000005, 0x00000008, 0x00000009, 0x0000000c, 0x0000000d,
    0x00000010, 0x00000011, 0x00000014, 0x00000015, 0x00000018, 0x00000019, 0x0000001c, 0x0000001d };

    /* Configurations to guarantee no packet modifications */
    SOC_LPORT_ITER(i) {
        /* ING_OUTER_TPID[0] is allowed outer TPID values */
        entry[0] = 0x1;
        bcm5343x_mem_set(unit, M_SYSTEM_CONFIG_TABLE(i), entry, 1);

        entry[0] = 0x0;
#ifdef CFG_SWITCH_VLAN_UNAWARE_INCLUDED
        /* DISABLE_VLAN_CHECKS[Bit 63] = 1, PACKET_MODIFICATION_DISABLE[Bit 62] = 1 */
        entry[1] = 0xC0000000;
#else
        entry[1] = 0x0;
#endif /* CFG_SWITCH_VLAN_UNAWARE_INCLUDED */
        entry[2] = 0x0;
        bcm5343x_mem_set(unit, M_SOURCE_TRUNK_MAP(i), entry, 3);

        bcm5343x_mem_set(unit, M_PORT(i), port_entry, 12);

        /* Clear Unknown Unicast Block Mask. */
        bcm5343x_reg_set(unit, R_UNKNOWN_UCAST_BLOCK_MASK_64(i), 0x0);

        /* Clear ingress block mask. */
        bcm5343x_reg_set(unit, R_ING_EGRMSKBMAP_64(i), 0x0);
    }

    for (i = 0; i <= BCM5343X_LPORT_MAX; i++) {
        if (-1 == SOC_PORT_L2P_MAPPING(i)) {
            continue;
        }

        /*
         * ING_PRI_CNG_MAP: Unity priority mapping and CNG = 0 or 1
         */
        for (j = 0; j < 16; j++) {
            bcm5343x_mem_set(unit, M_ING_PRI_CNG_MAP(i*16+j), &dot1pmap[j], 1);
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

    for (i = 0; i < 32; i++) {
        bcm5343x_mem_set(unit, M_TRUNK32_CONFIG_TABLE(i), &val, 1);
        bcm5343x_mem_set(unit, M_TRUNK32_PORT_TABLE(i), entry, 2);
    }

#if CONFIG_HURRICANE3_ROMCODE
/*
 * Assign 1p priority mapping:
 *  pri_0/1 ==> low    (COS0)
 *  pri_2/3 ==> normal (COS1)
 *  pri_4/5 ==> medium (COS2)
 *  pri_6/7 ==> high   (COS3)
 */
   #define INT_PRI_MAX  16 
   for (i = 0; i < INT_PRI_MAX ; i++) {
        if (i < 8) {
      bcm5343x_mem_set(0, M_COS_MAP(i), &cos_map[i], 1);
        } else {  
          bcm5343x_mem_set(0, M_COS_MAP(i), &cos_map[7], 1);
    }   
   }

#endif /* CONFIG_HURRICANE3_ROMCODE */

    enable_jumbo_frame(unit);
    config_schedule_mode(unit);

    /*
     * VLAN_DEFAULT_PBM is used as defalut VLAN members for those ports
     * that disable VLAN checks.
     */
    bcm5343x_reg_set(unit, R_VLAN_DEFAULT_PBM, BCM5343X_ALL_PORTS_MASK);

    /* ING_VLAN_TAG_ACTION_PROFILE:
     * UT_OTAG_ACTION (Bit 2-3) = 0x1
     * SIT_OTAG_ACTION (Bit 8-9) = 0x0
     * SOT_POTAG_ACTION (Bit 12-13) = 0x2
     * SOT_OTAG_ACTION (Bit 14-15) = 0x0
     * DT_POTAG_ACTION (Bit 20-21) = 0x2
     */
    val = 0x0202004;
    bcm5343x_mem_set(unit, M_ING_VLAN_TAG_ACTION_PROFILE(0), &val, 1);

    /*
     * Program l2 user entry table to drop below MAC addresses:
     * 0x0180C2000001, 0x0180C2000002 and 0x0180C200000E
     */

    /* VALID[Bit 0] = 1 */
    entry[0] = (0xC2000001 << 1) | 0x1;
    entry[1] = (0x0180 << 1) | 0x1;

    /* Encoded MASK: MAC address only */
    entry[2] = 0x7fffff80;
    entry[3] = 0x003f9fcf;
    /* DST_DISCARD[Bit 19] = 1 */
    entry[4] = 0x00080004;
    /* BPDU[Bit 2] = 1 */
    entry[5] = 0x4;
    bcm5343x_mem_set(unit, M_L2_USER_ENTRY(0), entry, 6);

    entry[0] = (0xC2000002 << 1) | 0x1;
    entry[2] = 0x7fffff40;
    bcm5343x_mem_set(unit, M_L2_USER_ENTRY(1), entry, 6);

    entry[0] = (0xC200000E << 1) | 0x1;
    entry[2] = 0x7ffffc40;
    bcm5343x_mem_set(unit, M_L2_USER_ENTRY(2), entry, 6);

#if !CONFIG_HURRICANE3_ROMCODE
#ifdef CFG_SWITCH_DOS_INCLUDED
    /*
     * Enable following Denial of Service protections:
     * DROP_IF_SIP_EQUALS_DIP
     * MIN_TCPHDR_SIZE = 0x14 (Default)
     * IP_FIRST_FRAG_CHECK_ENABLE
     * TCP_HDR_OFFSET_EQ1_ENABLE
     * TCP_HDR_PARTIAL_ENABLE
     */
    bcm5343x_reg_set(unit, R_DOS_CONTROL, 0x2280411);
    /* ICMP_V4_PING_SIZE_ENABLE, TCP_HDR_PARTIAL_ENABLE, 
            TCP_HDR_OFFSET_EQ1_ENABLE, ICMP_FRAG_PKTS_ENABLE */ 
    bcm5343x_reg_set(unit, R_DOS_CONTROL2, 0x01b00000);
#endif /* CFG_SWITCH_DOS_INCLUDED */

    /* enable FP_REFRESH_ENABLE [Bit 26] */
    bcm5343x_reg_get(unit, R_AUX_ARB_CONTROL_2, &val);
    val |= 0x4000000;
    bcm5343x_reg_set(unit, R_AUX_ARB_CONTROL_2, val);

    /*
     * Enable IPV4_RESERVED_MC_ADDR_IGMP_ENABLE[Bit 31], APPLY_EGR_MASK_ON_L3[Bit 13]
     * and APPLY_EGR_MASK_ON_L2[Bit 12]
     * Disable L2DST_HIT_ENABLE[Bit 2]
     */
    bcm5343x_reg64_get(unit, R_ING_CONFIG_64, entry, 2);
    entry[0] |= 0x80003000;
    entry[0] &= ~0x4;
    bcm5343x_reg64_set(unit, R_ING_CONFIG_64, entry, 2);

    /*
     * L3_IPV6_PFM=1, L3_IPV4_PFM=1, L2_PFM=1, IPV6L3_ENABLE=1, IPV4L3_ENABLE=1
     * IPMCV6_L2_ENABLE=1, IPMCV6_ENABLE=1, IPMCV4_L2_ENABLE=1, IPMCV4_ENABLE=1
     */
    entry[0] = 0x0003f015;
    entry[1] = 0x0;
    bcm5343x_mem_set(unit, M_VLAN_PROFILE(0), entry, 2);
#endif /* !CONFIG_HURRICANE3_ROMCODE */

    /* Do VLAN Membership check EN_EFILTER[Bit 3] for the outgoing port */
    SOC_LPORT_ITER(i) {
        bcm5343x_reg64_get(unit, R_EGR_PORT_64(i), entry, 2);
        entry[0] |= (0x1 << 3);
        bcm5343x_reg64_set(unit, R_EGR_PORT_64(i), entry, 2);
    }

#if CFG_RXTX_SUPPORT_ENABLED
    /*
     * Use VLAN 0 for CPU to transmit packets
     * All ports are untagged members, with STG=1 and VLAN_PROFILE_PTR=0
     */
    entry[0] = 0xfffffffd;
    entry[1] = 0x00000300;
    entry[2] = 0x0;
    entry[3] = 0x0;
    bcm5343x_mem_set(unit, M_VLAN(0), entry, 4);

    entry[0] = 0xfffffffc;
    entry[1] = 0xfffffffd;
    entry[2] = 0x00000101;
    bcm5343x_mem_set(unit, M_EGR_VLAN(0), entry, 3);

#ifdef __BOOTLOADER__
    /* Default VLAN 1 with STG=1 and VLAN_PROFILE_PTR=0 for bootloader */
    entry[0] = 0xfffffffc;
    entry[1] = 0x00000300;
    entry[2] = 0x0;
    entry[3] = 0x0;
    bcm5343x_mem_set(unit, M_VLAN(1), entry, 4);

    entry[0] = 0xfffffffc;
    entry[1] = 0xfffffffc;
    entry[2] = 0x00000101;
    bcm5343x_mem_set(unit, M_EGR_VLAN(1), entry, 3);
#endif /* __BOOTLOADER__ */

    /* Set VLAN_STG and EGR_VLAN_STG */
    entry[0] = 0xfffffff0;   
    entry[1] = 0xffffffff;
    entry[2] = 0x0;
    bcm5343x_mem_set(unit, M_VLAN_STG(1), entry, 3);
    bcm5343x_mem_set(unit, M_EGR_VLAN_STG(1), entry, 3);

    /* Make PORT_VID[Bit 35:24] = 0 for CPU port */
    bcm5343x_mem_get(unit, M_PORT(0), port_entry, 12);
    port_entry[0] &= 0x00ffffff;
    port_entry[1] &= 0xfffffff0;
    bcm5343x_mem_set(unit, M_PORT(0), port_entry, 12);

#if !CONFIG_HURRICANE3_ROMCODE
    /*
     * Trap DHCP[Bit 0] and ARP packets[Bit 4, 6] to CPU.
     * Note ARP reply is copied to CPU ONLY when l2 dst is hit.
     */
    SOC_LPORT_ITER(i) {
        bcm5343x_reg_set(unit, R_PROTOCOL_PKT_CONTROL(i), 0x51);
    }
#endif /* !CONFIG_HURRICANE3_ROMCODE */
#endif /* CFG_RXTX_SUPPORT_ENABLED */

    /* Enable aging timer */
    bcm5343x_reg_get(unit, R_L2_AGE_TIMER, &val);
    val |= (0x1 << 20);
    bcm5343x_reg_set(unit, R_L2_AGE_TIMER, val);
}

/* Function:
 *   bcm5343x_sw_init
 * Description:
 *   Perform chip specific initialization.
 *   This will be called by board_init()
 * Parameters:
 *   None
 * Returns:
 *   None
 */

sys_error_t
bcm5343x_sw_init(void)
{
    int   rv = 0;
    uint8 unit = 0, lport;
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED     
    int port_cnt = 0;
    sys_error_t sal_config_rv = SYS_OK, an_rv = SYS_OK;
    pbmp_t active_pbmp, phy_an_pbmp;
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */


    /* Get chip revision */
    bcm5343x_chip_revision(unit, &hr3_sw_info.devid, &hr3_sw_info.revid);

#if CFG_CONSOLE_ENABLED
    sal_printf("\ndevid = 0x%x, revid = 0x%x\n", hr3_sw_info.devid, hr3_sw_info.revid);
#endif /* CFG_CONSOLE_ENABLED */

#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED 
    sal_config_rv = sal_config_uint16_get(SAL_CONFIG_SKU_DEVID, &hr3_sw_info.devid);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Overwrite SKU device ID with value 0x%x.\n", hr3_sw_info.devid);
    }

    an_rv = sal_config_pbmp_get(SAL_CONFIG_PHY_AN_PORTS, &phy_an_pbmp);
    if (an_rv == SYS_OK) {
        sal_printf("Vendor Config : Set AN logical pbmp with value 0x%x.\n", phy_an_pbmp);
    }

    sal_config_rv = sal_config_uint8_get(SAL_CONFIG_QTC_INTERFACE, &qtc_interface);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set QTC interface to %d. (1:QSGMII, 2:SGMII, 3:Fiber)\n", qtc_interface);
        if ((qtc_interface != QTC_INTERFACE_QSGMII) && 
            (qtc_interface != QTC_INTERFACE_SGMII) && 
            (qtc_interface != QTC_INTERFACE_FIBER)) {
            sal_printf("The QTC interface %d is not valid and will change it to %d (1:QSGMII, 2:SGMII, 3:FIBER)\n", qtc_interface, CFG_QTC_INTERFACE);
            qtc_interface = CFG_QTC_INTERFACE;
        }
    }
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUD7ED */
    if (hr3_sw_info.devid == BCM53434_DEVICE_ID) {
        if (qtc_interface == QTC_INTERFACE_QSGMII) {
            SOC_PORT_COUNT(unit) = 24;
        } else {
             /* SGMII mode or Fiber mode */
            SOC_PORT_COUNT(unit) = 18;
        }
    } else {
        sal_printf("\nERROR : devid 0x%x is not supported in UM software !\n", hr3_sw_info.devid);
        return SYS_ERR_NOT_FOUND;
    }

    /* CPS reset complete SWITCH and CMICd */
    WRITECSR(CMIC_CPS_RESET, 0x1);
#if CONFIG_HURRICANE3_EMULATION
    sal_usleep(250000);
#else
    sal_usleep(1000);
#endif /* CONFIG_HURRICANE3_EMULATION */

      
    soc_reset(unit);

    soc_init_port_mapping(unit);

    soc_misc_init(unit);

    soc_mmu_init(unit);

    bcm5343x_system_init(unit);

    /* Probe PHYs */
    SOC_LPORT_ITER(lport) {
        rv = bmd_phy_probe(unit, lport);
        /* Configure 2-LANCE/4-LANE TSC if necessary. */
        if (CDK_SUCCESS(rv)) {
            if ((lport >= 10) && (lport <= 17)) {
                /* QTC ports */
                if (SOC_PORT_LANE_NUMBER(lport) == 4) {
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_qtce_xgxs",
                                      BMD_PHY_MODE_SERDES, 0);
                    if (qtc_interface == QTC_INTERFACE_SGMII) {
                        rv = bmd_phy_mode_set(unit, lport, "bcmi_qtce_xgxs",
                                  BMD_PHY_MODE_FIBER, 0);
                    } else {
                        rv = bmd_phy_mode_set(unit, lport, "bcmi_qtce_xgxs",
                                  BMD_PHY_MODE_FIBER, 1);
                    }
                } else if (SOC_PORT_LANE_NUMBER(lport) == 2) {
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_qtce_xgxs",
                                      BMD_PHY_MODE_SERDES, 1);
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_qtce_xgxs",
                                              BMD_PHY_MODE_2LANE, 1);
                } else {
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_qtce_xgxs",
                                      BMD_PHY_MODE_SERDES, 1);
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_qtce_xgxs",
                                              BMD_PHY_MODE_2LANE, 0);
                }

                rv = bmd_phy_fw_helper_set(unit, lport, _firmware_helper);
            }

            rv = bmd_phy_init(unit, lport);
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
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED  */
    SOC_LPORT_ITER(lport) {
        int ability, an;

        /* According the speed to configure the phy ability */
        ability = (BMD_PHY_ABIL_1000MB_FD | BMD_PHY_ABIL_100MB_FD | BMD_PHY_ABIL_10MB_FD);
        rv = bmd_phy_ability_set(unit, lport, "bcmi_qtce_xgxs", ability);
        if (!SOC_SUCCESS(rv)) {
            sal_printf("bcm5343x_sw_init set phy ability 0x%x on lport %d failed\n", ability, lport);
        }

        an = TRUE;

#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
        if (an_rv == SYS_OK){
            if (!(phy_an_pbmp & (0x1 << lport))) {
                an = FALSE;
            }
        }
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
                    
        rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, lport), an);
        if (!SOC_SUCCESS(rv)) {
            sal_printf("bcm5343x_sw_init set phy autoneg %d on lport %d failed\n", an, lport);
        }

        if (!an) {
            rv = PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, lport), SOC_PORT_SPEED_MAX(lport));
            if (!SOC_SUCCESS(rv)) {
                sal_printf("bcm5343x_sw_init set phy speed %d on lport %d failed\n", SOC_PORT_SPEED_MAX(lport), lport);
            }
        }
    }

    /* Init MACs */
    SOC_LPORT_ITER(lport) {
        hr3_sw_info.p_mac[lport] = &soc_mac_uni;

        MAC_INIT(hr3_sw_info.p_mac[lport], unit, lport);
        /* Probe function should leave port disabled */
        MAC_ENABLE_SET(hr3_sw_info.p_mac[lport], unit, lport, FALSE);

        hr3_sw_info.link[lport] = PORT_LINK_DOWN;
#if defined(CFG_SWITCH_EEE_INCLUDED)
        hr3_sw_info.need_process_for_eee_1s[lport] = FALSE;
#endif /*  CFG_SWITCH_EEE_INCLUDED */
    }

    bcm5343x_load_led_program(unit);

#if CONFIG_HURRICANE3_EMULATION
    SOC_LPORT_ITER(lport) {
        link_qt[lport] = PORT_LINK_UP;
    }
    bcm5343x_linkscan_task(NULL);
    sal_printf("all ports up!\n");
#endif /* CONFIG_HURRICANE3_EMULATION */

    /* Register background process for handling link status */
    timer_add(bcm5343x_linkscan_task, NULL, LINKSCAN_INTERVAL);

    if (rv) {
        return SYS_ERR;
    } else {
        return SYS_OK;
    }
}

/* Before FP_TCAM insert, encode FP_TCAM entry.
 * dm_entry: Original data
 * xy_entry: encode data
 * data_word: size of one FP_TCAM entry
 * bit_length: full_key/full_mask bit length
 */
void
bcm5343x_dm_to_xy(uint32 *dm_entry, uint32 *xy_entry, int data_words, int bit_length)
{
    int i;
    int data_bytes;
    int word_length = (bit_length + 31) / 32;;
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
        mask[i] = (temp_tcam_entry[i+7] >> 12) | ((temp_tcam_entry[i+8] & 0x1fff) << 20);
        if (i == 7) {
            key[i] = (temp_tcam_entry[i] >> 2) & 0x3ff;
            mask[i] = (temp_tcam_entry[i+7] >> 12) & 0x3ff;
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
            temp_tcam_entry[i] = ((key[i-1] >> 30) & 0x3) | ((key[i] << 2) & 0xffc) |
                          (mask[i-7] << 12);
        } else if (i <= 13) {
            temp_tcam_entry[i] = ((mask[i-7-1] >> 20) & 0xfff) | ((mask[i-7] << 12) & 0xfffff000);
        } else {
            temp_tcam_entry[i] = ((mask[i-7-1] >> 20) & 0xfff) | ((mask[i-7] << 12) & 0x2fff000);
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
bcm5343x_xy_to_dm(uint32 *xy_entry, uint32 *dm_entry, int data_words, int bit_length)
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
        mask[i] = (temp_tcam_entry[i+7] >> 12) | ((temp_tcam_entry[i+8] & 0x1fff) << 20);
        if (i == 7) {
            key[i] = (temp_tcam_entry[i] >> 2) & 0x3ff;
            mask[i] = (temp_tcam_entry[i+7] >> 12) & 0x3ff;
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
            temp_tcam_entry[i] = ((key[i-1] >> 30) & 0x3) | ((key[i] << 2) & 0xffc) |
                          (mask[i-7] << 12);
        } else if (i <= 13) {
            temp_tcam_entry[i] = ((mask[i-7-1] >> 20) & 0xfff) | ((mask[i-7] << 12) & 0xfffff000);
        } else {
            temp_tcam_entry[i] = ((mask[i-7-1] >> 20) & 0xfff) | ((mask[i-7] << 12) & 0x2fff000);
        }
    }
    sal_memcpy(dm_entry, temp_tcam_entry, data_bytes);
}

void
bcm5343x_loopback_enable(uint8 unit, uint8 port, int loopback_mode)
{
    int rv = 0;
    int link;
    int andone;
    if (loopback_mode == PORT_LOOPBACK_MAC) {
      uint32 flag;
      bcm5343x_handle_link_down(unit, port, TRUE);
      hr3_sw_info.loopback[port] = PORT_LOOPBACK_MAC;
      bcm5343x_handle_link_up(unit, port, TRUE, &flag);
      return;
    } else if (loopback_mode == PORT_LOOPBACK_NONE) {
        if (hr3_sw_info.loopback[port] != PORT_LOOPBACK_NONE) {
            if (hr3_sw_info.loopback[port] == PORT_LOOPBACK_MAC) {
                bcm5343x_handle_link_down(unit, port, TRUE);
            } else if (hr3_sw_info.loopback[port] == PORT_LOOPBACK_PHY) {
                rv = PHY_LOOPBACK_SET(BMD_PORT_PHY_CTRL(unit, port), 0);
                rv = PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit, port), &link, &andone);
            }
            hr3_sw_info.loopback[port] = PORT_LOOPBACK_NONE;
        }
        return;
    }

    hr3_sw_info.loopback[port] = loopback_mode;

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
        hr3_sw_info.link[port] = PORT_LINK_DOWN;
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

soc_switch_t soc_switch_bcm5343x =
{
    bcm5343x_chip_type,
    NULL,
    bcm5343x_port_count,
    NULL,
    NULL,
#if CFG_RXTX_SUPPORT_ENABLED
    bcm5343x_rx_set_handler,
    bcm5343x_rx_fill_buffer,
    bcm5343x_tx,
#endif /* CFG_RXTX_SUPPORT_ENABLED */

    bcm5343x_link_status,
    bcm5343x_chip_revision,
    bcm5343x_reg_get,
    bcm5343x_reg_set,
    bcm5343x_mem_get,
    bcm5343x_mem_set,
#ifdef CFG_SWITCH_VLAN_INCLUDED
    bcm5343x_pvlan_egress_set,
    bcm5343x_pvlan_egress_get,
    bcm5343x_qvlan_port_set,
    bcm5343x_qvlan_port_get,
    bcm5343x_vlan_create,
    bcm5343x_vlan_destroy,
    bcm5343x_vlan_type_set,
    bcm5343x_vlan_reset,
#endif  /* CFG_SWITCH_VLAN_INCLUDED */
    bcm5343x_phy_reg_get,
    bcm5343x_phy_reg_set,
};

