/*
 * $Id: feature.c,v 1.14 Broadcom SDK $
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

#include "appl/ssp.h"
#include "../content/sspmacro_feature.h"


#if (CFG_UIP_IPV6_ENABLED)
#define IPV6_SUPPORT
#endif  /* CFG_UIP_IPV6_ENABLED */

#if (CFG_PERSISTENCE_SUPPORT_ENABLED)
#define PERSISTENCE_SUPPORT
#endif  /* CFG_PERSISTENCE_SUPPORT_ENABLED */

SSPLOOP_RETVAL 
ssploop_feature_tag_enable(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem) REENTRANT
{
    UNREFERENCED_PARAMETER(psmem);

    if (index) { 
        /* already shown this feature */
        return SSPLOOP_STOP;
    }

    /*
     * If feature available, set proceed=1.
     * Else remain proceed = 0.
     */
    switch (params[0]) {
        case SSPMACRO_FEATURE_MCAST:
#ifndef CFG_SWITCH_MCAST_INCLUDED
        return SSPLOOP_STOP;
#endif
        break;
        case SSPMACRO_FEATURE_STAT:
#ifndef CFG_SWITCH_STAT_INCLUDED
        return SSPLOOP_STOP; 
#endif
        break;
        
        case SSPMACRO_FEATURE_PVID:
/* PVID is always shown and can be set on web page */
        break;

        case SSPMACRO_FEATURE_BONJOUR:
#ifndef CFG_ZEROCONF_MDNS_INCLUDED
            return SSPLOOP_STOP;
#endif  /*  BONJOUR_SUPPORT */
            break;
        case SSPMACRO_FEATURE_PERSISTENCE:
#ifndef PERSISTENCE_SUPPORT
            return SSPLOOP_STOP;
#endif  /* PERSISTENCE_SUPPORT */
            break;

        case SSPMACRO_FEATURE_IPV6:
#ifndef IPV6_SUPPORT
            return SSPLOOP_STOP;
#endif  /* IPV6_SUPPORT */
            break;

        case SSPMACRO_FEATURE_PVLAN:
#if !defined(CFG_SWITCH_PVLAN_INCLUDED)
            return SSPLOOP_STOP;
#endif  /* defined(CFG_SWITCH_PVLAN_INCLUDED) */
            break;

        case SSPMACRO_FEATURE_TRUNK:
#ifndef CFG_SWITCH_LAG_INCLUDED
            return SSPLOOP_STOP;
#endif /* CFG_SWITCH_LAG_INCLUDED */
            break;

        case SSPMACRO_FEATURE_CABLE:
#ifndef CFG_HW_CABLE_DIAG_INCLUDED
            return SSPLOOP_STOP;
#endif /* CFG_HW_CABLE_DIAG_INCLUDED */
            if (board_cable_diag_port_count() == 0) {
                return SSPLOOP_STOP;
            }
            break;

        case SSPMACRO_FEATURE_MIRROR:
#ifndef CFG_SWITCH_MIRROR_INCLUDED
            return SSPLOOP_STOP;
#endif /* CFG_SWITCH_MIRROR_INCLUDED */
            break;
        
        case SSPMACRO_FEATURE_LOOPDET:
#ifndef CFG_SWITCH_LOOPDETECT_INCLUDED
            return SSPLOOP_STOP;
#endif /* CFG_SWITCH_LOOPDETECT_INCLUDED */
            break;

        case SSPMACRO_FEATURE_RATE:
#ifndef CFG_SWITCH_RATE_INCLUDED
            return SSPLOOP_STOP;
#endif /* CFG_SWITCH_RATE_INCLUDED */
            break;

        case SSPMACRO_FEATURE_QOS:
#ifndef CFG_SWITCH_QOS_INCLUDED
            return SSPLOOP_STOP;
#endif /* CFG_SWITCH_QOS_INCLUDED */
            break;

        case SSPMACRO_FEATURE_UPLOADVC:
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
            if (board_upload_vc == 0) {
                return SSPLOOP_STOP;
            }
#else
            return SSPLOOP_STOP;
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
            break;

        default:
            return SSPLOOP_STOP;
    }

    return SSPLOOP_PROCEED;
}


