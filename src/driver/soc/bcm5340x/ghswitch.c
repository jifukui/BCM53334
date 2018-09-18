/*
 * $Id: ghswitch.c,v 1.92.2.1 Broadcom SDK $
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
#include "soc/bcm5340x.h"
#include "utils/net.h"

#define LINKSCAN_INTERVAL        (100000UL)   /* 100 ms */

#define PORT_LINK_UP                    (1)
#define PORT_LINK_DOWN                  (0)

bcm5340x_sw_info_t gh_sw_info;

/* Flow control is enabled on COSQ1 by default */
#ifdef CFG_FLOW_CONTROL_ENABLED_COSQ_OVERRIDE
#define CFG_FLOW_CONTROL_ENABLED_COSQ CFG_FLOW_CONTROL_ENABLED_COSQ_OVERRIDE
#else
#define CFG_FLOW_CONTROL_ENABLED_COSQ 1
#endif

#define RESCAL_STATUS_0_RESCAL_DONE(val)    \
    ((val) & 0x1)

/* Top strap pin status */

/* TOP  strap pin: bit 9 (lcpll1_refclk_sel)
 * 1:  from i_refclk, Xtal
 * 0: from LCPLL1 differential pad
 */
#define TOP_STRAP_STATUS_LCPLL1_REFCLK_SEL(val)    \
    (((val) >> 9) & 0x1)
/* TOP  strap pin: bit 26(TSC0), 27(TSC1)~30(TSC4), 31(TSCQ)
 * 1: power off
 * 0: power on
 */
#define TOP_STRAP_STATUS_TSC_FIELD(val)    \
    (((val) >> 26) & 0x3F)

/* Top strap pin status */
#define TOP_STRAP_STATUS_1_XTAL_FREQ_SEL(val)    \
    (((val) >> 0) & 0x1)
/* TOP  strap pin: bit 0 (xtal_freq_sel)
 * 1: xtal ouput clock frequence is 50MHz
 * 0: xtal ouput clock frequence is 25MHz
 */

uint8 config_id = CFG_CONFIG_OPTION;

/* BCM53401(Cascade Mode TDM)
 * Option 1 : 4xQSGMII + 8x1G + 4x10G
 * Option 2 : 4xQSGMII + 8x1G + 2x10G + 2x13G
 * Option 3 : 2xQSGMII + 16x1G + 4x10G
 */
static const uint32 gh_tdm_cascade_1[78] = {
        26, 27, 28, 29,  2, 18,
        26, 27, 28, 29,  3, 19,
        26, 27, 28, 29,  4, 20,
        26, 27, 28, 29,  5, 21,
        26, 27, 28, 29,  6, 22,
        26, 27, 28, 29,  7, 23,
        26, 27, 28, 29,  8, 24,
        26, 27, 28, 29,  9, 25,
        26, 27, 28, 29, 30, 34,
        26, 27, 28, 29, 31, 35,
        26, 27, 28, 29, 32, 36,
        26, 27, 28, 29, 33, 37,
        26, 27, 28, 29,  0, 63
};

static const int port_speed_max_cascade_1[] = {
      -1,  -1,  10,  10,  10,  10,  10,  10,
      10,  10,  -1,  -1,  -1,  -1,  -1,  -1,
      -1,  -1,  25,  25,  25,  25,  25,  25,
      25,  25, 110, 110, 110, 110,  25,  25,
      25,  25,  25,  25,  25,  25
};

static const int speed_limit_53401_op3[] = {
         0x2,  0x3,  0x4,  0x5,
        0x12, 0x13, 0x14, 0x15,
        0x16, 0x17, 0x18, 0x19,
        0x1a, 0x1e, 0x1c, 0x20,
        0x1b, 0x1f, 0x1d, 0x21,
        0x22, 0x23, 0x24, 0x25
};

/* 53424: 8x1GE + 16x1G + 4x10G */
static const int port_speed_max_53424[] = {
      -1,  -1,  10,  10,  10,  10,  10,  10,
      10,  10,  -1,  -1,  -1,  -1,  -1,  -1,
      -1,  -1,  10,  10,  10,  10,  10,  10,
      10,  10, 110, 110, 110, 110,  10,  10,
      10,  10,  10,  10,  10,  10
};

static const uint32 gh_tdm_cascade[96] = {
         2, 18, 26, 28, 30, 32,
         3, 19, 26, 28, 30, 32,
         4, 20, 26, 28, 30, 32,
         5, 21, 26, 28, 30, 32,
         6, 22, 26, 28, 30, 32,
         7, 23, 26, 28, 30, 32,
         8, 24, 26, 28, 30, 32,
         9, 25, 26, 28, 30, 32,
        10,  0, 26, 28, 30, 32,
        11, 63, 26, 28, 30, 32,
        12, 63, 26, 28, 30, 32,
        13, 63, 26, 28, 30, 32,
        14, 63, 26, 28, 30, 32,
        15, 63, 26, 28, 30, 32,
        16, 63, 26, 28, 30, 32,
        17, 63, 26, 28, 30, 32
};

static const int p2l_mapping_cascade[] = {
           0, -1,  2,  3,  4,  5,  6,  7,
           8,  9, 10, 11, 12, 13, 14, 15,
          16, 17, 18, 19, 20, 21, 22, 23,
          24, 25, 26, -1, 27, -1, 28, -1,
          29, -1, -1, -1, -1, -1
};

static const int port_speed_max_cascade[] = {
      -1,  -1,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  25,  25,
      25,  25, 110,  -1, 110,  -1, 130,  -1,
      130, -1,  -1,  -1,  -1,  -1
};



static const int port_speed_max_non_cascade[] = {
      -1,  -1,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  25,  25,
      25,  25, 110,  -1, 110,  -1, 110,  -1,
     110,  -1,  -1,  -1,  -1,  -1
};

static const int port_speed_max_non_cascade_2p5[] = {
      -1,  -1,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  25,  25,  25,  25,  25,  25,
      25,  25, 110,  -1, 110,  -1, 110,  -1,
     110,  -1,  -1,  -1,  -1,  -1
};

/* BCM53456 option 1 : 16x1GE(QSGMII)+ 8x2.5G + 4x10GE */
uint32 gh_tdm_cascade_2p5[98] = {
    26,  28,  30,  18,  22,   2,
    32,  26,  28,  19,  23,   3,
    30,  32,  26,  20,  24,   4,
    28,  30,  32,  21,  25,   5,
    26,  28,  30,  18,  22,   6,
    32,  26,  28,  19,  23,   7,
    30,  32,  26,  20,  24,   8,
    28,  30,  32,  21,  25,   9,
    26,  28,  30,  18,  22,  10,
    32,  26,  28,  19,  23,  11,
    30,  32,  26,  20,  24,  12,
    28,  30,  32,  21,  25,  13,
    26,  28,  30,  18,  22,  14,
    32,  26,  28,  19,  23,  15,
    30,  32,  26,  20,  24,  16,
    28,  30,  32,  21,  25,  17,
     0,   63,
};

static const int speed_limit_53456_op1[] = {
     0x2,  0x3,  0x4,  0x5,
    0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19,
    0x1a, 0x1e, 0x1c, 0x20,
    0x1b, 0x1f, 0x1d, 0x21,
    0x22, 0x23, 0x24, 0x25
};


/* BCM53402(Embedded Mode TDM) : 8X10G */
static const uint32 gh_tdm_low_port_count_10g_embeded[36] = {
        22, 23, 24, 25, 26, 27,
        28, 29,  0, 22, 23, 24,
        25, 26, 27, 28, 29, 63,
        22, 23, 24, 25, 26, 27,
        28, 29, 63, 22, 23, 24,
        25, 26, 27, 28, 29, 63
};

/* 8x10G: Low Port Count 10G Embedded */
static const int p2l_mapping_embedded_8x10g[] = {
         0, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1,  2,  3,
         4,  5,  6,  7,  8,  9, -1, -1,
        -1, -1, -1, -1, -1, -1
};

static const int port_speed_max_embedded_8x10g[] = {
    -1,   -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,   -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,   -1,  -1,  -1,  -1,  -1, 110, 110,
    110, 110, 110, 110, 110, 110,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1
};

/* BCM53405(10G Embedded) : 16X10G */
static const uint32 gh_tdm_10g_embedded[68] = {
        22, 23, 24, 25, 26, 27,
        28, 29, 30, 31, 32, 33,
        34, 35, 36, 37,  0, 22,
        23, 24, 25, 26, 27, 28,
        29, 30, 31, 32, 33, 34,
        35, 36, 37, 63, 22, 23,
        24, 25, 26, 27, 28, 29,
        30, 31, 32, 33, 34, 35,
        36, 37, 63, 22, 23, 24,
        25, 26, 27, 28, 29, 30,
        31, 32, 33, 34, 35, 36,
        37, 63
};

static const int p2l_mapping_embedded_16x10g[] = {
         0, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1,  2,  3,
         4,  5,  6,  7,  8,  9, 10, 11,
        12, 13, 14, 15, 16, 17
};

static const int port_speed_max_embedded_16x10g[] = {
    -1,   -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,   -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,   -1,  -1,  -1,  -1,  -1, 110, 110,
    110, 110, 110, 110, 110, 110, 110, 110,
    110, 110, 110, 110, 110, 110
};

/* BCM53406(Mixed Embeded Mode TDM)
 * Option 1 : 12x10G + 8x2.5G + 4x2.5G/5G
 * Option 2 : 4xXAUI + 8x10G
 * Option 3 : 4xXAUI + 4x10G
 * Option 4 : 15x1G/2.5G/5G/10G + 9x1G
 */
static const uint32 gh_tdm_mixed_embeded_53406_op2[68]={
        26, 27, 28, 29, 30, 31,
        32, 33,  2, 18, 22, 34,
        35, 36, 37,  0,  4, 26,
        27, 28, 29, 30, 31, 32,
        33,  2, 18, 22, 34, 35,
        36, 37, 63,  4, 26, 27,
        28, 29, 30, 31, 32, 33,
         2, 18, 22, 34, 35, 36,
        37, 63,  4, 26, 27, 28,
        29, 30, 31, 32, 33,  2,
        18, 22, 34, 35, 36, 37,
        63,  4,
};

static const int p2l_mapping_53406_op2[] = {
         0, -1,  2, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1,  6, -1, -1, -1, 10, -1,
        -1, -1, 14, -1, -1, -1, 18, 19,
        20, 21, 22, 23, 24, 25
};

static const int p2l_mapping_53406_op3[] = {
         0, -1,  2, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1,  6, -1, -1, -1, 10, -1,
        -1, -1, 14, -1, -1, -1, 18, 19,
        20, 21, -1, -1, -1, -1
};

static const int p2m_mapping_53406_op2[] = {
         0, -1,  2, -1,  4, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1,  6, -1, -1, -1, 10, -1,
        -1, -1, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25
};

static const int port_speed_max_53406_op2[] = {
         -1,  -1, 110,  -1,  -1,  -1,  -1,  -1,
         -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
         -1,  -1, 110,  -1,  -1,  -1, 110,  -1,
         -1,  -1, 110,  -1,  -1,  -1, 110, 110,
        110, 110, 110, 110, 110, 110
};

static const int speed_limit_53406_op2[] = {
         0x2,  0x3,  0x4,  0x5,
        0x1b, 0x13, 0x14, 0x15,
        0x1c, 0x17, 0x18, 0x19,
        0x1a, 0x12, 0x16, 0x1d,
        0x1e, 0x1f, 0x20, 0x21,
        0x22, 0x23, 0x24, 0x25
};

/* 53406 option 4 */
uint32 gh_tdm_mixed_embeded_53406_op4[101]={
        26,  27,  28,  29,  34,  35,
        36,  37,  30,  31,  32,  33,
        23,  24,  25,  26,  27,  28,
        29,  34,  35,  36,  37,  30,
        31,  32,  33,  23,  24,  25,
        26,  27,  28,  29,  34,  35,
        36,  37,  30,  31,  32,  33,
        23,  24,  25,  26,  27,  28,
        29,  34,  35,  36,  37,  30,
        31,  32,  33,  23,  24,  25,
        26,  27,  28,  29,  34,   2,
        35,  36,  37,  30,  31,   3,
        32,  33,  23,  24,  25,   4,
        26,  27,  28,  29,  34,   5,
        35,  36,  37,  30,  31,  18,
        32,  33,  23,  24,  25,  22,
        19,  20,  21,   0,  63
};

static const int port_speed_max_53406_op4[] = {
     -1,  -1,  10,  10,  10,  10,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  10,  10,  10,  10,  10, 110,
    110, 110, 110, 110, 110, 110, 110, 110,
    110, 110, 110, 110, 110, 110
};

static const int speed_limit_53406_op4[] = {
    0x16, 0x17, 0x18, 0x19,
    0x12, 0x13, 0x14, 0x15,
     0x2,  0x3,  0x4,  0x5,
    0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25
};

static const uint32 gh_tdm_mixed_embeded_4x5g[68] = {
        26, 27, 28, 29, 30, 31,
        32, 33,  2, 18, 22, 34,
        35, 36, 37,  0,  3, 26,
        27, 28, 29, 30, 31, 32,
        33,  4, 19, 23, 34, 35,
        36, 37, 63,  5, 26, 27,
        28, 29, 30, 31, 32, 33,
         2, 20, 24, 34, 35, 36,
        37, 63,  3, 26, 27, 28,
        29, 30, 31, 32, 33,  4,
        21, 25, 34, 35, 36, 37,
        63,  5};

static const int port_speed_max_mixed_embeded[] = {
        -1,  -1,  50,  50,  50,  50,  -1,  -1,
        -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  25,  25,  25,  25,  25,  25,
        25,  25, 110, 110, 110, 110, 110, 110,
       110, 110, 110, 110, 110, 110
};

/* BCM53408: 24x2.5G */
static const uint32 gh_tdm_mixed_embeded_2p5g[28] = {
         2, 18, 22, 26, 30, 34,
         0,  3, 19, 23, 27, 31,
        35, 63,  4, 20, 24, 28,
        32, 36, 63,  5, 21, 25,
        29, 33, 37, 63
};

static const int p2l_mapping_mixed_embeded[] = {
         0, -1,  2,  3,  4,  5, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1,  6,  7,  8,  9, 10, 11,
        12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25
};

static const int port_speed_max_mixed_embeded_24x2p5[] = {
    -1,  -1,  25,  25,  25,  25,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  25,  25,  25,  25,  25,  25,
    25,  25,  25,  25,  25,  25,  25,  25,
    25,  25,  25,  25,  25,  25
};

/* BCM53454: 20x2.5G + 4 * 10G */
static const int port_speed_max_53454[] = {
    -1,  -1,  25,  25,  25,  25,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  25,  25,  25,  25,  25,  25,
    25,  25,  110, 110, 110, 110,  25,  25,
    25,  25,  25,  25,  25,  25
};

/* Elkhound -53454 */
/* 53454/53455/53456/53457 */
uint32 gh_tdm_elkhound_53454[48]={
        26, 27, 18, 22,  2,  0,
        28, 29, 30, 34,  3, 63,
        26, 27, 19, 23,  4, 63,
        28, 29, 31, 35,  5, 63,
        26, 27, 20, 24,  6, 63,
        28, 29, 32, 36,  7, 63,
        26, 27, 21, 25,  8, 63,
        28, 29, 33, 37,  9, 63,
};

/* 53401 option 3/56062/5345x/5342x */
static const int p2l_mapping_cascade_1[] = {
       0, -1,  2,  3,  4,  5,  6,  7,
       8,  9, -1, -1, -1, -1, -1, -1,
      -1, -1, 10, 11, 12, 13, 14, 15,
      16, 17, 18, 19, 20, 21, 22, 23,
      24, 25, 26, 27, 28, 29
};

/* Bloodhound -53422 */
/* 53422/53424/53426 */
uint32 gh_tdm_bloodhound_53422[78]={
        26, 27, 28, 29, 18,  2,
        26, 27, 28, 29, 19,  3,
        26, 27, 28, 29, 20,  4,
        26, 27, 28, 29, 21,  5,
        26, 27, 28, 29, 22,  6,
        26, 27, 28, 29, 23,  7,
        26, 27, 28, 29, 24,  8,
        26, 27, 28, 29, 25,  9,
        26, 27, 28, 29, 30, 34,
        26, 27, 28, 29, 31, 35,
        26, 27, 28, 29, 32, 36,
        26, 27, 28, 29, 33, 37,
        26, 27, 28, 29,  0, 63,
};

/* 53422: 8x1G + 2x10G */
static const int port_speed_max_53422[] = {
      -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      -1,  -1,  10,  10,  10,  10,  10,  10,
      10,  10, 110, 110,  -1,  -1,  -1,  -1,
      -1,  -1,  -1,  -1,  -1,  -1
};

/* 53426*/
/* 20x1G + 4x10G */
static const int port_speed_max_53426[] = {
      -1,  -1,  10,  10,  10,  10,  -1,  -1,
      -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      -1,  -1,  10,  10,  10,  10,  10,  10,
      10,  10, 110, 110, 110, 110,  10,  10,
      10,  10,  10,  10,  10,  10
};

uint8 tsce_interface = CFG_TSCE_INTERFACE;

#ifdef CFG_LED_MICROCODE_INCLUDED
/* Left LED : Link,   Right LED : TX/RX activity */
static uint8 led_cascade_cfg1[] = {
 0x02, 0x02, 0xD2, 0x1B, 0x75, 0x16, 0xD2, 0x02,
 0x70, 0x0E, 0x71, 0x0E, 0x77, 0x14, 0x02, 0x02,
 0x12, 0x1A, 0x61, 0xD1, 0x77, 0x52, 0xD2, 0x1D,
 0x75, 0x2A, 0xD2, 0x1C, 0x70, 0x22, 0x71, 0x22,
 0x77, 0x28, 0x02, 0x1C, 0x12, 0x1C, 0x61, 0xD1,
 0x77, 0x52, 0xD2, 0x1F, 0x75, 0x3E, 0xD2, 0x1E,
 0x70, 0x36, 0x71, 0x36, 0x77, 0x3C, 0x02, 0x1E,
 0x12, 0x1E, 0x61, 0xD1, 0x77, 0x52, 0xD2, 0x21,
 0x75, 0x6A, 0xD2, 0x20, 0x70, 0x4A, 0x71, 0x4A,
 0x77, 0x50, 0x02, 0x20, 0x12, 0x20, 0x61, 0xD1,
 0x77, 0x52, 0x28, 0x60, 0xD0, 0x67, 0xC4, 0x75,
 0x5D, 0x67, 0xA4, 0x77, 0x5F, 0x67, 0x76, 0x06,
 0xD0, 0x80, 0x16, 0xD1, 0x81, 0xD1, 0x70, 0x02,
 0x77, 0x52, 0x12, 0xD2, 0x85, 0x05, 0xD2, 0x03,
 0x71, 0x74, 0x52, 0x00, 0x3A, 0x70, 0x32, 0x00,
 0x32, 0x01, 0xB7, 0x97, 0x75, 0x85, 0x12, 0xD4,
 0xFE, 0xD0, 0x02, 0x01, 0x50, 0x12, 0xD4, 0xFE,
 0xD0, 0x95, 0x75, 0x91, 0x85, 0x06, 0xD0, 0x77,
 0x99, 0x16, 0xD2, 0xDA, 0x01, 0x75, 0xE5, 0x77,
 0xCB, 0x12, 0xA0, 0xFE, 0xD0, 0x05, 0x0A, 0x00,
 0x75, 0xD8, 0x77, 0xE5, 0x12, 0xA0, 0xF8, 0x15,
 0x1A, 0x01, 0x75, 0xB3, 0x12, 0xD4, 0xFE, 0xD0,
 0x02, 0x01, 0x50, 0x12, 0xD4, 0xFE, 0xD0, 0x95,
 0x75, 0xBC, 0x85, 0x57, 0x16, 0xD2, 0xDA, 0x01,
 0x75, 0xCB, 0x77, 0xD8, 0x12, 0xA0, 0xF8, 0x15,
 0x1A, 0x01, 0x57, 0x22, 0x00, 0x87, 0x22, 0x01,
 0x87, 0x22, 0x00, 0x87, 0x22, 0x01, 0x87, 0x57,
 0x22, 0x00, 0x87, 0x22, 0x00, 0x87, 0x22, 0x00,
 0x87, 0x22, 0x00, 0x87, 0x57, 0x22, 0x00, 0x87,
 0x22, 0x01, 0x87, 0x22, 0x00, 0x87, 0x22, 0x00,
 0x87, 0x57, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};    

static uint8 led_embedded_16x10g_1[] = {
    0x02, 0x16, 0x28, 0x60, 0xD1, 0x67, 0x3D, 0x67,
    0x1C, 0x06, 0xD1, 0x80, 0xD2, 0x26, 0x74, 0x02,
    0x12, 0xD0, 0x85, 0x05, 0xD2, 0x05, 0x71, 0x1A,
    0x52, 0x00, 0x3A, 0x40, 0x32, 0x00, 0x32, 0x01,
    0xB7, 0x97, 0x75, 0x2B, 0x12, 0xD2, 0xFE, 0xD1,
    0x02, 0x0A, 0x50, 0x12, 0xD2, 0xFE, 0xD1, 0x95,
    0x75, 0x35, 0x85, 0x77, 0x4E, 0x16, 0xD0, 0xDA,
    0x02, 0x71, 0x47, 0x77, 0x4E, 0x67, 0x43, 0x75,
    0x4E, 0x77, 0x47, 0x32, 0x08, 0x97, 0x57, 0x32,
    0x0E, 0x87, 0x32, 0x0F, 0x87, 0x57, 0x32, 0x0F,
    0x87, 0x32, 0x0F, 0x87, 0x57, 0x00, 0x00, 0x00
};

