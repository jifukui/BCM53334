/*
 * $Id: bcm54210_drv.c,v 1.5 Broadcom SDK $
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
 * PHY driver for BCM54210.
 *
 */

#include <phy/phy.h>
#include <phy/ge_phy.h>
#include <phy/phy_drvlist.h>
#include <phy/chip/bcm54282_defs.h>

#define BCM54210_PHY_ID0                0x600d
#define BCM54210_PHY_ID1                0x8590

#define PHY_ID1_REV_MASK                0x000f

/* Lane from PHY control instance */
#define LANE_NUM_MASK                   0x7

/* Default LED control */
#define BCM54210_LED1_SEL(_pc)          0x0
#define BCM54210_LED2_SEL(_pc)          0x1
#define BCM54210_LED3_SEL(_pc)          0x3
#define BCM54210_LED4_SEL(_pc)          0x6
#define BCM54210_LEDCTRL(_pc)           0x8
#define BCM54210_LEDSELECT(_pc)         0x0

#define PHY_IS_BCM54210(_phyid0, _phyid1) \
                (((_phyid0) == BCM54210_PHY_ID0) && \
                 (((_phyid1) & ~PHY_ID1_REV_MASK) == BCM54210_PHY_ID1))

#define PAUSE_MODE_NO_PAUSE             0x0
#define PAUSE_MODE_SYM_PAUSE            0x1
#define PAUSE_MODE_ASYM_PAUSE_LP        0x2
#define PAUSE_MODE_SYM_ASYM_PAUSE_LD    0x3

#define MODE_SEL_COPPER_2_SGMII         0x0
#define MODE_SEL_FIBER_2_SGMII          0x1
#define MODE_SEL_SGMII_2_COPPER         0x2
#define MODE_SEL_GBIC                   0x3

#define SECOND_SERDES_MII_CONTROLr      0x00000b00
#define EXTERNAL_SERDES_CONTROLr        0x00000234

/***********************************************************************
 *
 * HELPER FUNCTIONS
 *
 ***********************************************************************/

/*
 * Function:
 *      _bcm54210_inst
 * Purpose:
 *      Retrieve PHY instance.
 * Parameters:
 *      pc - PHY control structure
 * Returns:
 *      PHY instance
 */
static int
_bcm54210_inst(phy_ctrl_t *pc)
{
    int inst = PHY_CTRL_PHY_INST(pc);

    if (inst < 0) {
        uint32_t addr = PHY_CTRL_PHY_ADDR(pc);

        while (addr > 8) {
            addr -= 8;
        }
        inst = addr - 1;
    }
    return inst & LANE_NUM_MASK;
}

/***********************************************************************
 *
 * PHY DRIVER FUNCTIONS
 *
 ***********************************************************************/

#if PHY_CONFIG_INCLUDE_CHIP_SYMBOLS == 1
extern cdk_symbols_t bcm54282_symbols;
#endif

/*
 * Function:
 *      bcm54210_phy_probe
 * Purpose:     
 *      Probe for PHY
 * Parameters:
 *      pc - PHY control structure
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm54210_phy_probe(phy_ctrl_t *pc)
{
    uint32_t phyid0, phyid1;
    int ioerr = 0;

    PHY_CTRL_CHECK(pc);

    ioerr += PHY_BUS_READ(pc, MII_PHY_ID0_REG, &phyid0);
    ioerr += PHY_BUS_READ(pc, MII_PHY_ID1_REG, &phyid1);
    if (PHY_IS_BCM54210(phyid0, phyid1)) {
#if PHY_CONFIG_INCLUDE_CHIP_SYMBOLS == 1
        PHY_CTRL_SYMBOLS(pc) = &bcm54282_symbols;
#endif
        /* Default is copper mode */
        PHY_CTRL_FLAGS(pc) &= ~PHY_F_FIBER_MODE;

        return ioerr ? CDK_E_IO : CDK_E_NONE;
    }
    return CDK_E_NOT_FOUND;
}

