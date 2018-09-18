/*
 * $Id: brd_misc.c,v 1.80 Broadcom SDK $
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

#ifdef __C51__
#ifdef CODE_USERCLASS
#pragma userclass (code = brdimpl)
#endif /* CODE_USERCLASS */
#endif /* __C51__ */

#include "system.h"
#include "utils/ports.h"
#include "appl/persistence.h"

#ifdef _BCM95333X_
#include "soc/bcm5333x.h"
#endif
/*
 * Typical implementation of certain board functions
 *
 * Note: Only applicable for single-unit board.
 */

#if defined(CFG_HW_CABLE_DIAG_INCLUDED)
/* Cable diag result */
#define     CD_NO_FAULT         0x0000 /* Good cable/PCB signal paths, but no Gigabit link */
#define     CD_PIN_SHORT_OR_XT  0x0001 /* Pin-short or cross-talk along 2 or more cable/PCB signal paths */
#define     CD_OPEN             0x0002 /* One or both pins are open for a twisted pair */
#define     CD_SHORT            0x0004 /* Two pins from the same twisted pair are shorted together */
#define     CD_FORCED           0x0008 /* Persistent noise present (most likely caused by Forced 10/100) */
#define     CD_GLINK            0x0010 /* Gigabit link is up and running */
#define     CD_NO_CABLE         0x0020 /* No cable connected */
#endif /* CFG_HW_CABLE_DIAG_INCLUDED */

sys_error_t
_port_cable_diag(uint8 unit, uint8 lport, int16 *fault, int16 *length)
{
#if defined(CFG_HW_CABLE_DIAG_INCLUDED)

    int i, max_pair = 4;
    phy_port_cable_diag_t cd_state;
    int duplex;
    uint32 speed;

    *fault = 0;
    *length = 0;

    /* Disable EEE before cable diag */
#ifdef CFG_SWITCH_EEE_INCLUDED
        bmd_phy_eee_set(unit, lport, PHY_EEE_NONE);
#endif
    /* ECD_CTRL : break link */
    SOC_IF_ERROR_RETURN(PHY_CABLE_DIAG(BMD_PORT_PHY_CTRL(unit, lport), &cd_state));
    /* Recover EEE enabled after cable diag */
#ifdef CFG_SWITCH_EEE_INCLUDED
        bmd_phy_eee_set(unit, lport, PHY_EEE_802_3);
#endif

    /* Check NO cable case */
    if ((cd_state.pair_state[0] == PhyPortCableState_Open) &&
        (0 == cd_state.pair_len[0])) {
        for (i = 1; i < max_pair; i++) {
            if (cd_state.pair_state[i] != PhyPortCableState_Open ||
                cd_state.pair_len[i] != 0) {
                break;
            }
        }
        if (i == max_pair) {
            *fault = CD_NO_CABLE;
            return SYS_OK;
        }
    }

    SOC_IF_ERROR_RETURN(PHY_SPEED_GET(BMD_PORT_PHY_CTRL(unit, lport), &speed));
    SOC_IF_ERROR_RETURN(PHY_DUPLEX_GET(BMD_PORT_PHY_CTRL(unit, lport), &duplex));
    if ((speed != 1000) && (speed != 0)){
        max_pair = 2;
    }

    for (i = 0; i < max_pair; i++) {
        *fault |= cd_state.pair_state[i];
        *length = MAX(cd_state.pair_len[i], *length);
    }

    if ((*fault & PhyPortCableState_Short) ||
        (*fault & PhyPortCableState_Crosstalk)) {
        *fault = CD_PIN_SHORT_OR_XT;
    } else if ((*fault & PhyPortCableState_Open) && (*fault & PhyPortCableState_Short)) {
        *fault = CD_OPEN | CD_SHORT;
    } else if (*fault & PhyPortCableState_Open) {
        *fault = CD_OPEN;
    } else if (*fault & PhyPortCableState_Short) {
        *fault = CD_SHORT;
    } else {
        *fault = CD_NO_FAULT;
    }

    *length *= 100;
    SAL_DEBUGF((" fault = 0x%x, length = %d\n", *fault, *length));

    /* Re-initialize JUMBO frame support */
    SOC_IF_ERROR_RETURN(PHY_INIT(BMD_PORT_PHY_CTRL(unit, lport)));

#endif /* CFG_HW_CABLE_DIAG_INCLUDED */
    return SYS_OK;
}

/*
 * Cable Diagnostics
 */
sys_error_t
brdimpl_port_cable_diag(uint16 uport, port_cable_diag_t *status)
{
    sys_error_t rv = SYS_OK;

#if defined(CFG_HW_CABLE_DIAG_INCLUDED)

    uint8 unit, lport;
    int16 fault, length;

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));
#ifdef _BCM95333X_    
    /* Get chip local port for BMD_PORT_PHY_CTRL to cable diag */
    lport = SOC_PORT_P2L_MAPPING(lport);
#endif    

    SOC_IF_ERROR_RETURN(
        _port_cable_diag(unit, lport, &fault, &length));

    status->length = (int32)(length/100);

    switch (fault) {
        case CD_NO_CABLE:
            status->state = PORT_CABLE_STATE_NO_CABLE;
            break;
        case (CD_OPEN|CD_SHORT):
            status->state = PORT_CABLE_STATE_OPENSHORT;
            break;
        case CD_OPEN:
            status->state = PORT_CABLE_STATE_OPEN;
            break;
        case CD_SHORT:
            status->state = PORT_CABLE_STATE_SHORT;
            break;
        case CD_PIN_SHORT_OR_XT:
            status->state = PORT_CABLE_STATE_CROSSTALK;
            break;
        default:
            status->state = PORT_CABLE_STATE_OK;
    }
#endif
    return rv;
}

#ifdef CFG_RESET_BUTTON_INCLUDED
static BOOL reset_button_detect = FALSE;

void
brdimpl_reset_button_detect(void *param) REENTRANT
{
    BOOL hard_reset;

    UNREFERENCED_PARAMETER(param);
    if (reset_button_detect == TRUE) {
        if (board_reset_button_get() == TRUE) {
#if CFG_PERSISTENCE_SUPPORT_ENABLED
            sal_printf("Load factory and Reset......\n");
            sal_usleep(500000);
            persistence_restore_factory_defaults();
            persistence_save_all_current_settings();
            board_reset((void *)&hard_reset);
#endif            
        } else {
            sal_printf("Reset......\n");
            sal_usleep(500000);
            board_reset((void *)&hard_reset);
        }
    } else {
        if (board_reset_button_get() == TRUE) {
            reset_button_detect = TRUE;
        }
    }    
} 
#endif /* CFG_RESET_BUTTON_INCLUDED */
