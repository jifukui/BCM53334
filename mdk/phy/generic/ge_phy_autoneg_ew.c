/*
 * $Id: ge_phy_autoneg_ew.c,v 1.2 Broadcom SDK $
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
 * Generic PHY driver.
 *
 */

#include <phy/phy.h>
#include <phy/ge_phy.h>

/*
 * Function:     
 *      ge_phy_autoneg_ew (Autoneg-ed mode with E@W on).
 * Purpose:    
 *      Determine autoneg-ed mode between 
 *      two ends of a link.
 * Parameters:
 *      pc - PHY control structure
 *      speed - (OUT) greatest common speed
 *      duplex - (OUT) greatest common duplex
 * Returns:    
 *      CDK_E_xxx.
 */
int
ge_phy_autoneg_ew(phy_ctrl_t *pc, uint32_t *speed, int *duplex)
{
    int ioerr = 0;
    int t_speed, t_duplex;
    uint32_t mii_assr;

    PHY_CTRL_CHECK(pc);

    /* Read status from Auxiliary Status Register */
    ioerr += PHY_BUS_READ(pc, 0x19, &mii_assr);

    switch ((mii_assr >> 8) & 0x7) {
    case 0x7:
        t_speed = 1000;
        t_duplex = TRUE;
        break;
    case 0x6:
        t_speed = 1000;
        t_duplex = FALSE;
        break;
    case 0x5:
        t_speed = 100;
        t_duplex = TRUE;
        break;
    case 0x3:
        t_speed = 100;
        t_duplex = FALSE;
        break;
    case 0x2:
        t_speed = 10;
        t_duplex = TRUE;
        break;
    case 0x1:
        t_speed = 10;
        t_duplex = FALSE;
        break;
    default:
        t_speed = 0; /* 0x4 is 100BASE-T4 which is not supported */
        t_duplex = FALSE;
        break;
    }

    if (speed)  *speed  = t_speed;
    if (duplex)    *duplex = t_duplex;

    return ioerr ? CDK_E_IO : CDK_E_NONE;
}
