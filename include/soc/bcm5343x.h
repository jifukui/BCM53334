/*
 * $Id: bcm5343x.h,v 1.32 Broadcom SDK $
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

#ifndef _BCM5343X_H_
#define _BCM5343X_H_

#include "auto_generated/bcm5343x_defs.h"

#ifndef __LINUX__
#define READCSR(x)    \
  (*(volatile uint32 *)(x))
#define WRITECSR(x,v) \
  (*(volatile uint32 *)(x) = (uint32)(v))
#endif

#define BCM56162_DEVICE_ID      0xb162
#define BCM53434_DEVICE_ID      0x8434


/* Bit positions used for set/clear commands. */
#define S_SCTL_MSG_START         0
#define S_SCTL_MSG_DONE          1

#define M_SCTL_MSG_START          _DD_MAKEMASK1(S_SCTL_MSG_START)
#define M_SCTL_MSG_DONE           _DD_MAKEMASK1(S_SCTL_MSG_DONE)

#define S_SCTL_MIIM_OP_DONE       0

#define S_SCTL_MIIM_SCAN_BUSY     14
#define S_SCTL_MIIM_RD_START      16
#define S_SCTL_MIIM_WR_START      17

#define S_SCTL_MIIM_LINK_SCAN_EN  19
#define S_SCTL_MIIM_PAUS_ESCAN_EN 20

#define M_SCTL_BITVAL             _DD_MAKEMASK1(7)
#define V_SCTL_BIT_0              0
#define V_SCTL_BIT_1              M_SCTL_BITVAL

#define M_SCTL_MIIM_SCAN_BUSY     _DD_MAKEMASK1(S_SCTL_MIIM_SCAN_BUSY)
#define M_SCTL_MIIM_RD_START      _DD_MAKEMASK1(S_SCTL_MIIM_RD_START)
#define M_SCTL_MIIM_WR_START      _DD_MAKEMASK1(S_SCTL_MIIM_WR_START)
#define M_SCTL_MIIM_OP_DONE       _DD_MAKEMASK1(S_SCTL_MIIM_OP_DONE)
#define M_SCTL_MIIM_LINK_SCAN_EN  _DD_MAKEMASK1(S_SCTL_MIIM_LINK_SCAN_EN)
#define M_SCTL_MIIM_PAUSE_SCAN_EN _DD_MAKEMASK1(S_SCTL_MIIM_PAUS_ESCAN_EN)

/* MIIMP: MIIM Parameter Register (0x0158) */
#define S_MIIMP_PHY_DATA          0
#define M_MIIMP_PHY_DATA          _DD_MAKEMASK(16,S_MIIMP_PHY_DATA)
#define V_MIIMP_PHY_DATA(x)       _DD_MAKEVALUE(x,S_MIIMP_PHY_DATA)
#define G_MIIMP_PHY_DATA(x)       _DD_GETVALUE(x,S_MIIMP_PHY_DATA,M_MIIMP_PHY_DATA)

#define S_MIIMP_PHY_ID            16
#define M_MIIMP_PHY_ID            _DD_MAKEMASK(5,S_MIIMP_PHY_ID)
#define V_MIIMP_PHY_ID(x)         _DD_MAKEVALUE(x,S_MIIMP_PHY_ID)
#define G_MIIMP_PHY_ID(x)         _DD_GETVALUE(x,S_MIIMP_PHY_ID,M_MIIMP_PHY_ID)

/* MIIMRD: MIIM Read Data Register (0x015C) */
#define S_MIIMRD_DATA             0
#define M_MIIMRD_DATA             _DD_MAKEMASK(16,S_MIIMRD_DATA)
#define V_MIIMRD_DATA(x)          _DD_MAKEVALUE(x,S_MIIMRD_DATA)
#define G_MIIMRD_DATA(x)          _DD_GETVALUE(x,S_MIIMP_PHY_DATA,M_MIIMP_PHY_DATA)



#define S_SCTL_BIT_POS           0
#define V_SCTL_BIT_POS(x)        _DD_MAKEVALUE(x,S_SCTL_BIT_POS)

/* CMIC_CMCx_SCHAN_CTRL(x) */
#define SC_CMCx_MSG_START               (0x00000001)
#define SC_CMCx_MSG_DONE                (0x00000002)
#define SC_CMCx_SCHAN_ABORT             (0x00000004)
#define SC_CMCx_MSG_SER_CHECK_FAIL      (0x00100000)
#define SC_CMCx_MSG_NAK                 (0x00200000)
#define SC_CMCx_MSG_TIMEOUT_TST         (0x00400000)
#define SC_CMCx_MSG_SCHAN_ERROR         (0x00800000)
#define SC_CMCx_MSG_ERROR_MASK          (0x00F00000)