/*
 * Function:
 *      bcm54210_phy_notify
 * Purpose:     
 *      Handle PHY notifications
 * Parameters:
 *      pc - PHY control structure
 *      event - PHY event
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm54210_phy_notify(phy_ctrl_t *pc, phy_event_t event)
{
    int rv = CDK_E_NONE;

    PHY_CTRL_CHECK(pc);

    switch (event) {
    case PhyEvent_ChangeToCopper:
        PHY_CTRL_FLAGS(pc) &= ~PHY_F_FIBER_MODE;
        /* Upstream PHY should operate in passthru mode */
        event = PhyEvent_ChangeToPassthru;
        break;
    case PhyEvent_ChangeToFiber:
        PHY_CTRL_FLAGS(pc) |= PHY_F_FIBER_MODE;
        /* Upstream PHY should remain in SGMII mode */
        event = PhyEvent_ChangeToPassthru;
        break;
    default:
        break;
    }

    /* Call up the PHY chain */
    rv = PHY_NOTIFY(PHY_CTRL_NEXT(pc), event);
    if (CDK_FAILURE(rv)) {
        return rv;
    }

    /* Upstream PHY must disable autoneg in passthru mode */
    if (event == PhyEvent_ChangeToPassthru) {
        rv = PHY_AUTONEG_SET(PHY_CTRL_NEXT(pc), 0);
        if (CDK_FAILURE(rv)) {
            return rv;
        }
    }

    return CDK_E_NONE;
}

