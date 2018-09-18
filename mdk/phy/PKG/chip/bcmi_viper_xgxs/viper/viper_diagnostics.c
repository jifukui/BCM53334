/*
 *         
 * $Id: viper_diagnostics.c,v 1.1 Broadcom SDK $
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
#include <phymod/phymod_diagnostics.h>
#if 0
#include <phymod/phymod_diagnostics_dispatch.h>
#endif
#include <phymod/phymod_util.h>
#include "viper_inc.h"

#ifdef PHYMOD_VIPER_SUPPORT

/*phymod, internal enum mappings*/
STATIC
int _viper_prbs_poly_phymod_to_viper(phymod_prbs_poly_t phymod_poly, viper_prbs_poly_t *viper_poly)
{
    switch(phymod_poly){
    case phymodPrbsPoly7:
        *viper_poly = VIPER_PRBS_POLYNOMIAL_7;
        break;
    case phymodPrbsPoly15:
        *viper_poly = VIPER_PRBS_POLYNOMIAL_15;
        break;
    case phymodPrbsPoly23:
        *viper_poly = VIPER_PRBS_POLYNOMIAL_23;
        break;
    case phymodPrbsPoly31:
        *viper_poly = VIPER_PRBS_POLYNOMIAL_31;
        break;
    default:
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_PARAM, (_PHYMOD_MSG("unsupported poly for viper %u"), phymod_poly));
    }
    return PHYMOD_E_NONE;
}

STATIC
int _viper_prbs_poly_viper_to_phymod(viper_prbs_poly_t viper_poly, phymod_prbs_poly_t *phymod_poly)
{
    switch(viper_poly){
    case VIPER_PRBS_POLYNOMIAL_7:
        *phymod_poly = phymodPrbsPoly7;
        break;
    case VIPER_PRBS_POLYNOMIAL_15:
        *phymod_poly = phymodPrbsPoly15;
        break;
    case VIPER_PRBS_POLYNOMIAL_23:
        *phymod_poly = phymodPrbsPoly23;
        break;
    case VIPER_PRBS_POLYNOMIAL_31:
        *phymod_poly = phymodPrbsPoly31;
        break;
    default:
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_INTERNAL, (_PHYMOD_MSG("uknown poly %u"), viper_poly));
    }
    return PHYMOD_E_NONE;
}



int viper_phy_rx_slicer_position_set(const phymod_phy_access_t* phy, uint32_t flags, const phymod_slicer_position_t* position)
{
        
    
    /* Place your code here */

        
    return PHYMOD_E_NONE;
    
}

int viper_phy_rx_slicer_position_get(const phymod_phy_access_t* phy, uint32_t flags, phymod_slicer_position_t* position)
{
        
    
    /* Place your code here */

        
    return PHYMOD_E_NONE;
    
}


int viper_phy_rx_slicer_position_max_get(const phymod_phy_access_t* phy, uint32_t flags, const phymod_slicer_position_t* position_min, const phymod_slicer_position_t* position_max)
{
        
    
    /* Place your code here */

        
    return PHYMOD_E_NONE;
    
}


int viper_phy_prbs_config_set(const phymod_phy_access_t* phy, uint32_t flags , const phymod_prbs_t* prbs)
{
    phymod_phy_access_t phy_copy;
    viper_prbs_poly_t viper_poly;
    int start_lane, num_lane;
    int i = 0;

    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));

    /*next figure out the lane num and start_lane based on the input*/
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    PHYMOD_IF_ERR_RETURN(_viper_prbs_poly_phymod_to_viper(prbs->poly, &viper_poly));

    for (i = 0; i < num_lane; i++) {
         phy_copy.access.lane_mask = 0x1 << (i + start_lane);
         PHYMOD_IF_ERR_RETURN(viper_prbs_lane_inv_data_set(&phy_copy.access, num_lane, prbs->invert));
         PHYMOD_IF_ERR_RETURN(viper_prbs_lane_poly_set(&phy_copy.access, num_lane, viper_poly));
    }

    return PHYMOD_E_NONE;
    
}

