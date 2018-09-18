/*
 * $Id: bcm5346x.h,v 1.32.2.1 Broadcom SDK $
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

#ifndef _BCM5346X_H_
#define _BCM5346X_H_

#include "auto_generated/bcm5346x_defs.h"
/* Chip revision ID */
#define BCM56270_DEVICE_ID   0xb270
#define BCM56271_DEVICE_ID   0xb271
#define BCM56272_DEVICE_ID   0xb272
#define BCM53460_DEVICE_ID   0x8460   /* confirm for metrolite */
#define BCM53461_DEVICE_ID   0x8461   /* confirm for metrolite */


#define _DD_MAKEMASK1(n) (1 << (n))
#define _DD_MAKEMASK(v,n) ((((1)<<(v))-1) << (n))
#define _DD_MAKEVALUE(v,n) ((v) << (n))
#define _DD_GETVALUE(v,n,m) (((v) & (m)) >> (n))

#ifndef __LINUX__
#define READCSR(x)   SYS_REG_READ32(x)
#define WRITECSR(x,v) SYS_REG_WRITE32(x,v)
#endif

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

/* SCHAN operation */
#define SC_OP_RD_MEM_CMD         0x07
#define SC_OP_RD_MEM_ACK         0x08
#define SC_OP_WR_MEM_CMD         0x09
#define SC_OP_RD_REG_CMD         0x0B
#define SC_OP_RD_REG_ACK         0x0C
#define SC_OP_WR_REG_CMD         0x0D
#define SC_OP_L2_INS_CMD         0x0F
#define SC_OP_L2_DEL_CMD         0x11

/* CMIC_CMCx_SCHAN_CTRL(x) */
#define SC_CMCx_MSG_START               (0x00000001)     /* confirm for metrolite */
#define SC_CMCx_MSG_DONE                (0x00000002)     /* confirm for metrolite */
#define SC_CMCx_SCHAN_ABORT             (0x00000004)     /* confirm for metrolite */
#define SC_CMCx_MSG_SER_CHECK_FAIL      (0x00100000)     /* confirm for metrolite */
#define SC_CMCx_MSG_NAK                 (0x00200000)     /* confirm for metrolite */
#define SC_CMCx_MSG_TIMEOUT_TST         (0x00400000)     /* confirm for metrolite */
#define SC_CMCx_MSG_SCHAN_ERROR         (0x00800000)     /* confirm for metrolite */
#define SC_CMCx_MSG_ERROR_MASK          (0x00F00000)     /* confirm for metrolite */

/*  CMIC_CMCx_MIIM_STAT(x) */
#define CMIC_MIIM_OPN_DONE              (0x00000001)     /* confirm for metrolite */

/*  CMIC_CMCx_MIIM_CTRL(x) */
#define CMIC_MIIM_WR_START              (0x00000001)     /* confirm for metrolite */
#define CMIC_MIIM_RD_START              (0x00000002)     /* confirm for metrolite */


/* CMIC_CMCx_CHy_DMA_CTRL */
#define PKTDMA_DIRECTION                (0x00000001)  /* confirm for metrolite */
#define PKTDMA_ENABLE                   (0x00000002)  /* confirm for metrolite */
#define PKTDMA_ABORT                    (0x00000004)  /* confirm for metrolite */
#define PKTDMA_SEL_INTR_ON_DESC_OR_PKT  (0x00000008)  /* confirm for metrolite */
#define PKTDMA_BIG_ENDIAN               (0x00000010)  /* confirm for metrolite */
#define PKTDMA_DESC_BIG_ENDIAN          (0x00000020)  /* confirm for metrolite */
#define RLD_STATUS_UPD_DIS              (0x00000080)  /* confirm for metrolite */

/* CMIC_CMCx_DMA_STAT(x) */
#define DS_CMCx_DMA_CHAIN_DONE(y)       (0x00000001 << (y))  /* confirm for metrolite */
#define DS_CMCx_DMA_DESC_DONE(y)        (0x00000010 << (y))  /* confirm for metrolite */
#define DS_CMCx_DMA_ACTIVE(y)           (0x00000100 << (y))  /* confirm for metrolite */
#define M_IRQ_CHAIN_DONE(ch)            (0x00000001 << (ch))  /* confirm for metrolite */

/* CMIC_CMCx_DMA_STAT_CLR(x) */
#define DS_DESCRD_CMPLT_CLR(y)          (0x00000001 << (y))  /* confirm for metrolite */
#define DS_INTR_COALESCING_CLR(y)       (0x00000010 << (y))  /* confirm for metrolite */


/* General Purpose */
#define TX_CH                      0
#define RX_CH1                     1

