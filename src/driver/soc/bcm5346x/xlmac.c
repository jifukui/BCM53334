/*
 * $Id: xlmac.c,v 1.11.2.4 Broadcom SDK $
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
#include "soc/bcm5346x.h"
#include "utils/system.h"

/*
 * Forward Declarations
 */
mac_driver_t soc_mac_xl;

#define JUMBO_MAXSZ         9216   /* Max legal value (per regsfile) */

#define SOC_XLMAC_SPEED_10     0x0
#define SOC_XLMAC_SPEED_100    0x1
#define SOC_XLMAC_SPEED_1000   0x2
#define SOC_XLMAC_SPEED_2500   0x3
#define SOC_XLMAC_SPEED_10000  0x4

extern sys_error_t soc_ml_port_enable_set(int unit, uint8 port, int enable);

/* Forwards */
static void
soc_port_credit_reset(uint8 unit, uint8 lport)
{
    soc_ml_port_enable_set(unit, lport, 0);
    soc_ml_port_enable_set(unit, lport, 1);
}

static void
soc_port_fifo_reset(uint8 unit, uint8 lport)
{
   uint32 val, orig_val;

   if (IS_XL_PORT(lport)) {
       bcm5346x_reg_get(unit, SOC_PORT_BLOCK(lport), R_XLPORT_SOFT_RESET, &orig_val);
       val = orig_val | (0x1 << SOC_PORT_BLOCK_INDEX(lport));
       bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_SOFT_RESET, val);
       bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_SOFT_RESET, orig_val);
  }
}
 
int
soc_port_thdo_rx_enable_set(int unit, uint8 lport, int enable)
{
   return SYS_OK;
}
int
soc_port_egress_buffer_sft_reset(int unit, uint8 port, int reset)
{
    return SYS_OK;
}

typedef struct {
    uint32 mac_ctrl[2];
    uint32 rx_ctrl[2];
    uint32 tx_ctrl[2];
    uint32 pfc_ctrl[2];
    uint32 pfc_type[2];
    uint32 pfc_opcode[2];
    uint32 pfc_da[2];
    uint32 rx_max_size[2];
    uint32 mac_mode[2];
    uint32 pause_ctrl[2];
    uint32 rx_mac_sa[2];
    uint32 tx_mac_sa[2];
    uint32 llfc_ctrl[2];
    uint32 eee_ctrl[2];
    uint32 eee_timers[2];
    uint32 rx_lss_ctrl[2];
    uint32 rx_vlan_tag[2];
    uint32 e2e_ctrl[2];
} _xlmac_x_cfg_t;

sys_error_t _xlmac_x_register_store(int unit, uint8 lport, _xlmac_x_cfg_t *port_cfg)
{
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), R_XLMAC_CTRL(lport), port_cfg->mac_ctrl, 2);
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), R_XLMAC_RX_CTRL(lport),port_cfg->rx_ctrl, 2);
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), R_XLMAC_TX_CTRL(lport),port_cfg->tx_ctrl, 2);
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), R_XLMAC_PFC_CTRL(lport), port_cfg->pfc_ctrl, 2);
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), R_XLMAC_RX_MAX_SIZE(lport), port_cfg->rx_max_size, 2);
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), R_XLMAC_MODE(lport), port_cfg->mac_mode, 2);
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), R_XLMAC_PAUSE_CTRL(lport), port_cfg->pause_ctrl, 2);
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), R_XLMAC_TX_MAC_SA(lport), port_cfg->tx_mac_sa, 2);
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), R_XLMAC_EEE_CTRL(lport), port_cfg->eee_ctrl, 2);
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), R_XLMAC_LLFC_CTRL(lport), port_cfg->llfc_ctrl, 2);
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), R_XLMAC_EEE_TIMERS(lport), port_cfg->eee_timers, 2);
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), R_XLMAC_RX_LSS_CTRL(lport), port_cfg->rx_lss_ctrl, 2);

    return SYS_OK;
}

sys_error_t _xlmac_x_register_restore(int unit, uint8 lport, _xlmac_x_cfg_t *port_cfg)
{
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), R_XLMAC_CTRL(lport), port_cfg->mac_ctrl, 2);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), R_XLMAC_RX_CTRL(lport), port_cfg->rx_ctrl, 2);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), R_XLMAC_TX_CTRL(lport), port_cfg->tx_ctrl, 2);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), R_XLMAC_PFC_CTRL(lport), port_cfg->pfc_ctrl, 2);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), R_XLMAC_RX_MAX_SIZE(lport), port_cfg->rx_max_size, 2);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), R_XLMAC_MODE(lport), port_cfg->mac_mode, 2);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), R_XLMAC_PAUSE_CTRL(lport), port_cfg->pause_ctrl, 2);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), R_XLMAC_TX_MAC_SA(lport), port_cfg->tx_mac_sa, 2);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), R_XLMAC_EEE_CTRL(lport), port_cfg->eee_ctrl, 2);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), R_XLMAC_LLFC_CTRL(lport), port_cfg->llfc_ctrl, 2);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), R_XLMAC_EEE_TIMERS(lport), port_cfg->eee_timers, 2);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), R_XLMAC_RX_LSS_CTRL(lport), port_cfg->rx_lss_ctrl, 2);

    return SYS_OK;
}

