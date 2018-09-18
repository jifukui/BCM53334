/*
 * $Id: mdk_phy.c,v 1.32 Broadcom SDK $
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
#include "xgsm_miim.h"

#define CDK_CONFIG_MIIM_MAX_POLLS   100000
#define MIIM_PARAM_ID_OFFSET 		16
#define MIIM_PARAM_REG_ADDR_OFFSET	24

extern phy_bus_t phy_bus_bcm5340x_miim_int;
extern phy_bus_t phy_bus_bcm95340xk_miim_ext;

static phy_bus_t *bcm5340x_phy_bus[] = {
    &phy_bus_bcm5340x_miim_int,
    &phy_bus_bcm95340xk_miim_ext,
    NULL
};

extern phy_driver_t bcm84848_drv;
extern phy_driver_t bcm54210_drv;
extern phy_driver_t bcm54280_drv;
extern phy_driver_t bcm54282_drv;
extern phy_driver_t bcmi_qsgmiie_serdes_drv;
extern phy_driver_t bcmi_tsce_xgxs_drv;

phy_driver_t *bmd_phy_drv_list[] = {
#ifdef INCLUDE_PHY_84848
    &bcm84848_drv,
#endif
#ifdef INCLUDE_PHY_54210
    &bcm54210_drv,
#endif
#ifdef INCLUDE_PHY_54280
    &bcm54280_drv,
#endif
#ifdef INCLUDE_PHY_54282
    &bcm54282_drv,
#endif
#ifdef INCLUDE_PHY_QSGMII
    &bcmi_qsgmiie_serdes_drv,
#endif
#ifdef INCLUDE_PHY_TSCE
    &bcmi_tsce_xgxs_drv,
#endif
    NULL
};

static uint32 phy_external_mode = 0;

#define PHY_EXTERNAL_MODE(lport) (phy_external_mode & (0x1 << lport))

bmd_phy_info_t bmd_phy_info[BOARD_NUM_OF_UNITS];

#define PHY_CTRL_NUM_MAX  (BCM5340X_LPORT_MAX*2)
/*
 * We do not want to rely on dynamic memory allocation,
 * so we allocate phy_ctrl blocks from a static pool.
 * The 'bus' member of the structure indicates whether
 * the block is free or in use.
 */
static phy_ctrl_t _phy_ctrl[PHY_CTRL_NUM_MAX];

int (*phy_reset_cb)(phy_ctrl_t *pc);
int (*phy_init_cb)(phy_ctrl_t *pc);

/*
 * Register PHY init callback function
 */
int
bmd_phy_init_cb_register(int (*init_cb)(phy_ctrl_t *pc))
{
    phy_init_cb = init_cb;

    return CDK_E_NONE;
}

/*
 * Register PHY reset callback function
 */
int
bmd_phy_reset_cb_register(int (*reset_cb)(phy_ctrl_t *pc))
{
    phy_reset_cb = reset_cb;

    return CDK_E_NONE;
}

int
bmd_phy_add(int unit, int lport, phy_ctrl_t *pc)
{
    pc->next = BMD_PORT_PHY_CTRL(unit, lport);
    BMD_PORT_PHY_CTRL(unit, lport) = pc;
    return CDK_E_NONE;
}

phy_ctrl_t *
bmd_phy_del(int unit, int lport)
{
    phy_ctrl_t *pc;

    if ((pc = BMD_PORT_PHY_CTRL(unit, lport)) != 0) {
        BMD_PORT_PHY_CTRL(unit, lport) = pc->next;
    }
    return pc;
}

static phy_ctrl_t *
phy_ctrl_alloc(void)
{
    int idx;
    phy_ctrl_t *pc;

    for (idx = 0, pc = &_phy_ctrl[0]; idx < PHY_CTRL_NUM_MAX; idx++, pc++) {
        if (pc->bus == 0) {
            return pc;
        }
    }
    return NULL;
}

static void
phy_ctrl_free(phy_ctrl_t *pc)
{
    pc->bus = 0;
}

/*
 * Probe all PHY buses associated with BMD device
 */
int
bmd_phy_probe_default(int unit, int lport, phy_driver_t **phy_drv)
{
    phy_bus_t **bus;
    phy_driver_t **drv;
    phy_ctrl_t pc_probe;
    phy_ctrl_t *pc;
    int rv;

    /* Remove any existing PHYs on this lport */
    while ((pc = bmd_phy_del(unit, lport)) != 0) {
        phy_ctrl_free(pc);
    }

    /* Bail if not PHY driver list is provided */
    if (phy_drv == NULL) {
        return CDK_E_NONE;
    }

    /* Check that we have PHY bus list */
    bus = BMD_PORT_PHY_BUS(unit, lport);
    if (bus == NULL) {
        return CDK_E_CONFIG;
    }

    /* Loop over PHY buses for this lport */
    while (*bus != NULL) {
        drv = phy_drv;
        /* Probe all PHY drivers on this bus */
        while (*drv != NULL) {
            /* Initialize PHY control used for probing */
            pc = &pc_probe;
            sal_memset(pc, 0, sizeof(*pc));
            pc->unit = unit;
            pc->port = SOC_PORT_L2P_MAPPING(lport);
            pc->bus = *bus;
            pc->drv = *drv;
            if (CDK_SUCCESS(PHY_PROBE(pc))) {
                /* Found known PHY on bus */
                pc = phy_ctrl_alloc();
                if (pc == NULL) {
                    return CDK_E_MEMORY;
                }
                sal_memcpy(pc, &pc_probe, sizeof(*pc));
                /* Install PHY */
                rv = bmd_phy_add(unit, lport, pc);
                if (CDK_FAILURE(rv)) {
                    return rv;
                }
                /* Move to next bus */
                break;
            }
            drv++;
        }
        bus++;
    }

    pc = BMD_PORT_PHY_CTRL(unit, lport);
    if (pc && pc->next) {
        /* If both external PHY and serdes are attached. */
        phy_external_mode |= (0x1 << lport);
    }

    return CDK_E_NONE;
}

