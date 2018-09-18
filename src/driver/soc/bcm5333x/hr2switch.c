/*
 * $Id: hr2switch.c,v 1.70.2.1 Broadcom SDK $
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
#include "soc/bcm5333x.h"
#include "utils/ui.h"

#define LINKSCAN_INTERVAL        (100000UL)   /* 100 ms */

#define PORT_LINK_UP                    (1)
#define PORT_LINK_DOWN                  (0)

bcm5333x_sw_info_t hr2_sw_info;
uint8 config_id = CFG_CONFIG_OPTION;
uint8 lport_active[BCM5333X_LPORT_MAX + 1] = {
                      1, 1, 1, 1, 1, 1, 1, 1,
                      1, 1, 1, 1, 1, 1, 1, 1,
                      1, 1, 1, 1, 1, 1, 1, 1,
                      1, 1, 1, 1, 1, 1};


/* Powersave Mode TDM, in physical port number */
static const uint32 hu2_tdm_powersave[48] = {
                      2,10,18,63,63,0,
                      3,11,19,63,63,63,
                      4,12,20,63,63,63,
                      5,13,21,63,63,63,
                      6,14,22,63,63,63,
                      7,15,23,63,63,63,
                      8,16,24,63,63,63,
                      9,17,25,63,63,63};
/* Powersave Plus Mode TDM */
static const uint32 hu2_tdm_powersave_plus[48] = {
                      2,10,18,26,63,0,
                      3,11,19,63,63,63,
                      4,12,20,27,63,63,
                      5,13,21,63,63,63,
                      6,14,22,28,63,63,
                      7,15,23,63,63,63,
                      8,16,24,29,63,63,
                      9,17,25,63,63,63};
/* Embedded Mode TDM */
static const uint32 hu2_tdm_embedded[60] = {
                      26,27,28,29,30,31,
                      26,27,28,29,63,63,
                      26,27,28,29,2,63,
                      26,27,28,29,10,63,
                      26,27,28,29,18,0,
                      26,27,28,29,32,33,
                      26,27,28,29,63,63,
                      26,27,28,29,6,63,
                      26,27,28,29,14,63,
                      26,27,28,29,22,63};
/* Embedded Plus Mode TDM */
static const uint32 hu2_tdm_embedded_plus[60] = {
                      26,30,31,32,63,63,
                      26,30,31,32,63,63,
                      26,30,31,32,2,63,
                      26,30,31,32,10,63,
                      26,30,31,32,18,0,
                      26,30,31,32,63,63,
                      26,30,31,32,63,63,
                      26,30,31,32,6,63,
                      26,30,31,32,14,63,
                      26,30,31,32,22,63};
/* Cascade Mode TDM */
static const uint32 hu2_tdm_cascade[75] = {2,14,26,28,30,32,
                      3,15,26,28,30,32,
                      4,16,26,28,30,32,
                      5,17,26,28,30,32,
                      0,  
                      6,18,26,28,30,32,
                      7,19,26,28,30,32,
                      8,20,26,28,30,32,
                      9,21,26,28,30,32,
                      63,
                      10,22,26,28,30,32,
                      11,23,26,28,30,32,
                      12,24,26,28,30,32,
                      13,25,26,28,30,32,
                      63};
/* XAUI Mode TDM */
static const uint32 hu2_tdm_xaui[75] = {2,14,26,27,28,30,
                      3,15,26,27,28,30,
                      4,16,26,27,28,30,
                      5,17,26,27,28,30,
                      0,  
                      6,18,26,27,28,30,
                      7,19,26,27,28,30,
                      8,20,26,27,28,30,
                      9,21,26,27,28,30,
                      63,
                      10,22,26,27,28,30,
                      11,23,26,27,28,30,
                      12,24,26,27,28,30,
                      13,25,26,27,28,30,
                      63};
/* Non-Cascade Mode TDM */
static const uint32 hu2_tdm_non_cascade[75] = {2,14,26,27,28,29,
                      3,15,26,27,28,29,
                      4,16,26,27,28,29,
                      5,17,26,27,28,29,
                      0,
                      6,18,26,27,28,29,
                      7,19,26,27,28,29,
                      8,20,26,27,28,29,
                      9,21,26,27,28,29,
                      63,
                      10,22,26,27,28,29,
                      11,23,26,27,28,29,
                      12,24,26,27,28,29,
                      13,25,26,27,28,29,
                      63};

/* Non-Cascade Mode TDM (option 1b)*/
static const uint32 hu2_tdm_non_cascade_1b[75] = {2,14,30,31,32,33,
                      3,15,30,31,32,33,
                      4,16,30,31,32,33,
                      5,17,30,31,32,33,
                      0,
                      6,18,30,31,32,33,
                      7,19,30,31,32,33,
                      8,20,30,31,32,33,
                      9,21,30,31,32,33,
                      63,
                      10,22,30,31,32,33,
                      11,23,30,31,32,33,
                      12,24,30,31,32,33,
                      13,25,30,31,32,33,
                      63};
/*
 * Pysical to logical port mappings:
 * QGPHY0 and GPORT0 block (physical port 2 ~ 9) are flipped(mirrored) in
 * physical design layout to better utilize area.
 *
 * Physical    Logical
 *    2          9
 *    3          8
 *    4          7
 *    5          6
 *    6          5
 *    7          4
 *    8          3
 *    9          2
 */
static const int p2l_mapping_24_0_0[] = { /* Powersave */
          0, -1,
          9,  8,  7,  6,  5,  4,  3,  2,
         10, 11, 12, 13, 14, 15, 16, 17,
         18, 19, 20, 21, 22, 23, 24, 25,
         -1, -1, -1, -1,
         -1, -1, -1, -1 };
static const int port_speed_max_24x1g[] = {
         -1, -1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
         -1, -1, -1, -1,
         -1, -1, -1, -1 };

static const int p2l_mapping_16_0_0[] = { /* Powersave */
          0, -1,
          9,  8,  7,  6,  5,  4,  3,  2,
         10, 11, 12, 13, 14, 15, 16, 17,
         -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1,
         -1, -1, -1, -1 };
static const int port_speed_max_16x1g[] = {
         -1, -1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
         -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1,
         -1, -1, -1, -1 };

static const int p2l_mapping_16_4_0[] = { /* Powersave */
          0, -1,
          9,  8,  7,  6,  5,  4,  3,  2,
         10, 11, 12, 13, 14, 15, 16, 17,
         -1, -1, -1, -1, -1, -1, -1, -1,
         27, 26, 29, 28,
         -1, -1, -1, -1 };
static const int port_speed_max_16x1g_4x1g[] = {
         -1, -1,
          1,   1,   1,   1,   1,   1,   1,   1,
          1,   1,   1,   1,   1,   1,   1,   1,
         -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
          1,   1,   1,   1,
         -1,  -1,  -1,  -1 };
static const int p2l_mapping_8_0_0[] = { /* Powersave */
          0, -1,
          9,  8,  7,  6,  5,  4,  3,  2,
         -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1,
         -1, -1, -1, -1 };

static const int port_speed_max_8x1g[] = {
         -1, -1,
          1,  1,  1,  1,  1,  1,  1,  1,
         -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1,
         -1, -1, -1, -1 };

static const int p2l_mapping_6_4_4[] = { /* Embedded */
          0, -1,
          2, -1, -1, -1,  6, -1, -1, -1,
         10, -1, -1, -1, 14, -1, -1, -1,
         18, -1, -1, -1, 22, -1, -1, -1,
         26, 27, 28, 29,
         21, 23, 24, 25 };
static const int port_speed_max_6x1g_4x1g_4x1g[] = {
         -1, -1,
          1, -1, -1, -1,  1, -1, -1, -1,
          1, -1, -1, -1,  1, -1, -1, -1,
          1, -1, -1, -1,  1, -1, -1, -1,
          1,  1,  1,  1,
          1,  1,  1,  1 };
static const int port_speed_max_6x1g_4x10g_4x1g[] = {
         -1, -1,
          1, -1, -1, -1,  1, -1, -1, -1,
          1, -1, -1, -1,  1, -1, -1, -1,
          1, -1, -1, -1,  1, -1, -1, -1,
         11, 11, 11, 11,
          1,  1,  1,  1 };

static const int p2l_mapping_6_1_4[] = { /* Embedded - XAUI */
          0, -1,
          2, -1, -1, -1,  6, -1, -1, -1,
         10, -1, -1, -1, 14, -1, -1, -1,
         18, -1, -1, -1, 22, -1, -1, -1,
         26, -1, -1, -1,
         21, 23, 24, 25 };
static const int port_speed_max_6x1g_1x10g_4x1g[] = {
         -1, -1,
          1, -1, -1, -1,  1, -1, -1, -1,
          1, -1, -1, -1,  1, -1, -1, -1,
          1, -1, -1, -1,  1, -1, -1, -1,
         10, -1, -1, -1,
          1,  1,  1,  1 };

static const int p2l_mapping_6_1_3[] = { /* Embedded Plus - XAUI */
          0, -1,
          2, -1, -1, -1,  6, -1, -1, -1,
         10, -1, -1, -1, 14, -1, -1, -1,
         18, -1, -1, -1, 22, -1, -1, -1,
         26, -1, -1, -1,
         27, 28, 29, -1 };
static const int port_speed_max_6x1g_1x10g_3x10g[] = {
         -1, -1,
          1, -1, -1, -1,  1, -1, -1, -1,
          1, -1, -1, -1,  1, -1, -1, -1,
          1, -1, -1, -1,  1, -1, -1, -1,
         10, -1, -1, -1,
         11, 11, 11,  -1 };

/* Wolfhound Option 1: 24P 1G + 4P 1G  (PHY) */
/* In Wolfhound ref platform, TSC ports are Even-Odd swapped */
static const int p2l_mapping_24_4_0_wh[] = {
          0, -1,
          9,  8,  7,  6,  5,  4,  3,  2,
         10, 11, 12, 13, 14, 15, 16, 17,
         18, 19, 20, 21, 22, 23, 24, 25,
         27, 26, 29, 28,
         -1, -1, -1, -1
    };
static const int port_speed_max_24x1g_4x1g[] = {
         -1, -1,
          1,   1,   1,   1,   1,   1,   1,   1,
          1,   1,   1,   1,   1,   1,   1,   1,
          1,   1,   1,   1,   1,   1,   1,   1,
          1,   1,   1,   1,
         -1,  -1,  -1,  -1
    };
/* Option 2: 24P 1G + 2P 1G + 2P 13G (PHY) */
static const int p2l_mapping_24_2_2[] =  { /* Cascade */
          0, -1,
          9,  8,  7,  6,  5,  4,  3,  2,
         10, 11, 12, 13, 14, 15, 16, 17,
         18, 19, 20, 21, 22, 23, 24, 25,
         26, -1, 27, -1,
         28, -1, 29, -1
    };
static const int port_speed_max_24x1g_2x1g_2x13g[] = {
         -1, -1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1, -1, 1, -1,
         13, -1, 13, -1
    };
static const int port_speed_max_24x1g_2x13g_2x1g[] = {
         -1, -1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
         13, -1, 13, -1,
          1, -1,  1, -1
    };
static const int port_speed_max_24x1g_2x10g_2x13g[] = {
         -1, -1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
         11, -1, 11, -1,
         13, -1, 13, -1
    };
/* Option 2A: 24P 1G + 2P 13G + 2P 1G/10G (PHY) */
static const int port_speed_max_24x1g_2x13g_2x10g[] = {
         -1, -1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
         13, -1, 13, -1,
         11, -1, 11, -1
    };
static const int port_speed_max_24x1g_2x10g_2x10g[] = {
         -1, -1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
         11, -1, 11, -1,
         11, -1, 11, -1
    };
static const int p2l_mapping_24_4_0[] = { /* Non-Cascade, Powersave Plus */
          0, -1,
          9,  8,  7,  6,  5,  4,  3,  2,
         10, 11, 12, 13, 14, 15, 16, 17,
         18, 19, 20, 21, 22, 23, 24, 25,
         26, 27, 28, 29,
         -1, -1, -1, -1
    };
static const int port_speed_max_24x1g_4x10g[] = {
         -1, -1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
         11, 11, 11, 11,
         -1, -1, -1, -1
    };
static const int p2l_mapping_24_0_4[] = { /* Non-Cascade, Powersave Plus */
          0, -1,
          9,  8,  7,  6,  5,  4,  3,  2,
         10, 11, 12, 13, 14, 15, 16, 17,
         18, 19, 20, 21, 22, 23, 24, 25,
         -1, -1, -1, -1,
         26, 27, 28, 29
    };
static const int port_speed_max_24x1g_4x10g_tsc1[] = {
         -1, -1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
          1,  1,  1,  1,  1,  1,  1,  1,
         -1, -1, -1, -1,
         11, 11, 11, 11
    };
static const int p2l_mapping_24_3_1[] =  { /* XAUI */
          0, -1,
          9,  8,  7,  6,  5,  4,  3,  2,
         10, 11, 12, 13, 14, 15, 16, 17,
         18, 19, 20, 21, 22, 23, 24, 25,
         26, 27, 28, -1,
         29, -1, -1, -1
    };
static const int port_speed_max_24x1g_3x10g_1x10g[] = {
        -1, -1,
         1,  1,  1,  1,  1,  1,  1,  1,
         1,  1,  1,  1,  1,  1,  1,  1,
         1,  1,  1,  1,  1,  1,  1,  1,
        11, 11, 11, -1,
        10, -1, -1, -1
    };
static const int p2l_mapping_24_1_1[] =  { /* 2xXAUI */
          0, -1,
          9,  8,  7,  6,  5,  4,  3,  2,
         10, 11, 12, 13, 14, 15, 16, 17,
         18, 19, 20, 21, 22, 23, 24, 25,
         26, -1, -1, -1,
         27, -1, -1, -1
    };
static const int port_speed_max_24x1g_1x10g_1x10g[] = {
        -1, -1,
         1,  1,  1,  1,  1,  1,  1,  1,
         1,  1,  1,  1,  1,  1,  1,  1,
         1,  1,  1,  1,  1,  1,  1,  1,
        10, -1, -1, -1,
        10, -1, -1, -1
    };