static uint8 led_mixed_embedded_1[] = {
 0x02, 0x02, 0xD2, 0x06, 0x75, 0x16, 0xD2, 0x02,
 0x70, 0x0E, 0x71, 0x0E, 0x77, 0x14, 0x02, 0x02,
 0x12, 0x05, 0x61, 0xD1, 0x77, 0x2A, 0xD2, 0x26,
 0x75, 0x42, 0xD2, 0x12, 0x70, 0x22, 0x71, 0x22,
 0x77, 0x28, 0x02, 0x12, 0x12, 0x25, 0x61, 0xD1,
 0x77, 0x2A, 0x28, 0x60, 0xD0, 0x67, 0x9C, 0x75,
 0x35, 0x67, 0x7C, 0x77, 0x37, 0x67, 0x4E, 0x06,
 0xD0, 0x80, 0x16, 0xD1, 0x81, 0xD1, 0x70, 0x02,
 0x77, 0x2A, 0x12, 0xD2, 0x85, 0x05, 0xD2, 0x03,
 0x71, 0x4C, 0x52, 0x00, 0x3A, 0x60, 0x32, 0x00,
 0x32, 0x01, 0xB7, 0x97, 0x75, 0x5D, 0x12, 0xD4,
 0xFE, 0xD0, 0x02, 0x01, 0x50, 0x12, 0xD4, 0xFE,
 0xD0, 0x95, 0x75, 0x69, 0x85, 0x06, 0xD0, 0x77,
 0x71, 0x16, 0xD2, 0xDA, 0x01, 0x75, 0xBD, 0x77,
 0xA3, 0x12, 0xA0, 0xFE, 0xD0, 0x05, 0x0A, 0x00,
 0x75, 0xB0, 0x77, 0xBD, 0x12, 0xA0, 0xF8, 0x15,
 0x1A, 0x01, 0x75, 0x8B, 0x12, 0xD4, 0xFE, 0xD0,
 0x02, 0x01, 0x50, 0x12, 0xD4, 0xFE, 0xD0, 0x95,
 0x75, 0x94, 0x85, 0x57, 0x16, 0xD2, 0xDA, 0x01,
 0x75, 0xA3, 0x77, 0xB0, 0x12, 0xA0, 0xF8, 0x15,
 0x1A, 0x01, 0x57, 0x22, 0x00, 0x87, 0x22, 0x01,
 0x87, 0x22, 0x00, 0x87, 0x22, 0x01, 0x87, 0x57,
 0x22, 0x00, 0x87, 0x22, 0x00, 0x87, 0x22, 0x00,
 0x87, 0x22, 0x00, 0x87, 0x57, 0x22, 0x00, 0x87,
 0x22, 0x01, 0x87, 0x22, 0x00, 0x87, 0x22, 0x00,
 0x87, 0x57, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* Left LED : TX/RX activity,   Right LED : LINK */
static uint8 led_cascade_cfg2[] = {
 0x02, 0x02, 0xD2, 0x1B, 0x75, 0x16, 0xD2, 0x02,
 0x70, 0x0E, 0x71, 0x0E, 0x77, 0x14, 0x02, 0x02,
 0x12, 0x1A, 0x61, 0xD1, 0x77, 0x52, 0xD2, 0x1D,
 0x75, 0x2A, 0xD2, 0x1C, 0x70, 0x22, 0x71, 0x22,
 0x77, 0x28, 0x02, 0x1C, 0x12, 0x1C, 0x61, 0xD1,
 0x77, 0x52, 0xD2, 0x1F, 0x75, 0x3E, 0xD2, 0x1E,
 0x70, 0x36, 0x71, 0x36, 0x77, 0x3C, 0x02, 0x1E,
 0x12, 0x1E, 0x61, 0xD1, 0x77, 0x52, 0xD2, 0x21,
 0x75, 0x6A, 0xD2, 0x20, 0x70, 0x4A, 0x71, 0x4A,
 0x77, 0x50, 0x02, 0x20, 0x12, 0x20, 0x61, 0xD1,
 0x77, 0x52, 0x28, 0x60, 0xD0, 0x67, 0xC4, 0x75,
 0x5D, 0x67, 0xA4, 0x77, 0x5F, 0x67, 0x76, 0x06,
 0xD0, 0x80, 0x16, 0xD1, 0x81, 0xD1, 0x70, 0x02,
 0x77, 0x52, 0x12, 0xD2, 0x85, 0x05, 0xD2, 0x03,
 0x71, 0x74, 0x52, 0x00, 0x3A, 0x70, 0x32, 0x00,
 0x32, 0x01, 0xB7, 0x97, 0x75, 0x85, 0x12, 0xD4,
 0xFE, 0xD0, 0x02, 0x01, 0x50, 0x12, 0xD4, 0xFE,
 0xD0, 0x95, 0x75, 0x91, 0x85, 0x06, 0xD0, 0x77,
 0x99, 0x16, 0xD2, 0xDA, 0x01, 0x75, 0xE5, 0x77,
 0xCB, 0x12, 0xA0, 0xFE, 0xD0, 0x05, 0x0A, 0x00,
 0x75, 0xD8, 0x77, 0xE5, 0x12, 0xA0, 0xF8, 0x15,
 0x1A, 0x01, 0x75, 0xB3, 0x12, 0xD4, 0xFE, 0xD0,
 0x02, 0x01, 0x50, 0x12, 0xD4, 0xFE, 0xD0, 0x95,
 0x75, 0xBC, 0x85, 0x57, 0x16, 0xD2, 0xDA, 0x01,
 0x75, 0xCB, 0x77, 0xD8, 0x12, 0xA0, 0xF8, 0x15,
 0x1A, 0x01, 0x57, 0x22, 0x00, 0x87, 0x22, 0x01,
 0x87, 0x22, 0x00, 0x87, 0x22, 0x01, 0x87, 0x57,
 0x22, 0x00, 0x87, 0x22, 0x00, 0x87, 0x22, 0x00,
 0x87, 0x22, 0x00, 0x87, 0x57, 0x22, 0x00, 0x87,
 0x22, 0x00, 0x87, 0x22, 0x00, 0x87, 0x22, 0x01,
 0x87, 0x57, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};    
static uint8 led_embedded_16x10g_2[] = {
    0x02, 0x16, 0x28, 0x60, 0xD1, 0x67, 0x1C, 0x67,
    0x3D, 0x06, 0xD1, 0x80, 0xD2, 0x26, 0x74, 0x02,
    0x12, 0xD0, 0x85, 0x05, 0xD2, 0x05, 0x71, 0x1A,
    0x52, 0x00, 0x3A, 0x40, 0x32, 0x00, 0x32, 0x01,
    0xB7, 0x97, 0x75, 0x2B, 0x12, 0xD2, 0xFE, 0xD1,
    0x02, 0x0A, 0x50, 0x12, 0xD2, 0xFE, 0xD1, 0x95,
    0x75, 0x35, 0x85, 0x77, 0x4E, 0x16, 0xD0, 0xDA,
    0x02, 0x71, 0x47, 0x77, 0x4E, 0x67, 0x43, 0x75,
    0x4E, 0x77, 0x47, 0x32, 0x08, 0x97, 0x57, 0x32,
    0x0E, 0x87, 0x32, 0x0F, 0x87, 0x57, 0x32, 0x0F,
    0x87, 0x32, 0x0F, 0x87, 0x57, 0x00, 0x00, 0x00
};

static uint8 led_mixed_embedded_2[] = {
 0x02, 0x02, 0xD2, 0x06, 0x75, 0x16, 0xD2, 0x02,
 0x70, 0x0E, 0x71, 0x0E, 0x77, 0x14, 0x02, 0x02,
 0x12, 0x05, 0x61, 0xD1, 0x77, 0x2A, 0xD2, 0x26,
 0x75, 0x42, 0xD2, 0x12, 0x70, 0x22, 0x71, 0x22,
 0x77, 0x28, 0x02, 0x12, 0x12, 0x25, 0x61, 0xD1,
 0x77, 0x2A, 0x28, 0x60, 0xD0, 0x67, 0x9C, 0x75,
 0x35, 0x67, 0x7C, 0x77, 0x37, 0x67, 0x4E, 0x06,
 0xD0, 0x80, 0x16, 0xD1, 0x81, 0xD1, 0x70, 0x02,
 0x77, 0x2A, 0x12, 0xD2, 0x85, 0x05, 0xD2, 0x03,
 0x71, 0x4C, 0x52, 0x00, 0x3A, 0x60, 0x32, 0x00,
 0x32, 0x01, 0xB7, 0x97, 0x75, 0x5D, 0x12, 0xD4,
 0xFE, 0xD0, 0x02, 0x01, 0x50, 0x12, 0xD4, 0xFE,
 0xD0, 0x95, 0x75, 0x69, 0x85, 0x06, 0xD0, 0x77,
 0x71, 0x16, 0xD2, 0xDA, 0x01, 0x75, 0xBD, 0x77,
 0xA3, 0x12, 0xA0, 0xFE, 0xD0, 0x05, 0x0A, 0x00,
 0x75, 0xB0, 0x77, 0xBD, 0x12, 0xA0, 0xF8, 0x15,
 0x1A, 0x01, 0x75, 0x8B, 0x12, 0xD4, 0xFE, 0xD0,
 0x02, 0x01, 0x50, 0x12, 0xD4, 0xFE, 0xD0, 0x95,
 0x75, 0x94, 0x85, 0x57, 0x16, 0xD2, 0xDA, 0x01,
 0x75, 0xA3, 0x77, 0xB0, 0x12, 0xA0, 0xF8, 0x15,
 0x1A, 0x01, 0x57, 0x22, 0x00, 0x87, 0x22, 0x01,
 0x87, 0x22, 0x00, 0x87, 0x22, 0x01, 0x87, 0x57,
 0x22, 0x00, 0x87, 0x22, 0x00, 0x87, 0x22, 0x00,
 0x87, 0x22, 0x00, 0x87, 0x57, 0x22, 0x00, 0x87,
 0x22, 0x00, 0x87, 0x22, 0x00, 0x87, 0x22, 0x01,
 0x87, 0x57, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#endif /* CFG_LED_MICROCODE_INCLUDED */

/* QSGMII_MODE: 1 = qsgmii_mode */
static int QSGMII_MODE = 0;

#if CONFIG_GREYHOUND_EMULATION
int link_qt[BCM5340X_LPORT_MAX+1];

void
bcm5340x_link_change(uint8 port)
{
    link_qt[port] ^= 1;
}
#endif
static void
bcm5340x_handle_link_up(uint8 unit, uint8 lport, int changed, uint32 *flags)
{
    BOOL   tx_pause, rx_pause;
    int    duplex;
    uint32 speed;
    uint32 val;
    int an = 0;

    if (1 == changed) {
        /* Port changes to link up from link down */
#if CONFIG_GREYHOUND_EMULATION
            if (IS_XL_PORT(lport)) {
                speed = 10000;
            } else {
                speed = 1000;
            }
            duplex = tx_pause = rx_pause = TRUE;
#else
        if (gh_sw_info.loopback[lport] == PORT_LOOPBACK_MAC) {
            /* Force link up in MAC loopback mode */
            speed = SOC_PORT_SPEED_MAX(lport);
            duplex = TRUE;
            an = tx_pause = rx_pause = FALSE;
        } else {
            int rv = 0;
            rv |= PHY_SPEED_GET(BMD_PORT_PHY_CTRL(unit, lport), &speed);
            rv |= PHY_DUPLEX_GET(BMD_PORT_PHY_CTRL(unit, lport), &duplex);
            rv |= PHY_AUTONEG_GET(BMD_PORT_PHY_CTRL(unit, lport), &an);
            if (an) {
                rv |= phy_pause_get(unit, lport, &tx_pause, &rx_pause);
                if (SOC_FAILURE(rv)) {
                    return;
                }
            } else {
                tx_pause = rx_pause = TRUE;
            }
        }
#endif /* CONFIG_GREYHOUND_EMULATION */


        SOC_PORT_LINK_STATUS(lport) = TRUE;
        SOC_PORT_SPEED_STATUS(lport) = speed;
        SOC_PORT_DUPLEX_STATUS(lport) = duplex ? TRUE : FALSE;
        SOC_PORT_AN_STATUS(lport) = an ? TRUE : FALSE;
        SOC_PORT_TX_PAUSE_STATUS(lport) = tx_pause;
        SOC_PORT_RX_PAUSE_STATUS(lport) = rx_pause;


#if CFG_CONSOLE_ENABLED
        sal_printf("\nlport %d (P:%d), speed = %d, duplex = %d, tx_pause = %d, rx_pause = %d, an = %d\n",
                   lport, SOC_PORT_L2P_MAPPING(lport), speed, duplex, tx_pause, rx_pause, an);
#endif /* CFG_CONSOLE_ENABLED */

        /* Update LED status */
        val = READCSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)));
        val |= 0x01;
        WRITECSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)), val);

        MAC_SPEED_SET(gh_sw_info.p_mac[lport], unit, lport, speed);

        MAC_DUPLEX_SET(gh_sw_info.p_mac[lport], unit, lport, duplex);

        /* Interface? */

        MAC_PAUSE_SET(gh_sw_info.p_mac[lport], unit, lport, tx_pause, rx_pause);

        if (gh_sw_info.loopback[lport] == PORT_LOOPBACK_MAC) {
            MAC_LOOPBACK_SET(gh_sw_info.p_mac[lport], unit, lport, TRUE);
        } else {
            MAC_LOOPBACK_SET(gh_sw_info.p_mac[lport], unit, lport, FALSE);
        }

        soc_phyctrl_notify(BMD_PORT_PHY_CTRL(unit, lport), PhyEvent_Speed, speed);
        soc_phyctrl_notify(BMD_PORT_PHY_CTRL(unit, lport), PhyEvent_Duplex, duplex);

        /* Enable the MAC. */
        MAC_ENABLE_SET(gh_sw_info.p_mac[lport], unit, lport, TRUE);

#if defined(CFG_SWITCH_EEE_INCLUDED)
        {
            uint8 eee_state;
            int eee_support;
            uint32 remote_eee = 0x0;
            int rv;
            uint32 entry[2];
            /* check if the port is auto-neg, need to use remote eee */
            /* use bit 12 to show the remote eee is enable(1) or not(0) */
            bmd_phy_eee_get(unit,lport, &eee_support);
            eee_state = FALSE;
            if ((an) && (eee_support)) {
                rv = PHY_CONFIG_GET(BMD_PORT_PHY_CTRL(unit, lport),
                            PhyConfig_AdvEEERemote, &remote_eee, NULL);
                if ((rv == CDK_E_NONE) && (remote_eee & 0x7)) {
                    eee_state = TRUE;
                }
            }

            if (eee_state == TRUE) {
                /* Enable EEE in UMAC_EEE_CTRL register after one second
                 * if EEE is enabled in S/W database
                 */
                gh_sw_info.link_up_time[lport] = sal_get_ticks();
                gh_sw_info.need_process_for_eee_1s[lport] = TRUE;
            }

            /* XLMAC EEE wake timer and delay timer setting */
            /* Ref_count: 0x285, speed_1G,  wake timer: 0x11, delay timer: 0x4
                                 speed_10G, wake timer: 0x7, delay timer: 0x1 */
            if (speed == 1000) {
                entry[0] = 0x4;
                entry[1] = (0x285 << 16) | 0x11;
            } else {
                entry[0] = 0x1;
                entry[1] = (0x285 << 16) | 0x7;
            }
            bcm5340x_reg64_set(unit, SOC_PORT_BLOCK(lport),
                        R_XLMAC_EEE_TIMERS(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);


        }
#endif /* CFG_SWITCH_EEE_INCLUDED */

        /* If everything has been completed */
        gh_sw_info.link[lport] = PORT_LINK_UP;

    } else {
        /* Port stays in link up state */
#if defined(CFG_SWITCH_EEE_INCLUDED)
        int eee_support;
        /* EEE one second delay for link up timer check */
        if ((gh_sw_info.need_process_for_eee_1s[lport]) &&
            (SAL_TIME_EXPIRED(gh_sw_info.link_up_time[lport], 1000))) {
#if CFG_CONSOLE_ENABLED
            SAL_DEBUGF(("EEE : enable eee for port %d\n", lport));
#endif /* CFG_CONSOLE_ENABLED */
            bmd_phy_eee_get(unit,lport, &eee_support);
            if (eee_support) {
                bcm5340x_port_eee_enable_set(unit, lport, TRUE, FALSE);
            } else {
                bcm5340x_port_eee_enable_set(unit, lport, FALSE, FALSE);
            }
            gh_sw_info.need_process_for_eee_1s[lport] = FALSE;
        }
#endif /* CFG_SWITCH_EEE_INCLUDED */
    }
}

void
bcm5340x_handle_link_down(uint8 unit, uint8 lport, int changed)
{
#if defined(CFG_SWITCH_EEE_INCLUDED)
    uint8 eee_state;
#endif /* CFG_SWITCH_EEE_INCLUDED */

#ifdef CFG_LOOPBACK_TEST_ENABLED
    if (1 == changed) {
#if CFG_CONSOLE_ENABLED
        if (board_linkdown_message) {
            sal_printf("port %d goes down!\n", lport);
        }
#endif /* CFG_CONSOLE_ENABLED */
	    gh_sw_info.link[lport] = PORT_LINK_DOWN;
    }
#else
    uint32 val;

    if (1 == changed) {
        /* Update LED status */
        val = READCSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)));
        val &= 0xfc;
        WRITECSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)), val);
    
        SOC_PORT_LINK_STATUS(lport) = FALSE;
#if CFG_CONSOLE_ENABLED
        if (board_linkdown_message) {
            sal_printf("port %d goes down!\n", lport);
        }
#endif /* CFG_CONSOLE_ENABLED */

        /* Port changes to link down from link up */
        MAC_ENABLE_SET(gh_sw_info.p_mac[lport], unit, lport, FALSE);

#if defined(CFG_SWITCH_EEE_INCLUDED)
        bcm5340x_port_eee_enable_get(unit, lport, &eee_state);
        if (eee_state == TRUE) {
            /* Disable EEE in UMAC_EEE_CTRL register if EEE is enabled in S/W database */
#if CFG_CONSOLE_ENABLED
            SAL_DEBUGF(("EEE : disable eee for lport %d\n", lport));
#endif /* CFG_CONSOLE_ENABLED */
            bcm5340x_port_eee_enable_set(unit, lport, FALSE, FALSE);
            gh_sw_info.need_process_for_eee_1s[lport] = FALSE;
        }
#endif /* CFG_SWITCH_EEE_INCLUDED */

        gh_sw_info.link[lport] = PORT_LINK_DOWN;
    } else {
        /* Port stays in link down state */
    }
#endif /* CFG_LOOPBACK_TEST_ENABLED */
}

/*
 *  Function : bcm5340x_linkscan_task
 *
 *  Purpose :
 *      Update the link status of switch ports.
 *
 *  Parameters :
 *
 *  Return :
 *
 *  Note :
 *
 */
void
bcm5340x_linkscan_task(void *param)
{
    uint8 unit = 0, lport;
    uint32 flags;
    int link;

    if (board_linkscan_disable)
        return;

    SOC_LPORT_ITER(lport) {
#if CONFIG_GREYHOUND_EMULATION
        link = (int)link_qt[lport];
#else
        if (gh_sw_info.loopback[lport] == PORT_LOOPBACK_MAC) {
            /* Force link up in mac loopback mode */
            link = PORT_LINK_UP;
        } else {
            int rv = 0;
            int andone;
            rv = PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit, lport), &link, &andone);
            if (rv < 0) {
                continue;
            }
        }
#endif /* CONFIG_GREYHOUND_EMULATION */

        if (link == PORT_LINK_UP) {
            /* Link up */
            flags = 0;
            bcm5340x_handle_link_up(unit, lport,
                (gh_sw_info.link[lport] == PORT_LINK_DOWN) ? TRUE : FALSE, &flags);
        } else {
            bcm5340x_handle_link_down(unit, lport,
                (gh_sw_info.link[lport] == PORT_LINK_UP) ? TRUE : FALSE);
        }
    }
}

