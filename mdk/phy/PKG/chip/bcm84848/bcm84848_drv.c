/*
 * $Id: bcm84848_drv.c,v 1.3 Broadcom SDK $
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
 * PHY driver for BCM84848.
 *
 */

#include <phy/phy.h>
#include <phy/phy_drvlist.h>
#include <phy/ge_phy.h>
#include <phy/phy_brcm_xe.h>

/* Private PHY flag is used to indicate firmware download method */
#define PHY_F_FW_MDIO_LOAD                      PHY_F_PRIVATE

#define PHY_ID1_REV_MASK                        0x000f

#define BCM84848_PMA_PMD_ID0                    0x600d
#define BCM84848_PMA_PMD_ID1                    0x84f0

#define BCM8484X_C45_DEV_PMA_PMD                MII_C45_DEV_PMA_PMD
#define BCM8484X_C45_DEV_PCS                    MII_C45_DEV_PCS
#define BCM8484X_C45_DEV_AN                     MII_C45_DEV_AN
#define BCM8484X_C45_DEV_PHYXS_M                MII_C45_DEV_PHY_XS
#define BCM8484X_C45_DEV_PHYXS_L                0x03
#define BCM8484X_C45_DEV_TOPLVL                 0x1e

#define C45_DEVAD(_a)                           LSHIFT32((_a),16)
#define DEVAD_PMA_PMD                           C45_DEVAD(BCM8484X_C45_DEV_PMA_PMD)
#define DEVAD_PCS                               C45_DEVAD(BCM8484X_C45_DEV_PCS)
#define DEVAD_AN                                C45_DEVAD(BCM8484X_C45_DEV_AN)
#define DEVAD_PHY_XS_M                          C45_DEVAD(BCM8484X_C45_DEV_PHYXS_M)
#define DEVAD_PHY_XS_L                          C45_DEVAD(BCM8484X_C45_DEV_PHYXS_L)
#define DEVAD_TOPLVL                            C45_DEVAD(BCM8484X_C45_DEV_TOPLVL)

/* MDIO Command Handler (MCH) flags */
#define MCH_F_SPECIAL                   (1L << 31)

/* MDIO commands */
#define MCH_C_NOP                       0x0000
#define MCH_C_GET_PAIR_SWAP             0x8000
#define MCH_C_SET_PAIR_SWAP             0x8001
#define MCH_C_GET_MACSEC_ENABLE         0x8002
#define MCH_C_SET_MACSEC_ENABLE         0x8003
#define MCH_C_GET_1588_ENABLE           0x8004
#define MCH_C_SET_1588_ENABLE           0x8005
#define MCH_C_GET_SHORT_REACH_ENABLE    0x8006
#define MCH_C_SET_SHORT_REACH_ENABLE    0x8007
#define MCH_C_GET_EEE_MODE              0x8008
#define MCH_C_SET_EEE_MODE              0x8009
#define MCH_C_GET_EMI_MODE_ENABLE       0x800a
#define MCH_C_SET_EMI_MODE_ENABLE       0x800b
#define MCH_C_GET_SNR                   0x8030
#define MCH_C_GET_CURRENT_TEMP          0x8031
#define MCH_C_SET_UPPER_TEMP_WARN_LVL   0x8032
#define MCH_C_GET_UPPER_TEMP_WARN_LVL   0x8033
#define MCH_C_SET_LOWER_TEMP_WARN_LVL   0x8034
#define MCH_C_GET_LOWER_TEMP_WARN_LVL   0x8035
#define MCH_C_PEEK_WORD                 0xc000
#define MCH_C_POKE_WORD                 0xc001
#define MCH_C_GET_DATA_BUF_ADDRESSES    0xc002

/* MDIO status values */
#define MCH_S_RECEIVED                  0x0001
#define MCH_S_IN_PROGRESS               0x0002
#define MCH_S_COMPLETE_PASS             0x0004
#define MCH_S_COMPLETE_ERROR            0x0008
#define MCH_S_OPEN_FOR_CMDS             0x0010
#define MCH_S_SYSTEM_BOOT               0x0020
#define MCH_S_NOT_OPEN_FOR_CMDS         0x0040
#define MCH_S_CLEAR_COMPLETE            0x0080
#define MCH_S_OPEN_OVERRIDE             0xA5A5

/***************************************
 * Top level registers 
 ***************************************/
#define PMA_PMD_ID0_REG                         (DEVAD_PMA_PMD + MII_PHY_ID0_REG)
#define PMA_PMD_ID1_REG                         (DEVAD_PMA_PMD + MII_PHY_ID1_REG)
#define TOPLVL_CONFIG_STRAP_REG                 (DEVAD_TOPLVL + 0x401a)

