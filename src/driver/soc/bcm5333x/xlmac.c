/*
 * $Id: xlmac.c,v 1.10 Broadcom SDK $
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
mac_driver_t soc_mac_xl;

#define SOC_XLMAC_SPEED_10     0x0
#define SOC_XLMAC_SPEED_100    0x1
#define SOC_XLMAC_SPEED_1000   0x2
#define SOC_XLMAC_SPEED_2500   0x3
#define SOC_XLMAC_SPEED_10000  0x4

/* Forwards */
static void
soc_port_credit_reset(uint8 unit, uint8 lport)
{
    uint32 val;

    /* Disable port */
    bcm5333x_reg_get(unit, SOC_PORT_BLOCK(lport), R_XLPORT_ENABLE_REG, &val);
    val &= ~(0x1 << SOC_PORT_BLOCK_INDEX(lport));
    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_ENABLE_REG, val);

    val = 1;
    bcm5333x_mem_set(unit, M_EGR_PORT_CREDIT_RESET(SOC_PORT_L2P_MAPPING(lport)), &val, 1);

    bcm5333x_reg_get(unit, SOC_PORT_BLOCK(lport), R_XLPORT_TXFIFO_CTRL(SOC_PORT_BLOCK_INDEX(lport)), &val);
    val |= 0x3;
    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_TXFIFO_CTRL(SOC_PORT_BLOCK_INDEX(lport)), val);
    
    sal_usleep(1000);

    val = 0;
    bcm5333x_mem_set(unit, M_EGR_PORT_CREDIT_RESET(SOC_PORT_L2P_MAPPING(lport)), &val, 1);

    bcm5333x_reg_get(unit, SOC_PORT_BLOCK(lport), R_XLPORT_TXFIFO_CTRL(SOC_PORT_BLOCK_INDEX(lport)), &val);
    val &= 0xfffffffc;
    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_TXFIFO_CTRL(SOC_PORT_BLOCK_INDEX(lport)), val);
    
    /* Enable port */
    bcm5333x_reg_get(unit, SOC_PORT_BLOCK(lport), R_XLPORT_ENABLE_REG, &val);
    val |= (0x1 << SOC_PORT_BLOCK_INDEX(lport));
    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_ENABLE_REG, val);
}

static void
soc_port_fifo_reset(uint8 unit, uint8 lport)
{
#if 0
    uint32 val, orig_val;

    if (IS_XL_PORT(lport)) {
        bcm5333x_reg_get(unit, SOC_PORT_BLOCK(lport), R_XLPORT_SOFT_RESET, &orig_val);
        val = orig_val | (0x1 << SOC_PORT_BLOCK_INDEX(lport));
        bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_SOFT_RESET, val);
        bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_SOFT_RESET, orig_val);
    }
#endif
}

void
soc_egress_drain_cells(uint8 unit, uint8 lport, uint32 drain_timeout)
{
    int cos;
    uint32 count, val, orig_val;
    
    bcm5333x_reg_get(unit, R_EGRMETERINGCONFIG(SOC_PORT_L2P_MAPPING(lport)), &orig_val);
    val = 0;
    bcm5333x_reg_set(unit, R_EGRMETERINGCONFIG(SOC_PORT_L2P_MAPPING(lport)), val);

    
    do {
        count = 0;
        for (cos = 0; cos < COS_QUEUE_NUM; cos++) {
            bcm5333x_reg_get(unit, R_COSLCCOUNT(cos, SOC_PORT_L2P_MAPPING(lport)), &val);
            count += val;
        }
    } while (count != 0);

    bcm5333x_reg_set(unit, R_EGRMETERINGCONFIG(SOC_PORT_L2P_MAPPING(lport)), orig_val);
}

