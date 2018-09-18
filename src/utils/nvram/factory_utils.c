/*
 * $Id: factory_utils.c,v 1.8 Broadcom SDK $
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
#include "utils/factory.h"

#if (CFG_FLASH_SUPPORT_ENABLED && defined(CFG_FACTORY_CONFIG_INCLUDED))

sys_error_t
factory_config_get(factory_config_t *pcfg)
{
    factory_config_t cfg;
    
    /*
     * Check block magic
     */
    flash_read((hsaddr_t)FACTORY_CONFIG_BASE_ADDR+FACTORY_CONFIG_OFFSET,
               (void *)&cfg, sizeof(factory_config_t)); 

    if (cfg.magic != FACTORY_CONFIG_MAGIC) {
        /* Block magic must be matched */
       return SYS_ERR;
    }

    if (pcfg) {
        sal_memcpy(pcfg, &cfg, sizeof(factory_config_t));
    }

    return SYS_OK;
}

sys_error_t
factory_config_set(factory_config_t *cfg)
{
    factory_config_t *pcfg;
    uint8 *buf;
    sys_error_t rv;
    size_t  block_size;

    block_size = flash_block_size(BOARD_FIRMWARE_ADDR);
    /* Allocate and copy block data */
    buf = sal_malloc(block_size);
    if (!buf) {
#if CFG_CONSOLE_ENABLED
        sal_printf("factory_config_set: malloc failed!\n");
#endif
        return SYS_ERR_OUT_OF_RESOURCE;
    }

    flash_read((hsaddr_t)FACTORY_CONFIG_BASE_ADDR, buf, block_size);

    pcfg = (factory_config_t *)(buf+FACTORY_CONFIG_OFFSET);

    pcfg->magic = FACTORY_CONFIG_MAGIC;

    sal_memcpy(pcfg->mac, cfg->mac, 6);

#ifdef CFG_PRODUCT_REGISTRATION_INCLUDED
    pcfg->serial_num_magic = cfg->serial_num_magic;
    sal_memcpy(pcfg->serial_num, cfg->serial_num, 20);
#endif /* CFG_PRODUCT_REGISTRATION_INCLUDED */    

    rv = flash_erase((hsaddr_t)FACTORY_CONFIG_BASE_ADDR, block_size);
    if (rv != SYS_OK) {
#if CFG_CONSOLE_ENABLED
        sal_printf("factory_config_set: failed to erase region 0x%x\n",
                   (hsaddr_t)FACTORY_CONFIG_BASE_ADDR);
#endif
        goto done;
    }

    rv = flash_program((hsaddr_t)FACTORY_CONFIG_BASE_ADDR,
                        (const void *)buf,
                        block_size);

    if (rv != SYS_OK) {
#if CFG_CONSOLE_ENABLED
        sal_printf("factory_config_set: failed to program region 0x%x\n",
                   (hsaddr_t)FACTORY_CONFIG_BASE_ADDR);
#endif
    }
done:
    sal_free(buf);
    return rv;
}

#endif /* CFG_FLASH_SUPPORT_ENABLED  && defined(CFG_FACTORY_CONFIG_INCLUDED) */
