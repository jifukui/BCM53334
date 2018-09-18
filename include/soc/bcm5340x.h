/*
 * $Id: bcm5340x.h,v 1.53 Broadcom SDK $
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

#ifndef _BCM5340X_H_
#define _BCM5340X_H_

#ifndef __LINUX__
#define READCSR(x)    \
  (*(volatile uint32 *)(x))
#define WRITECSR(x,v) \
  (*(volatile uint32 *)(x) = (uint32)(v))
#endif
#define BCM53401_DEVICE_ID      0x8401
#define BCM53402_DEVICE_ID      0x8402
#define BCM53405_DEVICE_ID      0x8405
#define BCM53406_DEVICE_ID      0x8406
#define BCM53408_DEVICE_ID      0x8408

#define BCM53411_DEVICE_ID      0x8411
#define BCM53412_DEVICE_ID      0x8412
#define BCM53415_DEVICE_ID      0x8415
#define BCM53416_DEVICE_ID      0x8416
#define BCM53418_DEVICE_ID      0x8418

#define BCM53454_DEVICE_ID      0x8454
#define BCM53455_DEVICE_ID      0x8455
#define BCM53456_DEVICE_ID      0x8456
#define BCM53457_DEVICE_ID      0x8457

/* Bloodhound */
#define BCM53422_DEVICE_ID      0x8422
#define BCM53424_DEVICE_ID      0x8424
#define BCM53426_DEVICE_ID      0x8426

#define BCM56050_DEVICE_ID      0xb050

#define _DD_MAKEMASK1(n) (1 << (n))
#define _DD_MAKEMASK(v,n) ((((1)<<(v))-1) << (n))
#define _DD_MAKEVALUE(v,n) ((v) << (n))
#define _DD_GETVALUE(v,n,m) (((v) & (m)) >> (n))

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

#define R_CMICD_M0_IDM_IO_CONTROL_DIRECT           0x18103408
#define R_CMICD_S0_IDM_IO_CONTROL_DIRECT           0x1810A408
#define R_IHOST_SCU_CONTROL                        0x19020000

#define R_CHIPCOMMONG_MII_MANAGEMENT_CONTROL       0x18002000
#define R_CHIPCOMMONG_MII_MANAGEMENT_COMMAND_DATA  0x18002004
#define R_AVS_REG_HW_MNTR_SEQUENCER_MASK_PVT_MNTR  17,0x02003c00
#define R_TOP_PVTMON_CTRL_0                        16,0x02050000
#define R_TOP_STRAP_STATUS                         16,0x0207D400
#define R_TOP_STRAP_STATUS_1                       16,0x0207D800

#define S_SMHDR_DATA_LEN         7
#define M_SMHDR_DATA_LEN         _DD_MAKEMASK(7,S_SMHDR_DATA_LEN)
#define V_SMHDR_DATA_LEN(x)      _DD_MAKEVALUE(x,S_SMHDR_DATA_LEN)
#define G_SMHDR_DATA_LEN(x)      _DD_GETVALUE(x,S_SMHDR_DATA_LEN,M_SMHDR_DATA_LEN)

/* SBUS v4 : block id [25:19] */
#define S_SMHDR_DEST_BLOCK       19
#define M_SMHDR_DEST_BLOCK       _DD_MAKEMASK(6,S_SMHDR_DEST_BLOCK)
#define V_SMHDR_DEST_BLOCK(x)    _DD_MAKEVALUE(x,S_SMHDR_DEST_BLOCK)
#define G_SMHDR_DEST_BLOCK(x)    _DD_GETVALUE(x,S_SMHDR_DEST_BLOCK,M_SMHDR_DEST_BLOCK)

#define S_SMHDR_OP_CODE          26
#define M_SMHDR_OP_CODE          _DD_MAKEMASK(6,S_SMHDR_OP_CODE)
#define V_SMHDR_OP_CODE(x)       _DD_MAKEVALUE(x,S_SMHDR_OP_CODE)
#define G_SMHDR_OP_CODE(x)       _DD_GETVALUE(x,S_SMHDR_OP_CODE,M_SMHDR_OP_CODE)

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

#define R_CMIC_SCHAN_D(word)     (CMIC_CMC1_SCHAN_MESSAGE0+4*(word))
                                  
#define M_PORT(idx)                 10,0x04000000+(idx)
#define M_VLAN(idx)                 10,0x10000000+(idx)
#define M_VLAN_PROFILE(idx)         10,0x10080000+(idx)
#define M_EGR_MASK(idx)             10,0x40000000+(idx)

#define M_EGR_VLAN(idx)             11,0x10100000+(idx)
                                    
#define M_VLAN_STG(idx)             10,0x10040000+(idx)
#define M_EGR_VLAN_STG(idx)         11,0x10140000+(idx)

#define R_EGR_VLAN_CONTROL_1(idx)    11,0x04000B00+(idx)
#define R_EGR_CONFIG                 11,0x06000400
#define M_EGR_PORT_REQUESTS(idx)     11,0x30040000+(idx)
#define M_EGR_PORT_CREDIT_RESET(idx) 11,0x300C0000+(idx)
#define M_EGR_ENABLE(idx)            11,0x30740000+(idx)
#define R_AUX_ARB_CONTROL            10,0x02000000
#define R_AUX_ARB_CONTROL_2          10,0x02000100
#define M_SYSTEM_CONFIG_TABLE(idx)   10,0x04080000+(idx) /* Stage[31:26] = 1 */
#define R_ING_CONFIG_64              10,0x06000000
#define R_DOS_CONTROL                10,0x06000100
#define R_DOS_CONTROL2               10,0x06000200
#define R_VLAN_CTRL                  10,0x06000300
#define M_COS_MAP(idx)               10,0x40480000+(idx)

/* CAMs */
#define R_VLAN_SUBNET_CAM_DBGCTRL     10,0x0e010200
#define R_VFP_CAM_CONTROL_TM_7_THRU_0 10,0x0e011d00
#define R_L2_USER_ENTRY_CAM_DBGCTRL   10,0x1E010000
#define R_SW2_RAM_CONTROL_4_64        10,0x46010700

/*
* For VT_DOT1Q, VLAN_DEFAULT controls the default VID and can be any value from 1~4094. 
* For VT_PORT_BASED, it keeps VID==1 as default.
*/
#define VLAN_DEFAULT                1
#define M_ING_VLAN_TAG_ACTION_PROFILE 10,0x0C2C0000
#define M_ING_PRI_CNG_MAP(idx)      10,0x341C0000+(idx)
#define R_VLAN_DEFAULT_PBM          10,0x12000000
#define R_L2_AUX_HASH_CONTROL       10,0x1E000100
#define R_L3_AUX_HASH_CONTROL       10,0x2A000000
#define R_SW2_FP_DST_ACTION_CONTROL 10,0x42003400
#define M_DSCP_TB(idx)              10,0x24000000+(idx)

