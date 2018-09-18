/*
 * $Id: bcm54880e_drv.c,v 1.2 Broadcom SDK $
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

#include "phy/ge_phy.h"
#include "phy/phy_brcm_shadow.h"

/* FUNCTIONS prototype */
static int
bcm54880e_phy_cl45_reg_modify(phy_ctrl_t *pc,
                                     uint32_t dev_addr, uint32_t reg_addr,
                                     uint32_t val, uint32_t mask);

extern phy_driver_t bcm54680e_drv;

#define BCM54880E_PHY_ID0               0x0362
#define BCM54880E_PHY_ID1               0x5d70

#define PHY_ID1_REV_MASK                0x000f

/* Default LED control */
#define BCM54880E_LED1_SEL(_pc)          0xA
#define BCM54880E_LED2_SEL(_pc)          0x3
#define BCM54880E_LED3_SEL(_pc)          0x3
#define BCM54880E_LED4_SEL(_pc)          0x6
#define BCM54880E_LEDCTRL(_pc)           0x8
#define BCM54880E_LEDSELECT(_pc)         0x8

/* Access to shadowed registers at offset 0x1c */
#define REG_1C_SEL(_s)                  ((_s) << 10)
#define REG_1C_WR(_s,_v)                (REG_1C_SEL(_s) | (_v) | 0x8000)

/* Access expansion registers at offset 0x15 */
#define MII_EXP_MAP_REG(_r)             ((_r) | 0x0f00)
#define MII_EXP_UNMAP                   (0)

/*
 * Non-standard MII Registers
 */
#define MII_ECR_REG             0x10 /* MII Extended Control Register */
#define MII_EXP_REG             0x15 /* MII Expansion registers */
#define MII_EXP_SEL             0x17 /* MII Expansion register select */
#define MII_TEST1_REG           0x1e /* MII Test Register 1 */

/* EEE Advertisement Registers */
#define MODIFY_PHY54880E_EEE_ADVr(_pc, _val, _mask) \
        bcm54880e_phy_cl45_reg_modify(_pc, 0x7, 0x3c, _val, _mask)
#define MODIFY_PHY54880E_EEE_803Dr(_pc, _val, _mask) \
        bcm54880e_phy_cl45_reg_modify(_pc, 0x7, 0x803d, _val, _mask)

/***********************************************************************
 *
 * PHY DRIVER FUNCTIONS
 *
 ***********************************************************************/

/*
 * Function:
 *      bcm54880e_phy_probe
 * Purpose:
 *      Probe for PHY
 * Parameters:
 *      pc - PHY control structure
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm54880e_phy_probe(phy_ctrl_t *pc)
{
    uint32_t phyid0, phyid1;
    int ioerr = 0;

    PHY_CTRL_CHECK(pc);

    ioerr += PHY_BUS_READ(pc, MII_PHY_ID0_REG, &phyid0);
    ioerr += PHY_BUS_READ(pc, MII_PHY_ID1_REG, &phyid1);

    if (phyid0 == BCM54880E_PHY_ID0 &&
        (phyid1 & ~PHY_ID1_REV_MASK) == BCM54880E_PHY_ID1) {
        return ioerr ? CDK_E_IO : CDK_E_NONE;
    }
    return CDK_E_NOT_FOUND;
}

/*
 * Function:
 *      bcm54880e_phy_notify
 * Purpose:
 *      Handle PHY notifications
 * Parameters:
 *      pc - PHY control structure
 *      event - PHY event
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm54880e_phy_notify(phy_ctrl_t *pc, phy_event_t event)
{
    return bcm54680e_drv.pd_notify(pc, event);
}

/*
 * Function:
 *      bcm54880e_phy_reset
 * Purpose:
 *      Reset PHY
 * Parameters:
 *      pc - PHY control structure
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm54880e_phy_reset(phy_ctrl_t *pc)
{
    return bcm54680e_drv.pd_reset(pc);
}

static int
bcm54880e_phy_cl45_reg_read(phy_ctrl_t *pc, uint32_t dev_addr,
                                  uint32_t reg_addr, uint32_t *val)
{
    int ioerr = 0;

    /* Write Device Address to register 0x0D (Set Function field to Address)*/
    ioerr += PHY_BUS_WRITE(pc, 0x0D, (dev_addr & 0x001f));

    /* Select the register by writing to register address to register 0x0E */
    ioerr += PHY_BUS_WRITE(pc, 0x0E, reg_addr);

    /* Write Device Address to register 0x0D (Set Function field to Data)*/
    ioerr += PHY_BUS_WRITE(pc, 0x0D,
                         ((0x4000) | (dev_addr & 0x001f)));

    /* Read register 0x0E to get the value */
    ioerr += PHY_BUS_READ(pc, 0x0E, val);

    return ioerr ? CDK_E_IO : CDK_E_NONE;
}