#ifdef CFG_LED_MICROCODE_INCLUDED
/* Left LED : Link,   Right LED : TX/RX activity */
static uint8 led_program_1[] = {
    0x02, 0x02, 0x28, 0x60, 0xE3, 0x67, 0x70, 0x75,
    0x0D, 0x67, 0x50, 0x77, 0x0F, 0x67, 0x22, 0x06,
    0xE3, 0x80, 0xD2, 0x1A, 0x74, 0x02, 0x12, 0xE0,
    0x85, 0x05, 0xD2, 0x03, 0x71, 0x20, 0x52, 0x00,
    0x3A, 0x60, 0x32, 0x00, 0x32, 0x01, 0xB7, 0x97,
    0x75, 0x31, 0x12, 0xE4, 0xFE, 0xE3, 0x02, 0x01,
    0x50, 0x12, 0xE4, 0xFE, 0xE3, 0x95, 0x75, 0x41,
    0x85, 0x06, 0xE3, 0x67, 0x49, 0x75, 0xAB, 0x77,
    0x91, 0x16, 0xE0, 0xDA, 0x01, 0x71, 0x91, 0x77,
    0x9E, 0x12, 0xA0, 0xF8, 0x15, 0x1A, 0x00, 0x57,
    0x12, 0xA0, 0xF8, 0x15, 0x1A, 0x01, 0x75, 0x5F,
    0x12, 0xE4, 0xFE, 0xE3, 0x02, 0x01, 0x50, 0x12,
    0xE4, 0xFE, 0xE3, 0x95, 0x75, 0x68, 0x85, 0x57,
    0x16, 0xE0, 0xDA, 0x01, 0x71, 0x77, 0x77, 0x84,
    0x12, 0xA0, 0xF8, 0x15, 0x1A, 0x01, 0x57, 0x32,
    0x0E, 0x87, 0x32, 0x0E, 0x87, 0x32, 0x0E, 0x87,
    0x32, 0x0E, 0x87, 0x57, 0x32, 0x0F, 0x87, 0x32,
    0x0F, 0x87, 0x32, 0x0E, 0x87, 0x32, 0x0E, 0x87,
    0x57, 0x32, 0x0F, 0x87, 0x32, 0x0E, 0x87, 0x32,
    0x0E, 0x87, 0x32, 0x0E, 0x87, 0x57, 0x32, 0x0F,
    0x87, 0x32, 0x0F, 0x87, 0x32, 0x0E, 0x87, 0x32,
    0x0E, 0x87, 0x57, 0x32, 0x0E, 0x87, 0x32, 0x0E,
    0x87, 0x32, 0x0E, 0x87, 0x32, 0x0E, 0x87, 0x57};

 /* Left LED : TX/RX activity,   Right LED : LINK */
 static uint8 led_program_2[] = {
    0x02, 0x02, 0x28, 0x60, 0xE3, 0x67, 0x70, 0x75,
    0x0D, 0x67, 0x50, 0x77, 0x0F, 0x67, 0x22, 0x06,
    0xE3, 0x80, 0xD2, 0x1A, 0x74, 0x02, 0x12, 0xE0,
    0x85, 0x05, 0xD2, 0x03, 0x71, 0x20, 0x52, 0x00,
    0x3A, 0x60, 0x32, 0x00, 0x32, 0x01, 0xB7, 0x97,
    0x75, 0x31, 0x12, 0xE4, 0xFE, 0xE3, 0x02, 0x01,
    0x50, 0x12, 0xE4, 0xFE, 0xE3, 0x95, 0x75, 0x41,
    0x85, 0x06, 0xE3, 0x67, 0x49, 0x75, 0xAB, 0x77,
    0x91, 0x16, 0xE0, 0xDA, 0x01, 0x71, 0x91, 0x77,
    0x9E, 0x12, 0xA0, 0xF8, 0x15, 0x1A, 0x00, 0x57,
    0x12, 0xA0, 0xF8, 0x15, 0x1A, 0x01, 0x75, 0x5F,
    0x12, 0xE4, 0xFE, 0xE3, 0x02, 0x01, 0x50, 0x12,
    0xE4, 0xFE, 0xE3, 0x95, 0x75, 0x68, 0x85, 0x57,
    0x16, 0xE0, 0xDA, 0x01, 0x71, 0x77, 0x77, 0x84,
    0x12, 0xA0, 0xF8, 0x15, 0x1A, 0x01, 0x57, 0x32,
    0x0E, 0x87, 0x32, 0x0E, 0x87, 0x32, 0x0E, 0x87,
    0x32, 0x0E, 0x87, 0x57, 0x32, 0x0F, 0x87, 0x32,
    0x0F, 0x87, 0x32, 0x0E, 0x87, 0x32, 0x0E, 0x87,
    0x57, 0x32, 0x0E, 0x87, 0x32, 0x0F, 0x87, 0x32,
    0x0E, 0x87, 0x32, 0x0E, 0x87, 0x57, 0x32, 0x0F,
    0x87, 0x32, 0x0F, 0x87, 0x32, 0x0E, 0x87, 0x32,
    0x0E, 0x87, 0x57, 0x32, 0x0E, 0x87, 0x32, 0x0E,
    0x87, 0x32, 0x0E, 0x87, 0x32, 0x0E, 0x87, 0x57};
#endif /* CFG_LED_MICROCODE_INCLUDED */

uint32 counter_val[BCM5333X_LPORT_MAX][R_MAX * 2];


static void bcm5333x_load_led_program(void);


void
bcm5333x_handle_link_up(uint8 unit, uint8 lport, int changed, uint32 *flags)
{
    int rv;
    uint32 val;
    int duplex;
    uint32 speed;
    BOOL   tx_pause, rx_pause;
    int an = 0;

    if (1 == changed) {
        /* Port changes to link up from link down */

        if (hr2_sw_info.loopback[lport] == PORT_LOOPBACK_MAC) {
            /* Force link up in mac loopback mode */
            speed = SOC_PORT_SPEED_MAX(lport)*1000;
            if (speed == 11000) {
                speed = 10000;
            }
            duplex = TRUE;
            an = tx_pause = rx_pause = FALSE;
        } else {
        
            rv = PHY_SPEED_GET(BMD_PORT_PHY_CTRL(unit, lport), &speed);
            if (rv) sal_printf("error 1:%d\n", rv);
            rv |= PHY_DUPLEX_GET(BMD_PORT_PHY_CTRL(unit, lport), &duplex);
            if (rv) sal_printf("error 2:%d\n", rv);
            rv |= PHY_AUTONEG_GET(BMD_PORT_PHY_CTRL(unit, lport), &an);
            if (an) {
                rv |= phy_pause_get(unit, lport, &tx_pause, &rx_pause);
                if (SOC_FAILURE(rv)) {
                    sal_printf("phy_pause_get lport %d error\n", lport);
                    return;
                }
            } else {
                tx_pause = rx_pause = TRUE;
            }
        }
        SOC_PORT_TX_PAUSE_STATUS(lport) = tx_pause;
        SOC_PORT_RX_PAUSE_STATUS(lport) = rx_pause;        
#if CFG_CONSOLE_ENABLED
        sal_printf("\nlport %d (P:%d), speed = %d, duplex = %d, tx_pause = %d, rx_pause = %d speed=%d, an=%d %s\n",
                lport, SOC_PORT_L2P_MAPPING(lport), speed, duplex, tx_pause, rx_pause, speed, an, BMD_PORT_PHY_CTRL(unit, (lport))->drv->drv_name);
#endif /* CFG_CONSOLE_ENABLED */

#ifdef CFG_QSGMII_SERDES_AN_DISABLED
        /* Program speed, duplex and pause first */
        if (lport >= PHY_FIRST_QSGMII_PORT) {
            _phy_qsgmii_notify_speed_duplex(unit, lport, speed, duplex);
        }
#endif /* CFG_QSGMII_SERDES_AN_DISABLED */

        /* Update LED status */
        val = READCSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)));
        val |= 0x01;
        WRITECSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)), val);

        MAC_SPEED_SET(hr2_sw_info.p_mac[lport], unit, lport, speed);
        MAC_DUPLEX_SET(hr2_sw_info.p_mac[lport], unit, lport, duplex);

        /* Interface? */

        MAC_PAUSE_SET(hr2_sw_info.p_mac[lport], unit, lport, tx_pause, rx_pause);

        if (hr2_sw_info.loopback[lport] == PORT_LOOPBACK_MAC) {
            MAC_LOOPBACK_SET(hr2_sw_info.p_mac[lport], unit, lport, TRUE);
        } else {
            MAC_LOOPBACK_SET(hr2_sw_info.p_mac[lport], unit, lport, FALSE);
        }

        soc_phyctrl_notify(BMD_PORT_PHY_CTRL(unit, lport), PhyEvent_Speed, speed);
        soc_phyctrl_notify(BMD_PORT_PHY_CTRL(unit, lport), PhyEvent_Duplex, duplex);

        MAC_ENABLE_SET(hr2_sw_info.p_mac[lport], unit, lport, TRUE);

        hr2_sw_info.link[lport] = PORT_LINK_UP;
#if defined(CFG_SWITCH_EEE_INCLUDED)
        {
            uint8 eee_state;
            int eee_support;
            uint32 remote_eee = 0x0;
            int rv;

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
                hr2_sw_info.link_up_time[lport] = sal_get_ticks();
                hr2_sw_info.need_process_for_eee_1s[lport] = TRUE;
            }
        }
#endif /* CFG_SWITCH_EEE_INCLUDED */
    } else {
        /* Port stays in link up state */
#if defined(CFG_SWITCH_EEE_INCLUDED)
        int eee_support;
        /* EEE one second delay for link up timer check */
        if ((hr2_sw_info.need_process_for_eee_1s[lport]) &&
            (SAL_TIME_EXPIRED(hr2_sw_info.link_up_time[lport], 1000))) {
            SAL_DEBUGF(("EEE : enable eee for port %d\n", lport));
            bmd_phy_eee_get(unit,lport, &eee_support);
            if (eee_support) {
                bcm5333x_port_eee_enable_set(unit, lport, TRUE, FALSE);    
            } else {
                bcm5333x_port_eee_enable_set(unit, lport, FALSE, FALSE);
            }

            hr2_sw_info.need_process_for_eee_1s[lport] = FALSE;
        }
#endif /* CFG_SWITCH_EEE_INCLUDED */
    }
}

void
bcm5333x_handle_link_down(uint8 unit, uint8 lport, int changed)
{
#if defined(CFG_SWITCH_EEE_INCLUDED)
    uint8 eee_state;
#endif /* CFG_SWITCH_EEE_INCLUDED */

#ifdef CFG_LOOPBACK_TEST_ENABLED
    if (1 == changed) {
#if CFG_CONSOLE_ENABLED
        if (board_linkdown_message) {
            sal_printf("lport %d goes down!\n", lport);
        }
#endif /* CFG_CONSOLE_ENABLED */
        hr2_sw_info.link[lport] = PORT_LINK_DOWN;
    }
#else
    uint32 val;

    if (1 == changed) {
        /* Update LED status */
        val = READCSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)));
        val &= 0xfc;
        WRITECSR(LED_PORT_STATUS_OFFSET(SOC_PORT_L2P_MAPPING(lport)), val);

        MAC_ENABLE_SET(hr2_sw_info.p_mac[lport], unit, lport, FALSE);
        hr2_sw_info.link[lport] = PORT_LINK_DOWN;

#if CFG_CONSOLE_ENABLED
        if (board_linkdown_message) {
            sal_printf("lport %d goes down!\n", lport);
        }
#endif /* CFG_CONSOLE_ENABLED */

#if defined(CFG_SWITCH_EEE_INCLUDED)
        bcm5333x_port_eee_enable_get(unit, lport, &eee_state);
        if (eee_state == TRUE) {
            /* Disable EEE in UMAC_EEE_CTRL register if EEE is enabled in S/W database */
            SAL_DEBUGF(("EEE : disable eee for port %d\n", lport));
            bcm5333x_port_eee_enable_set(unit, lport, FALSE, FALSE);
            hr2_sw_info.need_process_for_eee_1s[lport] = FALSE;
        }
#endif /* CFG_SWITCH_EEE_INCLUDED */
    } else {
        /* Port stays in link down state */
    }
#endif /* CFG_LOOPBACK_TEST_ENABLED */
}

/*
 *  Function : bcm5333x_linkscan_task
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
bcm5333x_linkscan_task(void *param)
{
    int unit = 0, rv, lport;

    uint32 flags;
    int link, andone;

    if (board_linkscan_disable) 
        return;
    
    SOC_LPORT_ITER(lport) {
        link = hr2_sw_info.link[lport];
        if (hr2_sw_info.loopback[lport] == PORT_LOOPBACK_MAC) {
            /* Force link up in mac loopback mode */
            link = PORT_LINK_UP;
        } else {

            rv = PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit, lport), &link, &andone);
            if (rv < 0) {
#if CFG_CONSOLE_ENABLED
                sal_printf("Failed to get link of port %d\n", (int)lport);
#endif /* CFG_CONSOLE_ENABLED */
                continue;
            }
        }

        if (link == PORT_LINK_UP) {
            /* Link up */
            flags = 0;
            if (hr2_sw_info.link[lport] == PORT_LINK_DOWN) {
                bcm5333x_handle_link_up(unit, lport, 1, &flags);
            } else {
                bcm5333x_handle_link_up(unit, lport, 0, &flags);
            }
        } else {
            if (hr2_sw_info.link[lport] == PORT_LINK_UP) {
                bcm5333x_handle_link_down(unit, lport, 1);
            } else {
                bcm5333x_handle_link_down(unit, lport, 0);
            }
        }
    }
}

/* please refer to MDK bcm56150_a0_xlport_reset() */
static void
soc_hr2_xlport_reset(uint8 unit)
{
    int i;
    uint32 val;
   /* XLPORT block id is 5 and 6 */
    for (i = 5; i < 7; i++) {
        /*
        * Reference clock selection: REFIN_ENf [Bit 2]
        */
        bcm5333x_reg_get(unit, i, R_XLPORT_XGXS0_CTRL_REG, &val);
        val |= (0x1 << 5);
        bcm5333x_reg_set(unit, i, R_XLPORT_XGXS0_CTRL_REG, val);
        sal_usleep(1100);

        /* Deassert power down [Bit 3]*/
        val &= ~(0x1 << 6);
        bcm5333x_reg_set(unit, i, R_XLPORT_XGXS0_CTRL_REG, val);
        sal_usleep(1100);

        /* Bring XGXS out of reset: RstB_HW[Bit 0] */
        val |= (0x1 << 2);
        bcm5333x_reg_set(unit, i, R_XLPORT_XGXS0_CTRL_REG, val);
        sal_usleep(1100);

        /* Bring reference clock out of reset */
        val |= (0x1 << 3);
        bcm5333x_reg_set(unit, i, R_XLPORT_XGXS0_CTRL_REG, val);
        sal_usleep(1100);

        /* Activate clocks */
        val |= (0x1 << 1);
        bcm5333x_reg_set(unit, i, R_XLPORT_XGXS0_CTRL_REG, val);
        sal_usleep(1100);

        /* Clear XLPORT counters */
        val = 0xf;
        bcm5333x_reg_set(unit, i, R_XLPORT_MIB_RESET, val);
        val = 0;
        bcm5333x_reg_set(unit, i, R_XLPORT_MIB_RESET, val);
    }
}

static void
soc_hr2_tsc_reset(void)
{
    int block;

    /* TSC reset */
    soc_hr2_xlport_reset(0);

    /* MAC reset */
    for (block = 5; block <= 6; block++) {
        uint32 val;
        bcm5333x_reg_get(0, block, R_XLPORT_MAC_CONTROL, &val);
        /* XMAC0_RESETf: [Bit 0] */
        val |= 0x1;
        bcm5333x_reg_set(0, block, R_XLPORT_MAC_CONTROL, val);
        sal_usleep(10);
        val &= ~0x1;
        bcm5333x_reg_set(0, block, R_XLPORT_MAC_CONTROL, val);

    }
}

