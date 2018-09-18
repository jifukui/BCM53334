/*
 * $Id: u_gif.c,v 1.3 Broadcom SDK $
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
    0x16, 0x00, 0xb3, 0x04, 0x00, 0xff, 0xff, 0xff,      /* ........ */
    0xde, 0xdf, 0xde, 0xad, 0xb6, 0xbd, 0x8c, 0x8e,      /* ........ */
    0x8c, 0x00, 0x00, 0x00, 0x84, 0x86, 0x84, 0x00,      /* ........ */
    0x31, 0x00, 0x75, 0x00, 0x5c, 0x00, 0x2e, 0x00,      /* 1.u.\... */
    0x69, 0x00, 0x67, 0x00, 0x66, 0x00, 0x00, 0x00,      /* i.g.f... */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /* ........ */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0xf9, 0x04,      /* .....!.. */
    0x08, 0x00, 0x00, 0x04, 0x00, 0x2c, 0x00, 0x00,      /* .....,.. */
    0x00, 0x00, 0x16, 0x00, 0x16, 0x00, 0x40, 0x04,      /* ......@. */
    0x50, 0x10, 0x84, 0x49, 0xab, 0xb5, 0xe2, 0x06,      /* P..I.... */
    0x42, 0x06, 0xe5, 0x5e, 0x15, 0x6a, 0x1c, 0x38,      /* B..^.j.8 */
    0x52, 0xa7, 0xa6, 0x4e, 0xe9, 0x04, 0x7e, 0x9d,      /* R..N..~. */
    0xd5, 0xae, 0xd7, 0x4c, 0x8b, 0x64, 0xec, 0xea,      /* ...L.d.. */
    0x68, 0x3e, 0xbe, 0xb8, 0x4b, 0xc9, 0x33, 0x94,      /* h>..K.3. */
    0xdd, 0x68, 0xb6, 0x63, 0x20, 0x79, 0x9c, 0x71,      /* .h.c y.q */
    0x2a, 0x4f, 0xa3, 0x90, 0x07, 0xec, 0x4d, 0x7f,      /* *O....M. */
    0x3c, 0x96, 0x0f, 0x96, 0x72, 0x52, 0xb3, 0x4b,      /* <...rR.K */
    0x55, 0x69, 0xd8, 0x45, 0x0e, 0xce, 0x1a, 0xe6,      /* Ui.E.... */
    0x6d, 0x20, 0x38, 0xbb, 0xdf, 0xf0, 0x78, 0x21,      /* m 8...x! */
    0x02, 0x00, 0x21, 0xfe, 0x00, 0x3b,                  /* ..!..;   */
};


RES_CONST_DECL SSP_DATA_ENTRY CODE sspfile_u_gif[] = {
    { 0x80, 0, 166, 0, 0, _text0000 },
};
