/*
 * $Id: t_gif.c,v 1.3 Broadcom SDK $
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
    0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x16, 0x00,      /* GIF89a.. */
    0x16, 0x00, 0xa2, 0x03, 0x00, 0xff, 0xfb, 0xff,      /* ........ */
    0xbd, 0xbe, 0xbd, 0x8c, 0x8e, 0x8c, 0x00, 0x00,      /* ........ */
    0x00, 0x84, 0x82, 0x84, 0x5c, 0x00, 0x70, 0x00,      /* ....\.p. */
    0x31, 0x00, 0x74, 0x00, 0x5c, 0x21, 0xf9, 0x04,      /* 1.t.\!.. */
    0x08, 0x00, 0x00, 0x03, 0x00, 0x2c, 0x00, 0x00,      /* .....,.. */
    0x00, 0x00, 0x16, 0x00, 0x16, 0x00, 0x40, 0x03,      /* ......@. */
    0x40, 0x08, 0xb1, 0xdc, 0xfe, 0x90, 0x8d, 0x21,      /* @......! */
    0xaa, 0x70, 0x37, 0x86, 0x99, 0x5f, 0xd7, 0xda,      /* .p7.._.. */
    0xf7, 0x70, 0x91, 0x08, 0x7a, 0x27, 0x68, 0x6e,      /* .p..z'hn */
    0x53, 0x3b, 0xa1, 0x1a, 0x09, 0xad, 0x12, 0x55,      /* S;.....U */
    0xa6, 0x21, 0x7e, 0xeb, 0x30, 0xdf, 0xac, 0x2e,      /* .!~.0... */
    0x97, 0x88, 0xb6, 0x90, 0xf5, 0x20, 0x46, 0x0c,      /* ..... F. */
    0x28, 0xf9, 0x5b, 0xda, 0x66, 0xa7, 0xca, 0xce,      /* (.[.f... */
    0xc7, 0xb8, 0x58, 0xae, 0xd8, 0x6c, 0x85, 0x90,      /* ..X..l.. */
    0x00, 0x00, 0x21, 0xfe, 0x00, 0x3b,                  /* ..!..;   */
};


RES_CONST_DECL SSP_DATA_ENTRY CODE sspfile_t_gif[] = {
    { 0x80, 0, 126, 0, 0, _text0000 },
};
