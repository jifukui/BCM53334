/*
 * $Id: bcm5333x.h,v 1.30 Broadcom SDK $
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

#ifndef _BCM5333X_H_
#define _BCM5333X_H_

/* Foxhound */
#define BCM53333_DEVICE_ID      0x8333
#define BCM53334_DEVICE_ID      0x8334
/* Deerhound*/
#define BCM53393_DEVICE_ID      0x8393
#define BCM53394_DEVICE_ID      0x8394
/* Wolfhound*/
#define BCM53342_DEVICE_ID      0x8342
#define BCM53343_DEVICE_ID      0x8343
#define BCM53344_DEVICE_ID      0x8344
#define BCM53346_DEVICE_ID      0x8346

/* BCM53394: Option 0-2
 * BCM53344: Option 0-2 
 * BCM53346: Option 0-6 
 */
#define CONFIG_OPTION  0

#define _DD_MAKEMASK1(n) (1 << (n))
#define _DD_MAKEMASK(v,n) ((((1)<<(v))-1) << (n))
#define _DD_MAKEVALUE(v,n) ((v) << (n))
#define _DD_GETVALUE(v,n,m) (((v) & (m)) >> (n))

/* Bit positions used for set/clear commands. */
#define S_SCTL_MSG_START         0
#define S_SCTL_MSG_DONE          1

#define M_SCTL_MSG_START         _DD_MAKEMASK1(S_SCTL_MSG_START)
#define M_SCTL_MSG_DONE          _DD_MAKEMASK1(S_SCTL_MSG_DONE)

#define S_SCTL_MIIM_OP_DONE       0

#define S_SCTL_MIIM_SCAN_BUSY     14
#define S_SCTL_MIIM_RD_START      16
#define S_SCTL_MIIM_WR_START      17

#define S_SCTL_MIIM_LINK_SCAN_EN  19
#define S_SCTL_MIIM_PAUS_ESCAN_EN 20

#define M_SCTL_BITVAL            _DD_MAKEMASK1(7)
#define V_SCTL_BIT_0             0
#define V_SCTL_BIT_1             M_SCTL_BITVAL

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

#define READCSR(x)    \
  (*(volatile uint32 *)(x))
#define WRITECSR(x,v) \
  (*(volatile uint32 *)(x) = (v))

#define S_SMHDR_DATA_LEN         7
#define M_SMHDR_DATA_LEN         _DD_MAKEMASK(7,S_SMHDR_DATA_LEN)
#define V_SMHDR_DATA_LEN(x)      _DD_MAKEVALUE(x,S_SMHDR_DATA_LEN)
#define G_SMHDR_DATA_LEN(x)      _DD_GETVALUE(x,S_SMHDR_DATA_LEN,M_SMHDR_DATA_LEN)

#define S_SMHDR_SRC_BLOCK        14
#define M_SMHDR_SRC_BLOCK        _DD_MAKEMASK(6,S_SMHDR_SRC_BLOCK)
#define V_SMHDR_SRC_BLOCK(x)     _DD_MAKEVALUE(x,S_SMHDR_SRC_BLOCK)
#define G_SMHDR_SRC_BLOCK(x)     _DD_GETVALUE(x,S_SMHDR_SRC_BLOCK,M_SMHDR_SRC_BLOCK)

#define S_SMHDR_DEST_BLOCK       20
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

#define SC_BLOCK_IPEPE           0x7
#define SC_BLOCK_CMIC            0xF
#define BLOCK_BP                 20     /* it is for XGS3 */

#define R_CMIC_SCHAN_D(word)     (CMIC_CMC1_SCHAN_MESSAGE0+4*(word))

#define M_PORT(idx)                 10,0x04000000+(idx)
#define M_IPORT(idx)                10,0x04040000+(idx)
#define M_VLAN(idx)                 10,0x14000000+(idx)
#define M_VLAN_PROFILE(idx)         10,0x14080000+(idx)
#define M_EGR_MASK(idx)             10,0x44000000+(idx)

#define M_EGR_VLAN(idx)             11,0x100C0000+(idx)

#define M_VLAN_STG(idx)             10,0x14040000+(idx)
#define M_EGR_VLAN_STG(idx)         11,0x10100000+(idx)

#define R_EGR_VLAN_CONTROL_1(idx)   11,0x04000B00+(idx)
#define R_EGR_CONFIG                11,0x06000400
#define R_EGR_CONFIG_1              11,0x06000500
#define M_EGR_PORT_REQUESTS(idx)     11,0x34040000+(idx)
#define M_EGR_PORT_CREDIT_RESET(idx) 11,0x340C0000+(idx)
#define M_EGR_ENABLE(idx)           11,0x345C0000+(idx)
#define R_AUX_ARB_CONTROL           10,0x02000000
#define R_AUX_ARB_CONTROL_2         10,0x02000100
#define M_SYSTEM_CONFIG_TABLE(idx)  10,0x04080000+(idx) /* Stage[31:26] = 1 */
#define R_ING_CONFIG_64             10,0x06000000
#define R_DOS_CONTROL               10,0x06000100
#define R_DOS_CONTROL2              10,0x06000200
#define R_VLAN_CTRL                 10,0x06000300
#define M_COS_MAP(idx)              10,0x44480000+(idx)

#define R_ICONTROL_OPCODE_BITMAP(idx) 10,0x38000500+(idx)
#define R_EGR_VLAN_CONTROL_3(idx)     11,0x14001700+(idx) 
#define M_EGR_VLAN_TAG_ACTION_PROFILE(idx)  11,0x14340000+(idx) 