#define SB2_MAX_PORTS_PER_BLOCK 4

int _xlmac_sb2_shim_war(int unit, uint8 lport)
{
    uint32 rval;
    int p, index;
    uint32 ctrl[2];
    pbmp_t block_pbmp;
    _xlmac_x_cfg_t *pcfg;

    if (IS_XL_PORT(lport)) {
        return SYS_OK;
    }

    
    block_pbmp = 0x1E;
    BCM_PBMP_ITER(block_pbmp, p) {
        /* SOFT_RESET[6] */
        bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(p), R_XLMAC_CTRL(p), ctrl, 2);
        /* If any of the port in block is active, dont apply war */
        if((ctrl[0] & 0x40) == 0x0) {
            return SYS_OK;
        }
    }

    /* Come here only when there is active war */
    pcfg = (_xlmac_x_cfg_t *) sal_malloc(sizeof(_xlmac_x_cfg_t) * SB2_MAX_PORTS_PER_BLOCK);
    if (pcfg == NULL) {
        return SYS_ERR_FALSE;
    }

    index = 0;
    BCM_PBMP_ITER(block_pbmp, p) {
        if(index >= SB2_MAX_PORTS_PER_BLOCK) {
            sal_free(pcfg);
            return SYS_ERR_FALSE;
        }
        _xlmac_x_register_store(unit, p, &pcfg[index]);
        index++;
    }

    p = SOC_PORT_BLOCK(lport);
    bcm5346x_reg_get(unit, p, R_XPORT_XMAC_CONTROL, &rval);
    /* XMAC_RESET[0] = 1 */
    rval = 1;
    bcm5346x_reg_set(unit, p, R_XPORT_XMAC_CONTROL, rval); 
    /* XMAC_RESET[0] = 0 */
    rval = 0;
    bcm5346x_reg_set(unit, p, R_XPORT_XMAC_CONTROL, rval);  
    index = 0;
    BCM_PBMP_ITER(block_pbmp, p) {
        _xlmac_x_register_restore(unit, p, &pcfg[index]);
        index++;
    }

    sal_free(pcfg);
    return SYS_OK;
}

/*
 * Function:
 *      soc_egress_cell_count
 * Purpose:
 *      Return the approximate number of cells of packets pending
 *      in the MMU destined for a specified egress.
 */
sys_error_t
soc_egress_cell_count(int unit, uint8 port, uint32 *count)
{
    
    *count = 0;

    return SYS_OK;
}

void
soc_egress_drain_cells(uint8 unit, uint8 lport, uint32 drain_timeout)
{
    uint32 new_cells = 0x0;
    sys_error_t rv;
    int i;
    
    /* Probably not required to continuously check COSLCCOUNT if the fast
     * MMU flush feature is available - done just as an insurance */
    rv = SYS_OK;

    for (i=0; i < drain_timeout; i++) {
        sal_usleep(100); 
        if ((rv = soc_egress_cell_count(unit, lport, &new_cells))) {
            break;
        }

        if (new_cells == 0) {
            rv = SYS_OK;
            return;
        }
    }
    sal_printf("soc_egress_drain_cells %d fail\n", lport);
}

#define SB2_QUEUE_FLUSH_RATE_EXP 14
#define SB2_QUEUE_FLUSH_BURST_EXP 14
#define SB2_QUEUE_FLUSH_RATE_MANTISSA 1023
#define SB2_QUEUE_FLUSH_BURST_MANTISSA 124
#define SB2_QUEUE_FLUSH_CYCLE_SEL 0

typedef struct {
    uint32 lls_l2_shaper_config_c[4];
    uint32 lls_l2_min_config_c[4];
    uint32 lls_l1_shaper_config_c[4];
    uint32 lls_l1_min_config_c[4];
    uint32 lls_l0_shaper_config_c[2];
    uint32 lls_l0_min_config_c[2];
    uint32 lls_port_shaper_config_c[2];
} lls_backup_t;

lls_backup_t lls_backup;