/*  CMIC_CMCx_MIIM_STAT(x) */
#define CMIC_MIIM_OPN_DONE              (0x00000001)

/*  CMIC_CMCx_MIIM_CTRL(x) */
#define CMIC_MIIM_WR_START              (0x00000001)
#define CMIC_MIIM_RD_START              (0x00000002)

#define SC_OP_RD_MEM_CMD         0x07
#define SC_OP_RD_MEM_ACK         0x08
#define SC_OP_WR_MEM_CMD         0x09
#define SC_OP_RD_REG_CMD         0x0B
#define SC_OP_RD_REG_ACK         0x0C
#define SC_OP_WR_REG_CMD         0x0D
#define SC_OP_L2_INS_CMD         0x0F
#define SC_OP_L2_DEL_CMD         0x11
                              

/* FP_TCAM index */
#define MDNS_TO_CPU_IDX                (0)
#define SYS_MAC_IDX                    (1 * ENTRIES_PER_SLICE)
#define RATE_IGR_IDX                   ((1 * ENTRIES_PER_SLICE) + 3)
#define QOS_BASE_IDX                   (2 * ENTRIES_PER_SLICE)
/* 802.1p higher than DSCP fp entries index */
#define DOT1P_BASE_IDX                 ((2 * ENTRIES_PER_SLICE) + 48)

/* MDNS Redirection table index */
#define MDNS_REDIR_IDX                 (2)
/* Loop Detect, per port from LOOP_COUNT_IDX + MIN~ */
#define LOOP_COUNT_IDX                 ( 3 * ENTRIES_PER_SLICE)
#define LOOP_REDIRECT_IDX              ( 3 * ENTRIES_PER_SLICE + 35)

#define L2_ENTRY_SIZE              16384

#define M_IRQ_CHAIN_DONE(ch)     (1 << (ch))
#define M_IRQ_DMA_ACTIVE(ch)     (0x100 << (ch))

/* CMIC_CMCx_CHy_DMA_CTRL */
#define PKTDMA_DIRECTION                (0x00000001)
#define PKTDMA_ENABLE                   (0x00000002)
#define PKTDMA_ABORT                    (0x00000004)
#define PKTDMA_SEL_INTR_ON_DESC_OR_PKT  (0x00000008)
#define PKTDMA_BIG_ENDIAN               (0x00000010)
#define PKTDMA_DESC_BIG_ENDIAN          (0x00000020)
#define RLD_STATUS_UPD_DIS              (0x00000080)

/* CMIC_CMCx_DMA_STAT(x) */
#define DS_CMCx_DMA_CHAIN_DONE(y)       (0x00000001 << (y))
#define DS_CMCx_DMA_DESC_DONE(y)        (0x00000010 << (y))
#define DS_CMCx_DMA_ACTIVE(y)           (0x00000100 << (y))

/* CMIC_CMCx_DMA_STAT_CLR(x) */
#define DS_DESCRD_CMPLT_CLR(y)          (0x00000001 << (y))
#define DS_INTR_COALESCING_CLR(y)       (0x00000010 << (y))

#define TX_CH                    0
#define RX_CH1                   1





#define MMU_R_HOLCOSPKTSETLIMIT     0x5D8
#define MMU_R_HOLCOSPKTRESETLIMIT   0x5D4
#define MMU_R_LWMCOSCELLSETLIMIT    0x40010   /* CELLRESETLIMIT = 16, CELLSETLIMIT = 16 */
#define MMU_R_HOLCOSCELLMAXLIMIT_COS0   0x1F607E8 /* COS0: CELLMAXLIMIT = 2024 (0x7E8), RESUMELIMIT = 2008 (0x7D8)*/
#define MMU_R_HOLCOSCELLMAXLIMIT_COS1   0xC0040   /* COS1~3: CELLMAXLIMIT = 64 (0x40), RESUMELIMIT = 48 (0x30) */
#define MMU_R_HOLCOSMINXQCNT        0x8
#define MMU_R_PGCELLLIMIT           0x78028    /* CELLSETLIMIT= 40 (0x28),CELLRESETLIMIT= 30(0x1E) */ 
#define MMU_R_PGDISCARDSETLIMIT     0x0FFF
#define MMU_R_DYNCELLLIMIT          0x22208a8 /* DYNCELLSETLIMIT = 2216 (0x8A8), DYNCELLRESETLIMISEL = 2184 (0x888) */
#define MMU_R_DYNXQCNTPORT          0x5D0
#define MMU_R_DYNRESETLIMPORT       0x5C0
#define MMU_R_IBPPKTSETLIMIT        0xC       /* PKTSETLIMIT = 12(0xC), RESETLIMITSEL = 0(75%) */