int
soc_phyctrl_notify(phy_ctrl_t *pc, phy_event_t event, uint32 value)
{
    int rv = CDK_E_NONE;
    int lport;
#if 0
    
    if (auto_cfg[unit][port]) {
        return CDK_E_NONE;
    }
#endif

    PHY_CTRL_CHECK(pc);

    lport = SOC_PORT_P2L_MAPPING(pc->port);
    if (!PHY_EXTERNAL_MODE(lport) || !(pc->next)) {
        return CDK_E_NONE;
    }

    switch (event) {
    case PhyEvent_Speed:
        rv = PHY_SPEED_SET(pc->next, value);
        break;
    case PhyEvent_Duplex:
        rv = PHY_DUPLEX_SET(pc->next, value);
        break;
    default:
        return CDK_E_NONE;
    }

    return rv;
}

sys_error_t
phy_reg_read(uint8 lport, uint16 reg_addr, uint16 *p_value)
{
    int rv = CDK_E_NONE;
    uint32 value;
    phy_ctrl_t *pc;

    pc = BMD_PORT_PHY_CTRL(0, lport);

    rv = PHY_BUS_READ(pc, (uint32)reg_addr, &value);

    *p_value = (uint16)value;

    if (!rv) {
        return SYS_OK;
    }
    else {
        return SYS_ERR;
    }
}

/*
 *  Function : phy_reg_write
 *
 *  Purpose :
 *      Write a value into a MII register.
 *
 *  Parameters :
 *      dev (input) : PHY id
 *      reg_addr (input) : PHY regsiter
 *      value (input) : PHY data to write into register
 *
 *  Return :
 *      SYS_OK : success
 *      SYS_ERR : failed to access the MII register
 *
 */
sys_error_t
phy_reg_write(uint8 lport, uint16 reg_addr, uint16 value)
{
    int rv = CDK_E_NONE;

    phy_ctrl_t *pc;

    pc = BMD_PORT_PHY_CTRL(0, lport);

    rv = PHY_BUS_WRITE(pc, (uint32)reg_addr, (uint32)value);

    if (!rv) {
        return SYS_OK;
    }
    else {
        return SYS_ERR;
    }
}

int
cdk_xgsm_miim_read(int unit, uint32_t phy_addr, uint32_t reg, uint32_t *val)
{
    int rv = CDK_E_NONE;
    uint32 polls, data, phy_param;

    MIIM_LOCK(unit);
    /*
     * Use clause 45 access if DEVAD specified.
     * Note that DEVAD 32 (0x20) can be used to access special DEVAD 0.
     */
    if (reg & 0x003f0000) {
        phy_addr |= CDK_XGSM_MIIM_CLAUSE45;
        reg &= 0x001fffff;
    }

    phy_param = (phy_addr << MIIM_PARAM_ID_OFFSET);
    WRITECSR(CMIC_CMC1_MIIM_PARAM, phy_param);

    WRITECSR(CMIC_CMC1_MIIM_ADDRESS, reg);

    /* Tell CMIC to start */
    WRITECSR(CMIC_CMC1_MIIM_CTRL, CMIC_MIIM_RD_START);

    /* Poll for completion */
    for (polls = 0; polls < CDK_CONFIG_MIIM_MAX_POLLS; polls++) {
        data = READCSR(CMIC_CMC1_MIIM_STAT);
        if (data & CMIC_MIIM_OPN_DONE) {
            break;
	    }
    }

    /* Check for timeout and error conditions */
    if (polls == CDK_CONFIG_MIIM_MAX_POLLS) {
        rv = -1;
        CDK_DEBUG_MIIM
            (("cdk_xgsm_miim_read[%d]: Timeout at phy_addr=0x%08x"
              "reg_addr=%08x\n",
              unit, phy_addr, reg));
    }

    WRITECSR(CMIC_CMC1_MIIM_CTRL, 0x0);

    if (rv >= 0) {
        *val = READCSR(CMIC_CMC1_MIIM_READ_DATA);
        CDK_DEBUG_MIIM
            (("cdk_xgsm_miim_read[%d]: phy_addr=0x%08x"
              "reg_addr=%08x data: 0x%08x\n",
              unit, phy_addr, reg, *val));
    }
    MIIM_UNLOCK(unit);
    return rv;
}

int
cdk_xgsm_miim_write(int unit, uint32_t phy_addr, uint32_t reg, uint32_t val)
{
    int rv = CDK_E_NONE;
    uint32 polls, data, phy_param;

    MIIM_LOCK(unit);
    /*
     * Use clause 45 access if DEVAD specified.
     * Note that DEVAD 32 (0x20) can be used to access special DEVAD 0.
     */
    if (reg & 0x003f0000) {
        phy_addr |= CDK_XGSM_MIIM_CLAUSE45;
        reg &= 0x001fffff;
    }

    phy_param = (phy_addr << MIIM_PARAM_ID_OFFSET) | val;
    WRITECSR(CMIC_CMC1_MIIM_PARAM, phy_param);

    WRITECSR(CMIC_CMC1_MIIM_ADDRESS, reg);

    /* Tell CMIC to start */
    WRITECSR(CMIC_CMC1_MIIM_CTRL, CMIC_MIIM_WR_START);

    /* Poll for completion */
    for (polls = 0; polls < CDK_CONFIG_MIIM_MAX_POLLS; polls++) {
        data = READCSR(CMIC_CMC1_MIIM_STAT);
        if (data & CMIC_MIIM_OPN_DONE) {
            break;
	    }
    }

    /* Check for timeout and error conditions */
    if (polls == CDK_CONFIG_MIIM_MAX_POLLS) {
        rv = -1;
        CDK_DEBUG_MIIM
            (("cdk_xgsm_miim_read[%d]: Timeout at phy_addr=0x%08x"
              "reg_addr=%08x\n",
              unit, phy_addr, reg));
    }

    WRITECSR(CMIC_CMC1_MIIM_CTRL, 0x0);
    MIIM_UNLOCK(unit);
    return rv;
}