void
soc_reset(void)
{
    uint32 val, to_usec;
    /* Use 156.25Mhz reference clock for LCPLL? */
#if CONFIG_HURRICANE2_EMULATION
    to_usec = 250000;
#else
    to_usec = 10000;
#endif /* CONFIG_HURRICANE2_EMULATION */
    /*
     * SBUS ring and block number:
     * Ring0: cmic -> ip(10) -> ep(11) -> cmic
     * Ring1: cmic -> xlport0(5) -> xlport1(6) -> cmic
     * Ring2: cmic -> gport0(2) -> gport1(3) -> gport2(4) -> cmic
     * Ring3: cmic -> mmu(12) -> cmic
     * Ring4: cmic -> otpc(13) -> top(16) -> cmic
     * Ring5: cmic -> ser(19) -> cmic
     * Ring 6,7 unused
     */
    WRITECSR(CMIC_SBUS_RING_MAP_0_7, 0x01122200); /* block 7  - 0 */
    WRITECSR(CMIC_SBUS_RING_MAP_8_15, 0x00430000); /* block 15 - 8 */
    WRITECSR(CMIC_SBUS_RING_MAP_16_23, 0x00005004); /* block 23 - 16 */
    WRITECSR(CMIC_SBUS_RING_MAP_24_31, 0x00000000); /* block 31 - 24 */

    sal_usleep(to_usec);

    /*
     * Bring port blocks out of reset:
     * TOP_QGPHY_RST_Lf [Bit 19~16]
     * TOP_QSGMII2X2_FIFO_RSTf [Bit14], TOP_QSGMII2X1_FIFO_RSTf [Bit13],
     * TOP_QSGMII2X0_FIFO_RSTf [Bit12],
     * TOP_QSGMII2X2_RST_Lf [Bit11], TOP_QSGMII2X1_RST_Lf [Bit10],
     * TOP_QSGMII2X0_RST_Lf [Bit9], TOP_GP2_RST_Lf [Bit 8],
     * TOP_GP1_RST_Lf [Bit 7], TOP_GP0_RST_Lf [Bit 6]
     * TOP_XLP0_RST_Lf [Bit 3], TOP_XLP1_RST_Lf [Bit 4],
     */
    bcm5333x_reg_get(0, R_TOP_SOFT_RESET_REG, &val);
    val |= 0xF7FD8;
    bcm5333x_reg_set(0, R_TOP_SOFT_RESET_REG, val);
    sal_usleep(to_usec);

    /*
     * Bring network sync out of reset
     * TOP_TS_RST_Lf [Bit 15], TOP_SPARE_RST_Lf [Bit 5]
     */
    bcm5333x_reg_get(0, R_TOP_SOFT_RESET_REG, &val);
    val |= 0x8020;
    bcm5333x_reg_set(0, R_TOP_SOFT_RESET_REG, val);

    /*
     * Bring network sync PLL out of reset
     * TOP_TS_PLL_RST_Lf [Bit 8] and then TOP_TS_PLL_POST_RST_Lf [Bit 9]
     */
    bcm5333x_reg_get(0, R_TOP_SOFT_RESET_REG_2, &val);
    val |= 0x100;
    bcm5333x_reg_set(0, R_TOP_SOFT_RESET_REG_2, val);
    val |= 0x200;
    bcm5333x_reg_set(0, R_TOP_SOFT_RESET_REG_2, val);

    sal_usleep(to_usec);

    WRITECSR(CMIC_SBUS_TIMEOUT, 0x7d0);

    /*
     * Bring IP, EP, and MMU blocks out of reset
     * TOP_IP_RST_L [Bit 0], TOP_EP_RST_Lf [Bit 1], TOP_MMU_RST_Lf[Bit2]
     */
    bcm5333x_reg_get(0, R_TOP_SOFT_RESET_REG, &val);
    val |= 0x7;
    bcm5333x_reg_set(0, R_TOP_SOFT_RESET_REG, val);
    sal_usleep(to_usec);

    /*
     * PLL programming modification to remove redundant power consumption
     *     m TOP_XG_PLL0_CTRL_3 CML_2ED_OUT_EN=0x0
     *     nm TOP_XG_PLL1_CTRL_3 CML_2ED_OUT_EN=0x0
     *     m TOP_BS_PLL0_CTRL_3 CML_2ED_OUT_EN=0x0
     *     m TOP_BS_PLL1_CTRL_3 CML_2ED_OUT_EN=0x0
     *     m TOP_MISC_CONTROL_1 CMIC_TO_XG_PLL0_SW_OVWR=0x1
     *     m TOP_MISC_CONTROL_1 CMIC_TO_XG_PLL1_SW_OVWR=0x1
     *     m TOP_MISC_CONTROL_1 CMIC_TO_BS_PLL0_SW_OVWR=0x1
     *     m TOP_MISC_CONTROL_1 CMIC_TO_BS_PLL1_SW_OVWR=0x1
     */

    switch (hr2_sw_info.devid) {
        case BCM53333_DEVICE_ID:
        case BCM53334_DEVICE_ID:
        case BCM53342_DEVICE_ID:
            bcm5333x_reg_get(0, R_TOP_XG_PLL0_CTRL_3, &val);
            val &= ~0x10000000;
            bcm5333x_reg_set(0, R_TOP_XG_PLL0_CTRL_3, val);
            bcm5333x_reg_get(0, R_TOP_MISC_CONTROL_1, &val);
            val |= 0x0001e000;
            bcm5333x_reg_set(0, R_TOP_MISC_CONTROL_1, val);
            break;
        case BCM53343_DEVICE_ID:
        case BCM53344_DEVICE_ID:
        case BCM53346_DEVICE_ID:
        case BCM53394_DEVICE_ID:
        case BCM53393_DEVICE_ID:
            bcm5333x_reg_get(0, R_TOP_MISC_CONTROL_1, &val);
            val |= 0x0001c000;
            bcm5333x_reg_set(0, R_TOP_MISC_CONTROL_1, val);
            soc_hr2_tsc_reset();
            break;
        default :
            break;
    }

    bcm5333x_reg_get(0, R_TOP_XG_PLL1_CTRL_3, &val);
    val &= ~0x10000000;
    bcm5333x_reg_set(0, R_TOP_XG_PLL1_CTRL_3, val);

    bcm5333x_reg_get(0, R_TOP_BS_PLL0_CTRL_3, &val);
    val &= ~0x10000000;
    bcm5333x_reg_set(0, R_TOP_BS_PLL0_CTRL_3, val);

    bcm5333x_reg_get(0, R_TOP_BS_PLL1_CTRL_3, &val);
    val &= ~0x10000000;
    bcm5333x_reg_set(0, R_TOP_BS_PLL1_CTRL_3, val);


    sal_usleep(to_usec);
}

soc_chip_type_t
bcm5333x_chip_type(void)
{
    return SOC_TYPE_SWITCH_XGS;
}

uint8
bcm5333x_port_count(uint8 unit)
{
    if (unit > 0) {
        return 0;
    }
    return hr2_sw_info.port_count;
}

sys_error_t
bcm5333x_link_status(uint8 unit, uint8 port, BOOL *link)
{
    if (link == NULL || unit > 0 || port > BCM5333X_LPORT_MAX || port < BCM5333X_LPORT_MIN) {
        return SYS_ERR_PARAMETER;
    }

    *link = hr2_sw_info.link[port];

    return SYS_OK;
}

static int
bcm5333x_sw_op(uint8 unit,
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
    if (i == 100) {
        sal_printf("S-CHAN %d:0x%x, timeout!\n", block_id, addr);
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

    return 0;
}

sys_error_t
bcm5333x_phy_reg_get(uint8 unit, uint8 lport,
                           uint16 reg_addr, uint16 *p_value)
{
    return phy_reg_read(lport, reg_addr, p_value);
}

sys_error_t
bcm5333x_phy_reg_set(uint8 unit, uint8 lport,
                           uint16 reg_addr, uint16 value)
{
    return phy_reg_write(lport, reg_addr, value);
}

sys_error_t
bcm5333x_reg_get(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *val)
{
    return bcm5333x_sw_op(unit, SC_OP_RD_REG_CMD, block_id, addr, val, 1);
}

sys_error_t
bcm5333x_reg_set(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 val)
{
    return bcm5333x_sw_op(unit, SC_OP_WR_REG_CMD, block_id, addr, &val, 1);
}

sys_error_t
bcm5333x_reg64_get(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *val,
                 int len)
{
    return bcm5333x_sw_op(unit, SC_OP_RD_REG_CMD, block_id, addr, val, len);
}

sys_error_t
bcm5333x_reg64_set(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *buf,
                 int len)
{
    return bcm5333x_sw_op(unit, SC_OP_WR_REG_CMD, block_id, addr, buf, len);
}

sys_error_t
bcm5333x_mem_get(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *buf,
                 int len)
{
    return bcm5333x_sw_op(unit, SC_OP_RD_MEM_CMD, block_id, addr, buf, len);
}

sys_error_t
bcm5333x_mem_set(uint8 unit,
                 uint8 block_id,
                 uint32 addr,
                 uint32 *buf,
                 int len)
{
    return bcm5333x_sw_op(unit, SC_OP_WR_MEM_CMD, block_id, addr, buf, len);
}

sys_error_t
bcm5333x_chip_revision(uint8 unit, uint16 *dev, uint16 *rev)
{
    uint32 val;

    if (unit > 0) {
        return -1;
    }

    val = READCSR(CMIC_DEV_REV_ID);
    hr2_sw_info.devid = val & 0xFFFF;
    hr2_sw_info.revid = val >> 16;

    return 0;
}


static void
bcm5333x_load_led_program(void)
{
    uint32 val;
#ifdef CFG_LED_MICROCODE_INCLUDED
    int i, offset, led_code_size;
    uint32 addr;
    uint8 *led_program;
    int byte_count = 0;
    uint8 led_option = 1;    
    uint8 led_program_3[256];
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED    
    sys_error_t sal_config_rv = SYS_OK;
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
    /* Port 2~9 are remapped */
    uint32 port_remap[9] = { 0x00187209, 0x00083105, 0x0034c2ca, 0x004503ce,
                             0x005544d2, 0x006585d6, 0x0069b71d, 0x0079f821,
                             0x00000022 };

    for (i = 0, addr = CMIC_LEDUP0_PORT_ORDER_REMAP_0_3; i < 9; i++, addr += 4) {
        WRITECSR(addr, port_remap[i]);
    }
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED 
    sal_config_rv = sal_config_uint8_get(SAL_CONFIG_LED_OPTION, &led_option);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Overwrite serial led option with value %d.\n", led_option);
    }

    byte_count = sal_config_bytes_get(SAL_CONFIG_LED_PROGRAM, led_program_3, 256);
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
    if (led_option == 2) {
        led_program = led_program_2;
        led_code_size = sizeof(led_program_2);
    } else if ((led_option == 3) && byte_count) {
        sal_printf("Vendor Config : Load customer LED ucdoe with length %d.\n", byte_count);
        led_program = led_program_3;
        led_code_size = sizeof(led_program_3);
    } else {
        led_program = led_program_1;
        led_code_size = sizeof(led_program_1);
    }

#define LED_RAM_SIZE     0x100

    for (offset = 0; offset < LED_RAM_SIZE; offset++) {
        WRITECSR(CMIC_LEDUP_PROGRAM_RAM_D(offset),
                      (offset >= led_code_size) ? 0 : *(led_program + offset));

        WRITECSR(CMIC_LEDUP_DATA_RAM_D(offset), 0);
    }

    /* Swizzle the gp_to_cmic_link_status from [7:0] to [0:7] for GPORT0 */
    bcm5333x_reg_set(0, 2, R_GPORT_LINK_STATUS_TO_CMIC, 0x1);

    bcm5333x_reg_set(0, R_XLPORT_LED_CHAIN_CONFIG, 0x6);
#endif /* CFG_LED_MICROCODE_INCLUDED */
    /* GPHY0 to LED mirroring */
    bcm5333x_reg_get(0, R_TOP_MISC_CONTROL_2, &val);
    val |= 0x2;
    bcm5333x_reg_set(0, R_TOP_MISC_CONTROL_2, val);

#ifdef CFG_LED_MICROCODE_INCLUDED
    #if (CFG_LED_MICROCODE_INCLUDED == 2)
    /* led_current_sel=0x3, led_faultdet_pdwn_l=0x3f */
    /* led_ser2par_sel=1 */
    bcm5333x_reg_set(0, R_TOP_PARALLEL_LED_CTRL, 0x3FB00);
    #endif
    /* enable LED processor */
    WRITECSR(CMIC_LEDUP0_CTRL, 0x6b);
#endif /* CFG_LED_MICROCODE_INCLUDED */
}

sys_error_t
bcm5333x_l2_op(uint8 unit,
               l2x_entry_t *entry,
               uint8 op_code
               )

{
    uint32 l2_entry[4];
    uint32 msg_hdr[2] = {0x90a00800, 0x1c000000};
    uint32 ctrl;
    int i;

    l2_entry[0] = ((entry->vlan_id << 2) |
                   (entry->mac_addr[5] << 14) |
                   (entry->mac_addr[4] << 22) |
                   (entry->mac_addr[3] << 30));

    l2_entry[1] = (((entry->mac_addr[3] >> 2) & 0x0000003F) |
                   (entry->mac_addr[2] << 6) |
                   (entry->mac_addr[1] << 14) |
                   (entry->mac_addr[0] << 22) |
                   (entry->port << 30));

    l2_entry[2] = ((entry->port >> 2) & 0x0000003F);
    /* set static and valid bit */
    l2_entry[3] = (1 << 3) | (1 << 1);

    if (op_code == SC_OP_L2_DEL_CMD) {
         msg_hdr[0] = 0x98a00800;
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
enable_jumbo_frame(void)
{
    int lport;
    uint32 entry[2];

    SOC_LPORT_ITER(lport) {
        if (IS_XL_PORT(lport)) {
            entry[0] = JUMBO_FRM_SIZE;
            entry[1] = 0x0;
            bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport),
                       R_XLMAC_RX_MAX_SIZE(SOC_PORT_BLOCK_INDEX(lport)),
                       entry, 2);
        } else {
            bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport),
                    R_FRM_LENGTH(SOC_PORT_BLOCK_INDEX(lport)), JUMBO_FRM_SIZE);
        }
    }
}