static void
_mac_xl_drain_cells(uint8 unit, uint8 lport)
{
    int         pause_rx, pfc_rx, llfc_rx;
    uint32      entry[2];

    /* Disable pause/pfc function */
    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* TX_PAUSE_ENf[Bit 17][Bit=1/RX_PAUSE_ENf[Bit 18]=1 */
    pause_rx = (entry[0] & (0x1 << 18));
    entry[0] &= ~(0x1 << 18);
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* PFC_REFRESH_ENf[Bit 32]=1 */
    pfc_rx = (entry[1] & 0x1);
    entry[1] &= ~0x1;
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_LLFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* RX_LLFC_EN[Bit 1]=1 */
    llfc_rx = (entry[0] & 0x2);
    entry[0] &= ~0x2;
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_LLFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* Drain data in TX FIFO without egressing */
    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* SOFT_RESET = 0x1 */
    entry[0] |= (0x1 << 6);
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_TX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* DISCARDf[Bit 2]/EP_DISCARDf[Bit 37]=1 */
    entry[0] |= 0x4;
    entry[1] |= (0x1 << 5);
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_TX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* SOFT_RESET = 0x0 */
    entry[0] &= ~(0x1 << 6);
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* Notify PHY driver */
#if 0
    soc_phyctrl_notify(unit, port, phyEventStop, PHY_STOP_DRAIN);
#endif

    /* Wait until mmu cell count is 0 */
    soc_egress_drain_cells(unit, lport, 250000);

    /* Wait until TX fifo cell count is 0 */
    do {
        bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_TXFIFO_CELL_CNT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    } while(entry[0] != 0);

    /* Notify PHY driver */
#if 0
    soc_phyctrl_notify(unit, port, phyEventResume, PHY_STOP_DRAIN);
#endif

    /* Stop TX FIFO drainging */
    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_TX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* DISCARDf[Bit 2]/EP_DISCARDf[Bit 37]=0 */
    entry[0] &= ~0x4;
    entry[1] &= ~(0x1 << 5);
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_TX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* Restore original pause/pfc/llfc configuration */
    if (pfc_rx) {
        bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        pfc_rx = (entry[1] & 0x1);
        entry[1] |= 0x1;
        bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    }

    if (llfc_rx) {
        bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_LLFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        entry[0] |= 0x2;
        bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_LLFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    }

    if (pause_rx) {
        bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* TX_PAUSE_ENf[Bit 17][Bit=1/RX_PAUSE_ENf[Bit 18]=1 */
        entry[0] |= (0x1 << 18);
        bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    }
}

static sys_error_t
mac_xl_enable_set(uint8 unit, uint8 lport, BOOL enable)
{
    uint32 ctrl[2], octrl[2], val;

    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), ctrl, 2);

    octrl[0] = ctrl[0];

    /* Don't disable TX[Bit 0] since it stops egress and hangs if CPU sends */
    ctrl[0] |= 0x1;
    if (enable) {
        /* RX[Bit 1] */
        ctrl[0] |= 0x2;
    } else {
        ctrl[0] &= ~0x2;
    }

    if (ctrl[0] == octrl[0]) {
        if (enable || (!enable && (ctrl[0] & 0x40))) { 
            /* Don't do it again */
            return SYS_OK;
        }
    }

    if (enable) {
        int i;
        for (i = 0; i < 10; i++) {
            /* Reset EP credit before de-assert SOFT_RESET */
            soc_port_credit_reset(unit, lport);
            bcm5333x_mem_get(unit, M_EGR_PORT_REQUESTS(SOC_PORT_L2P_MAPPING(lport)), &val, 1);
            if (val != 0) {
                break;
            } else {
                sal_printf("retry %d\n", i);
            }
        }
        /* Enable both TX and RX, deassert SOFT_RESET [Bit 6] */
        ctrl[0] &= ~(0x1 << 6);
        bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), ctrl, 2);

        /* Add port to EPC_LINK */
        bcm5333x_reg_get(unit, R_EPC_LINK_BMAP_64, &val);
        bcm5333x_reg_set(unit, R_EPC_LINK_BMAP_64, val | (1 << lport));
    } else {
        /* Disable RX */
        bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), ctrl, 2);

        /* Remove port from EPC_LINK */
        bcm5333x_reg_get(unit, R_EPC_LINK_BMAP_64, &val);
        bcm5333x_reg_set(unit, R_EPC_LINK_BMAP_64, val & ~(1 << lport));

        /* Drain cells */
        _mac_xl_drain_cells(unit, lport);

        /* Reset port FIFO */
        soc_port_fifo_reset(unit, lport);

        /* Put port into SOFT_RESET */
        ctrl[0] |= (0x1 << 6);

        bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), ctrl, 2);
    }

    return SYS_OK;
}