static int
bcm54880e_phy_cl45_reg_write(phy_ctrl_t *pc, uint32_t dev_addr,
                                   uint32_t reg_addr, uint32_t val)
{
    int ioerr = 0;

    /* Write Device Address to register 0x0D (Set Function field to Address)*/
    ioerr += PHY_BUS_WRITE(pc, 0x0D, (dev_addr & 0x001f));

    /* Select the register by writing to register address to register 0x0E */
    ioerr += PHY_BUS_WRITE(pc, 0x0E, reg_addr);

    /* Write Device Address to register 0x0D (Set Function field to Data)*/
    ioerr += PHY_BUS_WRITE(pc, 0x0D,
                         ((0x4000) | (dev_addr & 0x001f)));

    /* Write register 0x0E to write the value */
    ioerr += PHY_BUS_WRITE(pc, 0x0E, val);

    return ioerr ? CDK_E_IO : CDK_E_NONE;
}

static int
bcm54880e_phy_cl45_reg_modify(phy_ctrl_t *pc,
                                     uint32_t dev_addr, uint32_t reg_addr,
                                     uint32_t val, uint32_t mask)
{
    int ioerr = 0;
    uint32_t value = 0;

    ioerr += bcm54880e_phy_cl45_reg_read(pc, dev_addr, reg_addr, &value);

    value = (val & mask) | (value & ~mask);

    ioerr += bcm54880e_phy_cl45_reg_write(pc, dev_addr, reg_addr, value);

    return ioerr ? CDK_E_IO : CDK_E_NONE;
}

#define SOC_IF_ERROR_RETURN(op) \
    do { int __rv__; if ((__rv__ = (op)) != 0x0) return(__rv__); } while(0)

static int
bcm54880e_phy_eee_init(phy_ctrl_t *pc)
{
    int ioerr = 0;
    uint32_t value = 0;

    /* Enable native EEE mode becaue the native EEE mode is disabled by default */
    SOC_IF_ERROR_RETURN(
           MODIFY_PHY54880E_EEE_803Dr(pc, 0xc000, 0xc000)); /* 7.803d */
    SOC_IF_ERROR_RETURN(
           MODIFY_PHY54880E_EEE_ADVr(pc, 0x0006, 0x0006));
    /* exp af : phy_reg_ge_modify(unit, port, 0x00, 0x0FAF,0x15, 0x1, 0x1)); */
    ioerr += PHY_BUS_WRITE(pc, 0x17, 0x0FAF);
    ioerr += PHY_BUS_READ(pc, 0x15, &value);
    ioerr += PHY_BUS_WRITE(pc, 0x15, (value|0x1));

    /*
     * work around for 100Base-Tx clock ppm issue during link acquisition
     * Solution : increase the bandwidth setting during during link acquisition
     */
    ioerr += PHY_BUS_WRITE(pc, 0x18, 0x4c00);
    ioerr += PHY_BUS_WRITE(pc, 0x17, 0x4022);
    ioerr += PHY_BUS_WRITE(pc, 0x15, 0x017B);
    ioerr += PHY_BUS_WRITE(pc, 0x18, 0x4400);

    return ioerr ? CDK_E_IO : CDK_E_NONE;
}

int bcm54680_phy_cable_diag_init(uint8_t unit, phy_ctrl_t *pc);