static void
soc_pipe_mem_clear(void)
{
    uint32 val;
    /*
     * Reset the IPIPE and EPIPE block
     */
    bcm5333x_reg_set(0, R_ING_HW_RESET_CONTROL_1, 0x0);
    /*
     * Set COUNT[Bit 15-0] to # entries in largest IPIPE table, L2_ENTRYm 0x4000
     * RESET_ALL [Bit 16] = 1, VALID [Bit 17] = 1
     */
    bcm5333x_reg_set(0, R_ING_HW_RESET_CONTROL_2, 0x34000);

    bcm5333x_reg_set(0, R_EGR_HW_RESET_CONTROL_0, 0x0);
    /*
     * Set COUNT[Bit 15-0] to # entries in largest EPIPE table, EGR_VLAN 0x1000
     * RESET_ALL [Bit 16] = 1, VALID [Bit 17] = 1
     */
    bcm5333x_reg_set(0, R_EGR_HW_RESET_CONTROL_1, 0x31000);

    /* Wait for IPIPE memory initialization done. */
    do {
        /* Polling DONE[Bit 18] */
        bcm5333x_reg_get(0, R_ING_HW_RESET_CONTROL_2, &val);
        if (val & (0x1 << 18)) {
            break;
        }
        
        /*  soc_cm_debug(DK_WARN, "unit %d : ING_HW_RESET timeout\n", unit); */
    } while (1);

    /* Wait for EPIPE memory initialization done. */
    do {
        /* Polling DONE[Bit 18] */
        bcm5333x_reg_get(0, R_EGR_HW_RESET_CONTROL_1, &val);
        if (val & (0x1 << 18)) {
            break;
        }

        
        /*  soc_cm_debug(DK_WARN, "unit %d : EGR_HW_RESET timeout\n", unit); */
    } while (1);

    bcm5333x_reg_set(0, R_ING_HW_RESET_CONTROL_2, 0x0);
    bcm5333x_reg_set(0, R_EGR_HW_RESET_CONTROL_1, 0x0);

    
}

static void
soc_port_block_info_get(uint8 unit, uint8 pport, int *block_type, int *block_idx, int *port_idx)
{
    *block_type = PORT_BLOCK_TYPE_XLPORT;
    if ((pport >= PHY_XLPORT1_BASE) && (pport <= BCM5333X_PORT_MAX)) {
        *block_idx = XLPORT1_BLOCK_ID;
        *port_idx = (pport - PHY_XLPORT1_BASE) & 0x7;
    } else if (pport >= PHY_XLPORT0_BASE) {
        *block_idx = XLPORT0_BLOCK_ID;
        *port_idx = (pport - PHY_XLPORT0_BASE) & 0x7;
    } else if (pport >= PHY_GXPORT2_BASE) {
        *block_idx = GXPORT2_BLOCK_ID;
        *port_idx = (pport - PHY_GXPORT2_BASE) & 0x7;
        *block_type = PORT_BLOCK_TYPE_GXPORT;
    } else if (pport >= PHY_GXPORT1_BASE) {
        *block_idx = GXPORT1_BLOCK_ID;
        *port_idx = (pport - PHY_GXPORT1_BASE) & 0x7;
        *block_type = PORT_BLOCK_TYPE_GXPORT;
    } else {
        *block_idx = GXPORT0_BLOCK_ID;
        if (SOC_IS_DEERHOUND(unit)) {
            *port_idx = (pport - PHY_GXPORT0_BASE) & 0x7;
        } else {
            *port_idx = 7 - ((pport - PHY_GXPORT0_BASE) & 0x7);            
        }
        *block_type = PORT_BLOCK_TYPE_GXPORT;
    }
}


static void
soc_init_port_mapping(void)
{
    int i, port_count;
    const int *p2l_mapping = 0;
    const int *speed_max = 0;
    uint32 val;
    
    switch (hr2_sw_info.devid) {
        case BCM53333_DEVICE_ID:
            p2l_mapping = p2l_mapping_16_0_0;
            speed_max = port_speed_max_16x1g;
            break;
        case BCM53334_DEVICE_ID:
            p2l_mapping = p2l_mapping_24_0_0;
            speed_max = port_speed_max_24x1g;
            break;
        case BCM53393_DEVICE_ID:
            /* 14P 1G,  (no PHY) */
            p2l_mapping = p2l_mapping_6_4_4;
            speed_max = port_speed_max_6x1g_4x1g_4x1g;
            break;
        case BCM53394_DEVICE_ID:
            if (config_id == 1) {
                /* Option 1: 10P 1G + 4x1/2.5/5/10G  (no PHY) */
                p2l_mapping = p2l_mapping_6_4_4;
                speed_max = port_speed_max_6x1g_4x10g_4x1g;
            } else if (config_id == 2) {
                /* Option 2: 10P 1G + 1P XAUI (no Phy) */
                p2l_mapping = p2l_mapping_6_1_4;
                speed_max = port_speed_max_6x1g_1x10g_4x1g;
            } else if (config_id == 3) {
                /* Option 3: 6P 1G + 3x1/10G + 1P XAUI (no Phy) */
                p2l_mapping = p2l_mapping_6_1_3;
                speed_max = port_speed_max_6x1g_1x10g_3x10g;
            }
            break;
        case BCM53342_DEVICE_ID:
            /* 8P  1G (PHY) */
            p2l_mapping = p2l_mapping_8_0_0;
            speed_max = port_speed_max_8x1g;
            break;
        case BCM53343_DEVICE_ID:
            /* 16P  1G (PHY) + 4P 1G */
            p2l_mapping = p2l_mapping_16_4_0;
            speed_max = port_speed_max_16x1g_4x1g;
            break;
        case BCM53344_DEVICE_ID:
            if (config_id == 1) {
                /* 1 = Option 1: 24P 1G + 4x1G (PHY) */
                p2l_mapping = p2l_mapping_24_4_0_wh;
                speed_max = port_speed_max_24x1g_4x1g;
            } else if (config_id == 2) {
                /* 2 = Option 2: 24P 1G + 2P 1G + 2P 13G (PHY) */
                p2l_mapping = p2l_mapping_24_2_2;
                speed_max = port_speed_max_24x1g_2x1g_2x13g;
            } else if (config_id == 3) {
                /* 3 = Otion 2A: 24P 1G + 2P 13G + 2P 1G (PHY) */
                p2l_mapping = p2l_mapping_24_2_2;
                speed_max = port_speed_max_24x1g_2x13g_2x1g;
            }
            break;
        case BCM53346_DEVICE_ID:
            if (config_id == 1) {
                /* 1 = Option 1: 24P 1G + 4P 1G/10G(PHY, TSC0) */
                p2l_mapping = p2l_mapping_24_4_0;
                speed_max = port_speed_max_24x1g_4x10g;
            } else if (config_id == 2) {
                /* 2 = Option 1A: 24P 1G + 2P 1G/10G + 2P 1G/10G (PHY) */
                p2l_mapping = p2l_mapping_24_2_2;
                speed_max = port_speed_max_24x1g_2x10g_2x10g;
            } else if (config_id == 3) {
                /* 3 = Option 1B: 24P 1G + 4P 1G/10G  (PHY, TSC1) */
                p2l_mapping = p2l_mapping_24_0_4;
                speed_max = port_speed_max_24x1g_4x10g_tsc1;
            } else if (config_id == 4) {
                /* 4 = Option 2: 24P 1G + 2P 1G/10G + 2P 13G (PHY) */
                p2l_mapping = p2l_mapping_24_2_2;
                    speed_max = port_speed_max_24x1g_2x10g_2x13g;
            } else if (config_id == 5) {
                /* 5 = Option 2A: 24P 1G + 2P 13G + 2P 1G/10G (PHY) */
                p2l_mapping = p2l_mapping_24_2_2;
                speed_max = port_speed_max_24x1g_2x13g_2x10g;
            } else if (config_id == 6) {
                /* 6 = Option 3: 24P 1G + 3P 1G/10G + 1P XAUI (PHY) */
                p2l_mapping = p2l_mapping_24_3_1;
                speed_max = port_speed_max_24x1g_3x10g_1x10g;
            } else if (config_id == 7) {
                /* 7 = Option 4: 24P 1G + 1P XAUI + 1P XAUI (PHY) */
                p2l_mapping = p2l_mapping_24_1_1;
                speed_max = port_speed_max_24x1g_1x10g_1x10g;
            }
            break;
        default :
            break;
    }
    for (i = 0 ; i <= BCM5333X_LPORT_MAX ; i++) {
        SOC_PORT_L2P_MAPPING(i) = -1;
        SOC_PORT_SPEED_MAX(i) = -1;
    }

    for (i = 0; i <= BCM5333X_PORT_MAX ; i++) {
        SOC_PORT_P2L_MAPPING(i) = p2l_mapping[i];
        if (p2l_mapping[i] != -1) {
            SOC_PORT_L2P_MAPPING(p2l_mapping[i]) = i;
            SOC_PORT_SPEED_MAX(p2l_mapping[i]) = speed_max[i];
        }
    }

    /* Ingress physical to logical port mapping */
    for (i = 0; i <= BCM5333X_PORT_MAX; i++) {
        val = (p2l_mapping[i] == -1) ? 0x1F: p2l_mapping[i];
        bcm5333x_mem_set(0, M_ING_PHYSICAL_TO_LOGICAL_PORT_NUMBER_MAPPING_TABLE(i), &val, 1);
    }

   /* Egress logical to physical port mapping, needs a way for maximum logical port? */
    for (i = 0; i <= BCM5333X_LPORT_MAX; i++) {
        val = (hr2_sw_info.port_l2p_mapping[i] == -1) ? 0x3F : hr2_sw_info.port_l2p_mapping[i];
        bcm5333x_reg_set(0, R_EGR_LOGICAL_TO_PHYSICAL_PORT_NUMBER_MAPPING(i), val);
        /* MMU logical to physical port mapping
         * (Here, Same as Egress logical to physical port mapping)
         */
        if (val != 0x3F) {
            bcm5333x_reg_set(0, R_LOG_TO_PHY_PORT_MAPPING(i), val);
        }
    }

    port_count = 0;
    SOC_LPORT_ITER(i) {
        soc_port_block_info_get(0, SOC_PORT_L2P_MAPPING(i),
                                &SOC_PORT_BLOCK_TYPE(i),
                                &SOC_PORT_BLOCK(i), &SOC_PORT_BLOCK_INDEX(i));
        BCM5333x_ALL_PORTS_MASK |= (0x1 << i);
        port_count++;
    }
    SOC_PORT_COUNT(unit) = port_count;
}


#if CFG_PCIE_SERDES_POWERDOWN_ENABLED
static int
ccb_mii_write(int phy_addr, int reg_off, uint16 data)
{
    int timeout;
    uint32 val;

    for (timeout = 1000; timeout > 0; timeout -= 10) {
        val = READCSR(CCB_MII_MGMT_CTRL);
        if (!(val & MII_MGMT_CTRL_BUSY)) {
            break;
        }
        sal_usleep(10);
    }

    val = (1 << MII_CMD_DATA_SB_SHIFT) |
          (1 << MII_CMD_DATA_OP_SHIFT) |
          ((phy_addr) << MII_CMD_DATA_PA_SHIFT) |
          ((reg_off) << MII_CMD_DATA_RA_SHIFT) |
          (2 << MII_CMD_DATA_TA_SHIFT) |
          ((data) << MII_CMD_DATA_DATA_SHIFT);

    WRITECSR(CCB_MII_MGMT_CMD_DATA, val);

    for (timeout = 1000; timeout > 0; timeout -= 10) {
        val = READCSR(CCB_MII_MGMT_CTRL);
        if (!(val & MII_MGMT_CTRL_BUSY)) {
            break;
        }
        sal_usleep(10);
    }

    if (timeout < 0) {
#if CFG_CONSOLE_ENABLED
        sal_printf("ccb_mii_write timeout!\n");
#endif /* CFG_CONSOLE_ENABLED */
        return -1;
    }

    return 0;
}
#endif /* CFG_PCIE_SERDES_POWERDOWN_ENABLED */