/*
* For VT_DOT1Q, VLAN_DEFAULT controls the default VID and can be any value from 1~4094. 
* For VT_PORT_BASED, it keeps VID==1 as default.
*/
#define VLAN_DEFAULT               1     /* confirm for metrolite */
#define L2_ENTRY_SIZE              16384  /* confirm for metrolite */

/* FP_TCAM index */
#define MDNS_TO_CPU_IDX                (0)

#define SYS_MAC_IDX                    (1 * ENTRIES_PER_SLICE)
#define RATE_IGR_IDX                   ((1 * ENTRIES_PER_SLICE) + 3)
#define QOS_BASE_IDX                   (2 * ENTRIES_PER_SLICE)
/* 802.1p higher than DSCP fp entries index */
#define DOT1P_BASE_IDX                 ((2 * ENTRIES_PER_SLICE) + 48)

/* Redirection table index */
#define LOOP_REDIR_T_IDX                 (1)		//not used anymore
#define MDNS_REDIR_T_IDX                 (2)		//not used anymore

/* Loop Detect, per port from LOOP_COUNT_IDX + MIN~ */
#define LOOP_COUNT_IDX                 ( 3 * ENTRIES_PER_SLICE)
#define LOOP_REDIRECT_IDX              ( 3 * ENTRIES_PER_SLICE + 35)

/* TDM setting related */
#define ML_CMIC 0 /* cpu port */
#define ML_LPBK 13 /* loopback port */
#define ML_IDLE 14

/* FP field related */
/////////////////////////////////////////////////////////////////////////
#define FP_TCAM_T_SIZE		(15)
#define FP_POLICY_T_SIZE		(9)
#define FP_PORT_T_SIZE		(15)
#define FP_PORT_FIELD_SEL_T_SIZE		(7)
#define FP_GLOBAL_TCAM_MASK_T_SIZE		(4)
#define FP_METER_T_SIZE		(3)
#define FP_UDF_OFFSET_T_SIZE		(7)
#define FP_UDF_TCAM_T_SIZE		(6)
#define UDF_TABLE_INDEX_LOOPDETECT_ONE_TAG		(0)
#define UDF_TABLE_INDEX_LOOPDETECT_NO_TAG		(1)

#define FP_POLICY_TABLE__R_COS_INT_PRI_MINBIT			(21)
#define FP_POLICY_TABLE__R_COS_INT_PRI_MAXBIT			(32)
#define FP_POLICY_TABLE__R_COS_INT_PRI_MASK_LOWER			(0xFFF << (FP_POLICY_TABLE__R_COS_INT_PRI_MINBIT%32) )
#define FP_POLICY_TABLE__R_COS_INT_PRI_MASK_HIGHER		(0xFFF >> (32 - (FP_POLICY_TABLE__R_COS_INT_PRI_MINBIT%32) ) )
#define FP_POLICY_TABLE__R_COS_INT_PRI_UNSET_LOWER		(~FP_POLICY_TABLE__R_COS_INT_PRI_MASK_LOWER)
#define FP_POLICY_TABLE__R_COS_INT_PRI_UNSET_HIGHER		(~FP_POLICY_TABLE__R_COS_INT_PRI_MASK_HIGHER)

#define FP_POLICY_TABLE__Y_COS_INT_PRI_MINBIT			(33)
#define FP_POLICY_TABLE__Y_COS_INT_PRI_MASK				(0xFFF << (FP_POLICY_TABLE__Y_COS_INT_PRI_MINBIT%32) )
#define FP_POLICY_TABLE__Y_COS_INT_PRI_UNSET			(~FP_POLICY_TABLE__Y_COS_INT_PRI_MASK)
#define FP_POLICY_TABLE__G_COS_INT_PRI_MINBIT			(45)
#define FP_POLICY_TABLE__G_COS_INT_PRI_MASK				(0xFFF << (FP_POLICY_TABLE__G_COS_INT_PRI_MINBIT%32) )
#define FP_POLICY_TABLE__G_COS_INT_PRI_UNSET			(~FP_POLICY_TABLE__G_COS_INT_PRI_MASK)

#define FP_POLICY_TABLE__REDIRECTION_PROFILE_INDEX_MINBIT		(229)
#define FP_POLICY_TABLE__REDIRECTION_PROFILE_INDEX_MAXBIT		(236)
#define FP_POLICY_TABLE__REDIRECTION_PROFILE_INDEX_MASK			(0xFF <<(FP_POLICY_TABLE__REDIRECTION_PROFILE_INDEX_MINBIT%32) )
#define FP_POLICY_TABLE__REDIRECTION_PROFILE_INDEX_UNSET		(~FP_POLICY_TABLE__REDIRECTION_PROFILE_INDEX_MASK)

