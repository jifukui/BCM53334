/*
 *         
 * $Id: phymod.c,v 1.2 Broadcom SDK $
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
 *     
 */

#include <phymod/phymod.h>
#include <phymod/phymod_acc.h>

int phymod_osr_mode_to_actual_os(phymod_osr_mode_t osr_mode, uint32_t* os_int, uint32_t* os_remainder)
{
    if(PHYMOD_E_OK != phymod_osr_mode_t_validate(osr_mode)) {
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_PARAM, (_PHYMOD_MSG("osr_mode validation failed")));
    }

    if(os_int == NULL) {
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_PARAM, (_PHYMOD_MSG("os_int NULL parameter")));
    }
    if(os_remainder == NULL) {
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_PARAM, (_PHYMOD_MSG("os_remainder NULL parameter")));
    }

    *os_int = 0;
    *os_remainder = 0;

    switch (osr_mode) {
        case phymodOversampleMode1: 
            *os_int = 1;
            break;
        case phymodOversampleMode2: 
            *os_int = 2; 
            break;
        case phymodOversampleMode3: 
            *os_int = 3; 
            break;
        case phymodOversampleMode3P3: 
            *os_int = 3;
            *os_remainder = 300;
            break;
        case phymodOversampleMode4: 
            *os_int = 4; 
            break;
        case phymodOversampleMode5: 
            *os_int = 5; 
            break;
        case phymodOversampleMode7P5: 
            *os_int = 7;
            *os_remainder = 500;
            break;
        case phymodOversampleMode8: 
            *os_int = 8; 
            break;
        case phymodOversampleMode8P25:
            *os_int = 8;
            *os_remainder = 250;
            break;
        case phymodOversampleMode10: 
            *os_int = 10; 
            break;
        case phymodOversampleMode16P5:
            *os_int = 16;
            *os_remainder = 500;
            break;
        case phymodOversampleMode20P625:
            *os_int = 20;
            *os_remainder = 625;
            break;
        default:
            PHYMOD_RETURN_WITH_ERR(PHYMOD_E_INTERNAL, (_PHYMOD_MSG("OS mode not supported")));
    }
         
    return PHYMOD_E_NONE;
}

int phymod_osr_mode_t_validate(phymod_osr_mode_t phymod_osr_mode)
{
        
    if(phymod_osr_mode >= phymodOversampleModeCount) {
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_PARAM, (_PHYMOD_MSG("Parameter is out of range")));
    }
        
    return PHYMOD_E_NONE;
    
}





