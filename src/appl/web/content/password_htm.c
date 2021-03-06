/*
 * $Id: password_htm.c,v 1.8 Broadcom SDK $
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
    0x34, 0x2e, 0x30, 0x31, 0x20, 0x54, 0x72, 0x61,      /* 4.01 Tra */
    0x6e, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x61,      /* nsitiona */
    0x6c, 0x2f, 0x2f, 0x45, 0x4e, 0x22, 0x3e, 0x0a,      /* l//EN">. */
    0x3c, 0x48, 0x54, 0x4d, 0x4c, 0x3e, 0x3c, 0x48,      /* <HTML><H */
    0x45, 0x41, 0x44, 0x3e, 0x0a, 0x3c, 0x54, 0x49,      /* EAD>.<TI */
    0x54, 0x4c, 0x45, 0x3e, 0x50, 0x41, 0x53, 0x53,      /* TLE>PASS */
    0x57, 0x4f, 0x52, 0x44, 0x3c, 0x2f, 0x54, 0x49,      /* WORD</TI */
    0x54, 0x4c, 0x45, 0x3e, 0x0a, 0x3c, 0x4d, 0x45,      /* TLE>.<ME */
    0x54, 0x41, 0x20, 0x68, 0x74, 0x74, 0x70, 0x2d,      /* TA http- */
    0x65, 0x71, 0x75, 0x69, 0x76, 0x3d, 0x22, 0x43,      /* equiv="C */
    0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x54,      /* ontent-T */
    0x79, 0x70, 0x65, 0x22, 0x20, 0x63, 0x6f, 0x6e,      /* ype" con */
    0x74, 0x65, 0x6e, 0x74, 0x3d, 0x22, 0x74, 0x65,      /* tent="te */
    0x78, 0x74, 0x2f, 0x68, 0x74, 0x6d, 0x6c, 0x3b,      /* xt/html; */
    0x20, 0x63, 0x68, 0x61, 0x72, 0x73, 0x65, 0x74,      /*  charset */
    0x3d, 0x55, 0x54, 0x46, 0x2d, 0x38, 0x22, 0x3e,      /* =UTF-8"> */
    0x20, 0x0a, 0x3c, 0x4c, 0x49, 0x4e, 0x4b, 0x20,      /*  .<LINK  */
    0x68, 0x72, 0x65, 0x66, 0x3d, 0x22, 0x66, 0x6f,      /* href="fo */
    0x72, 0x6d, 0x2e, 0x63, 0x73, 0x73, 0x22, 0x20,      /* rm.css"  */
    0x72, 0x65, 0x6c, 0x3d, 0x73, 0x74, 0x79, 0x6c,      /* rel=styl */
    0x65, 0x73, 0x68, 0x65, 0x65, 0x74, 0x3e, 0x0a,      /* esheet>. */
    0x3c, 0x53, 0x43, 0x52, 0x49, 0x50, 0x54, 0x20,      /* <SCRIPT  */
    0x6c, 0x61, 0x6e, 0x67, 0x75, 0x61, 0x67, 0x65,      /* language */
    0x3d, 0x4a, 0x61, 0x76, 0x61, 0x53, 0x63, 0x72,      /* =JavaScr */
    0x69, 0x70, 0x74, 0x20, 0x74, 0x79, 0x70, 0x65,      /* ipt type */
    0x3d, 0x22, 0x74, 0x65, 0x78, 0x74, 0x2f, 0x6a,      /* ="text/j */
    0x61, 0x76, 0x61, 0x73, 0x63, 0x72, 0x69, 0x70,      /* avascrip */
    0x74, 0x22, 0x3e, 0x0a, 0x66, 0x75, 0x6e, 0x63,      /* t">.func */
    0x74, 0x69, 0x6f, 0x6e, 0x20, 0x64, 0x69, 0x73,      /* tion dis */
    0x70, 0x6c, 0x61, 0x79, 0x5f, 0x68, 0x65, 0x6c,      /* play_hel */
    0x70, 0x28, 0x29, 0x0a, 0x7b, 0x0a, 0x20, 0x20,      /* p().{.   */
    0x77, 0x69, 0x6e, 0x64, 0x6f, 0x77, 0x2e, 0x6f,      /* window.o */
    0x70, 0x65, 0x6e, 0x28, 0x27, 0x70, 0x61, 0x73,      /* pen('pas */
    0x73, 0x77, 0x6f, 0x72, 0x64, 0x5f, 0x68, 0x65,      /* sword_he */
    0x6c, 0x70, 0x2e, 0x68, 0x74, 0x6d, 0x27, 0x2c,      /* lp.htm', */
    0x27, 0x4d, 0x79, 0x57, 0x69, 0x6e, 0x64, 0x6f,      /* 'MyWindo */
    0x77, 0x73, 0x27, 0x2c, 0x27, 0x73, 0x74, 0x61,      /* ws','sta */
    0x74, 0x75, 0x73, 0x3d, 0x79, 0x65, 0x73, 0x2c,      /* tus=yes, */
    0x6c, 0x65, 0x66, 0x74, 0x3d, 0x35, 0x30, 0x2c,      /* left=50, */
    0x74, 0x6f, 0x70, 0x3d, 0x35, 0x30, 0x2c, 0x73,      /* top=50,s */
    0x63, 0x72, 0x6f, 0x6c, 0x6c, 0x62, 0x61, 0x72,      /* crollbar */
    0x73, 0x3d, 0x79, 0x65, 0x73, 0x2c, 0x72, 0x65,      /* s=yes,re */
    0x73, 0x69, 0x7a, 0x61, 0x62, 0x6c, 0x65, 0x3d,      /* sizable= */
    0x79, 0x65, 0x73, 0x2c, 0x77, 0x69, 0x64, 0x74,      /* yes,widt */
    0x68, 0x3d, 0x36, 0x30, 0x30, 0x2c, 0x68, 0x65,      /* h=600,he */
    0x69, 0x67, 0x68, 0x74, 0x3d, 0x34, 0x30, 0x30,      /* ight=400 */
    0x27, 0x29, 0x3b, 0x0a, 0x7d, 0x0a, 0x3c, 0x2f,      /* ');.}.</ */
    0x53, 0x43, 0x52, 0x49, 0x50, 0x54, 0x3e, 0x0a,      /* SCRIPT>. */
    0x3c, 0x2f, 0x48, 0x45, 0x41, 0x44, 0x3e, 0x0a,      /* </HEAD>. */
    0x3c, 0x42, 0x4f, 0x44, 0x59, 0x3e, 0x0a, 0x0a,      /* <BODY>.. */
    0x3c, 0x54, 0x41, 0x42, 0x4c, 0x45, 0x20, 0x63,      /* <TABLE c */
    0x65, 0x6c, 0x6c, 0x53, 0x70, 0x61, 0x63, 0x69,      /* ellSpaci */
    0x6e, 0x67, 0x3d, 0x22, 0x30, 0x22, 0x20, 0x62,      /* ng="0" b */
    0x6f, 0x72, 0x64, 0x65, 0x72, 0x3d, 0x22, 0x30,      /* order="0 */
    0x22, 0x3e, 0x0a, 0x3c, 0x54, 0x52, 0x20, 0x76,      /* ">.<TR v */
    0x41, 0x6c, 0x69, 0x67, 0x6e, 0x3d, 0x22, 0x74,      /* Align="t */
    0x6f, 0x70, 0x22, 0x3e, 0x0a, 0x3c, 0x54, 0x44,      /* op">.<TD */
    0x20, 0x77, 0x69, 0x64, 0x74, 0x68, 0x3d, 0x22,      /*  width=" */
    0x39, 0x30, 0x25, 0x22, 0x3e, 0x3c, 0x48, 0x31,      /* 90%"><H1 */
    0x3e, 0x43, 0x68, 0x61, 0x6e, 0x67, 0x65, 0x20,      /* >Change  */
    0x50, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64,      /* Password */
    0x3c, 0x2f, 0x48, 0x31, 0x3e, 0x3c, 0x2f, 0x54,      /* </H1></T */
    0x44, 0x3e, 0x0a, 0x3c, 0x54, 0x44, 0x20, 0x61,      /* D>.<TD a */
    0x6c, 0x69, 0x67, 0x6e, 0x3d, 0x22, 0x72, 0x69,      /* lign="ri */
    0x67, 0x68, 0x74, 0x22, 0x3e, 0x3c, 0x49, 0x4e,      /* ght"><IN */
    0x50, 0x55, 0x54, 0x20, 0x74, 0x79, 0x70, 0x65,      /* PUT type */
    0x3d, 0x22, 0x62, 0x75, 0x74, 0x74, 0x6f, 0x6e,      /* ="button */
    0x22, 0x20, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x3d,      /* " value= */
    0x22, 0x48, 0x65, 0x6c, 0x70, 0x22, 0x20, 0x6f,      /* "Help" o */
    0x6e, 0x63, 0x6c, 0x69, 0x63, 0x6b, 0x3d, 0x22,      /* nclick=" */
    0x6a, 0x61, 0x76, 0x61, 0x73, 0x63, 0x72, 0x69,      /* javascri */
    0x70, 0x74, 0x3a, 0x64, 0x69, 0x73, 0x70, 0x6c,      /* pt:displ */
    0x61, 0x79, 0x5f, 0x68, 0x65, 0x6c, 0x70, 0x28,      /* ay_help( */
    0x29, 0x3b, 0x22, 0x3e, 0x3c, 0x2f, 0x54, 0x44,      /* );"></TD */
    0x3e, 0x0a, 0x3c, 0x2f, 0x54, 0x52, 0x3e, 0x0a,      /* >.</TR>. */
    0x3c, 0x2f, 0x54, 0x41, 0x42, 0x4c, 0x45, 0x3e,      /* </TABLE> */
    0x0a, 0x0a, 0x3c, 0x46, 0x4f, 0x52, 0x4d, 0x20,      /* ..<FORM  */
    0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x70, 0x77, 0x64,      /* name=pwd */
    0x20, 0x6d, 0x65, 0x74, 0x68, 0x6f, 0x64, 0x3d,      /*  method= */
    0x70, 0x6f, 0x73, 0x74, 0x20, 0x61, 0x63, 0x74,      /* post act */
    0x69, 0x6f, 0x6e, 0x3d, 0x22, 0x70, 0x61, 0x73,      /* ion="pas */
    0x73, 0x77, 0x6f, 0x72, 0x64, 0x2e, 0x63, 0x67,      /* sword.cg */
    0x69, 0x22, 0x3e, 0x0a, 0x3c, 0x54, 0x41, 0x42,      /* i">.<TAB */
    0x4c, 0x45, 0x3e, 0x0a, 0x20, 0x20, 0x3c, 0x54,      /* LE>.  <T */
    0x42, 0x4f, 0x44, 0x59, 0x3e, 0x0a, 0x20, 0x20,      /* BODY>.   */
    0x3c, 0x54, 0x52, 0x3e, 0x0a, 0x20, 0x20, 0x20,      /* <TR>.    */
    0x20, 0x3c, 0x54, 0x44, 0x3e, 0x3c, 0x62, 0x3e,      /*  <TD><b> */
    0x4f, 0x6c, 0x64, 0x20, 0x50, 0x61, 0x73, 0x73,      /* Old Pass */
    0x77, 0x6f, 0x72, 0x64, 0x3a, 0x20, 0x3c, 0x2f,      /* word: </ */
    0x62, 0x3e, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x3c,      /* b>.    < */
    0x54, 0x44, 0x3e, 0x3c, 0x49, 0x4e, 0x50, 0x55,      /* TD><INPU */
    0x54, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3d, 0x70,      /* T type=p */
    0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64, 0x20,      /* assword  */
    0x76, 0x61, 0x6c, 0x75, 0x65, 0x3d, 0x22, 0x22,      /* value="" */
    0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x6f, 0x6c,      /*  name=ol */
    0x64, 0x3e, 0x0a, 0x20, 0x20, 0x3c, 0x54, 0x52,      /* d>.  <TR */
    0x3e, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x54,      /* >.    <T */
    0x44, 0x3e, 0x3c, 0x62, 0x3e, 0x4e, 0x65, 0x77,      /* D><b>New */
    0x20, 0x50, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72,      /*  Passwor */
    0x64, 0x3a, 0x20, 0x3c, 0x2f, 0x62, 0x3e, 0x0a,      /* d: </b>. */
    0x20, 0x20, 0x20, 0x20, 0x3c, 0x54, 0x44, 0x3e,      /*     <TD> */
    0x3c, 0x49, 0x4e, 0x50, 0x55, 0x54, 0x20, 0x74,      /* <INPUT t */
    0x79, 0x70, 0x65, 0x3d, 0x70, 0x61, 0x73, 0x73,      /* ype=pass */
    0x77, 0x6f, 0x72, 0x64, 0x20, 0x76, 0x61, 0x6c,      /* word val */
    0x75, 0x65, 0x3d, 0x22, 0x22, 0x20, 0x6e, 0x61,      /* ue="" na */
    0x6d, 0x65, 0x3d, 0x6e, 0x65, 0x77, 0x3e, 0x0a,      /* me=new>. */
    0x20, 0x20, 0x3c, 0x54, 0x52, 0x3e, 0x0a, 0x20,      /*   <TR>.  */
    0x20, 0x20, 0x20, 0x3c, 0x54, 0x44, 0x3e, 0x3c,      /*    <TD>< */
    0x62, 0x3e, 0x43, 0x6f, 0x6e, 0x66, 0x69, 0x72,      /* b>Confir */
    0x6d, 0x20, 0x4e, 0x65, 0x77, 0x20, 0x50, 0x61,      /* m New Pa */
    0x73, 0x73, 0x77, 0x6f, 0x72, 0x64, 0x3a, 0x26,      /* ssword:& */
    0x6e, 0x62, 0x73, 0x70, 0x3b, 0x26, 0x6e, 0x62,      /* nbsp;&nb */
    0x73, 0x70, 0x3b, 0x26, 0x6e, 0x62, 0x73, 0x70,      /* sp;&nbsp */
    0x3b, 0x3c, 0x2f, 0x62, 0x3e, 0x0a, 0x20, 0x20,      /* ;</b>.   */
    0x20, 0x20, 0x3c, 0x54, 0x44, 0x3e, 0x3c, 0x49,      /*   <TD><I */
    0x4e, 0x50, 0x55, 0x54, 0x20, 0x74, 0x79, 0x70,      /* NPUT typ */
    0x65, 0x3d, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6f,      /* e=passwo */
    0x72, 0x64, 0x20, 0x76, 0x61, 0x6c, 0x75, 0x65,      /* rd value */
    0x3d, 0x22, 0x22, 0x20, 0x6e, 0x61, 0x6d, 0x65,      /* ="" name */
    0x3d, 0x63, 0x66, 0x6d, 0x3e, 0x0a, 0x20, 0x20,      /* =cfm>.   */
    0x3c, 0x54, 0x52, 0x3e, 0x0a, 0x20, 0x20, 0x20,      /* <TR>.    */
    0x20, 0x3c, 0x54, 0x44, 0x20, 0x63, 0x6f, 0x6c,      /*  <TD col */
    0x73, 0x70, 0x61, 0x6e, 0x3d, 0x32, 0x20, 0x61,      /* span=2 a */
    0x6c, 0x69, 0x67, 0x6e, 0x3d, 0x63, 0x65, 0x6e,      /* lign=cen */
    0x74, 0x65, 0x72, 0x3e, 0x26, 0x6e, 0x62, 0x73,      /* ter>&nbs */
    0x70, 0x3b, 0x0a, 0x20, 0x20, 0x3c, 0x54, 0x52,      /* p;.  <TR */
    0x3e, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x54,      /* >.    <T */
    0x44, 0x20, 0x63, 0x6f, 0x6c, 0x73, 0x70, 0x61,      /* D colspa */
    0x6e, 0x3d, 0x32, 0x20, 0x61, 0x6c, 0x69, 0x67,      /* n=2 alig */
    0x6e, 0x3d, 0x63, 0x65, 0x6e, 0x74, 0x65, 0x72,      /* n=center */
    0x3e, 0x3c, 0x49, 0x4e, 0x50, 0x55, 0x54, 0x20,      /* ><INPUT  */
    0x74, 0x79, 0x70, 0x65, 0x3d, 0x73, 0x75, 0x62,      /* type=sub */
    0x6d, 0x69, 0x74, 0x20, 0x76, 0x61, 0x6c, 0x75,      /* mit valu */
    0x65, 0x3d, 0x41, 0x70, 0x70, 0x6c, 0x79, 0x3e,      /* e=Apply> */
    0x0a, 0x20, 0x20, 0x3c, 0x2f, 0x54, 0x52, 0x3e,      /* .  </TR> */
    0x0a, 0x20, 0x20, 0x3c, 0x2f, 0x54, 0x42, 0x4f,      /* .  </TBO */
    0x44, 0x59, 0x3e, 0x0a, 0x3c, 0x2f, 0x54, 0x41,      /* DY>.</TA */
    0x42, 0x4c, 0x45, 0x3e, 0x0a, 0x3c, 0x2f, 0x46,      /* BLE>.</F */
    0x4f, 0x52, 0x4d, 0x3e, 0x0a, 0x3c, 0x2f, 0x42,      /* ORM>.</B */
    0x4f, 0x44, 0x59, 0x3e, 0x3c, 0x2f, 0x48, 0x54,      /* ODY></HT */
    0x4d, 0x4c, 0x3e, 0x0a,                              /* ML>.     */
};


RES_CONST_DECL SSP_DATA_ENTRY CODE sspfile_password_htm[] = {
    { 0x80, 0, 1164, 0, 0, _text0000 },
};