sys_error_t
_soc_egress_metering_freeze(int unit, uint8 lport, void **setting)
{
    int l2_shaper_bucket = 4;
    int index  = SOC_PORT_UC_COSQ_BASE(lport);

    /* PORT       L0          L1        L2
     * CPU         0       (0-2)     (0-5)       (0~47)
     * LBP	      13   	     (3)     (6~8)    (48 ~ 71)
     * ge/xe  (1-12)      (4-15)    (9~20)    (72 ~ 96)
     * ge0         1           4         9    (72 ~ 75)
     */
    bcm5346x_mem_get(unit, M_LLS_L2_SHAPER_CONFIG_C(index/l2_shaper_bucket), lls_backup.lls_l2_shaper_config_c, 4);
    bcm5346x_mem_get(unit, M_LLS_L2_MIN_CONFIG_C(index/l2_shaper_bucket), lls_backup.lls_l2_min_config_c, 4);    
    bcm5346x_mem_get(unit, M_LLS_L1_SHAPER_CONFIG_C(lport-1 + 9), lls_backup.lls_l1_shaper_config_c, 4);
    bcm5346x_mem_get(unit, M_LLS_L1_MIN_CONFIG_C(lport-1 + 9), lls_backup.lls_l1_min_config_c, 4);
    bcm5346x_mem_get(unit, M_LLS_L0_SHAPER_CONFIG_C(lport-1 + 4), lls_backup.lls_l0_shaper_config_c, 2);
    bcm5346x_mem_get(unit, M_LLS_L0_MIN_CONFIG_C(lport-1 + 4), lls_backup.lls_l0_min_config_c, 2);
    bcm5346x_mem_get(unit, M_LLS_PORT_SHAPER_CONFIG_C(lport), lls_backup.lls_port_shaper_config_c, 2);

    return SYS_OK;
}

sys_error_t
_soc_egress_metering_thaw(int unit, uint8 lport, void *setting)
{
    int l2_shaper_bucket = 4;
    int index = SOC_PORT_UC_COSQ_BASE(lport);

    bcm5346x_mem_set(unit, M_LLS_L2_SHAPER_CONFIG_C(index/l2_shaper_bucket), lls_backup.lls_l2_shaper_config_c, 4);
    bcm5346x_mem_set(unit, M_LLS_L2_MIN_CONFIG_C(index/l2_shaper_bucket), lls_backup.lls_l2_min_config_c, 4);    
    bcm5346x_mem_set(unit, M_LLS_L1_SHAPER_CONFIG_C(lport-1 + 9), lls_backup.lls_l1_shaper_config_c, 4);
    bcm5346x_mem_set(unit, M_LLS_L1_MIN_CONFIG_C(lport-1 + 9), lls_backup.lls_l1_min_config_c, 4);
    bcm5346x_mem_set(unit, M_LLS_L0_SHAPER_CONFIG_C(lport-1 + 4), lls_backup.lls_l0_shaper_config_c, 2);
    bcm5346x_mem_set(unit, M_LLS_L0_MIN_CONFIG_C(lport-1 + 4), lls_backup.lls_l0_min_config_c, 2);
    bcm5346x_mem_set(unit, M_LLS_PORT_SHAPER_CONFIG_C(lport), lls_backup.lls_port_shaper_config_c, 2);

    return SYS_OK;
}

int
soc_sb2_cosq_max_bucket_set(int unit, int lport,
                        int index, uint32 level)
{
    uint32 buf[4];
    int l2_shaper_bucket = 4;

    buf[0] = 0x1df03bff;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0;
    bcm5346x_mem_set(unit, M_LLS_L1_SHAPER_CONFIG_C(lport-1 + 9), buf, 4);
    bcm5346x_mem_set(unit, M_LLS_L1_MIN_CONFIG_C(lport-1 + 9), buf, 4);
    bcm5346x_mem_set(unit, M_LLS_L0_SHAPER_CONFIG_C(lport-1 + 4), buf, 2);
    bcm5346x_mem_set(unit, M_LLS_L0_MIN_CONFIG_C(lport-1 + 4), buf, 2);
    bcm5346x_mem_set(unit, M_LLS_PORT_SHAPER_CONFIG_C(lport), buf, 2);
    buf[0] = 0xfdf03bff;
    buf[1] = 0xffbe077f;
    buf[2] = 0xfff7c0ef;
    buf[3] = 0xef81d;  
    bcm5346x_mem_set(unit, M_LLS_L2_SHAPER_CONFIG_C(index/l2_shaper_bucket), buf, 4);
    bcm5346x_mem_set(unit, M_LLS_L2_MIN_CONFIG_C(index/l2_shaper_bucket), buf, 4);

    /* ignore it if flush speed is fine */
    return SYS_OK;
}