#define MMU_R_TOTALDYNCELLSETLIMIT      0x9C0
#define MMU_R_TOTALDYNCELLRESETLIMIT    0x9A0

#define PORT_STATUS_FLAGS_GIGA_SPEED    (1 << 0)
#define PORT_STATUS_FLAGS_GREEN_MODE    (1 << 1)
#define PORT_STATUS_FLAGS_TX_PAUSE      (1 << 2)
#define PORT_STATUS_FLAGS_RX_PAUSE      (1 << 3)
#define PORT_STATUS_FLAGS_REMOTE_EEE    (1 << 4)

/* EEE parameters */
#define BCM5343X_PHY_EEE_100MB_MIN_WAKE_TIME    36  /* 36 micro seconds */
#define BCM5343X_PHY_EEE_1GB_MIN_WAKE_TIME      17  /* 17 micro seconds */


#ifdef CFG_SOC_SEMAPHORE_INCLUDED
#define SCHAN_LOCK(unit) \
        do { if (!READCSR(CMIC_SEMAPHORE_3_SHADOW))\
                 while (!READCSR(CMIC_SEMAPHORE_1)); } while(0)
#define SCHAN_UNLOCK(unit) \
        do { if (!READCSR(CMIC_SEMAPHORE_3_SHADOW))\
               WRITECSR(CMIC_SEMAPHORE_1, 0); } while(0)
#define MIIM_LOCK(unit) \
        do { while (!READCSR(CMIC_SEMAPHORE_2)); } while(0)
#define MIIM_UNLOCK(unit) \
        do { WRITECSR(CMIC_SEMAPHORE_2, 0); } while(0)
/* Access serdes registers through s-channel */
#define MIIM_SCHAN_LOCK(unit) \
        do { while (!READCSR(CMIC_SEMAPHORE_3)); } while(0)
#define MIIM_SCHAN_UNLOCK(unit) \
        do { WRITECSR(CMIC_SEMAPHORE_3, 0); } while(0)
#else
#define SCHAN_LOCK(unit)
#define SCHAN_UNLOCK(unit)
#define MIIM_LOCK(unit)
#define MIIM_UNLOCK(unit)
#define MIIM_SCHAN_LOCK(unit)
#define MIIM_SCHAN_UNLOCK(unit)
#endif /* CFG_SOC_SEMAPHORE_INCLUDED */

#define LED_PORT_STATUS_OFFSET(p)   CMIC_LEDUP_DATA_RAM_D(0xa0 + (p))

/*
* For VT_DOT1Q, VLAN_DEFAULT controls the default VID and can be any value from 1~4094. 
* For VT_PORT_BASED, it keeps VID==1 as default.
*/
#define VLAN_DEFAULT                 1

/* EEE functions */
#ifdef CFG_SWITCH_EEE_INCLUDED
extern void bcm5343x_port_eee_enable_get(uint8 unit, uint8 port, uint8 *enable);
extern sys_error_t bcm5343x_port_eee_enable_set(uint8 unit, 
                      uint8 port, uint8 enable, uint8 save);
extern sys_error_t bcm5343x_port_eee_tx_wake_time_set(uint8 unit, 
                      uint8 port, uint8 type, uint16 wake_time);
extern void bcm5343x_eee_init(void);
#endif /* CFG_SWITCH_EEE_INCLUDED */
/* End of EEE */

/* phy bcm542xx functions */
extern int bcm542xx_phy_cl45_reg_read(uint8 unit, uint8 port, 
                      uint8 dev_addr, uint16 reg_addr, uint16 *p_value);
extern int bcm542xx_phy_cl45_reg_write(uint8 unit, uint8 port, 
                      uint8 dev_addr, uint16 reg_addr, uint16 p_value);
extern int bcm542xx_phy_cl45_reg_modify(uint8 unit, uint8 port, 
                      uint8 dev_addr, uint16 reg_addr, uint16 val, uint16 mask);