/*
 * Function:
 *      bcm54210_phy_reset
 * Purpose:     
 *      Reset PHY
 * Parameters:
 *      pc - PHY control structure
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm54210_phy_reset(phy_ctrl_t *pc)
{
    return bcm54282_drv.pd_reset(pc);
}

/*
 * Function:
 *      bcm54210_phy_init
 * Purpose:     
 *      Initialize PHY driver
 * Parameters:
 *      pc - PHY control structure
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm54210_phy_init(phy_ctrl_t *pc)
{
    int ioerr = 0;
    int rv = CDK_E_NONE;
    TOP_MISC_GLOBAL_RESETr_t top_misc_global_reset;
    MII_AUX_CTRLr_t mii_aux_ctrl;
    DSP_TAP10r_t dsp_tap10;
    MODE_CTRLr_t mode_ctrl;
    MII_CTRLr_t mii_ctrl;
    MII_GB_CTRLr_t mii_gb_ctrl;
    SGMII_SLAVEr_t sgmii_slave;
    MII_1000X_CTRLr_t mii_1000x_ctrl;
    MII_1000X_ANAr_t mii_1000x_ana;
    POWER_MII_CTRLr_t power_mii_ctrl;
    AUTO_DETECT_MEDIUMr_t auto_detect_medium;
    MISC_1000X_CTRL_2r_t misc_ctrl_2;
    AUX_1000X_CTRLr_t aux_1000x_ctrl;
    EEE_803Dr_t eee_803d;
    EEE_ADVr_t eee_adv;
    EEE_STAT_CTRLr_t eee_stat_ctrl;
    MII_BUFFER_CONTROL_0r_t mii_buf_ctrl;
    LED_GPIO_CTRLr_t led_gpio_ctrl;
    LED_SELECTOR_1r_t led_sel_1;
    LED_SELECTOR_2r_t led_sel_2;
    LED_CTRLr_t led_ctrl;
    EXP_LED_SELECTORr_t exp_led_selector;
    SPARE_CTRLr_t spare_ctrl;
    EXP_PATT_GEN_STATr_t exp_patt_gen_stat;
    MII_PHY_ECRr_t mii_phy_ecr;
    int orig_inst;
    uint32_t led_val;

    PHY_CTRL_CHECK(pc);

    /* Get primary PHY instance */
    orig_inst = _bcm54210_inst(pc);

    /* Reset Top level register block*/
    TOP_MISC_GLOBAL_RESETr_CLR(top_misc_global_reset);
    TOP_MISC_GLOBAL_RESETr_TOP_MII_REG_SOFT_RSTf_SET(top_misc_global_reset, 1);
    TOP_MISC_GLOBAL_RESETr_RESET_1588f_SET(top_misc_global_reset, 1);
    ioerr += WRITE_TOP_MISC_GLOBAL_RESETr(pc, top_misc_global_reset);

    /* Recommended DSP_TAP10 setting for all the revisions */
    MII_AUX_CTRLr_CLR(mii_aux_ctrl);
    MII_AUX_CTRLr_ENABLE_DSP_CLOCKf_SET(mii_aux_ctrl, 1);
    MII_AUX_CTRLr_RESERVEDf_SET(mii_aux_ctrl, 1);
    ioerr += WRITE_MII_AUX_CTRLr(pc, mii_aux_ctrl);

    DSP_TAP10r_SET(dsp_tap10, 0x011B);
    ioerr += WRITE_DSP_TAP10r(pc, dsp_tap10);

    /* Disable Super Isolate */
    ioerr += READ_POWER_MII_CTRLr(pc, &power_mii_ctrl);
    POWER_MII_CTRLr_SUPER_ISOLATEf_SET(power_mii_ctrl, 0);
    ioerr += WRITE_POWER_MII_CTRLr(pc, power_mii_ctrl);

    /* Enable current mode LED */
    LED_GPIO_CTRLr_CLR(led_gpio_ctrl);
    LED_GPIO_CTRLr_WRITE_ENf_SET(led_gpio_ctrl, 1);
    LED_GPIO_CTRLr_SHD1C_SELf_SET(led_gpio_ctrl, 0xf);
    ioerr += WRITE_LED_GPIO_CTRLr(pc, led_gpio_ctrl);

    /* Copper Interface */
    ioerr += READ_MII_CTRLr(pc, &mii_ctrl);
    MII_CTRLr_POWER_DOWNf_SET(mii_ctrl, 0);
    ioerr += WRITE_MII_CTRLr(pc, mii_ctrl);

    MII_GB_CTRLr_CLR(mii_gb_ctrl);
    MII_GB_CTRLr_CAP_1000BASE_T_FDf_SET(mii_gb_ctrl, 1);
    MII_GB_CTRLr_SWITCH_DEVf_SET(mii_gb_ctrl, 1);
    ioerr += WRITE_MII_GB_CTRLr(pc, mii_gb_ctrl);

    MII_CTRLr_CLR(mii_ctrl);
    MII_CTRLr_FULL_DUPLEXf_SET(mii_ctrl, 1);
    MII_CTRLr_RESTART_ANf_SET(mii_ctrl, 1);
    MII_CTRLr_AUTONEGf_SET(mii_ctrl, 1);
    MII_CTRLr_SPEED_SEL0f_SET(mii_ctrl, 1);
    ioerr += WRITE_MII_CTRLr(pc, mii_ctrl);

    /* Disable auto-detection between SGMII-slave and 1000BASE-X */
    ioerr += READ_SGMII_SLAVEr(pc, &sgmii_slave);
    SGMII_SLAVEr_SGMII_SLAVE_AUTO_DETECTf_SET(sgmii_slave, 0);
    ioerr += WRITE_SGMII_SLAVEr(pc, sgmii_slave);

    /* Power down SerDes */
    ioerr += READ_MII_1000X_CTRLr(pc, &mii_1000x_ctrl);
    MII_1000X_CTRLr_POWER_DOWNf_SET(mii_1000x_ctrl, 1);
    ioerr += WRITE_MII_1000X_CTRLr(pc, mii_1000x_ctrl);

    /* Fiber Interface */
    /* remove power down of SerDes */
    ioerr += READ_MII_1000X_CTRLr(pc, &mii_1000x_ctrl);
    MII_1000X_CTRLr_POWER_DOWNf_SET(mii_1000x_ctrl, 0);
    ioerr += WRITE_MII_1000X_CTRLr(pc, mii_1000x_ctrl);

    /* set the advertisement of serdes */
    ioerr += READ_MII_1000X_ANAr(pc, &mii_1000x_ana);
    MII_1000X_ANAr_CAP_1000BASE_X_FDf_SET(mii_1000x_ana, 1);
    MII_1000X_ANAr_CAP_1000BASE_X_HDf_SET(mii_1000x_ana, 1);
    MII_1000X_ANAr_PAUSEf_SET(mii_1000x_ana, PAUSE_MODE_SYM_ASYM_PAUSE_LD);
    ioerr += WRITE_MII_1000X_ANAr(pc, mii_1000x_ana);

    /* Default SerDes config & restart autonegotiation */
    MII_1000X_CTRLr_CLR(mii_1000x_ctrl);
    MII_1000X_CTRLr_FULL_DUPLEXf_SET(mii_1000x_ctrl, 1);
    MII_1000X_CTRLr_SS_MSBf_SET(mii_1000x_ctrl, 1);
    MII_1000X_CTRLr_AUTONEGf_SET(mii_1000x_ctrl, 1);
    MII_1000X_CTRLr_RESTART_ANf_SET(mii_1000x_ctrl, 1);
    ioerr += WRITE_MII_1000X_CTRLr(pc, mii_1000x_ctrl);

    /* Configure Auto-detect Medium */
    ioerr += READ_AUTO_DETECT_MEDIUMr(pc, &auto_detect_medium);
    SGMII_AUTO_DETECT_MEDIUMr_AUTO_DETECT_MEDIA_ENABLEf_SET(auto_detect_medium, 1);
    AUTO_DETECT_MEDIUMr_AUTO_DETECT_MEDIA_PRIORITYf_SET(auto_detect_medium, 1);
    AUTO_DETECT_MEDIUMr_AUTO_DETECT_MEDIA_DEFAULTf_SET(auto_detect_medium, 1);
    AUTO_DETECT_MEDIUMr_QFSD_WITH_SYNC_STATUSf_SET(auto_detect_medium, 1);
    AUTO_DETECT_MEDIUMr_SIGNAL_DETECT_INVERTf_SET(auto_detect_medium, 1);
    ioerr += WRITE_AUTO_DETECT_MEDIUMr(pc, auto_detect_medium);

    ioerr += READ_MODE_CTRLr(pc, &mode_ctrl);
    MODE_CTRLr_MODE_SELf_SET(mode_ctrl, MODE_SEL_SGMII_2_COPPER);
    ioerr += WRITE_MODE_CTRLr(pc, mode_ctrl);

    /* Power-up the secondary serdes interface */
    ioerr += phy_brcm_rdb_write(pc, SECOND_SERDES_MII_CONTROLr, 0x1140);

    /* Write RDB Register 0x234 = 0x518F (54210S-An3000-R) */
    ioerr += phy_brcm_rdb_write(pc, EXTERNAL_SERDES_CONTROLr, 0x518F);

    /* Configure LED mode and turn on jumbo frame support */
    ioerr += READ_MII_AUX_CTRLr(pc, &mii_aux_ctrl);
    MII_AUX_CTRLr_EXT_PKT_LENf_SET(mii_aux_ctrl, 1);
    ioerr += WRITE_MII_AUX_CTRLr(pc, mii_aux_ctrl);

    /* Enable extended packet length (4.5k through 25k) */
    ioerr += READ_EXP_PATT_GEN_STATr(pc, &exp_patt_gen_stat);
    EXP_PATT_GEN_STATr_GMII_RGMII_FIFO_ELASTICITY_1f_SET(exp_patt_gen_stat, 1);
    ioerr += WRITE_EXP_PATT_GEN_STATr(pc, exp_patt_gen_stat);

    /* Enable traffic LED mode and set FIFO elasticity to 10K (jumbo frames) */
    ioerr += READ_MII_PHY_ECRr(pc, &mii_phy_ecr);
    MII_PHY_ECRr_LED_TRAFFIC_ENf_SET(mii_phy_ecr, 1);
    MII_PHY_ECRr_PCS_TX_FIFO_ELASTf_SET(mii_phy_ecr, 1);
    ioerr += WRITE_MII_PHY_ECRr(pc, mii_phy_ecr);

    ioerr += READ_MISC_1000X_CTRL_2r(pc, &misc_ctrl_2);
    MISC_1000X_CTRL_2r_SERDES_JUMBO_MSBf_SET(misc_ctrl_2, 1);
    ioerr += WRITE_MISC_1000X_CTRL_2r(pc, misc_ctrl_2);

    ioerr += READ_AUX_1000X_CTRLr(pc, &aux_1000x_ctrl);
    AUX_1000X_CTRLr_SERDES_JUMBO_LSBf_SET(aux_1000x_ctrl, 1);
    ioerr += WRITE_AUX_1000X_CTRLr(pc, aux_1000x_ctrl);

    /* Reset EEE to default state (LPI disabled) */
    ioerr += READ_EEE_803Dr(pc, &eee_803d);
    EEE_803Dr_LPI_FEATURE_ENABLEf_SET(eee_803d, 0);
    EEE_803Dr_EEE_CAPABILITY_USING_QSGMII_ANf_SET(eee_803d, 0);
    ioerr += WRITE_EEE_803Dr(pc, eee_803d);

    /* Do not advertise EEE capability */   
    ioerr += READ_EEE_ADVr(pc, &eee_adv);
    EEE_ADVr_EEE_1000BASE_Tf_SET(eee_adv, 0);
    EEE_ADVr_EEE_100BASE_TXf_SET(eee_adv, 0);
    ioerr += WRITE_EEE_ADVr(pc, eee_adv);
    
    /* Reset counters and other settings : EEE_STAT_CTRLr */   
    ioerr += READ_EEE_STAT_CTRLr(pc, &eee_stat_ctrl);
    EEE_STAT_CTRLr_STATISTIC_SATURATE_MODEf_SET(eee_stat_ctrl, 0);
    EEE_STAT_CTRLr_RESERVED1f_SET(eee_stat_ctrl, 0);
    EEE_STAT_CTRLr_RESERVED0f_SET(eee_stat_ctrl, 0);
    EEE_STAT_CTRLr_STATISTIC_COUNTERS_RESETf_SET(eee_stat_ctrl, 0);
    EEE_STAT_CTRLr_STATISTIC_COUNTERS_ENABLEf_SET(eee_stat_ctrl, 0);
    ioerr += WRITE_EEE_STAT_CTRLr(pc, eee_stat_ctrl);
    
    /* Switch to instance 0 for AutogrEEEn configuration */
    if (phy_ctrl_change_inst(pc, 0, _bcm54210_inst) < 0) {
        return CDK_E_FAIL;
    }

    /* Disable AutogrEEEn and reset other settings */
    MII_BUFFER_CONTROL_0r_CLR(mii_buf_ctrl);
    MII_BUFFER_CONTROL_0r_DISABLE_LOC_LPI_HALF_DUPLEXf_SET(mii_buf_ctrl, 1);
    ioerr += WRITE_MII_BUFFER_CONTROL_0r(pc, orig_inst, mii_buf_ctrl);

    /* Restore instance */
    phy_ctrl_change_inst(pc, orig_inst, _bcm54210_inst);

    /* Disable LED current mode */
    ioerr += READ_LED_GPIO_CTRLr(pc, &led_gpio_ctrl);
    LED_GPIO_CTRLr_CURRENT_LED_DISABLEf_SET(led_gpio_ctrl, 0xf);
    ioerr += WRITE_LED_GPIO_CTRLr(pc, led_gpio_ctrl);

    /* Configure LED selectors */
    led_val = BCM54210_LED1_SEL(pc) | (BCM54210_LED2_SEL(pc) << 4);
    LED_SELECTOR_1r_SET(led_sel_1, led_val);
    ioerr += WRITE_LED_SELECTOR_1r(pc, led_sel_1);

    led_val = BCM54210_LED3_SEL(pc) | (BCM54210_LED4_SEL(pc) << 4);
    LED_SELECTOR_2r_SET(led_sel_2, led_val);
    ioerr += WRITE_LED_SELECTOR_2r(pc, led_sel_2);

    LED_CTRLr_SET(led_ctrl, BCM54210_LEDCTRL(pc));
    ioerr += WRITE_LED_CTRLr(pc, led_ctrl);

    EXP_LED_SELECTORr_SET(exp_led_selector, BCM54210_LEDSELECT(pc));
    ioerr += WRITE_EXP_LED_SELECTORr(pc, exp_led_selector);

    /* Change link speed LED mode */
    ioerr += READ_SPARE_CTRLr(pc, &spare_ctrl);
    SPARE_CTRLr_LINK_SPEED_LED_MODEf_SET(spare_ctrl, 0);
    SPARE_CTRLr_LINK_LED_MODEf_SET(spare_ctrl, 0);
    ioerr += WRITE_SPARE_CTRLr(pc, spare_ctrl);    

    /* Enable PHY temperature monitor */
    rv = PHY_CONFIG_SET(pc, PhyConfig_TempMon, 1, NULL);
    if (CDK_FAILURE(rv)) {
        return rv;
    }

    /* Call up the PHY chain */
    rv = PHY_INIT(PHY_CTRL_NEXT(pc));
    if (CDK_FAILURE(rv)) {
        return rv;
    }

    /* Set default medium */
    rv = PHY_NOTIFY(pc, PhyEvent_ChangeToCopper);
    if (CDK_FAILURE(rv)) {
        return rv;
    }

    return ioerr ? CDK_E_IO : CDK_E_NONE;
}