/* CAMs */
#define R_VLAN_SUBNET_CAM_DBGCTRL     10,0x12010200
#define R_VFP_CAM_CONTROL_TM_7_THRU_0 10,0x12011A00
#define R_L2_USER_ENTRY_CAM_DBGCTRL   10,0x1E010000
#define R_SW2_RAM_CONTROL_4           10,0x46010700

/*
* For VT_DOT1Q, VLAN_DEFAULT controls the default VID and can be any value from 1~4094. 
* For VT_PORT_BASED, it keeps VID==1 as default.
*/
#define VLAN_DEFAULT                1
#define M_ING_VLAN_TAG_ACTION_PROFILE 10,0x102C0000
#define M_ING_PRI_CNG_MAP(idx)      10,0x381C0000+(idx)
#define R_VLAN_DEFAULT_PBM          10,0x16000000
#define R_L2_AUX_HASH_CONTROL       10,0x1E000100
#define R_L3_AUX_HASH_CONTROL       10,0x2A000000
#define R_SW2_FP_DST_ACTION_CONTROL 10,0x46003400
#define M_DSCP_TB(idx)              10,0x28000000+(idx)

#define R_EGR_PORT(idx)             11,0x04000900+(idx)
#define M_L2_USER_ENTRY(idx)        10,0x1c100000+(idx)

#define M_MODPORT_MAP(idx)          10,0x44140000+(idx)

/* Trunk related */
#define R_USER_TRUNK_HASH_SELECT(idx)  10,0x44006500+(idx)
#define R_IUSER_TRUNK_HASH_SELECT(idx) 10,0x44006600+(idx)
#define M_TRUNK_GROUP(idx)          10,0x44080000+(idx)
#define M_TRUNK_BITMAP(idx)         10,0x440C0000+(idx)
#define M_NONUCAST_TRUNK_BLOCK_MASK(idx) 10,0x44240000+(idx)
#define M_TRUNK32_CONFIG_TABLE(idx) 10,0x040c0000+(idx)
#define M_SOURCE_TRUNK_MAP(idx)     10,0x08000000+(idx)
#define M_TRUNK32_PORT_TABLE(idx)   10,0x08180000+(idx)
#define R_HG_TRUNK_BITMAP(idx)      10,0x46006900+(idx << 8)
#define R_HG_TRUNK_GROUP(idx)       10,0x46006b00+(idx << 8)
#define R_RTAG7_HASH_HG_TRUNK(idx)  10,0x46006f00+(idx << 8)
#define R_HIGIG_TRUNK_CONTROL       10,0x46002600

#define R_HASH_CONTROL              10,0x06000400

/* L2 age */
#define R_L2_AGE_TIMER               10,0x02000400

/* FP related */
#define R_FP_SLICE_ENABLE             10,0x3e000000
#define M_IFP_REDIRECTION_PROFILE(idx) 10,0x40040000+(idx)
#define M_FP_PORT_FIELD_SEL(idx)       10,0x38000000+(idx)
#define M_FP_SLICE_ENTRY_PORT_SEL(idx) X,0x0d710000+(idx)
#define M_FP_TCAM(idx)                 10,0x3C040000+(idx)
#define M_FP_SLICE_MAP                 10,0x3C080000
#define M_FP_POLICY_TABLE(idx)         10,0x3C0C0000+(idx)
#define M_FP_COUNTER_TABLE(idx)        10,0x3C140000+(idx)
#define M_FP_GLOBAL_MASK_TCAM(idx)     10,0x3C180000+(idx)
#define M_FP_UDF_OFFSET(idx)           10,0x08040000+(idx)

/* Mirror related */
#define R_MIRROR_CONTROL(idx)          10,0x44000600+(idx)
#define R_EMIRROR_CONTROL_64(idx)      10,0x44000800+(idx)
#define M_IM_MTP_INDEX(idx)            10,0x442c0000+(idx)
#define M_EM_MTP_INDEX(idx)            10,0x44300000+(idx)

#define M_EGR_IM_MTP_INDEX(idx)        11,0x14240000+(idx)
#define M_EGR_EM_MTP_INDEX(idx)        11,0x14280000+(idx)


/* rate related */
#define R_EGRMETERINGCONFIG(idx)           12,0x00015600+(idx)
#define R_EGRMETERINGBUCKET(idx)           12,0x00015700+(idx)

#define R_STORM_CONTROL_METER_CONFIG(idx)  10,0x38000200+(idx)
#define M_FP_METER_TABLE(idx)              10,0x3c100000+(idx)
#define M_FP_STORM_CONTROL_METERS(idx)     10,0x3c1c0000+(idx)

#define M_FP_SC_BCAST_METER_TABLE(idx)     X,0x0D780000+(idx)
#define M_FP_SC_MCAST_METER_TABLE          X,0x0D780019
#define M_FP_SC_DLF_METER_TABLE            X,0x0D780032

/* Counter related */

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
#define R_TFCS(idx)                     0x00005300+(idx)
/*  Alignment Error Frame Counter (receive only) */
#define R_RALN(idx)                     0x00001600+(idx)
/* Oversized Frame Counter */
#define R_ROVR(idx)                     0x00001A00+(idx)
#define R_TOVR(idx)                     0x00005500+(idx)