static int
soc_port_num_lanes(int unit, int lport)
{
    int lanes = 1;

    switch (hr2_sw_info.devid) {
        case BCM53394_DEVICE_ID:
            if ((config_id == 2) || (config_id == 3)) {
                if (lport == 26) {
                    lanes = 4;
                }
            }
            break;
        case BCM53344_DEVICE_ID:
            if (config_id == 2){
                /* 24P 1G + 2P 1G + 2P 13G */
                if (lport >= 28) {
                    lanes = 2;
                }
            } else if (config_id == 3){
                /* 24P 1G + 2P 13G + 2P 1G */
                if (lport <= 27) {
                    lanes = 2;
                }
            }
            break;
        case BCM53346_DEVICE_ID:
            if (config_id == 4) {
                /* 24P 1G + 2P 1G/10G + 2P 13G */
                if (lport >= 28) {
                    lanes = 2;
                }
            } else if (config_id == 5) {
                /* 24P 1G + 2P 13G + 2P 1G/10G */
                if (lport <= 27) {
                    lanes = 2;
                }
            } else if ((config_id == 7) ||
                ((config_id == 6) && (lport !=26))) {
                lanes = 4;
            } 
            break;
        default :
            break;
    }
    return lanes;
}
static void
soc_misc_init(void)
{
#define NUM_XLPORT 4
    int i, bindex, lport, lanes;
    uint32 rst_val, val, mode, entry[2];

#if CFG_PCIE_SERDES_POWERDOWN_ENABLED
    /* Power down PCIE serdes(PHY addr 2) through ChipcommonB MDIO chain */
    val = READCSR(CCB_MII_MGMT_CTRL);
    /* Internal MDIO */
    val &= ~MII_MGMT_CTRL_EXT;
    /* Enable preamble sequence */
    val |= MII_MGMT_CTRL_PRE;

    /* Configure MDIO Clock, ChipCommonB clock = 12.5 MHz by default */
    val &= ~(MII_MGMT_CTRL_MDCDIV_MASK);
    val |= (12 & MII_MGMT_CTRL_MDCDIV_MASK);

    WRITECSR(CCB_MII_MGMT_CTRL, val);

    ccb_mii_write(0x2, 0x1F, 0x8010);
    ccb_mii_write(0x2, 0x16, 0x7F7F);

    ccb_mii_write(0x2, 0x1F, 0x8010);
    ccb_mii_write(0x2, 0x17, 0x7F7F);

    ccb_mii_write(0x2, 0x1F, 0x8010);
    ccb_mii_write(0x2, 0x18, 0x7F7F);

    ccb_mii_write(0x2, 0x1F, 0x8010);
    ccb_mii_write(0x2, 0x19, 0x7F7F);
#endif /* CFG_PCIE_SERDES_POWERDOWN_ENABLED */

    if (SOC_IS_FOXHOUND(0)) {
        /* Enable clock gating for XLPORT core for power saving. */
        for (i = 5; i < 7; i++) {
            /* XLPORT block number: 5 and 6 */
            bcm5333x_reg_set(0, i, R_XLPORT_POWER_SAVE, 0x1);
        }
    }

    soc_pipe_mem_clear();

    soc_init_port_mapping();

    /* Bump up core clock to 135 MHz if any TSC port is > 1G */
    /* All TSC ports */
    SOC_XLPORT_ITER(lport) {
        if (SOC_PORT_SPEED_MAX(lport) > 1) {
            bcm5333x_reg_get(0, R_TOP_MISC_CONTROL_1, &val);
            val |= 0x00000800;
            bcm5333x_reg_set(0, R_TOP_MISC_CONTROL_1, val);
            bcm5333x_reg_get(0, R_TOP_CORE_PLL_CTRL4, &val);
            val &= 0xfffff807;
            val |= 25 << 3;
            bcm5333x_reg_set(0, R_TOP_CORE_PLL_CTRL4, val);
            bcm5333x_reg_get(0, R_TOP_CORE_PLL_CTRL2, &val);
            val |= 0x000c0000;
            bcm5333x_reg_set(0, R_TOP_CORE_PLL_CTRL2, val);
            break;
        }
    }

    /* Turn on ingress/egress/mmu parity? */

    /* Metering Clock [Bit 5] ? */
    bcm5333x_reg_get(0, R_MISCCONFIG, &val);
    val |= 0x20;
    bcm5333x_reg_set(0, R_MISCCONFIG, val);

    /* Enable dual hash on L2 and L3 tables? */
    /* HASH_SELECT = FB_HASH_CRC32_LOWER, INSERT_LEAST_FULL_HALF = 1 */
    bcm5333x_reg_set(0, R_L2_AUX_HASH_CONTROL, 0x15);
    bcm5333x_reg_set(0, R_L3_AUX_HASH_CONTROL, 0x15);

    /* Egress Enable */
    val = 0x1;
    for (i = 0; i <= BCM5333X_PORT_MAX; i++) {
        if (i == 1) {
            continue;
        }
        bcm5333x_mem_set(0, M_EGR_ENABLE(i), &val, 1);
    }

    /* GMAC init */
    for (i = 2; i < 5; i++) {
        /* Clear counter and enable gport */
        bcm5333x_reg_set(0, i, R_GPORT_CONFIG, 0x3);
    }

    for (i = 2; i < 5; i++) {
        /* Enable gport */
        bcm5333x_reg_set(0, i, R_GPORT_CONFIG, 0x1);
    }

    /*
     * Set reference clock (based on 200MHz core clock)
     * to be 200MHz * (1/40) = 5MHz
     */
    WRITECSR(CMIC_RATE_ADJUST_EXT_MDIO, 0x10028);
    /* Match the Internal MDC freq with above for External MDC */
    WRITECSR(CMIC_RATE_ADJUST_INT_MDIO, 0x10028);

    /* GMAC init */

    SOC_LPORT_ITER(lport) {
        if (IS_XL_PORT(lport) && (SOC_PORT_BLOCK_INDEX(lport) == 0)) {
            i = SOC_PORT_L2P_MAPPING(lport);
            if (SOC_PORT_P2L_MAPPING(i) == -1) {
                continue;
            }
            rst_val = 0;
            for (bindex = 0; bindex < NUM_XLPORT; bindex++) {
                if (SOC_PORT_P2L_MAPPING(i + bindex) != -1) {
                    rst_val |= (0x1 << bindex);
                }
            }
            /* Assert XLPORT soft reset */
            bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_XLPORT_SOFT_RESET, rst_val);

            mode = SOC_HU2_PORT_MODE_QUAD;
            lanes = soc_port_num_lanes(0, lport);
            if (lanes == 2) {
                mode = SOC_HU2_PORT_MODE_DUAL;
            } else if (lanes == 4) {
                mode = SOC_HU2_PORT_MODE_SINGLE;
            }
            bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_XLPORT_MODE_REG, &val);
            val &= 0xffffffc0;
            val = (mode << 3) | mode;
            bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_XLPORT_MODE_REG, val);
            bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_XLPORT_CNTMAXSIZE, 0xde8);

            /* De-assert XLPORT soft reset */
            bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_XLPORT_SOFT_RESET, 0);

            /* Enable XLPORT */
            bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_XLPORT_ENABLE_REG, rst_val);
        }
    }

    bcm5333x_reg64_get(0, R_ING_CONFIG_64, entry, 2);
    entry[0] |= 0x2080300c;
    entry[1] |= 0xc0;
    bcm5333x_reg64_set(0, R_ING_CONFIG_64, entry, 2);

    bcm5333x_reg_get(0, R_EGR_CONFIG_1, &val);
    val |= 0x1;
    bcm5333x_reg_set(0, R_EGR_CONFIG_1, val);


    /* The HW defaults for EGR_VLAN_CONTROL_1.VT_MISS_UNTAG == 1, which
     * causes the outer tag to be removed from packets that don't have
     * a hit in the egress vlan tranlation table. Set to 0 to disable this.
     */
    for (i = 0; i <= BCM5333X_LPORT_MAX; i++) {
        if (i == 1) {
            continue;
        }
        bcm5333x_reg_set(0, R_EGR_VLAN_CONTROL_1(i), 0x0);
    }

    /* Enable SRC_REMOVAL_EN[Bit 0] and LAG_RES_EN[Bit2] */
    bcm5333x_reg_set(0, R_SW2_FP_DST_ACTION_CONTROL, 0x5);
    /* Enable MMU parity error interrupt */

    /* Directed Mirroring ON by default */

    if (SOC_IS_FOXHOUND(unit)){
        /* Clock gate the xlport buffers in EDATABUF: XP0~7[Bit10~3] = 1 */
        bcm5333x_reg_set(0, R_EGR_PORT_BUFFER_CLK_SHUTDOWN, 0x7f8);
    }
}

static void
soc_mmu_init(void)
{
    int i, j;
    uint32 addr, val;
    int tdm_size;
    const uint32 *arr = NULL;

    /* TDM initialization */
    switch (hr2_sw_info.devid) {
        case BCM53333_DEVICE_ID:
        case BCM53334_DEVICE_ID:
        case BCM53342_DEVICE_ID:
            tdm_size = 48;
            arr = hu2_tdm_powersave;
           break;
        case BCM53393_DEVICE_ID:
            tdm_size = 60;
            arr = hu2_tdm_embedded;
            break;
        case BCM53394_DEVICE_ID:
            tdm_size = 60;
            if (config_id == 3) {
                arr = hu2_tdm_embedded_plus;
            } else {
                arr = hu2_tdm_embedded;
            }
            break;
        case BCM53343_DEVICE_ID:
            tdm_size = 48;
            arr = hu2_tdm_powersave_plus;
            break;
        case BCM53344_DEVICE_ID:
            if (config_id == 1) {
                /* 0 = Option 1: 24P 1G + 4P 1G (PHY) */
                tdm_size = 48;
                arr = hu2_tdm_powersave_plus;
            } else if ((config_id == 2) ||
                       (config_id == 3)) {
                /* 
                 * 1 = Option 2: 24P 1G + 2P 1G + 2P 13G (PHY)
                 * 2 = Option 2A 24P 1G + 2P 13G + 2P 1G (PHY)
                 */
                tdm_size = 75;
                arr = hu2_tdm_cascade;
            }
            break;
        case BCM53346_DEVICE_ID:
            if (config_id == 1) {
                tdm_size = 75;
                arr = hu2_tdm_non_cascade;
            } else if (config_id == 3) {
                tdm_size = 75;
                arr = hu2_tdm_non_cascade_1b;
            } else if ((config_id == 2) ||
                       (config_id == 4) ||
                       (config_id == 5)) {
                /* Option 1A, 2, 2A */
                tdm_size = 75;
                arr = hu2_tdm_cascade;
            } else if ((config_id == 6) ||
                       (config_id == 7)) {
                /* Option 3 and 4 */
                tdm_size = 75;
                arr = hu2_tdm_xaui;
            }
            break;
        default :
            break;
    }

    /* DISABLE [Bit 0] = 1, TDM_WRAP_PTR[Bit 7:1] = TDM_SIZE-1 */
    val = ((tdm_size-1) << 1 | 1);
    bcm5333x_reg_set(0, R_IARB_TDM_CONTROL, val);

    for (i = 0; i < tdm_size; i++) {
        bcm5333x_mem_set(0, M_IARB_TDM_TABLE(i), (uint32 *)&arr[i], 1);
        /* TDM programmed in MMU is in Logical port domain */
        if (hr2_sw_info.port_p2l_mapping[arr[i]] != -1) {
            val = (arr[i] != 63) ? hr2_sw_info.port_p2l_mapping[arr[i]] : 63;
        } else {
            val = 0x3f;
        }

        if (i == (tdm_size - 1)) {
            /* WRAP_EN [Bit 6] = 1 */
            val |= 0x40;
        }
        bcm5333x_mem_set(0, M_MMU_ARB_TDM_TABLE(i), &val, 1);
    }

    /* DISABLE [Bit 0] = 0, TDM_WRAP_PTR[Bit 7:1] = tdm_size-1 */
    val = (tdm_size - 1) << 1;
    bcm5333x_reg_set(0, R_IARB_TDM_CONTROL, val);

    for (i = 0; i < COS_QUEUE_NUM; i++) {
        for (j = 0; j <= BCM5333X_LPORT_MAX; j++) {
            if (j == 1) {
                /* Skip port 1 */
                continue;
            }
            /*
             * The HOLCOSPKTSETLIMITr register controls BOTH the XQ
             * size per cosq AND the HOL set limit for that cosq.
             */
            /* HOLCOSPKTSETLIMIT = 1496, HOLCOSPKTRESETLIMIT = 1492 */
            addr = R_HOLCOSPKTSETLIMIT(i, j);
            bcm5333x_reg_set(0, 12, addr, MMU_R_HOLCOSPKTSETLIMIT);
            addr = R_HOLCOSPKTRESETLIMIT(i, j);
            bcm5333x_reg_set(0, 12, addr, MMU_R_HOLCOSPKTRESETLIMIT);
            /* RESETLIMITSEL = 16, LWMCOSCELLSETLIMIT.RESETLIMITSEL = 16 */
            addr = R_LWMCOSCELLSETLIMIT(i, j);
            bcm5333x_reg_set(0, 12, addr, MMU_R_LWMCOSCELLSETLIMIT);

            addr = R_HOLCOSCELLMAXLIMIT(i, j);
            
            /* 
             * Based on recommended priority to queue mapping in 802.1p as
             * below, it uses normal queue to forward general data packet
             * (includes jumbo packet). So we let normal queue has higher HOL
             * threshold to ensure data packet with larger size won't hit HOL
             * drop condition.
            
             * 802.1p priority              COS queue
             * 1 and 2                        Low (0)
             * 0 and 3                        Normal (1)
             * 4 and 5                        Medium (2)
             * 6 and 7                        High (3)
             */
            if (i == 1) {
                /* COS1: CELLMAXLIMIT = 2024, RESUMELIMIT = 2008 */
                bcm5333x_reg_set(0, 12, addr, MMU_R_HOLCOSCELLMAXLIMIT_COS1);
            } else {
                /* other COS: CELLMAXLIMIT = 64, RESUMELIMIT = 48 */
                bcm5333x_reg_set(0, 12, addr, MMU_R_HOLCOSCELLMAXLIMIT_COS0);
            }

            /* HOLCOSMINXQCNT = 8 */
            addr = R_HOLCOSMINXQCNT(i, j);
            bcm5333x_reg_set(0, 12, addr, MMU_R_HOLCOSMINXQCNT);
        }
    }

    for (i = 0; i <= BCM5333X_LPORT_MAX; i++) {
        if (i == 1) {
            continue;
        }
         /* DYNCELLRESETLIMISEL = 2184, DYNCELLLIMIT = 2216 */
         bcm5333x_reg_set(0, R_DYNCELLLIMIT(i), MMU_R_DYNCELLLIMIT);
         /* DYNXQCNTPORT = 1488 */
         bcm5333x_reg_set(0, R_DYNXQCNTPORT(i), MMU_R_DYNXQCNTPORT);

         bcm5333x_reg_set(0, R_IBPPKTSETLIMIT(i), MMU_R_IBPPKTSETLIMIT);
         bcm5333x_reg_set(0, R_IBPCELLSETLIMIT(i), MMU_R_IBPCELLSETLIMIT);

         bcm5333x_reg_set(0, R_IBPDISCARDSETLIMIT(i), MMU_R_IBPDISCARDSETLIMIT);
    }

    /* TOTALDYNCELLLIMIT = 2496, TOTALDYNCELLRESETLIMITSEL = 2464 */
    bcm5333x_reg_set(0, R_TOTALDYNCELLSETLIMIT, MMU_R_TOTALDYNCELLSETLIMIT);
    bcm5333x_reg_set(0, R_TOTALDYNCELLRESETLIMIT, MMU_R_TOTALDYNCELLRESETLIMIT);

    /* DYN_XQ_EN[Bit8] = 1, HOL_CELL_SOP_DROP_EN[Bit7] = 1 */
    bcm5333x_reg_get(0, R_MISCCONFIG, &val);
    val |= 0x180;
    bcm5333x_reg_set(0, R_MISCCONFIG, val);

    /* Apply TM=0x10 setting for all CAMs */
    bcm5333x_reg_set(0, R_SW2_RAM_CONTROL_4, 0x10);
    bcm5333x_reg_set(0, R_L2_USER_ENTRY_CAM_DBGCTRL, 0x10);
    bcm5333x_reg_set(0, R_VLAN_SUBNET_CAM_DBGCTRL, (0x10 << 2));
    bcm5333x_reg_set(0, R_VFP_CAM_CONTROL_TM_7_THRU_0, 0x10);

    /* Port enable */
#if CFG_RXTX_SUPPORT_ENABLED
    bcm5333x_reg_set(0, R_MMUPORTENABLE, 0x3FFFFFFD);
#else
    bcm5333x_reg_set(0, R_MMUPORTENABLE, 0x3FFFFFFC);
#endif /* CFG_RXTX_SUPPORT_ENABLED */
}

static void
config_schedule_mode(void)
{
    int i, j;

    SOC_LPORT_ITER(i) {
        /* Weighted Round Robin mode */
        bcm5333x_reg_set(0, R_WRRWEIGHT_COS0(i), 0x81);
        bcm5333x_reg_set(0, R_WRRWEIGHT_COS1(i), 0x82);
        bcm5333x_reg_set(0, R_WRRWEIGHT_COS2(i), 0x84);
        bcm5333x_reg_set(0, R_WRRWEIGHT_COS3(i), 0x88);
        bcm5333x_reg_set(0, R_XQCOSARBSEL(i), 0xE);

        /* MAX_THD_SEL = 0 : Disable MAX shaper */
        for (j = 0; j < COS_QUEUE_NUM; j++) {
            bcm5333x_reg_set(0, R_MAXBUCKETCONFIG(j, i), 0x0);

            /* SRED disabled, tail drop */
            bcm5333x_reg_set(0, R_CNG0COSDROPRATE(j, i), 0x0);

            bcm5333x_reg_set(0, R_CNG1COSDROPRATE(j, i), 0x0);
        }
    }
}