#define FP_POLICY_TABLE__G_PACKET_REDIRECTION_MINBIT				(208)
#define FP_POLICY_TABLE__G_PACKET_REDIRECTION_MAXBIT				(210)
#define FP_POLICY_TABLE__G_PACKET_REDIRECTION_MASK					(0x7 <<(FP_POLICY_TABLE__G_PACKET_REDIRECTION_MINBIT%32) )
#define FP_POLICY_TABLE__G_PACKET_REDIRECTION_UNSET					(~FP_POLICY_TABLE__G_PACKET_REDIRECTION_MASK)

#define FP_POLICY_TABLE__METER_PAIR_MODE_MODIFIER_BIT				(135)
#define FP_POLICY_TABLE__METER_PAIR_MODE_MODIFIER_MASK			(1 << (FP_POLICY_TABLE__METER_PAIR_MODE_MODIFIER_BIT%32) )
#define FP_POLICY_TABLE__METER_PAIR_MODE_MODIFIER_UNSET			(~FP_POLICY_TABLE__METER_PAIR_MODE_MODIFIER_MASK)
#define FP_POLICY_TABLE__METER_PAIR_INDEX_MINBIT				(261)
#define FP_POLICY_TABLE__METER_PAIR_INDEX_MAXBIT				(270)
#define FP_POLICY_TABLE__METER_PAIR_INDEX_MASK					(0x3FF <<(FP_POLICY_TABLE__METER_PAIR_INDEX_MINBIT%32) )
#define FP_POLICY_TABLE__METER_PAIR_INDEX_UNSET					(~FP_POLICY_TABLE__METER_PAIR_INDEX_MASK)

#define FP_POLICY_TABLE__COUNTER_MODE_MINBIT						(144)
#define FP_POLICY_TABLE__COUNTER_MODE_MAXBIT						(149)
#define FP_POLICY_TABLE__COUNTER_MODE_MASK							(0x3F <<(FP_POLICY_TABLE__COUNTER_MODE_MINBIT%32) )
#define FP_POLICY_TABLE__COUNTER_MODE_UNSET							(~FP_POLICY_TABLE__COUNTER_MODE_MASK)
#define FP_POLICY_TABLE__COUNTER_INDEX_MINBIT						(150)
#define FP_POLICY_TABLE__COUNTER_INDEX_MAXBIT						(156)
#define FP_POLICY_TABLE__COUNTER_INDEX_MASK							(0x7F <<(FP_POLICY_TABLE__COUNTER_INDEX_MINBIT%32) )
#define FP_POLICY_TABLE__COUNTER_INDEX_UNSET						(~FP_POLICY_TABLE__COUNTER_INDEX_MASK)

#define PORT_TABLE__TRUST_DSCP_V4		(1<<0) 
#define PORT_TABLE__TRUST_DSCP_V6		(1<<1) 
#define PORT_TABLE__V4V6_DSCP_ENABLE		(PORT_TABLE__TRUST_DSCP_V4 | PORT_TABLE__TRUST_DSCP_V6)
#define PORT_TABLE__TRUST_INCOMING_VID_BIT	(68) 
#define PORT_TABLE__FILTER_ENABLE_BIT				(85) 
#define PORT_TABLE__USE_INNER_PRI_BIT				(305) 
#define PORT_TABLE__FP_PORT_FIELD_SEL_INDEX				(390) 
#define PORT_TABLE__FP_PORT_FIELD_SEL_INDEX_MASK	(0xFF << (PORT_TABLE__FP_PORT_FIELD_SEL_INDEX%32) )
#define PORT_TABLE__FP_PORT_FIELD_SEL_INDEX_UNSET	(~PORT_TABLE__FP_PORT_FIELD_SEL_INDEX_MASK)

#define PORT_TABLE__STORM_CONTROL_PTR					(376) 
#define PORT_TABLE__STORM_CONTROL_PTR_MASK		(0x7F << (PORT_TABLE__STORM_CONTROL_PTR%32) )
#define PORT_TABLE__STORM_CONTROL_PTR_UNSET		(~PORT_TABLE__STORM_CONTROL_PTR_MASK)

#define PORT_TABLE__TRUST_DSCP_PTR				(176)
#define PORT_TABLE__TRUST_DSCP_PTR_MASK		(0x3F << (PORT_TABLE__TRUST_DSCP_PTR%32))
#define PORT_TABLE__TRUST_DSCP_PTR_UNSET	(~PORT_TABLE__TRUST_DSCP_PTR_MASK)