/* IPIPE HOLD Drop Counter */
#define R_HOLD_COS(idx)                  10,0x44005100+(idx)
#define R_HOLD_COS_PORT_SELECT           10,0x44006800
#define R_HOLD_COS0                      10,0x44005B00 
#define R_HOLD_COS1                      10,0x44005C00 
#define R_HOLD_COS2                      10,0x44005D00 
#define R_HOLD_COS3                      10,0x44005E00 
#define R_HOLD_COS4                      10,0x44005F00 
#define R_HOLD_COS5                      10,0x4406000 
#define R_HOLD_COS6                      10,0x44006100 
#define R_HOLD_COS7                      10,0x44006200 


/* mcast related*/
#define R_IGMP_MLD_PKT_CONTROL(idx)     10,0x1c000500+(idx)
#define M_L2_ENTRY(idx)                 10,0x1c000000+(idx)
#define M_L2MC(idx)                     10,0x24000000+(idx)
#define L2_ENTRY_SIZE              16384
#define L3_IPMC(idx)                    10,0x2c000000+(idx)


#define R_UNKNOWN_MCAST_BLOCK_MASK_64(idx)   10,0x44000200+(idx)
#define R_IUNKNOWN_MCAST_BLOCK_MASK_64(idx)  10,0x44000300+(idx)

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

#define CMIC_CMC1_CH0_DMA_CTRL   0x48032140
#define CMIC_CMC1_CH1_DMA_CTRL   0x48032144
#define CMIC_CMC1_CHx_DMA_CTRL(ch) (0x48032140+4*(ch))
#define CMIC_CMC1_DMA_STAT       0x48032150
#define CMIC_CMC1_DMA_DESC0      0x48032158
#define CMIC_CMC1_DMA_DESC(ch)   (CMIC_CMC1_DMA_DESC0+4*(ch))   /* 0 <= ch <= 3 */

#define CMIC_CMC1_CH1_COS_CTRL_RX_0 0x48032170
#define CMIC_CMC1_DMA_STAT_CLR      0x480321a4

#define CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITS 0x4801a000

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
#define R_CNG0COSDROPRATE(cos, idx)  12,0x0001dd00+((cos) << 8)+(idx)
#define R_CNG1COSDROPRATE(cos, idx)  12,0x0001fd00+((cos) << 8)+(idx)

#define R_LOG_TO_PHY_PORT_MAPPING(idx) 12,0x00086000+(idx)

/* Stage [31:26] = 13 */
#define R_EGR_LOGICAL_TO_PHYSICAL_PORT_NUMBER_MAPPING(idx)  11,0x34001B00+(idx)

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
#define R_HOLCOSPKTRESETLIMIT_COS0(idx) 12,0x00002800+(idx)
#define R_HOLCOSPKTRESETLIMIT_COS1(idx) 12,0x00002900+(idx)
#define R_HOLCOSPKTRESETLIMIT_COS2(idx) 12,0x00002a00+(idx)
#define R_HOLCOSPKTRESETLIMIT_COS3(idx) 12,0x00002b00+(idx)

#define R_HOLCOSPKTSETLIMIT(cos, idx)   0x00000800+((cos) << 8)+(idx)
#define R_HOLCOSPKTSETLIMIT_COS0(idx)   12,0x00000800+(idx)
#define R_HOLCOSPKTSETLIMIT_COS1(idx)   12,0x00000900+(idx)
#define R_HOLCOSPKTSETLIMIT_COS2(idx)   12,0x00000a00+(idx)
#define R_HOLCOSPKTSETLIMIT_COS3(idx)   12,0x00000b00+(idx)

#define R_LWMCOSCELLSETLIMIT(cos, idx)  0x0000cd00+((cos) << 8)+(idx)
#define R_LWMCOSCELLSETLIMIT_COS0(idx)  12,0x0000cd00+(idx)
#define R_LWMCOSCELLSETLIMIT_COS1(idx)  12,0x0000ce00+(idx)
#define R_LWMCOSCELLSETLIMIT_COS2(idx)  12,0x0000cf00+(idx)
#define R_LWMCOSCELLSETLIMIT_COS3(idx)  12,0x0000d000+(idx)

#define R_DYNCELLLIMIT(idx)             12,0x00012d00+(idx)

#define R_HOLCOSCELLMAXLIMIT(cos, idx)  0x0000ed00+((cos) << 8)+(idx)
#define R_HOLCOSCELLMAXLIMIT_COS0(idx)  12,0x0000ed00
#define R_HOLCOSCELLMAXLIMIT_COS1(idx)  12,0x0000ee00
#define R_HOLCOSCELLMAXLIMIT_COS2(idx)  12,0x0000ef00
#define R_HOLCOSCELLMAXLIMIT_COS3(idx)  12,0x0000f000

#define R_HOLCOSMINXQCNT(cos, idx)      0x00004800+((cos) << 8)+(idx)
#define R_HOLCOS0MINXQCNT(idx)          12,0x00004800+(idx)
#define R_HOLCOS1MINXQCNT(idx)          12,0x00004900+(idx)
#define R_HOLCOS2MINXQCNT(idx)          12,0x00004a00+(idx)
#define R_HOLCOS3MINXQCNT(idx)          12,0x00004b00+(idx)

#define R_DYNXQCNTPORT(idx)             12,0x0000ca00+(idx)

#define R_TOTALDYNCELLSETLIMIT          12,0x02025B00
#define R_TOTALDYNCELLRESETLIMIT        12,0x02030400

#define R_MISCCONFIG                    12,0x02025A00