int
_soc_kt2_cosq_begin_port_flush(int unit, int port, int hw_index)
{
    uint32 map_entry, map_entry_org;
    int index =0 , eindex = 0;
    uint32 rval;
    int flush_active = 0;
    int timeout_val;

    /* FLUSH_NUM[18:21] FLUSH_ID0[8:0] FLUSH_TYPE[22] FLUSH_ACTIVE[23] */
    bcm5346x_reg_get(unit, R_TOQ_FLUSH0, &rval);     
    rval &= ~(0xF << 18);
    rval |= (1 << 18);
    rval &= ~(0x1FF << 0);
    rval |= ((hw_index & 0x1FF) << 0);
    rval &= ~(0x1 << 23);
    rval |= ((1) << 23);
    rval &= ~(0x1 << 22);
    bcm5346x_reg_set(unit, R_TOQ_FLUSH0, rval);	 

    if (hw_index > 0) {
        index = hw_index / 16 ;
        eindex = (hw_index % 16) / 8;
        /* INDEX1[11:6] INDEX0[5:0] */
        bcm5346x_mem_get(unit, M_MMU_INTFI_FC_MAP_TBL2(index), &map_entry, 1);
        map_entry_org = map_entry;
        if (eindex == 0) {
            map_entry &= ~(0x3F);
        } else {
            map_entry &= ~(0x3F << 6);
        }
        bcm5346x_mem_set(unit, M_MMU_INTFI_FC_MAP_TBL2(index), &map_entry, 1);
    }
	 
    timeout_val = 160000;
    /* Wait for flush completion */
    flush_active = 1; 
    while (flush_active) {
        timeout_val -= 1000;
        sal_usleep(1000);
        /* FLUSH_ACTIVE[23] */ 
        bcm5346x_reg_get(unit, R_TOQ_FLUSH0, &rval);
        if (rval & (1 << 23)) {
            flush_active = 1;
        } else {
            flush_active = 0;
        }
        if (timeout_val <= 0) {
            if (hw_index > 0) {
                bcm5346x_mem_set(unit, M_MMU_INTFI_FC_MAP_TBL2(index), &map_entry_org, 1);
            }
            sal_printf("ERROR: Port %d Queue flush operation failed for queue %d \n", port, hw_index);
            return (SYS_ERR_TIMEOUT);
        }
    }		 

    if (hw_index > 0) {
        bcm5346x_mem_set(unit, M_MMU_INTFI_FC_MAP_TBL2(index), &map_entry_org, 1);
    }

    return SYS_OK;    
}

int
_soc_kt2_cosq_end_port_flush(int unit, int hw_index)
{
    uint32 rval = 0;

    /* QUEUE_NUM[8:0] */
    bcm5346x_reg_get(unit, R_THDO_QUEUE_DISABLE_CFG2, &rval);
    rval &= ~(0x1FF);
    rval |= (hw_index & 0xFE0); 
    bcm5346x_reg_set(unit, R_THDO_QUEUE_DISABLE_CFG2, rval);

    /* QUEUE_RD[2] */
    bcm5346x_reg_get(unit, R_THDO_QUEUE_DISABLE_CFG1, &rval);
    rval |= (1 << 2); 
    bcm5346x_reg_set(unit, R_THDO_QUEUE_DISABLE_CFG1, rval);

    /* QUEUE_BITMAP[13:0] for ML */
    bcm5346x_reg_get(unit, R_THDO_QUEUE_DISABLE_STATUS, &rval); 
    if (rval & (1 << (hw_index % 32))) {
        /* QUEUE_NUM[8:0] */
        bcm5346x_reg_get(unit, R_THDO_QUEUE_DISABLE_CFG2, &rval);
        rval &= ~(0x1FF);
        rval |= (hw_index); 
        bcm5346x_reg_set(unit, R_THDO_QUEUE_DISABLE_CFG2, rval);

        /* QUEUE_WR[0] */
        bcm5346x_reg_get(unit, R_THDO_QUEUE_DISABLE_CFG1, &rval);
        rval |= (1 << 0);
        bcm5346x_reg_set(unit, R_THDO_QUEUE_DISABLE_CFG1, rval);
    }

    return SYS_OK;
}

int
soc_kt_port_flush_state_set(int unit, int port, int enable)
{
    return SYS_OK;
}

sys_error_t soc_kt_port_flush(int unit, int lport, int enable)
{
    int  index;
    /* for queue flush */    
    for (index = SOC_PORT_UC_COSQ_BASE(lport) ; index < (SOC_PORT_UC_COSQ_BASE(lport) + 4); index++ ) {
        if (enable == 1) {
            /* Set the threshold for L2 node to max shaper rate - 33 Gbps */
            SOC_IF_ERROR_RETURN(soc_sb2_cosq_max_bucket_set(unit, lport, index, 0));
            /* Flush the queues */
            SOC_IF_ERROR_RETURN(_soc_kt2_cosq_begin_port_flush(unit, lport, index));
        } else {
            _soc_kt2_cosq_end_port_flush(unit, index);
        }
    } 

    return SYS_OK;
}

int
soc_mmu_backpressure_clear(int unit, uint8 port)
{
    return SYS_OK;
}