#ifdef CFG_SWITCH_LAG_INCLUDED
#if defined(CFG_SWITCH_RATE_INCLUDED) || defined(CFG_SWITCH_QOS_INCLUDED) || defined(CFG_SWITCH_LOOPDETECT_INCLUDED)
static void
_bcm5333x_lag_group_fp_set(uint8 unit, int start_index, uint8 lagid,
                     pbmp_t pbmp, pbmp_t old_pbmp, uint8 revise_redirect_pbmp, uint8 cpu_include)
{
    int i, j;
    uint32 tcam_entry[15], xy_entry[15], dm_entry[15];

    uint32 global_tcam_mask_entry[3] = { 0x1, 0xffffffc0, 0x7ff };

    /* The entry (pbmp) bit 0 is cpu port.
     * The policy[0] redirect entry, bit 5 is cpu port.
     */
    if (cpu_include == TRUE) {
        j = 0;
    } else {
        j = BCM5333X_LPORT_MIN;
    }
    for (i = start_index; j <= BCM5333X_LPORT_MAX; i++, j++) {
        if ((j > 0) && (j < BCM5333X_LPORT_MIN)) {
            continue;
        }
        bcm5333x_mem_get(unit, M_FP_TCAM(i), dm_entry, 15);
        bcm5333x_xy_to_dm(dm_entry, tcam_entry, 15, 234);
        /*  Revise the source tgid qualify if the port is trunk port */
        if (old_pbmp & (0x1 << j)) {
            tcam_entry[1] &= ~(0x7fff << 12);
            tcam_entry[1] |= (j << 12);
        }
        if (pbmp & (0x1 << j)) {
            tcam_entry[1] &= ~(0x7fff << 12);
            tcam_entry[1] |= ((lagid | 0x2000) << 12);
        }
        bcm5333x_dm_to_xy(tcam_entry, xy_entry, 15, 234);
        bcm5333x_mem_set(unit, M_FP_TCAM(i), xy_entry, 15);
        bcm5333x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(i), global_tcam_mask_entry, 3);
    }
}
#endif /* CFG_SWITCH_RATE_INCLUDED || CFG_SWITCH_QOS_INCLUDED || CFG_SWITCH_LOOPDETECT_INCLUDED */

/*
 *  Function : bcm5333x_lag_group_set
 *  Purpose :
 *      Set lag group membership.
 *  Parameters :
 *  Return :
 *  Note :
 *
 */
