/*
 * $Id: bcm5346x_miim_int.c,v 1.3 Broadcom SDK $
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
#include "xgsm_miim.h"
#include "soc/bcm5346x.h"

static const uint16 _phy_addr_bcm5346x[] = {
    0xFF,  /* Port  0 (cmic) N/A */
    0x01 + CDK_XGSM_MIIM_IBUS(0), /* Port  1 ( ge0) IntBus=0 Addr=0x01*/
    0x01 + CDK_XGSM_MIIM_IBUS(0), /* Port  2 ( ge1) IntBus=0 Addr=0x02*/
    0x01 + CDK_XGSM_MIIM_IBUS(0), /* Port  3 ( ge2) IntBus=0 Addr=0x03*/
    0x01 + CDK_XGSM_MIIM_IBUS(0), /* Port  4 ( ge3) IntBus=0 Addr=0x04*/
    0x01 + CDK_XGSM_MIIM_IBUS(1), /* Port  5 ( ge4) IntBus=1 Addr=0x01*/
    0x01 + CDK_XGSM_MIIM_IBUS(1), /* Port  6 ( ge5) IntBus=1 Addr=0x01*/
    0x01 + CDK_XGSM_MIIM_IBUS(1), /* Port  7 ( ge6) IntBus=1 Addr=0x01*/
    0x01 + CDK_XGSM_MIIM_IBUS(1), /* Port  8 ( ge7) IntBus=1 Addr=0x01*/
    0x05 + CDK_XGSM_MIIM_IBUS(1), /* Port  9 ( ge8) IntBus=1 Addr=0x05*/
    0x05 + CDK_XGSM_MIIM_IBUS(1), /* Port 10 ( ge9) IntBus=1 Addr=0x05*/
    0x05 + CDK_XGSM_MIIM_IBUS(1), /* Port 11 ( ge10) IntBus=1 Addr=0x05*/
    0x05 + CDK_XGSM_MIIM_IBUS(1), /* Port 12 ( ge11) IntBus=1 Addr=0x05*/

};

static uint32_t
_phy_addr(int pport)
{
    if (SOC_PORT_P2L_MAPPING(pport) > BCM5346X_LPORT_MAX) {
        return 0;
    }

    return _phy_addr_bcm5346x[SOC_PORT_P2L_MAPPING(pport)];
}

static int 
_read(int unit, uint32_t addr, uint32_t reg, uint32_t *val)
{
    if (CDK_XGSM_MIIM_IBUS_NUM(addr) == 1) {
        return cdk_xgsm_sbus_read(unit, addr, reg, val);
    } else {
        return cdk_xgsm_miim_read(unit, addr, reg, val);
    }
}

static int 
_write(int unit, uint32_t addr, uint32_t reg, uint32_t val)
{
    if (CDK_XGSM_MIIM_IBUS_NUM(addr) == 1) {
        return cdk_xgsm_sbus_write(unit, addr, reg, val);
    } else {
        return cdk_xgsm_miim_write(unit, addr, reg, val);
    }
}

static int
_phy_inst(int pport)
{
    return (pport - 1) % 4;
}

phy_bus_t phy_bus_bcm5346x_miim_int = {
    "bcm5346x_miim_int",
    _phy_addr,
    _read,
    _write,
    _phy_inst
};