#define R_EGR_PORT_64(idx)          11,0x04000900+(idx)
#define M_L2_USER_ENTRY(idx)        10,0x1c100000+(idx)

/* Trunk related */
#define M_TRUNK_GROUP(idx)          10,0x40080000+(idx)
#define M_TRUNK_BITMAP(idx)         10,0x400C0000+(idx)
#define M_NONUCAST_TRUNK_BLOCK_MASK(idx) 10,0x40240000+(idx)
#define M_TRUNK32_CONFIG_TABLE(idx) 10,0x040c0000+(idx)
#define M_SOURCE_TRUNK_MAP(idx)     10,0x08000000+(idx)
#define M_TRUNK32_PORT_TABLE(idx)   10,0x08180000+(idx)

#define R_HASH_CONTROL              10,0x06000400

/* L2 age */
#define R_L2_AGE_TIMER               10,0x02000400

/* FP related */
#define R_FP_SLICE_ENABLE             10,0x3A000000
#define M_IFP_REDIRECTION_PROFILE(idx) 10,0x3C040000+(idx)
#define M_FP_PORT_FIELD_SEL(idx)       10,0x34000000+(idx)
#define M_FP_SLICE_ENTRY_PORT_SEL(idx) X,0x0d710000+(idx)
#define M_FP_TCAM(idx)                 10,0x38040000+(idx)
#define M_FP_SLICE_MAP                 10,0x38080000
#define M_FP_POLICY_TABLE(idx)         10,0x380C0000+(idx)
#define M_FP_COUNTER_TABLE(idx)        10,0x38140000+(idx)
#define M_FP_GLOBAL_MASK_TCAM(idx)     10,0x38180000+(idx)
#define M_FP_UDF_OFFSET(idx)           10,0x08040000+(idx)
/* FP_TCAM index */
#define RATE_IGR_IDX                   ((1 * ENTRIES_PER_SLICE) + 3)
/* 802.1p higher than DSCP fp entries index */
#define DOT1P_BASE_IDX                 ((2 * ENTRIES_PER_SLICE) + 48)
#define QOS_BASE_IDX                   (2 * ENTRIES_PER_SLICE)
#define MDNS_TO_CPU_IDX                (0)
/* MDNS Redirection table index */
#define MDNS_REDIR_IDX                 (2)
/* Loop Detect, per port from LOOP_COUNT_IDX + MIN~ */
#define LOOP_COUNT_IDX                 ( 3 * ENTRIES_PER_SLICE)
#define LOOP_REDIRECT_IDX              ( 3 * ENTRIES_PER_SLICE + 35)

/* Mirror related */
#define R_MIRROR_CONTROL(idx)          10,0x40000600+(idx)
#define R_EMIRROR_CONTROL_64(idx)      10,0x40000800+(idx)
#define M_IM_MTP_INDEX(idx)            10,0x402c0000+(idx)
#define M_EM_MTP_INDEX(idx)            10,0x40300000+(idx)

#define M_EGR_IM_MTP_INDEX(idx)        11,0x14240000+(idx)
#define M_EGR_EM_MTP_INDEX(idx)        11,0x14280000+(idx)


/* rate related */
#define R_EGRMETERINGCONFIG(idx)           12,0x00015600+(idx)
#define R_EGRMETERINGBUCKET(idx)           12,0x00015700+(idx)

#define R_STORM_CONTROL_METER_CONFIG(idx)  10,0x34000200+(idx)
#define M_FP_METER_TABLE(idx)              10,0x38100000+(idx)
#define M_FP_STORM_CONTROL_METERS(idx)     10,0x381c0000+(idx)

#define M_FP_SC_BCAST_METER_TABLE(idx)     X,0x0D780000+(idx)
#define M_FP_SC_MCAST_METER_TABLE          X,0x0D780019
#define M_FP_SC_DLF_METER_TABLE            X,0x0D780032

/* counter related */
/* GPORT counters */

/* Unicast Frame Counter */
#define R_GRUC(idx)                     0x00001d00+(idx)
#define R_GTUC(idx)                     0x00003a00+(idx)
/* Multicast Frame Counter */
#define R_GRMCA(idx)                    0x00000f00+(idx)
#define R_GTMCA(idx)                    0x00002c00+(idx)
/* Broadcast Frame Counter */
#define R_GRBCA(idx)                    0x00001000+(idx)
#define R_GTBCA(idx)                    0x00002d00+(idx)
/* Byte Counter */
#define R_GRBYT(idx)                     0x00001c00+(idx)
#define R_GTBYT(idx)                     0x00003900+(idx)
/* Pause Frame Counter */
#define R_GRXPF(idx)                    0x00000200+(idx)
#define R_GTXPF(idx)                    0x00001F00+(idx)
/* Frame counter */
#define R_GRPKT(idx)                    0x00001b00+(idx)
#define R_GTPKT(idx)                    0x00003800+(idx)
/* FCS Error Frame Counter */
#define R_GRFCS(idx)                    0x00000000+(idx)
#define R_GTFCS(idx)                    0x00002100+(idx)
/*  Alignment Error Frame Counter (receive only) */
#define R_GRALN(idx)                    0x00000400+(idx)
/* Oversized Frame Counter */
#define R_GROVR(idx)                    0x00000800+(idx)
#define R_GTOVR(idx)                    0x00002300+(idx)

/* EEE LPI counter */
#define R_GRX_EEE_LPI_EVENT_COUNTER(idx)     0x00003c00+(idx)
#define R_GRX_EEE_LPI_DURATION_COUNTER(idx)  0x00003d00+(idx)
#define R_GTX_EEE_LPI_EVENT_COUNTER(idx)     0x00003e00+(idx)
#define R_GTX_EEE_LPI_DURATION_COUNTER(idx)  0x00003f00+(idx)

/* XLPORT counter */

/* Unicast Frame Counter */
#define R_RUCA(idx)                     0x00000C00+(idx) 
#define R_TUCA(idx)                     0x00004D00+(idx)
/* Multicast Frame Counter */
#define R_RMCA(idx)                     0x00000D00+(idx) 
#define R_TMCA(idx)                     0x00004E00+(idx)
/* Broadcast Frame Counter */
#define R_RBCA(idx)                     0x00000E00+(idx)
#define R_TBCA(idx)                     0x00004F00+(idx)
/* Byte Counter */
#define R_RBYT(idx)                     0x00003d00+(idx)
#define R_TBYT(idx)                     0x00006f00+(idx)
/* Pause Frame Counter */
#define R_RXPF(idx)                     0x00001100+(idx)
#define R_TXPF(idx)                     0x00005000+(idx)
/* Frame counter */
#define R_RPKT(idx)                     0x00000b00+(idx)
#define R_TPKT(idx)                     0x00004c00+(idx)
/* FCS Error Frame Counter */
#define R_RFCS(idx)                     0x00000f00+(idx)
//#define R_TFCS(idx)                     0x00005300+(idx)
/*  Alignment Error Frame Counter (receive only) */
#define R_RALN(idx)                     0x00001600+(idx)
/* Oversized Frame Counter */
#define R_ROVR(idx)                     0x00001A00+(idx)
#define R_TOVR(idx)                     0x00005500+(idx)

