/*
 * $Id: porthelp2_htm.c,v 1.4 Broadcom SDK $
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
    0x3c, 0x21, 0x44, 0x4f, 0x43, 0x54, 0x59, 0x50,      /* <!DOCTYP */
    0x45, 0x20, 0x48, 0x54, 0x4d, 0x4c, 0x20, 0x50,      /* E HTML P */
    0x55, 0x42, 0x4c, 0x49, 0x43, 0x20, 0x22, 0x2d,      /* UBLIC "- */
    0x2f, 0x2f, 0x57, 0x33, 0x43, 0x2f, 0x2f, 0x44,      /* //W3C//D */
    0x54, 0x44, 0x20, 0x48, 0x54, 0x4d, 0x4c, 0x20,      /* TD HTML  */
    0x34, 0x2e, 0x30, 0x20, 0x54, 0x72, 0x61, 0x6e,      /* 4.0 Tran */
    0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x61, 0x6c,      /* sitional */
    0x2f, 0x2f, 0x45, 0x4e, 0x22, 0x3e, 0x0a, 0x3c,      /* //EN">.< */
    0x48, 0x54, 0x4d, 0x4c, 0x3e, 0x3c, 0x48, 0x45,      /* HTML><HE */
    0x41, 0x44, 0x3e, 0x3c, 0x54, 0x49, 0x54, 0x4c,      /* AD><TITL */
    0x45, 0x3e, 0x3c, 0x2f, 0x54, 0x49, 0x54, 0x4c,      /* E></TITL */
    0x45, 0x3e, 0x0a, 0x3c, 0x4d, 0x45, 0x54, 0x41,      /* E>.<META */
    0x20, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74,      /*  content */
    0x3d, 0x22, 0x74, 0x65, 0x78, 0x74, 0x2f, 0x68,      /* ="text/h */
    0x74, 0x6d, 0x6c, 0x22, 0x20, 0x68, 0x74, 0x74,      /* tml" htt */
    0x70, 0x2d, 0x65, 0x71, 0x75, 0x69, 0x76, 0x3d,      /* p-equiv= */
    0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d,      /* Content- */
    0x54, 0x79, 0x70, 0x65, 0x3e, 0x0a, 0x3c, 0x4c,      /* Type>.<L */
    0x49, 0x4e, 0x4b, 0x20, 0x68, 0x72, 0x65, 0x66,      /* INK href */
    0x3d, 0x22, 0x66, 0x6f, 0x72, 0x6d, 0x2e, 0x63,      /* ="form.c */
    0x73, 0x73, 0x22, 0x20, 0x72, 0x65, 0x6c, 0x3d,      /* ss" rel= */
    0x73, 0x74, 0x79, 0x6c, 0x65, 0x73, 0x68, 0x65,      /* styleshe */
    0x65, 0x74, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3d,      /* et type= */
    0x74, 0x65, 0x78, 0x74, 0x2f, 0x63, 0x73, 0x73,      /* text/css */
    0x3e, 0x0a, 0x0a, 0x3c, 0x2f, 0x48, 0x45, 0x41,      /* >..</HEA */
    0x44, 0x3e, 0x0a, 0x3c, 0x42, 0x4f, 0x44, 0x59,      /* D>.<BODY */
    0x3e, 0x0a, 0x3c, 0x66, 0x6f, 0x6e, 0x74, 0x20,      /* >.<font  */
    0x73, 0x69, 0x7a, 0x65, 0x3d, 0x34, 0x3e, 0x3c,      /* size=4>< */
    0x42, 0x3e, 0x47, 0x65, 0x74, 0x74, 0x69, 0x6e,      /* B>Gettin */
    0x67, 0x20, 0x73, 0x74, 0x61, 0x72, 0x74, 0x65,      /* g starte */
    0x64, 0x20, 0x77, 0x69, 0x74, 0x68, 0x20, 0x50,      /* d with P */
    0x4f, 0x52, 0x54, 0x20, 0x63, 0x6f, 0x6e, 0x66,      /* ORT conf */
    0x69, 0x67, 0x75, 0x72, 0x61, 0x74, 0x69, 0x6f,      /* iguratio */
    0x6e, 0x3c, 0x2f, 0x42, 0x3e, 0x3c, 0x2f, 0x66,      /* n</B></f */
    0x6f, 0x6e, 0x74, 0x3e, 0x3c, 0x42, 0x52, 0x3e,      /* ont><BR> */
    0x3c, 0x42, 0x52, 0x3e, 0x0a, 0x54, 0x68, 0x65,      /* <BR>.The */
    0x20, 0x50, 0x6f, 0x72, 0x74, 0x20, 0x43, 0x6f,      /*  Port Co */
    0x6e, 0x66, 0x69, 0x67, 0x75, 0x72, 0x61, 0x74,      /* nfigurat */
    0x69, 0x6f, 0x6e, 0x20, 0x73, 0x63, 0x72, 0x65,      /* ion scre */
    0x65, 0x6e, 0x20, 0x64, 0x65, 0x66, 0x69, 0x6e,      /* en defin */
    0x65, 0x73, 0x20, 0x73, 0x70, 0x65, 0x65, 0x64,      /* es speed */
    0x2c, 0x20, 0x64, 0x75, 0x70, 0x6c, 0x65, 0x78,      /* , duplex */
    0x69, 0x6e, 0x67, 0x2c, 0x20, 0x61, 0x6e, 0x64,      /* ing, and */
    0x20, 0x66, 0x6c, 0x6f, 0x77, 0x20, 0x63, 0x6f,      /*  flow co */
    0x6e, 0x74, 0x72, 0x6f, 0x6c, 0x20, 0x6f, 0x70,      /* ntrol op */
    0x65, 0x72, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20,      /* eration  */
    0x66, 0x6f, 0x72, 0x20, 0x61, 0x20, 0x70, 0x6f,      /* for a po */
    0x72, 0x74, 0x20, 0x77, 0x68, 0x65, 0x6e, 0x20,      /* rt when  */
    0x61, 0x75, 0x74, 0x6f, 0x2d, 0x6e, 0x65, 0x67,      /* auto-neg */
    0x6f, 0x74, 0x69, 0x61, 0x74, 0x69, 0x6f, 0x6e,      /* otiation */
    0x20, 0x69, 0x73, 0x20, 0x6f, 0x66, 0x66, 0x2e,      /*  is off. */
    0x0a, 0x57, 0x68, 0x65, 0x6e, 0x20, 0x61, 0x75,      /* .When au */
    0x74, 0x6f, 0x2d, 0x6e, 0x65, 0x67, 0x6f, 0x74,      /* to-negot */
    0x69, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x69,      /* iation i */
    0x73, 0x20, 0x6f, 0x6e, 0x2c, 0x20, 0x74, 0x68,      /* s on, th */
    0x6f, 0x73, 0x65, 0x20, 0x64, 0x61, 0x74, 0x61,      /* ose data */
    0x20, 0x61, 0x72, 0x65, 0x20, 0x6e, 0x65, 0x67,      /*  are neg */
    0x6f, 0x74, 0x69, 0x61, 0x74, 0x65, 0x64, 0x20,      /* otiated  */
    0x66, 0x72, 0x6f, 0x6d, 0x20, 0x74, 0x68, 0x65,      /* from the */
    0x20, 0x6c, 0x69, 0x6e, 0x6b, 0x20, 0x70, 0x61,      /*  link pa */
    0x72, 0x74, 0x6e, 0x65, 0x72, 0x2e, 0x20, 0x4f,      /* rtner. O */
    0x74, 0x68, 0x65, 0x72, 0x77, 0x69, 0x73, 0x65,      /* therwise */
    0x2c, 0x20, 0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65,      /* , enable */
    0x20, 0x6f, 0x72, 0x20, 0x64, 0x69, 0x73, 0x61,      /*  or disa */
    0x62, 0x6c, 0x65, 0x20, 0x70, 0x6f, 0x72, 0x74,      /* ble port */
    0x73, 0x20, 0x74, 0x6f, 0x20, 0x63, 0x6f, 0x6e,      /* s to con */
    0x74, 0x72, 0x6f, 0x6c, 0x0a, 0x70, 0x61, 0x63,      /* trol.pac */
    0x6b, 0x65, 0x74, 0x20, 0x66, 0x6f, 0x72, 0x77,      /* ket forw */
    0x61, 0x72, 0x64, 0x69, 0x6e, 0x67, 0x2e, 0x3c,      /* arding.< */
    0x42, 0x52, 0x3e, 0x3c, 0x42, 0x52, 0x3e, 0x0a,      /* BR><BR>. */
    0x3c, 0x42, 0x3e, 0x50, 0x6f, 0x72, 0x74, 0x3c,      /* <B>Port< */
    0x2f, 0x42, 0x3e, 0x20, 0x73, 0x70, 0x65, 0x63,      /* /B> spec */
    0x69, 0x66, 0x69, 0x65, 0x73, 0x20, 0x74, 0x68,      /* ifies th */
    0x65, 0x20, 0x70, 0x6f, 0x72, 0x74, 0x20, 0x6e,      /* e port n */
    0x75, 0x6d, 0x62, 0x65, 0x72, 0x20, 0x74, 0x6f,      /* umber to */
    0x20, 0x63, 0x6f, 0x6e, 0x74, 0x72, 0x6f, 0x6c,      /*  control */
    0x2e, 0x3c, 0x42, 0x52, 0x3e, 0x3c, 0x42, 0x52,      /* .<BR><BR */
    0x3e, 0x0a, 0x3c, 0x42, 0x3e, 0x41, 0x64, 0x6d,      /* >.<B>Adm */
    0x69, 0x6e, 0x3c, 0x2f, 0x42, 0x3e, 0x20, 0x65,      /* in</B> e */
    0x6e, 0x61, 0x62, 0x6c, 0x65, 0x73, 0x20, 0x6f,      /* nables o */
    0x72, 0x20, 0x64, 0x69, 0x73, 0x61, 0x62, 0x6c,      /* r disabl */
    0x65, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x70,      /* es the p */
    0x6f, 0x72, 0x74, 0x2e, 0x3c, 0x42, 0x52, 0x3e,      /* ort.<BR> */
    0x3c, 0x42, 0x52, 0x3e, 0x0a, 0x3c, 0x42, 0x3e,      /* <BR>.<B> */
    0x41, 0x75, 0x74, 0x6f, 0x20, 0x4e, 0x65, 0x67,      /* Auto Neg */
    0x6f, 0x74, 0x69, 0x61, 0x74, 0x69, 0x6f, 0x6e,      /* otiation */
    0x3c, 0x2f, 0x42, 0x3e, 0x20, 0x65, 0x6e, 0x61,      /* </B> ena */
    0x62, 0x6c, 0x65, 0x73, 0x20, 0x6f, 0x72, 0x20,      /* bles or  */
    0x64, 0x69, 0x73, 0x61, 0x62, 0x6c, 0x65, 0x73,      /* disables */
    0x20, 0x61, 0x75, 0x74, 0x6f, 0x2d, 0x6e, 0x65,      /*  auto-ne */
    0x67, 0x6f, 0x74, 0x69, 0x61, 0x74, 0x69, 0x6f,      /* gotiatio */
    0x6e, 0x2e, 0x20, 0x0a, 0x57, 0x68, 0x65, 0x6e,      /* n. .When */
    0x20, 0x61, 0x75, 0x74, 0x6f, 0x2d, 0x6e, 0x65,      /*  auto-ne */
    0x67, 0x6f, 0x74, 0x69, 0x61, 0x74, 0x69, 0x6f,      /* gotiatio */
    0x6e, 0x20, 0x69, 0x73, 0x20, 0x65, 0x6e, 0x61,      /* n is ena */
    0x62, 0x6c, 0x65, 0x64, 0x2c, 0x20, 0x74, 0x68,      /* bled, th */
    0x65, 0x20, 0x70, 0x6f, 0x72, 0x74, 0x20, 0x6e,      /* e port n */
    0x65, 0x67, 0x6f, 0x74, 0x69, 0x61, 0x74, 0x65,      /* egotiate */
    0x73, 0x20, 0x77, 0x69, 0x74, 0x68, 0x20, 0x74,      /* s with t */
    0x68, 0x65, 0x20, 0x6c, 0x69, 0x6e, 0x6b, 0x20,      /* he link  */
    0x70, 0x61, 0x72, 0x74, 0x6e, 0x65, 0x72, 0x20,      /* partner  */
    0x61, 0x6e, 0x64, 0x20, 0x0a, 0x77, 0x6f, 0x72,      /* and .wor */
    0x6b, 0x73, 0x20, 0x6f, 0x75, 0x74, 0x20, 0x73,      /* ks out s */
    0x70, 0x65, 0x65, 0x64, 0x2c, 0x20, 0x64, 0x75,      /* peed, du */
    0x70, 0x6c, 0x65, 0x78, 0x20, 0x6f, 0x70, 0x65,      /* plex ope */
    0x72, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x2c, 0x20,      /* ration,  */
    0x61, 0x6e, 0x64, 0x20, 0x66, 0x6c, 0x6f, 0x77,      /* and flow */
    0x20, 0x63, 0x6f, 0x6e, 0x74, 0x72, 0x6f, 0x6c,      /*  control */
    0x2e, 0x20, 0x57, 0x68, 0x65, 0x6e, 0x20, 0x61,      /* . When a */
    0x75, 0x74, 0x6f, 0x2d, 0x6e, 0x65, 0x67, 0x6f,      /* uto-nego */
    0x74, 0x69, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20,      /* tiation  */
    0x69, 0x73, 0x20, 0x64, 0x69, 0x73, 0x61, 0x62,      /* is disab */
    0x6c, 0x65, 0x64, 0x2c, 0x20, 0x0a, 0x70, 0x6f,      /* led, .po */
    0x72, 0x74, 0x20, 0x73, 0x70, 0x65, 0x65, 0x64,      /* rt speed */
    0x2c, 0x20, 0x64, 0x75, 0x70, 0x6c, 0x65, 0x78,      /* , duplex */
    0x20, 0x6f, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69,      /*  operati */
    0x6f, 0x6e, 0x2c, 0x20, 0x61, 0x6e, 0x64, 0x20,      /* on, and  */
    0x66, 0x6c, 0x6f, 0x77, 0x20, 0x63, 0x6f, 0x6e,      /* flow con */
    0x74, 0x72, 0x6f, 0x6c, 0x20, 0x69, 0x73, 0x20,      /* trol is  */
    0x70, 0x72, 0x6f, 0x67, 0x72, 0x61, 0x6d, 0x6d,      /* programm */
    0x61, 0x62, 0x6c, 0x65, 0x20, 0x62, 0x79, 0x20,      /* able by  */
    0x74, 0x68, 0x65, 0x20, 0x75, 0x73, 0x65, 0x72,      /* the user */
    0x2e, 0x3c, 0x42, 0x52, 0x3e, 0x3c, 0x42, 0x52,      /* .<BR><BR */
    0x3e, 0x0a, 0x3c, 0x42, 0x3e, 0x44, 0x75, 0x70,      /* >.<B>Dup */
    0x6c, 0x65, 0x78, 0x20, 0x53, 0x70, 0x65, 0x65,      /* lex Spee */
    0x64, 0x3c, 0x2f, 0x42, 0x3e, 0x20, 0x63, 0x6f,      /* d</B> co */
    0x6e, 0x74, 0x72, 0x6f, 0x6c, 0x73, 0x20, 0x64,      /* ntrols d */
    0x75, 0x70, 0x6c, 0x65, 0x78, 0x20, 0x73, 0x70,      /* uplex sp */
    0x65, 0x65, 0x64, 0x20, 0x66, 0x6f, 0x72, 0x20,      /* eed for  */
    0x74, 0x68, 0x65, 0x20, 0x70, 0x6f, 0x72, 0x74,      /* the port */
    0x2e, 0x3c, 0x42, 0x52, 0x3e, 0x3c, 0x42, 0x52,      /* .<BR><BR */
    0x3e, 0x0a, 0x3c, 0x42, 0x3e, 0x50, 0x56, 0x49,      /* >.<B>PVI */
    0x44, 0x3c, 0x2f, 0x42, 0x3e, 0x20, 0x61, 0x73,      /* D</B> as */
    0x73, 0x69, 0x67, 0x6e, 0x73, 0x20, 0x56, 0x4c,      /* signs VL */
    0x41, 0x4e, 0x20, 0x49, 0x44, 0x20, 0x6f, 0x66,      /* AN ID of */
    0x20, 0x70, 0x61, 0x63, 0x6b, 0x65, 0x74, 0x20,      /*  packet  */
    0x77, 0x68, 0x65, 0x6e, 0x20, 0x70, 0x61, 0x63,      /* when pac */
    0x6b, 0x65, 0x74, 0x73, 0x20, 0x61, 0x72, 0x72,      /* kets arr */
    0x69, 0x76, 0x65, 0x20, 0x61, 0x74, 0x20, 0x74,      /* ive at t */
    0x68, 0x65, 0x20, 0x70, 0x6f, 0x72, 0x74, 0x20,      /* he port  */
    0x66, 0x6f, 0x72, 0x20, 0x75, 0x6e, 0x74, 0x61,      /* for unta */
    0x67, 0x20, 0x70, 0x61, 0x63, 0x6b, 0x65, 0x74,      /* g packet */
    0x20, 0x6f, 0x72, 0x20, 0x70, 0x72, 0x69, 0x6f,      /*  or prio */
    0x72, 0x69, 0x74, 0x79, 0x20, 0x74, 0x61, 0x67,      /* rity tag */
    0x20, 0x70, 0x61, 0x63, 0x6b, 0x65, 0x74, 0x2e,      /*  packet. */
    0x3c, 0x42, 0x52, 0x3e, 0x3c, 0x42, 0x52, 0x3e,      /* <BR><BR> */
    0x0a, 0x3c, 0x2f, 0x42, 0x4f, 0x44, 0x59, 0x3e,      /* .</BODY> */
    0x3c, 0x2f, 0x48, 0x54, 0x4d, 0x4c, 0x3e, 0x0a,      /* </HTML>. */
};


RES_CONST_DECL SSP_DATA_ENTRY CODE sspfile_porthelp2_htm[] = {
    { 0x80, 0, 1184, 0, 0, _text0000 },
};