/***********************************************************************
 *
 * HELPER FUNCTIONS FOR SBUS MDIO ACCESS
 *
 ***********************************************************************/
#define INDACC_POLLING_TIMEOUT          1000

#define QSGMII_REG_ADDR_REG(_phy_reg) \
                    ((((_phy_reg) & 0x8000) >> 11) | ((_phy_reg) & 0xf))
#define QSGMII_REG_ADDR_LANE_SET(_phy_reg_new, _phy_reg) \
                    ((_phy_reg_new) |= ((_phy_reg) & 0x7ff0000))
#define QSGMII_REG_ADDR_LANE(_phy_reg) \
                    (((_phy_reg) >> 16) & 0x7ff)

static int
_indirect_access_read(int unit, uint32_t phy_addr,
                      uint32_t phy_reg, uint32_t *phy_data)
{
    int rv = CDK_E_NONE;
    int ioerr = 0;
    int idx;
    uint32_t lane;
    uint32_t indacc_rdata;
    uint32_t indacc_ctlsts;

    lane = QSGMII_REG_ADDR_LANE(phy_reg);
    if (lane > 15) {
        return CDK_E_PARAM;
    }

    /*
     * CHIP_INDACC_CTLSTS Register
     * Desc - Indirect Access Control and status register for PM4x10Q Top; access to QSGMII_2x_PCS MDIO registers
     * Field -
     * WR_REQ : bit 31 Write Request; 0 = ready; Write to 1 to start write transaction
     * RD_REQ : bit 30 Read Request; 0 = ready; Write to 1 to start read transaction
     * WR_RDY : bit 29 Write Ready status; 1 = ready for a WR_REQ
     * RD_RDY : bit 28 Read Ready status; 1 = ready for a RD_REQ
     * TARGET_SELECT_REQ : bit 10 Select mdio register set of QSGMII0 or QSGMII1 for the mdio transaction
     * ADDRESS : bit 9:0 Indirect Access Adress Field
     */
    indacc_ctlsts = ((lane / 8) << 10);
    indacc_ctlsts |= (1 << 30);

    /* Assigning the reg_addr in Clause22 format */
    indacc_ctlsts |= (phy_reg & 0x1f);
    ioerr += bcm5340x_reg_set(unit, R_CHIP_INDACC_CTLSTS, indacc_ctlsts);

    for (idx = 0; idx < INDACC_POLLING_TIMEOUT; idx++) {
        ioerr += bcm5340x_reg_get(unit, R_CHIP_INDACC_CTLSTS, &indacc_ctlsts);
        if (indacc_ctlsts & (1L << 28)) {
            break;
        }
    }
    if (idx >= INDACC_POLLING_TIMEOUT) {
        sal_printf("bcm53400_a0_bmd_attach[%d]: "
                  "In-direct access polling timeout\n", unit);
        return ioerr ? CDK_E_IO : CDK_E_TIMEOUT;
    }

    /*
     * CHIP_INDACC_RDATA Register
     * Desc - Indirect Access READ DATA register for PM4x10Q Top
     * Field -
     * RDDATA : bit 15:0 Indirect Access READ Data register (Read Only)
     */

    ioerr += bcm5340x_reg_get(unit, R_CHIP_INDACC_RDATA, &indacc_rdata);
    *phy_data = indacc_rdata;

    indacc_ctlsts = 0;
    ioerr += bcm5340x_reg_set(unit, R_CHIP_INDACC_CTLSTS, indacc_ctlsts);

    return ioerr ? CDK_E_IO : rv;
}

static int
_indirect_access_write(int unit, uint32_t phy_addr,
                       uint32_t phy_reg, uint32_t phy_data)
{
    int rv = CDK_E_NONE;
    int ioerr = 0;
    int idx;
    uint32_t lane;
    uint32_t indacc_wdata;
    uint32_t indacc_ctlsts;

    indacc_wdata = phy_data;

    /*
     * CHIP_INDACC_WDATA Register
     * Desc - Indirect Access WRITE DATA register for PM4x10Q Top
     * Field -
     * WRDATA : bit 15:0 Indirect Access WRITE Data register
     */
    ioerr += bcm5340x_reg_set(unit, R_CHIP_INDACC_WDATA, indacc_wdata);

    lane = QSGMII_REG_ADDR_LANE(phy_reg);
    if (lane > 15) {
        return CDK_E_PARAM;
    }

    /*
     * CHIP_INDACC_CTLSTS Register
     * Desc - Indirect Access Control and status register for PM4x10Q Top; access to QSGMII_2x_PCS MDIO registers
     * Field -
     * WR_REQ : bit 31 Write Request; 0 = ready; Write to 1 to start write transaction
     * RD_REQ : bit 30 Read Request; 0 = ready; Write to 1 to start read transaction
     * WR_RDY : bit 29 Write Ready status; 1 = ready for a WR_REQ
     * RD_RDY : bit 28 Read Ready status; 1 = ready for a RD_REQ
     * TARGET_SELECT_REQ : bit 10 Select mdio register set of QSGMII0 or QSGMII1 for the mdio transaction
     * ADDRESS : bit 9:0 Indirect Access Adress Field
     */
    indacc_ctlsts = ((lane / 8) << 10);
    indacc_ctlsts |= (1 << 31);
    /* Assigning the reg_addr in Clause22 format */
    indacc_ctlsts |= (phy_reg & 0x1f);
    ioerr += bcm5340x_reg_set(unit, R_CHIP_INDACC_CTLSTS, indacc_ctlsts);

    for (idx = 0; idx < INDACC_POLLING_TIMEOUT; idx++) {
        ioerr += bcm5340x_reg_get(unit, R_CHIP_INDACC_CTLSTS, &indacc_ctlsts);
        if (indacc_ctlsts & (1L << 29)) {
            break;
        }
    }
    if (idx >= INDACC_POLLING_TIMEOUT) {
        sal_printf("bcm53400_a0_bmd_attach[%d]: "
                  "In-direct access polling timeout\n", unit);
        return ioerr ? CDK_E_IO : CDK_E_TIMEOUT;
    }

    indacc_ctlsts = 0;
    ioerr += bcm5340x_reg_set(unit, R_CHIP_INDACC_CTLSTS, indacc_ctlsts);

    return ioerr ? CDK_E_IO : rv;
}