#define FP_PORT_FIELD_SEL__SLICE0_F1_MINBIT		(0)
#define FP_PORT_FIELD_SEL__SLICE0_F1_MAXBIT		(3)
#define FP_PORT_FIELD_SEL__SLICE0_F1_MASK			(0xF << FP_PORT_FIELD_SEL__SLICE0_F1_MINBIT)
#define FP_PORT_FIELD_SEL__SLICE0_F1_UNSET		(~FP_PORT_FIELD_SEL__SLICE0_F1_MASK)
#define FP_PORT_FIELD_SEL__SLICE0_F2_MINBIT		(4)
#define FP_PORT_FIELD_SEL__SLICE0_F2_MAXBIT		(7)
#define FP_PORT_FIELD_SEL__SLICE0_F2_MASK			(0xF << FP_PORT_FIELD_SEL__SLICE0_F2_MINBIT)
#define FP_PORT_FIELD_SEL__SLICE0_F2_UNSET		(~FP_PORT_FIELD_SEL__SLICE0_F2_MASK)
#define FP_PORT_FIELD_SEL__SLICE0_F3_MINBIT		(8)
#define FP_PORT_FIELD_SEL__SLICE0_F3_MAXBIT		(11)
#define FP_PORT_FIELD_SEL__SLICE0_F3_MASK			(0xF << FP_PORT_FIELD_SEL__SLICE0_F3_MINBIT)
#define FP_PORT_FIELD_SEL__SLICE0_F3_UNSET		(~FP_PORT_FIELD_SEL__SLICE0_F3_MASK)

#define FP_PORT_FIELD_SEL__SLICE1_F1_MINBIT		(15)
#define FP_PORT_FIELD_SEL__SLICE1_F1_MAXBIT		(18)
#define FP_PORT_FIELD_SEL__SLICE1_F1_MASK			(0xF << FP_PORT_FIELD_SEL__SLICE1_F1_MINBIT)
#define FP_PORT_FIELD_SEL__SLICE1_F1_UNSET		(~FP_PORT_FIELD_SEL__SLICE1_F1_MASK)
#define FP_PORT_FIELD_SEL__SLICE1_F2_MINBIT		(19)
#define FP_PORT_FIELD_SEL__SLICE1_F2_MAXBIT		(22)
#define FP_PORT_FIELD_SEL__SLICE1_F2_MASK			(0xF << FP_PORT_FIELD_SEL__SLICE1_F2_MINBIT)
#define FP_PORT_FIELD_SEL__SLICE1_F2_UNSET		(~FP_PORT_FIELD_SEL__SLICE1_F2_MASK)
#define FP_PORT_FIELD_SEL__SLICE1_F3_MINBIT		(23)
#define FP_PORT_FIELD_SEL__SLICE1_F3_MAXBIT		(26)
#define FP_PORT_FIELD_SEL__SLICE1_F3_MASK			(0xF << FP_PORT_FIELD_SEL__SLICE1_F3_MINBIT)
#define FP_PORT_FIELD_SEL__SLICE1_F3_UNSET		(~FP_PORT_FIELD_SEL__SLICE1_F3_MASK)

#define FP_PORT_FIELD_SEL__SLICE2_F1_MINBIT		(30)
#define FP_PORT_FIELD_SEL__SLICE2_F1_MAXBIT		(33)
#define FP_PORT_FIELD_SEL__SLICE2_F1_MASK_LOWER			(0xF << (FP_PORT_FIELD_SEL__SLICE2_F1_MINBIT%32) )
#define FP_PORT_FIELD_SEL__SLICE2_F1_MASK_HIGHER		(0xF >> (32 - (FP_PORT_FIELD_SEL__SLICE2_F1_MINBIT%32) ) )
#define FP_PORT_FIELD_SEL__SLICE2_F1_UNSET_LOWER		(~FP_PORT_FIELD_SEL__SLICE2_F1_MASK_LOWER)
#define FP_PORT_FIELD_SEL__SLICE2_F1_UNSET_HIGHER		(~FP_PORT_FIELD_SEL__SLICE2_F1_MASK_HIGHER)

#define FP_PORT_FIELD_SEL__SLICE2_F2_MINBIT		(34)
#define FP_PORT_FIELD_SEL__SLICE2_F2_MAXBIT		(37)
#define FP_PORT_FIELD_SEL__SLICE2_F2_MASK			(0xF << (FP_PORT_FIELD_SEL__SLICE2_F2_MINBIT%32))
#define FP_PORT_FIELD_SEL__SLICE2_F2_UNSET		(~FP_PORT_FIELD_SEL__SLICE2_F2_MASK)
#define FP_PORT_FIELD_SEL__SLICE2_F3_MINBIT		(38)
#define FP_PORT_FIELD_SEL__SLICE2_F3_MAXBIT		(41)
#define FP_PORT_FIELD_SEL__SLICE2_F3_MASK			(0xF << (FP_PORT_FIELD_SEL__SLICE2_F3_MINBIT%32))
#define FP_PORT_FIELD_SEL__SLICE2_F3_UNSET		(~FP_PORT_FIELD_SEL__SLICE2_F3_MASK)

