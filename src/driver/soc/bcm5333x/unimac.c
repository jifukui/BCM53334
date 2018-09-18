/*
 * $Id: unimac.c,v 1.10 Broadcom SDK $
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
#include "utils/system.h"

/*
 * Forward Declarations
 */
mac_driver_t soc_mac_uni;

#define SOC_UNIMAC_SPEED_10     0x0
#define SOC_UNIMAC_SPEED_100    0x1
#define SOC_UNIMAC_SPEED_1000   0x2
#define SOC_UNIMAC_SPEED_2500   0x3

extern void
soc_egress_drain_cells(uint8 unit, uint8 lport, uint32 drain_timeout);

static int
_mac_uni_sw_reset(uint8 unit, uint8 lport, BOOL reset_assert)
{
    uint32 command_config;
    int reset_sleep_usec;
    reset_sleep_usec = 2;

    bcm5333x_reg_get(unit, SOC_PORT_BLOCK(lport),
        R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), &command_config);

    if (reset_assert) {
        /* SIDE EFFECT: TX and RX are disabled when SW_RESET is set. */
        /* Assert SW_RESET */
        command_config |= (0x1 << 13);
    } else {
        /* Deassert SW_RESET */
        command_config &= ~(0x1 << 13);
    }

    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport),
        R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), command_config);

    sal_usleep(reset_sleep_usec);

    return SYS_OK;
}

static void
_mac_uni_drain_cells(uint8 unit, uint8 lport)
{
    uint32 val;
    BOOL pause_tx = 1, pause_rx = 1;

    /* First put the port in flush state - the packets from the XQ of the
     * port are purged after dequeue.
     */
    bcm5333x_reg_set(unit, R_MMUFLUSHCONTROL, (1 << lport));

    bcm5333x_reg_get(unit, SOC_PORT_BLOCK(lport),
        R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), &val);

    /* Bit[8]: PAUSE_IGNORE, Bit[28]: IGNORE_TX_PAUSE */
    if (val & (0x1 << 28)) {
        pause_tx = 0;
    }

    if (val & (0x1 << 8)) {
        pause_rx = 0;
    }

    /* Disable pause function */
    soc_mac_uni.md_pause_set(unit, lport, 0, 0);

    /* Drop out all packets in TX FIFO without egressing any packets */
    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport),
                R_FLUSH_CONTROL(SOC_PORT_BLOCK_INDEX(lport)), 0x1);

#if 0
    /* Notify PHY driver */
    soc_phyctrl_notify(unit, port, phyEventStop, PHY_STOP_DRAIN));
#endif

    /* Disable switch egress metering so that packet draining is not rate
     * limited.
     */
    soc_egress_drain_cells(unit, lport, 250000);

#if 0
    /* Notify PHY driver */
    soc_phyctrl_notify(unit, lport, phyEventResume, PHY_STOP_DRAIN));
#endif

    /* Soft-reset is recommended here.
     * SOC_IF_ERROR_RETURN
     *     (soc_mac_uni.md_control_set(unit, lport, SOC_MAC_CONTROL_SW_RESET,
     *                                 TRUE));
     * SOC_IF_ERROR_RETURN
     *     (soc_mac_uni.md_control_set(unit, lport, SOC_MAC_CONTROL_SW_RESET,
     *                                 FALSE));
     */

    /* Bring the TxFifo out of flush */
    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport),
                R_FLUSH_CONTROL(SOC_PORT_BLOCK_INDEX(lport)), 0x0);

    /* Restore original pause configuration */
    soc_mac_uni.md_pause_set(unit, lport, pause_tx, pause_rx);

    /* Bring the switch MMU out of flush */
    bcm5333x_reg_set(unit, R_MMUFLUSHCONTROL, 0);
}

static sys_error_t
mac_uni_enable_set(uint8 unit, uint8 lport, BOOL enable)
{
    uint32 val, command_config;

    bcm5333x_reg_get(unit, SOC_PORT_BLOCK(lport),
        R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), &command_config);

    /* First put the MAC in reset */
    _mac_uni_sw_reset(unit, lport, TRUE);

    /* de-assert RX_ENA and TX_ENA */
    command_config &= ~(0x3);
    command_config |= (0x1 << 13);
    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport),
        R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), command_config);
    _mac_uni_sw_reset(unit, lport, FALSE);

    sal_usleep(2);
    /* Bring the MAC out of reset */
    if (!enable) {
        /* Put in reset */

        /* Remove port from EPC_LINK */
        bcm5333x_reg_get(unit, R_EPC_LINK_BMAP_64, &val);
        if (SOC_IS_DEERHOUND(unit)) {
            bcm5333x_reg_set(unit, R_EPC_LINK_BMAP_64, val & ~(1 << lport));
        } else {
            bcm5333x_reg_set(unit, R_EPC_LINK_BMAP_64, val & ~(1 << SOC_PORT_L2P_MAPPING(lport)));
        }

        _mac_uni_drain_cells(unit, lport);
        _mac_uni_sw_reset(unit, lport, TRUE);
#if 0
        soc_phyctrl_notify(unit, port, phyEventStop, PHY_STOP_MAC_DIS));