sys_error_t
soc_mmu_flush_enable(int unit, uint8 port, int enable)
{
    sys_error_t rv;
    uint32 *setting;

    if (enable == 0) {
        rv = soc_kt_port_flush(unit, port, enable);
    } else {
        rv = soc_mmu_backpressure_clear(unit, port);
        if (SOC_FAILURE(rv)) {
            return rv;
        }
        /*
        *****************************************************
        * NOTE: Must not exit soc_kt_port_flush
        *		 without calling soc_egress_metering_thaw,
        *		 soc_egress_metering_freeze holds the lock.
        *		 soc_egress_metering_freeze releases the lock
        *		 on failure.
        *****************************************************
        */
        rv = _soc_egress_metering_freeze(unit, port, (void **) &setting);
        if (SOC_FAILURE(rv)) {
            return rv;
        }

        rv = soc_kt_port_flush(unit, port, enable);

        /* Restore egress metering configuration. */
        rv = _soc_egress_metering_thaw(unit, port, setting);
    }

    return rv;
}

static void
_mac_xl_drain_cells(uint8 unit, uint8 lport, int notify_phy, int queue_enable)
{
    int         pause_rx, pfc_rx, llfc_rx;
    uint32      entry[2];

    /* Drop all packets after dequeue */
    soc_mmu_flush_enable(unit, lport, 1);

    /* Disable pause/pfc function  */
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* TX_PAUSE_ENf[Bit 17][Bit=1/RX_PAUSE_ENf[Bit 18]=0 */
    pause_rx = (entry[0] & (0x1 << 18));
    entry[0] &= ~(0x1 << 18);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* RX_PFC_ENf[Bit 36]=1 */
    pfc_rx = (entry[1] & (0x1 << 4));
    entry[1] &= ~(0x1 << 4);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_LLFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* RX_LLFC_EN[Bit 1]=1 */
    llfc_rx = (entry[0] & 0x2);
    entry[0] &= ~0x2;
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_LLFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* Assert SOFT_RESET before DISCARD just in case there is no credit left */
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* SOFT_RESET[Bit 6] = 0x1 */
    entry[0] |= (0x1 << 6);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* Drain data in TX FIFO without egressing */
    /* DISCARDf[Bit 2]/EP_DISCARDf[Bit 37]=1 */
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_TX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    entry[0] |= 0x4;
    entry[1] |= (0x1 << 5);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_TX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* Reset EP credit before de-assert SOFT_RESET */
    soc_port_credit_reset(unit, lport);    

    /* De-assert SOFT_RESET to let the drain start */
    /* SOFT_RESET[Bit 6] = 0x0 */
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    entry[0] &= ~(0x1 << 6);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* Notify PHY driver */
#if 0
    soc_phyctrl_notify(unit, port, phyEventStop, PHY_STOP_DRAIN);
#endif

    /*
     * Wait until all queues for the port are empty and no packets are being
     * sent to MAC.
     */
    soc_egress_drain_cells(unit, lport, 250000);

    /* Wait until TX fifo cell count is 0 CELL_CNT[Bit5 ~ Bit0]*/
    do {
        bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_TXFIFO_CELL_CNT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    } while((entry[0] & 0x3F)!= 0);

    /* Notify PHY driver */
#if 0
    soc_phyctrl_notify(unit, port, phyEventResume, PHY_STOP_DRAIN);
#endif

    /* Stop TX FIFO drainging */
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_TX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* DISCARDf[Bit 2]/EP_DISCARDf[Bit 37]=0 */
    entry[0] &= ~0x4;
    entry[1] &= ~(0x1 << 5);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_TX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    if (queue_enable) {
        soc_mmu_flush_enable(unit, lport, 0);
    }

    /* Restore original pause/pfc/llfc configuration */
    if (pfc_rx) {
        bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* RX_PFC_ENf[Bit 36]=1 */
        entry[1] |= (0x1 << 4);
        bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    }

    if (llfc_rx) {
        bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_LLFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
         /* SOFT_RESET[Bit 6] = 0x1 */
        entry[0] |= 0x2;
        bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_LLFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    }

    if (pause_rx) {
        bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* TX_PAUSE_ENf[Bit 17][Bit=1/RX_PAUSE_ENf[Bit 18]=1 */
        entry[0] |= (0x1 << 18);
        bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    }
}

sys_error_t
soc_ml_port_enable_set(int unit, uint8 port, int enable)
{
    uint32       val32;
    uint32       entry[2];      
   
    if (!IS_XL_PORT(port)) {
        bcm5346x_reg_get(unit, R_TXLP_PORT_ENABLE, &val32); 
        /* PORT_ENABLEf [BIT 4 ~ 7] */
        if (enable) {
            val32 |= (1 << (((port-1) % 4) + 4));
        } else {
            val32 &= ~(1 << (((port-1) % 4) + 4));
        }
        bcm5346x_reg_set(unit, R_TXLP_PORT_ENABLE, val32);
        
        
        bcm5346x_reg_get(unit, R_TXLP_MIN_STARTCNT, &val32);
        /* NLP_STARTCNT BIT[2:0] */   
        val32 &= (0xFFFFFFFC);   
        val32 |= 0x03;
        bcm5346x_reg_set(unit, R_TXLP_MIN_STARTCNT, val32);
    }

    sal_memset(entry, 0, sizeof(entry));
    if (enable) {
        /* PRT_ENABLEf EGR_ENABLEm.BIT0 */
        entry[0] |= 1;
    } 
    bcm5346x_mem_set(unit, M_EGR_ENABLE(port), entry, 1);  

    if (!IS_XL_PORT(port)) {
        bcm5346x_reg_get(unit, SOC_PORT_BLOCK(port), R_XPORT_PORT_ENABLE, &val32);
        if (enable) {        
            val32 |= (1 << SOC_PORT_BLOCK_INDEX(port));
        } else {
            val32 &= ~(1 << SOC_PORT_BLOCK_INDEX(port));
        }
        bcm5346x_reg_set(unit, SOC_PORT_BLOCK(port), R_XPORT_PORT_ENABLE, val32);        
    } else {
   	    bcm5346x_reg_get(unit, SOC_PORT_BLOCK(port), R_XLPORT_ENABLE_REG, &val32);       
        if (enable) {
            val32 |= (1 << SOC_PORT_BLOCK_INDEX(port));
        } else {
            val32 &= ~(1 << SOC_PORT_BLOCK_INDEX(port));
        }
        bcm5346x_reg_set(unit, SOC_PORT_BLOCK(port), R_XLPORT_ENABLE_REG, val32);  

        /* Bring IECELL block out of reset. */        
        bcm5346x_reg_get(unit, SOC_ICELL_BLOCK(port), R_IECELL_CONFIG(SOC_PORT_BLOCK_INDEX(port)), &val32);        
        /* SOFT_RESETf BIT0 */
        if (enable) {
            val32 &= ~(1);    
        } else {
            val32 |= 1;
        }
        bcm5346x_reg_set(unit, SOC_ICELL_BLOCK(port), R_IECELL_CONFIG(SOC_PORT_BLOCK_INDEX(port)), val32);        
    }
    return SOC_E_NONE;
}

static sys_error_t
mac_xl_enable_set(uint8 unit, uint8 lport, BOOL enable)
{
    uint32 ctrl[2], octrl[2], val;

    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), ctrl, 2);

    octrl[0] = ctrl[0];

    /* Don't disable TX_EN[Bit 0] since it stops egress and hangs if CPU sends */
    ctrl[0] |= 0x1;
    if (enable) {
        /* RX_EN[Bit 1] */
        ctrl[0] |= 0x2;
    } else {
        ctrl[0] &= ~0x2;
    }

    if (ctrl[0] == octrl[0]) {
        if (enable || (!enable && (ctrl[0] & 0x40))) { 
            /* Don't do it again */
            return SYS_OK;
        }
    }

    if (enable) {
        /* Reset EP credit before de-assert SOFT_RESET */            
        soc_port_credit_reset(unit, lport);

        soc_mmu_flush_enable(unit, lport, FALSE);

        /* Enable both TX and RX, deassert SOFT_RESET [Bit 6] */
        ctrl[0] &= ~(0x1 << 6);
        bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), ctrl, 2);

        /* Deassert EGR_XLPORT_BUFFER_SFT_RESET */
        soc_port_egress_buffer_sft_reset(unit, lport, 0);

        /* Add port to EPC_LINK */
        bcm5346x_mem_get(unit, M_EPC_LINK_BMAP, &val, 1);
        val |= (1 << lport);
        bcm5346x_mem_set(unit, M_EPC_LINK_BMAP, &val, 1);

        /* Enable output threshold RX */
        SOC_IF_ERROR_RETURN
            (soc_port_thdo_rx_enable_set(unit, lport, 1)); 
    } else {
        /* Disable RX */
        bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), ctrl, 2);

        /* Remove port from EPC_LINK */
        bcm5346x_mem_get(unit, M_EPC_LINK_BMAP, &val, 1);
        val = val & ~(1 << lport);
        bcm5346x_mem_set(unit, M_EPC_LINK_BMAP, &val, 1);

        sal_usleep(1000);

        /* Drain cells */
        _mac_xl_drain_cells(unit, lport, 1, 0);

        /* Reset port FIFO */
        soc_port_fifo_reset(unit, lport);

        /* Put port into SOFT_RESET */
        ctrl[0] |= (0x1 << 6);

        bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), ctrl, 2);

        if(!IS_XL_PORT(lport)) {
            _xlmac_sb2_shim_war(unit, lport);
        }

        /* Disable output threshold RX */
        SOC_IF_ERROR_RETURN
            (soc_port_thdo_rx_enable_set(unit, lport, 0));               
    }

    return SYS_OK;
}