static int _mdio_addr_to_port(uint32_t phy_addr){
    if (CDK_XGSM_MIIM_IBUS_NUM(phy_addr) == 1) {
        return (phy_addr & 0x1f) + 17;
    } else {
        return (phy_addr & 0x1f) + 1;
    }
}

/***********************************************************************
 *
 * HELPER FUNCTIONS FOR SBUS MDIO ACCESS
 *
 ***********************************************************************/
#define PHY_REG_ADDR_DEVAD(reg)     (((reg) >> 27) & 0x1f)
#define IS_QSGMII_REGISTER(reg)     (((reg) & 0xf800f000) == 0x8000)
#if 0 
#define QSGMII_REG_ADDR_REG(reg)    ((((reg) & 0x8000) >> 11) | ((reg) & 0xf))
#define QSGMII_REG_ADDR_LANE(reg)   (((reg) >> 16) & 0x7ff)
#define QSGMII_REG_ADDR_LANE_SET(nreg, reg)     ((nreg) |= ((reg) & 0x7ff0000))
#endif

static int
_qsgmii_access_check(int unit, int lport, uint32_t phy_reg)
{
    int qflag;

    /* Check whether the access is to QSGMII or not */
    qflag = 0;
    if (PHY_REG_ADDR_DEVAD(phy_reg) == 0){  /* DEVAD==0 for PCS */
        if (IS_QSGMII_REGISTER(phy_reg)) {
            qflag = 1;
        } else if ((QSGMII_REG_ADDR_REG(phy_reg) < 0x10) &&
                    IS_GX_PORT(lport)) {
            qflag = 1;
        }
    }
    return qflag;
}

int
cdk_xgsm_sbus_read(int unit, uint32_t phy_addr,
                         uint32_t phy_reg, uint32_t *phy_data)
{
    int rv = CDK_E_NONE;
    int pport, lport;
    /* XLPORT_WC_UCMEM_DATAm_t ucmem_data; */
    uint32_t xlport_wc_ucmem_data[4];
    uint32_t reg_addr, reg_data;
    uint8_t block_id;

    pport = _mdio_addr_to_port(phy_addr);

    lport = SOC_PORT_P2L_MAPPING(pport);

    MIIM_SCHAN_LOCK(unit);

    if (_qsgmii_access_check(unit, lport, phy_reg)) {
        /* TSCQ sbus access */
        *phy_data = 0;

        /* AER process : AER block selection */
        reg_addr = 0x1f;
        QSGMII_REG_ADDR_LANE_SET(reg_addr, phy_reg);
        reg_data = 0xffd0;
        rv = _indirect_access_write(unit, phy_addr, reg_addr, reg_data);

        if (CDK_SUCCESS(rv)) {
            /* AER process : lane control */
            reg_addr = 0x1e;
            QSGMII_REG_ADDR_LANE_SET(reg_addr, phy_reg);
            reg_data = QSGMII_REG_ADDR_LANE(phy_reg) & 0x7;
            rv = _indirect_access_write(unit, phy_addr, reg_addr, reg_data);
        }

        /* Target register block selection */
        if (CDK_SUCCESS(rv)) {
            reg_addr = 0x1f;
            QSGMII_REG_ADDR_LANE_SET(reg_addr, phy_reg);
            reg_data = phy_reg & 0xfff0;
            rv = _indirect_access_write(unit, phy_addr, reg_addr, reg_data);
        }

        /* Read data */
        if (CDK_SUCCESS(rv)) {
            reg_addr = QSGMII_REG_ADDR_REG(phy_reg);
            QSGMII_REG_ADDR_LANE_SET(reg_addr, phy_reg);
            rv = _indirect_access_read(unit, phy_addr, reg_addr, &reg_data);
        }

        if (CDK_SUCCESS(rv)) {
            *phy_data = reg_data;
        }
    } else {
        /* TSCE sbus access */
        xlport_wc_ucmem_data[0] = (phy_reg | ((phy_addr & 0x1f) << 19)) & 0xffffffff;
        xlport_wc_ucmem_data[1] = 0x0;
        xlport_wc_ucmem_data[2] = 0x0;
        xlport_wc_ucmem_data[3] = 0x0;

        if (IS_GX_PORT(lport)) {
            block_id = XLPORT5_BLOCK_ID;
        } else {
            block_id = SOC_PORT_BLOCK(lport);
        }

        /* rv = WRITE_XLPORT_WC_UCMEM_DATAm(unit, 0, ucmem_data, lport); */
        rv = bcm5340x_mem_set(unit, block_id, M_XLPORT_WC_UCMEM_DATA(0), xlport_wc_ucmem_data, 4);

        if (CDK_SUCCESS(rv)) {
            /* rv = READ_XLPORT_WC_UCMEM_DATAm(unit, 0, &ucmem_data, lport); */
            rv = bcm5340x_mem_get(unit, block_id, M_XLPORT_WC_UCMEM_DATA(0), xlport_wc_ucmem_data, 4);
        }
        /* *phy_data = XLPORT_WC_UCMEM_DATAm_GET(ucmem_data, 1); */
        *phy_data = xlport_wc_ucmem_data[1];
    }
    MIIM_SCHAN_UNLOCK(unit);
    return rv;
}