#define R_IBPPKTSETLIMIT(idx)           12,0x00000000+(idx)
#define R_IBPCELLSETLIMIT(idx)          12,0x00000200+(idx)
#define R_IBPDISCARDSETLIMIT(idx)       12,0x00000300+(idx)

/* HOLCOSPKTSETLIMIT = 1496, HOLCOSPKTRESETLIMIT = 1492 */
#define MMU_R_HOLCOSPKTSETLIMIT     0x5d8
#define MMU_R_HOLCOSPKTRESETLIMIT   0x5d4

/* RESETLIMITSEL = 16, LWMCOSCELLSETLIMIT.RESETLIMITSEL = 16 */
#define MMU_R_LWMCOSCELLSETLIMIT    0x40010

/* DYNCELLRESETLIMISEL = 2184, DYNCELLLIMIT = 2216 */
#define MMU_R_DYNCELLLIMIT          0x22208a8

#define MMU_R_HOLCOSMINXQCNT        0x8

#define MMU_R_DYNXQCNTPORT          0x5d0

#define MMU_R_IBPPKTSETLIMIT        0xc
#define MMU_R_IBPCELLSETLIMIT       0x28
#define MMU_R_IBPDISCARDSETLIMIT    0xFFF

/* COS1: CELLMAXLIMIT = 2024, RESUMELIMIT = 2008 */
#define MMU_R_HOLCOSCELLMAXLIMIT_COS1   0x1f607e8
/* COS0/COS2/COS3: CELLMAXLIMIT = 64, RESUMELIMIT = 48 */
#define MMU_R_HOLCOSCELLMAXLIMIT_COS0   0xc040
#define MMU_R_HOLCOSCELLMAXLIMIT_COS2   0xc040
#define MMU_R_HOLCOSCELLMAXLIMIT_COS3   0xc040

/* TOTALDYNCELLLIMIT = 2496, TOTALDYNCELLRESETLIMITSEL = 2464 */
#define MMU_R_TOTALDYNCELLSETLIMIT      0x9c0
#define MMU_R_TOTALDYNCELLRESETLIMIT    0x9a0

/* End of MMU setting */

#define PORT_STATUS_FLAGS_GIGA_SPEED    (1 << 0)
#define PORT_STATUS_FLAGS_GREEN_MODE    (1 << 1)
#define PORT_STATUS_FLAGS_TX_PAUSE      (1 << 2)
#define PORT_STATUS_FLAGS_RX_PAUSE      (1 << 3)
#define PORT_STATUS_FLAGS_REMOTE_EEE    (1 << 4)

/* EEE related */
/* MAC register*/
#define R_UMAC_EEE_CTRL(idx)                  0x00011900+(idx)
#define R_XLMAC_EEE_CTRL(idx)                 0x00061A00+(idx)
#define R_MII_EEE_WAKE_TIMER(idx)             0x00012000+(idx)
#define R_GMII_EEE_WAKE_TIMER(idx)            0x00012100+(idx)
#define R_XLMAC_EEE_TIMERS(idx)               0x00012100+(idx)
#define R_MII_EEE_DELAY_ENTRY_TIMER(idx)      0x00011A00+(idx)
#define R_GMII_EEE_DELAY_ENTRY_TIMER(idx)     0x00011B00+(idx)
#define R_UMAC_EEE_REF_COUNT(idx)             0x00011C00+(idx)

/* EEE parameters */
#define BCM5333X_PHY_EEE_100MB_MIN_WAKE_TIME    0x1E  /* 30 micro seconds */
#define BCM5333X_PHY_EEE_1GB_MIN_WAKE_TIME      0x11  /* 17 micro seconds */

#define CMIC_SBUS_TIMEOUT                        0x48010094

#define CMIC_SBUS_RING_MAP_0_7                   0x48010098
#define CMIC_SBUS_RING_MAP_8_15                  0x4801009c
#define CMIC_SBUS_RING_MAP_16_23                 0x480100a0
#define CMIC_SBUS_RING_MAP_24_31                 0x480100a4

#define CMIC_CPS_RESET                           0x48010220

#define CMIC_RATE_ADJUST_EXT_MDIO                0x48011000
#define CMIC_RATE_ADJUST_INT_MDIO                0x48011004

#define CMIC_DEV_REV_ID                          0x48010224

#define CMIC_LEDUP0_CTRL                         0x48020000
#define CMIC_LEDUP0_PORT_ORDER_REMAP_0_3         0x48020010
#define CMIC_LEDUP0_PORT_ORDER_REMAP_60_63       0x4802004c

#define CMIC_LEDUP0_DATA_RAM0                    0x48020400
#define CMIC_LEDUP0_PROGRAM_RAM0                 0x48020800

#define CMIC_LEDUP_PROGRAM_RAM_D(word) \
        (CMIC_LEDUP0_PROGRAM_RAM0+(4*(word)))

#define CMIC_LEDUP_DATA_RAM_D(word) \
        (CMIC_LEDUP0_DATA_RAM0+(4*(word)))
#define LED_PORT_STATUS_OFFSET(p)   CMIC_LEDUP_DATA_RAM_D(0xa0 + (p))

#define R_XLPORT_LED_CHAIN_CONFIG                5,0x02022b00

#define CMIC_CMC1_SCHAN_CTRL                     0x48032000
#define CMIC_CMC1_SCHAN_ACK_DATA_BEAT_COUNT      0x48032004
#define CMIC_CMC1_SCHAN_ERR                      0x48032008
#define CMIC_CMC1_SCHAN_MESSAGE0                 0x4803200c

