/*
 * $Id: b_gif.c,v 1.3 Broadcom SDK $
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
    0x16, 0x00, 0xa2, 0xff, 0x00, 0xff, 0xfb, 0xff,      /* ........ */
    0xef, 0xeb, 0xef, 0xbd, 0xbe, 0xbd, 0xef, 0xef,      /* ........ */
    0xef, 0x8c, 0x8e, 0x8c, 0x84, 0x82, 0x84, 0x00,      /* ........ */
    0x70, 0x00, 0x67, 0x00, 0x61, 0x21, 0xf9, 0x04,      /* p.g.a!.. */
    0x09, 0x00, 0x00, 0xff, 0x00, 0x2c, 0x00, 0x00,      /* .....,.. */
    0x00, 0x00, 0x16, 0x00, 0x16, 0x00, 0x40, 0x03,      /* ......@. */
    0x34, 0x08, 0xb1, 0xdc, 0xee, 0x62, 0xc8, 0x49,      /* 4....b.I */
    0xab, 0x25, 0x36, 0x6b, 0xac, 0xfb, 0xe4, 0x5e,      /* .%6k...^ */
    0x07, 0x86, 0xd9, 0x48, 0x56, 0xe6, 0xf9, 0xa9,      /* ...HV... */
    0x25, 0x7b, 0xb9, 0x28, 0x4c, 0xa5, 0x2c, 0xad,      /* %{.(L.,. */
    0xda, 0x27, 0x4e, 0xea, 0x21, 0xef, 0xf9, 0x22,      /* .'N.!.." */
    0xd9, 0x4a, 0x38, 0x20, 0x08, 0x08, 0xc8, 0xa4,      /* .J8 .... */
    0x72, 0xb9, 0x2c, 0x24, 0x00, 0x00, 0x21, 0xfe,      /* r.,$..!. */
    0x00, 0x3b,                                          /* .;       */
};


RES_CONST_DECL SSP_DATA_ENTRY CODE sspfile_b_gif[] = {
    { 0x80, 0, 114, 0, 0, _text0000 },
};