sys_error_t
bcm5333x_lag_group_set(uint8 unit, uint8 lagid, pbmp_t pbmp)
{
    uint32 bitmap_entry = (uint32)pbmp, old_bitmap_entry;
    uint32 entry[3] = {0, 0, 0};
    uint32 group_entry[4] = {0, 0, 0, 0};
    uint8 i, j, count = 0;
    uint8 trunk_port[BOARD_MAX_PORT_PER_LAG];

    for (i = BCM5333X_LPORT_MIN; i <= BCM5333X_LPORT_MAX; i++) {
        if(bitmap_entry & (0x1 << i)) {
            count ++;
            if (count > BOARD_MAX_PORT_PER_LAG) {
                 sal_printf("bcm5333x_lag_group_set %d fail\n", count);
                 continue;
            }
            trunk_port[count-1] = i;
        }
    }

    bcm5333x_mem_get(0, M_TRUNK_BITMAP(lagid), &old_bitmap_entry, 1);

    if (bitmap_entry != old_bitmap_entry) {
        /* Need to update source port qualifier in FP TCAM entry  */
#ifdef CFG_SWITCH_RATE_INCLUDED
        /*
         * Slice 1 Entry 0~23 (one entry for each port):
         * Rate ingress
         */
        _bcm5333x_lag_group_fp_set(unit, ENTRIES_PER_SLICE+3, lagid,
                                   bitmap_entry, old_bitmap_entry, FALSE, FALSE);
#endif /* CFG_SWITCH_RATE_INCLUDED */

#ifdef CFG_SWITCH_QOS_INCLUDED
        /*
         * Slice 2 Entry 0~23 (one entry for each port):
         * Port based QoS
         */
        _bcm5333x_lag_group_fp_set(unit, (ENTRIES_PER_SLICE * 2 + BCM5333X_LPORT_MIN), lagid, 
                                   bitmap_entry, old_bitmap_entry, FALSE, FALSE);
#endif /* CFG_SWITCH_QOS_INCLUDED */

#ifdef CFG_SWITCH_LOOPDETECT_INCLUDED
        /*
         * Slice 3 Entry BCM5333X_PORT_MIN~BCM5333X_PORT_MAX (one entry for each port):
         * Loop detect counter
         */
        _bcm5333x_lag_group_fp_set(unit, ENTRIES_PER_SLICE*3+BCM5333X_LPORT_MIN, lagid,
                                   bitmap_entry, old_bitmap_entry, FALSE, FALSE);

		/*
         * Slice 3, #define LOOP_REDIRECT_IDX              ( 3 * ENTRIES_PER_SLICE + 35)
         * (one entry for each port, including CPU):
         * Update both source port qualifier 
         */
        _bcm5333x_lag_group_fp_set(unit, (ENTRIES_PER_SLICE*3 + 35), lagid,
                                    bitmap_entry, old_bitmap_entry, TRUE, TRUE);
#endif /* CFG_SWITCH_LOOPDETECT_INCLUDED */
    }

    /* Set TRUNK Bitmap, TRUNK Group, Source TRUNK Map and NonUcast TRUNK Block Mask Table */
    for (i = BCM5333X_LPORT_MIN; i <= BCM5333X_LPORT_MAX; i++) {
        if(old_bitmap_entry & (0x1 << i)) {
            entry[0] = 0x0;
            bcm5333x_mem_set(unit, M_SOURCE_TRUNK_MAP(i), entry, 3);
        }

        if(bitmap_entry & (0x1 << i)) {
            entry[0] = 0x1 | (lagid << 2);
            bcm5333x_mem_set(unit, M_SOURCE_TRUNK_MAP(i), entry, 3);
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

    bcm5333x_mem_set(unit, M_TRUNK_BITMAP(lagid), &bitmap_entry, 1);
    bcm5333x_mem_set(unit, M_TRUNK_GROUP(lagid), group_entry, 4);


    for (i = 0; i < 64; i++) {
        bcm5333x_mem_get(0, M_NONUCAST_TRUNK_BLOCK_MASK(i),
                                      entry, 1);
        entry[0] &= ~old_bitmap_entry;
        entry[0] |= bitmap_entry;
        if (count != 0) {
            entry[0] &= ~(0x1 << trunk_port[i%count]);
        }
        bcm5333x_mem_set(0, M_NONUCAST_TRUNK_BLOCK_MASK(i),
                                      entry, 1);
    }
    return SYS_OK;
}

/*
 *  Function : bcm5333x_lag_group_get
 *  Purpose :
 *      Get lag group membership.
 *  Parameters :
 *  Return :
 *  Note :
 *
 */
void
bcm5333x_lag_group_get(uint8 unit, uint8 lagid, pbmp_t *lpbmp) {
    uint32 bitmap_entry;

    bcm5333x_mem_get(unit, M_TRUNK_BITMAP(lagid), &bitmap_entry, 1);

    *lpbmp = (pbmp_t)bitmap_entry;
}
#endif /* CFG_SWITCH_LAG_INCLUDED */


static void
bcm5333x_system_init(void)
{
    int i, j;
    uint32 entry[6];
    uint32 val;
    uint32 port_entry[8] = { 0x01000000, 0x02000000, 0x00200000, 0x10004000,
                             0x00000001, 0x0, 0x0, 0x0 };
    uint32 dot1pmap[16] = {
    0x00000000, 0x00000001, 0x00000004, 0x00000005, 0x00000008, 0x00000009, 0x0000000c, 0x0000000d,
    0x00000010, 0x00000011, 0x00000014, 0x00000015, 0x00000018, 0x00000019, 0x0000001c, 0x0000001d };
	uint16 index = 0;

    /* Configurations to guarantee no packet modifications */
    for (i = BCM5333X_LPORT_MIN; i <= BCM5333X_LPORT_MAX; i++) {
        /* ING_OUTER_TPID[0] for allowed outer TPID values */
        entry[0] = 0x1;
        bcm5333x_mem_set(0, M_SYSTEM_CONFIG_TABLE(i), entry, 1);

        /* DISABLE_VLAN_CHECKS[Bit 63] = 1, PACKET_MODIFICATION_DISABLE[Bit 62] = 1 */
        entry[0] = 0x0;
        entry[1] = 0x0;
        entry[2] = 0x0;
        bcm5333x_mem_set(0, M_SOURCE_TRUNK_MAP(i), entry, 3);

        /*
         * PORT table:
         * CML_FLAGS_MOVE[3:0] (Bit 128-125) = 0x8
         * CML_FLAGS_NEW[3:0] (Bit 124-121) = 0x8
         * TRUST_INCOMING_VID (Bit 110) = 0x1
         * OUTER_TPID_ENABLE[3:0] (Bit 88-85) = 0x1
         * TRUST_OUTER_DOT1P (Bit 57) = 0x1
         * PORT_VID (Bit 35-24) = 0x1
         */
        bcm5333x_mem_set(0, M_PORT(i), port_entry, 8);

        /* Clear Unknown Unicast Block Mask. */
        bcm5333x_reg_set(0, R_UNKNOWN_UCAST_BLOCK_MASK_64(i), 0x0);

        /* Clear ingress block mask. */
        bcm5333x_reg_set(0, R_ING_EGRMSKBMAP_64(i), 0x0);
    }

    for (i = 0; i <= BCM5333X_LPORT_MAX; i++) {
        /*
         * ING_PRI_CNG_MAP: Unity priority mapping and CNG = 0 or 1
         */
        for (j = 0; j < 16; j++) {
            bcm5333x_mem_set(0, M_ING_PRI_CNG_MAP(i*16+j), &dot1pmap[j], 1);
        }
    }

    /* TRUNK32_CONFIG_TABLE: OUTER_TPID_ENABLE[3:0] (Bit 0-3) = 0x1 */
    val = 0x1;
    /*
     * TRUNK32_PORT_TABLE:
     * DISABLE_VLAN_CHECKS[Bit 31] = 1, PACKET_MODIFICATION_DISABLE[Bit 30] = 1
     */
    entry[0] = 0x0;
    entry[1] = 0x0;

    for (i = 0; i < 32; i++) {
        bcm5333x_mem_set(0, M_TRUNK32_CONFIG_TABLE(i), &val, 1);
        bcm5333x_mem_set(0, M_TRUNK32_PORT_TABLE(i), entry, 2);
    }

    enable_jumbo_frame();
    config_schedule_mode();

    /*
     * VLAN_DEFAULT_PBM is used as defalut VLAN members for those ports
     * that disable VLAN checks.
     */
    bcm5333x_reg_set(0, R_VLAN_DEFAULT_PBM, 0x3FFFFFFC);

    /* ING_VLAN_TAG_ACTION_PROFILE:
     * UT_OTAG_ACTION (Bit 2-3) = 0x1
     * SIT_OTAG_ACTION (Bit 8-9) = 0x0
     * SOT_POTAG_ACTION (Bit 12-13) = 0x2
     * SOT_OTAG_ACTION (Bit 14-15) = 0x0
     * DT_POTAG_ACTION (Bit 20-21) = 0x2
     */
    val = 0x0202004;
    bcm5333x_mem_set(0, M_ING_VLAN_TAG_ACTION_PROFILE, &val, 1);

    /*
     * Program l2 user entry table to drop below MAC addresses:
     * 0x0180C2000001, 0x0180C2000002 and 0x0180C200000E
     */

    /* VALID[Bit 0] = 1 */
    entry[0] = (0xC2000001 << 1) | 0x1;
    entry[1] = (0x0180 << 1) | 0x1;

    /* Encoded MASK: MAC address only */
    entry[2] = 0x80000040;
    entry[3] = 0xffc06030;
    /* DST_DISCARD[Bit 19] = 1 */
    entry[4] = 0x000807ff;
    /* BPDU[Bit 2] = 1 */
    entry[5] = 0x4;
    bcm5333x_mem_set(0, M_L2_USER_ENTRY(0), entry, 6);

    entry[0] = (0xC2000002 << 1) | 0x1;
    entry[2] = 0x80000080;
    bcm5333x_mem_set(0, M_L2_USER_ENTRY(1), entry, 6);

    entry[0] = (0xC200000E << 1) | 0x1;
    entry[2] = 0x80000380;
    bcm5333x_mem_set(0, M_L2_USER_ENTRY(2), entry, 6);

#ifdef CFG_SWITCH_DOS_INCLUDED
    /*
     * Enable following Denial of Service protections:
     * DROP_IF_SIP_EQUALS_DIP
     * MIN_TCPHDR_SIZE = 0x14 (Default)
     * IP_FIRST_FRAG_CHECK_ENABLE
     * TCP_HDR_OFFSET_EQ1_ENABLE
     * TCP_HDR_PARTIAL_ENABLE
     */
    bcm5333x_reg_set(0, R_DOS_CONTROL, 0x2280411);
    bcm5333x_reg_set(0, R_DOS_CONTROL2, 0x01300000);
#endif /* CFG_SWITCH_DOS_INCLUDED */

    /* enable FP_REFRESH_ENABLE */
    bcm5333x_reg_get(0, R_AUX_ARB_CONTROL_2, &val);
    val |= 0x4000000;
    bcm5333x_reg_set(0, R_AUX_ARB_CONTROL_2, val);

    /*
     * Enable IPV4_RESERVED_MC_ADDR_IGMP_ENABLE[Bit 31], APPLY_EGR_MASK_ON_L3[Bit 13]
     * and APPLY_EGR_MASK_ON_L2[Bit 12]
     * Disable L2DST_HIT_ENABLE[Bit 2]
     */
    bcm5333x_reg64_get(0, R_ING_CONFIG_64, entry, 2);
    entry[0] |= 0x80003000;
    entry[0] &= ~0x4;
    bcm5333x_reg64_set(0, R_ING_CONFIG_64, entry, 2);

    /*
     * L3_IPV6_PFM=1, L3_IPV4_PFM=1, L2_PFM=1, IPV6L3_ENABLE=1, IPV4L3_ENABLE=1
     * IPMCV6_L2_ENABLE=1, IPMCV6_ENABLE=1, IPMCV4_L2_ENABLE=1, IPMCV4_ENABLE=1
     */
    entry[0] = 0x0003f015;
    bcm5333x_mem_set(0, M_VLAN_PROFILE(0), entry, 1);

    /* Do VLAN Membership check EN_EFILTER[Bit 3] for the outgoing port */
    for (i = 0; i <= BCM5333X_LPORT_MAX; i++) {
        if (i == 1) {
            continue;
        }
        bcm5333x_reg_get(0, R_EGR_PORT(i), &val);
        val |= (0x1 << 3);
        bcm5333x_reg_set(0, R_EGR_PORT(i), val);
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
    bcm5333x_mem_set(0, M_VLAN(0), entry, 4);

    entry[0] = 0x3ffffffc;
    entry[1] = 0x1fffffff;
    entry[2] = 0x00000010;
    bcm5333x_mem_set(0, M_EGR_VLAN(0), entry, 3);

#ifdef __BOOTLOADER__
    /* Default VLAN 1 with STG=1 and VLAN_PROFILE_PTR=0 for bootloader */
    entry[0] = 0x3ffffffc;
    entry[1] = 0x000000c0;
    entry[2] = 0x0;
    entry[3] = 0x0;
    bcm5333x_mem_set(0, M_VLAN(1), entry, 4);

    entry[0] = 0x3ffffffc;
    entry[1] = 0x1fffffff;
    entry[2] = 0x00000010;
    bcm5333x_mem_set(0, M_EGR_VLAN(1), entry, 3);
#endif /* __BOOTLOADER__ */

    /* Set VLAN_STG and EGR_VLAN_STG */
    entry[0] = 0xfffffff0;
    entry[1] = 0x0fffffff;
    bcm5333x_mem_set(0, M_VLAN_STG(1), entry, 2);
    bcm5333x_mem_set(0, M_EGR_VLAN_STG(1), entry, 2);

    /* Make PORT_VID[Bit 35:24] = 0 for CPU port */
    bcm5333x_mem_get(0, M_PORT(0), port_entry, 8);
    port_entry[0] &= 0x00ffffff;
    port_entry[1] &= 0xfffffff0;
    bcm5333x_mem_set(0, M_PORT(0), port_entry, 8);

    /*
     * Trap DHCP[Bit 0] and ARP packets[Bit 4, 6] to CPU.
     * Note ARP reply is copied to CPU ONLY when l2 dst is hit.
     */
    for (i = BCM5333X_LPORT_MIN ; i <= BCM5333X_LPORT_MAX ; i++) {
        bcm5333x_reg_set(0, R_PROTOCOL_PKT_CONTROL(i), 0x51);
    }
#endif /* CFG_RXTX_SUPPORT_ENABLED */

    /* Enable aging timer */
    bcm5333x_reg_get(0, R_L2_AGE_TIMER, &val);
    val |= (0x1 << 20);
    bcm5333x_reg_set(0, R_L2_AGE_TIMER, val);
	igmp_resource_init();	
	l3_ipmc_id_get(&index);
    

	/* bit 22 valid */
	sal_memset(entry, 0 , 3 * sizeof(uint32));
	entry[0] = 0x400000;

}

#define BMD_MODID(_u)  (_u)
#define HG_TRUNKID(_u)  (0)

static int 
_bmd_post_init(int unit)
{
    int lport, dest_unit, dest_modid;
    uint32 val, entry[8];
    uint32 hg_trunk_grp, hg_pbmp, port_idx;
    uint8 myunit = 0;

    /* Prepare HiGig trunk group configuration */
    hg_trunk_grp = 0;
    hg_pbmp = 0;
    port_idx = 0;

    SOC_XLPORT_ITER(lport) {
        if (IS_HG_PORT(lport)) {
            /* Add port to HiGig trunk members */
            if ((port_idx % 2) == 0) {
                /* HIGIG_TRUNK_PORT0[7:3], HIGIG_TRUNK_PORT2[17:13] */
                hg_trunk_grp |= (lport << 3) | (lport << 13);
            } else {
                /* HIGIG_TRUNK_PORT1[12:8], HIGIG_TRUNK_PORT3[22:18] */
                hg_trunk_grp |= (lport << 8) | (lport << 18);
            }
            hg_pbmp |= (1 << port_idx);
        }
        port_idx++;      
    }

    if (port_idx == 0)  {
        /* Check if higig ports enabled in the SKU */
        return SYS_OK;
    }

    /* We assume that units 0 and 1 are used */
    if (unit == 0) {
        dest_unit = 1;
    } else if (unit == 1) {
        dest_unit = 0;
    } else {
        return SYS_ERR_PARAMETER;
    }
    dest_modid = BMD_MODID(dest_unit);

    /* HIGIG_TRUNK_RTAG[2:0] = type 3 */
    hg_trunk_grp |= 0x3;

    /* Write trunk group to hardware */
    bcm5333x_reg_set(myunit, R_HG_TRUNK_GROUP(HG_TRUNKID(unit)), hg_trunk_grp);
    bcm5333x_reg_set(myunit, R_HG_TRUNK_BITMAP(HG_TRUNKID(unit)), hg_pbmp);

    /* Configure egress HiGig ports for destination module ID */
    /* Update HIGIG_PORT_BITMAPf in MODPORT_MAP */
    /* Bit0 = port26 Bit1 = port27 Bit2 = port28 Bit3 = port29. */
    bcm5333x_mem_set(myunit, M_MODPORT_MAP(dest_modid), &hg_pbmp, 1);

    for (lport = 0; lport <= BCM5333X_LPORT_MAX; lport++) {
        if (SOC_PORT_L2P_MAPPING(lport) == -1) {
            continue;
        }

        bcm5333x_mem_get(myunit, M_PORT(lport), entry, 8);
        /* PORT table: MY_MODID [46:40] */
        entry[1] &= ~(0x7f << 8);
        entry[1] |= (BMD_MODID(unit) << 8);

        if (IS_HG_PORT(lport)) {
            /* Port type [37:36] = 1, Higig2 [55] = 1, HIGIG_TRUNK[51] = 1 */
            entry[1] |= (0x880010);
        }

        bcm5333x_mem_set(myunit, M_PORT(lport), entry, 8);

        if (IS_HG_PORT(lport)) {
            bcm5333x_mem_set(myunit, M_IPORT(lport), entry, 8);
        }

        /* EGR_PORT: MY_MODID[15:9]
         * Higig port: PORT_TYPE[1:0] = 1, HIGIG2[2] = 1 
         */
        bcm5333x_reg_get(myunit, R_EGR_PORT(lport), &val);
        val &= ~(0xfe07);
        if (IS_HG_PORT(lport)) {
            val |= 0x5;
        }
        val |= (BMD_MODID(unit) << 9);
        bcm5333x_reg_set(myunit, R_EGR_PORT(lport), val);

        if (IS_XL_PORT(lport) && lport != 0) {
            /* Update MY_MODID[8:1] and higig2 mode[11] in XLPORT_CONFIG */
            val = (BMD_MODID(unit) << 1);
            if (IS_HG_PORT(lport)) {
                val |= (1 << 11);
            }
            bcm5333x_reg_set(myunit, SOC_PORT_BLOCK(lport),
                             R_XLPORT_CONFIG(SOC_PORT_BLOCK_INDEX(lport)), val);
        }
    }

    entry[0] = 0x1;
    for (lport = BCM5333X_LPORT_MIN; lport <= BCM5333X_LPORT_MAX; lport++) {
        /* ING_OUTER_TPID[0] for allowed outer TPID values */
        /* indexed via \{SRC_MODID[5:0], SRC_PORT[5:0]\}' */
        bcm5333x_mem_set(myunit, M_SYSTEM_CONFIG_TABLE((BMD_MODID(unit) << 6)+lport), entry, 1);
        bcm5333x_mem_set(myunit, M_SYSTEM_CONFIG_TABLE((dest_modid << 6)+lport), entry, 1);
    }

    /* Create special egress action profile for HiGig ports */
    /* EGR_VLAN_TAG_ACTION_PROFILEm_SOT_OTAG_ACTIONf_SET(egr_action, 3);
     * EGR_VLAN_TAG_ACTION_PROFILEm_DT_OTAG_ACTIONf_SET(egr_action, 3);
     */
    val = 0x303;
    bcm5333x_mem_set(myunit, M_EGR_VLAN_TAG_ACTION_PROFILE(1), &val, 1);
    val = (1 << 16);
    SOC_HGPORT_ITER(lport) {
        bcm5333x_reg_set(myunit, R_EGR_VLAN_CONTROL_3(lport), val);
    }

#ifdef DISABLE_VLAN_CHECK_INCLUDED
    entry[0] = 0x0;
    /* DISABLE_VLAN_CHECKS[Bit 63] = 1, PACKET_MODIFICATION_DISABLE[Bit 62] = 1 */
    entry[1] = 0xC0000000;
    entry[2] = 0x0;
    for (lport = BCM5333X_LPORT_MIN; lport <= BCM5333X_LPORT_MAX; lport++) {
        /* indexed by the concatenation of \{MY_MODID[5:0], INGRESS_PORT[5:0]\} */
        bcm5333x_mem_set(myunit, M_SOURCE_TRUNK_MAP((BMD_MODID(unit) << 6)+lport), entry, 3);
        bcm5333x_mem_set(myunit, M_SOURCE_TRUNK_MAP((dest_modid << 6)+lport), entry, 3);
    }

    /* 
     * TRUNK32_PORT_TABLE:
     * DISABLE_VLAN_CHECKS[Bit 31] = 1, PACKET_MODIFICATION_DISABLE[Bit 30] = 1 
     */
    entry[0] = 0xC0000000;
    entry[1] = 0x0;

    for (lport = 0; lport < 32; lport++) {
        bcm5333x_mem_set(myunit, M_TRUNK32_PORT_TABLE(lport), entry, 2);
    }
#endif /* DISABLE_VLAN_CHECK_INCLUDED */

    return SYS_OK;
}

#define FW_ALIGN_BYTES                  16
#define FW_ALIGN_MASK                   (FW_ALIGN_BYTES - 1)
int
_firmware_helper(void *ctx, uint32 offset, uint32 size, void *data)
{
    uint32 wbuf[4], ucmem_data[4];
    uint32 *fw_data;
    uint32 *fw_entry;
    uint32 fw_size;
    uint32 idx, wdx;
    phy_ctrl_t *pc = (phy_ctrl_t *)ctx;
    int lport;

    /* Check if PHY driver requests optimized MDC clock */
    if (data == NULL) {
        return SYS_OK;
    }
    if (sal_strcmp(pc->drv->drv_name, "bcmi_tsc_xgxs") != 0) {
        return SYS_OK;
    }
    if (size == 0) {
        return SYS_ERR_PARAMETER;
    }

    /* Aligned firmware size */
    fw_size = (size + FW_ALIGN_MASK) & ~FW_ALIGN_MASK;

    /* Enable parallel bus access, ACCESS_MODE [Bit 0] = 0x1 */
    lport = SOC_PORT_P2L_MAPPING(pc->port);
    bcm5333x_reg_set(pc->unit, SOC_PORT_BLOCK(lport), R_XLPORT_WC_UCMEM_CTRL, 0x1);

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
            ucmem_data[(wdx+2)&0x3] = fw_entry[wdx];
        }
        bcm5333x_mem_set(pc->unit, SOC_PORT_BLOCK(lport),
                         M_XLPORT_WC_UCMEM_DATA(idx >> 4), ucmem_data, 4);
    }

    /* Disable parallel bus access */
    bcm5333x_reg_set(pc->unit, SOC_PORT_BLOCK(lport), R_XLPORT_WC_UCMEM_CTRL, 0x0);

    return SYS_OK;
}

extern int 
bmd_phy_fw_helper_set(int unit, int port,
                      int (*fw_helper)(void *, uint32_t, uint32_t, void *));

/* Function:
 *   bcm5333x_sw_init
 * Description:
 *   Perform chip specific initialization.
 *   This will be called by board_init()
 * Parameters:
 *   None
 * Returns:
 *   None
 */

sys_error_t
bcm5333x_sw_init(void)
{
    uint8 unit = 0;
    uint8 lport;
    int rv = 0;
    int speed, an, ability;
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
    int port_cnt = 0;
    sys_error_t sal_config_rv = SYS_OK;
    pbmp_t active_pbmp, speed_1000_pbmp, speed_10000_pbmp;
    sys_error_t an_rv = SYS_OK, cl73_rv = SYS_OK, cl37_rv = SYS_OK;
    pbmp_t phy_an_pbmp, phy_cl73_pbmp, phy_cl37_pbmp;
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
    
    /* CPS reset complete SWITCH and CMICd */
    WRITECSR(CMIC_CPS_RESET, 0x1);
    sal_usleep(1000);

    /* Get chip revision */
    bcm5333x_chip_revision(unit, &hr2_sw_info.devid, &hr2_sw_info.revid);

#if CFG_CONSOLE_ENABLED
    sal_printf("\ndevid = 0x%x, revid = 0x%x\n", hr2_sw_info.devid, hr2_sw_info.revid);
#endif /* CFG_CONSOLE_ENABLED */

#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED 
    sal_config_rv = sal_config_uint16_get(SAL_CONFIG_SKU_DEVID, &hr2_sw_info.devid);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Overwrite SKU device ID with value 0x%x.\n", hr2_sw_info.devid);
    }

    sal_config_rv = sal_config_uint8_get(SAL_CONFIG_SKU_OPTION, &config_id);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Overwrite SKU option with value %d.\n", config_id);
    }
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */

    if ((hr2_sw_info.devid != BCM53333_DEVICE_ID) &&
        (hr2_sw_info.devid != BCM53334_DEVICE_ID) &&
        (hr2_sw_info.devid != BCM53393_DEVICE_ID) &&
        (hr2_sw_info.devid != BCM53394_DEVICE_ID) &&
        (hr2_sw_info.devid != BCM53342_DEVICE_ID) &&
        (hr2_sw_info.devid != BCM53343_DEVICE_ID) &&
        (hr2_sw_info.devid != BCM53344_DEVICE_ID) &&
        (hr2_sw_info.devid != BCM53346_DEVICE_ID)) {
        sal_printf("\nERROR : devid 0x%x is not supported in UM software !\n", hr2_sw_info.devid);
        return SYS_ERR_NOT_FOUND;
    }

    soc_reset();

    soc_misc_init();

    soc_mmu_init();

    bcm5333x_system_init();

	sal_memset(counter_val, 0 ,sizeof(uint32) * BCM5333X_LPORT_MAX *(R_MAX * 2));	

    /* Probe PHYs */
    SOC_LPORT_ITER(lport) {
        rv = bmd_phy_probe(unit, lport);
        if (CDK_SUCCESS(rv)) {
            if (IS_XL_PORT(lport)) {
                int lanes = soc_port_num_lanes(unit, lport);
                if (lanes == 4) {
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_tsc_xgxs",
                                            BMD_PHY_MODE_SERDES, 0);
                } else if (lanes == 2) {
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_tsc_xgxs",
                                            BMD_PHY_MODE_SERDES, 1);
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_tsc_xgxs", 
                                            BMD_PHY_MODE_2LANE, 1);
                } else { /* lanes = 1 */
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_tsc_xgxs",
                                            BMD_PHY_MODE_SERDES, 1);
                    rv = bmd_phy_mode_set(unit, lport, "bcmi_tsc_xgxs", 
                                            BMD_PHY_MODE_2LANE, 0);
                }

                if (SOC_PORT_BLOCK_INDEX(lport) == 0) {
                    rv = bmd_phy_fw_base_set(unit, lport, "bcmi_tsc_xgxs", 0xee0);
                    rv = bmd_phy_fw_helper_set(unit, lport, _firmware_helper);
                }
            }
        }
    }

    /* Init PHYs */
    if (CDK_SUCCESS(rv)) {
        rv = bmd_phy_staged_init(unit);
    }
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED 
    sal_config_rv = sal_config_pbmp_get(SAL_CONFIG_VALID_PORTS, &active_pbmp);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set valid logical pbmp with value 0x%x.\n", active_pbmp);
        SOC_LPORT_ITER(lport) {
            if (active_pbmp & (0x1 << lport)) {
                port_cnt ++;
                lport_active[lport] = 1; 
            } else {
                lport_active[lport] = 0; 
            }
        }
        SOC_PORT_COUNT(unit) = port_cnt;
    }

    sal_config_rv = sal_config_pbmp_get(SAL_CONFIG_SPEED_1000_PORTS, &speed_1000_pbmp);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set speed (1G) logical pbmp with value 0x%x.\n", speed_1000_pbmp);
        SOC_LPORT_ITER(lport) {
            if ((speed_1000_pbmp & (0x1 << lport)) && (SOC_PORT_SPEED_MAX(lport) >= 1)){
               SOC_PORT_SPEED_MAX(lport) = 1;
            }
        }
    }

    sal_config_rv = sal_config_pbmp_get(SAL_CONFIG_SPEED_10000_PORTS, &speed_10000_pbmp);
    if (sal_config_rv == SYS_OK) {
        sal_printf("Vendor Config : Set speed (10G) logical pbmp with value 0x%x.\n", speed_10000_pbmp);
        SOC_LPORT_ITER(lport) {
            if ((speed_10000_pbmp & (0x1 << lport)) && (SOC_PORT_SPEED_MAX(lport) >= 10)){
               SOC_PORT_SPEED_MAX(lport)= 10;
            }
        }
    }

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