/*
 * Function:    
 *      bcm54210_phy_link_get
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
bcm54210_phy_link_get(phy_ctrl_t *pc, int *link, int *autoneg_done)
{
    int ioerr = 0;
    int rv = CDK_E_NONE;
    int autoneg;
    MODE_CTRLr_t mode_ctrl;
    MII_1000X_STATr_t mii_1000x_stat;
    uint32_t speed;

    PHY_CTRL_CHECK(pc);

    if (link == NULL) {
        return CDK_E_PARAM;
    }
    
    *link = FALSE;

    ioerr += READ_MODE_CTRLr(pc, &mode_ctrl);

    rv = PHY_AUTONEG_GET(pc, &autoneg);
    if (CDK_FAILURE(rv)) {
        return rv;
    }

    if (autoneg == 0) {
        /* Forced mode */
        if (PHY_CTRL_FLAGS(pc) & PHY_F_FIBER_MODE) {
            PHY_NOTIFY(pc, PhyEvent_ChangeToCopper);
        }
    } else if (MODE_CTRLr_COPPER_ENG_DETf_GET(mode_ctrl) == 1 &&
               MODE_CTRLr_FIBER_SIGNAL_DETf_GET(mode_ctrl) == 0) {
        /* Copper energy detect */
        if (PHY_CTRL_FLAGS(pc) & PHY_F_FIBER_MODE) {
            PHY_NOTIFY(pc, PhyEvent_ChangeToCopper);
        }
    } else {
        /* Fiber signal detect (or no link) */
        if ((PHY_CTRL_FLAGS(pc) & PHY_F_FIBER_MODE) == 0) {
            PHY_NOTIFY(pc, PhyEvent_ChangeToFiber);
        }
    }
        
    if (PHY_CTRL_FLAGS(pc) & PHY_F_FIBER_MODE) {
        /* Get fiber link status */
        ioerr += READ_MII_1000X_STATr(pc, &mii_1000x_stat);
        ioerr += READ_MODE_CTRLr(pc, &mode_ctrl);

        if (MODE_CTRLr_FIBER_SIGNAL_DETf_GET(mode_ctrl) &&
            MODE_CTRLr_SERDES_LINKf_GET(mode_ctrl)) {
            *link = TRUE;
        }

        if (autoneg_done) {
            /* Use register value by default */
            *autoneg_done = MII_1000X_STATr_AUTONEG_DONEf_GET(mii_1000x_stat);

            /* Force done for 100FX */
            rv = PHY_SPEED_GET(pc, &speed);
            if (CDK_FAILURE(rv)) {
                return rv;
            }
            
            if (speed == 100) {
                *autoneg_done = 1;
            }
        }
    } else {
        rv = ge_phy_link_get(pc, link, autoneg_done);
        if (CDK_FAILURE(rv)) {
            return rv;
        }
    }

    return ioerr ? CDK_E_IO : CDK_E_NONE;
}

