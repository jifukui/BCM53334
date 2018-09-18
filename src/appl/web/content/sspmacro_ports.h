/*
 * $Id: sspmacro_ports.h,v 1.12.6.1 Broadcom SDK $
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

#ifndef _SSPMACRO_PORTS_H_
#define _SSPMACRO_PORTS_H_


#define SSPMACRO_PORTS_ALLPORTS          (0)    /* Assigned */
#define SSPMACRO_PORTS_COLUMNPORTS     (100)    /* Assigned */
#define SSPMACRO_PORTS_NOALIGN           (3)
#define SSPMACRO_PORTS_ONEROW        (5)
#define SSPMACRO_PORTS_LEFT_HALF        (24)
#define SSPMACRO_PORTS_RIGHT_HALF        (4)
#define SSPMACRO_PORTS_ROWSPAN       (1)
#define SSPMACRO_PORTS_NEWROW            (2)
#define SSPMACRO_PORTS_TWOROWS       (6)
#define SSPMACRO_PORTS_TOTAL_COUNT       (20)
#define SSPMACRO_PORTS_LINK              (7)
#define SSPMACRO_PORTS_FLOWCTRL          (8)
#define SSPMACRO_PORTS_PVID              (9)
#define SSPMACRO_PORTS_SPEED            (11)
#define SSPMACRO_PORTS_TYPE             (12)
#define SSPMACRO_PORTS_NUM              (13)
#define SSPMACRO_PORTS_ADMIN            (14)
#define SSPMACRO_PORTS_AUTONEGO         (15)
#define SSPMACRO_PORTS_PORTDESC		(16)
#define SSPMACRO_PORTS_PORTEN		(17)

#define MAX_PORTS_PER_ROW 28

void sspvar_ports_tag_status(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT;


#endif /* _SSPMACRO_PORTS_H_ */