#define CMIC_CMC1_MIIM_PARAM                     0x48032080
#define CMIC_CMC1_MIIM_READ_DATA                 0x48032084
#define CMIC_CMC1_MIIM_ADDRESS                   0x48032088
#define CMIC_CMC1_MIIM_CTRL                      0x4803208c
#define CMIC_CMC1_MIIM_STAT                      0x48032090

#define R_TX_IPG_LENGTH(idx)                     0x00011700+(idx)

#define R_COMMAND_CONFIG(idx)                    0x00010200+(idx)


#define R_MAC_0(idx)                             0x00010300+(idx)
#define R_MAC_1(idx)                             0x00010400+(idx)
#define R_FRM_LENGTH(idx)                        0x00010500+(idx)
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
#define R_XLMAC_TXFIFO_CELL_CNT(idx)             0x00062B00+(idx)

#define R_XLMAC_RX_MAX_SIZE(idx)                  0x00060800+(idx)


#define R_GPORT_CONFIG                           0x02020000

#define R_GPORT_LINK_STATUS_TO_CMIC              0x02020500
#define R_GPORT_RSV_MASK                         0x02020600
#define R_GPORT_STAT_UPDATE_MASK                 0x02020700
#define R_XLPORT_POWER_SAVE                      0x02020D00
#define R_XLPORT_CONFIG(idx)                     0x00020000+(idx)
#define R_XLPORT_CNTMAXSIZE                      0x00020100
#define R_XLPORT_MAC_RSV_MASK(idx)               0x00020800+(idx)
#define R_XLPORT_MODE_REG                        0x02020A00
#define R_XLPORT_ENABLE_REG                      0x02020B00
#define R_XLPORT_SOFT_RESET                      0x02020C00
#define R_XLPORT_POWER_SAVE                      0x02020D00
#define R_XLPORT_MAC_CONTROL                     0x02021000
#define R_XLPORT_XGXS0_CTRL_REG                  0x02021400
#define R_XLPORT_WC_UCMEM_CTRL                   0x02021900
#define M_XLPORT_WC_UCMEM_DATA(idx)              0x00000000+(idx)
#define R_XLPORT_MIB_RESET                       0x02022c00
#define R_XLPORT_TXFIFO_CTRL(idx)                0x00020300+(idx)

#define R_XLPORT_MAC_RSV_MASK(idx)               0x00020800+(idx)
#define R_ING_HW_RESET_CONTROL_1                 10,0x02000200
#define R_ING_HW_RESET_CONTROL_2                 10,0x02000300
#define R_IP_TO_CMICM_CREDIT_TRANSFER            10,0x02002300
#define R_EPC_LINK_BMAP_64                       10,0x46002700
#define R_ING_MISC_CONFIG                        10,0x46002E00
#define R_PROTOCOL_PKT_CONTROL(idx)              10,0x1C000400+(idx)
#define R_UNKNOWN_UCAST_BLOCK_MASK_64(idx)       10,0x44000000+(idx)
#define R_IUNKNOWN_UCAST_BLOCK_MASK_64(idx)      10,0x44000100+(idx)

#define R_ING_EGRMSKBMAP_64(idx)                 10,0x44000E00+(idx)
#define R_IING_EGRMSKBMAP_64(idx)                10,0x44000F00+(idx)

#define R_EGR_HW_RESET_CONTROL_0                 11,0x02000000
#define R_EGR_HW_RESET_CONTROL_1                 11,0x02000100
#define R_EGR_PORT_BUFFER_CLK_SHUTDOWN           11,0x36011900

#define R_TOP_SOFT_RESET_REG                     16,0x02030400
#define R_TOP_SOFT_RESET_REG_2                   16,0x02030800
#define R_TOP_MISC_STATUS                        16,0x02031800
#define R_TOP_CLOCKING_ENFORCE_PSG               16,0x02032400
#define R_TOP_PARALLEL_LED_CTRL                  16,0x02035000
#define R_TOP_CORE_PLL_CTRL2                     16,0x02038800
#define R_TOP_CORE_PLL_CTRL4                     16,0x02039000
#define R_TOP_TOP_CORE_PLL_STATUS_1              16,0x02040800
#define R_TOP_XG_PLL0_CTRL_0                     16,0x02042000
#define R_TOP_XG_PLL0_CTRL_1                     16,0x02042400
#define R_TOP_XG_PLL0_CTRL_2                     16,0x02042800
#define R_TOP_XG_PLL0_CTRL_3                     16,0x02042C00
#define R_TOP_XG_PLL0_CTRL_4                     16,0x02043000
#define R_TOP_XG_PLL0_STATUS                     16,0x02043400
#define R_TOP_PVTMON_CTRL_0                      16,0x02050000
#define R_TOP_PVTMON_CTRL_1                      16,0x02050400

#define R_TOP_XG_PLL1_CTRL_0                     16,0x02043800
#define R_TOP_XG_PLL1_CTRL_1                     16,0x02043C00
#define R_TOP_XG_PLL1_CTRL_2                     16,0x02044000
#define R_TOP_XG_PLL1_CTRL_3                     16,0x02044400
#define R_TOP_XG_PLL1_CTRL_4                     16,0x02044800
#define R_TOP_XG_PLL1_STATUS                     16,0x02044C00