/* XLPORT EEE LPI counter */
#define R_RX_EEE_LPI_EVENT_COUNTER(idx)     0x00003600+(idx)
#define R_RX_EEE_LPI_DURATION_COUNTER(idx)  0x00003700+(idx)
#define R_TX_EEE_LPI_EVENT_COUNTER(idx)     0x00006A00+(idx)
#define R_TX_EEE_LPI_DURATION_COUNTER(idx)  0x00006B00+(idx)

/* IPIPE HOLD Drop Counter */
#define R_HOLD_COS(idx)                  10,0x42005B00+(idx)
#define R_HOLD_COS_PORT_SELECT           10,0x42006800
#define R_HOLD_COS0                      10,0x42005B00 
#define R_HOLD_COS1                      10,0x42005C00 
#define R_HOLD_COS2                      10,0x42005D00 
#define R_HOLD_COS3                      10,0x42005E00 
#define R_HOLD_COS4                      10,0x42005F00 
#define R_HOLD_COS5                      10,0x42006000 
#define R_HOLD_COS6                      10,0x42006100 
#define R_HOLD_COS7                      10,0x42006200 

/* mcast related*/
#define R_IGMP_MLD_PKT_CONTROL(idx)     10,0x1c000500+(idx)
#define M_L2_ENTRY(idx)                 10,0x1c000000+(idx)
#define M_L2MC(idx)                     10,0x20000000+(idx)
#define L2_ENTRY_SIZE              16384

#define R_UNKNOWN_MCAST_BLOCK_MASK_64(idx)   10,0x40000200+(idx)
#define R_IUNKNOWN_MCAST_BLOCK_MASK_64(idx)  10,0x40000300+(idx)

#define M_IRQ_CHAIN_DONE(ch)     (1 << (ch))
#define M_IRQ_DMA_ACTIVE(ch)     (0x100 << (ch))

#define S_DCB1_BYTE_COUNT        0
#define M_DCB1_BYTE_COUNT        _DD_MAKEMASK(16,S_DCB1_BYTE_COUNT)
#define V_DCB1_BYTE_COUNT(x)     _DD_MAKEVALUE(x,S_DCB1_BYTE_COUNT)
#define G_DCB1_BYTE_COUNT(x)     _DD_GETVALUE(x,S_DCB1_BYTE_COUNT,M_DCB1_BYTE_COUNT)

#define S_DCB1_SRC_PORT          16
#define M_DCB1_SRC_PORT          _DD_MAKEMASK(8,S_DCB1_SRC_PORT)
#define V_DCB1_SRC_PORT(x)       _DD_MAKEVALUE(x,S_DCB1_SRC_PORT)
#define G_DCB1_SRC_PORT(x)       _DD_GETVALUE(x,S_DCB1_SRC_PORT,M_DCB1_SRC_PORT)

#define S_DCB1_OUTER_PRI         10
#define M_DCB1_OUTER_PRI         _DD_MAKEMASK(3,S_DCB1_OUTER_PRI)
#define V_DCB1_OUTER_PRI(x)      _DD_MAKEVALUE(x,S_DCB1_OUTER_PRI)
#define G_DCB1_OUTER_PRI(x)      _DD_GETVALUE(x,S_DCB1_OUTER_PRI,M_DCB1_OUTER_PRI)

#define CMIC_CMC1_CH0_DMA_CTRL   0x3232140
#define CMIC_CMC1_CH1_DMA_CTRL   0x3232144
#define CMIC_CMC1_CHx_DMA_CTRL(ch) (0x3232140+4*(ch))
#define CMIC_CMC1_DMA_STAT       0x3232150
#define CMIC_CMC1_DMA_DESC0      0x3232158
#define CMIC_CMC1_DMA_DESC(ch)   (CMIC_CMC1_DMA_DESC0+4*(ch))   /* 0 <= ch <= 3 */

#define CMIC_CMC1_CH1_COS_CTRL_RX_0 0x3232170
#define CMIC_CMC1_DMA_STAT_CLR      0x32321a4

#define CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITS 0x321a000

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

/* MMU setting */
#define R_WRRWEIGHT_COS0(idx)        12,0x00013300+(idx)
#define R_WRRWEIGHT_COS1(idx)        12,0x00013400+(idx)
#define R_WRRWEIGHT_COS2(idx)        12,0x00013500+(idx)
#define R_WRRWEIGHT_COS3(idx)        12,0x00013600+(idx)
#define R_WRRWEIGHT_COS4(idx)        12,0x00013700+(idx)
#define R_WRRWEIGHT_COS5(idx)        12,0x00013800+(idx)
#define R_WRRWEIGHT_COS6(idx)        12,0x00013900+(idx)
#define R_WRRWEIGHT_COS7(idx)        12,0x00013A00+(idx)
#define R_XQCOSARBSEL(idx)           12,0x00013200+(idx)
#define R_MAXBUCKETCONFIG(cos, idx)  12,0x00019c00+((cos) << 8)+(idx)
#define R_COSLCCOUNT(cos, idx)       12,0x00010D00+((cos) << 8)+(idx)

#define R_LOG_TO_PHY_PORT_MAPPING(idx) 12,0x00086000+(idx)
#define M_MMU_IPMC_GROUP_TBL(port,idx) 12,(0x28098000+(port * 0x4000)+idx)

/* Stage [31:26] = 13 */
#define R_EGR_LOGICAL_TO_PHYSICAL_PORT_NUMBER_MAPPING(idx)  11,0x30001C00+(idx)

#define M_ING_PHYSICAL_TO_LOGICAL_PORT_NUMBER_MAPPING_TABLE(idx) 10,0x000C0000+(idx)

#define M_IARB_TDM_TABLE(idx)        10,0x00080000+(idx)
#define R_IARB_TDM_CONTROL           10,0x02000900

#define M_MMU_ARB_TDM_TABLE(idx)     12,0x2C078000+(idx)
#define R_MMUPORTENABLE              12,0x02025E00
#define R_MMUFLUSHCONTROL            12,0x02030C00
#define R_FLUSH_CONTROL(idx)         0x0001cd00+(idx)

#define R_XQEMPTY(idx)                  12,0x00012F00+(idx)

#define R_GEX_GBODE_CELL_CNT(idx)       0x02021100+((idx) << 8)

#define R_HOLCOSPKTRESETLIMIT(cos, idx) 0x00002800+((cos) << 8)+(idx)
#define R_HOLCOSPKTSETLIMIT(cos, idx)   0x00000800+((cos) << 8)+(idx)
#define R_LWMCOSCELLSETLIMIT(cos, idx)  0x0000cd00+((cos) << 8)+(idx)
#define R_DYNCELLLIMIT(idx)             12,0x00012d00+(idx)
#define R_HOLCOSCELLMAXLIMIT(cos, idx)  0x0000ed00+((cos) << 8)+(idx)
#define R_HOLCOSMINXQCNT(cos, idx)      0x00004800+((cos) << 8)+(idx)
#define R_DYNXQCNTPORT(idx)             12,0x0000ca00+(idx)
#define R_DYNRESETLIMPORT(idx)          12,0x0000cb00+(idx)