/*
 * Function:    
 *      bcm54210_phy_duplex_set
 * Purpose:     
 *      Set the current duplex mode (forced).
 * Parameters:
 *      pc - PHY control structure
 *      duplex - non-zero indicates full duplex, zero indicates half
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm54210_phy_duplex_set(phy_ctrl_t *pc, int duplex)
{
    return bcm54282_drv.pd_duplex_set(pc, duplex);
}

/*
 * Function:    
 *      bcm54210_phy_duplex_get
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
bcm54210_phy_duplex_get(phy_ctrl_t *pc, int *duplex)
{
    return bcm54282_drv.pd_duplex_get(pc, duplex);
}

/*
 * Function:    
 *      bcm54210_phy_speed_set
 * Purpose:     
 *      Set the current operating speed (forced).
 * Parameters:
 *      pc - PHY control structure
 *      speed - new link speed
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm54210_phy_speed_set(phy_ctrl_t *pc, uint32_t speed)
{
    return bcm54282_drv.pd_speed_set(pc, speed);
}

/*
 * Function:    
 *      bcm54210_phy_speed_get
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
bcm54210_phy_speed_get(phy_ctrl_t *pc, uint32_t *speed)
{
    return bcm54282_drv.pd_speed_get(pc, speed);
}

/*
 * Function:    
 *      bcm54210_phy_autoneg_set
 * Purpose:     
 *      Enable or disabled auto-negotiation on the specified port.
 * Parameters:
 *      pc - PHY control structure
 *      autoneg - non-zero enables autoneg, zero disables autoneg
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm54210_phy_autoneg_set(phy_ctrl_t *pc, int autoneg)
{
    int ioerr = 0;
    int rv = CDK_E_NONE;
    MII_1000X_CTRLr_t mii_1000x_ctrl;

    PHY_CTRL_CHECK(pc);

    if (autoneg) {
        autoneg = 1;
    }

    /* Fiber mode */
    if (PHY_CTRL_FLAGS(pc) & PHY_F_FIBER_MODE) {
        /*
         * When enabling auto-neg, set the default speed to 1000Mbps first.
         * PHY will not enable auto-neg if the PHY is in 100FX mode.
         */
        if (autoneg) {
            rv = PHY_SPEED_SET(pc, 1000);
            if (CDK_FAILURE(rv)) {
                return rv;
            }
        } else {
            PHY_CTRL_FLAGS(pc) &= ~PHY_F_FIBER_MODE;
        }
    }

    /* Set auto-neg mode */
    ioerr += READ_MII_1000X_CTRLr(pc, &mii_1000x_ctrl);
    MII_1000X_CTRLr_AUTONEGf_SET(mii_1000x_ctrl, autoneg);
    MII_1000X_CTRLr_RESTART_ANf_SET(mii_1000x_ctrl, autoneg);
    ioerr += WRITE_MII_1000X_CTRLr(pc, mii_1000x_ctrl);

    /* Copper mode */
    rv = ge_phy_autoneg_set(pc, autoneg);
    if (CDK_FAILURE(rv)) {
        return rv;
    }

    return ioerr ? CDK_E_IO : CDK_E_NONE;
}