/*
 * Function:
 *      bcm54880e_phy_init
 * Purpose:
 *      Initialize PHY driver
 * Parameters:
 *      pc - PHY control structure
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm54880e_phy_init(phy_ctrl_t *pc)
{
    uint32_t ctrl, tmp;
    int ioerr = 0;
    int rv = CDK_E_NONE;

    PHY_CTRL_CHECK(pc);

    /* Power up copper interface */
    ioerr += PHY_BUS_READ(pc, MII_CTRL_REG, &ctrl);
    ctrl &= ~MII_CTRL_PD;
    ioerr += PHY_BUS_WRITE(pc, MII_CTRL_REG, ctrl);

    /* Set port mode */
    ioerr += PHY_BUS_READ(pc, MII_GB_CTRL_REG, &ctrl);
    ctrl |= MII_GB_CTRL_PT;
    ioerr += PHY_BUS_WRITE(pc, MII_GB_CTRL_REG, ctrl);

    /* Enable link speed LED mode */
    ioerr += PHY_BUS_WRITE(pc, 0x1c, REG_1C_SEL(0x2));
    ioerr += PHY_BUS_READ(pc, 0x1c, &tmp);
    tmp |= 0x0004;
    ioerr += PHY_BUS_WRITE(pc, 0x1c, REG_1C_WR(0x2, tmp));

    /* Configure Extended Control Register */
    ioerr += PHY_BUS_READ(pc, MII_ECR_REG, &tmp);
    /* Enable LEDs to indicate traffic status */
    tmp |= 0x0020;
    ioerr += PHY_BUS_WRITE(pc, MII_ECR_REG, tmp);

    /* Enable extended packet length (4.5k through 25k) */
    ioerr += PHY_BUS_WRITE(pc, 0x18, 0x0007);
    ioerr += PHY_BUS_READ(pc, 0x18, &tmp);
    tmp |= 0x4000;
    ioerr += PHY_BUS_WRITE(pc, 0x18, tmp);

    /* Configure LED selectors */
    ioerr += PHY_BUS_WRITE(pc, 0x1c,
                           REG_1C_WR(0x0d, BCM54880E_LED1_SEL(pc) |
                                     (BCM54880E_LED2_SEL(pc) << 4)));
    ioerr += PHY_BUS_WRITE(pc, 0x1c,
                           REG_1C_WR(0x0e, BCM54880E_LED3_SEL(pc) |
                                     (BCM54880E_LED4_SEL(pc) << 4)));
    ioerr += PHY_BUS_WRITE(pc, 0x1c,
                           REG_1C_WR(0x09, BCM54880E_LEDCTRL(pc)));
    ioerr += PHY_BUS_WRITE(pc, MII_EXP_SEL, MII_EXP_MAP_REG(0x4));
    ioerr += PHY_BUS_WRITE(pc, MII_EXP_REG, BCM54880E_LEDSELECT(pc));
    ioerr += PHY_BUS_WRITE(pc, MII_EXP_SEL, MII_EXP_UNMAP);
    /* If using LED link/activity mode, disable LED traffic mode */
    if ((BCM54880E_LEDCTRL(pc) & 0x10) || BCM54880E_LEDSELECT(pc) == 0x01) {
        ioerr += PHY_BUS_READ(pc, MII_ECR_REG, &tmp);
        tmp &= ~0x0020;
        ioerr += PHY_BUS_WRITE(pc, MII_ECR_REG, tmp);
    }

#ifdef CFG_SWITCH_LOOPDETECT_INCLUDED
    /* 2 blinks per second for loop detection */
    ioerr += PHY_BUS_WRITE(pc, 0x17, 0xf06);
    ioerr += PHY_BUS_WRITE(pc, 0x15, 0x2);