/***********************************************************************
 *
 * PHY DRIVER FUNCTIONS
 *
 ***********************************************************************/

/*
 * Function:
 *      bcm84848_phy_probe
 * Purpose:     
 *      Probe for 84848 PHY
 * Parameters:
 *      pc - PHY control structure
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm84848_phy_probe(phy_ctrl_t *pc)
{
    int ioerr = 0;
    uint32_t phyid0, phyid1;

    PHY_CTRL_CHECK(pc);

    ioerr += PHY_BUS_READ(pc, PMA_PMD_ID0_REG, &phyid0);
    ioerr += PHY_BUS_READ(pc, PMA_PMD_ID1_REG, &phyid1);
    if (ioerr) {
        return CDK_E_IO;
    }

    if ((phyid0 == BCM84848_PMA_PMD_ID0) &&
        ((phyid1 & ~PHY_ID1_REV_MASK) == BCM84848_PMA_PMD_ID1)) {
#if PHY_CONFIG_EXTERNAL_BOOT_ROM == 0
        /* Use MDIO download if external ROM is disabled */
        PHY_CTRL_FLAGS(pc) |= PHY_F_FW_MDIO_LOAD;
#endif
        return ioerr ? CDK_E_IO : CDK_E_NONE;
    }
    return CDK_E_NOT_FOUND;
}

/*
 * Function:
 *      bcm84848_phy_notify
 * Purpose:     
 *      Handle PHY notifications
 * Parameters:
 *      pc - PHY control structure
 *      event - PHY event
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm84848_phy_notify(phy_ctrl_t *pc, phy_event_t event)
{
    PHY_CTRL_CHECK(pc);

    return bcm84846_drv.pd_notify(pc, event);
}

/*
 * Function:
 *      bcm84848_phy_reset
 * Purpose:     
 *      Reset 84848 PHY
 * Parameters:
 *      pc - PHY control structure
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm84848_phy_reset(phy_ctrl_t *pc)
{
    PHY_CTRL_CHECK(pc);

    return bcm84846_drv.pd_reset(pc);
}

/*
 * Function:
 *      bcm84848_phy_init
 * Purpose:     
 *      Initialize 84848 PHY driver
 * Parameters:
 *      pc - PHY control structure
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm84848_phy_init(phy_ctrl_t *pc)
{
    PHY_CTRL_CHECK(pc);

    return bcm84846_drv.pd_init(pc);
}

/*
 * Function:    
 *      bcm84848_phy_link_get
 * Purpose:     
 *      Determine the current link up/down status
 * Parameters:
 *      pc - PHY control structure
 *      link - (OUT) non-zero indicates link established.
 *      autoneg_done - (OUT) if true, auto-negotiation is complete
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm84848_phy_link_get(phy_ctrl_t *pc, int *link, int *autoneg_done)
{
    PHY_CTRL_CHECK(pc);

    return bcm84846_drv.pd_link_get(pc, link, autoneg_done);
}

/*
 * Function:    
 *      bcm84848_phy_duplex_set
 * Purpose:     
 *      Set the current duplex mode (forced).
 * Parameters:
 *      pc - PHY control structure
 *      duplex - non-zero indicates full duplex, zero indicates half
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm84848_phy_duplex_set(phy_ctrl_t *pc, int duplex)
{
    PHY_CTRL_CHECK(pc);

    return bcm84846_drv.pd_duplex_set(pc, duplex);
}

/*
 * Function:    
 *      bcm84848_phy_duplex_get
 * Purpose:     
 *      Get the current operating duplex mode.
 * Parameters:
 *      pc - PHY control structure
 *      duplex - (OUT) non-zero indicates full duplex, zero indicates half
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm84848_phy_duplex_get(phy_ctrl_t *pc, int *duplex)
{
    PHY_CTRL_CHECK(pc);

    return bcm84846_drv.pd_duplex_get(pc, duplex);
}

/*
 * Function:    
 *      bcm84848_phy_speed_set
 * Purpose:     
 *      Set the current operating speed (forced).
 * Parameters:
 *      pc - PHY control structure
 *      speed - new link speed
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm84848_phy_speed_set(phy_ctrl_t *pc, uint32_t speed)
{
    PHY_CTRL_CHECK(pc);

    return bcm84846_drv.pd_speed_set(pc, speed);
}

/*
 * Function:    
 *      bcm84848_phy_speed_get
 * Purpose:     
 *      Get the current operating speed. If autoneg is enabled, 
 *      then operating mode is returned, otherwise forced mode is returned.
 * Parameters:
 *      pc - PHY control structure
 *      speed - (OUT) current link speed
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm84848_phy_speed_get(phy_ctrl_t *pc, uint32_t *speed)
{
    PHY_CTRL_CHECK(pc);

    return bcm84846_drv.pd_speed_get(pc, speed);
}

/*
 * Function:    
 *      bcm84848_phy_autoneg_set
 * Purpose:     
 *      Enable or disable auto-negotiation on the specified port.
 * Parameters:
 *      pc - PHY control structure
 *      autoneg - non-zero enables autoneg, zero disables autoneg
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm84848_phy_autoneg_set(phy_ctrl_t *pc, int autoneg)
{
    PHY_CTRL_CHECK(pc);

    return bcm84846_drv.pd_autoneg_set(pc, autoneg);
}

/*
 * Function:    
 *      bcm84848_phy_autoneg_get
 * Purpose:     
 *      Get the current auto-negotiation setting.
 * Parameters:
 *      pc - PHY control structure
 *      autoneg - (OUT) non-zero indicates autoneg enabled
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm84848_phy_autoneg_get(phy_ctrl_t *pc, int *autoneg)
{
    PHY_CTRL_CHECK(pc);

    return bcm84846_drv.pd_autoneg_get(pc, autoneg);
}

/*
 * Function:    
 *      bcm84848_phy_loopback_set
 * Purpose:     
 *      Set the internal PHY loopback mode.
 * Parameters:
 *      pc - PHY control structure
 *      enable - non-zero enables PHY loopback
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm84848_phy_loopback_set(phy_ctrl_t *pc, int enable)
{
    PHY_CTRL_CHECK(pc);

    return bcm84846_drv.pd_loopback_set(pc, enable);
}

/*
 * Function:    
 *      bcm84848_phy_loopback_get
 * Purpose:     
 *      Get the local PHY loopback mode.
 * Parameters:
 *      pc - PHY control structure
 *      enable - (OUT) non-zero indicates PHY loopback enabled
 * Returns:     
 *      CDK_E_xxx
 * Notes:
 *      Return correct value independently of passthru flag.
 */