static void
soc_gh_pll_setting(uint8 unit, uint32 to_usec)
{
    uint32 val, top_strap_val, top_strap_1_val;

    bcm5340x_reg_get(unit, R_TOP_STRAP_STATUS, &top_strap_val);

    bcm5340x_reg_get(unit, R_TOP_STRAP_STATUS_1, &top_strap_1_val);

    /* TOP_LCPLL_SOFT_RESET[Bit 20] = 0x1 */
    bcm5340x_reg_get(unit, R_TOP_SOFT_RESET_REG, &val);
    val |= (0x1 << 20);
    bcm5340x_reg_set(unit, R_TOP_SOFT_RESET_REG, val);

    if (TOP_STRAP_STATUS_1_XTAL_FREQ_SEL(top_strap_1_val)) {
        /* 50 MHz clock source. */

        {   /* BROAD SYNC PLL related */

            /* BROAD_SYNC0_LCPLL_FBDIV_1[Bit 15:0] = 0x1000 */
            bcm5340x_reg_get(unit, R_TOP_BROAD_SYNC0_LCPLL_FBDIV_CTRL_1, &val);
            val &= ~(0xffff << 0);
            val |= (0x1000 << 0);
            bcm5340x_reg_set(unit, R_TOP_BROAD_SYNC0_LCPLL_FBDIV_CTRL_1, val);

            /* CP1=1,CP=3,CZ=3,RP=7,RZ=7,icp=16 */
            bcm5340x_reg_get(unit, R_TOP_BS_PLL0_CTRL_5, &val);
            /* CP1[Bit 23:22] = 1 */
            val &= ~(0x3 << 22);
            val |= (1 << 22);
            /* CP[Bit 21:20] = 3 */
            val &= ~(0x3 << 20);
            val |= (3 << 20);
            /* CZ[Bit 19:18] = 3 */
            val &= ~(0x3 << 18);
            val |= (3 << 18);
            /* ICP[Bit 17:12] = 16 */
            val &= ~(0x3f << 12);
            val |= (16 << 12);
            /* RP[Bit 11:9] = 7 */
            val &= ~(0x7 << 9);
            val |= (7 << 9);
            /* RZ[Bit 8:4] = 7 */
            val &= ~(0x1f << 4);
            val |= (7 << 4);
            bcm5340x_reg_set(unit, R_TOP_BS_PLL0_CTRL_5, val);

            /* BROAD_SYNC1_LCPLL_FBDIV_1[Bit 15:0] = 0x0f80 */
            bcm5340x_reg_get(unit, R_TOP_BROAD_SYNC1_LCPLL_FBDIV_CTRL_1, &val);
            val &= ~(0xffff << 0);
            val |= (0x0f80 << 0);
            bcm5340x_reg_set(unit, R_TOP_BROAD_SYNC1_LCPLL_FBDIV_CTRL_1, val);

            /* CP1=1,CP=3,CZ=3,RP=7,RZ=7,icp=16 */
            bcm5340x_reg_get(unit, R_TOP_BS_PLL1_CTRL_5, &val);
            /* CP1[Bit 23:22] = 1 */
            val &= ~(0x3 << 22);
            val |= (1 << 22);
            /* CP[Bit 21:20] = 3 */
            val &= ~(0x3 << 20);
            val |= (3 << 20);
            /* CZ[Bit 19:18] = 3 */
            val &= ~(0x3 << 18);
            val |= (3 << 18);
            /* ICP[Bit 17:12] = 16 */
            val &= ~(0x3f << 12);
            val |= (16 << 12);
            /* RP[Bit 11:9] = 7 */
            val &= ~(0x7 << 9);
            val |= (7 << 9);
            /* RZ[Bit 8:4] = 7 */
            val &= ~(0x1f << 4);
            val |= (7 << 4);
            bcm5340x_reg_set(unit, R_TOP_BS_PLL1_CTRL_5, val);
        }

        /* FREQ_DOUBLER_ON[Bit 13] = 0 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL0_CTRL_7, &val);
        val &= ~(0x1 << 13);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL0_CTRL_7, val);

        /* CP1=1,CP=3,CZ=3,RP=7,RZ=7,icp=16 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL0_CTRL_5, &val);
        /* CP1[Bit 23:22] = 1 */
        val &= ~(0x3 << 22);
        val |= (1 << 22);
        /* CP[Bit 21:20] = 3 */
        val &= ~(0x3 << 20);
        val |= (3 << 20);
        /* CZ[Bit 19:18] = 3 */
        val &= ~(0x3 << 18);
        val |= (3 << 18);
        /* ICP[Bit 17:12] = 16 */
        val &= ~(0x3f << 12);
        val |= (16 << 12);
        /* RP[Bit 11:9] = 7 */
        val &= ~(0x7 << 9);
        val |= (7 << 9);
        /* RZ[Bit 8:4] = 7 */
        val &= ~(0x1f << 4);
        val |= (7 << 4);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL0_CTRL_5, val);
    } else {
        /* 25 MHz clock source */

        {   /* BROAD SYNC PLL related */

            /* BROAD_SYNC0_LCPLL_FBDIV_1[Bit 15:0] = 0x2000 */
            bcm5340x_reg_get(unit, R_TOP_BROAD_SYNC0_LCPLL_FBDIV_CTRL_1, &val);
            val &= ~(0xffff << 0);
            val |= (0x2000 << 0);
            bcm5340x_reg_set(unit, R_TOP_BROAD_SYNC0_LCPLL_FBDIV_CTRL_1, val);

            /* CP1=0,CP=0,CZ=3,RP=0,RZ=8,icp=32 */
            bcm5340x_reg_get(unit, R_TOP_BS_PLL0_CTRL_5, &val);
            /* CP1[Bit 23:22] = 1 */
            val &= ~(0x3 << 22);
            /* CP[Bit 21:20] = 1 */
            val &= ~(0x3 << 20);
            /* CZ[Bit 19:18] = 3 */
            val &= ~(0x3 << 18);
            val |= (3 << 18);
            /* ICP[Bit 17:12] = 32 */
            val &= ~(0x3f << 12);
            val |= (32 << 12);
            /* RP[Bit 11:9] = 0 */
            val &= ~(0x7 << 9);
            /* RZ[Bit 8:4] = 8 */
            val &= ~(0x1f << 4);
            val |= (8 << 4);
            bcm5340x_reg_set(unit, R_TOP_BS_PLL0_CTRL_5, val);

            /* BROAD_SYNC1_LCPLL_FBDIV_1[Bit 15:0] = 0x1f00 */
            bcm5340x_reg_get(unit, R_TOP_BROAD_SYNC1_LCPLL_FBDIV_CTRL_1, &val);
            val &= ~(0xffff << 0);
            val |= (0x1f00 << 0);
            bcm5340x_reg_set(unit, R_TOP_BROAD_SYNC1_LCPLL_FBDIV_CTRL_1, val);

            /* CP1=0,CP=0,CZ=3,RP=0,RZ=8,icp=32 */
            bcm5340x_reg_get(unit, R_TOP_BS_PLL1_CTRL_5, &val);
            /* CP1[Bit 23:22] = 1 */
            val &= ~(0x3 << 22);
            /* CP[Bit 21:20] = 1 */
            val &= ~(0x3 << 20);
            /* CZ[Bit 19:18] = 3 */
            val &= ~(0x3 << 18);
            val |= (3 << 18);
            /* ICP[Bit 17:12] = 32 */
            val &= ~(0x3f << 12);
            val |= (32 << 12);
            /* RP[Bit 11:9] = 0 */
            val &= ~(0x7 << 9);
            /* RZ[Bit 8:4] = 8 */
            val &= ~(0x1f << 4);
            val |= (8 << 4);
            bcm5340x_reg_set(unit, R_TOP_BS_PLL1_CTRL_5, val);
        }

        /* FREQ_DOUBLER_ON[Bit 13] = 1 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL0_CTRL_7, &val);
        val |= (1 << 13);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL0_CTRL_7, val);

        /* CP1=1,CP=1,CZ=3,RP=0,RZ=8,icp=32 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL0_CTRL_5, &val);
        /* CP1[Bit 23:22] = 1 */
        val &= ~(0x3 << 22);
        val |= (1 << 22);
        /* CP[Bit 21:20] = 1 */
        val &= ~(0x3 << 20);
        val |= (1 << 20);
        /* CZ[Bit 19:18] = 3 */
        val &= ~(0x3 << 18);
        val |= (3 << 18);
        /* ICP[Bit 17:12] = 32 */
        val &= ~(0x3f << 12);
        val |= (32 << 12);
        /* RP[Bit 11:9] = 0 */
        val &= ~(0x7 << 9);
        /* RZ[Bit 8:4] = 8 */
        val &= ~(0x1f << 4);
        val |= (8 << 4);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL0_CTRL_5, val);
    }

    {   /* BROAD SYNC PLL related */

        /* PDIV[Bit 25:22] = 0x1 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL0_CTRL_1, &val);
        val &= ~(0xf << 22);
        val |= (0x1 << 22);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL0_CTRL_1, val);

        /* CH0_MDIV[Bit 7:0] = 0xa0 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL0_CTRL_0, &val);
        val &= ~(0xff << 0);
        val |= (0xa0 << 0);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL0_CTRL_0, val);

        /* FREQ_DOUBLER_ON[Bit 13] = 0 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL0_CTRL_7, &val);
        val &= ~(0x1 << 13);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL0_CTRL_7, val);

        /* MSC_CTRL[Bit 15:0] = 0x0012 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL0_CTRL_6, &val);
        val &= ~(0xffff << 0);
        val |= (0x0012 << 0);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL0_CTRL_6, val);

        /* VCO_CONT_ADJ[Bit 3:2] = 0x1 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL0_CTRL_7, &val);
        val &= ~(0x3 << 2);
        val |= (0x1 << 2);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL0_CTRL_7, val);
        
        /* VCO_CUR[Bit 4:2] = 0x0 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL0_CTRL_3, &val);
        val &= ~(0x7 << 2);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL0_CTRL_3, val);
        
        /* VCO_GAIN[Bit 3:0] = 0x3 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL0_CTRL_5, &val);
        val &= ~(0xf << 0);
        val |= (0x3 << 0);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL0_CTRL_5, val);

        /* CMIC_TO_BS_PLL0_SW_OVWR[Bit 15] = 0x1 */
        bcm5340x_reg_get(unit, R_TOP_MISC_CONTROL_1, &val);
        val |= (0x1 << 15);
        bcm5340x_reg_set(unit, R_TOP_MISC_CONTROL_1, val);

        /* BROAD_SYNC0_LCPLL_FBDIV_0[Bit 31:0] = 0x0 */
        val = 0x0;
        bcm5340x_reg_set(unit, R_TOP_BROAD_SYNC0_LCPLL_FBDIV_CTRL_0, val);

        /* cpp[Bit 27:16] = 0x80 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL0_CTRL_7, &val);
        val &= ~(0xFFF << 16);
        val |= (0x80 << 16);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL0_CTRL_7, val);

        /* PDIV[Bit 25:22] = 0x1 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL1_CTRL_1, &val);
        val &= ~(0xf << 22);
        val |= (0x1 << 22);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL1_CTRL_1, val);

        /* CH0_MDIV[Bit 7:0] = 0x9b */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL1_CTRL_0, &val);
        val &= ~(0xff << 0);
        val |= (0x9b << 0);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL1_CTRL_0, val);

        /* FREQ_DOUBLER_ON[Bit 13] = 0 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL1_CTRL_7, &val);
        val &= ~(0x1 << 13);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL1_CTRL_7, val);

        /* MSC_CTRL[Bit 15:0] = 0x0012 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL1_CTRL_6, &val);
        val &= ~(0xffff << 0);
        val |= (0x0012 << 0);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL1_CTRL_6, val);

        /* VCO_CONT_ADJ[Bit 3:2] = 0x1 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL1_CTRL_7, &val);
        val &= ~(0x3 << 2);
        val |= (0x1 << 2);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL1_CTRL_7, val);
        
        /* VCO_CUR[Bit 4:2] = 0x0 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL1_CTRL_3, &val);
        val &= ~(0x7 << 2);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL1_CTRL_3, val);
        
        /* VCO_GAIN[Bit 3:0] = 0x3 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL1_CTRL_5, &val);
        val &= ~(0xf << 0);
        val |= (0x3 << 0);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL1_CTRL_5, val);

        /* CMIC_TO_BS_PLL0_SW_OVWR[Bit 16] = 0x1 */
        bcm5340x_reg_get(unit, R_TOP_MISC_CONTROL_1, &val);
        val |= (0x1 << 16);
        bcm5340x_reg_set(unit, R_TOP_MISC_CONTROL_1, val);

        /* BROAD_SYNC1_LCPLL_FBDIV_0[Bit 31:0] = 0x0 */
        val = 0x0;
        bcm5340x_reg_set(unit, R_TOP_BROAD_SYNC1_LCPLL_FBDIV_CTRL_0, val);

        /* cpp[Bit 27:16] = 0x80 */
        bcm5340x_reg_get(unit, R_TOP_BS_PLL1_CTRL_7, &val);
        val &= ~(0xFFF << 16);
        val |= (0x80 << 16);
        bcm5340x_reg_set(unit, R_TOP_BS_PLL1_CTRL_7, val);
    }

    /* PDIV[Bit 25:22] = 0x1 */
    bcm5340x_reg_get(unit, R_TOP_XG_PLL0_CTRL_1, &val);
    val &= ~(0xf << 22);
    val |= (0x1 << 22);
    bcm5340x_reg_set(unit, R_TOP_XG_PLL0_CTRL_1, val);

    /* CH0_MDIV[Bit 7:0] = 0x18 */
    bcm5340x_reg_get(unit, R_TOP_XG_PLL0_CTRL_0, &val);
    val &= ~(0xff << 0);
    val |= (0x18 << 0);
    bcm5340x_reg_set(unit, R_TOP_XG_PLL0_CTRL_0, val);

    /* MSC_CTRL[Bit 15:0] = 0x0012, except msc_ctrl[Bit 12] (enable LCPLL0 for external PHY) */
    bcm5340x_reg_get(unit, R_TOP_XG_PLL0_CTRL_6, &val);
    val &= ~(0xefff << 0);
    val |= (0x0012 << 0);
    bcm5340x_reg_set(unit, R_TOP_XG_PLL0_CTRL_6, val);

    /* VCO_CONT_ADJ[Bit 3:2] = 0x1 */
    bcm5340x_reg_get(unit, R_TOP_XG_PLL0_CTRL_7, &val);
    val &= ~(0x3 << 2);
    val |= (0x1 << 2);
    bcm5340x_reg_set(unit, R_TOP_XG_PLL0_CTRL_7, val);
    
    /* VCO_CUR[Bit 4:2] = 0x0 */
    bcm5340x_reg_get(unit, R_TOP_XG_PLL0_CTRL_3, &val);
    val &= ~(0x7 << 2);
    bcm5340x_reg_set(unit, R_TOP_XG_PLL0_CTRL_3, val);
    
    /* VCO_GAIN[Bit 3:0] = 0x3 */
    bcm5340x_reg_get(unit, R_TOP_XG_PLL0_CTRL_5, &val);
    val &= ~(0xf << 0);
    val |= (0x3 << 0);
    bcm5340x_reg_set(unit, R_TOP_XG_PLL0_CTRL_5, val);

    /* CMIC_TO_XG_PLL0_SW_OVWR[Bit 13] = 0x1 */
    bcm5340x_reg_get(unit, R_TOP_MISC_CONTROL_1, &val);
    val |= (0x1 << 13);
    bcm5340x_reg_set(unit, R_TOP_MISC_CONTROL_1, val);

    /* XG0_LCPLL_FBDIV_0[Bit 31:0] = 0x0 */
    val = 0x0;
    bcm5340x_reg_set(unit, R_TOP_XG0_LCPLL_FBDIV_CTRL_0, val);

    /* XG0_LCPLL_FBDIV_1[Bit 15:0] = 0x0f00 */
    bcm5340x_reg_get(unit, R_TOP_XG0_LCPLL_FBDIV_CTRL_1, &val);
    val &= ~(0xffff << 0);
    val |= (0x0f00 << 0);
    bcm5340x_reg_set(unit, R_TOP_XG0_LCPLL_FBDIV_CTRL_1, val);

    /* cpp[Bit 27:16] = 0x80 */
    bcm5340x_reg_get(unit, R_TOP_XG_PLL0_CTRL_7, &val);
    val &= ~(0xFFF << 16);
    val |= (0x80 << 16);
    bcm5340x_reg_set(unit, R_TOP_XG_PLL0_CTRL_7, val);

    /* Bit 9: strap_lcpll1_refclk_sel
     * 1 : from i_refclk, Xtal
     * 0 : from LCPLL1 differential pad
     */
    if (TOP_STRAP_STATUS_LCPLL1_REFCLK_SEL(top_strap_val)) {
        /* strap_lcpll1_refclk_sel == 1 */
        /* PDIV[Bit 25:22] = 0x1 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_1, &val);
        val &= ~(0xf << 22);
        val |= (0x1 << 22);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_1, val);

        /* CH0_MDIV[Bit 7:0] = 0x14 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_0, &val);
        val &= ~(0xff << 0);
        val |= (0x14 << 0);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_0, val);

        if (TOP_STRAP_STATUS_1_XTAL_FREQ_SEL(top_strap_1_val)) {
            /* 50MHz ref */

            /* FREQ_DOUBLER_ON[Bit 13] = 0 */
            bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_7, &val);
            val &= ~(0x1 << 13);
            bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_7, val);
        } else {
            /* 25MHz ref */

            /* FREQ_DOUBLER_ON[Bit 13] = 1 */
            bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_7, &val);
            val |= (1 << 13);
            bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_7, val);
        }

        /* MSC_CTRL[Bit 15:0] = 0x0192 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_6, &val);
        val &= ~(0xffff << 0);
        val |= (0x0192 << 0);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_6, val);

        /* VCO_CONT_ADJ[Bit 3:2] = 0x1 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_7, &val);
        val &= ~(0x3 << 2);
        val |= (0x1 << 2);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_7, val);

        /* VCO_CUR[Bit 4:2] = 0x0 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_3, &val);
        val &= ~(0x7 << 2);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_3, val);

        /* VCO_GAIN[Bit 3:0] = 0x3 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_5, &val);
        val &= ~(0xf << 0);
        val |= (0x3 << 0);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_5, val);

        /* CMIC_TO_XG_PLL1_SW_OVWR[Bit 14] = 0x1 */
        bcm5340x_reg_get(unit, R_TOP_MISC_CONTROL_1, &val);
        val |= (0x1 << 14);
        bcm5340x_reg_set(unit, R_TOP_MISC_CONTROL_1, val);

        /* XG1_LCPLL_FBDIV_0[Bit 31:0] = 0x0 */
        val = 0x0;
        bcm5340x_reg_set(unit, R_TOP_XG1_LCPLL_FBDIV_CTRL_0, val);

        /* XG1_LCPLL_FBDIV_1[Bit 15:0] = 0x0fa0 */
        bcm5340x_reg_get(unit, R_TOP_XG1_LCPLL_FBDIV_CTRL_1, &val);
        val &= ~(0xffff << 0);
        val |= (0x0fa0 << 0);
        bcm5340x_reg_set(unit, R_TOP_XG1_LCPLL_FBDIV_CTRL_1, val);

        /* CP1=1,CP=3,CZ=3,RP=7,RZ=7,icp=16 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_5, &val);
        /* CP1[Bit 23:22] = 1 */
        val &= ~(0x3 << 22);
        val |= (1 << 22);
        /* CP[Bit 21:20] = 3 */
        val &= ~(0x3 << 20);
        val |= (3 << 20);
        /* CZ[Bit 19:18] = 3 */
        val &= ~(0x3 << 18);
        val |= (3 << 18);
        /* ICP[Bit 17:12] = 16 */
        val &= ~(0x3f << 12);
        val |= (16 << 12);
        /* RP[Bit 11:9] = 7 */
        val &= ~(0x7 << 9);
        val |= (7 << 9);
        /* RZ[Bit 8:4] = 7 */
        val &= ~(0x1f << 4);
        val |= (7 << 4);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_5, val);

        /* cpp[Bit 27:16] = 0x80 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_7, &val);
        val &= ~(0xFFF << 16);
        val |= (0x80 << 16);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_7, val);
    } else {
        /* strap_sts = 0:  from LCPLL1 differential pad */
        /* PDIV[Bit 25:22] = 0x3 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_1, &val);
        val &= ~(0xf << 22);
        val |= (0x3 << 22);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_1, val);

        /* VCO_CONT_ADJ[Bit 3:2] = 0x1 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_7, &val);
        val &= ~(0x3 << 2);
        val |= (0x1 << 2);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_7, val);

        /* VCO_CUR[Bit 4:2] = 0x0 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_3, &val);
        val &= ~(0x7 << 2);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_3, val);

        /* VCO_GAIN[Bit 3:0] = 0x3 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_5, &val);
        val &= ~(0xf << 0);
        val |= (0x3 << 0);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_5, val);

        /* CMIC_TO_XG_PLL1_SW_OVWR[Bit 14] = 0x1 */
        bcm5340x_reg_get(unit, R_TOP_MISC_CONTROL_1, &val);
        val |= (0x1 << 14);
        bcm5340x_reg_set(unit, R_TOP_MISC_CONTROL_1, val);

        /* XG1_LCPLL_FBDIV_0[Bit 31:0] = 0x0 */
        val = 0x0;
        bcm5340x_reg_set(unit, R_TOP_XG1_LCPLL_FBDIV_CTRL_0, val);

        /* XG1_LCPLL_FBDIV_1[Bit 15:0] = 0x0f00 */
        bcm5340x_reg_get(unit, R_TOP_XG1_LCPLL_FBDIV_CTRL_1, &val);
        val &= ~(0xffff << 0);
        val |= (0x0f00 << 0);
        bcm5340x_reg_set(unit, R_TOP_XG1_LCPLL_FBDIV_CTRL_1, val);

        /* CP1=1,CP=1,CZ=3,RP=0,RZ=8,icp=32 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_5, &val);
        /* CP1[Bit 23:22] = 1 */
        val &= ~(0x3 << 22);
        val |= (1 << 22);
        /* CP[Bit 21:20] = 1 */
        val &= ~(0x3 << 20);
        val |= (1 << 20);
        /* CZ[Bit 19:18] = 3 */
        val &= ~(0x3 << 18);
        val |= (3 << 18);
        /* ICP[Bit 17:12] = 32 */
        val &= ~(0x3f << 12);
        val |= (32 << 12);
        /* RP[Bit 11:9] = 0 */
        val &= ~(0x7 << 9);
        /* RZ[Bit 8:4] = 8 */
        val &= ~(0x1f << 4);
        val |= (8 << 4);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_5, val);

        /* cpp[Bit 27:16] = 0x80 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_7, &val);
        val &= ~(0xFFF << 16);
        val |= (0x80 << 16);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_7, val);

        /* MSC_CTRL[Bit 15:0] = 0xcbd4 */
        bcm5340x_reg_get(unit, R_TOP_XG_PLL1_CTRL_6, &val);
        val &= ~(0xffff << 0);
        val |= (0xcbd4 << 0);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL1_CTRL_6, val);
    }

    /* TOP_LCPLL_SOFT_RESET[Bit 20] = 0 */
    bcm5340x_reg_get(unit, R_TOP_SOFT_RESET_REG, &val);
    val &= ~(0x1 << 20);
    bcm5340x_reg_set(unit, R_TOP_SOFT_RESET_REG, val);

    sal_usleep(to_usec);
}