#endif

    /* Advertise pause capability */
    ioerr += PHY_BUS_READ(pc, MII_ANA_REG, &ctrl);
    ctrl |= MII_ANA_PAUSE;
    ioerr += PHY_BUS_WRITE(pc, MII_ANA_REG, ctrl);

    /* Restart auto-negotiation */
    ioerr += PHY_BUS_READ(pc, MII_CTRL_REG, &ctrl);
    ctrl |= MII_CTRL_RAN;
    ioerr += PHY_BUS_WRITE(pc, MII_CTRL_REG, ctrl);

    ioerr += bcm54880e_phy_eee_init(pc);
    ioerr += bcm54680_phy_cable_diag_init(pc->unit, pc);

    /* Call up the PHY chain */
    if (CDK_SUCCESS(rv)) {
        rv = PHY_INIT(PHY_CTRL_NEXT(pc));
    }

    /* Set default medium */
    if (CDK_SUCCESS(rv)) {
        PHY_NOTIFY(pc, PhyEvent_ChangeToCopper);
    }

    return ioerr ? CDK_E_IO : rv;
}

/*
 * Function:
 *      bcm54880e_phy_link_get
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
bcm54880e_phy_link_get(phy_ctrl_t *pc, int *link, int *autoneg_done)
{
    return bcm54680e_drv.pd_link_get(pc, link, autoneg_done);
}

/*
 * Function:
 *      bcm54880e_phy_duplex_set
 * Purpose:
 *      Set the current duplex mode (forced).
 * Parameters:
 *      pc - PHY control structure
 *      duplex - non-zero indicates full duplex, zero indicates half
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm54880e_phy_duplex_set(phy_ctrl_t *pc, int duplex)
{
    return bcm54680e_drv.pd_duplex_set(pc, duplex);
}

/*
 * Function:
 *      bcm54880e_phy_duplex_get
 * Purpose:
 *      Get the current operating duplex mode. If autoneg is enabled,
 *      then operating mode is returned, otherwise forced mode is returned.
 * Parameters:
 *      pc - PHY control structure
 *      duplex - (OUT) non-zero indicates full duplex, zero indicates half
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm54880e_phy_duplex_get(phy_ctrl_t *pc, int *duplex)
{
    return bcm54680e_drv.pd_duplex_get(pc, duplex);
}

/*
 * Function:
 *      bcm54880e_phy_speed_set
 * Purpose:
 *      Set the current operating speed (forced).
 * Parameters:
 *      pc - PHY control structure
 *      speed - new link speed
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm54880e_phy_speed_set(phy_ctrl_t *pc, uint32_t speed)
{
    return bcm54680e_drv.pd_speed_set(pc, speed);
}

/*
 * Function:
 *      bcm54880e_phy_speed_get
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
bcm54880e_phy_speed_get(phy_ctrl_t *pc, uint32_t *speed)
{
    return bcm54680e_drv.pd_speed_get(pc, speed);
}

/*
 * Function:
 *      bcm54880e_phy_autoneg_set
 * Purpose:
 *      Enable or disabled auto-negotiation on the specified port.
 * Parameters:
 *      pc - PHY control structure
 *      autoneg - non-zero enables autoneg, zero disables autoneg
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm54880e_phy_autoneg_set(phy_ctrl_t *pc, int autoneg)
{
    return bcm54680e_drv.pd_autoneg_set(pc, autoneg);
}

/*
 * Function:
 *      bcm54880e_phy_autoneg_get
 * Purpose:
 *      Get the current auto-negotiation status (enabled/busy)
 * Parameters:
 *      pc - PHY control structure
 *      autoneg - (OUT) non-zero indicates autoneg enabled
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm54880e_phy_autoneg_get(phy_ctrl_t *pc, int *autoneg)
{
    return bcm54680e_drv.pd_autoneg_get(pc, autoneg);
}

/*
 * Function:
 *      bcm54880e_phy_loopback_set
 * Purpose:
 *      Set the internal PHY loopback mode.
 * Parameters:
 *      pc - PHY control structure
 *      enable - non-zero enables PHY loopback
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm54880e_phy_loopback_set(phy_ctrl_t *pc, int enable)
{
    return bcm54680e_drv.pd_loopback_set(pc, enable);
}

/*
 * Function:
 *      bcm54880e_phy_loopback_get
 * Purpose:
 *      Get the local PHY loopback mode.
 * Parameters:
 *      pc - PHY control structure
 *      enable - (OUT) non-zero indicates PHY loopback enabled
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm54880e_phy_loopback_get(phy_ctrl_t *pc, int *enable)
{
    return bcm54680e_drv.pd_loopback_get(pc, enable);
}

/*
 * Function:
 *      bcm54880e_phy_ability_get
 * Purpose:
 *      Get the abilities of the PHY.
 * Parameters:
 *      pc - PHY control structure
 *      abil - (OUT) ability mask indicating supported options/speeds.
 * Returns:
 *      CDK_E_xxx
 */