/*
 * Function:    
 *      bcm54210_phy_autoneg_get
 * Purpose:     
 *      Get the current auto-negotiation status (enabled/busy)
 * Parameters:
 *      pc - PHY control structure
 *      autoneg - (OUT) non-zero indicates autoneg enabled
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm54210_phy_autoneg_get(phy_ctrl_t *pc, int *autoneg)
{
    return bcm54282_drv.pd_autoneg_get(pc, autoneg);
}

/*
 * Function:    
 *      bcm54210_phy_loopback_set
 * Purpose:     
 *      Set the internal PHY loopback mode.
 * Parameters:
 *      pc - PHY control structure
 *      enable - non-zero enables PHY loopback
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm54210_phy_loopback_set(phy_ctrl_t *pc, int enable)
{
    return bcm54282_drv.pd_loopback_set(pc, enable);
}

/*
 * Function:    
 *      bcm54210_phy_loopback_get
 * Purpose:     
 *      Get the local PHY loopback mode.
 * Parameters:
 *      pc - PHY control structure
 *      enable - (OUT) non-zero indicates PHY loopback enabled
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm54210_phy_loopback_get(phy_ctrl_t *pc, int *enable)
{
    return bcm54282_drv.pd_loopback_get(pc, enable);
}

/*
 * Function:    
 *      bcm54210_phy_ability_get
 * Purpose:     
 *      Get the abilities of the PHY.
 * Parameters:
 *      pc - PHY control structure
 *      abil - (OUT) ability mask indicating supported options/speeds.
 * Returns:     
 *      CDK_E_xxx
 */