int
cdk_xgsm_sbus_write(int unit, uint32_t phy_addr,
                          uint32_t phy_reg, uint32_t phy_data)
{
    int rv = CDK_E_NONE;
    int pport, lport;
    /* XLPORT_WC_UCMEM_DATAm_t ucmem_data; */
    uint32_t xlport_wc_ucmem_data[4];
    uint32_t reg_addr, reg_data;
    uint8_t block_id;

    pport = _mdio_addr_to_port(phy_addr);

    lport = SOC_PORT_P2L_MAPPING(pport);

    MIIM_SCHAN_LOCK(unit);

    if (_qsgmii_access_check(unit, lport, phy_reg)) {
        /* TSCQ sbus access */

        /* AER process : AER block selection */
        reg_addr = 0x1f;
        QSGMII_REG_ADDR_LANE_SET(reg_addr, phy_reg);
        reg_data = 0xffd0;
        rv = _indirect_access_write(unit, phy_addr, reg_addr, reg_data);

        if (CDK_SUCCESS(rv)) {
            /* AER process : lane control */
            reg_addr = 0x1e;
            QSGMII_REG_ADDR_LANE_SET(reg_addr, phy_reg);
            reg_data = QSGMII_REG_ADDR_LANE(phy_reg) & 0x7;
            rv = _indirect_access_write(unit, phy_addr, reg_addr, reg_data);
        }

        /* Target register block selection */
        if (CDK_SUCCESS(rv)) {
            reg_addr = 0x1f;
            QSGMII_REG_ADDR_LANE_SET(reg_addr, phy_reg);
            reg_data = phy_reg & 0xfff0;
            rv = _indirect_access_write(unit, phy_addr, reg_addr, reg_data);
        }

        /* Write data */
        if (CDK_SUCCESS(rv)) {
            reg_addr = QSGMII_REG_ADDR_REG(phy_reg);
            QSGMII_REG_ADDR_LANE_SET(reg_addr, phy_reg);
            rv = _indirect_access_write(unit, phy_addr, reg_addr, phy_data);
        }
    } else {
        /* TSCE sbus access */
        if ((phy_data & 0xffff0000) == 0) {
            phy_data |= 0xffff0000;
        }

        xlport_wc_ucmem_data[0] = (phy_reg | ((phy_addr & 0x1f) << 19)) & 0xffffffff;;
        xlport_wc_ucmem_data[1] = ((phy_data & 0xffff) << 16) |
                  ((~phy_data & 0xffff0000) >> 16);
        xlport_wc_ucmem_data[2] = 1; /* for TSC register write */
        xlport_wc_ucmem_data[3] = 0x0;

        if (IS_GX_PORT(lport)) {
            block_id = XLPORT5_BLOCK_ID;
        } else {
            block_id = SOC_PORT_BLOCK(lport);
        }

        /* rv = WRITE_XLPORT_WC_UCMEM_DATAm(unit, 0, ucmem_data, lport); */
        rv = bcm5340x_mem_set(unit, block_id, M_XLPORT_WC_UCMEM_DATA(0), xlport_wc_ucmem_data, 4);
    }
    MIIM_SCHAN_UNLOCK(unit);
    return rv;
}


int
bmd_phy_init(int unit, int lport)
{
    int rv = CDK_E_NONE;
    
    if (BMD_PORT_PHY_CTRL(unit, lport)) {
        rv = PHY_RESET(BMD_PORT_PHY_CTRL(unit, lport));
        if (CDK_SUCCESS(rv) && phy_reset_cb) {
            rv = phy_reset_cb(BMD_PORT_PHY_CTRL(unit, lport));
        }
        if (CDK_SUCCESS(rv)) {
            rv = PHY_INIT(BMD_PORT_PHY_CTRL(unit, lport));
        }
        if (CDK_SUCCESS(rv) && phy_init_cb) {
            rv = phy_init_cb(BMD_PORT_PHY_CTRL(unit, lport));
        }
    }

    return rv;
}

int
bmd_phy_attach(int unit, int lport)
{
    int rv;

    CDK_VERB(("bmd_phy_attach: lport=%d\n", lport));

    BMD_PORT_PHY_BUS(unit, lport) = bcm5340x_phy_bus;

    rv = bmd_phy_probe_default(unit, lport, bmd_phy_drv_list);

    if (CDK_SUCCESS(rv)) {
        rv = bmd_phy_init(unit, lport);
    }

    return rv;
}

int
bmd_phy_probe(int unit, int lport)
{
    BMD_PORT_PHY_BUS(unit, lport) = bcm5340x_phy_bus;
    return bmd_phy_probe_default(unit, lport, bmd_phy_drv_list);
    return CDK_E_NONE;
}

int
bmd_phy_mode_set(int unit, int lport, char *name, int mode, int enable)
{
    int rv = CDK_E_NONE;

    phy_ctrl_t *pc = BMD_PORT_PHY_CTRL(unit, lport);

    while (pc != NULL) {
        if (name && pc->drv && pc->drv->drv_name &&
            (sal_strcmp(pc->drv->drv_name, name) != 0)) {
            pc = pc->next;
            continue;
        }
        switch (mode) {
        case BMD_PHY_MODE_WAN:
            rv = PHY_CONFIG_SET(pc, PhyConfig_Mode,
                                enable ? PHY_MODE_WAN : PHY_MODE_LAN, NULL);
            if (!enable && rv == CDK_E_UNAVAIL) {
                rv = CDK_E_NONE;
            }
            break;
        case BMD_PHY_MODE_2LANE:
            if (enable) {
                PHY_CTRL_FLAGS(pc) |= PHY_F_2LANE_MODE;
            } else {
                PHY_CTRL_FLAGS(pc) &= ~PHY_F_2LANE_MODE;
            }
            break;
        case BMD_PHY_MODE_SERDES:
            if (enable) {
                PHY_CTRL_FLAGS(pc) |= PHY_F_SERDES_MODE;
            } else {
                PHY_CTRL_FLAGS(pc) &= ~PHY_F_SERDES_MODE;
            }
            break;
        case BMD_PHY_MODE_FIBER:
            if (enable) {
                PHY_CTRL_FLAGS(pc) |= PHY_F_FIBER_MODE;
            } else {
                PHY_CTRL_FLAGS(pc) &= ~PHY_F_FIBER_MODE;
            }
            break;
        default:
            rv = CDK_E_PARAM;
            break;
        }
        break;
    }
    return rv;
}