#define R_TOTALDYNCELLSETLIMIT          12,0x02025B00
#define R_TOTALDYNCELLRESETLIMIT        12,0x02030400

#define R_MISCCONFIG                    12,0x02025A00

#define R_IBPPKTSETLIMIT(idx)           12,0x00000000+(idx)
#define R_PGCELLLIMIT(cos, idx)         0x177000+((cos) << 8)+(idx)
#define R_PGDISCARDSETLIMIT(cos, idx)   0x178000+((cos) << 8)+(idx)

/* 24g_4uplink MMU setting */
#define MMU_R_HOLCOSPKTSETLIMIT     0x1780
#define MMU_R_HOLCOSPKTRESETLIMIT   0x177c
#define MMU_R_LWMCOSCELLSETLIMIT    0x40010   /* CELLRESETLIMIT = 16, CELLSETLIMIT = 16 */
#define MMU_R_HOLCOSCELLMAXLIMIT_COS0   0x7951e60 /* COS0: CELLMAXLIMIT = 7776 (0x1e60), RESUMELIMIT = 7764 (0x1e54)*/
#define MMU_R_HOLCOSCELLMAXLIMIT_COS1   0x90030   /* COS1~3: CELLMAXLIMIT = 48 (0x30), RESUMELIMIT = 36 (0x24) */
#define MMU_R_HOLCOSMINXQCNT        0x10
#define MMU_R_PGCELLLIMIT           0xa0050    /* CELLSETLIMIT=0x50,CELLRESETLIMIT=0x28 */ 
#define MMU_R_PGDISCARDSETLIMIT     0x3FFF
#define MMU_1M_R_PGDISCARDSETLIMIT  0x1FFF

#define MMU_R_DYNCELLLIMIT          0x7e61fb0 /* DYNCELLRESETLIMISEL = 8088 (0x1f98), DYNCELLSETLIMIT = 8112 (0x1fb0) */
#define MMU_R_DYNXQCNTPORT          0x1770
#define MMU_R_DYNRESETLIMPORT       0x1760
#define MMU_R_IBPPKTSETLIMIT        0xc       /* PKTSETLIMIT = 12, RESETLIMITSEL = 0(75%) */

#define MMU_R_TOTALDYNCELLSETLIMIT      0x3180
#define MMU_R_TOTALDYNCELLRESETLIMIT    0x3168
#define MMU_1M_R_TOTALDYNCELLSETLIMIT   0x1180
#define MMU_1M_R_TOTALDYNCELLRESETLIMIT 0x1168

/* 16x10g MMU setting */
#define MMU_16X10G_R_HOLCOSPKTSETLIMIT      0x1780
#define MMU_16X10G_R_HOLCOSPKTRESETLIMIT    0x177c
#define MMU_16X10G_R_LWMCOSCELLSETLIMIT     0x40010   /* CELLRESETLIMIT = 16, CELLSETLIMIT = 16 */
#define MMU_16X10G_R_HOLCOSCELLMAXLIMIT_COS0    0x3fc1000 /* COS0: CELLMAXLIMIT = 4096 (0x1000), RESUMELIMIT = 4080 (0xFF0)*/
#define MMU_16X10G_R_HOLCOSCELLMAXLIMIT_COS1    0x3fc1000 /* COS1~7: CELLMAXLIMIT = 4096 (0x1000), RESUMELIMIT = 4080 (0xFF0) */
#define MMU_16X10G_R_HOLCOSMINXQCNT         0x10
#define MMU_16X10G_R_PGCELLLIMIT            0xa0050    /* CELLSETLIMIT=0x50,CELLRESETLIMIT=0x28 */   
#define MMU_16X10G_R_PGDISCARDSETLIMIT      0x3FFF
#define MMU_16X10G_1M_R_PGDISCARDSETLIMIT   0x1FFF


#define MMU_16X10G_R_DYNCELLLIMIT           0x44e1150 /* DYNCELLRESETLIMISEL = 4408 (0x1138), DYNCELLSETLIMIT = 4432 (0x1150) */
#define MMU_16X10G_R_DYNXQCNTPORT           0x1770
#define MMU_16X10G_R_DYNRESETLIMPORT        0x1760
#define MMU_16X10G_R_IBPPKTSETLIMIT         0x4050    /* PKTSETLIMIT = 80, RESETLIMITSEL = 1(50%) */    

#define MMU_16X10G_R_TOTALDYNCELLSETLIMIT       0x3780
#define MMU_16X10G_R_TOTALDYNCELLRESETLIMIT     0x3760
#define MMU_16X10G_1M_R_TOTALDYNCELLSETLIMIT    0x1780
#define MMU_16X10G_1M_R_TOTALDYNCELLRESETLIMIT  0x1760

/* 12g_12x10g MMU setting */
#define MMU_12G_12X10G_R_HOLCOSPKTSETLIMIT     0x1780
#define MMU_12G_12X10G_R_HOLCOSPKTRESETLIMIT   0x177c
#define MMU_12G_12X10G_R_LWMCOSCELLSETLIMIT    0x40010   /* CELLRESETLIMIT = 16, CELLSETLIMIT = 16 */
#define MMU_12G_12X10G_R_HOLCOSCELLMAXLIMIT_COS0    0x35d0d80 /* COS0: CELLMAXLIMIT = 3456 (0xd80), RESUMELIMIT = 3444 (0xd74)*/
#define MMU_12G_12X10G_R_HOLCOSCELLMAXLIMIT_COS1    0x90030 /* COS1~3: CELLMAXLIMIT = 48 (0x30), RESUMELIMIT = 36 (0x24) */
#define MMU_12G_12X10G_R_HOLCOSMINXQCNT        0x10
#define MMU_12G_12X10G_R_PGCELLLIMIT           0xa0050    /* CELLSETLIMIT=0x50,CELLRESETLIMIT=0x28 */
#define MMU_12G_12X10G_R_PGDISCARDSETLIMIT     0x3FFF
#define MMU_12G_12X10G_1M_R_PGDISCARDSETLIMIT  0x1FFF

#define MMU_12G_12X10G_R_DYNCELLLIMIT          0x3ae0ed0 /* DYNCELLRESETLIMISEL = 3768 (0xeb8), DYNCELLLIMIT = 3792 (0xed0) */
#define MMU_12G_12X10G_R_DYNXQCNTPORT          0x1770
#define MMU_12G_12X10G_R_DYNRESETLIMPORT       0x1760
#define MMU_12G_12X10G_R_IBPPKTSETLIMIT        0x50       /* PKTSETLIMIT = 0x50, RESETLIMITSEL = 0(75%) */ 