static void
soc_reset(uint8 unit)
{
    uint32 val, to_usec, top_strap_val, tmp_val;

#if CONFIG_GREYHOUND_EMULATION
    to_usec = 250000;
#else
    to_usec = 10000;
#endif /* CONFIG_GREYHOUND_EMULATION */

    /* Use 156.25Mhz reference clock for LCPLL? */

#ifdef CFG_SBUS_FORMAT_V3
    WRITECSR(CMIC_SBUS_RING_MAP_0_7, 0x11122200); /* block 7  - 0 */
    WRITECSR(CMIC_SBUS_RING_MAP_8_15, 0x00330000); /* block 15 - 8 */
    WRITECSR(CMIC_SBUS_RING_MAP_16_23, 0x00005004); /* block 23 - 16 */
    WRITECSR(CMIC_SBUS_RING_MAP_24_31, 0x00000000); /* block 31 - 24 */
#else
    /*
     * Greyhound:
     * Ring0: cmic -> ip-ep -> cmic
     * RING1: cmic -> xlport0 -> xlport1-> xlport2 -> xlport3 -> xlport4 -> cmic
     * Ring2: cmic -> xlport5 -> gxport0 -> gxport1 -> PMQ0 -> cmic
     * Ring3: cmic -> mmu -> cmic
     * Ring4: cmic -> top -> otpc -> avs_top -> cmic
     * Ring5: cmic -> cmic_ser ->cmic
     * Ring 6,7 unused
     */
    WRITECSR(CMIC_SBUS_RING_MAP_0_7, 0x11112200); /* block 7  - 0 */
    WRITECSR(CMIC_SBUS_RING_MAP_8_15, 0x00430001); /* block 15 - 8 */
    WRITECSR(CMIC_SBUS_RING_MAP_16_23, 0x00005064); /* block 23 - 16 */
    WRITECSR(CMIC_SBUS_RING_MAP_24_31, 0x00000000); /* block 31 - 24 */
    WRITECSR(CMIC_SBUS_RING_MAP_32_39, 0x00002222); /* block 39 - 32 */
#endif /* CFG_SBUS_FORMAT_V3 */

    WRITECSR(CMIC_SBUS_TIMEOUT, 0x7d0);

    /* Skip polling TOP_XG_PLL0_STATUS since we have to keep going anyway */
    sal_usleep(to_usec);

    /* PCIe Serdes workaround */
    bcm5340x_reg_get(unit, R_RESCAL_STATUS_0, &val);
    /* RESCAL_DONE[Bit 0] */
    if (RESCAL_STATUS_0_RESCAL_DONE(val)) {
        WRITECSR(R_CHIPCOMMONG_MII_MANAGEMENT_CONTROL, 0x88);
        WRITECSR(R_CHIPCOMMONG_MII_MANAGEMENT_COMMAND_DATA, 0x517e1000);
        /* RESCAL_PON[Bit 31:28] */
        WRITECSR(R_CHIPCOMMONG_MII_MANAGEMENT_COMMAND_DATA, (0x5106c030 | (val >> 28)));
        sal_usleep(to_usec);
        WRITECSR(R_CHIPCOMMONG_MII_MANAGEMENT_CONTROL, 0x0);
    }

    /* Power down USB PHY in Lite sku */
    if ((gh_sw_info.devid == BCM53401_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53402_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53405_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53406_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53408_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53454_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53456_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53422_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53424_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53426_DEVICE_ID)) {
        val = READCSR(R_IPROC_WRAP_USBPHY_CTRL_2);
        /* PHY_ISO[Bit 17] = 1*/
        val |= (0x1 << 17);
        WRITECSR(R_IPROC_WRAP_USBPHY_CTRL_2, val);
        val = READCSR(R_USBH_UTMI_P0CTL);
        /* DFE_POWERUP_FSM[Bit 11] = 0 */
        val &= ~(0x1 << 11);
        WRITECSR(R_USBH_UTMI_P0CTL, val);
    }

    /* AVS PVTMON settings fixed in temperature mode */
    val = 0x7e;
    bcm5340x_reg_set(unit, R_AVS_REG_HW_MNTR_SEQUENCER_MASK_PVT_MNTR, val);
    bcm5340x_reg_get(unit, R_TOP_PVTMON_CTRL_0, &val);
    /* BG_ADJ[Bit 2:0] = 1*/
    val &= ~0x7;
    val |= 0x1;
    bcm5340x_reg_set(unit, R_TOP_PVTMON_CTRL_0, val);

#ifdef _BCM95340X_
    /* BCM953411K and BCM953456K needs to enable LCPLL0 for external PHY */
    if ((gh_sw_info.devid == BCM53401_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53411_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53456_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53457_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53424_DEVICE_ID)) {
        val = READCSR(CMIC_GP_OUT_EN);
        val |= (1 << 3);
        WRITECSR(CMIC_GP_OUT_EN, val);

        val = READCSR(CMIC_GP_DATA_OUT);
        val &= ~(1 << 3);
        WRITECSR(CMIC_GP_DATA_OUT, val);

        bcm5340x_reg_get(unit, R_TOP_XG_PLL0_CTRL_6, &val);
        /* msc_ctrl[Bit 12] = 1 to enable LCPLL0 for external PHY */
        val |= (1 << 12);
        bcm5340x_reg_set(unit, R_TOP_XG_PLL0_CTRL_6, val);

        val = READCSR(CMIC_GP_OUT_EN);
        val |= (1 << 3);
        WRITECSR(CMIC_GP_OUT_EN, val);

        val = READCSR(CMIC_GP_DATA_OUT);
        val |= (1 << 3);
        WRITECSR(CMIC_GP_DATA_OUT, val);
    }
#endif /* _BCM95340X_ */

    /* GH PLL setting */
    soc_gh_pll_setting(unit, to_usec);

    /*
     * Bring port blocks out of reset:
     * TOP_XLP0_RST_L~TOP_XLP4_RST_L [Bit 8~4] and TOP_GXP0_RST_L [Bit 3]
     */
    bcm5340x_reg_get(unit, R_TOP_SOFT_RESET_REG, &val);
    val |= 0x1F8;
    bcm5340x_reg_set(unit, R_TOP_SOFT_RESET_REG, val);
    sal_usleep(to_usec);

    /*
     * Bring network sync out of reset
     * TOP_TS_RST_Lf [Bit 15], TOP_SPARE_RST_Lf [Bit 9]
     */
    bcm5340x_reg_get(unit, R_TOP_SOFT_RESET_REG, &val);
    val |= 0x8200;
    bcm5340x_reg_set(unit, R_TOP_SOFT_RESET_REG, val);

    /*
     * Bring network sync PLL out of reset
     * TOP_TS_PLL_RST_Lf [Bit 8] and then TOP_TS_PLL_POST_RST_Lf [Bit 9]
     */
    bcm5340x_reg_get(unit, R_TOP_SOFT_RESET_REG_2, &val);
    val |= 0x100;
    bcm5340x_reg_set(unit, R_TOP_SOFT_RESET_REG_2, val);
    val |= 0x200;
    bcm5340x_reg_set(unit, R_TOP_SOFT_RESET_REG_2, val);

	sal_usleep(to_usec);

    /*
     * Bring IP, EP, and MMU blocks out of reset
     * TOP_IP_RST_L [Bit 0], TOP_EP_RST_Lf [Bit 1], TOP_MMU_RST_Lf [Bit 2]
     */
    bcm5340x_reg_get(unit, R_TOP_SOFT_RESET_REG, &val);
    val |= 0x7;
    bcm5340x_reg_set(unit, R_TOP_SOFT_RESET_REG, val);
    sal_usleep(to_usec);

    bcm5340x_reg_get(unit, R_PGW_CTRL_0, &val);

    if ((BCM53401_DEVICE_ID == gh_sw_info.devid) ||
        (BCM53411_DEVICE_ID == gh_sw_info.devid) ||
        (BCM53456_DEVICE_ID == gh_sw_info.devid) ||
        (BCM53457_DEVICE_ID == gh_sw_info.devid)) {
        if (config_id == 3) {
            /* Enable all PM and PMQ for now */
            val &= ~0x3F;
        } else {
            /* Disable PM_4X10_4 [Bit 5] */
            val &= ~0x1F;
        }
    } else if ((BCM53402_DEVICE_ID == gh_sw_info.devid) ||
        (BCM53412_DEVICE_ID == gh_sw_info.devid)) {
        /* Disable PM_4X10_4 [Bit 5] */
        val &= ~0x0C;
    } else if ((BCM53405_DEVICE_ID == gh_sw_info.devid) ||
        (BCM53415_DEVICE_ID == gh_sw_info.devid)) {
        /* Disable PM4X10Q [Bit 0] and PM_4X10_0 [Bit 1] */
        val &= ~0x3C;
    } else if (BCM53422_DEVICE_ID == gh_sw_info.devid) {
        /* Disable PMQ[Bit 0], PM_4X10_3/4 [Bit 4,5] */
        val &= ~0x0E;
    } else if (BCM53424_DEVICE_ID == gh_sw_info.devid) {
        if (config_id == 3) {
            /* Enable all PM and PMQ for now */
            val &= ~0x3F;
        } else {
            /* Disable PM_4X10_4 [Bit 5] */
            val &= ~0x1F;
        }
    } else if ((BCM53406_DEVICE_ID == gh_sw_info.devid) &&
               (config_id == 3)) {
        /* Disable PM_4X10_4 [Bit 5] */
        val &= ~0x1F;
    } else {
        /* Enable all PM and PMQ for now */
        val &= ~0x3F;
    }

    bcm5340x_reg_get(unit, R_TOP_STRAP_STATUS, &top_strap_val);

    /* Config if some block is power down
     * Bit 5: TSC0 Q, Bit0: TSC0, Bit1: TSC1, Bit2: TSC2, etc..
     * 1 : power off
     * 0 : power on
     */

    tmp_val = ((TOP_STRAP_STATUS_TSC_FIELD(top_strap_val) >> 5) & 0x1) |
               ((TOP_STRAP_STATUS_TSC_FIELD(top_strap_val) << 1) & 0x3e);

    val |= tmp_val;

    /* PGW_CTRL_0:
     * Bit 0: TSCQ
     * Bit 1~ : TSC0~
     */
    bcm5340x_reg_set(unit, R_PGW_CTRL_0, val);

    if ((BCM53401_DEVICE_ID == gh_sw_info.devid) ||
        (BCM53411_DEVICE_ID == gh_sw_info.devid)) {
        if (config_id == 3) {
            int idx;
            for (idx = 0; idx < ((int) (sizeof(speed_limit_53401_op3) / sizeof((speed_limit_53401_op3)[0]))); idx++) {
                bcm5340x_reg_set(unit, R_SPEED_LIMIT_ENTRY_PHYSICAL_PORT_NUMBER(idx), speed_limit_53401_op3[idx]);
            }
        }
    } else if ((BCM53406_DEVICE_ID == gh_sw_info.devid) ||
               (BCM53416_DEVICE_ID == gh_sw_info.devid)) {
        if ((config_id == 2) || (config_id == 3)) {
            int idx;

            for (idx = 0; idx < ((int) (sizeof(speed_limit_53406_op2) / sizeof((speed_limit_53406_op2)[0]))); idx++) {
                bcm5340x_reg_set(unit, R_SPEED_LIMIT_ENTRY_PHYSICAL_PORT_NUMBER(idx), speed_limit_53406_op2[idx]);
            }
        } else if (config_id == 4) {
            int idx;
            for (idx = 0; idx < ((int) (sizeof(speed_limit_53406_op4) / sizeof((speed_limit_53406_op4)[0]))); idx++) {
                bcm5340x_reg_set(unit, R_SPEED_LIMIT_ENTRY_PHYSICAL_PORT_NUMBER(idx), speed_limit_53406_op4[idx]);
            }
        }
    } else if (BCM53456_DEVICE_ID == gh_sw_info.devid) {
        if (config_id != 3) {
            int idx;

            for (idx = 0; idx < ((int) (sizeof(speed_limit_53456_op1) / sizeof((speed_limit_53456_op1)[0]))); idx++) {
                bcm5340x_reg_set(unit, R_SPEED_LIMIT_ENTRY_PHYSICAL_PORT_NUMBER(idx), speed_limit_53456_op1[idx]);
            }
        }
    } else if (BCM53424_DEVICE_ID == gh_sw_info.devid) {
        if (config_id != 3) {
            int idx;

            for (idx = 0; idx < ((int) (sizeof(speed_limit_53456_op1) / sizeof((speed_limit_53456_op1)[0]))); idx++) {
                bcm5340x_reg_set(unit, R_SPEED_LIMIT_ENTRY_PHYSICAL_PORT_NUMBER(idx), speed_limit_53456_op1[idx]);
            }
        }
    }
}

soc_chip_type_t
bcm5340x_chip_type(void)
{
    return SOC_TYPE_SWITCH_XGS;
}

uint8
bcm5340x_port_count(uint8 unit)
{
    if (unit > 0) {
        return 0;
    }
    return SOC_PORT_COUNT(unit);
}

sys_error_t
bcm5340x_link_status(uint8 unit, uint8 port, BOOL *link)
{
    if (link == NULL || unit > 0 || port > BCM5340X_LPORT_MAX) {
        return SYS_ERR_PARAMETER;
    }

    *link = gh_sw_info.link[port];

    return SYS_OK;
}

static int
bcm5340x_sw_op(uint8 unit,
               uint32 op,
               uint8 block_id,
               uint32 addr,
               uint32 *buf,
               int len)
{
    uint32 msg_hdr;
    uint32 ctrl;
    int i;

    if (buf == NULL || unit > 0) {
        return -1;
    }

    msg_hdr = (V_SMHDR_OP_CODE(op) | V_SMHDR_DEST_BLOCK(block_id));

    if (op != SC_OP_RD_MEM_CMD) {
        msg_hdr |= V_SMHDR_DATA_LEN(len*4);
    }

    SCHAN_LOCK(unit);

    WRITECSR(R_CMIC_SCHAN_D(0), msg_hdr);
    WRITECSR(R_CMIC_SCHAN_D(1), addr);

    if (op == SC_OP_WR_REG_CMD || op == SC_OP_WR_MEM_CMD) {
        for (i = 0; i < len; i++) {
            WRITECSR(R_CMIC_SCHAN_D(2+i), buf[i]);
        }
    }

    WRITECSR(CMIC_CMC1_SCHAN_CTRL, SC_CMCx_MSG_START);

    for (i = 0; i < 100; i++) {
        sal_usleep(2);
        ctrl = READCSR(CMIC_CMC1_SCHAN_CTRL);
        if (ctrl & SC_CMCx_MSG_DONE) {
            break;
        }
    }

#if CFG_CONSOLE_ENABLED
    if ((i == 100) || ((ctrl & SC_CMCx_MSG_ERROR_MASK) != 0)) {
        sal_printf("S-CHAN op=0x%x, %d:0x%x, error(%d-0x%08x)\n", op, block_id, addr, i, ctrl);
    }
#endif /* CFG_CONSOLE_ENABLED */

    ctrl = READCSR(CMIC_CMC1_SCHAN_CTRL);
    ctrl &= ~SC_CMCx_MSG_DONE;
    WRITECSR(CMIC_CMC1_SCHAN_CTRL, ctrl);

    if (op == SC_OP_RD_REG_CMD || op == SC_OP_RD_MEM_CMD) {
        for (i = 0; i < len; i++) {
            buf[i] = READCSR(R_CMIC_SCHAN_D(1+i));
        }
    }

    SCHAN_UNLOCK(unit);
    return 0;
}

sys_error_t
bcm5340x_phy_reg_get(uint8 unit, uint8 port,
                           uint16 reg_addr, uint16 *p_value)
{
    return SYS_OK;
}

sys_error_t
bcm5340x_phy_reg_set(uint8 unit, uint8 port,
                           uint16 reg_addr, uint16 value)
{
    return SYS_OK;
}

sys_error_t
bcm5340x_reg_get(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *val)
{
	return bcm5340x_sw_op(unit, SC_OP_RD_REG_CMD, block_id, addr, val, 1);
}

sys_error_t
bcm5340x_reg_set(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 val)
{
	return bcm5340x_sw_op(unit, SC_OP_WR_REG_CMD, block_id, addr, &val, 1);
}

sys_error_t
bcm5340x_reg64_get(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *val,
                 int len)
{
	return bcm5340x_sw_op(unit, SC_OP_RD_REG_CMD, block_id, addr, val, len);
}

sys_error_t
bcm5340x_reg64_set(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *buf,
                 int len)
{
	return bcm5340x_sw_op(unit, SC_OP_WR_REG_CMD, block_id, addr, buf, len);
}

sys_error_t
bcm5340x_mem_get(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *buf,
                 int len)
{
	return bcm5340x_sw_op(unit, SC_OP_RD_MEM_CMD, block_id, addr, buf, len);
}

sys_error_t
bcm5340x_mem_set(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *buf,
                 int len)
{
    return bcm5340x_sw_op(unit, SC_OP_WR_MEM_CMD, block_id, addr, buf, len);
}

sys_error_t
bcm5340x_chip_revision(uint8 unit, uint16 *dev, uint16 *rev)
{
    uint32 val;

    if (unit > 0) {
        return -1;
    }

    val = READCSR(CMIC_DEV_REV_ID);
    gh_sw_info.devid = val & 0xFFFF;
    gh_sw_info.revid = val >> 16;

    return 0;
}

static void
bcm5340x_load_led_program(uint8 unit)
{
#ifdef CFG_LED_MICROCODE_INCLUDED
    uint8 *led_program;
    int i, offset, led_code_size;
    uint32 addr, val;
    int byte_count = 0;
    uint8 led_option = 1;    
    uint8 led_program_3[256];
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED    
    sys_error_t sal_config_rv = SYS_OK;
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
    
    /* Default is for 53405, if is another device, needs to update it*/
    /* Remap for embedded_16x10g TDM
     * CMIC_LEDUP0_PORT_ORDER_REMAP_0_3    {8'd00, 6'd34,  6'd35,  6'd36,  6'd37});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_4_7,   {8'd00, 6'd30,  6'd31,  6'd32,  6'd33});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_8_11,  {8'd00, 6'd26,  6'd27,  6'd28,  6'd29});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_12_15, {8'd00, 6'd22,  6'd23,  6'd24,  6'd25});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_16_19, {8'd00, 6'd00,  6'd38,  6'd00,  6'd00});
     */
    uint32 port_remap[9] = { 0x008a3925, 0x0079f821, 0x0069b71d, 0x00597619,
                             0x00026000, 0x0, 0x0, 0x0, 0x0 };
                             
    if (BCM53456_DEVICE_ID == gh_sw_info.devid) {
        if (config_id == 1) {
        
    /* Remap for TDM 53456
     * CMIC_LEDUP0_PORT_ORDER_REMAP_0_3    {8'd00, 6'd31,  6'd32,  6'd33,  6'd00});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_4_7,   {8'd00, 6'd27,  6'd28,  6'd29,  6'd30});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_8_11,  {8'd00, 6'd23,  6'd24,  6'd25,  6'd26});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_12_15, {8'd00, 6'd19,  6'd20,  6'd21,  6'd22});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_16_19, {8'd00, 6'd04,  6'd03,  6'd02,  6'd18});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_20_23, {8'd00, 6'd08,  6'd07,  6'd06,  6'd05});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_24_27, {8'd00, 6'd12,  6'd11,  6'd10,  6'd09});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_28_31, {8'd00, 6'd16,  6'd15,  6'd14,  6'd13});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_32_35, {8'd00, 6'd00,  6'd00,  6'd00,  6'd17});
     */
            port_remap[0] = 0x007E0840;
            port_remap[1] = 0x006DC75E;
            port_remap[2] = 0x005D865A;
            port_remap[3] = 0x004D4556;
            port_remap[4] = 0x00103092;
            port_remap[5] = 0x00207185;
            port_remap[6] = 0x0030B289;
            port_remap[7] = 0x40F38D;
            port_remap[8] = 0x11;
        }
    } else if (BCM53456_DEVICE_ID == gh_sw_info.devid) {
        if (config_id == 1) {
        
    /* Remap for TDM 53406
     * CMIC_LEDUP0_PORT_ORDER_REMAP_0_3    {8'd00, 6'd34,  6'd35,  6'd36,  6'd37});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_4_7,   {8'd00, 6'd30,  6'd31,  6'd32,  6'd33});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_8_11,  {8'd00, 6'd26,  6'd27,  6'd28,  6'd29});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_12_15, {8'd00, 6'd22,  6'd23,  6'd24,  6'd25});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_16_19, {8'd00, 6'd18,  6'd19,  6'd20,  6'd21});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_20_23, {8'd00, 6'd02,  6'd03,  6'd04,  6'd05});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_24_27, {8'd00, 6'd00,  6'd00,  6'd00,  6'd00});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_28_31, {8'd00, 6'd00,  6'd00,  6'd00,  6'd00});
     * CMIC_LEDUP0_PORT_ORDER_REMAP_32_35, {8'd00, 6'd00,  6'd00,  6'd00,  6'd00});
     */
            port_remap[0] = 0xD35DB7;
            port_remap[1] = 0xC31CB3;
            port_remap[2] = 0x9A7A29;
            port_remap[3] = 0x8A3925;
            port_remap[4] = 0x619821;
            port_remap[5] = 0x83105;
            port_remap[6] = 0;
            port_remap[7] = 0;
            port_remap[8] = 0;
        }
    }
    
    
    
    for (i = 0, addr = CMIC_LEDUP0_PORT_ORDER_REMAP_0_3; i < 9; i++, addr += 4) {
        WRITECSR(addr, port_remap[i]);
    }
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
    sal_config_rv = sal_config_uint8_get(SAL_CONFIG_LED_OPTION, &led_option);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Overwrite led option with value %d.\n", led_option);
    }

    byte_count = sal_config_bytes_get(SAL_CONFIG_LED_PROGRAM, led_program_3, 256);
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */

    if (BCM53405_DEVICE_ID == gh_sw_info.devid) {
        if (led_option == 2) {
            led_program = led_embedded_16x10g_2;
            led_code_size = sizeof(led_embedded_16x10g_2);
        } else if ((led_option == 3) && byte_count) {
            sal_printf("Vendor Config : Load customer LED ucdoe with length %d.\n", byte_count);
            led_program = led_program_3;
            led_code_size = sizeof(led_program_3);
        } else {
            led_program = led_embedded_16x10g_1;
            led_code_size = sizeof(led_embedded_16x10g_1);
        }
    } else if ((BCM53456_DEVICE_ID == gh_sw_info.devid) &&
        (config_id == 1)) {
        if (led_option == 2) {
            led_program = led_cascade_cfg2;
            led_code_size = sizeof(led_cascade_cfg2);
        } else if ((led_option == 3) && byte_count) {
            sal_printf("Vendor Config : Load customer LED ucdoe with length %d.\n", byte_count);
            led_program = led_program_3;
            led_code_size = sizeof(led_program_3);
        } else {
            led_program = led_cascade_cfg1;
            led_code_size = sizeof(led_cascade_cfg1);
        }
    } else if ((BCM53406_DEVICE_ID == gh_sw_info.devid) &&
        (config_id == 1)) {
        if (led_option == 2) {
            led_program = led_mixed_embedded_2;
            led_code_size = sizeof(led_mixed_embedded_2);
        } else if ((led_option == 3) && byte_count) {
            sal_printf("Vendor Config : Load customer LED ucdoe with length %d.\n", byte_count);
            led_program = led_program_3;
            led_code_size = sizeof(led_program_3);
        } else {
            led_program = led_mixed_embedded_1;
            led_code_size = sizeof(led_mixed_embedded_1);
        }
    } else {
            sal_printf("No serial led support. \n");
			return;
    }

#define LED_RAM_SIZE     0x100

    for (offset = 0; offset < LED_RAM_SIZE; offset++) {
        WRITECSR(CMIC_LEDUP_PROGRAM_RAM_D(offset),
                      (offset >= led_code_size) ? 0 : *(led_program + offset));

        WRITECSR(CMIC_LEDUP_DATA_RAM_D(offset), 0);
    }

    /* enable LED processor */
    val = READCSR(CMIC_LEDUP0_CTRL);
    val |= 0x1;
    WRITECSR(CMIC_LEDUP0_CTRL, val);
#endif /* CFG_LED_MICROCODE_INCLUDED */
}

sys_error_t
bcm5340x_l2_op(uint8 unit,
               l2x_entry_t *entry,
               uint8 op_code
               )

{
    uint32 l2_entry[4];
    uint32 msg_hdr[2] = {0x90500800, 0x1c000000};
    uint32 ctrl;
    int i;

	l2_entry[0] = ((entry->vlan_id << 3) |
                   (entry->mac_addr[5] << 15) |
                   (entry->mac_addr[4] << 23) |
                   (entry->mac_addr[3] << 31));

	l2_entry[1] = (((entry->mac_addr[3] >> 1) & 0x0000007F) |
                   (entry->mac_addr[2] << 7) |
                   (entry->mac_addr[1] << 15) |
                   (entry->mac_addr[0] << 23) |
                   (entry->port << 31));

    l2_entry[2] = ((entry->port >> 1) & 0x0000001F);
    /* set static and valid bit */
    l2_entry[3] = (1 << 4) | (1 << 2);

    if (op_code == SC_OP_L2_DEL_CMD) {
         msg_hdr[0] = 0x98500800;
    }

    WRITECSR(R_CMIC_SCHAN_D(0), msg_hdr[0]);
    WRITECSR(R_CMIC_SCHAN_D(1), msg_hdr[1]);
    WRITECSR(R_CMIC_SCHAN_D(2), l2_entry[0]);
    WRITECSR(R_CMIC_SCHAN_D(3), l2_entry[1]);
    WRITECSR(R_CMIC_SCHAN_D(4), l2_entry[2]);
    WRITECSR(R_CMIC_SCHAN_D(5), l2_entry[3]);

    WRITECSR(CMIC_CMC1_SCHAN_CTRL, SC_CMCx_MSG_START);

    for (i = 0; i < 100; i++) {
        sal_usleep(2);
        ctrl = READCSR(CMIC_CMC1_SCHAN_CTRL);
        if (ctrl & SC_CMCx_MSG_DONE)
            break;
    }

    return SYS_OK;
}

