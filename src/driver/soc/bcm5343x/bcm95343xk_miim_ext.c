/*
 * $Id: bcm95343xk_miim_ext.c,v 1.9 Broadcom SDK $
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
#include "soc/bcm5343x.h"

extern uint8 qtc_interface;

static const uint16 _phy_addr_bcm5343x[] = {
    0xFF,  /* Port 0 (cmic)  N/A */
    0xFF,  /* Port 1         N/A */
    0xFF,  /* Port 2         N/A */
    0xFF,  /* Port 3         N/A */
    0xFF,  /* Port 4         N/A */
    0xFF,  /* Port 5         N/A */
    0xFF,  /* Port 6         N/A */
    0xFF,  /* Port 7         N/A */
    0xFF,  /* Port 8         N/A */
    0xFF,  /* Port 9         N/A */
    0xFF,  /* Port 10        N/A */
    0xFF,  /* Port 11        N/A */
    0xFF,  /* Port 12        N/A */
    0xFF,  /* Port 13        N/A */
    0xFF,  /* Port 14        N/A */
    0xFF,  /* Port 15        N/A */
    0xFF,  /* Port 16        N/A */
    0xFF,  /* Port 17        N/A */
    0x0A + CDK_XGSM_MIIM_EBUS(0), /* Port 18 ExtBus=0 Addr=0x0A*/
    0x0B + CDK_XGSM_MIIM_EBUS(0), /* Port 19 ExtBus=0 Addr=0x0B*/
    0x0C + CDK_XGSM_MIIM_EBUS(0), /* Port 20 ExtBus=0 Addr=0x0C*/
    0x0D + CDK_XGSM_MIIM_EBUS(0), /* Port 21 ExtBus=0 Addr=0x0D*/
    0x0E + CDK_XGSM_MIIM_EBUS(0), /* Port 22 ExtBus=0 Addr=0x0E*/
    0x0F + CDK_XGSM_MIIM_EBUS(0), /* Port 23 ExtBus=0 Addr=0x0F*/
    0x10 + CDK_XGSM_MIIM_EBUS(0), /* Port 24 ExtBus=0 Addr=0x10*/
    0x11 + CDK_XGSM_MIIM_EBUS(0), /* Port 25 ExtBus=0 Addr=0x11*/
    0xFF,  /* Port 26        N/A */
    0xFF,  /* Port 27        N/A */
    0xFF,  /* Port 28        N/A */
    0xFF,  /* Port 29        N/A */
    0xFF,  /* Port 30        N/A */
    0xFF,  /* Port 31        N/A */
    0xFF,  /* Port 32        N/A */
    0xFF,  /* Port 33        N/A */

};

static uint32_t
_phy_addr(int pport)
{
    if (SOC_PORT_P2L_MAPPING(pport) > BCM5343X_LPORT_MAX) {
        return 0xFF;
    }

    if (qtc_interface == QTC_INTERFACE_QSGMII) {
        return _phy_addr_bcm5343x[pport];
    } else {
        /* SGMII mode or Fiber mode */
        return 0xFF;
    }    
}

static int 
_read(int unit, uint32_t addr, uint32_t reg, uint32_t *val)
{
    return cdk_xgsm_miim_read(unit, addr, reg, val);
}

static int 
_write(int unit, uint32_t addr, uint32_t reg, uint32_t val)
{
    return cdk_xgsm_miim_write(unit, addr, reg, val);
}

static int
_phy_inst(int port)
{
    return port - 2;
}

phy_bus_t phy_bus_bcm95343xk_miim_ext = {
    "bcm95343xk_miim_ext",
    _phy_addr,
    _read,
    _write,
    _phy_inst
};