static int
bcm54210_phy_ability_get(phy_ctrl_t *pc, uint32_t *abil)
{
    PHY_CTRL_CHECK(pc);

    *abil = (PHY_ABIL_1000MB | PHY_ABIL_100MB | PHY_ABIL_10MB | 
             PHY_ABIL_LOOPBACK | PHY_ABIL_SGMII);
    
    return CDK_E_NONE;
}

/*
 * Function:
 *      bcm54210_phy_config_set
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
bcm54210_phy_config_set(phy_ctrl_t *pc, phy_config_t cfg, uint32_t val, void *cd)
{
    return bcm54282_drv.pd_config_set(pc, cfg, val, cd);
}

/*
 * Function:
 *      bcm54210_phy_config_get
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
bcm54210_phy_config_get(phy_ctrl_t *pc, phy_config_t cfg, uint32_t *val, void *cd)
{
    return bcm54282_drv.pd_config_get(pc, cfg, val, cd);
}

/*
 * Function:
 *      bcm54210_phy_status_get
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
bcm54210_phy_status_get(phy_ctrl_t *pc, phy_status_t st, uint32_t *val)
{
    return bcm54282_drv.pd_status_get(pc, st, val);
}

int bcm542xx_phy_cable_diag(phy_ctrl_t *pc, phy_port_cable_diag_t *status);

/*
 * Variable:    bcm54210_drv drv
 * Purpose:     PHY Driver for BCM54210.
 */