#define R_TOP_BS_PLL0_CTRL_0                     16,0x0204B000
#define R_TOP_BS_PLL0_CTRL_1                     16,0x0204B400
#define R_TOP_BS_PLL0_CTRL_2                     16,0x0204B800
#define R_TOP_BS_PLL0_CTRL_3                     16,0x0204BC00
#define R_TOP_BS_PLL0_CTRL_4                     16,0x0204C000
#define R_TOP_BS_PLL0_STATUS                     16,0x0204C400

#define R_TOP_BS_PLL1_CTRL_0                     16,0x02078000
#define R_TOP_BS_PLL1_CTRL_1                     16,0x02078400
#define R_TOP_BS_PLL1_CTRL_2                     16,0x02078800
#define R_TOP_BS_PLL1_CTRL_3                     16,0x02078C00
#define R_TOP_BS_PLL1_CTRL_4                     16,0x02079000
#define R_TOP_BS_PLL1_STATUS                     16,0x02079400

#define R_TOP_MISC_CONTROL_1                     16,0x02031400
#define R_TOP_MISC_CONTROL_2                     16,0x02079800

#define CCB_MII_MGMT_CTRL                        0x18032000
#define MII_MGMT_CTRL_MDCDIV_MASK                0x7F
#define MII_MGMT_CTRL_PRE                        0x80
#define MII_MGMT_CTRL_BUSY                       0x100
#define MII_MGMT_CTRL_EXT                        0x200

#define CCB_MII_MGMT_CMD_DATA                    0x18032004
#define MII_CMD_DATA_DATA_SHIFT                  0
#define MII_CMD_DATA_TA_SHIFT                    10
#define MII_CMD_DATA_RA_SHIFT                    18
#define MII_CMD_DATA_PA_SHIFT                    23
#define MII_CMD_DATA_OP_SHIFT                    28
#define MII_CMD_DATA_SB_SHIFT                    30

#ifdef CFG_RESET_BUTTON_INCLUDED
#define CMIC_GP_DATA_IN                          0x48002000
#define CMIC_GP_DATA_OUT                         0x48002004
#define CMIC_GP_OUT_EN                           0x48002008

#define CCA_GPIOINPUT                            0x18000060
#define CCA_GPIOOUT                              0x18000064
#define CCA_GPIOOUTEN                            0x18000068
#endif /* CFG_RESET_BUTTON_INCLUDED */

/* EEE functions */
#ifdef CFG_SWITCH_EEE_INCLUDED
extern void bcm5333x_port_eee_enable_get(uint8 unit, uint8 port, uint8 *enable);
extern sys_error_t bcm5333x_port_eee_enable_set(uint8 unit,
                      uint8 port, uint8 enable, uint8 save);
extern sys_error_t bcm5333x_port_eee_tx_wake_time_set(uint8 unit,
                      uint8 port, uint8 type, uint16 wake_time);
extern void bcm5333x_eee_init(void);
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
#define PHY_FIRST_QSGMII_PORT                   18

#define PHY_FIRST_QGPHY_PORT0                    2
#define PHY_SECOND_QGPHY_PORT0                  10

/* physical port */
#define BCM5333X_PORT_MIN                        2
#define BCM5333X_PORT_MAX                       33
/* logical port */
#define BCM5333X_LPORT_MIN                        2
#define BCM5333X_LPORT_MAX                       29


#define GXPORT0_BLOCK_ID                         2
#define GXPORT1_BLOCK_ID                         3
#define GXPORT2_BLOCK_ID                         4
#define XLPORT0_BLOCK_ID                         5
#define XLPORT1_BLOCK_ID                         6

#define PHY_GXPORT0_BASE                          2
#define PHY_GXPORT1_BASE                         10 /* port 10 - 17 */
#define PHY_GXPORT2_BASE                         18 /* port 18 - 25 */
#define PHY_XLPORT0_BASE                         26 /* port 26 - 29 */
#define PHY_XLPORT1_BASE                         30 /* port 30 - 33 */

typedef enum port_block_type_s {
    PORT_BLOCK_TYPE_XLPORT,
    PORT_BLOCK_TYPE_GXPORT,
    PORT_BLOCK_TYPE_COUNT
} port_block_type_t;

enum soc_hu2_port_mode_e {
    /* WARNING: values given match hardware register; do not modify */
    SOC_HU2_PORT_MODE_QUAD = 0,
    SOC_HU2_PORT_MODE_TRI_012 = 1,
    SOC_HU2_PORT_MODE_TRI_023 = 2,
    SOC_HU2_PORT_MODE_DUAL = 3,
    SOC_HU2_PORT_MODE_SINGLE = 4
};