static sys_error_t
mac_xl_init(uint8 unit, uint8 lport)
{
    uint32 mac_ctrl[2], entry[2];
    uint32 val32;
    uint8 system_mac[6];

    if (IS_XL_PORT(lport)) {
        bcm5346x_reg_get(unit, R_TXLP_PORT_ENABLE, &val32); 

        /* LP_ENABLEf [BIT 3 ~ 0] = 0 */
        val32 &= ~(1 << (lport));
        bcm5346x_reg_set(unit, R_TXLP_PORT_ENABLE, val32);
    }

    /* Disable Tx/Rx, assume that MAC is stable (or out of reset) */
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), mac_ctrl, 2);

    /* Reset EP credit before de-assert SOFT_RESET Bit[6]*/
    if (mac_ctrl[0] & 0x40) {
        soc_port_credit_reset(unit, lport);
    }

    /* Enable XGMII_IPG_CHECK_DISABLEf for higig port */

    /* Disable XGMII_IPG_CHECK_DISABLEf[11])/SOFT_RESETf[6]/RX_ENf[1]/TX_ENf[0] */
    mac_ctrl[0] &= ~(0x843);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), mac_ctrl, 2);

    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_RX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* Set STRICT_PREAMBLE[Bit 3] for XE ports */
    /* STRIP_CRCf[Bit 2]=0 */
    entry[0] &= ~(0x1 << 2);
    if (SOC_PORT_SPEED_MAX(unit) >= 10000) { /* && IS_XE_PORT(unit, port)) {*/
        entry[0] |= (0x1 << 3);    
    } else {
        entry[0] &= ~(0x1 << 3);
    }
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_RX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_TX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* AVERAGE_IPGf[Bit 18:12] : XE port = 96/8, HIGIG pott = 64/8
     * CRC_MODEf[Bit 1:0]=2
     */
    entry[0] &= 0xfff80ffc;
    entry[0] |= ((12 & 0x7f) << 12) | 0x2;
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_TX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* Enable pause except for stacking ports */
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* TX_PAUSE_ENf[Bit 17][Bit=1/RX_PAUSE_ENf[Bit 18]=1 */
    entry[0] |= 0x00060000;
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_PFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* PFC_REFRESH_ENf[Bit 32]=1 */
    entry[1] |= 0x1;
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_PFC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* RX_MAX_SIZE BIT0 ~ BIT13 */
    entry[1] = 0;
    entry[0] = (JUMBO_MAXSZ) & ((1<<14)-1);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_RX_MAX_SIZE(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_MODE(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    entry[0] &= ~(0x7 << 4);
    /* 
     * SPEED_MODE Bit[6:4]: 4 = 10000, 3 = 2500, 2 = 1000, 1 = 100, 0 = 10 
     * Assign max_speed = 10000 
     */
    entry[0] &= ~(0x7 << 4);     
    switch (SOC_PORT_SPEED_MAX(unit)) {
        case 10:
           entry[0] |= (0x0 << 4);
           break;
        case 100:
           entry[0] |= (0x1 << 4);
           break;
        case 1000:
           entry[0] |= (0x2 << 4);
           break;
        case 2500:
           entry[0] |= (0x3 << 4);
           break;
        default:
        case 10000:
           entry[0] |= (0x4 << 4);
           break;                   	     	  	   
    }
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_MODE(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* Initialize mask for purging packet data received from the MAC 
       Diable length check */
    if (IS_XL_PORT(lport)) {
        bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), 
            R_XLPORT_MAC_RSV_MASK(SOC_PORT_BLOCK_INDEX(lport)), 0x58);
	} else {
        bcm5346x_reg_set(unit, SOC_PORT_BLOCK(lport), 
            R_MAC_RSV_MASK(SOC_PORT_BLOCK_INDEX(lport)), 0x58);
    }
    /* Enable DROP_TX_DATA_ON_LOCAL_FAULTf[Bit 4] and
     * DROP_TX_DATA_ON_REMOTE_FAULTf[Bit 5]
     */
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_RX_LSS_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    entry[0] |= 0x30;
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_RX_LSS_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    /* EXTENDED_HIG2_ENf[Bit 14] = 1 , SW_LINK_STATUSf[Bit 12] = 1 */
    bcm5346x_reg64_get(unit,  SOC_PORT_BLOCK(lport), 
        R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), mac_ctrl, 2);
    mac_ctrl[0] |= 0x9000;
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), mac_ctrl, 2);

    /* Disable loopback and bring XLMAC out of reset */
    /* LOCAL_LPBKf[Bit 2] = 0, RX_ENf[Bit 1] = 1, TX_ENf[Bit 0]=1 */
    bcm5346x_reg64_get(unit,  SOC_PORT_BLOCK(lport), 
        R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), mac_ctrl, 2);
    mac_ctrl[0] &= ~(0x4);
    mac_ctrl[0] |= 0x3;
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), mac_ctrl, 2);

    get_system_mac(system_mac);

    entry[0] = (system_mac[0] << 8) | system_mac[1];
    entry[1] = (system_mac[2] << 24) | (system_mac[3] << 16) | 
               (system_mac[4] << 8) | system_mac[5];
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport),
        R_XLMAC_TX_MAC_SA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    return SYS_OK;
}