#define JUMBO_FRM_SIZE (9216)

static void
enable_jumbo_frame(uint8 unit)
{
    int lport;
    uint32 entry[2];
    SOC_LPORT_ITER(lport) {
        if (IS_GX_PORT(lport)) {
            bcm5340x_reg_set(unit, SOC_PORT_BLOCK(lport),
                R_FRM_LENGTH(SOC_PORT_BLOCK_INDEX(lport)), JUMBO_FRM_SIZE);
        } else if (IS_XL_PORT(lport)) {
            /* Set jumbo max size */
            entry[0] = JUMBO_FRM_SIZE;
            entry[1] = 0x0;
            bcm5340x_reg64_set(unit, SOC_PORT_BLOCK(lport),
                               R_XLMAC_RX_MAX_SIZE(SOC_PORT_BLOCK_INDEX(lport)),
                               entry, 2);

        }
    }
}

static void
soc_pipe_mem_clear(uint8 unit)
{
    uint32 val;
    /*
     * Reset the IPIPE and EPIPE block
     */
    bcm5340x_reg_set(unit, R_ING_HW_RESET_CONTROL_1, 0x0);
    /*
     * Set COUNT[Bit 15-0] to # entries in largest IPIPE table, L2_ENTRYm 0x4000
     * RESET_ALL [Bit 16] = 1, VALID [Bit 17] = 1
     */
    bcm5340x_reg_set(unit, R_ING_HW_RESET_CONTROL_2, 0x34000);

    bcm5340x_reg_set(unit, R_EGR_HW_RESET_CONTROL_0, 0x0);
    /*
     * Set COUNT[Bit 15-0] to # entries in largest EPIPE table, EGR_VLAN 0x1000
     * RESET_ALL [Bit 16] = 1, VALID [Bit 17] = 1
     */
    bcm5340x_reg_set(unit, R_EGR_HW_RESET_CONTROL_1, 0x31000);

    /* Wait for IPIPE memory initialization done. */
    do {
        /* Polling DONE[Bit 18] */
        bcm5340x_reg_get(unit, R_ING_HW_RESET_CONTROL_2, &val);
        if (val & (0x1 << 18)) {
            break;
        }
        
        /*  soc_cm_debug(DK_WARN, "unit %d : ING_HW_RESET timeout\n", unit); */
    } while (1);

    /* Wait for EPIPE memory initialization done. */
    do {
        /* Polling DONE[Bit 18] */
        bcm5340x_reg_get(unit, R_EGR_HW_RESET_CONTROL_1, &val);
        if (val & (0x1 << 18)) {
            break;
        }

        
        /*  soc_cm_debug(DK_WARN, "unit %d : EGR_HW_RESET timeout\n", unit); */
    } while (1);

    bcm5340x_reg_set(unit, R_ING_HW_RESET_CONTROL_2, 0x0);
    bcm5340x_reg_set(unit, R_EGR_HW_RESET_CONTROL_1, 0x0);

    
}

static void
soc_port_block_info_get(uint8 unit, uint8 pport, int *block_type, int *block_idx, int *port_idx)
{
    *block_type = PORT_BLOCK_TYPE_XLPORT;
    if ((pport >= PHY_XLPORT4_BASE) && (pport <= BCM5340X_PORT_MAX)) {
        *block_idx = XLPORT4_BLOCK_ID;
        *port_idx = (pport - PHY_XLPORT4_BASE) & 0x3;
    } else if (pport >= PHY_XLPORT3_BASE) {
        *block_idx = XLPORT3_BLOCK_ID;
        *port_idx = (pport - PHY_XLPORT3_BASE) & 0x3;
    } else if (pport >= PHY_XLPORT2_BASE) {
        *block_idx = XLPORT2_BLOCK_ID;
        *port_idx = (pport - PHY_XLPORT2_BASE) & 0x3;
    } else if (pport >= PHY_XLPORT1_BASE) {
        *block_idx = XLPORT1_BLOCK_ID;
        *port_idx = (pport - PHY_XLPORT1_BASE) & 0x3;
    } else if (pport >= PHY_XLPORT0_BASE) {
        *block_idx = XLPORT0_BLOCK_ID;
        *port_idx = (pport - PHY_XLPORT0_BASE) & 0x3;
    } else if (pport >= PHY_GXPORT1_BASE) {
        *block_idx = GXPORT1_BLOCK_ID;
        *port_idx = (pport - PHY_GXPORT1_BASE) & 0x7;
        *block_type = PORT_BLOCK_TYPE_GXPORT;
    } else if (pport >= PHY_GXPORT0_BASE) {
        if ((gh_sw_info.devid == BCM53401_DEVICE_ID) ||
            (gh_sw_info.devid == BCM53411_DEVICE_ID) ||
            (gh_sw_info.devid == BCM53456_DEVICE_ID) ||
            (gh_sw_info.devid == BCM53457_DEVICE_ID) ||
            (gh_sw_info.devid == BCM53424_DEVICE_ID)) {
            *block_idx = GXPORT0_BLOCK_ID;
            *port_idx = (pport - PHY_GXPORT0_BASE) & 0x7;
            *block_type = PORT_BLOCK_TYPE_GXPORT;
        } else {
            *block_idx = XLPORT5_BLOCK_ID;
            *port_idx = (pport - BCM5340X_PORT_MIN) & 0x3;
        }
    }
}

static sys_error_t
soc_init_port_mapping(uint8 unit)
{
    int port_cnt = 0;
    int i, j, tsc_idx;
    const int *p2l_mapping = 0;
    const int *speed_max = 0;
    int tmp_speed_max[BCM5340X_PORT_MAX + 1];
    uint32 val;
    int block_start_port;

    switch (gh_sw_info.devid) {
        case BCM53401_DEVICE_ID:
        case BCM53411_DEVICE_ID:
        case BCM53456_DEVICE_ID:
        case BCM53457_DEVICE_ID:
            /*
             * Option 1 : 4xQSGMII + 8x1G + 4x10G
             * Option 2 : 4xQSGMII + 8x1G + 2x10G + 2x13G
             * Option 3 : 2xQSGMII + 16x1G + 4x10G
             */
            if (config_id == 3) {
                p2l_mapping = p2l_mapping_cascade_1;
                speed_max = port_speed_max_cascade_1;
            } else if (config_id == 2) {
                p2l_mapping = p2l_mapping_cascade;
                speed_max = port_speed_max_cascade;
            } else if (config_id == 1) {
                p2l_mapping = p2l_mapping_cascade;
                speed_max = port_speed_max_non_cascade_2p5;
            } else {
                sal_printf("\nERROR : CFG_CONFIG_OPTION %d is not supported for devid 0x%x in UM software !\n", CFG_CONFIG_OPTION, gh_sw_info.devid);
                return SYS_ERR_NOT_FOUND;
            }
           break;
        case BCM53402_DEVICE_ID:
        case BCM53412_DEVICE_ID:
           /* 8 x 10G */
           p2l_mapping = p2l_mapping_embedded_8x10g;
           speed_max = port_speed_max_embedded_8x10g;
           break;
        case BCM53405_DEVICE_ID:
        case BCM53415_DEVICE_ID:
           /* 16 x 10G */
           p2l_mapping = p2l_mapping_embedded_16x10g;
           speed_max = port_speed_max_embedded_16x10g;
           break;
        case BCM53406_DEVICE_ID:
        case BCM53416_DEVICE_ID:
            /*
             * Option 1 : 12x10G + 8x2.5G + 4x2.5G/5G
             * Option 2 : 4xXAUI + 8x10G
             * Option 3 : 4xXAUI + 4x10G
             * Option 4 : 15x1G/2.5G/5G/10G + 9x1G
             */
            if (config_id == 3) {
                p2l_mapping = p2l_mapping_53406_op3;
                speed_max = port_speed_max_53406_op2;
            } else if (config_id == 4) {
                p2l_mapping = p2l_mapping_mixed_embeded;
                speed_max = port_speed_max_53406_op4;
            } else if (config_id == 2) {
                p2l_mapping = p2l_mapping_53406_op2;
                speed_max = port_speed_max_53406_op2;
            } else if (config_id == 1) {
                p2l_mapping = p2l_mapping_mixed_embeded;
                speed_max = port_speed_max_mixed_embeded;
            } else {
                sal_printf("\nERROR : CFG_CONFIG_OPTION %d is not supported for devid 0x%x in UM software !\n", CFG_CONFIG_OPTION, gh_sw_info.devid);
                return SYS_ERR_NOT_FOUND;
            }
            break;
        case BCM53408_DEVICE_ID:
        case BCM53418_DEVICE_ID:
            /* 24 x 2.5G */
            p2l_mapping = p2l_mapping_mixed_embeded;
            speed_max = port_speed_max_mixed_embeded_24x2p5;
            break;
       case BCM53454_DEVICE_ID:
       case BCM53455_DEVICE_ID:
            /* 20 x 2.5G + 4 X 10G */
            p2l_mapping = p2l_mapping_cascade_1;
            speed_max = port_speed_max_53454;
            break;
       case BCM53422_DEVICE_ID:
            p2l_mapping = p2l_mapping_cascade_1;
            speed_max = port_speed_max_53422;
            break;
       case BCM53424_DEVICE_ID:
           /*
            * Option 1 : 4xQSGMII + 8x1G + 4x1G/2.5G/5G/10G
            * Option 2 : 4xQSGMII + 8x1G + 2x10G + 2x13G
            * Option 3 : 2xQSGMII + 16x1G + 4x1G/2.5G/5G/10G
            */
            if (config_id == 3) {
                p2l_mapping = p2l_mapping_cascade_1;
                speed_max = port_speed_max_53424;
            } else if (config_id == 2) {
                p2l_mapping = p2l_mapping_cascade;
                speed_max = port_speed_max_cascade;
            } else if (config_id == 1) {
                p2l_mapping = p2l_mapping_cascade;
                speed_max = port_speed_max_non_cascade;
            } else {
                sal_printf("\nERROR : CFG_CONFIG_OPTION %d is not supported for devid 0x%x in UM software !\n", CFG_CONFIG_OPTION, gh_sw_info.devid);
                return SYS_ERR_NOT_FOUND;
            }
          break;
       case BCM53426_DEVICE_ID:
          p2l_mapping = p2l_mapping_cascade_1;
          speed_max = port_speed_max_53426;
          break;
    }

    for (i = 0; i <= BCM5340X_PORT_MAX ; i++) {

        tmp_speed_max[i] = speed_max[i];
    }

    /* Overwrite tmp_speed_max table if the TSC module is power off */
    bcm5340x_reg_get(unit, R_TOP_STRAP_STATUS, &val);

    /* TOP_STRAP_STATUS: bit26(TSC0)~bit31(TSCQ)
     * from BCM5340X_PORT_MIN to (PHY_XLPORT0_BASE - 1) is TSCQ
     * from PHY_XLPORT0_BASE~: TSC0~
     * 4 ports per TSC core
     */
    if (((TOP_STRAP_STATUS_TSC_FIELD(val) >> 5) & 0x1) == 1) {
        for (i = BCM5340X_PORT_MIN; i < PHY_XLPORT0_BASE; i++) {
            tmp_speed_max[i] = -1;
        }
    }
    for (tsc_idx = 0; tsc_idx <= 4; tsc_idx++) {
        if ((TOP_STRAP_STATUS_TSC_FIELD(val)) & (0x1<<tsc_idx)) {
            j = PHY_XLPORT0_BASE + 4 * tsc_idx;
            for (i = j; i <= (j + 3); i++) {
                tmp_speed_max[i] = -1;
            }
        }
    }

    /* Get SOC_PORT_COUNT from speed_max table */
    port_cnt = 0;
    for (i = 0; i <= BCM5340X_PORT_MAX ; i++) {
        if((tmp_speed_max[i] > 0) && (p2l_mapping[i] > 0)) {
            port_cnt++;
        }
    }
    SOC_PORT_COUNT(unit) = port_cnt;

    for (i = 0 ; i <= BCM5340X_LPORT_MAX ; i++) {
        gh_sw_info.port_l2p_mapping[i] = -1;
    }

    for (i = 0; i <= BCM5340X_PORT_MAX ; i++) {
        gh_sw_info.port_p2l_mapping[i] = p2l_mapping[i];
        if (p2l_mapping[i] != -1) {
            if (tmp_speed_max[i] != -1) {
                gh_sw_info.port_l2p_mapping[p2l_mapping[i]] = i;
            } else if (i == 0) {
                gh_sw_info.port_l2p_mapping[p2l_mapping[i]] = 0;
            } else {
                gh_sw_info.port_l2p_mapping[p2l_mapping[i]] = -1;
            }
        }
    }

    /* Ingress physical to logical port mapping */
    for (i = 0; i <= BCM5340X_PORT_MAX; i++) {
        val = (p2l_mapping[i] == -1) ? 0x1F: p2l_mapping[i];
        bcm5340x_mem_set(unit, M_ING_PHYSICAL_TO_LOGICAL_PORT_NUMBER_MAPPING_TABLE(i), &val, 1);
        if (tmp_speed_max[i] != -1) {
            if (tmp_speed_max[i] >= 100) {
                gh_sw_info.port_speed_max[p2l_mapping[i]] = 10000;
            }
            else if (tmp_speed_max[i] >= 50) {
                gh_sw_info.port_speed_max[p2l_mapping[i]] = 5000;
            }
            else if (tmp_speed_max[i] >= 25) {
                gh_sw_info.port_speed_max[p2l_mapping[i]] = 2500;
            }
            else {
                gh_sw_info.port_speed_max[p2l_mapping[i]] = 1000;
            }
        }
    }

    /* Egress logical to physical port mapping, needs a way for maximum logical port? */
    for (i = 0; i <= BCM5340X_LPORT_MAX; i++) {
        val = (gh_sw_info.port_l2p_mapping[i] == -1) ? 0x3F : gh_sw_info.port_l2p_mapping[i];
        bcm5340x_reg_set(unit, R_EGR_LOGICAL_TO_PHYSICAL_PORT_NUMBER_MAPPING(i), val);
        /* MMU logical to physical port mapping
         * (Here, Same as Egress logical to physical port mapping)
         */
        if (val != 0x3F) {
            bcm5340x_reg_set(unit, R_LOG_TO_PHY_PORT_MAPPING(i), val);
        }
    }

    SOC_LPORT_ITER(i) {
        soc_port_block_info_get(unit, SOC_PORT_L2P_MAPPING(i),
                                &SOC_PORT_BLOCK_TYPE(i),
                                &SOC_PORT_BLOCK(i), &SOC_PORT_BLOCK_INDEX(i));

        BCM5340X_ALL_PORTS_MASK |= (0x1 << i);
    }

    SOC_LPORT_ITER(i) {
        block_start_port = SOC_PORT_L2P_MAPPING(i) - SOC_PORT_BLOCK_INDEX(i);
        SOC_PORT_LANE_NUMBER(i) = 4;
        if (p2l_mapping[block_start_port + 1] != -1) {
            SOC_PORT_LANE_NUMBER(i) = 1;
        } else if (p2l_mapping[block_start_port + 2] != -1) {
            if ((BCM53401_DEVICE_ID == gh_sw_info.devid) ||
                (BCM53411_DEVICE_ID == gh_sw_info.devid) ||
                (BCM53456_DEVICE_ID == gh_sw_info.devid) ||
                (BCM53457_DEVICE_ID == gh_sw_info.devid) ||
                (BCM53424_DEVICE_ID == gh_sw_info.devid)) {
                SOC_PORT_LANE_NUMBER(i) = 1;
            } else  {
                SOC_PORT_LANE_NUMBER(i) = 2;
            }
        }
    }

    return SYS_OK;
}

static void
soc_tsc_xgxs_reset(uint8 unit, uint8 port)
{
    uint32      val;
#if CONFIG_GREYHOUND_EMULATION
    int         sleep_usec = 50000;
    int         lcpll = 0;
#else
    int         sleep_usec = 1100;
    int         lcpll = 1;
#endif /* CONFIG_GREYHOUND_EMULATION */
    uint8_t block_id;

    if (IS_GX_PORT(port)) {
        block_id = XLPORT5_BLOCK_ID;
    } else {
        block_id = SOC_PORT_BLOCK(port);
    }

    /*
     * Reference clock selection: REFIN_ENf [Bit 2]
     */
    bcm5340x_reg_get(unit, block_id, R_XLPORT_XGXS0_CTRL_REG, &val);
    val &= ~(0x1 << 2);
    val |= (lcpll << 2);
    bcm5340x_reg_set(unit, block_id, R_XLPORT_XGXS0_CTRL_REG, val);

    /* Deassert power down [Bit 3]*/
    val &= ~(0x1 << 3);
    bcm5340x_reg_set(unit, block_id, R_XLPORT_XGXS0_CTRL_REG, val);
    sal_usleep(sleep_usec);

    /* Bring XGXS out of reset: RstB_HW[Bit 0] */
    val |= 0x1;
    bcm5340x_reg_set(unit, block_id, R_XLPORT_XGXS0_CTRL_REG, val);
    sal_usleep(sleep_usec);
}

static void
soc_gh_tsc_reset(uint8 unit)
{
    uint8 lport;

    /* TSC reset */
    SOC_LPORT_ITER(lport) {
        if ((IS_XL_PORT(lport) && (SOC_PORT_BLOCK_INDEX(lport) == 0)) ||
            (IS_GX_PORT(lport) && (lport == 2))){
            soc_tsc_xgxs_reset(unit, lport);
        }
    }

    /* MAC reset */
    SOC_LPORT_ITER(lport) {
        if (IS_XL_PORT(lport) && (SOC_PORT_BLOCK_INDEX(lport) == 0)) {
            uint32 val;
            bcm5340x_reg_get(unit, SOC_PORT_BLOCK(lport), R_XLPORT_MAC_CONTROL, &val);
            /* XMAC0_RESETf: [Bit 0] */
            val |= 0x1;
            bcm5340x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_MAC_CONTROL, val);
            sal_usleep(10);
            val &= ~0x1;
            bcm5340x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_MAC_CONTROL, val);
        }
    }
}

static void
soc_misc_init(uint8 unit)
{
#define NUM_XLPORT 4
    int i, bindex, lport, valid_port;
    uint32 rst_val, val, mode, top_strap_val;

    soc_pipe_mem_clear(unit);

    /* Reset XLPORT MIB counter */
     SOC_LPORT_ITER(lport) {
        if (IS_XL_PORT(lport) && (SOC_PORT_BLOCK_INDEX(lport) == 0)) {
            bcm5340x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_MIB_RESET, 0xf);
            bcm5340x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_MIB_RESET, 0x0);
        }
    }

    

    /* set QMODE enable for TSC0Q at QSGMII mode before unimac init process
         *  - GH's QSGMII in TSC0Q only and check proper SKU to enable QMODE.
         */
    bcm5340x_reg_get(unit, R_TOP_STRAP_STATUS, &top_strap_val);
    if (((TOP_STRAP_STATUS_TSC_FIELD(top_strap_val) >> 5) & 0x1 ) == 0) {
        if ((BCM53401_DEVICE_ID == gh_sw_info.devid) ||
            (BCM53411_DEVICE_ID == gh_sw_info.devid) ||
            (BCM53456_DEVICE_ID == gh_sw_info.devid) ||
            (BCM53457_DEVICE_ID == gh_sw_info.devid) ||
            (BCM53424_DEVICE_ID == gh_sw_info.devid)) {
            bcm5340x_reg_get(unit, R_CHIP_CONFIG, &val);
            val |= 0x1;
            bcm5340x_reg_set(unit, R_CHIP_CONFIG, val);
        }
    }

    /* GMAC init */
    SOC_LPORT_ITER(i) {
        if (IS_GX_PORT(i) && (SOC_PORT_BLOCK_INDEX(i) == 0)) {
            /* Clear counter and enable gport */
            bcm5340x_reg_set(unit, SOC_PORT_BLOCK(i), R_GPORT_CONFIG, 0x3);
        }
    }

    SOC_LPORT_ITER(i) {
        if (IS_GX_PORT(i) && (SOC_PORT_BLOCK_INDEX(i) == 0)) {
            /* Enable gport */
            bcm5340x_reg_set(unit, SOC_PORT_BLOCK(i), R_GPORT_CONFIG, 0x1);
        }
    }

    

    SOC_LPORT_ITER(lport) {
        if (IS_XL_PORT(lport) && (SOC_PORT_BLOCK_INDEX(lport) == 0)) {
            i = SOC_PORT_L2P_MAPPING(lport);
            rst_val = valid_port = 0;
            for (bindex = 0; bindex < NUM_XLPORT; bindex++) {
                if (SOC_PORT_P2L_MAPPING(i + bindex) != -1) {
                    rst_val |= (0x1 << bindex);
                    valid_port++;
                }
            }
            /* XPORT0_CORE_PORT_MODE [Bit 5:3] and XPORT0_PHY_PORT_MODE [Bit 2:0] */
            switch(valid_port){
                case (1):
                    mode = SOC_GH_PORT_MODE_SINGLE;
                    break;
                case (2):
                    /* Note: Dual mode if max speed > 10000 */
                    mode = SOC_GH_PORT_MODE_QUAD;
                    break;
                case (4):
                default:
                    mode = SOC_GH_PORT_MODE_QUAD;
                break;
            }
            val = (mode << 3) | mode;
            bcm5340x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_MODE_REG, val);

            /* Enable XLPORT */
            bcm5340x_reg_set(unit, SOC_PORT_BLOCK(lport), R_XLPORT_ENABLE_REG, rst_val);
        }
    }

#if !CONFIG_GREYHOUND_ROMCODE
    /* Metering Clock [Bit 5] */
    bcm5340x_reg_get(unit, R_MISCCONFIG, &val);
    val |= 0x20;
    bcm5340x_reg_set(unit, R_MISCCONFIG, val);

    /* Enable dual hash on L2 and L3 tables */
    /* HASH_SELECT[Bit3:1] = FB_HASH_CRC32_LOWER(2), INSERT_LEAST_FULL_HALF[Bit 0] = 1 */
    bcm5340x_reg_set(unit, R_L2_AUX_HASH_CONTROL, 0x15);
    bcm5340x_reg_set(unit, R_L3_AUX_HASH_CONTROL, 0x15);