typedef struct mac_driver_s {
    char         *drv_name;
    sys_error_t  (*md_init)(uint8, uint8);
    sys_error_t  (*md_enable_set)(uint8, uint8, BOOL);
    sys_error_t  (*md_duplex_set)(uint8, uint8, BOOL);
    sys_error_t  (*md_speed_set)(uint8, uint8, int);
    sys_error_t  (*md_pause_set)(uint8, uint8, BOOL, BOOL);
    sys_error_t  (*md_lb_set)(uint8, uint8, BOOL);
#if 0    
    sys_error_t  (*md_ifg_set)(uint8, uint8, BOOL, int);
#endif
    sys_error_t  (*md_frame_max_set)(uint8, uint8, uint32);
    sys_error_t  (*md_frame_max_get)(uint8, uint8, uint32*);
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
  
#define MAC_MTU_SET(_md, _u, _p, _s) \
        _MAC_CALL((_md), md_frame_max_set, ((_u), (_p), (_s)))

#define MAC_MTU_GET(_md, _u, _p, _s) \
        _MAC_CALL((_md), md_frame_max_get, ((_u), (_p), (_s)))

extern mac_driver_t soc_mac_xl, soc_mac_uni;

typedef struct
{
    uint16  devid;
    uint16  revid;
    uint8   link[BCM5333X_LPORT_MAX + 1];           /* link status */
    uint8   loopback[BCM5333X_LPORT_MAX + 1];       /* loopback mode */
    uint8   port_count;
    uint32  pbmp;
#if defined(CFG_SWITCH_EEE_INCLUDED)
    tick_t link_up_time[BCM5333X_LPORT_MAX+1];
    uint8 need_process_for_eee_1s[BCM5333X_LPORT_MAX+1];
#endif /*  CFG_SWITCH_EEE_INCLUDED */
    int    port_p2l_mapping[BCM5333X_PORT_MAX + 1];   /* phy to logic */
    int    port_l2p_mapping[BCM5333X_LPORT_MAX + 1];  /* logic to phy */
    int    port_speed_max[BCM5333X_LPORT_MAX + 1];    /* logical port */
    int    port_block_id[BCM5333X_LPORT_MAX + 1];     /* logical port */
    int    port_block_port_id[BCM5333X_LPORT_MAX + 1];/* logical port */
    int    port_block_type[BCM5333X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_tx_pause_status[BCM5333X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_rx_pause_status[BCM5333X_LPORT_MAX + 1];   /* logical port */    
    mac_driver_t *p_mac[BCM5333X_LPORT_MAX + 1];      /* mac driver */
} bcm5333x_sw_info_t;

extern bcm5333x_sw_info_t hr2_sw_info;

#define COS_QUEUE_NUM                  (4)

#define SOC_IS_FOXHOUND(unit)       ((hr2_sw_info.devid == BCM53333_DEVICE_ID) || \
                                     (hr2_sw_info.devid == BCM53334_DEVICE_ID))

#define SOC_IS_WOLFHOUND(unit)      ((hr2_sw_info.devid == BCM53344_DEVICE_ID) || \
                                     (hr2_sw_info.devid == BCM53346_DEVICE_ID))

#define SOC_IS_DEERHOUND(unit)      ((hr2_sw_info.devid == BCM53393_DEVICE_ID) || \
                                     (hr2_sw_info.devid == BCM53394_DEVICE_ID))


/* Mask of all logical ports (2 ~ 29) */
#define BCM5333x_ALL_PORTS_MASK     (hr2_sw_info.pbmp)
/*
 * Macros to get the device block type, block index and index
 * within block for a given port
 */
#define SOC_PORT_BLOCK_TYPE(port)   (hr2_sw_info.port_block_type[(port)])
#define SOC_PORT_BLOCK(port)        (hr2_sw_info.port_block_id[(port)])
#define SOC_PORT_BLOCK_INDEX(port)  (hr2_sw_info.port_block_port_id[(port)])

#define SOC_PORT_P2L_MAPPING(port)  (hr2_sw_info.port_p2l_mapping[(port)])
#define SOC_PORT_L2P_MAPPING(port)  (hr2_sw_info.port_l2p_mapping[(port)])
#define SOC_PORT_SPEED_MAX(port)    (hr2_sw_info.port_speed_max[(port)])
#define SOC_PORT_COUNT(unit)        (hr2_sw_info.port_count)

#define IS_XL_PORT(port) \
        (SOC_PORT_BLOCK_TYPE(port) == PORT_BLOCK_TYPE_XLPORT)

#define IS_GX_PORT(port) \
        (SOC_PORT_BLOCK_TYPE(port) == PORT_BLOCK_TYPE_GXPORT)

#define IS_HG_PORT(port) \
        (SOC_PORT_SPEED_MAX(port) == 13)

#define SOC_PORT_TX_PAUSE_STATUS(port)   (hr2_sw_info.port_tx_pause_status[(port)])
#define SOC_PORT_RX_PAUSE_STATUS(port)   (hr2_sw_info.port_rx_pause_status[(port)])           
/* SOC interface */
extern sys_error_t bcm5333x_chip_revision(uint8 unit, uint16 *dev, uint16 *rev);
extern uint8 bcm5333x_port_count(uint8 unit);
extern sys_error_t bcm5333x_reg_get(uint8 unit, uint8 block_id, uint32 addr, uint32 *val);
extern sys_error_t bcm5333x_reg_set(uint8 unit, uint8 block_id, uint32 addr, uint32 val);
extern sys_error_t bcm5333x_reg64_get(uint8 unit, uint8 block_id, uint32 addr, uint32 *val, int len);
extern sys_error_t bcm5333x_reg64_set(uint8 unit, uint8 block_id, uint32 addr, uint32 *buf, int len);
extern sys_error_t bcm5333x_mem_get(uint8 unit, uint8 block_id, uint32 addr, uint32 *buf, int len);
extern sys_error_t bcm5333x_mem_set(uint8 unit, uint8 block_id, uint32 addr, uint32 *buf, int len);
extern sys_error_t bcm5333x_l2_op(uint8 unit, l2x_entry_t *entry, uint8 op);
extern sys_error_t bcm5333x_rx_set_handler(uint8 unit, SOC_RX_HANDLER fn,
                                           BOOL intr);
extern sys_error_t bcm5333x_rx_fill_buffer(uint8 unit,
                                           soc_rx_packet_t *pkt);