static int
bcm84848_phy_loopback_get(phy_ctrl_t *pc, int *enable)
{
    PHY_CTRL_CHECK(pc);

    return bcm84846_drv.pd_loopback_get(pc, enable);
}

/*
 * Function:    
 *      bcm84848_phy_ability_get
 * Purpose:     
 *      Get the abilities of the PHY.
 * Parameters:
 *      pc - PHY control structure
 *      abil - (OUT) ability mask indicating supported options/speeds.
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm84848_phy_ability_get(phy_ctrl_t *pc, uint32_t *abil)
{
    PHY_CTRL_CHECK(pc);

    return bcm84846_drv.pd_ability_get(pc, abil);
}

/*
 * Function:
 *      bcm84848_phy_config_set
 * Purpose:
 *      Modify PHY configuration value.
 * Parameters:
 *      pc - PHY control structure
 *      cfg - Configuration parameter
 *      val - Configuration value
 *      cd - Additional configuration data (if any)
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm84848_phy_config_set(phy_ctrl_t *pc, phy_config_t cfg, uint32_t val, void *cd)
{
    int rv = CDK_E_NONE;
    uint32_t data;
    PHY_CTRL_CHECK(pc);

    switch (cfg) {
    case PhyConfig_Enable: {

        rv += PHY_BUS_READ(pc, TOPLVL_CONFIG_STRAP_REG, &data);
        if (val) {
            data &= ~(1U << 7);
            PHY_CTRL_FLAGS(pc) &= ~PHY_F_PHY_DISABLE;
        } else {
            data |= (1U << 7);
            PHY_CTRL_FLAGS(pc) |= PHY_F_PHY_DISABLE;
        }
        rv += PHY_BUS_WRITE(pc, TOPLVL_CONFIG_STRAP_REG, data);

        return rv;
    }
    default:
        return bcm84846_drv.pd_config_set(pc, cfg, val, cd);
        break;
    }

    return CDK_E_UNAVAIL;
}

/*
 * Function:
 *      bcm84848_phy_config_get
 * Purpose:
 *      Get PHY configuration value.
 * Parameters:
 *      pc - PHY control structure
 *      cfg - Configuration parameter
 *      val - (OUT) Configuration value
 *      cd - (OUT) Additional configuration data (if any)
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm84848_phy_config_get(phy_ctrl_t *pc, phy_config_t cfg, uint32_t *val, void *cd)
{
    PHY_CTRL_CHECK(pc);
    
    switch (cfg) {
    case PhyConfig_Enable: {
        *val = (PHY_CTRL_FLAGS(pc) & PHY_F_PHY_DISABLE) ? 0 : 1;
        return CDK_E_NONE;
    }
    default:
        return bcm84846_drv.pd_config_get(pc, cfg, val, cd);
        break;
    }

    return CDK_E_UNAVAIL;
}

/*
 * Function:
 *      bcm84848_phy_status_get
 * Purpose:
 *      Get PHY status value.
 * Parameters:
 *      pc - PHY control structure
 *      st - Status parameter
 *      val - (OUT) Status value
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm84848_phy_status_get(phy_ctrl_t *pc, phy_status_t st, uint32_t *val)
{
    PHY_CTRL_CHECK(pc);

    return bcm84846_drv.pd_status_get(pc, st, val);
}

#define BCM8484X_ECD_CTRL_STATUSr  (0x001e4006)
#define BCM8484X_ECD_RESULTr       (0x0001a896)
#define BCM8484X_ECD_PAIRA_LENGTHr (0x0001a897)
#define BCM8484X_ECD_PAIRB_LENGTHr (0x0001a898)
#define BCM8484X_ECD_PAIRC_LENGTHr (0x0001a899)
#define BCM8484X_ECD_PAIRD_LENGTHr (0x0001a89a)

int
bcm848xx_phy_cable_diag(phy_ctrl_t *pc, phy_port_cable_diag_t *status)
{
    int i, ioerr, rv;
    uint32_t val, result, clength;

    status->fuzz_len = 10;
    status->npairs = 4;
    status->state = PhyPortCableState_Ok;

    /* run now, not at AN, enable inter pair short check, don't break link, length in meters */
    ioerr = phy_brcm_xe_write(pc, BCM8484X_ECD_CTRL_STATUSr, 0x8400);

    PHY_SYS_USLEEP(100);

    do {
        rv = phy_brcm_xe_read(pc, BCM8484X_ECD_CTRL_STATUSr, &val);
        if (((val & (1U<<11)) == 0) || CDK_FAILURE(rv)) {
            break;
        }
    } while (1); /* timeout? */

    ioerr += phy_brcm_xe_read(pc, BCM8484X_ECD_RESULTr, &result);

    for( i=3; i>=0; i--) {

        switch (result & 0xf) {
        case 0x1:
            status->pair_state[i] = PhyPortCableState_Ok;
            break;
        case 0x2:
            status->pair_state[i] = PhyPortCableState_Open;
            break;
        case 0x3:
            status->pair_state[i] = PhyPortCableState_Short;
            break;
        case 0x4:
            status->pair_state[i] = PhyPortCableState_Crosstalk;
            break;
        default:
            status->pair_state[i] = PhyPortCableState_Unknown;
            break;
        }

        if (status->pair_state[i] > status->state) {
            status->state = status->pair_state[i];
        }

        switch (i) {
        case 3:
            ioerr += phy_brcm_xe_read(pc, BCM8484X_ECD_PAIRD_LENGTHr, &clength);
            break;
        case 2:
            ioerr += phy_brcm_xe_read(pc, BCM8484X_ECD_PAIRC_LENGTHr, &clength);
            break;
        case 1:
            ioerr += phy_brcm_xe_read(pc, BCM8484X_ECD_PAIRB_LENGTHr, &clength);
            break;
        case 0:
            ioerr += phy_brcm_xe_read(pc, BCM8484X_ECD_PAIRA_LENGTHr, &clength);
            break;
        }
        status->pair_len[i] = clength;
        result >>= 4;
    }

    return ioerr ? CDK_E_IO : CDK_E_NONE;
}