int
bmd_phy_fw_base_set(int unit, int lport, char *name, uint32_t fw_base)
{
    int rv = CDK_E_NONE;

    phy_ctrl_t *pc = BMD_PORT_PHY_CTRL(unit, lport);

    while (pc != NULL) {
        if (name && pc->drv && pc->drv->drv_name &&
            (sal_strcmp(pc->drv->drv_name, name) != 0)) {
            pc = pc->next;
            continue;
        }
        rv = PHY_CONFIG_SET(pc, PhyConfig_RamBase, fw_base, NULL);
        if (rv == CDK_E_UNAVAIL) {
            rv = CDK_E_NONE;
        }
        break;
    }
    return rv;
}
int
bmd_phy_fw_helper_set(int unit, int lport,
                      int (*fw_helper)(void *, uint32_t, uint32_t, void *))
{
    phy_ctrl_t *pc = BMD_PORT_PHY_CTRL(unit, lport);

    while (pc != NULL) {
        PHY_CTRL_FW_HELPER(pc) = fw_helper;
        pc = pc->next;
    }
    return CDK_E_NONE;
}

int
bmd_phy_line_interface_set(int unit, int lport, int intf)
{
    int pref_intf;
    phy_ctrl_t *pc = BMD_PORT_PHY_CTRL(unit, lport);    
    
    if (BMD_PORT_PHY_CTRL(unit, lport)) {
        switch (intf) {
        case BMD_PHY_IF_XFI:
            pref_intf = PHY_IF_XFI;
            break;
        case BMD_PHY_IF_SFI:
            pref_intf = PHY_IF_SFI;
            if (sal_strcmp(pc->drv->drv_name, "bcmi_tsce_xgxs") != 0) {
                PHY_CTRL_FLAGS(pc) |= PHY_F_FIBER_MODE;
            }
            break;
        case BMD_PHY_IF_KR:
            pref_intf = PHY_IF_KR;
            break;
        case BMD_PHY_IF_CR:
            pref_intf = PHY_IF_CR;
            break;
        case BMD_PHY_IF_HIGIG:
            pref_intf = PHY_IF_HIGIG;
            break;
        default:
            pref_intf = 0;
            break;
        }
        PHY_CTRL_LINE_INTF(BMD_PORT_PHY_CTRL(unit, lport)) = pref_intf;
    }
    return CDK_E_NONE;
}

int 
bmd_phy_external_mode_get(int unit, int lport)
{
    return PHY_EXTERNAL_MODE(lport);
}

int 
bmd_phy_ability_set(int unit, int lport, char *name, int ability)
{
    int rv = CDK_E_NONE;

    phy_ctrl_t *pc = BMD_PORT_PHY_CTRL(unit, lport);
    int phy_abil = 0;

    while (pc != NULL) {
        if (name && pc->drv && pc->drv->drv_name &&
            ((sal_strcmp(pc->drv->drv_name, "bcmi_tsce_xgxs") != 0) &&
              (sal_strcmp(pc->drv->drv_name, "bcmi_qsgmiie_serdes_drv") != 0))) {
            pc = pc->next;
            continue;
        }

        if (ability & BMD_PHY_ABIL_10MB_HD)
            phy_abil |= PHY_ABIL_10MB_HD;
        if (ability & BMD_PHY_ABIL_10MB_FD)
            phy_abil |= PHY_ABIL_10MB_FD;
        if (ability & BMD_PHY_ABIL_100MB_HD)
            phy_abil |= PHY_ABIL_100MB_HD;
        if (ability & BMD_PHY_ABIL_100MB_FD)
            phy_abil |= PHY_ABIL_100MB_FD;
        if (ability & BMD_PHY_ABIL_1000MB_HD)
            phy_abil |= PHY_ABIL_1000MB_HD;
        if (ability & BMD_PHY_ABIL_1000MB_FD)
            phy_abil |= PHY_ABIL_1000MB_FD;
        if (ability & BMD_PHY_ABIL_2500MB)
            phy_abil |= PHY_ABIL_2500MB;
        if (ability & BMD_PHY_ABIL_3000MB)
            phy_abil |= PHY_ABIL_3000MB;
        if (ability & BMD_PHY_ABIL_10GB)
            phy_abil |= PHY_ABIL_10GB;
        if (ability & BMD_PHY_ABIL_13GB)
            phy_abil |= PHY_ABIL_13GB;
        if (ability & BMD_PHY_ABIL_16GB)
            phy_abil |= PHY_ABIL_16GB;
        if (ability & BMD_PHY_ABIL_21GB)
            phy_abil |= PHY_ABIL_21GB;
        if (ability & BMD_PHY_ABIL_25GB)
            phy_abil |= PHY_ABIL_25GB;
        if (ability & BMD_PHY_ABIL_30GB)
            phy_abil |= PHY_ABIL_30GB;
        if (ability & BMD_PHY_ABIL_40GB)
            phy_abil |= PHY_ABIL_40GB;

        phy_abil |= (PHY_ABIL_PAUSE_TX | PHY_ABIL_PAUSE_RX);

        rv = PHY_CONFIG_SET(pc, PhyConfig_AdvLocal, phy_abil, NULL);
        if (rv == CDK_E_UNAVAIL) {
            rv = CDK_E_NONE;
        }
        break;
    }
    return rv;
}

int
bmd_phy_eee_set(int unit, int lport, int mode)
{
    if (BMD_PORT_PHY_CTRL(unit, lport)) {
        uint32_t eee_mode = PHY_EEE_NONE;
        int rv;

        if (!PHY_EXTERNAL_MODE(lport)) {
            /* serdes, set to no EEE */
            eee_mode = PHY_EEE_NONE;
        } else if (mode == BMD_PHY_M_EEE_802_3) {
            eee_mode = PHY_EEE_802_3;
        } else if (mode == BMD_PHY_M_EEE_AUTO) {
            eee_mode = PHY_EEE_AUTO;
        }
        rv = PHY_CONFIG_SET(BMD_PORT_PHY_CTRL(unit, lport),
                            PhyConfig_EEE, eee_mode, NULL);
        if (mode == PHY_EEE_NONE && rv == CDK_E_UNAVAIL) {
            rv = CDK_E_NONE;
        }
        return rv;
    }
    return CDK_E_NONE;
}