#define FP_PORT_FIELD_SEL__SLICE3_F1_MINBIT		(45)
#define FP_PORT_FIELD_SEL__SLICE3_F1_MAXBIT		(48)
#define FP_PORT_FIELD_SEL__SLICE3_F1_MASK			(0xF << (FP_PORT_FIELD_SEL__SLICE3_F1_MINBIT%32) )
#define FP_PORT_FIELD_SEL__SLICE3_F1_UNSET		(~FP_PORT_FIELD_SEL__SLICE3_F1_MASK)
#define FP_PORT_FIELD_SEL__SLICE3_F2_MINBIT		(49)
#define FP_PORT_FIELD_SEL__SLICE3_F2_MAXBIT		(52)
#define FP_PORT_FIELD_SEL__SLICE3_F2_MASK			(0xF << (FP_PORT_FIELD_SEL__SLICE3_F2_MINBIT%32) )
#define FP_PORT_FIELD_SEL__SLICE3_F2_UNSET		(~FP_PORT_FIELD_SEL__SLICE3_F2_MASK)
#define FP_PORT_FIELD_SEL__SLICE3_F3_MINBIT		(53)
#define FP_PORT_FIELD_SEL__SLICE3_F3_MAXBIT		(56)
#define FP_PORT_FIELD_SEL__SLICE3_F3_MASK			(0xF << (FP_PORT_FIELD_SEL__SLICE3_F3_MINBIT%32) )
#define FP_PORT_FIELD_SEL__SLICE3_F3_UNSET		(~FP_PORT_FIELD_SEL__SLICE3_F3_MASK)

#define IFP_SINGLE_WIDE_F1__MINBIT				(194)
#define IFP_SINGLE_WIDE_F1__MAXBIT				(233)
#define IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT				(IFP_SINGLE_WIDE_F1__MINBIT+0)
#define IFP_SINGLE_WIDE_F1_11__SGLP_MAXBIT				(IFP_SINGLE_WIDE_F1__MINBIT+15)
#define IFP_SINGLE_WIDE_F1_11__SGLP_MASK	(0xFFFF << (IFP_SINGLE_WIDE_F1_11__SGLP_MINBIT%32) )
#define IFP_SINGLE_WIDE_F1_11__SGLP_UNSET	(~IFP_SINGLE_WIDE_F1_11__SGLP_MASK)

#define IFP_SINGLE_WIDE_F2__MINBIT				(66)
#define IFP_SINGLE_WIDE_F2__MAXBIT				(193)
#define IFP_SINGLE_WIDE_F2_0__L4_DST_MINBIT	(IFP_SINGLE_WIDE_F2__MINBIT+24)
#define IFP_SINGLE_WIDE_F2_0__L4_DST_MAXBIT	(IFP_SINGLE_WIDE_F2__MINBIT+39)
#define IFP_SINGLE_WIDE_F2_0__L4_DST_MASK_LOWER		(0xFFFF << (IFP_SINGLE_WIDE_F2_0__L4_DST_MINBIT%32) )
#define IFP_SINGLE_WIDE_F2_0__L4_DST_MASK_HIGHER		(0xFFFF >> (32 - (IFP_SINGLE_WIDE_F2_0__L4_DST_MINBIT%32)) )
#define IFP_SINGLE_WIDE_F2_0__L4_DST_UNSET_LOWER		(~IFP_SINGLE_WIDE_F2_0__L4_DST_MASK_LOWER)
#define IFP_SINGLE_WIDE_F2_0__L4_DST_UNSET_HIGHER	(~IFP_SINGLE_WIDE_F2_0__L4_DST_MASK_HIGHER)

#define IFP_SINGLE_WIDE_F3__MINBIT 			(26)
#define IFP_SINGLE_WIDE_F3__MAXBIT 			(65)
#define IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT		(IFP_SINGLE_WIDE_F3__MINBIT+0)
#define IFP_SINGLE_WIDE_F3_11__SGLP_MAXBIT		(IFP_SINGLE_WIDE_F3__MINBIT+15)
#define IFP_SINGLE_WIDE_F3_11__SGLP_MASK_LOWER		(0xFFFF << (IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT%32) )
#define IFP_SINGLE_WIDE_F3_11__SGLP_MASK_HIGHER	(0xFFFF >> (32 - (IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT%32) ) )
#define IFP_SINGLE_WIDE_F3_11__SGLP_UNSET_LOWER	(~IFP_SINGLE_WIDE_F3_11__SGLP_MASK_LOWER)
#define IFP_SINGLE_WIDE_F3_11__SGLP_UNSET_HIGHER	(~IFP_SINGLE_WIDE_F3_11__SGLP_MASK_HIGHER)

