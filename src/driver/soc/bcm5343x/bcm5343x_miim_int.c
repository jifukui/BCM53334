/*
 * $Id: bcm5343x_miim_int.c,v 1.6 Broadcom SDK $
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

static const uint16 _phy_addr_bcm5343x[] = {
    0xFF,  /* Port  0 (cmic) N/A */
    0xFF,  /* Port  1        N/A */
    0xFF,  /* Port  2        N/A */
    0xFF,  /* Port  3        N/A */
    0xFF,  /* Port  4        N/A */
    0xFF,  /* Port  5        N/A */
    0xFF,  /* Port  6        N/A */
    0xFF,  /* Port  7        N/A */
    0xFF,  /* Port  8        N/A */
    0xFF,  /* Port  9        N/A */
    0x01 + CDK_XGSM_MIIM_IBUS(0), /* Port 10 IntBus=0 Addr=0x01 */
    0x02 + CDK_XGSM_MIIM_IBUS(0), /* Port 11 IntBus=0 Addr=0x02 */
    0x03 + CDK_XGSM_MIIM_IBUS(0), /* Port 12 IntBus=0 Addr=0x03 */
    0x04 + CDK_XGSM_MIIM_IBUS(0), /* Port 13 IntBus=0 Addr=0x04 */
    0x05 + CDK_XGSM_MIIM_IBUS(0), /* Port 14 IntBus=0 Addr=0x05 */
    0x06 + CDK_XGSM_MIIM_IBUS(0), /* Port 15 IntBus=0 Addr=0x06 */
    0x07 + CDK_XGSM_MIIM_IBUS(0), /* Port 16 IntBus=0 Addr=0x07 */
    0x08 + CDK_XGSM_MIIM_IBUS(0), /* Port 17 IntBus=0 Addr=0x08 */
    0x0d + CDK_XGSM_MIIM_IBUS(1), /* Port 18 IntBus=1 Addr=0x0d */
    0x0d + CDK_XGSM_MIIM_IBUS(1), /* Port 19 IntBus=1 Addr=0x0d */
    0x0d + CDK_XGSM_MIIM_IBUS(1), /* Port 20 IntBus=1 Addr=0x0d */
    0x0d + CDK_XGSM_MIIM_IBUS(1), /* Port 21 IntBus=1 Addr=0x0d */
    0x0d + CDK_XGSM_MIIM_IBUS(1), /* Port 22 IntBus=1 Addr=0x0d */
    0x0d + CDK_XGSM_MIIM_IBUS(1), /* Port 23 IntBus=1 Addr=0x0d */
    0x0d + CDK_XGSM_MIIM_IBUS(1), /* Port 24 IntBus=1 Addr=0x0d */
    0x0d + CDK_XGSM_MIIM_IBUS(1), /* Port 25 IntBus=1 Addr=0x0d */
    0x09 + CDK_XGSM_MIIM_IBUS(0), /* Port 26 IntBus=0 Addr=0x09 */
    0x0a + CDK_XGSM_MIIM_IBUS(0), /* Port 27 IntBus=0 Addr=0x0a */
    0x0b + CDK_XGSM_MIIM_IBUS(0), /* Port 28 IntBus=0 Addr=0x0b */
    0x0c + CDK_XGSM_MIIM_IBUS(0), /* Port 29 IntBus=0 Addr=0x0c */
    0x0d + CDK_XGSM_MIIM_IBUS(0), /* Port 30 IntBus=0 Addr=0x0d */
    0x0e + CDK_XGSM_MIIM_IBUS(0), /* Port 31 IntBus=0 Addr=0x0e */
    0x0f + CDK_XGSM_MIIM_IBUS(0), /* Port 32 IntBus=0 Addr=0x0f */
    0x10 + CDK_XGSM_MIIM_IBUS(0), /* Port 33 IntBus=0 Addr=0x10 */
};

static uint32_t
_phy_addr(int pport)
{
    if (SOC_PORT_P2L_MAPPING(pport) > BCM5343X_LPORT_MAX) {
        return 0xFF;
    }

    return _phy_addr_bcm5343x[pport];
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
    return pport - 2;
}

phy_bus_t phy_bus_bcm5343x_miim_int = {
    "bcm5343x_miim_int",
    _phy_addr,
    _read,
    _write,
    _phy_inst
};