extern int bcm542xx_phy_direct_reg_write(uint8 unit, uint8 port, 
                      uint16 reg_addr, uint16 data);
extern int bcm542xx_phy_reg_modify(uint8 unit, uint8 port, uint16 reg_bank,
                            uint8 reg_addr, uint16 data, uint16 mask);
/* End of phy bcm542xx functions */

typedef struct l2x_entry_s {
    uint16 vlan_id;
    uint8  mac_addr[6];
    /* Port or multicast index */
    uint8  port;
} l2x_entry_t;




/* physical port */
#define BCM5343X_PORT_MIN                        10
#define BCM5343X_PORT_MAX                        37

/* logical port */
#define BCM5343X_LPORT_MIN                        2
#define BCM5343X_LPORT_MAX                       29

typedef enum port_block_type_s {
    PORT_BLOCK_TYPE_XLPORT,
    PORT_BLOCK_TYPE_GXPORT,
    PORT_BLOCK_TYPE_COUNT
} port_block_type_t;

enum soc_hr3_port_mode_e {
    /* WARNING: values given match hardware register; do not modify */
    SOC_HR3_PORT_MODE_QUAD = 0,
    SOC_HR3_PORT_MODE_TRI_012 = 1,
    SOC_HR3_PORT_MODE_TRI_023 = 2,
    SOC_HR3_PORT_MODE_DUAL = 3,
    SOC_HR3_PORT_MODE_SINGLE = 4
};

typedef enum qtc_interface_s {
    QTC_INTERFACE_QSGMII = 1,
    QTC_INTERFACE_SGMII = 2,
    QTC_INTERFACE_FIBER = 3,
} qtc_interface_t;

/* SOC interface */
extern sys_error_t bcm5343x_chip_revision(uint8 unit, uint16 *dev, uint16 *rev);
extern uint8 bcm5343x_port_count(uint8 unit);
extern sys_error_t bcm5343x_reg_get(uint8 unit, uint8 block_id, uint32 addr, uint32 *val);
extern sys_error_t bcm5343x_reg_set(uint8 unit, uint8 block_id, uint32 addr, uint32 val);
extern sys_error_t bcm5343x_reg64_get(uint8 unit, uint8 block_id, uint32 addr, uint32 *val, int len);
extern sys_error_t bcm5343x_reg64_set(uint8 unit, uint8 block_id, uint32 addr, uint32 *buf, int len);
extern sys_error_t bcm5343x_mem_get(uint8 unit, uint8 block_id, uint32 addr, uint32 *buf, int len);
extern sys_error_t bcm5343x_mem_set(uint8 unit, uint8 block_id, uint32 addr, uint32 *buf, int len);
extern sys_error_t bcm5343x_l2_op(uint8 unit, l2x_entry_t *entry, uint8 op);
extern sys_error_t bcm5343x_rx_set_handler(uint8 unit, SOC_RX_HANDLER fn, 
                                           BOOL intr);
extern sys_error_t bcm5343x_rx_fill_buffer(uint8 unit, 
                                           soc_rx_packet_t *pkt);
extern sys_error_t bcm5343x_tx(uint8 unit, soc_tx_packet_t *pkt);
extern sys_error_t bcm5343x_link_status(uint8 unit, uint8 port, BOOL *link);
extern void bcm5343x_rxtx_stop(void);

/* FP_TCAM encode/decode utility */
extern void bcm5343x_dm_to_xy(uint32 *dm_entry, uint32 *xy_entry, int data_words, int bit_length);
extern void bcm5343x_xy_to_dm(uint32 *xy_entry, uint32 *dm_entry, int data_words, int bit_length);

#ifdef CFG_SWITCH_VLAN_INCLUDED
extern sys_error_t bcm5343x_pvlan_egress_set(uint8 unit, 
                    uint8 pport, 
                    pbmp_t pbmp);
extern sys_error_t bcm5343x_pvlan_egress_get(uint8 unit, 
                    uint8 pport, 
                    pbmp_t *pbmp);
extern sys_error_t bcm5343x_qvlan_port_set(uint8 unit, 
                    uint16 vlan_id, 
                    pbmp_t pbmp, 
                    pbmp_t tag_pbmp);
extern sys_error_t bcm5343x_qvlan_port_get(uint8 unit, 
                    uint16 vlan_id, 
                    pbmp_t *pbmp, 
                    pbmp_t *tag_pbmp);