#define IFP_SINGLE_WIDE_F3_3__OUTER_VLAN_TAG_MINBIT	(IFP_SINGLE_WIDE_F3__MINBIT+0)
#define IFP_SINGLE_WIDE_F3_3__OUTER_VLAN_TAG_MAXBIT	(IFP_SINGLE_WIDE_F3__MINBIT+15)
#define IFP_SINGLE_WIDE_F3_3__OUTER_VLAN_TAG_PRI_MINBIT	(IFP_SINGLE_WIDE_F3_3__OUTER_VLAN_TAG_MINBIT+13)

#define UDF_TCAM_KEY_MINBIT			(1)
#define UDF_TCAM_KEY_MAXBIT			(80)
#define UDF_TCAM_MASK_MINBIT		(81)
#define UDF_TCAM_MASK_MAXBIT		(160)
/////////////////////////////////////////////////////////////////////////
/* Egress rate limit */
#define LLS_PORT_SHAPER_BUCKET_C__NOT_ACTIVE_IN_LLS_BIT			(22)
#define LLS_PORT_SHAPER_BUCKET_C__NOT_ACTIVE_IN_LLS_MASK		(1<< LLS_PORT_SHAPER_BUCKET_C__NOT_ACTIVE_IN_LLS_BIT)
#define LLS_PORT_SHAPER_BUCKET_C__NOT_ACTIVE_IN_LLS_UNSET		(~LLS_PORT_SHAPER_BUCKET_C__NOT_ACTIVE_IN_LLS_MASK)

/* EEE functions */
#ifdef CFG_SWITCH_EEE_INCLUDED
extern void bcm5346x_port_eee_enable_get(uint8 unit, uint8 port, uint8 *enable);
extern sys_error_t bcm5346x_port_eee_enable_set(uint8 unit, 
                      uint8 port, uint8 enable, uint8 save);
extern sys_error_t bcm5346x_port_eee_tx_wake_time_set(uint8 unit, 
                      uint8 port, uint8 type, uint16 wake_time);
extern void bcm5346x_eee_init(void);
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

typedef enum port_block_type_s {
    PORT_BLOCK_TYPE_XLPORT,
    PORT_BLOCK_TYPE_MXQPORT,
    PORT_BLOCK_TYPE_COUNT
} port_block_type_t;

typedef enum serdes_interface_s {
    SERDES_INTERFACE_SGMII = 1,
    SERDES_INTERFACE_FIBER = 2,
} serdes_interface_t;

/* SOC interface */
extern sys_error_t bcm5346x_chip_revision(uint8 unit, uint16 *dev, uint16 *rev);
extern uint8 bcm5346x_port_count(uint8 unit);
extern sys_error_t bcm5346x_reg_get(uint8 unit, uint8 block_id, uint32 addr, uint32 *val);
extern sys_error_t bcm5346x_reg_set(uint8 unit, uint8 block_id, uint32 addr, uint32 val);
extern sys_error_t bcm5346x_reg64_get(uint8 unit, uint8 block_id, uint32 addr, uint32 *val, int len);
extern sys_error_t bcm5346x_reg64_set(uint8 unit, uint8 block_id, uint32 addr, uint32 *buf, int len);
extern sys_error_t bcm5346x_mem_get(uint8 unit, uint8 block_id, uint32 addr, uint32 *buf, int len);
extern sys_error_t bcm5346x_mem_set(uint8 unit, uint8 block_id, uint32 addr, uint32 *buf, int len);
extern sys_error_t bcm5346x_l2_op(uint8 unit, l2x_entry_t *entry, uint8 op);
extern sys_error_t bcm5346x_rx_set_handler(uint8 unit, SOC_RX_HANDLER fn, 
                                           BOOL intr);
extern sys_error_t bcm5346x_rx_fill_buffer(uint8 unit, 
                                           soc_rx_packet_t *pkt);
extern sys_error_t bcm5346x_tx(uint8 unit, soc_tx_packet_t *pkt);
extern sys_error_t bcm5346x_link_status(uint8 unit, uint8 port, BOOL *link);
extern void bcm5346x_rxtx_stop(void);