#endif

    /* Egress Enable */
    val = 0x1;
    SOC_LPORT_ITER(lport) {
        bcm5340x_mem_set(unit, M_EGR_ENABLE(SOC_PORT_L2P_MAPPING(lport)), &val, 1);
    }
    

    /*
     * Set reference clock (based on 200MHz core clock)
     * to be 200MHz * (1/40) = 5MHz
     */
    WRITECSR(CMIC_RATE_ADJUST_EXT_MDIO, 0x10028);
    /* Match the Internal MDC freq with above for External MDC */
    WRITECSR(CMIC_RATE_ADJUST_INT_MDIO, 0x10028);


    /* The HW defaults for EGR_VLAN_CONTROL_1.VT_MISS_UNTAG == 1, which
     * causes the outer tag to be removed from packets that don't have
     * a hit in the egress vlan tranlation table. Set to 0 to disable this.
     */
    for (i = BCM5340X_LPORT_MIN; i <= BCM5340X_LPORT_MAX; i++) {
        bcm5340x_reg_set(unit, R_EGR_VLAN_CONTROL_1(i), 0x0);
    }

#if !CONFIG_GREYHOUND_ROMCODE
    /* Enable SRC_REMOVAL_EN[Bit 0] and LAG_RES_EN[Bit2] */
    bcm5340x_reg_set(unit, R_SW2_FP_DST_ACTION_CONTROL, 0x5);
#endif

    

    /*(GH-1622) XQ IPMC group table controllers only initialize address
     *0~63 of group table.
     */
#define MMU_IPMC_GROUP_TBL_MAXIDX 256
    val = 0;
    for (lport = BCM5340X_LPORT_MIN; lport <= BCM5340X_LPORT_MAX; lport++) {
        for (bindex = 0; bindex < MMU_IPMC_GROUP_TBL_MAXIDX; bindex++)  {
            bcm5340x_mem_set(unit, M_MMU_IPMC_GROUP_TBL(lport,bindex), &val, 1);
        }
    }

    if ((BCM53405_DEVICE_ID == gh_sw_info.devid) ||
        (BCM53415_DEVICE_ID == gh_sw_info.devid)) {
        /* Shutdown GP,XP0~3(Bit[1-5]) */
        bcm5340x_reg_set(unit, R_EGR_PORT_BUFFER_CLK_SHUTDOWN, 0x3E);
    }
}

static void
soc_mmu_init(uint8 unit)
{
    int i, j;
    uint32 addr, val;
    int tdm_size;
    const uint32 *arr = NULL;

    /* TDM initialization */
    switch (gh_sw_info.devid) {
        case BCM53454_DEVICE_ID:
        case BCM53455_DEVICE_ID:
            tdm_size = 48;
            arr = gh_tdm_elkhound_53454;
            break;
        case BCM53456_DEVICE_ID:
        case BCM53457_DEVICE_ID:
            if (config_id == 3) {
                tdm_size = 48;
                arr = gh_tdm_elkhound_53454;
            } else {
                tdm_size = 98;
                arr = gh_tdm_cascade_2p5;
            }
            break;
        case BCM53401_DEVICE_ID:
        case BCM53411_DEVICE_ID:
            if (config_id == 3) {
                tdm_size = 78;
                arr = gh_tdm_cascade_1;
            } else {
                tdm_size = 96;
                arr = gh_tdm_cascade;
            }
            break;
        case BCM53402_DEVICE_ID:
        case BCM53412_DEVICE_ID:
            tdm_size = 36;
            arr = gh_tdm_low_port_count_10g_embeded;
            break;
        case BCM53405_DEVICE_ID:
        case BCM53415_DEVICE_ID:
            tdm_size = 68;
            arr = gh_tdm_10g_embedded;
            break;
        case BCM53406_DEVICE_ID:
        case BCM53416_DEVICE_ID:
            tdm_size = 68;
            if (config_id == 3) {
                arr = gh_tdm_mixed_embeded_53406_op2;
            } else if (config_id == 4) {
                arr = gh_tdm_mixed_embeded_53406_op4;
                tdm_size = 101;
            } else if (config_id == 2) {
                arr = gh_tdm_mixed_embeded_53406_op2;
            } else {
               arr = gh_tdm_mixed_embeded_4x5g;
            }
            break;
        case BCM53408_DEVICE_ID:
        case BCM53418_DEVICE_ID:
                tdm_size = 28;
                arr = gh_tdm_mixed_embeded_2p5g;
            break;
        case BCM53422_DEVICE_ID:
        case BCM53426_DEVICE_ID:
                tdm_size = 78;
                arr = gh_tdm_bloodhound_53422;
            break;
        case BCM53424_DEVICE_ID:
            if (config_id == 3) {
                tdm_size = 78;
                arr = gh_tdm_bloodhound_53422;
            } else {
                tdm_size = 96;
                arr = gh_tdm_cascade;
            }
            break;
    }

    /* DISABLE [Bit 0] = 1, TDM_WRAP_PTR[Bit 7:1] = TDM_SIZE-1 */
    val = ((tdm_size-1) << 1 | 1);
    bcm5340x_reg_set(0, R_IARB_TDM_CONTROL, val);

    for (i = 0; i < tdm_size; i++) {
        bcm5340x_mem_set(unit, M_IARB_TDM_TABLE(i), (uint32 *)&arr[i], 1);
        /* TDM programmed in MMU is in Logical port domain */
        if ((gh_sw_info.devid == BCM53406_DEVICE_ID) &&
            ((config_id == 2) || (config_id == 3))) {
            val = (arr[i] != 63) ? p2m_mapping_53406_op2[arr[i]] : 63;
        } else {
            val = (arr[i] != 63) ? gh_sw_info.port_p2l_mapping[arr[i]] : 63;
        }
        if (i == (tdm_size - 1)) {
            /* WRAP_EN [Bit 6] = 1 */
            val |= 0x40;
        }
        bcm5340x_mem_set(unit, M_MMU_ARB_TDM_TABLE(i), &val, 1);
    }

    /* DISABLE [Bit 0] = 0, TDM_WRAP_PTR[Bit 7:1] = POWERSAVE_MODE_TDM_SIZE-1 */
    val = (tdm_size-1) << 1;
    bcm5340x_reg_set(unit, R_IARB_TDM_CONTROL, val);

    /* MMU initialization */
    if ((gh_sw_info.devid == BCM53402_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53412_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53405_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53415_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53422_DEVICE_ID)) {

        /* 16x10g MMU setting, for example BCM53402 and BCM53405  */
        for (i = 0; i < COS_QUEUE_NUM; i++) {
            for (j = 0; j <= BCM5340X_LPORT_MAX; j++) {
                if (-1 == SOC_PORT_L2P_MAPPING(j)) {
                    continue;
                }
                /*
                 * The HOLCOSPKTSETLIMITr register controls BOTH the XQ
                 * size per cosq AND the HOL set limit for that cosq.
                 */
                addr = R_HOLCOSPKTSETLIMIT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_16X10G_R_HOLCOSPKTSETLIMIT);
                addr = R_HOLCOSPKTRESETLIMIT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_16X10G_R_HOLCOSPKTRESETLIMIT);
                addr = R_LWMCOSCELLSETLIMIT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_16X10G_R_LWMCOSCELLSETLIMIT);

                addr = R_HOLCOSCELLMAXLIMIT(i, j);
                if (i == CFG_FLOW_CONTROL_ENABLED_COSQ) {
                    bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_16X10G_R_HOLCOSCELLMAXLIMIT_COS0);
                } else {
                    bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_16X10G_R_HOLCOSCELLMAXLIMIT_COS1);
                }

                addr = R_HOLCOSMINXQCNT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_16X10G_R_HOLCOSMINXQCNT);
                addr = R_PGCELLLIMIT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_16X10G_R_PGCELLLIMIT);
                addr = R_PGDISCARDSETLIMIT(i, j);
                if (gh_sw_info.devid == BCM53422_DEVICE_ID) {
                    bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_16X10G_1M_R_PGDISCARDSETLIMIT);
                } else {
                    bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_16X10G_R_PGDISCARDSETLIMIT);
                }

            }
        }

        for (i = 0; i <= BCM5340X_LPORT_MAX; i++) {
            if (-1 == SOC_PORT_L2P_MAPPING(i)) {
                continue;
            }
            bcm5340x_reg_set(unit, R_DYNCELLLIMIT(i), MMU_16X10G_R_DYNCELLLIMIT);
            bcm5340x_reg_set(unit, R_DYNXQCNTPORT(i), MMU_16X10G_R_DYNXQCNTPORT);
            bcm5340x_reg_set(unit, R_DYNRESETLIMPORT(i), MMU_16X10G_R_DYNRESETLIMPORT);

            bcm5340x_reg_set(unit, R_IBPPKTSETLIMIT(i), MMU_16X10G_R_IBPPKTSETLIMIT);
        }

        if (gh_sw_info.devid == BCM53422_DEVICE_ID) {
            bcm5340x_reg_set(unit, R_TOTALDYNCELLSETLIMIT, MMU_16X10G_1M_R_TOTALDYNCELLSETLIMIT);
            bcm5340x_reg_set(unit, R_TOTALDYNCELLRESETLIMIT, MMU_16X10G_1M_R_TOTALDYNCELLRESETLIMIT);
        } else {
            bcm5340x_reg_set(unit, R_TOTALDYNCELLSETLIMIT, MMU_16X10G_R_TOTALDYNCELLSETLIMIT);
            bcm5340x_reg_set(unit, R_TOTALDYNCELLRESETLIMIT, MMU_16X10G_R_TOTALDYNCELLRESETLIMIT);
        }
    } else if ((gh_sw_info.devid == BCM53406_DEVICE_ID) ||
               (gh_sw_info.devid == BCM53416_DEVICE_ID) ||
               (gh_sw_info.devid == BCM53426_DEVICE_ID)) {
        /* 12g + 12x10g MMU setting, for example BCM53406  */
        for (i = 0; i < COS_QUEUE_NUM; i++) {
            for (j = 0; j <= BCM5340X_LPORT_MAX; j++) {
                if (-1 == SOC_PORT_L2P_MAPPING(j)) {
                    continue;
                }
                /*
                 * The HOLCOSPKTSETLIMITr register controls BOTH the XQ
                 * size per cosq AND the HOL set limit for that cosq.
                 */
                addr = R_HOLCOSPKTSETLIMIT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_12G_12X10G_R_HOLCOSPKTSETLIMIT);
                addr = R_HOLCOSPKTRESETLIMIT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_12G_12X10G_R_HOLCOSPKTRESETLIMIT);
                addr = R_LWMCOSCELLSETLIMIT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_12G_12X10G_R_LWMCOSCELLSETLIMIT);

                addr = R_HOLCOSCELLMAXLIMIT(i, j);
                if (i == CFG_FLOW_CONTROL_ENABLED_COSQ) {
                    bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_12G_12X10G_R_HOLCOSCELLMAXLIMIT_COS0);
                } else {
                    bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_12G_12X10G_R_HOLCOSCELLMAXLIMIT_COS1);
                }

                addr = R_HOLCOSMINXQCNT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_12G_12X10G_R_HOLCOSMINXQCNT);
                addr = R_PGCELLLIMIT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_12G_12X10G_R_PGCELLLIMIT);
                addr = R_PGDISCARDSETLIMIT(i, j);
                if (gh_sw_info.devid == BCM53426_DEVICE_ID) {
                    bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_12G_12X10G_1M_R_PGDISCARDSETLIMIT);
                } else {
                    bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_12G_12X10G_R_PGDISCARDSETLIMIT);
                }
            }
        }

        for (i = 0; i <= BCM5340X_LPORT_MAX; i++) {
            if (-1 == SOC_PORT_L2P_MAPPING(i)) {
                continue;
            }
            bcm5340x_reg_set(unit, R_DYNCELLLIMIT(i), MMU_12G_12X10G_R_DYNCELLLIMIT);
            bcm5340x_reg_set(unit, R_DYNXQCNTPORT(i), MMU_12G_12X10G_R_DYNXQCNTPORT);
            bcm5340x_reg_set(unit, R_DYNRESETLIMPORT(i), MMU_12G_12X10G_R_DYNRESETLIMPORT);

            bcm5340x_reg_set(unit, R_IBPPKTSETLIMIT(i), MMU_12G_12X10G_R_IBPPKTSETLIMIT);
        }

        if (gh_sw_info.devid == BCM53426_DEVICE_ID) {
            bcm5340x_reg_set(unit, R_TOTALDYNCELLSETLIMIT, MMU_12G_12X10G_1M_R_TOTALDYNCELLSETLIMIT);
            bcm5340x_reg_set(unit, R_TOTALDYNCELLRESETLIMIT, MMU_12G_12X10G_1M_R_TOTALDYNCELLRESETLIMIT);
        } else {
            bcm5340x_reg_set(unit, R_TOTALDYNCELLSETLIMIT, MMU_12G_12X10G_R_TOTALDYNCELLSETLIMIT);
            bcm5340x_reg_set(unit, R_TOTALDYNCELLRESETLIMIT, MMU_12G_12X10G_R_TOTALDYNCELLRESETLIMIT);
        }
    } else {
        /* 24g + 4 uplink MMU setting, for example BCM53401 and BCM53408  */
        for (i = 0; i < COS_QUEUE_NUM; i++) {
            for (j = 0; j <= BCM5340X_LPORT_MAX; j++) {
                if (-1 == SOC_PORT_L2P_MAPPING(j)) {
                    continue;
                }
                /*
                 * The HOLCOSPKTSETLIMITr register controls BOTH the XQ
                 * size per cosq AND the HOL set limit for that cosq.
                 */
                addr = R_HOLCOSPKTSETLIMIT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_R_HOLCOSPKTSETLIMIT);
                addr = R_HOLCOSPKTRESETLIMIT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_R_HOLCOSPKTRESETLIMIT);
                addr = R_LWMCOSCELLSETLIMIT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_R_LWMCOSCELLSETLIMIT);

                addr = R_HOLCOSCELLMAXLIMIT(i, j);
                if (i == CFG_FLOW_CONTROL_ENABLED_COSQ) {
                    bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_R_HOLCOSCELLMAXLIMIT_COS0);
                } else {
                    bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_R_HOLCOSCELLMAXLIMIT_COS1);
                }

                addr = R_HOLCOSMINXQCNT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_R_HOLCOSMINXQCNT);
                addr = R_PGCELLLIMIT(i, j);
                bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_R_PGCELLLIMIT);
                addr = R_PGDISCARDSETLIMIT(i, j);
                if (gh_sw_info.devid == BCM53424_DEVICE_ID) {
                    bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_1M_R_PGDISCARDSETLIMIT);
                } else {
                    bcm5340x_reg_set(unit, MMU_BLOCK_ID, addr, MMU_R_PGDISCARDSETLIMIT);
                }
            }
        }

        for (i = 0; i <= BCM5340X_LPORT_MAX; i++) {
            if (-1 == SOC_PORT_L2P_MAPPING(i)) {
                continue;
            }
            bcm5340x_reg_set(unit, R_DYNCELLLIMIT(i), MMU_R_DYNCELLLIMIT);
            bcm5340x_reg_set(unit, R_DYNXQCNTPORT(i), MMU_R_DYNXQCNTPORT);
            bcm5340x_reg_set(unit, R_DYNRESETLIMPORT(i), MMU_R_DYNRESETLIMPORT);

            bcm5340x_reg_set(unit, R_IBPPKTSETLIMIT(i), MMU_R_IBPPKTSETLIMIT);
        }

        if ((gh_sw_info.devid == BCM53408_DEVICE_ID) ||
            (gh_sw_info.devid == BCM53418_DEVICE_ID) ||
            (gh_sw_info.devid == BCM53454_DEVICE_ID) ||
            (gh_sw_info.devid == BCM53455_DEVICE_ID)) {
            bcm5340x_reg_set(unit, R_TOTALDYNCELLSETLIMIT, MMU_12G_12X10G_R_TOTALDYNCELLSETLIMIT);
            bcm5340x_reg_set(unit, R_TOTALDYNCELLRESETLIMIT, MMU_12G_12X10G_R_TOTALDYNCELLRESETLIMIT);
        } else if (gh_sw_info.devid == BCM53424_DEVICE_ID) {
            bcm5340x_reg_set(unit, R_TOTALDYNCELLSETLIMIT, MMU_1M_R_TOTALDYNCELLSETLIMIT);
            bcm5340x_reg_set(unit, R_TOTALDYNCELLRESETLIMIT, MMU_1M_R_TOTALDYNCELLRESETLIMIT);
        } else {
            bcm5340x_reg_set(unit, R_TOTALDYNCELLSETLIMIT, MMU_R_TOTALDYNCELLSETLIMIT);
            bcm5340x_reg_set(unit, R_TOTALDYNCELLRESETLIMIT, MMU_R_TOTALDYNCELLRESETLIMIT);
        }
    }

    /* DYN_XQ_EN[Bit8] = 1, HOL_CELL_SOP_DROP_EN[Bit7] = 1, SKIDMARKER[Bit3:2] = 3 */
    bcm5340x_reg_get(unit, R_MISCCONFIG, &val);
    val |= 0x18c;
    bcm5340x_reg_set(unit, R_MISCCONFIG, val);

    /* Port enable */
#if CFG_RXTX_SUPPORT_ENABLED
    /* Add CPU port */
    bcm5340x_reg_set(unit, R_MMUPORTENABLE, BCM5340X_ALL_PORTS_MASK | 0x1);
#else
    bcm5340x_reg_set(unit, R_MMUPORTENABLE, BCM5340X_ALL_PORTS_MASK);
#endif /* CFG_RXTX_SUPPORT_ENABLED */
}

static void
config_schedule_mode(uint8 unit)
{
    int i, j;

    SOC_LPORT_ITER(i) {
#if CONFIG_GREYHOUND_ROMCODE
        /* Strict Priority Mode[Bit 0-1] = 0x0, MTU_Quanta_Select[Bit 2-3]=0x3 */
        bcm5340x_reg_set(unit, R_XQCOSARBSEL(i), 0xC);
#else
        
        bcm5340x_reg_set(unit, R_WRRWEIGHT_COS0(i), 0x81);
        bcm5340x_reg_set(unit, R_WRRWEIGHT_COS1(i), 0x82);
        bcm5340x_reg_set(unit, R_WRRWEIGHT_COS2(i), 0x84);
        bcm5340x_reg_set(unit, R_WRRWEIGHT_COS3(i), 0x88);
        bcm5340x_reg_set(unit, R_XQCOSARBSEL(i), 0xE);
#endif /* CONFIG_GREYHOUND_ROMCODE */
        /* MAX_THD_SEL = 0 : Disable MAX shaper */
        for (j = 0; j < COS_QUEUE_NUM; j++) {
        	bcm5340x_reg_set(unit, R_MAXBUCKETCONFIG(j, i), 0x0);
        }
    }
}

#ifdef CFG_SWITCH_LAG_INCLUDED
#if defined(CFG_SWITCH_RATE_INCLUDED) || defined(CFG_SWITCH_QOS_INCLUDED) || defined(CFG_SWITCH_LOOPDETECT_INCLUDED)
static void
_bcm5340x_lag_group_fp_set(uint8 unit, int start_index, uint8 lagid,
                     pbmp_t pbmp, pbmp_t old_pbmp, uint8 revise_redirect_pbmp, uint8 cpu_include)
{
    int i, j;
    uint32 tcam_entry[15], xy_entry[15], dm_entry[15];
    uint32 global_tcam_mask_entry[3] = { 0x1, 0, 0 };

    /* The entry (pbmp) bit 0 is cpu port.
     * The policy[0] redirect entry, bit 5 is cpu port.
     */

    if (cpu_include == TRUE) {
        j = 0;
    } else {
        j = BCM5340X_LPORT_MIN;
    }
    for (i = start_index; j <= BCM5340X_LPORT_MAX; i++, j++) {
        if ((j > 0) && (j < BCM5340X_LPORT_MIN)) {
            continue;
        }
        bcm5340x_mem_get(unit, M_FP_TCAM(i), dm_entry, 15);
        bcm5340x_xy_to_dm(dm_entry, tcam_entry, 15, 480);
        /*  Revise the source tgid qualify if the port is trunk port */
        if (old_pbmp & (0x1 << j)) {
            tcam_entry[0] &= ~(0x1f << 27);
            tcam_entry[1] &= ~0x1ff;
            tcam_entry[0] |= (j << 27);
        }
        if (pbmp & (0x1 << j)) {
            tcam_entry[0] &= ~(0x1f << 27);
            tcam_entry[1] &= ~0x1ff;
            tcam_entry[0] |= (lagid << 27);
            tcam_entry[1] |= 0x00000100;
        }
        bcm5340x_dm_to_xy(tcam_entry, xy_entry, 15, 480);
        bcm5340x_mem_set(unit, M_FP_TCAM(i), xy_entry, 15);
        bcm5340x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(i), global_tcam_mask_entry, 3);
    }
}
#endif /* CFG_SWITCH_RATE_INCLUDED || CFG_SWITCH_QOS_INCLUDED || CFG_SWITCH_LOOPDETECT_INCLUDED */

/*
 *  Function : bcm5340x_lag_group_set
 *  Purpose :
 *      Set lag group membership.
 *  Parameters :
 *  Return :
 *  Note :
 *
 */