int
bmd_phy_eee_get(int unit, int lport, int *mode)
{
    *mode = PHY_EEE_NONE;

    if (BMD_PORT_PHY_CTRL(unit, lport)) {
        uint32_t eee_mode;
        int rv = PHY_CONFIG_GET(BMD_PORT_PHY_CTRL(unit, lport),
                                PhyConfig_EEE, &eee_mode, NULL);
        if (CDK_FAILURE(rv) && rv != CDK_E_UNAVAIL) {
            return rv;
        }
        if (eee_mode == PHY_EEE_802_3) {
            *mode = BMD_PHY_M_EEE_802_3;
        } else if (eee_mode == PHY_EEE_AUTO) {
            *mode = BMD_PHY_M_EEE_AUTO;
        }
    }
    return CDK_E_NONE;
}

typedef struct _bcast_sig_s {
    const char *drv_name;
    const char *bus_name;
    uint32_t addr;
} _bcast_sig_t;

#define MAX_BCAST_SIG   8
#define MAX_INIT_STAGE  8

int
bmd_phy_staged_init(int unit)
{
    int rv = CDK_E_NONE;
    int lport, idx, found;
    phy_ctrl_t *pc;
    uint32_t addr;
    _bcast_sig_t bcast_sig[MAX_BCAST_SIG];
    int num_sig, stage, done;

    num_sig = 0;
    SOC_LPORT_ITER(lport) {
        pc = BMD_PORT_PHY_CTRL(unit, lport);
        for (; pc != NULL; pc = pc->next) {
            /* Let driver know that staged init is being used */
            PHY_CTRL_FLAGS(pc) |= PHY_F_STAGED_INIT;
            /* Mark as broadcast slave by default */
            PHY_CTRL_FLAGS(pc) &= ~PHY_F_BCAST_MSTR;
            /* Get broadcast signature */
            rv = PHY_CONFIG_GET(pc, PhyConfig_BcastAddr, &addr, NULL);
            if (CDK_FAILURE(rv)) {
                continue;
            }
            if (pc->drv == NULL ||  pc->drv->drv_name == NULL) {
                continue;
            }
            if (pc->bus == NULL ||  pc->bus->drv_name == NULL) {
                continue;
            }
            /* Check if broadcast signature exists */
            found = 0;
            for (idx = 0; idx < num_sig; idx++) {
                if (bcast_sig[idx].drv_name == pc->drv->drv_name &&
                    bcast_sig[idx].bus_name == pc->bus->drv_name &&
                    bcast_sig[idx].addr == addr) {
                    found = 1;
                    break;
                }
            }
            if (found) {
                continue;
            }
            if (idx >= MAX_BCAST_SIG) {
                return CDK_E_FAIL;
            }
            /* Add new broadcast signature */
            bcast_sig[idx].drv_name = pc->drv->drv_name;
            bcast_sig[idx].bus_name = pc->bus->drv_name;
            bcast_sig[idx].addr = addr;
            CDK_VERB(("PHY init: new bcast sig: %s %s 0x%04x\n",
                      bcast_sig[idx].drv_name,
                      bcast_sig[idx].bus_name,
                      bcast_sig[idx].addr));
            num_sig++;
            /* Mark as master for this broadcast domain */
            PHY_CTRL_FLAGS(pc) |= PHY_F_BCAST_MSTR;
        }
    }

    /* Reset all PHYs */
    SOC_LPORT_ITER(lport) {
        pc = BMD_PORT_PHY_CTRL(unit, lport);
        if (pc == NULL) {
            continue;
        }
        rv = PHY_RESET(pc);
        if (CDK_FAILURE(rv)) {
            return rv;
        }
    }

    /* Perform reset callbacks */
    if (phy_reset_cb) {
        SOC_LPORT_ITER(lport) {
            pc = BMD_PORT_PHY_CTRL(unit, lport);
            if (pc == NULL) {
                continue;
            }
            rv = phy_reset_cb(pc);
            if (CDK_FAILURE(rv)) {
                return rv;
            }
        }
    }

    /* Repeat staged initialization until no more work */
    stage = 0;
    do {
        CDK_VERB(("PHY init: stage %d\n", stage));
        done = 1;
        SOC_LPORT_ITER(lport) {
            CDK_VVERB(("PHY init: stage %d, lport %d\n", stage, lport));
            pc = BMD_PORT_PHY_CTRL(unit, lport);
            for (; pc != NULL; pc = pc->next) {
                rv = PHY_CONFIG_SET(pc, PhyConfig_InitStage, stage, NULL);
                if (rv == CDK_E_UNAVAIL) {
                    /* Perform standard init if stage 0 is unsupported */
                    if (stage == 0) {
                        rv = PHY_INIT(pc);
                        if (CDK_FAILURE(rv)) {
                            return rv;
                        }
                        while (pc->next != NULL) {
                            pc = pc->next;
                        }
                    }
                    rv = CDK_E_NONE;
                    continue;
                }
                if (CDK_FAILURE(rv)) {
                    return rv;
                }
                done = 0;
            }
        }
        /* Add safety guard against loops */
        if (++stage > MAX_INIT_STAGE) {
            return CDK_E_INTERNAL;
        }
    } while (!done);

    /* Perform init callbacks */
    if (phy_init_cb) {
        SOC_LPORT_ITER(lport) {
            pc = BMD_PORT_PHY_CTRL(unit, lport);
            if (pc == NULL) {
                continue;
            }
            rv = phy_init_cb(pc);
            if (CDK_FAILURE(rv)) {
                return rv;
            }
        }
    }
    return rv;
}