#define MMU_12G_12X10G_R_TOTALDYNCELLSETLIMIT   0x3380
#define MMU_12G_12X10G_R_TOTALDYNCELLRESETLIMIT 0x3368
#define MMU_12G_12X10G_1M_R_TOTALDYNCELLSETLIMIT   0x1380
#define MMU_12G_12X10G_1M_R_TOTALDYNCELLRESETLIMIT 0x1368
/* End of MMU setting */

#define PORT_STATUS_FLAGS_GIGA_SPEED    (1 << 0)
#define PORT_STATUS_FLAGS_GREEN_MODE    (1 << 1)
#define PORT_STATUS_FLAGS_TX_PAUSE      (1 << 2)
#define PORT_STATUS_FLAGS_RX_PAUSE      (1 << 3)
#define PORT_STATUS_FLAGS_REMOTE_EEE    (1 << 4)

/* EEE related */
/* MAC register*/
#define R_UMAC_EEE_CTRL(idx)            0x00011900+(idx)
#define R_MII_EEE_DELAY_ENTRY_TIMER(idx)  0x00011A00+(idx)
#define R_GMII_EEE_DELAY_ENTRY_TIMER(idx) 0x00011B00+(idx)
#define R_UMAC_EEE_REF_COUNT(idx)       0x00011C00+(idx)
#define R_MII_EEE_WAKE_TIMER(idx)       0x00012000+(idx)
#define R_GMII_EEE_WAKE_TIMER(idx)      0x00012100+(idx)

/* EEE parameters */
#define BCM5340X_PHY_EEE_100MB_MIN_WAKE_TIME    36  /* 36 micro seconds */
#define BCM5340X_PHY_EEE_1GB_MIN_WAKE_TIME      17  /* 17 micro seconds */

#define CMIC_SBUS_TIMEOUT                        0x3210094

#define CMIC_SBUS_RING_MAP_0_7                   0x3210098
#define CMIC_SBUS_RING_MAP_8_15                  0x321009c
#define CMIC_SBUS_RING_MAP_16_23                 0x32100a0
#define CMIC_SBUS_RING_MAP_24_31                 0x32100a4
#define CMIC_SBUS_RING_MAP_32_39                 0x32100a8

#define CMIC_CPS_RESET                           0x3210220

#define CMIC_RATE_ADJUST_EXT_MDIO                0x3211000
#define CMIC_RATE_ADJUST_INT_MDIO                0x3211004

#define CMIC_DEV_REV_ID                          0x3210224

#define CMIC_SEMAPHORE_1                         0x3210300
#define CMIC_SEMAPHORE_1_SHADOW                  0x3210304
#define CMIC_SEMAPHORE_2                         0x3210308
#define CMIC_SEMAPHORE_2_SHADOW                  0x321030C
#define CMIC_SEMAPHORE_3                         0x3210310
#define CMIC_SEMAPHORE_3_SHADOW                  0x3210314

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

#define CMIC_LEDUP0_CTRL                         0x3220000
#define CMIC_LEDUP0_PORT_ORDER_REMAP_0_3         0x3220010
#define CMIC_LEDUP0_PORT_ORDER_REMAP_60_63       0x322004c

#define CMIC_LEDUP0_DATA_RAM0                    0x3220400
#define CMIC_LEDUP0_PROGRAM_RAM0                 0x3220800
#define CMIC_PCIE_USERIF_PURGE_CONTROL         (0x03210260)
#define CMIC_CMC1_HOSTMEM_ADDR_REMAP_0         (0x03232474)
#define CMIC_CMC1_HOSTMEM_ADDR_REMAP_1         (0x03232478)
#define CMIC_CMC1_HOSTMEM_ADDR_REMAP_2         (0x0323247c)
#define CMIC_CMC1_HOSTMEM_ADDR_REMAP_3         (0x032324ac)


#define CMIC_LEDUP_PROGRAM_RAM_D(word) \
		(CMIC_LEDUP0_PROGRAM_RAM0+(4*(word)))

#define CMIC_LEDUP_DATA_RAM_D(word) \
		(CMIC_LEDUP0_DATA_RAM0+(4*(word)))
#define LED_PORT_STATUS_OFFSET(p)   CMIC_LEDUP_DATA_RAM_D(0xa0 + (p))

#define R_XLPORT_LED_CHAIN_CONFIG                0x02022300

#define CMIC_CMC1_SCHAN_CTRL                     0x3232000
#define CMIC_CMC1_SCHAN_ACK_DATA_BEAT_COUNT      0x3232004
#define CMIC_CMC1_SCHAN_ERR                      0x3232008
#define CMIC_CMC1_SCHAN_MESSAGE0                 0x323200c

#define CMIC_CMC1_MIIM_PARAM                     0x3232080
#define CMIC_CMC1_MIIM_READ_DATA                 0x3232084
#define CMIC_CMC1_MIIM_ADDRESS                   0x3232088
#define CMIC_CMC1_MIIM_CTRL                      0x323208c
#define CMIC_CMC1_MIIM_STAT                      0x3232090

#define CMIC_GP_DATA_OUT                         0x3202004
#define CMIC_GP_OUT_EN                           0x3202008

#define R_TX_IPG_LENGTH(idx)                     0x00011700+(idx)
#define R_COMMAND_CONFIG(idx)                    0x00010200+(idx)
#define R_MAC_0(idx)                             0x00010300+(idx)
#define R_MAC_1(idx)                             0x00010400+(idx)
#define R_FRM_LENGTH(idx)                        0x00010500+(idx)
#define R_GPORT_RSV_MASK                         0x02020600
#define R_GPORT_STAT_UPDATE_MASK                 0x02020700

#define R_XLMAC_CTRL(idx)                        0x00060000+(idx)
#define R_XLMAC_MODE(idx)                        0x00060100+(idx)
#define R_XLMAC_TX_CTRL(idx)                     0x00060400+(idx)
#define R_XLMAC_TX_MAC_SA(idx)                   0x00060500+(idx)
#define R_XLMAC_RX_CTRL(idx)                     0x00060600+(idx)
#define R_XLMAC_RX_MAX_SIZE(idx)                 0x00060800+(idx)
#define R_XLMAC_RX_LSS_CTRL(idx)                 0x00060A00+(idx)
#define R_XLMAC_PAUSE_CTRL(idx)                  0x00060D00+(idx)
#define R_XLMAC_PFC_CTRL(idx)                    0x00060E00+(idx)
#define R_XLMAC_LLFC_CTRL(idx)                   0x00061200+(idx)
#define R_XLMAC_EEE_CTRL(idx)                    0x00061A00+(idx)
#define R_XLMAC_EEE_TIMERS(idx)                  0x00061B00+(idx)
#define R_XLMAC_TXFIFO_CELL_CNT(idx)             0x00062B00+(idx)
#define R_XLMAC_TXFIFO_CELL_REQ_CNT(idx)         0x00062C00+(idx)
#define R_PGW_XL_CONFIG                          0x00080000+(idx)
#define R_PGW_XL_TXFIFO_CTRL(idx)                0x00080100+(idx)
#define R_PGW_GX_CONFIG                          0x00090000+(idx)
#define R_PGW_GX_TXFIFO_CTRL(idx)                0x00090100+(idx)
#define R_PGW_XL_ECC_CONTROL                     0x02080400
#define R_GPORT_CONFIG                           0x02020000
#define R_GPORT_CONFIG_BLOCK_2_BASE              2,0x02020000
#define R_GPORT_CONFIG_BLOCK_3_BASE              3,0x02020000
#define R_GPORT_CONFIG_BLOCK_4_BASE              4,0x02020000

