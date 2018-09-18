/*
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
 *  $Id: eagle_tsc_dependencies.c,v 1.4 Broadcom SDK $
*/  

#ifdef LINUX
/* #include <stdint.h> */
#endif
#include "srds_api_err_code.h"
#include "eagle_tsc_common.h"
#include <phymod/acc/phymod_tsc_iblk.h>
#include "eagle_tsc_dependencies.h"
#include <phymod/phymod.h> 


err_code_t eagle_tsc_delay_us(uint32_t delay_us){
    PHYMOD_USLEEP(delay_us);
    return ( 0 );
}

err_code_t eagle_tsc_delay_ns(uint16_t delay_ns) {
    uint32_t delay;
    delay = delay_ns / 1000; 
    if (!delay ) {
        delay = 1;
    }
    PHYMOD_USLEEP(delay);
    return ( 0 );
}

uint8_t eagle_tsc_get_lane(const phymod_access_t *pa) {
    if (pa->lane_mask == 0x1) {
       return ( 0 );
    } else if (pa->lane_mask == 0x2) {
       return ( 1 );
    } else if (pa->lane_mask == 0x4) {
       return ( 2 );
    } else if (pa->lane_mask == 0x8) {
       return ( 3 );
    } else if (pa->lane_mask == 0xc) {
       return ( 2 );
    } else {
       return ( 0 );
    }
}

/**
@brief   Eagle PMD Modify-Write
@param   phymod access handle to current Eagle Context
@param   address
@param   mask
@param   lsb
@param   val
@returns The ERR_CODE_NONE upon successful completion, else returns ERR_CODE_DATA_NOTAVAIL
*/
err_code_t eagle_tsc_pmd_mwr_reg(const phymod_access_t *pa, uint16_t address, uint16_t mask, uint8_t lsb, uint16_t val) { 

    uint32_t mymask = ( uint32_t) mask;
    phymod_access_t pa_copy;    
    uint32_t i;
    int error_code;

    uint32_t data = ((mymask << 16) & 0xffff0000) | val << lsb;

    error_code = 0;
    PHYMOD_MEMCPY(&pa_copy, pa, sizeof(phymod_access_t));
    for(i=1; i <= 0x8; i = i << 1) {
        if ( i & pa->lane_mask ) {
            pa_copy.lane_mask = i;
            error_code+=phymod_tsc_iblk_write(&pa_copy, (PHYMOD_REG_ACC_TSC_IBLK | 0x10000 | (uint32_t) address), data);
        }
    }
    if(error_code)
      return  ERR_CODE_DATA_NOTAVAIL;
    return  ERR_CODE_NONE;
}


/* phymod_tsc_iblk_read(const phymod_access_t *pa, uint32_t addr, uint32_t *data) */
uint16_t eagle_tsc_pmd_rd_reg(const phymod_access_t *pa, uint16_t address){

    uint32_t data;
    phymod_tsc_iblk_read(pa, (PHYMOD_REG_ACC_TSC_IBLK | 0x10000 | (uint32_t) address), &data);
    data = data & 0xffff; 
    return ( (uint16_t)data );
}


err_code_t eagle_tsc_pmd_rdt_reg(const phymod_access_t *pa, uint16_t address, uint16_t *val) {
    uint32_t data;
    phymod_tsc_iblk_read(pa, (PHYMOD_REG_ACC_TSC_IBLK | 0x10000 | (uint32_t) address), &data);
    data = data & 0xffff; 
    *val = (uint16_t)data;
    return ( 0 );
}

/**
@brief   Eagle PMD Write
@param   phymod access handle to current Eagle Context
@param   address
@param   val
@returns The ERR_CODE_NONE upon successful completion, else returns ERR_CODE_DATA_NOTAVAIL
*/
err_code_t eagle_tsc_pmd_wr_reg(const phymod_access_t *pa, uint16_t address, uint16_t val){
    uint32_t data = 0xffff & val;
    uint32_t error_code;

    error_code = phymod_tsc_iblk_write(pa, (PHYMOD_REG_ACC_TSC_IBLK | 0x10000 | (uint32_t) address), data);
    if(error_code)
      return  ERR_CODE_DATA_NOTAVAIL;
    return  ERR_CODE_NONE;
}

uint8_t eagle_tsc_get_core(void) {
    return(0);
}

err_code_t eagle_tsc_uc_lane_idx_to_system_id(char *string , uint8_t uc_lane_idx) {
    static char info[256];
   /* Indicates Eagle Core */
    PHYMOD_SPRINTF(info, "%s_%d", "FC_", uc_lane_idx);
    /* PHYMOD_STRNCPY(string,info, PHYMOD_STRLEN(info)); */
    PHYMOD_STRCPY(string,info );
    return ERR_CODE_NONE;
}


