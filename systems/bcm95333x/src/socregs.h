/*
 * $Id: socregs.h,v 1.2 Broadcom SDK $
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
 * File:    low_mem_ca9.S
 * Purpose: H/W exception handling
 */

#define ChipcommonA_CoreCtrl     0x18000008
#define ChipcommonA_ClkDiv       0x180000a4

#define IHOST_PROC_CLK_WR_ACCESS 0x19000000
#define IHOST_PROC_CLK_PLLARMA   0x19000c00
#define IHOST_PROC_CLK_PLLARMA__pllarm_lock 28
#define IHOST_PROC_CLK_PLLARMA__pllarm_soft_post_resetb 1

#define IHOST_PROC_CLK_PLLARMCTRL5 0x19000c20

#define IHOST_PROC_CLK_POLICY_FREQ   0x19000008
#define IHOST_PROC_CLK_POLICY_CTL    0x1900000c
#define IHOST_PROC_CLK_POLICY_CTL__GO_AC 1
#define IHOST_PROC_CLK_POLICY_CTL__GO    0
#define IHOST_PROC_CLK_CORE0_CLKGATE 0x19000200
#define IHOST_PROC_CLK_CORE1_CLKGATE 0x19000204
#define IHOST_PROC_CLK_ARM_SWITCH_CLKGATE 0x19000210
#define IHOST_PROC_CLK_ARM_PERIPH_CLKGATE 0x19000300
#define IHOST_PROC_CLK_APB0_CLKGATE  0x19000400

#define IHOST_SCU_CONTROL        0x19020000
#define IHOST_SCU_INVALIDATE_ALL 0x1902000c
#define IHOST_L2C_CACHE_ID       0x19022000

#define IPROC_WRAP_GEN_PLL_CTRL0 0x1803fc00
#define IPROC_WRAP_GEN_PLL_CTRL0__FAST_LOCK 28
#define IPROC_WRAP_GEN_PLL_CTRL1 0x1803fc04
#define IPROC_WRAP_GEN_PLL_CTRL1__NDIV_INT_R 0
#define IPROC_WRAP_GEN_PLL_CTRL2 0x1803fc08
#define IPROC_WRAP_GEN_PLL_CTRL2__CH3_MDIV_R 8
#define IPROC_WRAP_GEN_PLL_CTRL2__CH4_MDIV_R 16
#define IPROC_WRAP_GEN_PLL_CTRL3 0x1803fc0c
#define IPROC_WRAP_GEN_PLL_CTRL3__SW_TO_GEN_PLL_LOAD 28
#define IPROC_WRAP_GEN_PLL_CTRL3__LOAD_EN_CH_R 16

#define IPROC_WRAP_GEN_PLL_STATUS 0x1803fc18
#define IPROC_WRAP_GEN_PLL_STATUS__GEN_PLL_LOCK 0

#define IHOST_PROC_CLK_WR_ACCESS                        0x19000000
#define IHOST_PROC_CLK_POLICY_FREQ                      0x19000008
#define IHOST_PROC_CLK_POLICY_CTL                       0x1900000c
#define IHOST_PROC_CLK_POLICY_CTL__GO                   0
#define IHOST_PROC_CLK_POLICY_CTL__GO_AC                1

#define IHOST_PROC_CLK_CORE0_CLKGATE                    0x19000200
#define IHOST_PROC_CLK_CORE1_CLKGATE                    0x19000204
#define IHOST_PROC_CLK_ARM_SWITCH_CLKGATE               0x19000210
#define IHOST_PROC_CLK_ARM_PERIPH_CLKGATE               0x19000300
#define IHOST_PROC_CLK_APB0_CLKGATE                     0x19000400

#define IHOST_PROC_CLK_PLLARMA                          0x19000c00
#define IHOST_PROC_CLK_PLLARMA__pllarm_soft_post_resetb 1
#define IHOST_PROC_CLK_PLLARMA__pllarm_lock             28