#define R_GPORT_LINK_STATUS_TO_CMIC              0x02023100
#define R_XLPORT_CONFIG(idx)                     0x00020000+(idx)
#define R_XLPORT_CNTMAXSIZE(idx)                 0x00020100+(idx)
#define R_XLPORT_LAG_FAILOVER_CONFIG(idx)        0x00020200+(idx)
#define R_XLPORT_MAC_RSV_MASK(idx)               0x00020800+(idx)
#define R_XLPORT_MODE_REG                        0x02020A00
#define R_XLPORT_ENABLE_REG                      0x02020B00
#define R_XLPORT_SOFT_RESET                      0x02020C00
#define R_XLPORT_POWER_SAVE                      0x02020D00
#define R_XLPORT_MAC_CONTROL                     0x02021000
#define R_XLPORT_TSC_PLL_LOCK_STATUS             0x02021300
#define R_XLPORT_XGXS0_CTRL_REG                  0x02021400
#define R_XLPORT_WC_UCMEM_CTRL                   0x02021900
#define R_XLPORT_ECC_CONTROL                     0x02021A00
#define M_XLPORT_WC_UCMEM_DATA(idx)              0x00000000+(idx)
#define R_XLPORT_MIB_RESET                       0x02022400

#define R_ING_HW_RESET_CONTROL_1                 10,0x02000200
#define R_ING_HW_RESET_CONTROL_2                 10,0x02000300
#define R_IP_TO_CMICM_CREDIT_TRANSFER            10,0x02002300
#define R_EPC_LINK_BMAP_64                       10,0x42002700
#define R_ING_MISC_CONFIG                        10,0x46002E00
#define R_PROTOCOL_PKT_CONTROL(idx)              10,0x1C000400+(idx)
#define R_UNKNOWN_UCAST_BLOCK_MASK_64(idx)       10,0x40000000+(idx)
#define R_IUNKNOWN_UCAST_BLOCK_MASK_64(idx)      10,0x40000100+(idx)

#define R_ING_EGRMSKBMAP_64(idx)                 10,0x40000E00+(idx)
#define R_IING_EGRMSKBMAP_64(idx)                10,0x40000F00+(idx)

#define R_EGR_HW_RESET_CONTROL_0                 11,0x02000000
#define R_EGR_HW_RESET_CONTROL_1                 11,0x02000100
#define R_EGR_PORT_BUFFER_CLK_SHUTDOWN           11,0x32012200

#define R_TOP_SOFT_RESET_REG                     16,0x02030400
#define R_TOP_SOFT_RESET_REG_2                   16,0x02030800
#define R_TOP_MISC_CONTROL_1                     16,0x02031400
#define R_TOP_MISC_STATUS                        16,0x02031800
#define R_TOP_CLOCKING_ENFORCE_PSG               16,0x02032400
#define R_TOP_TOP_CORE_PLL_STATUS_1              16,0x02040800
#define R_TOP_XG_PLL0_CTRL_0                     16,0x02042000
#define R_TOP_XG_PLL0_CTRL_1                     16,0x02042400
#define R_TOP_XG_PLL0_CTRL_2                     16,0x02042800
#define R_TOP_XG_PLL0_CTRL_3                     16,0x02042C00
#define R_TOP_XG_PLL0_CTRL_4                     16,0x02043000
#define R_TOP_XG_PLL0_CTRL_5                     16,0x02043100
#define R_TOP_XG_PLL0_CTRL_6                     16,0x02043200
#define R_TOP_XG_PLL0_CTRL_7                     16,0x02043300
#define R_TOP_XG_PLL0_STATUS                     16,0x02043400
#define R_TOP_XG0_LCPLL_FBDIV_CTRL_0             16,0x0207A400
#define R_TOP_XG0_LCPLL_FBDIV_CTRL_1             16,0x0207A800
#define R_TOP_BS_PLL0_CTRL_0                     16,0x0204B000
#define R_TOP_BS_PLL0_CTRL_1                     16,0x0204B400
#define R_TOP_BS_PLL0_CTRL_2                     16,0x0204B800
#define R_TOP_BS_PLL0_CTRL_3                     16,0x0204BC00
#define R_TOP_BS_PLL0_CTRL_4                     16,0x0204C000
#define R_TOP_BS_PLL0_CTRL_5                     16,0x0204C100
#define R_TOP_BS_PLL0_CTRL_6                     16,0x0204C200
#define R_TOP_BS_PLL0_CTRL_7                     16,0x0204C300
#define R_TOP_BROAD_SYNC0_LCPLL_FBDIV_CTRL_0     16,0x0207B400
#define R_TOP_BROAD_SYNC0_LCPLL_FBDIV_CTRL_1     16,0x0207B800

#define R_TOP_XG_PLL1_CTRL_0                     16,0x02043800
#define R_TOP_XG_PLL1_CTRL_1                     16,0x02043C00
#define R_TOP_XG_PLL1_CTRL_2                     16,0x02044000
#define R_TOP_XG_PLL1_CTRL_3                     16,0x02044400
#define R_TOP_XG_PLL1_CTRL_4                     16,0x02044800
#define R_TOP_XG_PLL1_CTRL_5                     16,0x02044900
#define R_TOP_XG_PLL1_CTRL_6                     16,0x02044A00
#define R_TOP_XG_PLL1_CTRL_7                     16,0x02044B00
#define R_TOP_XG_PLL1_STATUS                     16,0x02044C00
#define R_TOP_XG1_LCPLL_FBDIV_CTRL_0             16,0x0207AC00
#define R_TOP_XG1_LCPLL_FBDIV_CTRL_1             16,0x0207B000
#define R_TOP_BS_PLL1_CTRL_0                     16,0x02078000
#define R_TOP_BS_PLL1_CTRL_1                     16,0x02078400
#define R_TOP_BS_PLL1_CTRL_2                     16,0x02078800
#define R_TOP_BS_PLL1_CTRL_3                     16,0x02078C00
#define R_TOP_BS_PLL1_CTRL_4                     16,0x02079000
#define R_TOP_BS_PLL1_CTRL_5                     16,0x02079100
#define R_TOP_BS_PLL1_CTRL_6                     16,0x02079200
#define R_TOP_BS_PLL1_CTRL_7                     16,0x02079300
#define R_TOP_BROAD_SYNC1_LCPLL_FBDIV_CTRL_0     16,0x0207BC00
#define R_TOP_BROAD_SYNC1_LCPLL_FBDIV_CTRL_1     16,0x0207C000