sys_error_t
bcm5340x_lag_group_set(uint8 unit, uint8 lagid, pbmp_t pbmp)
{
    uint32 bitmap_entry = (uint32)pbmp, old_bitmap_entry;
    uint32 entry[3] = {0, 0, 0};
    uint32 group_entry[4] = {0, 0, 0, 0};
    uint8 i, j, count = 0;
    uint8 trunk_port[BOARD_MAX_PORT_PER_LAG];

    for (i = BCM5340X_LPORT_MIN; i <= BCM5340X_LPORT_MAX; i++) {
        if(bitmap_entry & (0x1 << i)) {
            count ++;
            trunk_port[count-1] = i;
        }
    }

    bcm5340x_mem_get(0, M_TRUNK_BITMAP(lagid), &old_bitmap_entry, 1);

    if (bitmap_entry != old_bitmap_entry) {
        /* Need to update source port qualifier in FP TCAM entry  */
#ifdef CFG_SWITCH_RATE_INCLUDED
        /*
         * Slice 1 Entry 0~23 (one entry for each port):
         * Rate ingress
         */
        _bcm5340x_lag_group_fp_set(unit, RATE_IGR_IDX, lagid,
                                   bitmap_entry, old_bitmap_entry, FALSE, FALSE);
#endif /* CFG_SWITCH_RATE_INCLUDED */

#ifdef CFG_SWITCH_QOS_INCLUDED
        /*
         * Slice 2 Entry 0~23 (one entry for each port):
         * Port based QoS
         */
        _bcm5340x_lag_group_fp_set(unit, QOS_BASE_IDX, lagid,
                                   bitmap_entry, old_bitmap_entry, FALSE, FALSE);
#endif /* CFG_SWITCH_QOS_INCLUDED */

#ifdef CFG_SWITCH_LOOPDETECT_INCLUDED
        /*
         * Slice 3 Entry BCM5340X_PORT_MIN~BCM5340X_PORT_MAX (one entry for each port):
         * Loop detect counter
         */
        _bcm5340x_lag_group_fp_set(unit, LOOP_COUNT_IDX, lagid,
                                   bitmap_entry, old_bitmap_entry, FALSE, FALSE);
                                   
		/*
         * Slice 3, #define LOOP_REDIRECT_IDX              ( 3 * ENTRIES_PER_SLICE + 35)
         * (one entry for each port, including CPU):
         * Update both source port qualifier 
         */
        _bcm5340x_lag_group_fp_set(unit, LOOP_REDIRECT_IDX, lagid,
                                    bitmap_entry, old_bitmap_entry, TRUE, TRUE);
#endif /* CFG_SWITCH_LOOPDETECT_INCLUDED */

    }

	/* Set TRUNK Bitmap, TRUNK Group, Source TRUNK Map and NonUcast TRUNK Block Mask Table */
    for (i = BCM5340X_PORT_MIN; i <= BCM5340X_PORT_MAX; i++) {
        if(old_bitmap_entry & (0x1 << i)) {
            entry[0] = 0x0;
            bcm5340x_mem_set(unit, M_SOURCE_TRUNK_MAP(i), entry, 3);
        }

        if(bitmap_entry & (0x1 << i)) {
            entry[0] = 0x1 | (lagid << 2);
            bcm5340x_mem_set(unit, M_SOURCE_TRUNK_MAP(i), entry, 3);
        }
    }

    for (i = 0; i < BOARD_MAX_PORT_PER_LAG; i++) {
        if (count == 0) {
            j = 0;
        } else {
            j = trunk_port[i%count];
        }
        switch (i) {
            case 0:
                group_entry[0] |= j;
                break;
            case 1:
                group_entry[0] |= (j << 13);
                break;
            case 2:
                group_entry[0] |= (j << 26);
                break;
            case 3:
                group_entry[1] |= (j << 7);
                break;
            case 4:
                group_entry[1] |= (j << 20);
                break;
            case 5:
                group_entry[2] |= (j << 1);
                break;
            case 6:
                group_entry[2] |= (j << 14);
                break;
            case 7:
                group_entry[2] |= (j << 27);
                group_entry[3] |= (j >> 5);
                break;
        }
    }

	/* Set RTAG to 0x3 (SA+DA) */
    group_entry[3] |= (0x3 << 8);

    bcm5340x_mem_set(unit, M_TRUNK_BITMAP(lagid), &bitmap_entry, 1);
    bcm5340x_mem_set(unit, M_TRUNK_GROUP(lagid), group_entry, 4);


    for (i = 0; i < 64; i++) {
        bcm5340x_mem_get(0, M_NONUCAST_TRUNK_BLOCK_MASK(i),
                                      entry, 1);
        entry[0] &= ~old_bitmap_entry;
        entry[0] |= bitmap_entry;
        if (count != 0) {
            entry[0] &= ~(0x1 << trunk_port[i%count]);
        }
        bcm5340x_mem_set(0, M_NONUCAST_TRUNK_BLOCK_MASK(i),
                                      entry, 1);
    }
    return SYS_OK;
}

/*
 *  Function : bcm5340x_lag_group_get
 *  Purpose :
 *      Get lag group membership.
 *  Parameters :
 *  Return :
 *  Note :
 *
 */
void
bcm5340x_lag_group_get(uint8 unit, uint8 lagid, pbmp_t *pbmp) {
    uint32 bitmap_entry;

    bcm5340x_mem_get(unit, M_TRUNK_BITMAP(lagid), &bitmap_entry, 1);

    *pbmp = (pbmp_t)bitmap_entry;
}
#endif /* CFG_SWITCH_LAG_INCLUDED */

static void
bcm5340x_system_init(uint8 unit)
{
    int i, j;
    uint32 entry[6];
    uint32 val;

    /* Default port_entry :
     * bit 35 - 24 : PORT_VID = 0x1
     * bit 60 : TRUST_OUTER_DOT1P = 0x1
     * bit 99 - 96 : OUTER_TPID_ENABLE = 0x1
     * bit 121 : TRUST_INCOMING_VID = 0x1
     * bit 135 - 132 : CML_FLAGS_NEW = 0x8
     * bit 139 - 136 : CML_FLAGS_MOVE = 0x8
     */
    uint32 port_entry[12] = { 0x01000000, 0x10000000, 0x00000000, 0x02000001,
                              0x00000880, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0  };

    uint32 dot1pmap[16] = {
    0x00000000, 0x00000001, 0x00000004, 0x00000005, 0x00000008, 0x00000009, 0x0000000c, 0x0000000d,
    0x00000010, 0x00000011, 0x00000014, 0x00000015, 0x00000018, 0x00000019, 0x0000001c, 0x0000001d };

    /* Configurations to guarantee no packet modifications */
    for (i = BCM5340X_LPORT_MIN; i <= BCM5340X_LPORT_MAX; i++) {
        /* ING_OUTER_TPID[0] is allowed outer TPID values */
        entry[0] = 0x1;
        bcm5340x_mem_set(unit, M_SYSTEM_CONFIG_TABLE(i), entry, 1);

        entry[0] = 0x0;
#ifdef CFG_SWITCH_VLAN_UNAWARE_INCLUDED
        /* DISABLE_VLAN_CHECKS[Bit 63] = 1, PACKET_MODIFICATION_DISABLE[Bit 62] = 1 */
        entry[1] = 0xC0000000;
#else
        entry[1] = 0x0;
#endif /* CFG_SWITCH_VLAN_UNAWARE_INCLUDED */
        entry[2] = 0x0;
        bcm5340x_mem_set(unit, M_SOURCE_TRUNK_MAP(i), entry, 3);

        bcm5340x_mem_set(unit, M_PORT(i), port_entry, 12);

        /* Clear Unknown Unicast Block Mask. */
        bcm5340x_reg_set(unit, R_UNKNOWN_UCAST_BLOCK_MASK_64(i), 0x0);

        /* Clear ingress block mask. */
        bcm5340x_reg_set(unit, R_ING_EGRMSKBMAP_64(i), 0x0);
    }

    for (i = 0; i <= BCM5340X_LPORT_MAX; i++) {
        /*
         * ING_PRI_CNG_MAP: Unity priority mapping and CNG = 0 or 1
         */
        for (j = 0; j < 16; j++) {
            bcm5340x_mem_set(unit, M_ING_PRI_CNG_MAP(i*16+j), &dot1pmap[j], 1);
        }
    }

    /* TRUNK32_CONFIG_TABLE: OUTER_TPID_ENABLE[3:0] (Bit 0-3) = 0x1 */
    val = 0x1;
    /*
     * TRUNK32_PORT_TABLE:
     * DISABLE_VLAN_CHECKS[Bit 31] = 1, PACKET_MODIFICATION_DISABLE[Bit 30] = 1
     */
#ifdef CFG_SWITCH_VLAN_UNAWARE_INCLUDED
    entry[0] = 0xC0000000;
#else
    entry[0] = 0x0;
#endif /* CFG_SWITCH_VLAN_UNAWARE_INCLUDED */
    entry[1] = 0x0;

    for (i = 0; i < 32; i++) {
        bcm5340x_mem_set(unit, M_TRUNK32_CONFIG_TABLE(i), &val, 1);
        bcm5340x_mem_set(unit, M_TRUNK32_PORT_TABLE(i), entry, 2);
    }

#if CONFIG_GREYHOUND_ROMCODE
#define INT_PRI_MAX  16
    /* COS_MAP: unity mapping */
    for (i = 0; i < INT_PRI_MAX; i++) {
        if (i < COS_QUEUE_NUM) {
            val = (i << 3) | i;
        }
        bcm5340x_mem_set(unit, M_COS_MAP(i), &val, 1);
    }
#endif /* CONFIG_GREYHOUND_ROMCODE */

    enable_jumbo_frame(unit);
    config_schedule_mode(unit);

    /*
     * VLAN_DEFAULT_PBM is used as defalut VLAN members for those ports
     * that disable VLAN checks.
     */
    bcm5340x_reg_set(unit, R_VLAN_DEFAULT_PBM, BCM5340X_ALL_PORTS_MASK);

    /* ING_VLAN_TAG_ACTION_PROFILE:
     * UT_OTAG_ACTION (Bit 2-3) = 0x1
     * SIT_OTAG_ACTION (Bit 8-9) = 0x0
     * SOT_POTAG_ACTION (Bit 12-13) = 0x2
     * SOT_OTAG_ACTION (Bit 14-15) = 0x0
     * DT_POTAG_ACTION (Bit 20-21) = 0x2
     */
    val = 0x0202004;
    bcm5340x_mem_set(unit, M_ING_VLAN_TAG_ACTION_PROFILE, &val, 1);

    /*
     * Program l2 user entry table to drop below MAC addresses:
     * 0x0180C2000001, 0x0180C2000002 and 0x0180C200000E
     */

    /* VALID[Bit 0] = 1 */
    entry[0] = (0xC2000001 << 1) | 0x1;
    entry[1] = (0x0180 << 1) | 0x1;

    /* Encoded MASK: MAC address only */
    entry[2] = 0x7fffff80;
    entry[3] = 0x003f9fcf;
    /* DST_DISCARD[Bit 19] = 1 */
    entry[4] = 0x00080004;
    /* BPDU[Bit 2] = 1 */
    entry[5] = 0x4;
    bcm5340x_mem_set(unit, M_L2_USER_ENTRY(0), entry, 6);

    entry[0] = (0xC2000002 << 1) | 0x1;
    entry[2] = 0x7fffff40;
    bcm5340x_mem_set(unit, M_L2_USER_ENTRY(1), entry, 6);

    entry[0] = (0xC200000E << 1) | 0x1;
    entry[2] = 0x7ffffc40;
    bcm5340x_mem_set(unit, M_L2_USER_ENTRY(2), entry, 6);

#if !CONFIG_GREYHOUND_ROMCODE
#ifdef CFG_SWITCH_DOS_INCLUDED
    /*
     * Enable following Denial of Service protections:
     * DROP_IF_SIP_EQUALS_DIP
     * MIN_TCPHDR_SIZE = 0x14 (Default)
     * IP_FIRST_FRAG_CHECK_ENABLE
     * TCP_HDR_OFFSET_EQ1_ENABLE
     * TCP_HDR_PARTIAL_ENABLE
     */
    bcm5340x_reg_set(unit, R_DOS_CONTROL, 0x2280411);
    bcm5340x_reg_set(unit, R_DOS_CONTROL2, 0x01300000);
#endif /* CFG_SWITCH_DOS_INCLUDED */

    /* enable FP_REFRESH_ENABLE [Bit 26] */
    bcm5340x_reg_get(unit, R_AUX_ARB_CONTROL_2, &val);
    val |= 0x4000000;
    bcm5340x_reg_set(unit, R_AUX_ARB_CONTROL_2, val);

    /*
     * Enable IPV4_RESERVED_MC_ADDR_IGMP_ENABLE[Bit 31], APPLY_EGR_MASK_ON_L3[Bit 13]
     * and APPLY_EGR_MASK_ON_L2[Bit 12]
     * Disable L2DST_HIT_ENABLE[Bit 2]
     */
    bcm5340x_reg64_get(unit, R_ING_CONFIG_64, entry, 2);
    entry[0] |= 0x80003000;
    entry[0] &= ~0x4;
    bcm5340x_reg64_set(unit, R_ING_CONFIG_64, entry, 2);

    /*
     * L3_IPV6_PFM=1, L3_IPV4_PFM=1, L2_PFM=1, IPV6L3_ENABLE=1, IPV4L3_ENABLE=1
     * IPMCV6_L2_ENABLE=1, IPMCV6_ENABLE=1, IPMCV4_L2_ENABLE=1, IPMCV4_ENABLE=1
     */
    entry[0] = 0x0003f015;
    bcm5340x_mem_set(unit, M_VLAN_PROFILE(0), entry, 1);
#endif /* !CONFIG_GREYHOUND_ROMCODE */

    /* Do VLAN Membership check EN_EFILTER[Bit 3] for the outgoing port */
    for (i = BCM5340X_LPORT_MIN; i <= BCM5340X_LPORT_MAX; i++) {
        bcm5340x_reg64_get(unit, R_EGR_PORT_64(i), entry, 2);
        entry[0] |= (0x1 << 3);
        bcm5340x_reg64_set(unit, R_EGR_PORT_64(i), entry, 2);
    }

#if CFG_RXTX_SUPPORT_ENABLED
    /*
     * Use VLAN 0 for CPU to transmit packets
     * All ports are untagged members, with STG=1 and VLAN_PROFILE_PTR=0
     */
    entry[0] = 0x3ffffffd;
    entry[1] = 0x000000c0;
    entry[2] = 0x0;
    entry[3] = 0x0;
    bcm5340x_mem_set(unit, M_VLAN(0), entry, 4);

    entry[0] = 0x43fffffc;
    entry[1] = 0x1fffffff;
    entry[2] = 0x00000010;
    bcm5340x_mem_set(unit, M_EGR_VLAN(0), entry, 3);

#ifdef __BOOTLOADER__
    /* Default VLAN 1 with STG=1 and VLAN_PROFILE_PTR=0 for bootloader */
    entry[0] = 0x3ffffffc;
    entry[1] = 0x000000c0;
    entry[2] = 0x0;
    entry[3] = 0x0;
    bcm5340x_mem_set(unit, M_VLAN(1), entry, 4);

    entry[0] = 0x03fffffc;
    entry[1] = 0x1fffffff;
    entry[2] = 0x00000010;
    bcm5340x_mem_set(unit, M_EGR_VLAN(1), entry, 3);
#endif /* __BOOTLOADER__ */

    /* Set VLAN_STG and EGR_VLAN_STG */
    entry[0] = 0xfffffff0;
    entry[1] = 0x0fffffff;
    bcm5340x_mem_set(unit, M_VLAN_STG(1), entry, 2);
    bcm5340x_mem_set(unit, M_EGR_VLAN_STG(1), entry, 2);

    /* Make PORT_VID[Bit 35:24] = 0 for CPU port */
    bcm5340x_mem_get(unit, M_PORT(0), port_entry, 12);
    port_entry[0] &= 0x00ffffff;
    port_entry[1] &= 0xfffffff0;
    bcm5340x_mem_set(unit, M_PORT(0), port_entry, 12);

#if !CONFIG_GREYHOUND_ROMCODE
    /*
     * Trap DHCP[Bit 0] and ARP packets[Bit 4, 6] to CPU.
     * Note ARP reply is copied to CPU ONLY when l2 dst is hit.
     */
    for (i = BCM5340X_LPORT_MIN ; i <= BCM5340X_LPORT_MAX ; i++) {
        bcm5340x_reg_set(unit, R_PROTOCOL_PKT_CONTROL(i), 0x51);
    }
#endif
#endif /* CFG_RXTX_SUPPORT_ENABLED */

    /* Enable aging timer */
    bcm5340x_reg_get(unit, R_L2_AGE_TIMER, &val);
    val |= (0x1 << 20);
    bcm5340x_reg_set(unit, R_L2_AGE_TIMER, val);

}

#define FW_ALIGN_BYTES                  16
#define FW_ALIGN_MASK                   (FW_ALIGN_BYTES - 1)
int
_firmware_helper(void *ctx, uint32 offset, uint32 size, void *data)
{
    int ioerr = 0;
    uint32 val;
    uint32 wbuf[4], ucmem_data[4];
    uint32 *fw_data;
    uint32 *fw_entry;
    uint32 fw_size;
    uint32 idx, wdx;
    phy_ctrl_t *pc = (phy_ctrl_t *)ctx;
    int lport;


    /* Check if PHY driver requests optimized MDC clock */
    if (data == NULL) {
        uint32 rate_adjust;
        val = 1;

        /* Offset value is MDC clock in kHz (or zero for default) */
        if (offset) {
            val = offset / 1500;
        }

        rate_adjust = READCSR(CMIC_RATE_ADJUST_EXT_MDIO);
        rate_adjust &= ~ (0xFFFF0000);
        rate_adjust |= val << 16;
        ioerr += WRITECSR(CMIC_RATE_ADJUST_EXT_MDIO, val);

        return ioerr ? SYS_ERR_STATE : SYS_OK;
    }
    if (sal_strcmp(pc->drv->drv_name, "bcmi_tsce_xgxs") != 0) {
        return CDK_E_NONE;
    }
    if (size == 0) {
        return SYS_ERR_PARAMETER;
    }
    /* Aligned firmware size */
    fw_size = (size + FW_ALIGN_MASK) & ~FW_ALIGN_MASK;

    /* Enable parallel bus access, ACCESS_MODE [Bit 0] = 0x1 */
    lport = SOC_PORT_P2L_MAPPING(pc->port);
    bcm5340x_reg_set(pc->unit, SOC_PORT_BLOCK(lport), R_XLPORT_WC_UCMEM_CTRL, 0x1);

    /* DMA buffer needs 32-bit words in little endian order */
    fw_data = (uint32 *)data;
    for (idx = 0; idx < fw_size; idx += 16) {
        if (idx + 15 < size) {
            fw_entry = &fw_data[idx >> 2];
        } else {
            /* Use staging buffer for modulo bytes */
            sal_memset(wbuf, 0, sizeof(wbuf));
            sal_memcpy(wbuf, &fw_data[idx >> 2], 16 - (fw_size - size));
            fw_entry = wbuf;
        }
        for (wdx = 0; wdx < 4; wdx++) {
            ucmem_data[wdx] = htol32(fw_entry[wdx]);
        }
        bcm5340x_mem_set(pc->unit, SOC_PORT_BLOCK(lport),
                         M_XLPORT_WC_UCMEM_DATA(idx >> 4), ucmem_data, 4);
    }

    /* Disable parallel bus access */
    bcm5340x_reg_set(pc->unit, SOC_PORT_BLOCK(lport), R_XLPORT_WC_UCMEM_CTRL, 0x0);

    return ioerr ? SYS_ERR_STATE : SYS_OK;
}

extern int
bmd_phy_fw_helper_set(int unit, int port,
                      int (*fw_helper)(void *, uint32, uint32, void *));

/* Function:
 *   bcm5340x_sw_init
 * Description:
 *   Perform chip specific initialization.
 *   This will be called by board_init()
 * Parameters:
 *   None
 * Returns:
 *   None
 */

sys_error_t
bcm5340x_sw_init(void)
{
    int   rv = 0;
    uint8 unit = 0, lport;
    uint32 top_strap_val;
    int port_speed[BCM5340X_LPORT_MAX+1];
    int flag_vco = 0;
    int flag_act = 0;
    uint8 cmp_lport;
    pbmp_t vco_6250_pbmp;
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED    
    int port_cnt = 0;
    sys_error_t sal_config_rv = SYS_OK, an_rv = SYS_OK, cl73_rv = SYS_OK, cl37_rv = SYS_OK;
    pbmp_t active_pbmp, phy_an_pbmp, phy_cl73_pbmp, phy_cl37_pbmp;
    pbmp_t speed_1000_pbmp, speed_2500_pbmp, speed_5000_pbmp,speed_10000_pbmp;
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */

    /* Get chip revision */
    bcm5340x_chip_revision(unit, &gh_sw_info.devid, &gh_sw_info.revid);
#if CFG_CONSOLE_ENABLED
    sal_printf("\ndevid = 0x%x, revid = 0x%x\n", gh_sw_info.devid, gh_sw_info.revid);
#endif /* CFG_CONSOLE_ENABLED */

#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
    sal_config_rv = sal_config_uint16_get(SAL_CONFIG_SKU_DEVID, &gh_sw_info.devid);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Overwrite SKU device ID with value 0x%x.\n", gh_sw_info.devid);
    }

    sal_config_rv = sal_config_uint8_get(SAL_CONFIG_SKU_OPTION, &config_id);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Overwrite SKU option with value %d.\n", config_id);
    }

    sal_config_rv = sal_config_uint8_get(SAL_CONFIG_TSCE_INTERFACE, &tsce_interface);
 
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set TSCE interface to %d. (1:SGMII, 2:Fiber)\n", tsce_interface);
        if ((tsce_interface != TSCE_INTERFACE_SGMII) && 
            (tsce_interface != TSCE_INTERFACE_FIBER)) {
            sal_printf("The TSCE interface %d is not valid and will change it to %d (1:SGMII, 2:FIBER)\n", tsce_interface, CFG_TSCE_INTERFACE);
            tsce_interface = CFG_TSCE_INTERFACE;
        }
    }
    
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
    if ((gh_sw_info.devid != BCM53401_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53402_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53405_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53406_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53408_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53411_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53412_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53415_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53416_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53418_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53454_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53455_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53456_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53457_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53422_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53424_DEVICE_ID) &&
        (gh_sw_info.devid != BCM53426_DEVICE_ID)) {
        sal_printf("\nERROR : devid 0x%x is not supported in UM software !\n", gh_sw_info.devid);
        return SYS_ERR_NOT_FOUND;
    }

    /* move SOC_PORT_COUNT to soc_init_port_mapping() after getting speed_max table */
    /* Configure QSGMII mode */
    if ((gh_sw_info.devid == BCM53401_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53424_DEVICE_ID) ||
        (gh_sw_info.devid == BCM53456_DEVICE_ID)) {
        QSGMII_MODE = 1;
    }

    /* CPS reset complete SWITCH and CMICd */
    WRITECSR(CMIC_CPS_RESET, 0x1);
#if CONFIG_GREYHOUND_EMULATION
    sal_usleep(250000);
#else
    sal_usleep(1000);