/* FP_TCAM encode/decode utility */
extern void bcm5346x_dm_to_xy(uint32 *dm_entry, uint32 *xy_entry, int data_words, int bit_length);
extern void bcm5346x_xy_to_dm(uint32 *xy_entry, uint32 *dm_entry, int data_words, int bit_length);
extern void bcm5346x_udf_dm_to_xy(uint32 *dm_entry, uint32 *xy_entry, int data_words, int bit_length);
extern void bcm5346x_udf_xy_to_dm(uint32 *xy_entry, uint32 *dm_entry, int data_words, int bit_length);

#ifdef CFG_SWITCH_VLAN_INCLUDED
extern sys_error_t bcm5346x_pvlan_egress_set(uint8 unit, 
                    uint8 pport, 
                    pbmp_t pbmp);
extern sys_error_t bcm5346x_pvlan_egress_get(uint8 unit, 
                    uint8 pport, 
                    pbmp_t *pbmp);
extern sys_error_t bcm5346x_qvlan_port_set(uint8 unit, 
                    uint16 vlan_id, 
                    pbmp_t pbmp, 
                    pbmp_t tag_pbmp);
extern sys_error_t bcm5346x_qvlan_port_get(uint8 unit, 
                    uint16 vlan_id, 
                    pbmp_t *pbmp, 
                    pbmp_t *tag_pbmp);
extern sys_error_t bcm5346x_vlan_create(uint8 unit, 
                    vlan_type_t type, uint16 vlan_id);
extern sys_error_t bcm5346x_vlan_destroy(uint8 unit, uint16 vlan_id);
extern sys_error_t bcm5346x_vlan_type_set(uint8 unit, vlan_type_t type);
extern sys_error_t bcm5346x_vlan_reset(uint8 unit);
#endif /* CFG_SWITCH_VLAN_INCLUDED */

#ifdef CFG_SWITCH_LAG_INCLUDED
extern sys_error_t bcm5346x_lag_group_set(uint8 unit, uint8 lagid, pbmp_t pbmp);
extern void bcm5346x_lag_group_get(uint8 unit, uint8 lagid, pbmp_t *pbmp);
#endif /* CFG_SWITCH_LAG_INCLUDED */

extern void bcm5346x_loop_detect_enable(BOOL enable);
extern uint8 bcm5346x_loop_detect_status_get(void);

#ifdef CFG_ZEROCONF_MDNS_INCLUDED
sys_error_t bcm5346x_mdns_enable_set(uint8 unit, BOOL mdns_enable);
sys_error_t bcm5346x_mdns_enable_get(uint8 unit, BOOL *mdns_enable);
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

/* Initialization */
extern sys_error_t bcm5346x_sw_init(void);
extern void bcm5346x_dscp_map_enable(BOOL enable);
extern void bcm5346x_qos_init(void);
extern void bcm5346x_rate_init(void);
extern void bcm5346x_loop_detect_init(void);
extern void bcm5346x_rxtx_init(void);
extern void bcm5346x_loopback_enable(uint8 unit, uint8 port, int loopback_mode);
extern sys_error_t bcm5346x_mdns_enable_set(uint8 unit, BOOL enable);
extern sys_error_t bcm5346x_mdns_enable_get(uint8 unit, BOOL *enable);


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

extern mac_driver_t soc_mac_xl;


/* physical port */
#define BCM5346X_PORT_MIN                        1
#define BCM5346X_PORT_MAX                       12

/* logical port */
#define BCM5346X_LPORT_MIN                       1
#define BCM5346X_LPORT_MAX                      12