phy_driver_t bcm54210_drv = {
    "bcm54210",
    "BCM54210 Gigabit PHY Driver",  
    0,
    bcm54210_phy_probe,                 /* pd_probe */
    bcm54210_phy_notify,                /* pd_notify */
    bcm54210_phy_reset,                 /* pd_reset */
    bcm54210_phy_init,                  /* pd_init */
    bcm54210_phy_link_get,              /* pd_link_get */
    bcm54210_phy_duplex_set,            /* pd_duplex_set */
    bcm54210_phy_duplex_get,            /* pd_duplex_get */
    bcm54210_phy_speed_set,             /* pd_speed_set */
    bcm54210_phy_speed_get,             /* pd_speed_get */
    bcm54210_phy_autoneg_set,           /* pd_autoneg_set */
    bcm54210_phy_autoneg_get,           /* pd_autoneg_get */
    bcm54210_phy_loopback_set,          /* pd_loopback_set */
    bcm54210_phy_loopback_get,          /* pd_loopback_get */
    bcm54210_phy_ability_get,           /* pd_ability_get */
    bcm54210_phy_config_set,            /* pd_config_set */
    bcm54210_phy_config_get,            /* pd_config_get */
    bcm54210_phy_status_get,            /* pd_status_get */
    bcm542xx_phy_cable_diag             /* pd_cable_diag */
};