static sys_error_t
mac_xl_init(uint8 unit, uint8 lport)
{
    uint32 mac_ctrl[2], entry[2];
    uint8 system_mac[6];   

    /* Disable Tx/Rx, assume that MAC is stable (or out of reset) */
    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), mac_ctrl, 2);

    /* Reset EP credit before de-assert SOFT_RESET Bit[6]*/
    if (mac_ctrl[0] & 0x40) {
        soc_port_credit_reset(unit, lport);
    }
    /* Enable XGMII_IPG_CHECK_DISABLEf for higig port */

    /* Disable XGMII_IPG_CHECK_DISABLEf[11])/SOFT_RESETf[6]/RX_ENf[1]/TX_ENf[0] */
    mac_ctrl[0] &= ~(0x843);
    if (IS_HG_PORT(lport)) {
        /* XGMII_IPG_CHECK_DISABLEf[11] = 1 */
        mac_ctrl[0] |= 0x800;
    }
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), mac_ctrl, 2);

    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_RX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* Set STRICT_PREAMBLE[Bit 3] for XE ports */
    /* STRIP_CRCf[Bit 2]=0 */
    entry[0] &= ~(0x1 << 2);
    if (!IS_HG_PORT(lport)) {
        entry[0] |= (0x1 << 3);
    }
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_RX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_TX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* AVERAGE_IPGf[Bit 18:12] : XE port = 96/8, HIGIG pott = 64/8
     * CRC_MODEf[Bit 1:0]=2(Replace) 
     */
    entry[0] &= 0xfff80ffc;
    if (IS_HG_PORT(lport)) {
        entry[0] |= ((8 & 0x7f) << 12) | 0x2;
    } else {
        entry[0] |= ((12 & 0x7f) << 12) | 0x2;
    }
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_TX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* Enable pause except for stacking ports */
    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* TX_PAUSE_ENf[Bit 17][Bit=1/RX_PAUSE_ENf[Bit 18]=1 */
    entry[0] |= 0x00060000;
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* PFC_REFRESH_ENf[Bit 32]=1 */
    entry[1] |= 0x1;
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_MODE(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* Setup header mode for higig2 mode */
    entry[0] &= ~(0x7);
    if (IS_HG_PORT(lport)) {
        entry[0] |= 0x2;
    }

    entry[0] &= ~(0x7 << 4);
    /* 
     * Bit[6:4]: 4 = 10000 or plus, 3 = 2500, 2 = 1000, 1 = 100, 0 = 10 
     * Assign max_speed = 10000 
     */
    entry[0] |= (0x4 << SOC_XLMAC_SPEED_10000);
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_MODE(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* Initialize mask for purging packet data received from the MAC */
    bcm5333x_reg_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLPORT_MAC_RSV_MASK(SOC_PORT_BLOCK_INDEX(lport)), 0x58);

    /* Enable DROP_TX_DATA_ON_LOCAL_FAULTf[Bit 4] and
     * DROP_TX_DATA_ON_REMOTE_FAULTf[Bit 5]
     */
    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_RX_LSS_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    entry[0] |= 0x30;
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_RX_LSS_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* Disable loopback and bring XLMAC out of reset */
    /* LOCAL_LPBKf[Bit 2] = 0, RX_ENf[Bit 1] = 1, TX_ENf[Bit 0]=1 */
    mac_ctrl[0] &= ~(0x4);
    mac_ctrl[0] |= 0x3;
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), mac_ctrl, 2);

    get_system_mac(system_mac);

    entry[0] = (system_mac[0] << 8) | system_mac[1];
    entry[1] = (system_mac[2] << 24) | (system_mac[3] << 16) | (system_mac[4] << 8) | system_mac[5];

    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport),
            R_XLMAC_TX_MAC_SA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    return SYS_OK;
}
static sys_error_t
mac_xl_duplex_set(uint8 unit, uint8 lport, BOOL duplex)
{
    return SYS_OK;
}