extern sys_error_t bcm5343x_vlan_create(uint8 unit, 
                    vlan_type_t type, uint16 vlan_id);
extern sys_error_t bcm5343x_vlan_destroy(uint8 unit, uint16 vlan_id);
extern sys_error_t bcm5343x_vlan_type_set(uint8 unit, vlan_type_t type);
extern sys_error_t bcm5343x_vlan_reset(uint8 unit);
#endif /* CFG_SWITCH_VLAN_INCLUDED */

#ifdef CFG_SWITCH_LAG_INCLUDED
extern sys_error_t bcm5343x_lag_group_set(uint8 unit, uint8 lagid, pbmp_t pbmp);
extern void bcm5343x_lag_group_get(uint8 unit, uint8 lagid, pbmp_t *pbmp);
#endif /* CFG_SWITCH_LAG_INCLUDED */

extern void bcm5343x_loop_detect_enable(BOOL enable);
extern uint8 bcm5343x_loop_detect_status_get(void);

#ifdef CFG_ZEROCONF_MDNS_INCLUDED
sys_error_t bcm5343x_mdns_enable_set(uint8 unit, BOOL mdns_enable);
sys_error_t bcm5343x_mdns_enable_get(uint8 unit, BOOL *mdns_enable);
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

/* Initialization */
extern sys_error_t bcm5343x_sw_init(void);
extern void bcm5343x_dscp_map_enable(BOOL enable);
extern void bcm5343x_qos_init(void);
extern void bcm5343x_rate_init(void);
extern void bcm5343x_loop_detect_init(void);
extern void bcm5343x_rxtx_init(void);
extern void bcm5343x_loopback_enable(uint8 unit, uint8 port, int loopback_mode);
extern sys_error_t bcm5343x_mdns_enable_set(uint8 unit, BOOL enable);
extern sys_error_t bcm5343x_mdns_enable_get(uint8 unit, BOOL *enable);


typedef struct mac_driver_s {
    char         *drv_name;
    sys_error_t  (*md_init)(uint8, uint8);
    sys_error_t  (*md_enable_set)(uint8, uint8, BOOL);
    sys_error_t  (*md_duplex_set)(uint8, uint8, BOOL);
    sys_error_t  (*md_speed_set)(uint8, uint8, int);
    sys_error_t  (*md_pause_set)(uint8, uint8, BOOL, BOOL);
    sys_error_t  (*md_lb_set)(uint8, uint8, BOOL);
#if 0
    sys_error_t  (*md_frame_max_set)(uint8, uint8, int);
    sys_error_t  (*md_ifg_set)(uint8, uint8, BOOL, int);
#endif
} mac_driver_t;

#define _MAC_CALL(_md, _mf, _ma) \
        ((_md) == 0 ? SYS_ERR_PARAMETER : \
         ((_md)->_mf == 0 ? SYS_ERR_NOT_IMPLEMENTED : (_md)->_mf _ma))

#define MAC_INIT(_md, _u, _p) \
        _MAC_CALL((_md), md_init, ((_u), (_p)))

#define MAC_ENABLE_SET(_md, _u, _p, _e) \
        _MAC_CALL((_md), md_enable_set, ((_u), (_p), (_e)))

#define MAC_DUPLEX_SET(_md, _u, _p, _d) \
        _MAC_CALL((_md), md_duplex_set, ((_u), (_p), (_d)))

#define MAC_SPEED_SET(_md, _u, _p, _s) \
        _MAC_CALL((_md), md_speed_set, ((_u), (_p), (_s)))

#define MAC_PAUSE_SET(_md, _u, _p, _tx, _rx) \
        _MAC_CALL((_md), md_pause_set, ((_u), (_p), (_tx), (_rx)))

#define MAC_LOOPBACK_SET(_md, _u, _p, _l) \
        _MAC_CALL((_md), md_lb_set, ((_u), (_p), (_l)))

extern mac_driver_t soc_mac_xl, soc_mac_uni;