int viper_phy_prbs_config_get(const phymod_phy_access_t* phy, uint32_t flags , phymod_prbs_t* prbs)
{
    phymod_phy_access_t phy_copy;
    phymod_prbs_t config_tmp;
    viper_prbs_poly_t viper_poly;
    int start_lane, num_lane;
    int i = 0;

    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));

    /*next figure out the lane num and start_lane based on the input*/
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    for (i = 0; i < num_lane; i++) {
        PHYMOD_IF_ERR_RETURN(viper_prbs_lane_inv_data_get(&phy_copy.access, num_lane ,&config_tmp.invert));
        PHYMOD_IF_ERR_RETURN(viper_prbs_lane_poly_get(&phy_copy.access, num_lane, &viper_poly));
        PHYMOD_IF_ERR_RETURN(_viper_prbs_poly_viper_to_phymod(viper_poly, &config_tmp.poly));
        prbs->invert = config_tmp.invert;
        prbs->poly = config_tmp.poly;
    }
    return PHYMOD_E_NONE;
    
}


int viper_phy_prbs_enable_set(const phymod_phy_access_t* phy, uint32_t flags , uint32_t enable)
{
    phymod_phy_access_t phy_copy;
    int start_lane, num_lane;
    int i = 0;

    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));

    /*next figure out the lane num and start_lane based on the input*/
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    for (i = 0; i < num_lane; i++) {
        PHYMOD_IF_ERR_RETURN(viper_prbs_enable_set(&phy_copy.access, num_lane, enable));
    }
    return PHYMOD_E_NONE;
    
}

int viper_phy_prbs_enable_get(const phymod_phy_access_t* phy, uint32_t flags , uint32_t* enable)
{
    phymod_phy_access_t phy_copy;
    uint32_t enable_tmp;
    int start_lane, num_lane;
    int i = 0;

    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));

    /*next figure out the lane num and start_lane based on the input*/
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    for (i = 0; i < num_lane; i++) {
        PHYMOD_IF_ERR_RETURN(viper_prbs_enable_get(&phy_copy.access, num_lane, &enable_tmp));
        *enable = enable_tmp;
    }
    return PHYMOD_E_NONE;
    
}


int viper_phy_prbs_status_get(const phymod_phy_access_t* phy, uint32_t flags, phymod_prbs_status_t* prbs_status)
{
    viper_prbs_status_t status;
    int i, start_lane, num_lane;
    phymod_phy_access_t phy_copy;

    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));
    /* next figure out the lane num and start_lane based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    for (i = 0; i < num_lane; i++) {
        phy_copy.access.lane_mask = 0x1 << (i + start_lane);
        PHYMOD_IF_ERR_RETURN(viper_prbs_status_get(&phy_copy.access, &status));
        prbs_status->prbs_lock = status.prbs_lock; 
        prbs_status->prbs_lock_loss = status.prbs_lock_loss; 
        prbs_status->error_count = status.error_count; 
    }
 
    return PHYMOD_E_NONE;
}


int viper_phy_pattern_config_set(const phymod_phy_access_t* phy, const phymod_pattern_t* pattern)
{
        
    
    /* Place your code here */

        
    return PHYMOD_E_NONE;
    
}

int viper_phy_pattern_config_get(const phymod_phy_access_t* phy, phymod_pattern_t* pattern)
{
        
    
    /* Place your code here */

        
    return PHYMOD_E_NONE;
    
}


int viper_phy_pattern_enable_set(const phymod_phy_access_t* phy, uint32_t enable, const phymod_pattern_t* pattern)
{
        
    
    /* Place your code here */

        
    return PHYMOD_E_NONE;
    
}

int viper_phy_pattern_enable_get(const phymod_phy_access_t* phy, uint32_t* enable)
{
        
    
    /* Place your code here */

        
    return PHYMOD_E_NONE;
    
}


int viper_core_diagnostics_get(const phymod_core_access_t* core, phymod_core_diagnostics_t* diag)
{
        
    
    /* Place your code here */

        
    return PHYMOD_E_NONE;
    
}


int viper_phy_diagnostics_get(const phymod_phy_access_t* phy, phymod_phy_diagnostics_t* diag)
{
        
    
    /* Place your code here */

        
    return PHYMOD_E_NONE;
    
}


int viper_phy_pmd_info_dump(const phymod_phy_access_t* phy, char* type)
{
        
    
    /* Place your code here */

        
    return PHYMOD_E_NONE;
    
}


int viper_phy_pcs_info_dump(const phymod_phy_access_t* phy, char* type)
{
        
    
    /* Place your code here */

        
    return PHYMOD_E_NONE;
    
}


#endif /* PHYMOD_VIPER_SUPPORT */