#define R_RESCAL_STATUS_0                        16,0x02057A00
#define R_PGW_CTRL_0                             16,0x02058000
#define R_TOP_MISC_CONTROL_2                     16,0x02079800
#define R_TOP_MISC_CONTROL_3                     16,0x02079A00
#define R_CHIP_CONFIG                            35,0x02010100
#define R_CHIP_SBUS_CFG                          35,0x02010200
#define R_CHIP_SWRST                             35,0x02010300
#define R_CHIP_INDACC_CTLSTS                     35,0x02010400
#define R_CHIP_INDACC_WDATA                      35,0x02010500
#define R_CHIP_INDACC_RDATA                      35,0x02010600
#define R_CHIP_UMACSPEED                         35,0x02010700
#define R_CHIP_INDACC_WDATA                      35,0x02010500
#define R_CHIP_INDACC_RDATA                      35,0x02010600
#define R_CHIP_INDACC_RDATA                      35,0x02010600

#define R_SPEED_LIMIT_ENTRY_PHYSICAL_PORT_NUMBER(idx)             16,0x02051800 + (0x400 * idx)

#ifdef CFG_RESET_BUTTON_INCLUDED
#define CMIC_GP_DATA_IN                          0x3202000

#define CCG_GP_DATA_IN                           0x1800a000
#define CCG_GP_DATA_OUT                          0x1800a004
#define CCG_GP_OUT_EN                            0x1800a008
#endif /* CFG_RESET_BUTTON_INCLUDED */

/* USB related */
#define R_IPROC_WRAP_USBPHY_CTRL_2               0x1800FC4C
#define R_USBH_UTMI_P0CTL                        0x18049510

/* EEE functions */
#ifdef CFG_SWITCH_EEE_INCLUDED
extern void bcm5340x_port_eee_enable_get(uint8 unit, uint8 port, uint8 *enable);
extern sys_error_t bcm5340x_port_eee_enable_set(uint8 unit, 
                      uint8 port, uint8 enable, uint8 save);
extern sys_error_t bcm5340x_port_eee_tx_wake_time_set(uint8 unit, 
                      uint8 port, uint8 type, uint16 wake_time);
extern void bcm5340x_eee_init(void);
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

/* Switch characteristics */

#define XLPORT0_BLOCK_ID                         4
#define XLPORT1_BLOCK_ID                         5
#define XLPORT2_BLOCK_ID                         6
#define XLPORT3_BLOCK_ID                         7
#define XLPORT4_BLOCK_ID                         8
#define MMU_BLOCK_ID                            12
#define XLPORT5_BLOCK_ID                        32

#define GXPORT0_BLOCK_ID                        33
#define GXPORT1_BLOCK_ID                        34

#define PHY_GXPORT0_BASE                         2  /* port 2 - 9 in 53401 */
#define PHY_XLPORT5_BASE                         2  /* port 2 - 5 in 53406 and 53408 */
#define PHY_GXPORT1_BASE                         10 /* port 10 - 17 */
#define PHY_XLPORT0_BASE                         18 /* port 18 - 21 */
#define PHY_XLPORT1_BASE                         22 /* port 22 - 25 */
#define PHY_XLPORT2_BASE                         26 /* port 26 - 29 */
#define PHY_XLPORT3_BASE                         30 /* port 30 - 33 */
#define PHY_XLPORT4_BASE                         34 /* port 34 - 37 */

/* physical port */
#define BCM5340X_PORT_MIN                         2
#define BCM5340X_PORT_MAX                        37

/* logical port */
#define BCM5340X_LPORT_MIN                        2
#define BCM5340X_LPORT_MAX                       29

typedef enum port_block_type_s {
    PORT_BLOCK_TYPE_XLPORT,
    PORT_BLOCK_TYPE_GXPORT,
    PORT_BLOCK_TYPE_COUNT
} port_block_type_t;

enum soc_gh_port_mode_e {
    /* WARNING: values given match hardware register; do not modify */
    SOC_GH_PORT_MODE_QUAD = 0,
    SOC_GH_PORT_MODE_TRI_012 = 1,
    SOC_GH_PORT_MODE_TRI_023 = 2,
    SOC_GH_PORT_MODE_DUAL = 3,
    SOC_GH_PORT_MODE_SINGLE = 4
};

typedef enum tsce_interface_s {
    TSCE_INTERFACE_SGMII = 1,
    TSCE_INTERFACE_FIBER = 2,
} tsce_interface_t;

/* SOC interface */
extern sys_error_t bcm5340x_chip_revision(uint8 unit, uint16 *dev, uint16 *rev);
extern uint8 bcm5340x_port_count(uint8 unit);
extern sys_error_t bcm5340x_reg_get(uint8 unit, uint8 block_id, uint32 addr, uint32 *val);
extern sys_error_t bcm5340x_reg_set(uint8 unit, uint8 block_id, uint32 addr, uint32 val);
extern sys_error_t bcm5340x_reg64_get(uint8 unit, uint8 block_id, uint32 addr, uint32 *val, int len);
extern sys_error_t bcm5340x_reg64_set(uint8 unit, uint8 block_id, uint32 addr, uint32 *buf, int len);
extern sys_error_t bcm5340x_mem_get(uint8 unit, uint8 block_id, uint32 addr, uint32 *buf, int len);
extern sys_error_t bcm5340x_mem_set(uint8 unit, uint8 block_id, uint32 addr, uint32 *buf, int len);
extern sys_error_t bcm5340x_l2_op(uint8 unit, l2x_entry_t *entry, uint8 op);
extern sys_error_t bcm5340x_rx_set_handler(uint8 unit, SOC_RX_HANDLER fn, 
                                           BOOL intr);
extern sys_error_t bcm5340x_rx_fill_buffer(uint8 unit, 
                                           soc_rx_packet_t *pkt);
extern sys_error_t bcm5340x_tx(uint8 unit, soc_tx_packet_t *pkt);
extern sys_error_t bcm5340x_link_status(uint8 unit, uint8 port, BOOL *link);
extern void bcm5340x_rxtx_stop(void);

/* FP_TCAM encode/decode utility */
extern void bcm5340x_dm_to_xy(uint32 *dm_entry, uint32 *xy_entry, int data_words, int bit_length);
extern void bcm5340x_xy_to_dm(uint32 *xy_entry, uint32 *dm_entry, int data_words, int bit_length);

#ifdef CFG_SWITCH_VLAN_INCLUDED
extern sys_error_t bcm5340x_pvlan_egress_set(uint8 unit, 
                    uint8 pport, 
                    pbmp_t pbmp);
extern sys_error_t bcm5340x_pvlan_egress_get(uint8 unit, 
                    uint8 pport, 
                    pbmp_t *pbmp);
