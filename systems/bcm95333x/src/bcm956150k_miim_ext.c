/*
 * $Id: bcm956150k_miim_ext.c,v 1.7 Broadcom SDK $
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
#include "soc/bcm5333x.h"

static const uint16 _phy_addr_bcm5339x[] = {
    0xFF,  /* Port  0 (cmic) N/A */
    0xFF,  /* Port  1        N/A */
    0x02 + CDK_XGSM_MIIM_EBUS(0), /* Port  2 ExtBus=0 Addr=0x02*/
    0xFF, /* Port  3        N/A */
    0xFF, /* Port  4        N/A */
    0xFF, /* Port  5        N/A */
    0x03 + CDK_XGSM_MIIM_EBUS(0), /* Port  6 ExtBus=0 Addr=0x03*/
    0xFF, /* Port  7        N/A */
    0xFF, /* Port  8        N/A */
    0xFF, /* Port  9        N/A */
    0x04 + CDK_XGSM_MIIM_EBUS(0), /* Port 10 ExtBus=0 Addr=0x04*/
    0xFF, /* Port  11        N/A */
    0xFF, /* Port  12        N/A */
    0xFF, /* Port  13        N/A */
    0x05 + CDK_XGSM_MIIM_EBUS(0), /* Port 14  ExtBus=0 Addr=0x05*/
    0xFF, /* Port  15        N/A */
    0xFF, /* Port  16        N/A */
    0xFF, /* Port  17        N/A */
    0x06 + CDK_XGSM_MIIM_EBUS(0), /* Port 18  ExtBus=0 Addr=0x06*/
    0xFF, /* Port  19        N/A */
    0xFF, /* Port  20        N/A */
    0xFF, /* Port  21        N/A */
    0x07 + CDK_XGSM_MIIM_EBUS(0), /* Port 22 ExtBus=0 Addr=0x07*/
    0xFF, /* Port  23        N/A */
    0xFF, /* Port  24        N/A */
    0xFF, /* Port  25        N/A */
    0xFF, /* Port  26        N/A */
    0xFF, /* Port  27        N/A */
    0xFF, /* Port  28        N/A */
    0xFF, /* Port  29        N/A */
    0xFF, /* Port  30        N/A */
    0xFF, /* Port  31        N/A */
    0xFF, /* Port  32        N/A */
    0xFF /* Port  33        N/A */
};

static uint32_t
_phy_addr(int pport)
{
    if (pport < BCM5333X_PORT_MAX) {
        switch (hr2_sw_info.devid) {
            case BCM53333_DEVICE_ID:
            case BCM53334_DEVICE_ID:
            case BCM53346_DEVICE_ID:
                if (pport >= 2 && pport < 18) {
                    return 0xFF;
                } else if (pport < 26) {
                    //return pport + ((pport - 0x2) >> 3) + CDK_XGSM_MIIM_EBUS(0);
                    return pport;
                } else if (pport < 34) {
                    return 0x01 + (pport - 26) + CDK_XGSM_MIIM_EBUS(1);
                }
                break;
            case BCM53344_DEVICE_ID:
                /* BCM953344R */
                if (pport >= 2 && pport < 18) {
                    return 0xFF;
                } else if (pport < 26) {
                    return (pport - 18) + CDK_XGSM_MIIM_EBUS(0);
                } else if (pport < 34) {
                    return 0x01 + (pport - 26) + CDK_XGSM_MIIM_EBUS(1);
                }
                break;
            case BCM53393_DEVICE_ID:
            case BCM53394_DEVICE_ID:
                 return _phy_addr_bcm5339x[pport];
            default :
                break;
        }
    }
    /* Should not get here */
    return 0xFF;
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

phy_bus_t phy_bus_bcm956150k_miim_ext = {
    "bcm956150k_miim_ext",
    _phy_addr,
    _read,
    _write,
    _phy_inst
};