extern sys_error_t bcm5333x_tx(uint8 unit, soc_tx_packet_t *pkt);
extern sys_error_t bcm5333x_link_status(uint8 unit, uint8 port, BOOL *link);
extern void bcm5333x_rxtx_stop(void);

/* FP_TCAM encode/decode utility */
extern void bcm5333x_dm_to_xy(uint32 *dm_entry, uint32 *xy_entry, int data_words, int bit_length);
extern void bcm5333x_xy_to_dm(uint32 *xy_entry, uint32 *dm_entry, int data_words, int bit_length);

#ifdef CFG_SWITCH_VLAN_INCLUDED
extern sys_error_t bcm5333x_pvlan_egress_set(uint8 unit,
                    uint8 pport,
                    pbmp_t pbmp);
extern sys_error_t bcm5333x_pvlan_egress_get(uint8 unit,
                    uint8 pport,
                    pbmp_t *pbmp);
extern sys_error_t bcm5333x_qvlan_port_set(uint8 unit,
                    uint16 vlan_id,
                    pbmp_t pbmp,
                    pbmp_t tag_pbmp);
extern sys_error_t bcm5333x_qvlan_port_get(uint8 unit,
                    uint16 vlan_id,
                    pbmp_t *pbmp,
                    pbmp_t *tag_pbmp);
extern sys_error_t bcm5333x_vlan_create(uint8 unit,
                    vlan_type_t type, uint16 vlan_id);
extern sys_error_t bcm5333x_vlan_destroy(uint8 unit, uint16 vlan_id);
extern sys_error_t bcm5333x_vlan_type_set(uint8 unit, vlan_type_t type);
extern sys_error_t bcm5333x_vlan_reset(uint8 unit);
extern sys_error_t bcm5333x_vlan_l3_ipmc_set(uint8 unit, uint16 vlan_id,
	                  uint16 uc_idx, uint16 umc_idx);
extern sys_error_t bcm5333x_l3_ipmc_set(uint8 unit, uint16 idx, pbmp_t lpbmp);
extern sys_error_t bcm5333x_vlan_l3_ipmc_del(uint8 unit, uint16 vlan_id);
extern sys_error_t bcm5333x_l3_ipmc_del(uint8 unit, uint16 id);
extern sys_error_t bcm5333x_l3_ipmc_get(uint8 unit, uint16 idx, pbmp_t *lpbmp);
extern sys_error_t bcm5333x_vlan_l3_ipmc_get(uint8 unit, uint16 vlan_id, uint16 *uc_idx, uint16 *umc_idx);


#endif /* CFG_SWITCH_VLAN_INCLUDED */


#ifdef CFG_SWITCH_LAG_INCLUDED
extern sys_error_t bcm5333x_lag_group_set(uint8 unit, uint8 lagid, pbmp_t pbmp);
extern void bcm5333x_lag_group_get(uint8 unit, uint8 lagid, pbmp_t *pbmp);
#endif /* CFG_SWITCH_LAG_INCLUDED */

extern void bcm5333x_loop_detect_enable(BOOL enable);
extern uint8 bcm5333x_loop_detect_status_get(void);

#ifdef CFG_ZEROCONF_MDNS_INCLUDED
sys_error_t bcm5333x_mdns_enable_set(uint8 unit, BOOL mdns_enable);
sys_error_t bcm5333x_mdns_enable_get(uint8 unit, BOOL *mdns_enable);
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

/* Initialization */
extern sys_error_t bcm5333x_sw_init(void);
extern void bcm5333x_dscp_map_enable(BOOL enable);
extern void bcm5333x_qos_init(void);
extern void bcm5333x_rate_init(void);
extern void bcm5333x_loop_detect_init(void);
extern void bcm5333x_rxtx_init(void);
extern void bcm5333x_loopback_enable(uint8 unit, uint8 lport, int loopback_mode);

extern uint8 lport_active[BCM5333X_LPORT_MAX + 1];

/* Physical port iteration */
#define SOC_PPORT_ITER(_p)       \
        for ((_p) = BCM5333X_PORT_MIN; \
             (_p) <= BCM5333X_PORT_MAX; \
             (_p)++) \
                if (hr2_sw_info.port_p2l_mapping[(_p)] != -1)

/* Logical port iteration */
#define SOC_LPORT_ITER(_p)       \
        for ((_p) = BCM5333X_LPORT_MIN; \
             (_p) <= BCM5333X_LPORT_MAX; \
             (_p)++) \
                if ((hr2_sw_info.port_l2p_mapping[(_p)] != -1) && (lport_active[(_p)]))

#define SOC_XLPORT_ITER(_p)       \
        for ((_p) = BCM5333X_LPORT_MIN; \
             (_p) <= BCM5333X_LPORT_MAX; \
             (_p)++) \
                if ((hr2_sw_info.port_l2p_mapping[(_p)] != -1) && \
                     IS_XL_PORT((_p)) && (lport_active[(_p)]))

#define SOC_HGPORT_ITER(_p)       \
        for ((_p) = BCM5333X_LPORT_MIN; \
             (_p) <= BCM5333X_LPORT_MAX; \
             (_p)++) \
                if ((hr2_sw_info.port_l2p_mapping[(_p)] != -1) && \
                     IS_HG_PORT((_p)) && (lport_active[(_p)]))


extern void
bcm5333x_port_disacrd_mod_get(uint8 unit, uint8 lport,
                        uint32* disard);
extern void
bcm5333x_port_stg_state_get(uint8 unit, uint8 lport,
                        uint32* stg);

#endif /* _BCM5333x_H_ */