extern sys_error_t bcm5340x_qvlan_port_set(uint8 unit, 
                    uint16 vlan_id, 
                    pbmp_t pbmp, 
                    pbmp_t tag_pbmp);
extern sys_error_t bcm5340x_qvlan_port_get(uint8 unit, 
                    uint16 vlan_id, 
                    pbmp_t *pbmp, 
                    pbmp_t *tag_pbmp);
extern sys_error_t bcm5340x_vlan_create(uint8 unit, 
                    vlan_type_t type, uint16 vlan_id);
extern sys_error_t bcm5340x_vlan_destroy(uint8 unit, uint16 vlan_id);
extern sys_error_t bcm5340x_vlan_type_set(uint8 unit, vlan_type_t type);
extern sys_error_t bcm5340x_vlan_reset(uint8 unit);
#endif /* CFG_SWITCH_VLAN_INCLUDED */

#ifdef CFG_SWITCH_LAG_INCLUDED
extern sys_error_t bcm5340x_lag_group_set(uint8 unit, uint8 lagid, pbmp_t lpbmp);
extern void bcm5340x_lag_group_get(uint8 unit, uint8 lagid, pbmp_t *lpbmp);
#endif /* CFG_SWITCH_LAG_INCLUDED */

extern void bcm5340x_loop_detect_enable(BOOL enable);
extern uint8 bcm5340x_loop_detect_status_get(void);

#ifdef CFG_ZEROCONF_MDNS_INCLUDED
sys_error_t bcm5340x_mdns_enable_set(uint8 unit, BOOL mdns_enable);
sys_error_t bcm5340x_mdns_enable_get(uint8 unit, BOOL *mdns_enable);
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

/* Initialization */
extern sys_error_t bcm5340x_sw_init(void);
extern void bcm5340x_dscp_map_enable(BOOL enable);
extern void bcm5340x_qos_init(void);
extern void bcm5340x_rate_init(void);
extern void bcm5340x_loop_detect_init(void);
extern void bcm5340x_rxtx_init(void);
extern void bcm5340x_loopback_enable(uint8 unit, uint8 port, int loopback_mode);
extern sys_error_t bcm5340x_mdns_enable_set(uint8 unit, BOOL enable);
extern sys_error_t bcm5340x_mdns_enable_get(uint8 unit, BOOL *enable);


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
    uint8   link[BCM5340X_LPORT_MAX + 1];           /* link status */
    uint8   loopback[BCM5340X_LPORT_MAX + 1];       /* loopback mode */
    uint8   port_count;
    uint32  pbmp;
#if defined(CFG_SWITCH_EEE_INCLUDED)
    tick_t link_up_time[BCM5340X_LPORT_MAX+1];
    uint8 need_process_for_eee_1s[BCM5340X_LPORT_MAX+1];
#endif /*  CFG_SWITCH_EEE_INCLUDED */
    int    port_p2l_mapping[BCM5340X_PORT_MAX + 1];   /* phy to logic */
    int    port_l2p_mapping[BCM5340X_LPORT_MAX + 1];  /* logic to phy */
    int    port_block_id[BCM5340X_LPORT_MAX + 1];     /* logical port */
    int    port_block_port_id[BCM5340X_LPORT_MAX + 1];/* logical port */
    int    port_block_type[BCM5340X_LPORT_MAX + 1];   /* logical port */
    int    port_speed_max[BCM5340X_LPORT_MAX + 1];    /* logical port */
    int    lane_number[BCM5340X_LPORT_MAX + 1];    /* logical port */
    int    port_admin_status[BCM5340X_LPORT_MAX + 1];   /* logical port */
    int    port_speed_satus[BCM5340X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_link_status[BCM5340X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_duplex_status[BCM5340X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_an_status[BCM5340X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_tx_pause_status[BCM5340X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_rx_pause_status[BCM5340X_LPORT_MAX + 1];   /* logical port */
    mac_driver_t *p_mac[BCM5340X_LPORT_MAX + 1];      /* mac driver */
} bcm5340x_sw_info_t;

extern bcm5340x_sw_info_t gh_sw_info;

#define COS_QUEUE_NUM                  (4)

/* Mask of all logical ports */
#define BCM5340X_ALL_PORTS_MASK     (gh_sw_info.pbmp)

/*
 * Macros to get the device block type, block index and index
 * within block for a given port
 */
#define SOC_PORT_SPEED_MAX(port)   (gh_sw_info.port_speed_max[(port)])
#define SOC_PORT_LANE_NUMBER(port)   (gh_sw_info.lane_number[(port)])
#define SOC_PORT_ADMIN_STATUS(port)   (gh_sw_info.port_admin_status[(port)])
#define SOC_PORT_SPEED_STATUS(port)   (gh_sw_info.port_speed_satus[(port)])
#define SOC_PORT_LINK_STATUS(port)   (gh_sw_info.port_link_status[(port)])
#define SOC_PORT_DUPLEX_STATUS(port)   (gh_sw_info.port_duplex_status[(port)])
#define SOC_PORT_AN_STATUS(port)   (gh_sw_info.port_an_status[(port)])
#define SOC_PORT_TX_PAUSE_STATUS(port)   (gh_sw_info.port_tx_pause_status[(port)])
#define SOC_PORT_RX_PAUSE_STATUS(port)   (gh_sw_info.port_rx_pause_status[(port)])
#define SOC_PORT_BLOCK_TYPE(port)   (gh_sw_info.port_block_type[(port)])
#define SOC_PORT_BLOCK(port)        (gh_sw_info.port_block_id[(port)])
#define SOC_PORT_BLOCK_INDEX(port)  (gh_sw_info.port_block_port_id[(port)])

#define SOC_PORT_P2L_MAPPING(port)  (gh_sw_info.port_p2l_mapping[(port)])
#define SOC_PORT_L2P_MAPPING(port)  (gh_sw_info.port_l2p_mapping[(port)])
#define SOC_PORT_COUNT(unit)        (gh_sw_info.port_count)

#define IS_XL_PORT(port) \
        (SOC_PORT_BLOCK_TYPE(port) == PORT_BLOCK_TYPE_XLPORT)

#define IS_GX_PORT(port) \
        (SOC_PORT_BLOCK_TYPE(port) == PORT_BLOCK_TYPE_GXPORT)

#define XLPORT_SUBPORT(port)        SOC_PORT_BLOCK_INDEX(port)

/* Physical port iteration */
#define SOC_PPORT_ITER(_p)       \
        for ((_p) = BCM5340X_PORT_MIN; \
             (_p) <= BCM5340X_PORT_MAX; \
             (_p)++) \
                if (gh_sw_info.port_p2l_mapping[(_p)] != -1)

/* Logical port iteration */
#define SOC_LPORT_ITER(_p)       \
        for ((_p) = BCM5340X_LPORT_MIN; \
             (_p) <= BCM5340X_LPORT_MAX; \
             (_p)++) \
                if ((gh_sw_info.port_l2p_mapping[(_p)] != -1))

        
#endif /* _BCM5340X_H_ */