static int
bcm54880e_phy_ability_get(phy_ctrl_t *pc, uint32_t *abil)
{
    return bcm54680e_drv.pd_ability_get(pc, abil);
}

/*
 * Function:
 *      bcm54880e_phy_config_set
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
bcm54880e_phy_config_set(phy_ctrl_t *pc, phy_config_t cfg, uint32_t val, void *cd)
{
    PHY_CTRL_CHECK(pc);

    switch (cfg) {
    case PhyConfig_Enable: {
            int ioerr = 0;
            uint32_t ctrl;
            
            ioerr += PHY_BUS_READ(pc, MII_CTRL_REG, &ctrl);
            if (val) {
                ctrl &= ~MII_CTRL_PD;
            } else {
                ctrl |= MII_CTRL_PD;
            }
            ioerr += PHY_BUS_WRITE(pc, MII_CTRL_REG, ctrl);

            return ioerr;
        }
    default:
        return bcm54680e_drv.pd_config_set(pc, cfg, val, cd);
        break;
    }

    return CDK_E_UNAVAIL;
}

/*
 * Function:
 *      bcm54880e_phy_config_get
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
bcm54880e_phy_config_get(phy_ctrl_t *pc, phy_config_t cfg, uint32_t *val, void *cd)
{
    PHY_CTRL_CHECK(pc);

    switch (cfg) {
    case PhyConfig_Enable: {
        int ioerr = 0;
        uint32_t ctrl;
        
        ioerr += PHY_BUS_READ(pc, MII_CTRL_REG, &ctrl);
        if (ctrl & MII_CTRL_PD) {
            *val = 0;
        } else {
            *val = 1;
        }

        return ioerr;
        }
    default:
        return bcm54680e_drv.pd_config_get(pc, cfg, val, cd);
        break;
    }
    return CDK_E_UNAVAIL;
    
}

int bcm54680_phy_cable_diag(phy_ctrl_t *pc, phy_port_cable_diag_t *status);

/*
 * Variable:    bcm54880e_phy drv
 * Purpose:     PHY Driver for BCM54880E.
 */
phy_driver_t bcm54880e_drv = {
    "bcm54880e",
    "BCM54880E Gigabit PHY Driver (EEE)",
    0,
    bcm54880e_phy_probe,                /* pd_probe */
    bcm54880e_phy_notify,               /* pd_notify */
    bcm54880e_phy_reset,                /* pd_reset */
    bcm54880e_phy_init,                 /* pd_init */
    bcm54880e_phy_link_get,             /* pd_link_get */
    bcm54880e_phy_duplex_set,           /* pd_duplex_set */
    bcm54880e_phy_duplex_get,           /* pd_duplex_get */
    bcm54880e_phy_speed_set,            /* pd_speed_set */
    bcm54880e_phy_speed_get,            /* pd_speed_get */
    bcm54880e_phy_autoneg_set,          /* pd_autoneg_set */
    bcm54880e_phy_autoneg_get,          /* pd_autoneg_get */
    bcm54880e_phy_loopback_set,         /* pd_loopback_set */
    bcm54880e_phy_loopback_get,         /* pd_loopback_get */
    bcm54880e_phy_ability_get,          /* pd_ability_get */
    bcm54880e_phy_config_set,           /* pd_config_set */
    bcm54880e_phy_config_get,           /* pd_config_get */
    NULL,                               /* pd_status_get */
    bcm54680_phy_cable_diag            /* pd_cable_diag */
};