static sys_error_t
mac_xl_speed_set(uint8 unit, uint8 lport, int speed)
{
    uint32 mode;
    uint32 entry[2];

    switch (speed) {
    case 10:
        mode = SOC_XLMAC_SPEED_10;
        break;
    case 100:
        mode = SOC_XLMAC_SPEED_100;
        break;
    case 1000:
        mode = SOC_XLMAC_SPEED_1000;
        break;
    case 2500:
        mode = SOC_XLMAC_SPEED_2500;
        break;
    case 5000:
        mode = SOC_XLMAC_SPEED_10000;
        break;
    case 0:
        return SYS_OK;              /* Support NULL PHY */
    default:
        if (speed < 10000) {
            return SYS_ERR_PARAMETER;
        }
        mode = SOC_XLMAC_SPEED_10000;
        break;
    }

    /* 
     * Have to disable MAC before setting speed. Assume XLMAC is always in
     * disabled state when setting speed in unmanaged mode.
     */

    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
                       R_XLMAC_MODE(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    entry[0] &= ~(0x7 << 4);
    entry[0] |= (mode << 4);
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
                       R_XLMAC_MODE(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
                       R_XLMAC_RX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* Enable STRICT_PREAMBLEf[Bit 3] if speed >= 10000 */
    if (speed >= 10000 && !IS_HG_PORT(lport)) {
        /* && IS_XE_PORT(unit, port) */
        entry[0] |= (0x1 << 3);
    } else {
        entry[0] &= ~(0x1 << 3);
    }
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
                       R_XLMAC_RX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
                       R_XLMAC_RX_LSS_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* Enable LOCAL_FAULT_DISABLEf[Bit 0] and REMOTE_FAULT_DISABLEf[Bit 1]
     * if speed < 5000 
     */
    if (speed < 5000) {
        entry[0] |= 0x3;
    } else {
        entry[0] &= ~0x3;
    }
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
                       R_XLMAC_RX_LSS_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

#ifdef CFG_TIMESTAMP_MAC_DELAY
    /* Set Timestamp Mac Delays */
    _mac_xl_timestamp_delay_set(unit, port, speed);
#endif
    return SYS_OK;
}
static sys_error_t
mac_xl_pause_set(uint8 unit, uint8 lport, BOOL pause_tx, BOOL pause_rx)
{
    uint32 entry[2];
    /* Bit[18]: RX_PAUSE_EN, Bit[17]: TX_PAUSE_EN */
    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    if (pause_tx) {
        entry[0] |= (0x1 << 17);
    } else {
        entry[0] &= ~(0x1 << 17);
    }

    if (pause_rx) {
        entry[0] |= (0x1 << 18);
    } else {
        entry[0] &= ~(0x1 << 18);
    }

    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    return SYS_OK;
}
/*
 * Function:
 *      mac_xl_loopback_set
 * Purpose:
 *      Set a XLMAC into/out-of loopback mode
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS unit # on unit.
 *      loopback - Boolean: true -> loopback mode, false -> normal operation
 * Note:
 *      On Xlmac, when setting loopback, we enable the TX/RX function also.
 *      Note that to test the PHY, we use the remote loopback facility.
 * Returns:
 *      SOC_E_XXX
 */
static sys_error_t
mac_xl_loopback_set(uint8 unit, uint8 lport, BOOL lb)
{
    uint32 entry[2];
#if 0
    /* need to enable clock compensation for applicable serdes device */
    (void)soc_phyctrl_notify(unit, port, phyEventMacLoopback, lb ? 1 : 0);
#endif

    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
                       R_XLMAC_RX_LSS_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* Set LOCAL_FAULT_DISABLEf[Bit 0] and REMOTE_FAULT_DISABLEf[Bit 1] */
    if (lb) {
        entry[0] |= 0x3;
    } else {
        entry[0] &= ~0x3;
    }
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
                       R_XLMAC_RX_LSS_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* LOCAL_LPBK[Bit 2] = 0x1 */
    if (lb) {
        entry[0] |= 0x4;
    } else {
        entry[0] &= ~0x4;
    }
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    return 0;
}

static sys_error_t
mac_xl_frame_max_set(uint8 unit, uint8 lport, uint32 mtu)
{
    uint32 entry[2];

    entry[1] = 0;
    entry[0] = mtu;
    bcm5333x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
                       R_XLMAC_RX_MAX_SIZE(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    return SYS_OK;
}

static sys_error_t
mac_xl_frame_max_get(uint8 unit, uint8 lport, uint32* mtu)
{
    uint32 entry[2];    

    if (mtu == NULL) {
        return SYS_ERR;
    }
    bcm5333x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    *mtu = entry[0];

    return SYS_OK;
}



mac_driver_t soc_mac_xl = {
    "XLMAC Driver",               /* drv_name */
    mac_xl_init,                  /* md_init  */
    mac_xl_enable_set,            /* md_enable_set */
    mac_xl_duplex_set,            /* md_duplex_set */
    mac_xl_speed_set,             /* md_speed_set */
    mac_xl_pause_set,             /* md_pause_set */
    mac_xl_loopback_set,          /* md_lb_set */
#if 0
    mac_xl_loopback_get,          /* md_lb_get */    
#endif
    mac_xl_frame_max_set,          /* md_frame_max_set */
    mac_xl_frame_max_get,     /* md_frame_max_get */
 }; 