#endif
    } else {
        _mac_uni_sw_reset(unit, lport, TRUE);
        /* if it is to enable, assert RX_ENA and TX_ENA */
        command_config |= 0x3;
        command_config &= ~(0x1 << 13);

        bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport),
            R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), command_config);
        _mac_uni_sw_reset(unit, lport, FALSE);
        sal_usleep(2);

        /* Add port to EPC_LINK */
        bcm5333x_reg_get(unit, R_EPC_LINK_BMAP_64, &val);
        if (SOC_IS_DEERHOUND(unit)) {
            bcm5333x_reg_set(unit, R_EPC_LINK_BMAP_64, val | (1 << lport));
        } else {
            bcm5333x_reg_set(unit, R_EPC_LINK_BMAP_64, val | (1 << SOC_PORT_L2P_MAPPING(lport)));
        }
#if 0
        _mac_uni_sw_reset(unit, lport, TRUE);
        soc_phyctrl_notify(unit, port, phyEventResume, PHY_STOP_MAC_DIS);
        _mac_uni_sw_reset(unit, lport, FALSE);
#endif
    }
    return SYS_OK;
}

static sys_error_t
mac_uni_init(uint8 unit, uint8 lport)
{
    uint32 command_config = 0x10020D8;
    uint8 system_mac[6];
    uint32 mac_0, mac_1;

    /* First put the MAC in reset and sleep */
    _mac_uni_sw_reset(unit, lport, TRUE);

    /* Do the initialization */
    /*
     * ETH_SPEEDf = 1000, PROMIS_ENf = 1, CRC_FWDf=1, PAUSE_FWDf = 1
     */
    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport),
        R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), command_config);

    /* Initialize mask for purging packet data received from the MAC */
    if (0 == SOC_PORT_BLOCK_INDEX(lport)) {
        bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport), R_GPORT_RSV_MASK, 0x70);
        bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport), R_GPORT_STAT_UPDATE_MASK, 0x70);
    }

    /* Bring the UniMAC out of reset */
    _mac_uni_sw_reset(unit, lport, FALSE);

    /* Pulse the Serdes AN if using auto_cfg mode */
#if 0
    if (auto_cfg[unit][port]) {
        soc_phyctrl_notify(unit, port, phyEventAutoneg, 0);
        soc_phyctrl_notify(unit, port, phyEventAutoneg, 1);
    }
#endif

    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport),
        R_TX_IPG_LENGTH(SOC_PORT_BLOCK_INDEX(lport)), 12);

    get_system_mac(system_mac);

    mac_0 = system_mac[0] << 24 |
            system_mac[1] << 16 |
            system_mac[2] << 8 |
            system_mac[3] << 0;
    mac_1 = system_mac[4] << 8 |
            system_mac[5];

    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport), R_MAC_0(SOC_PORT_BLOCK_INDEX(lport)), mac_0);
    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport), R_MAC_1(SOC_PORT_BLOCK_INDEX(lport)), mac_1);    

    return SYS_OK;
}

static sys_error_t
mac_uni_duplex_set(uint8 unit, uint8 lport, BOOL duplex)
{
    uint32 speed, command_config;
#if 0
    if (auto_cfg[unit][port]) {
        return SYS_OK;
    }
#endif
    bcm5333x_reg_get(unit, SOC_PORT_BLOCK(lport),
        R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), &command_config);

    speed = (command_config >> 2) & 0x3;

    if (speed >= SOC_UNIMAC_SPEED_1000) {
        /* If speed is 1000 or 2500 Mbps, duplex bit is ignored by unimac
         * and unimac runs at full duplex mode.
         */
        return SYS_OK;
    }

    /* Bit[10]: Half duplex enable */
    if (duplex) {
        command_config &= ~(0x1 << 10);
    } else {
        command_config |= (0x1 << 10);
    }

    /* First put the MAC in reset */
    _mac_uni_sw_reset(unit, lport, TRUE);
    command_config |= (0x1 << 13);
    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport),
        R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), command_config);

    /* Set IPG to match new duplex */

#if 0
    /*
     * Notify internal PHY driver of duplex change in case it is being
     * used as pass-through to an external PHY.
     */
    soc_phyctrl_notify(unit, port, phyEventDuplex, duplex);
#endif

    _mac_uni_sw_reset(unit, lport, FALSE);

    return SYS_OK;
}
static sys_error_t
mac_uni_speed_set(uint8 unit, uint8 lport, int speed)
{
    uint32 speed_select, command_config;

    bcm5333x_reg_get(unit, SOC_PORT_BLOCK(lport),
        R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), &command_config);