extern int
phy_brcm_xe_read(phy_ctrl_t *pc, uint32_t addr, uint32_t *data);
extern int
phy_tsc_iblk_read(phy_ctrl_t *pc, uint32_t addr, uint32_t *data);

#define BCM54282_MII_ANPr             (0x00000005)
#define BCMI_TSC_XGXS_LP_BASE_PAGE1r  (0x0000c198)
#define SOC_PA_PAUSE_TX        (1 << 0)       /* TX pause capable */
#define SOC_PA_PAUSE_RX        (1 << 1)       /* RX pause capable */

int
phy_pause_get(uint8 unit, uint8 lport, BOOL *tx_pause, BOOL *rx_pause)
{
    int rv = 0;
    phy_ctrl_t *pc = BMD_PORT_PHY_CTRL(unit, lport);
    uint32_t remote_advert, local_advert = (SOC_PA_PAUSE_RX | SOC_PA_PAUSE_TX);
    uint32_t ability;
    PHY_CTRL_CHECK(pc);

    if (PHY_EXTERNAL_MODE(lport)) {
        if (!sal_strcmp(pc->drv->drv_name,"bcm84848")) {
            rv = PHY_CONFIG_GET(BMD_PORT_PHY_CTRL(unit, lport),
                               PhyConfig_AdvRemote, &ability, NULL);
        }
#if !CONFIG_GREYHOUND_ROMCODE
        else if (!sal_strcmp(pc->drv->drv_name,"bcm5428(9)2")) {
            rv = PHY_CONFIG_GET(BMD_PORT_PHY_CTRL(unit, lport),
                               PhyConfig_AdvRemote, &ability, NULL);

        }
        else if (!sal_strcmp(pc->drv->drv_name,"bcm5428(9)0")) {
            rv = PHY_CONFIG_GET(BMD_PORT_PHY_CTRL(unit, lport),
                               PhyConfig_AdvRemote, &ability, NULL);
        }
        else if (!sal_strcmp(pc->drv->drv_name,"bcm54210")) {
            rv = PHY_CONFIG_GET(BMD_PORT_PHY_CTRL(unit, lport),
                               PhyConfig_AdvRemote, &ability, NULL);
        }
#endif
        else {
            return SYS_ERR_PARAMETER;
        }
    } else {
        if (!sal_strcmp(pc->drv->drv_name,"bcmi_tsce_xgxs") ||
            !sal_strcmp(pc->drv->drv_name,"bcmi_qsgmiie_serdes_drv")) {
            /* Assume CL73 AN Bit[11:10] */
            rv = PHY_CONFIG_GET(BMD_PORT_PHY_CTRL(unit, lport),
                               PhyConfig_AdvRemote, &ability, NULL);

        } else {
            return SYS_ERR_PARAMETER;
        }
    }

    if (CDK_FAILURE(rv)) {
        PHY_WARN(pc, ("read remote ability failed!\n"));
        return rv;
    }

    remote_advert = 0;
    if (ability & PHY_ABIL_PAUSE_TX) {
        remote_advert |= SOC_PA_PAUSE_TX;
    }
    if (ability & PHY_ABIL_PAUSE_RX) {
        remote_advert |= SOC_PA_PAUSE_RX;
    }

    /*
     * IEEE 802.3 Flow Control Resolution.
     * Please see $SDK/doc/pause-resolution.txt for more information.
     * Assume local always advertises PASUE and ASYM PAUSE.
     */
    *tx_pause =
            ((remote_advert & SOC_PA_PAUSE_RX) &&
             (local_advert & SOC_PA_PAUSE_RX)) ||
             ((remote_advert & SOC_PA_PAUSE_RX) &&
             !(remote_advert & SOC_PA_PAUSE_TX) &&
             (local_advert & SOC_PA_PAUSE_TX));

    *rx_pause =
             ((remote_advert & SOC_PA_PAUSE_RX) &&
             (local_advert & SOC_PA_PAUSE_RX)) ||
             ((local_advert & SOC_PA_PAUSE_RX) &&
             (remote_advert & SOC_PA_PAUSE_TX) &&
             !(local_advert & SOC_PA_PAUSE_TX));

    return(rv);
}

void *cdk_memset(void *dest,int c,size_t cnt) {
    return sal_memset(dest, c, cnt);
}

void *cdk_memcpy(void *dest,const void *src,size_t cnt) {
    return sal_memcpy(dest, src, cnt);
}

size_t cdk_strlen(const char *str) {
   return sal_strlen(str);
}

int cdk_strncmp(const char *dest,const char *src,size_t cnt) {
   return sal_strncmp(dest, src, cnt);
}

int cdk_strcmp(const char *dest,const char *src) {
   return sal_strcmp(dest, src);
}

char *cdk_strcat(char *dest,const char *src) {
   return sal_strcat(dest, src);
}

char *cdk_strncpy(char *dest,const char *src,size_t cnt) {
	return sal_strncpy(dest, src, cnt);
}

char *cdk_strcpy(char *dest,const char *src) {
	return sal_strcpy(dest, src);
}

int 
cdk_sprintf(char *buf, const char *fmt, ...)
{
    va_list arg_ptr;

    if (buf == NULL || fmt == NULL) {
        return 0;
    }
    
    va_start(arg_ptr, fmt);
    vsprintf(buf, fmt, arg_ptr);
    va_end(arg_ptr);
    
    return sal_strlen(buf) + 1;
}


void
usleep(uint32 usec) {
    sal_usleep(usec);
}

int
phymod_is_write_disabled(const phymod_access_t *pa, uint32_t *data)
{
    if(pa && PHYMOD_ACC_BUS(pa)) {
		if (PHYMOD_ACC_BUS(pa)->is_write_disabled == NULL) { 
			*data = 0; 
		} else {
          return PHYMOD_ACC_BUS(pa)->is_write_disabled(PHYMOD_ACC_USER_ACC(pa),data);
		}
    } else {
          *data = 0;
	}
    return 0;
}