typedef struct
{
    uint16  devid;
    uint16  revid;
    uint8   link[BCM5346X_LPORT_MAX + 1];           /* link status */
    uint8   loopback[BCM5346X_LPORT_MAX + 1];       /* loopback mode */
    uint8   port_count;
    uint32  pbmp;
#if defined(CFG_SWITCH_EEE_INCLUDED)
    tick_t link_up_time[BCM5346X_LPORT_MAX+1];
    uint8 need_process_for_eee_1s[BCM5346X_LPORT_MAX+1];
#endif /*  CFG_SWITCH_EEE_INCLUDED */
    int    port_p2l_mapping[BCM5346X_PORT_MAX + 1];   /* phy to logic */
    int    port_l2p_mapping[BCM5346X_LPORT_MAX + 1];  /* logic to phy */
    int    port_block_id[BCM5346X_LPORT_MAX + 1];     /* logical port */
    int    port_block_port_id[BCM5346X_LPORT_MAX + 1];/* logical port */
    int    port_block_type[BCM5346X_LPORT_MAX + 1];   /* logical port */
    int    port_speed_max[BCM5346X_LPORT_MAX + 1];    /* logical port */
    int    lane_number[BCM5346X_LPORT_MAX + 1];    /* logical port */
    int    port_admin_status[BCM5346X_LPORT_MAX + 1];   /* logical port */
    int    port_speed_satus[BCM5346X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_link_status[BCM5346X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_duplex_status[BCM5346X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_an_status[BCM5346X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_tx_pause_status[BCM5346X_LPORT_MAX + 1];   /* logical port */
    BOOL    port_rx_pause_status[BCM5346X_LPORT_MAX + 1];   /* logical port */
    mac_driver_t *p_mac[BCM5346X_LPORT_MAX + 1];      /* mac driver */
    int    port_cosq_base[BCM5346X_LPORT_MAX + 2];     /* logical port */
    int    port_uc_cosq_base[BCM5346X_LPORT_MAX + 2];     /* logical port */
    int    port_num_uc_cosq[BCM5346X_LPORT_MAX + 2];     /* logical port */
} bcm5346x_sw_info_t;

extern bcm5346x_sw_info_t ml_sw_info;

#define COS_QUEUE_NUM                  (4)

#define ML_MAX_SERVICE_POOLS           (4)

#define MAX_PRIORITY_GROUPS            (8)


/* Mask of all logical ports */
#define BCM5346X_ALL_PORTS_MASK     (ml_sw_info.pbmp)

/*
 * Macros to get the device block type, block index and index
 * within block for a given port
 */
#define SOC_PORT_SPEED_MAX(port)   (ml_sw_info.port_speed_max[(port)])
#define SOC_PORT_LANE_NUMBER(port)   (ml_sw_info.lane_number[(port)])
#define SOC_PORT_ADMIN_STATUS(port)   (ml_sw_info.port_admin_status[(port)])
#define SOC_PORT_SPEED_STATUS(port)   (ml_sw_info.port_speed_satus[(port)])
#define SOC_PORT_LINK_STATUS(port)   (ml_sw_info.port_link_status[(port)])
#define SOC_PORT_DUPLEX_STATUS(port)   (ml_sw_info.port_duplex_status[(port)])
#define SOC_PORT_AN_STATUS(port)   (ml_sw_info.port_an_status[(port)])
#define SOC_PORT_TX_PAUSE_STATUS(port)   (ml_sw_info.port_tx_pause_status[(port)])
#define SOC_PORT_RX_PAUSE_STATUS(port)   (ml_sw_info.port_rx_pause_status[(port)])
#define SOC_PORT_BLOCK_TYPE(port)   (ml_sw_info.port_block_type[(port)])
#define SOC_PORT_BLOCK(port)        (ml_sw_info.port_block_id[(port)])
#define SOC_PORT_BLOCK_INDEX(port)  (ml_sw_info.port_block_port_id[(port)])

#define SOC_PORT_COSQ_BASE(port)        (ml_sw_info.port_cosq_base[(port)])
#define SOC_PORT_UC_COSQ_BASE(port)        (ml_sw_info.port_uc_cosq_base[(port)])
#define SOC_PORT_NUM_UC_COSQ(port)        (ml_sw_info.port_num_uc_cosq[(port)])

#define SOC_PORT_P2L_MAPPING(port)  (ml_sw_info.port_p2l_mapping[(port)])
#define SOC_PORT_L2P_MAPPING(port)  (ml_sw_info.port_l2p_mapping[(port)])
#define SOC_PORT_COUNT(unit)        (ml_sw_info.port_count)




#define SOC_ICELL_BLOCK(port)    ((port >= PHY_XLPORT1_BASE) ? (IECELL1_BLOCK_ID) : (IECELL0_BLOCK_ID))               

#define IS_XL_PORT(port) \
        (SOC_PORT_BLOCK_TYPE(port) == PORT_BLOCK_TYPE_XLPORT)

#define IS_GX_PORT(port) \
        (SOC_PORT_BLOCK_TYPE(port) == PORT_BLOCK_TYPE_MXQPORT)  /* ML NEED TO CHANGE to IS_MXQ_PORT */

/* Physical port iteration */
#define SOC_PPORT_ITER(_p)       \
        for ((_p) = BCM5346X_PORT_MIN; \
             (_p) <= BCM5346X_PORT_MAX; \
             (_p)++) \
                if (ml_sw_info.port_p2l_mapping[(_p)] != -1)

/* Logical port iteration */
#define SOC_LPORT_ITER(_p)       \
        for ((_p) = BCM5346X_LPORT_MIN; \
             (_p) <= BCM5346X_LPORT_MAX; \
             (_p)++) \
                if ((ml_sw_info.port_l2p_mapping[(_p)] != -1))


#endif /* _BCM5346X_H_ */