#endif

    soc_reset(unit);

    rv = soc_init_port_mapping(unit);
    if (rv != SYS_OK) {
        return rv;
    }

    soc_gh_tsc_reset(unit);

    soc_misc_init(unit);

    soc_mmu_init(unit);

    bcm5340x_system_init(unit);

    /* Probe PHYs */
    SOC_LPORT_ITER(lport) {
        rv = bmd_phy_probe(unit, lport);
        if (CDK_SUCCESS(rv)) {
            rv = PHY_CONFIG_SET(BMD_PORT_PHY_CTRL(unit, lport), PhyConfig_InitSpeed, SOC_PORT_SPEED_MAX(lport), NULL);
            /* Configure 2-LANCE/4-LANE TSC if necessary. */
            if (SOC_PORT_LANE_NUMBER(lport) == 4) {
                rv = bmd_phy_mode_set(unit, lport, "bcmi_tsce_xgxs",
                                  BMD_PHY_MODE_SERDES, 0);
            } else if (SOC_PORT_LANE_NUMBER(lport) == 2) {
                rv = bmd_phy_mode_set(unit, lport, "bcmi_tsce_xgxs",
                                  BMD_PHY_MODE_SERDES, 1);
                rv = bmd_phy_mode_set(unit, lport, "bcmi_tsce_xgxs",
                                          BMD_PHY_MODE_2LANE, 1);
            } else {
                /* lane number = 1 */
                rv = bmd_phy_mode_set(unit, lport, "bcmi_tsce_xgxs",
                                  BMD_PHY_MODE_SERDES, 1);
                rv = bmd_phy_mode_set(unit, lport, "bcmi_tsce_xgxs",
                                          BMD_PHY_MODE_2LANE, 0);
                if (SOC_PORT_SPEED_MAX(lport) == 1000) {
                    if (tsce_interface == TSCE_INTERFACE_SGMII) {
                        rv = bmd_phy_mode_set(unit, lport, "bcmi_tsce_xgxs",
                                  BMD_PHY_MODE_FIBER, 0);
                    } else {
                        rv = bmd_phy_mode_set(unit, lport, "bcmi_tsce_xgxs",
                                  BMD_PHY_MODE_FIBER, 1);
                    }
                }
            }

            if (CDK_SUCCESS(rv)) {
                rv = bmd_phy_fw_helper_set(unit, lport, _firmware_helper);
            }
            if (CDK_SUCCESS(rv)) {
                rv = bmd_phy_init(unit, lport);
            }
        }
    }
    
    
    SOC_LPORT_ITER(lport) {
        if (SOC_PORT_SPEED_MAX(lport) < 10000) {
            port_speed[lport] = 1000;
        } else {
            port_speed[lport] = 10000;
        }
    }
    
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
    sal_config_rv = sal_config_pbmp_get(SAL_CONFIG_VALID_PORTS, &active_pbmp);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set valid logical pbmp with value 0x%x.\n", active_pbmp);
        SOC_LPORT_ITER(lport) {
            if (active_pbmp & (0x1 << lport)) {
                port_cnt++; 
            } else {
                SOC_PORT_L2P_MAPPING(lport) = -1;
            }
        }
        SOC_PORT_COUNT(unit) = port_cnt;
    }

    sal_config_rv = sal_config_pbmp_get(SAL_CONFIG_SPEED_1000_PORTS, &speed_1000_pbmp);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set speed (1G) logical pbmp with value 0x%x.\n", speed_1000_pbmp);
        SOC_LPORT_ITER(lport) {
            if ((speed_1000_pbmp & (0x1 << lport)) && (SOC_PORT_SPEED_MAX(lport) >= 1000)){
               port_speed[lport] = 1000;
            }
        }
    }

    sal_config_rv = sal_config_pbmp_get(SAL_CONFIG_SPEED_2500_PORTS, &speed_2500_pbmp);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set speed (2.5G) logical pbmp with value 0x%x.\n", speed_2500_pbmp);
        SOC_LPORT_ITER(lport) {
            if ((speed_2500_pbmp & (0x1 << lport)) && (SOC_PORT_SPEED_MAX(lport) >= 2500)){
               port_speed[lport] = 2500;
            }
        }
    }

    sal_config_rv = sal_config_pbmp_get(SAL_CONFIG_SPEED_5000_PORTS, &speed_5000_pbmp);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set speed (5G) logical pbmp with value 0x%x.\n", speed_5000_pbmp);
        SOC_LPORT_ITER(lport) {
            if ((speed_5000_pbmp & (0x1 << lport)) && (SOC_PORT_SPEED_MAX(lport) >= 5000)){
               port_speed[lport] = 5000;
            }
        }
    }

    sal_config_rv = sal_config_pbmp_get(SAL_CONFIG_SPEED_10000_PORTS, &speed_10000_pbmp);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set speed (10G) logical pbmp with value 0x%x.\n", speed_10000_pbmp);
        SOC_LPORT_ITER(lport) {
            if ((speed_10000_pbmp & (0x1 << lport)) && (SOC_PORT_SPEED_MAX(lport) >= 10000)){
               port_speed[lport] = 10000;
            }
        }
    }
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
    bcm5340x_reg_get(unit, R_TOP_STRAP_STATUS, &top_strap_val);
    if (((TOP_STRAP_STATUS_TSC_FIELD(top_strap_val) >> 5) & 0x1) == 0) {
        if ((gh_sw_info.devid == BCM53401_DEVICE_ID) ||
            (gh_sw_info.devid == BCM53411_DEVICE_ID) ||
            (gh_sw_info.devid == BCM53456_DEVICE_ID) ||
            (gh_sw_info.devid == BCM53457_DEVICE_ID) ||
            (gh_sw_info.devid == BCM53424_DEVICE_ID)) {
            /* Release QSGMII reset state after QSGMII-PCS and unimac init */

            bcm5340x_reg_set(unit, R_CHIP_SWRST, 0xF0);
        }
    }
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
    an_rv = sal_config_pbmp_get(SAL_CONFIG_PHY_AN_PORTS, &phy_an_pbmp);
    if (an_rv == SYS_OK) {
        sal_printf("Vendor Config : Set AN logical pbmp with value 0x%x.\n", phy_an_pbmp);
    }

    cl73_rv = sal_config_pbmp_get(SAL_CONFIG_PHY_CL73_PORTS, &phy_cl73_pbmp);
    if (cl73_rv == SYS_OK) {
        sal_printf("Vendor Config : Set CL73 logical pbmp with value 0x%x.\n", phy_cl73_pbmp);
    }

    cl37_rv = sal_config_pbmp_get(SAL_CONFIG_PHY_CL37_PORTS, &phy_cl37_pbmp);
    if (cl37_rv == SYS_OK) {
        sal_printf("Vendor Config : Set CL37 logical pbmp with value 0x%x.\n", phy_cl37_pbmp);
    }
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */

    /* Decide which ports need to set to vco 6.25 */
    vco_6250_pbmp = 0;
    SOC_LPORT_ITER(lport) {
        if (port_speed[lport] == 2500) {
            /* Only need to do once at the 1st 2.5g port */
            if (flag_act) {
                break;
            }
            flag_act = 1;
            flag_vco = 1;
            SOC_LPORT_ITER(cmp_lport) {
                if (SOC_PORT_BLOCK(lport) == SOC_PORT_BLOCK(cmp_lport)) {
                    if (port_speed[cmp_lport] == 1000) {
                        if (flag_vco == 1) {
                            flag_vco = 2;
                        }
                    } else if ((port_speed[cmp_lport] == 10000) || (port_speed[cmp_lport] == 5000)) {
                        if (flag_vco == 1) {
                            flag_vco = 0;
                        }
                    }
                }
            }
            /* flag_vco == 0: vco used default 10.3125
                           1: vco used 6.25
                           2: vco used 6.25 (there are 1G ports)
            */
            if ((flag_vco == 1) || (flag_vco == 2)) {
                SOC_LPORT_ITER(cmp_lport) {
                    if (SOC_PORT_BLOCK(lport) == SOC_PORT_BLOCK(cmp_lport)) {
                        vco_6250_pbmp |= 0x1 << cmp_lport;
                    }
                }
            }
        }
    }

    SOC_LPORT_ITER(lport) {
        int ability, an;

        /* According the speed to configure the phy ability */
        ability = (BMD_PHY_ABIL_1000MB_FD | BMD_PHY_ABIL_100MB_FD);
        if (port_speed[lport] == 1000) {
            ability |= BMD_PHY_ABIL_10MB_FD;
        } else if (port_speed[lport] == 2500) {
            ability |= BMD_PHY_ABIL_2500MB;
        } else if (port_speed[lport] == 10000) {
            ability |= (BMD_PHY_ABIL_10GB | BMD_PHY_ABIL_2500MB);
        } else if (port_speed[lport] == 13000) {
            ability |= BMD_PHY_ABIL_13GB;
        }
        rv = bmd_phy_ability_set(unit, lport, "bcmi_tsce_xgxs", ability);
        if (!SOC_SUCCESS(rv)) {
            sal_printf("bcm5340x_sw_init set phy ability 0x%x on lport %d failed\n", ability, lport);
        }

        phy_ctrl_t *pc = BMD_PORT_PHY_CTRL(unit, lport);
        
        if (port_speed[lport] == 1000) {
            if (tsce_interface == TSCE_INTERFACE_SGMII) {
                an = 0;
            } else {
                an = CFG_CONFIG_1G_PORT_AN;
                if (an == 2) {
                    PHY_CTRL_FLAGS(pc) |= PHY_F_CLAUSE37;
                } else if (an == 1) {
                    PHY_CTRL_FLAGS(pc) |= PHY_F_CLAUSE73;
                }
                
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
                if (an_rv == SYS_OK){
                    if (phy_an_pbmp & (0x1 << lport)) {
                        /* Set or clear CL37 flag */
                        if (cl37_rv == SYS_OK) {
                            if (!(PHY_CTRL_FLAGS(pc) & PHY_F_CLAUSE37)) {
                                if  (phy_cl37_pbmp & (0x1 << lport)) {
                                    /* set cl37 flag*/
                                    PHY_CTRL_FLAGS(pc) |= PHY_F_CLAUSE37;
                                }
                            } else {
                                if  (!(phy_cl37_pbmp & (0x1 << lport))) {
                                    /* clear cl37 flag*/
                                    PHY_CTRL_FLAGS(pc) &= ~PHY_F_CLAUSE37;
                                }
                            }
                        }
                        
                        /* Set or clear CL73 flag */
                        if (cl73_rv == SYS_OK) {
                            if (!(PHY_CTRL_FLAGS(pc) & PHY_F_CLAUSE73)) {
                                if  (phy_cl73_pbmp & (0x1 << lport)) {
                                    /* set cl73 flag*/
                                    PHY_CTRL_FLAGS(pc) |= PHY_F_CLAUSE73;
                                }
                            } else {
                                /* an = 1 */
                                if  (!(phy_cl73_pbmp & (0x1 << lport))) {
                                    /* clear cl73 flag*/
                                    PHY_CTRL_FLAGS(pc) &= ~PHY_F_CLAUSE73;
                                }
                            }
                        }
                        an = 1;
                    } else {
                        an = 0;
                    }
                }
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
            }

            /* Detect if 1G need to use 6.25 vco */
            if (vco_6250_pbmp & (0x1 << lport)) {
                PHY_CTRL_FLAGS(pc) |= PHY_F_6250_VCO;
            }

            an = an ? TRUE : FALSE;
        } else if (port_speed[lport] == 10000) {
            an = CFG_CONFIG_10G_PORT_AN;
            PHY_CTRL_FLAGS(pc) |= PHY_F_CLAUSE73;
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
             if (an_rv == SYS_OK){
                if (phy_an_pbmp & (0x1 << lport)) {
                    /* Clause 73 */
                    an = 1;
                } else {
                    an = 0;
                }
             }
#endif  /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */          

             /*  The default mode of 10G port is force mode with XFI interface.
                 User can change it to SFI interface by calling  bmd_phy_line_interface_set.
             if (!an) {
                 rv = bmd_phy_line_interface_set(unit, lport, BMD_PHY_IF_SFI);
             } */

             an = an ? TRUE : FALSE;
             
        } else if (port_speed[lport] == 2500) {
            an = FALSE;
            /* speed 2.5 set to 6.25 vco */
            if (vco_6250_pbmp & (0x1 << lport)) {
                PHY_CTRL_FLAGS(pc) |= PHY_F_6250_VCO;
            }
        }else if (port_speed[lport] == 5000) {
            an = FALSE;
        } else {
            sal_printf("bcm5340x_sw_init : wrong max speed %d on port %d\n", port_speed[lport], lport);
            return SYS_ERR;
        }

        if (!bmd_phy_external_mode_get(unit, lport)) {
            rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, lport), an);
            if (!SOC_SUCCESS(rv)) {
                sal_printf("bcm5340x_sw_init set phy autoneg %d on lport %d failed\n", an, lport);
            }

            /* only serdes phy, set the autoneg need to use speed set to active this port */
            rv = PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, lport), port_speed[lport]);
        } else {
            /* For current design of external PHY driver, AN needs to be set to diabled before calling phy_speed_set */
            rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, lport), FALSE);
            if (!SOC_SUCCESS(rv)) {
                sal_printf("bcm5340x_sw_init set phy autoneg %d on lport %d failed\n", an, lport);
            }

            rv = PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, lport), port_speed[lport]);
            if (!SOC_SUCCESS(rv)) {
                sal_printf("bcm5340x_sw_init set phy speed %d on lport %d failed\n", port_speed[lport], lport);
            }

            /* Let AN to be enabled always for copper ports */
            rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, lport), TRUE);
            if (!SOC_SUCCESS(rv)) {
                sal_printf("bcm5340x_sw_init set phy autoneg %d on lport %d failed\n", an, lport);
            }
        }
    }

    /* Init MACs */
    SOC_LPORT_ITER(lport) {
        if (IS_XL_PORT(lport)) {
            gh_sw_info.p_mac[lport] = &soc_mac_xl;
        } else {
            gh_sw_info.p_mac[lport] = &soc_mac_uni;
        }

        MAC_INIT(gh_sw_info.p_mac[lport], unit, lport);
        /* Probe function should leave port disabled */
        MAC_ENABLE_SET(gh_sw_info.p_mac[lport], unit, lport, FALSE);

        gh_sw_info.link[lport] = PORT_LINK_DOWN;
#if defined(CFG_SWITCH_EEE_INCLUDED)
        gh_sw_info.need_process_for_eee_1s[lport] = FALSE;
#endif /*  CFG_SWITCH_EEE_INCLUDED */
    }

    bcm5340x_load_led_program(unit);

#if CONFIG_GREYHOUND_EMULATION
    SOC_LPORT_ITER(lport) {
        link_qt[lport] = PORT_LINK_UP;
    }
    bcm5340x_linkscan_task(NULL);
    sal_printf("all ports up!\n");
#else
    /* Register background process for handling link status */
    timer_add(bcm5340x_linkscan_task, NULL, LINKSCAN_INTERVAL);
#endif /* CONFIG_GREYHOUND_EMULATION */

    if (rv) {
        return SYS_ERR;
    } else {
        return SYS_OK;
    }
}

/* Before FP_TCAM insert, encode FP_TCAM entry.
 * dm_entry: Original data
 * xy_entry: encode data
 * data_word: size of one FP_TCAM entry
 * bit_length: full_key/full_mask bit length
 */
void
bcm5340x_dm_to_xy(uint32 *dm_entry, uint32 *xy_entry, int data_words, int bit_length)
{
    int i;
    int data_bytes;
    int word_length = (bit_length + 31) / 32;;
    uint32 temp_tcam_entry[data_words];

    uint32 key[word_length];
    uint32 mask[word_length];
    uint32 converted_key, converted_mask;
    uint32 xor_value;

    xor_value = 0xffffffff;
    data_bytes = data_words * sizeof(uint32);
    sal_memcpy(temp_tcam_entry, dm_entry, data_bytes);
    for (i = 0; i < word_length; i++) {
        key[i] = (temp_tcam_entry[i] >> 2) | ((temp_tcam_entry[i+1] & 0x3) << 30);
        mask[i] = (temp_tcam_entry[i+7] >> 12) | ((temp_tcam_entry[i+8] & 0x1fff) << 20);
        if (i == 7) {
            key[i] = (temp_tcam_entry[i] >> 2) & 0x3ff;
            mask[i] = (temp_tcam_entry[i+7] >> 12) & 0x3ff;
        }
    }
    for (i = 0; i < word_length; i++) {
        converted_key = key[i] & mask[i];
        converted_mask = (key[i] | ~mask[i]) ^ xor_value;
        mask[i] = converted_mask;
        key[i] = converted_key;
    }
    if ((bit_length & 0x1f) != 0) {
        mask[i - 1] &= (1 << (bit_length & 0x1f)) - 1;
    }
    for (i = 0; i < data_words; i++) {
        if (i == 0) {
            temp_tcam_entry[i] = (dm_entry[0] & 0x3) | ((key[0] << 2) & 0xfffffffc);
        } else if (i <= 6) {
            temp_tcam_entry[i] = ((key[i-1] >> 30) & 0x3) | ((key[i] << 2) & 0xfffffffc);
        } else if (i == 7) {
            temp_tcam_entry[i] = ((key[i-1] >> 30) & 0x3) | ((key[i] << 2) & 0xffc) |
                          (mask[i-7] << 12);
        } else if (i <= 13) {
            temp_tcam_entry[i] = ((mask[i-7-1] >> 20) & 0xfff) | ((mask[i-7] << 12) & 0xfffff000);
        } else {
            temp_tcam_entry[i] = ((mask[i-7-1] >> 20) & 0xfff) | ((mask[i-7] << 12) & 0x2fff000);
        }
    }
    sal_memcpy(xy_entry, temp_tcam_entry, data_bytes);

}

/* Before Decode FP_TCAM entry.
 * dm_entry: Original data
 * xy_entry: encode data
 * data_word: size of one FP_TCAM entry
 * bit_length: full_key/full_mask bit length
 */
void
bcm5340x_xy_to_dm(uint32 *xy_entry, uint32 *dm_entry, int data_words, int bit_length)
{
    int i;
    int data_bytes;
    int word_length = (bit_length + 31) / 32;;
    uint32 temp_tcam_entry[data_words];

    uint32 key[word_length];
    uint32 mask[word_length];
    uint32 xor_value;

    xor_value = 0;
    data_bytes = data_words * sizeof(uint32);
    sal_memcpy(dm_entry, xy_entry, data_bytes);
    sal_memcpy(temp_tcam_entry, xy_entry, data_bytes);

    for (i = 0; i < word_length; i++) {
        key[i] = (temp_tcam_entry[i] >> 2) | ((temp_tcam_entry[i+1] & 0x3) << 30);
        mask[i] = (temp_tcam_entry[i+7] >> 12) | ((temp_tcam_entry[i+8] & 0x1fff) << 20);
        if (i == 7) {
            key[i] = (temp_tcam_entry[i] >> 2) & 0x3ff;
            mask[i] = (temp_tcam_entry[i+7] >> 12) & 0x3ff;
        }
    }
    for (i = 0; i < word_length; i++) {
        mask[i] = key[i] | (mask[i] ^ xor_value);
    }
    if ((bit_length & 0x1f) != 0) {
        mask[i - 1] &= (1 << (bit_length & 0x1f)) - 1;
    }

    for (i = 0; i < data_words; i++) {
        if (i == 0) {
            temp_tcam_entry[i] = (temp_tcam_entry[0] & 0x3) | ((key[0] << 2) & 0xfffffffc);
        } else if (i <= 6) {
            temp_tcam_entry[i] = ((key[i-1] >> 30) & 0x3) | ((key[i] << 2) & 0xfffffffc);
        } else if (i == 7) {
            temp_tcam_entry[i] = ((key[i-1] >> 30) & 0x3) | ((key[i] << 2) & 0xffc) |
                          (mask[i-7] << 12);
        } else if (i <= 13) {
            temp_tcam_entry[i] = ((mask[i-7-1] >> 20) & 0xfff) | ((mask[i-7] << 12) & 0xfffff000);
        } else {
            temp_tcam_entry[i] = ((mask[i-7-1] >> 20) & 0xfff) | ((mask[i-7] << 12) & 0x2fff000);
        }
    }
    sal_memcpy(dm_entry, temp_tcam_entry, data_bytes);
}

void
bcm5340x_loopback_enable(uint8 unit, uint8 port, int loopback_mode)
{
    int rv = 0;
    int link;
    int andone;

    if (loopback_mode == PORT_LOOPBACK_NONE) {
        if (gh_sw_info.loopback[port] != PORT_LOOPBACK_NONE) {
            if (gh_sw_info.loopback[port] == PORT_LOOPBACK_MAC) {
                bcm5340x_handle_link_down(unit, port, TRUE);
            } else if (gh_sw_info.loopback[port] == PORT_LOOPBACK_PHY) {
                rv = PHY_LOOPBACK_SET(BMD_PORT_PHY_CTRL(unit, port), 0);
                rv = PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit, port), &link, &andone);
            }
            gh_sw_info.loopback[port] = PORT_LOOPBACK_NONE;
        }
        return;
    }

    gh_sw_info.loopback[port] = loopback_mode;

    rv = PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit, port), &link, &andone);
    if (rv < 0) {
#if CFG_CONSOLE_ENABLED
        sal_printf("Failed to get link of port %d\n", (int)port);
#endif /* CFG_CONSOLE_ENABLED */
        return;
    }

    if (link) {
        /* Force link change */
        sal_printf("force port %d link change\n", port);
        gh_sw_info.link[port] = PORT_LINK_DOWN;
    }

    if (loopback_mode == PORT_LOOPBACK_PHY) {
        rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, port), 0);
        rv |= PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, port), SOC_PORT_SPEED_MAX(port));
        rv |= PHY_LOOPBACK_SET(BMD_PORT_PHY_CTRL(unit, port), 1);
        if (rv < 0) {
#if CFG_CONSOLE_ENABLED
            sal_printf("Failed to set phy loopback of port %d\n", (int)port);
#endif /* CFG_CONSOLE_ENABLED */
        }
    }
}

soc_switch_t soc_switch_bcm5340x =
{
    bcm5340x_chip_type,
    NULL,
    bcm5340x_port_count,
    NULL,
    NULL,
#if CFG_RXTX_SUPPORT_ENABLED
    bcm5340x_rx_set_handler,
    bcm5340x_rx_fill_buffer,
    bcm5340x_tx,
#endif /* CFG_RXTX_SUPPORT_ENABLED */

    bcm5340x_link_status,
    bcm5340x_chip_revision,
    bcm5340x_reg_get,
    bcm5340x_reg_set,
    bcm5340x_mem_get,
    bcm5340x_mem_set,
#ifdef CFG_SWITCH_VLAN_INCLUDED
    bcm5340x_pvlan_egress_set,
    bcm5340x_pvlan_egress_get,
    bcm5340x_qvlan_port_set,
    bcm5340x_qvlan_port_get,
    bcm5340x_vlan_create,
    bcm5340x_vlan_destroy,
    bcm5340x_vlan_type_set,
    bcm5340x_vlan_reset,
#endif  /* CFG_SWITCH_VLAN_INCLUDED */
    bcm5340x_phy_reg_get,
    bcm5340x_phy_reg_set,
};