static sys_error_t
mac_xl_duplex_set(uint8 unit, uint8 lport, BOOL duplex)
{
    return SYS_OK;
}

static sys_error_t
mac_xl_speed_set(uint8 unit, uint8 lport, int speed)
{
    uint32 mode;
    uint32 entry[2];

    switch (speed) {
        case 10:
            mode = SOC_XLMAC_SPEED_10;
            break;
        case 100:
            mode = SOC_XLMAC_SPEED_100;
            break;
        case 1000:
            mode = SOC_XLMAC_SPEED_1000;
            break;
        case 2500:
            mode = SOC_XLMAC_SPEED_2500;
            break;
        case 5000:
            mode = SOC_XLMAC_SPEED_10000;
            break;
        case 0:
            return SYS_OK;              /* Support NULL PHY */
        default:
            if (speed < 10000) {
                return SYS_ERR_PARAMETER;
            }
            mode = SOC_XLMAC_SPEED_10000;
            break;
    }

    /* 
     * Have to disable MAC before setting speed. Assume XLMAC is always in
     * disabled state when setting speed in unmanaged mode.
     */
    /* SPEED_MODE[Bit 4~ Bit 6] */
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_MODE(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    entry[0] &= ~(0x7 << 4);
    entry[0] |= (mode << 4);
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_MODE(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_RX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* Enable STRICT_PREAMBLEf[Bit 3] if speed >= 10000 */
    if (speed >= 10000) {
        /* && IS_XE_PORT(unit, port) */
        entry[0] |= (0x1 << 3);
    } else {
        entry[0] &= ~(0x1 << 3);
    }
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_RX_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_RX_LSS_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* Enable LOCAL_FAULT_DISABLEf[Bit 0] and REMOTE_FAULT_DISABLEf[Bit 1]
     * if speed < 5000 
     */
    if (speed < 5000) {
        entry[0] |= 0x3;
    } else {
        entry[0] &= ~0x3;
    }
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_RX_LSS_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

#ifdef CFG_TIMESTAMP_MAC_DELAY
    /* Set Timestamp Mac Delays */
    _mac_xl_timestamp_delay_set(unit, lport, speed);
#endif
    return SYS_OK;
}

static sys_error_t
mac_xl_pause_set(uint8 unit, uint8 lport, BOOL pause_tx, BOOL pause_rx)
{
    uint32 entry[2];

    /* Bit[18]: RX_PAUSE_EN, Bit[17]: TX_PAUSE_EN */
    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    if (pause_tx) {
        entry[0] |= (0x1 << 17);
    } else {
        entry[0] &= ~(0x1 << 17);
    }
    if (pause_rx) {
        entry[0] |= (0x1 << 18);
    } else {
        entry[0] &= ~(0x1 << 18);
    }
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_PAUSE_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    return SYS_OK;
}

/*
 * Function:
 *      mac_xl_loopback_set
 * Purpose:
 *      Set a XLMAC into/out-of loopback mode
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS unit # on unit.
 *      loopback - Boolean: true -> loopback mode, false -> normal operation
 * Note:
 *      On Xlmac, when setting loopback, we enable the TX/RX function also.
 *      Note that to test the PHY, we use the remote loopback facility.
 * Returns:
 *      SOC_E_XXX
 */
static sys_error_t
mac_xl_loopback_set(uint8 unit, uint8 lport, BOOL lb)
{
    uint32 entry[2];

#if 0
    /* need to enable clock compensation for applicable serdes device */
    (void)soc_phyctrl_notify(unit, port, phyEventMacLoopback, lb ? 1 : 0);
#endif

    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_RX_LSS_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* Set LOCAL_FAULT_DISABLEf[Bit 0] and REMOTE_FAULT_DISABLEf[Bit 1] */
    if (lb) {
        entry[0] |= 0x3;
    } else {
        entry[0] &= ~0x3;
    }
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_RX_LSS_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    bcm5346x_reg64_get(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    /* LOCAL_LPBK[Bit 2] = 0x1 */
    if (lb) {
        entry[0] |= 0x4;
    } else {
        entry[0] &= ~0x4;
    }
    bcm5346x_reg64_set(unit, SOC_PORT_BLOCK(lport), 
        R_XLMAC_CTRL(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);

    return SYS_OK;
}

mac_driver_t soc_mac_xl = {
    "XLMAC Driver",               /* drv_name */
    mac_xl_init,                  /* md_init  */
    mac_xl_enable_set,            /* md_enable_set */
    mac_xl_duplex_set,            /* md_duplex_set */
    mac_xl_speed_set,             /* md_speed_set */
    mac_xl_pause_set,             /* md_pause_set */
    mac_xl_loopback_set,          /* md_lb_set */
#if 0
    mac_xl_loopback_get,          /* md_lb_get */
    mac_xl_frame_max_set          /* md_frame_max_set */
#endif
};

