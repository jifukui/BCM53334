/*
 * $Id: left_css.c,v 1.3 Broadcom SDK $
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

/***** GENERATED FILE; DO NOT EDIT. *****/

#include "appl/ssp.h"

static RES_CONST_DECL unsigned char CODE _text0000[] = {
    0x42, 0x4f, 0x44, 0x59, 0x20, 0x7b, 0x0a, 0x09,      /* BODY {.. */
    0x46, 0x4f, 0x4e, 0x54, 0x2d, 0x53, 0x49, 0x5a,      /* FONT-SIZ */
    0x45, 0x3a, 0x20, 0x31, 0x32, 0x70, 0x78, 0x3b,      /* E: 12px; */
    0x20, 0x4d, 0x41, 0x52, 0x47, 0x49, 0x4e, 0x3a,      /*  MARGIN: */
    0x20, 0x36, 0x70, 0x78, 0x3b, 0x20, 0x46, 0x4f,      /*  6px; FO */
    0x4e, 0x54, 0x2d, 0x46, 0x41, 0x4d, 0x49, 0x4c,      /* NT-FAMIL */
    0x59, 0x3a, 0x20, 0x41, 0x72, 0x69, 0x61, 0x6c,      /* Y: Arial */
    0x3b, 0x20, 0x48, 0x45, 0x49, 0x47, 0x48, 0x54,      /* ; HEIGHT */
    0x3a, 0x20, 0x30, 0x70, 0x78, 0x0a, 0x7d, 0x0a,      /* : 0px.}. */
    0x41, 0x20, 0x7b, 0x0a, 0x09, 0x46, 0x4f, 0x4e,      /* A {..FON */
    0x54, 0x2d, 0x57, 0x45, 0x49, 0x47, 0x48, 0x54,      /* T-WEIGHT */
    0x3a, 0x20, 0x62, 0x6f, 0x6c, 0x64, 0x3b, 0x20,      /* : bold;  */
    0x46, 0x4f, 0x4e, 0x54, 0x2d, 0x53, 0x49, 0x5a,      /* FONT-SIZ */
    0x45, 0x3a, 0x20, 0x31, 0x30, 0x70, 0x74, 0x3b,      /* E: 10pt; */
    0x20, 0x43, 0x4f, 0x4c, 0x4f, 0x52, 0x3a, 0x20,      /*  COLOR:  */
    0x77, 0x68, 0x69, 0x74, 0x65, 0x3b, 0x20, 0x46,      /* white; F */
    0x4f, 0x4e, 0x54, 0x2d, 0x46, 0x41, 0x4d, 0x49,      /* ONT-FAMI */
    0x4c, 0x59, 0x3a, 0x20, 0x41, 0x72, 0x69, 0x61,      /* LY: Aria */
    0x6c, 0x0a, 0x7d, 0x0a,                              /* l.}.     */
};


RES_CONST_DECL SSP_DATA_ENTRY CODE sspfile_left_css[] = {
    { 0x80, 0, 148, 0, 0, _text0000 },
};