#endif /*  CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */

    SOC_XLPORT_ITER(lport) {
        phy_ctrl_t *pc = NULL;

        if (SOC_PORT_BLOCK_INDEX(lport) == 0) {
            rv = bmd_phy_laneswap_set(unit, lport);
        }

        switch (hr2_sw_info.devid) {
            case BCM53393_DEVICE_ID:
            case BCM53394_DEVICE_ID:

                pc = BMD_PORT_PHY_CTRL(unit, lport);

                speed = SOC_PORT_SPEED_MAX(lport)*1000;
                if (speed == 11000) {
                    speed = 10000;
                }

                if (speed == 1000) {
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
            
                    an = an ? TRUE : FALSE;
            
                    rv = PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, lport), 1000);   
                    rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, lport), an);
                    if (!SOC_SUCCESS(rv)) {
                        sal_printf("bcm5333x_sw_init set phy autoneg %d on lport %d failed\n", an, lport);
                    }
            
                    if (!an) {
                        rv = PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, lport), 1000);
                        if (!SOC_SUCCESS(rv)) {
                            sal_printf("bcm5333x_sw_init set phy speed %d on lport %d failed\n", 1000, lport);
                        }
                    }         
                } else if(speed == 10000) {
                    /* Add 10G ability */
                    ability = PHY_ABIL_10GB;
                    rv = bmd_phy_ability_set(unit, lport, "bcmi_tsc_xgxs", ability);
                    if (!SOC_SUCCESS(rv)) {
                        sal_printf("bcm5343x_sw_init set phy ability 0x%x on lport %d failed\n", ability, lport);
                    }

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

                    an = an ? TRUE : FALSE;
                    
                    rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, lport), an);
                    if (!SOC_SUCCESS(rv)) {
                        sal_printf("bcm5333x_sw_init set phy autoneg %d on lport %d failed\n", an, lport);
                    }
                    
                    if (!an) {
                        rv = PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, lport), 10000);
                        if (!SOC_SUCCESS(rv)) {
                            sal_printf("bcm5333x_sw_init set phy speed %d on lport %d failed\n", 10000, lport);
                        }
                    }  
                } else {
                    sal_printf("\nERROR : speed % is not supported in bcm5339x_xlport_mode_set with lport !\n", speed, lport);
                }
                break;
            case BCM53343_DEVICE_ID:
            case BCM53344_DEVICE_ID:
            case BCM53346_DEVICE_ID:
                if (IS_HG_PORT(lport)) {                    
                    an = FALSE;

                    rv = bmd_phy_line_interface_set(unit, lport, BMD_PHY_IF_HIGIG);

                    rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, lport), an);
                    if (!an) {
                        speed = SOC_PORT_SPEED_MAX(lport)*1000;
                        if (speed == 11000) {
                            speed = 10000;
                        }

                        rv = PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, lport), speed);
                        if (!SOC_SUCCESS(rv)) {
                            sal_printf("bcm5334x_xlport_mode_set set phy speed %d on lport %d failed %d\n", speed, lport, rv);
                        }
                    }
                } else {
                    pc = BMD_PORT_PHY_CTRL(unit, lport);

                    speed = SOC_PORT_SPEED_MAX(lport)*1000;
                    if (speed == 11000) {
                        speed = 10000;
                    }
    
                    if (speed == 1000) {
                        an = 0;
                        
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
                        if (an_rv == SYS_OK){
                            if (phy_an_pbmp & (0x1 << lport)) {
                                /* set cl37 flag*/
                                if (cl37_rv == SYS_OK) {
                                    if  (phy_cl37_pbmp & (0x1 << lport)) {
                                        PHY_CTRL_FLAGS(pc) |= PHY_F_CLAUSE37;
                                    }
                                }
                                
                                /* set cl73 flag*/
                                if (cl73_rv == SYS_OK) {
                                    if  (phy_cl73_pbmp & (0x1 << lport)) {
                                        PHY_CTRL_FLAGS(pc) |= PHY_F_CLAUSE73;
                                    }
                                }
                                an = 1;
                            }
                        }
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
                
                        an = an ? TRUE : FALSE;
                
                        rv = PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, lport), 1000);   
                        rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, lport), an);
                        if (!SOC_SUCCESS(rv)) {
                            sal_printf("bcm5333x_sw_init set phy autoneg %d on lport %d failed\n", an, lport);
                        }
                
                        if (!an) {
                            rv = PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, lport), 1000);
                            if (!SOC_SUCCESS(rv)) {
                                sal_printf("bcm5333x_sw_init set phy speed %d on lport %d failed\n", 1000, lport);
                            }
                        }         
                    } else if(speed == 10000) {
                        /* Add 10G ability */
                        ability = PHY_ABIL_10GB;
                        rv = bmd_phy_ability_set(unit, lport, "bcmi_tsc_xgxs", ability);
                        if (!SOC_SUCCESS(rv)) {
                            sal_printf("bcm5343x_sw_init set phy ability 0x%x on lport %d failed\n", ability, lport);
                        }
    
                        an = 0;
                        PHY_CTRL_FLAGS(pc) |= PHY_F_CLAUSE73;
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
                         if (an_rv == SYS_OK){
                            if (phy_an_pbmp & (0x1 << lport)) {
                                /* Clause 73 */
                                an = 1;
                            }
                         }
#endif  /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
    
                        an = an ? TRUE : FALSE;
                        
                        rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, lport), an);
                        if (!SOC_SUCCESS(rv)) {
                            sal_printf("bcm5333x_sw_init set phy autoneg %d on lport %d failed\n", an, lport);
                        }
                        
                        if (!an) {
                            rv = PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, lport), 10000);
                            if (!SOC_SUCCESS(rv)) {
                                sal_printf("bcm5333x_sw_init set phy speed %d on lport %d failed\n", 10000, lport);
                            }
                        }  
                    } else {
                        sal_printf("\nERROR : speed % is not supported in bcm5339x_xlport_mode_set with lport !\n", speed, lport);
                    }
                }
                break;
            default:
                break;
        }
    }

    /* Init MACs */
    SOC_LPORT_ITER(lport) {
        if (IS_XL_PORT(lport)) {
            hr2_sw_info.p_mac[lport] = &soc_mac_xl;
        }
        else {
            hr2_sw_info.p_mac[lport] = &soc_mac_uni;
        }
        MAC_INIT(hr2_sw_info.p_mac[lport], unit, lport);
        /* Probe function should leave port disabled */
        MAC_ENABLE_SET(hr2_sw_info.p_mac[lport], unit, lport, FALSE);

        hr2_sw_info.link[lport] = PORT_LINK_DOWN;
#if defined(CFG_SWITCH_EEE_INCLUDED)
        hr2_sw_info.need_process_for_eee_1s[lport] = FALSE;
#endif /*  CFG_SWITCH_EEE_INCLUDED */
    }

    _bmd_post_init(unit);

    bcm5333x_load_led_program();

#if CONFIG_HURRICANE2_EMULATION
    bcm5333x_linkscan_task(NULL);
#if CFG_CONSOLE_ENABLED
    sal_printf("all ports up!\n");
#endif /* CFG_CONSOLE_ENABLED */
#else
    /* Register background process for handling link status */
    timer_add(bcm5333x_linkscan_task, NULL, LINKSCAN_INTERVAL);
#endif

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
bcm5333x_dm_to_xy(uint32 *dm_entry, uint32 *xy_entry, int data_words, int bit_length)
{
    int i;
    int data_bytes;
    int word_length = (bit_length + 31) / 32;;
    uint32 temp_tcam_entry[data_words];

    uint32 key[word_length];
    uint32 mask[word_length];
    uint32 converted_key, converted_mask;

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
        converted_mask = key[i] | ~mask[i];
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
bcm5333x_xy_to_dm(uint32 *xy_entry, uint32 *dm_entry, int data_words, int bit_length)
{
    int i;
    int data_bytes;
    int word_length = (bit_length + 31) / 32;;
    uint32 temp_tcam_entry[data_words];

    uint32 key[word_length];
    uint32 mask[word_length];

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
        mask[i] = key[i] | ~mask[i];
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
bcm5333x_loopback_enable(uint8 unit, uint8 lport, int loopback_mode)
{
    int rv;
    int link;
    int andone;

    if (loopback_mode == PORT_LOOPBACK_NONE) {
        if (hr2_sw_info.loopback[lport] != PORT_LOOPBACK_NONE) {
            if (hr2_sw_info.loopback[lport] == PORT_LOOPBACK_MAC) {
                bcm5333x_handle_link_down(unit, lport, TRUE);
            } else if (hr2_sw_info.loopback[lport] == PORT_LOOPBACK_PHY) {
                rv = PHY_LOOPBACK_SET(BMD_PORT_PHY_CTRL(unit, lport), 0);
                rv = PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit, lport), &link, &andone);
            }
            hr2_sw_info.loopback[lport] = PORT_LOOPBACK_NONE;
        }
        return;
    }
    hr2_sw_info.loopback[lport] = (uint8)loopback_mode;

    rv = PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit, lport), &link, &andone);
    if (rv < 0) {
#if CFG_CONSOLE_ENABLED
        sal_printf("Failed to get link of lport %d\n", (int)lport);
#endif /* CFG_CONSOLE_ENABLED */
        return;
    }

    if (link) {
        /* Force link change */
        sal_printf("force lport %d link change\n", lport);
        hr2_sw_info.link[lport] = PORT_LINK_DOWN;
    }

    if (loopback_mode == PORT_LOOPBACK_PHY) {
        int speed = SOC_PORT_SPEED_MAX(lport)*1000;
        rv = PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit, lport), 0);
        if (speed == 11000) {
            speed = 10000;
        }
        rv = PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit, lport), speed);
        rv = PHY_LOOPBACK_SET(BMD_PORT_PHY_CTRL(unit, lport), 1);
        if (rv < 0) {
#if CFG_CONSOLE_ENABLED
            sal_printf("Failed to set phy loopback of port %d\n", (int)lport);
#endif /* CFG_CONSOLE_ENABLED */
            return;
        }
    }
}

sys_error_t
bcm5333x_port_info_get(uint8 unit, uint8 lport, bcm_port_info_t *info)
{
    int rv;    
    int duplex;
    uint32 speed;
    BOOL   tx_pause, rx_pause;
    int an = 0;
    int loopback;
    int link;
    int andone;
    uint8 pport;
    uint32 en;
    uint32 mtu;
    uint32 disard;
	uint32 stg;
	//sal_printf("%s:%d  lport = %d\n", __FUNCTION__, __LINE__, lport);
    if (info == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return SYS_ERR;
    }
    rv = PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit, lport), &link, &andone);
    if (rv) {
        sal_printf("error 0:%d\n", rv);
        return  SYS_ERR;
    }
    rv = PHY_SPEED_GET(BMD_PORT_PHY_CTRL(unit, lport), &speed);
    if (rv) {
        sal_printf("error 1:%d\n", rv);
        return SYS_ERR;
    }
    rv |= PHY_DUPLEX_GET(BMD_PORT_PHY_CTRL(unit, lport), &duplex);
    if (rv) {
        sal_printf("error 2:%d\n", rv);
        return SYS_ERR;
    }
    rv |= PHY_AUTONEG_GET(BMD_PORT_PHY_CTRL(unit, lport), &an);
    if (an) {
        rv |= phy_pause_get(unit, lport, &tx_pause, &rx_pause);
        if (SOC_FAILURE(rv)) {
            sal_printf("phy_pause_get lport %d error\n", lport);
            return  SYS_ERR;;
        }
    } else {
        tx_pause = TRUE;
        rx_pause = TRUE;
    }
    PHY_LOOPBACK_GET(BMD_PORT_PHY_CTRL(unit, lport), &loopback);

    pport = SOC_PORT_L2P_MAPPING(lport);
    if (!SOC_IS_DEERHOUND(unit) && pport < PHY_SECOND_QGPHY_PORT0) {
        /* Get chip local port for BMD_PORT_PHY_CTRL for FH */
        rv = PHY_CONFIG_GET(BMD_PORT_PHY_CTRL(unit, pport), 
                       PhyConfig_Enable, &en, NULL);
    } else {   
    
        rv = PHY_CONFIG_GET(BMD_PORT_PHY_CTRL(unit, lport), 
                       PhyConfig_Enable, &en, NULL);
    }
    if (rv) {
        sal_printf("error 3:%d\n", rv);
        return SYS_ERR;
    }
    MAC_MTU_GET(hr2_sw_info.p_mac[lport], unit, lport, &mtu);
    info->enable = en;
    info->linkstatus = link;
    info->speed = speed;
    info->duplex = duplex;
    info->autoneg = an;
    info->pause_tx = tx_pause;
    info->pause_rx = rx_pause;
    info->loopback = loopback;
    info->frame_max = mtu;
    sal_memset(info->phy_name, 0 , 256);
    sal_memcpy(info->phy_name, BMD_PORT_PHY_CTRL(unit, (lport))->drv->drv_name,
		sizeof(BMD_PORT_PHY_CTRL(unit, (lport))->drv->drv_name));
	bcm5333x_port_disacrd_mod_get(unit,lport, &disard);
	info->learn = disard;
	bcm5333x_port_stg_state_get(unit, lport, &stg);
	info->stp_state = stg;
    return SYS_OK;
}

soc_switch_t soc_switch_bcm5333x =
{
    bcm5333x_chip_type,
    NULL,
    bcm5333x_port_count,
    NULL,
    NULL,
#if CFG_RXTX_SUPPORT_ENABLED
    bcm5333x_rx_set_handler,
    bcm5333x_rx_fill_buffer,
    bcm5333x_tx,
#endif /* CFG_RXTX_SUPPORT_ENABLED */

    bcm5333x_link_status,
    bcm5333x_chip_revision,
    bcm5333x_reg_get,
    bcm5333x_reg_set,
    bcm5333x_mem_get,
    bcm5333x_mem_set,
#ifdef CFG_SWITCH_VLAN_INCLUDED
    bcm5333x_pvlan_egress_set,
    bcm5333x_pvlan_egress_get,
    bcm5333x_qvlan_port_set,
    bcm5333x_qvlan_port_get,
    bcm5333x_vlan_create,
    bcm5333x_vlan_destroy,
    bcm5333x_vlan_type_set,
    bcm5333x_vlan_reset,
#endif  /* CFG_SWITCH_VLAN_INCLUDED */
    bcm5333x_phy_reg_get,
    bcm5333x_phy_reg_set,
    bcm5333x_port_info_get,

};