#if 0
    if (auto_cfg[unit][port]) {
        return SYS_OK;
    }
#endif

    switch (speed) {
    case 10:
        speed_select = SOC_UNIMAC_SPEED_10;
    break;
    /* support non-standard speed in Broadreach mode */
    case 20:
    case 25:
    case 33:
    case 50:
    /* fall through to case 100 */
    case 100:
        speed_select = SOC_UNIMAC_SPEED_100;
    break;
    case 1000:
        speed_select = SOC_UNIMAC_SPEED_1000;
        break;
    case 2500:
        speed_select = SOC_UNIMAC_SPEED_2500;
        break;
    case 0:
        return (SYS_OK);              /* Support NULL PHY */
    default:
        return (SYS_ERR_PARAMETER);
    }

    command_config &= ~(0x3 << 2);
    command_config |= (speed_select << 2);

    /* First reset the MAC */
    _mac_uni_sw_reset(unit, lport, TRUE);
    command_config |= (0x1 << 13);
    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport),
        R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), command_config);
#if 0
    /*
     * Notify internal PHY driver of speed change in case it is being
     * used as pass-through to an external PHY.
     */
    soc_phyctrl_notify(unit, port, phyEventSpeed, speed);

     /* Set IPG to match new speed */
    mac_uni_ipg_update(unit, port);
#endif

    /* Bring the MAC out of reset */
    _mac_uni_sw_reset(unit, lport, FALSE);

#if 0
    /* MAC speed switch results in a tx clock glitch to the serdes in 100fx mode.
     * reset serdes txfifo clears this condition. However this reset triggers
     * a link transition. Do not apply this reset if speed is already in 100M
     */
    if ((speed == 100) && (cur_speed != SOC_UNIMAC_SPEED_100)) {
        (void)soc_phyctrl_notify(unit, port, phyEventTxFifoReset, 100);
    }
#endif

    return SYS_OK;
}
static sys_error_t
mac_uni_pause_set(uint8 unit, uint8 lport, BOOL pause_tx, BOOL pause_rx)
{
    uint32 command_config;

    bcm5333x_reg_get(unit, SOC_PORT_BLOCK(lport),
        R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), &command_config);

    if (pause_tx) {
        command_config &= ~(0x1 << 28);
    } else {
        command_config |= (0x1 << 28);
    }

    if (pause_rx) {
        command_config &= ~(0x1 << 8);
    } else {
        command_config |= (0x1 << 8);
    }

    /* First put the MAC in reset */
    _mac_uni_sw_reset(unit, lport, TRUE);
    command_config |= (0x1 << 13);
    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport),
        R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), command_config);
    /* Bring the MAC out of reset */
    _mac_uni_sw_reset(unit, lport, FALSE);

    /* Add 2usec delay before deasserting SW_RESET */
    sal_usleep(2);


    return SYS_OK;
}
static sys_error_t
mac_uni_loopback_set(uint8 unit, uint8 lport, BOOL lb)
{
    uint32 command_config;

    _mac_uni_sw_reset(unit, lport, TRUE);

    bcm5333x_reg_get(unit, SOC_PORT_BLOCK(lport),
        R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), &command_config);

    if (lb) {
        command_config |= (0x1 << 15);
    } else {
        command_config &= ~(0x1 << 15);
    }

    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport),
        R_COMMAND_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), command_config);

    _mac_uni_sw_reset(unit, lport, FALSE);

    return 0;
}

static sys_error_t
mac_uni_frame_max_set(uint8 unit, uint8 lport, uint32 mtu)
{
    uint32 entry[2];

    entry[1] = 0;
    entry[0] = mtu;
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
                       R_FRM_LENGTH(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    return SYS_OK;
}

static sys_error_t
mac_uni_frame_max_get(uint8 unit, uint8 lport, uint32* mtu)
{
    uint32 entry[2];    

    if (mtu == NULL) {
        return SYS_ERR;
    }
    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_FRM_LENGTH(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    *mtu = entry[0];

    return SYS_OK;
}


mac_driver_t soc_mac_uni = {
    "UniMAC Driver",                 /* drv_name */
    mac_uni_init,                    /* md_init  */
    mac_uni_enable_set,              /* md_enable_set */
    mac_uni_duplex_set,              /* md_duplex_set */
    mac_uni_speed_set,               /* md_speed_set */
    mac_uni_pause_set,               /* md_pause_set */
    mac_uni_loopback_set,            /* md_lb_set */
#if 0
    mac_uni_ifg_set,                 /* md_ifg_set */
#endif
    mac_uni_frame_max_set,      /* md_frame_max_set */
    mac_uni_frame_max_get,     /* md_frame_max_get */
};