typedef struct
{
    uint16  devid;
    uint16  revid;
    uint8   link[BCM5343X_LPORT_MAX + 1];           /* link status */
    uint8   loopback[BCM5343X_LPORT_MAX + 1];       /* loopback mode */
    uint8   port_count;
    uint32  pbmp;
#if defined(CFG_SWITCH_EEE_INCLUDED)
    tick_t link_up_time[BCM5343X_LPORT_MAX+1];
    uint8 need_process_for_eee_1s[BCM5343X_LPORT_MAX+1];
#endif /*  CFG_SWITCH_EEE_INCLUDED */
    int    port_p2l_mapping[BCM5343X_PORT_MAX + 1];   /* phy to logic */
    int    port_l2p_mapping[BCM5343X_LPORT_MAX + 1];  /* logic to phy */
    int    port_block_id[BCM5343X_LPORT_MAX + 1];     /* logical port */
    int    port_block_port_id[BCM5343X_LPORT_MAX + 1];/* logical port */
    int    port_block_type[BCM5343X_LPORT_MAX + 1];   /* logical port */
    int    port_speed_max[BCM5343X_LPORT_MAX + 1];    /* logical port */
    int    lane_number[BCM5343X_LPORT_MAX + 1];    /* logical port */
    int    port_admin_status[BCM5343X_LPORT_MAX + 1];   /* logical port */
    int    port_speed_satus[BCM5343X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_link_status[BCM5343X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_duplex_status[BCM5343X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_an_status[BCM5343X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_tx_pause_status[BCM5343X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_rx_pause_status[BCM5343X_LPORT_MAX + 1];   /* logical port */
    mac_driver_t *p_mac[BCM5343X_LPORT_MAX + 1];      /* mac driver */
    uint32 cpu_clock;
} bcm5343x_sw_info_t;

extern bcm5343x_sw_info_t hr3_sw_info;

#define COS_QUEUE_NUM                  (4)

/* Mask of all logical ports */
#define BCM5343X_ALL_PORTS_MASK     (hr3_sw_info.pbmp)

/*
 * Macros to get the device block type, block index and index
 * within block for a given port
 */
#define SOC_PORT_SPEED_MAX(port)   (hr3_sw_info.port_speed_max[(port)])
#define SOC_PORT_LANE_NUMBER(port)   (hr3_sw_info.lane_number[(port)])
#define SOC_PORT_ADMIN_STATUS(port)   (hr3_sw_info.port_admin_status[(port)])
#define SOC_PORT_SPEED_STATUS(port)   (hr3_sw_info.port_speed_satus[(port)])
#define SOC_PORT_LINK_STATUS(port)   (hr3_sw_info.port_link_status[(port)])
#define SOC_PORT_DUPLEX_STATUS(port)   (hr3_sw_info.port_duplex_status[(port)])
#define SOC_PORT_AN_STATUS(port)   (hr3_sw_info.port_an_status[(port)])
#define SOC_PORT_TX_PAUSE_STATUS(port)   (hr3_sw_info.port_tx_pause_status[(port)])
#define SOC_PORT_RX_PAUSE_STATUS(port)   (hr3_sw_info.port_rx_pause_status[(port)])
#define SOC_PORT_BLOCK_TYPE(port)   (hr3_sw_info.port_block_type[(port)])
#define SOC_PORT_BLOCK(port)        (hr3_sw_info.port_block_id[(port)])
#define SOC_PORT_BLOCK_INDEX(port)  (hr3_sw_info.port_block_port_id[(port)])

#define SOC_PORT_P2L_MAPPING(port)  (hr3_sw_info.port_p2l_mapping[(port)])
#define SOC_PORT_L2P_MAPPING(port)  (hr3_sw_info.port_l2p_mapping[(port)])
#define SOC_PORT_COUNT(unit)        (hr3_sw_info.port_count)

#define IS_XL_PORT(port) \
        (SOC_PORT_BLOCK_TYPE(port) == PORT_BLOCK_TYPE_XLPORT)

#define IS_GX_PORT(port) \
        (SOC_PORT_BLOCK_TYPE(port) == PORT_BLOCK_TYPE_GXPORT)

#define XLPORT_SUBPORT(port)        SOC_PORT_BLOCK_INDEX(port)

/* Physical port iteration */
#define SOC_PPORT_ITER(_p)       \
        for ((_p) = BCM5343X_PORT_MIN; \
             (_p) <= BCM5343X_PORT_MAX; \
             (_p)++) \
                if (hr3_sw_info.port_p2l_mapping[(_p)] != -1)

/* Logical port iteration */
#define SOC_LPORT_ITER(_p)       \
        for ((_p) = BCM5343X_LPORT_MIN; \
             (_p) <= BCM5343X_LPORT_MAX; \
             (_p)++) \
                if ((hr3_sw_info.port_l2p_mapping[(_p)] != -1))


#endif /* _BCM5343X_H_ */