/*
 * Variable:    bcm84848_drv
 * Purpose:     PHY Driver for BCM84848.
 */
phy_driver_t bcm84848_drv = {
    "bcm84848",
    "BCM84848 10GbE PHY Driver",  
    0,
    bcm84848_phy_probe,                  /* pd_probe */
    bcm84848_phy_notify,                 /* pd_notify */
    bcm84848_phy_reset,                  /* pd_reset */
    bcm84848_phy_init,                   /* pd_init */
    bcm84848_phy_link_get,               /* pd_link_get */
    bcm84848_phy_duplex_set,             /* pd_duplex_set */
    bcm84848_phy_duplex_get,             /* pd_duplex_get */
    bcm84848_phy_speed_set,              /* pd_speed_set */
    bcm84848_phy_speed_get,              /* pd_speed_get */
    bcm84848_phy_autoneg_set,            /* pd_autoneg_set */
    bcm84848_phy_autoneg_get,            /* pd_autoneg_get */
    bcm84848_phy_loopback_set,           /* pd_loopback_set */
    bcm84848_phy_loopback_get,           /* pd_loopback_get */
    bcm84848_phy_ability_get,            /* pd_ability_get */
    bcm84848_phy_config_set,             /* pd_config_set */
    bcm84848_phy_config_get,             /* pd_config_get */
    bcm84848_phy_status_get,             /* pd_status_get */
    bcm848xx_phy_cable_diag              /* pd_cable_diag */
};
