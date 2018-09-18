/*
 * $Id: statcbk.c,v 1.9 Broadcom SDK $
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
#if defined(CFG_SWITCH_STAT_INCLUDED)

#include "utilcbk.h"
#include "../content/sspmacro_ports.h"
#include "../content/sspmacro_stat.h"
#include "../content/errormsg_htm.h"

#define COUNTER_DEBUG 0

#if COUNTER_DEBUG
#define COUNTER_DBG(x)    do { sal_printf("COUNTER: "); \
                       sal_printf x; sal_printf("\n");\
                       } while(0);
#else
#define COUNTER_DBG(x)
#endif

#define MAX_PORT_NUM board_uport_count()
#define NUM_PORTS_PER_SYSTEM MAX_PORT_NUM
#define ODD_PORT_NUM ((MAX_PORT_NUM % 2) == 1)


static SSP_HANDLER_RETVAL
show_error(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem, int error) {
    webutil_show_error(
        cxt, psmem,
        "STATISTICS",
        "system error information!!",
        err_button_retry,
        err_action_back
        );
    /* We don't want to process it more */
    /* cxt->flags = 0; */
    return SSP_HANDLER_RET_MODIFIED;
}


void sspvar_stat_tag_ssi(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
    int uport, uport2;
    uint32 val = 0;
    port_stat_t stat, stat2;

    /* user port */
    uport = SAL_ZUPORT_TO_UPORT(params[1]);
    uport2 = uport + ((NUM_PORTS_PER_SYSTEM+1)/2);

    ret->type = SSPVAR_RET_STRING;
    ret->val_data.string = ssputil_shared_buffer;
    sal_memset(ssputil_shared_buffer, 0, sizeof(ssputil_shared_buffer));

    sal_memset(&stat, 0, sizeof(port_stat_t));
    sal_memset(&stat2, 0, sizeof(port_stat_t));

    board_port_stat_get((uint16) uport, &stat);
    board_port_stat_get((uint16) uport2, &stat2);

    switch (params[0]) {
        case SSPMACRO_STAT_TX1:
            val = stat.TxOctets_lo;
            sal_sprintf(ssputil_shared_buffer, "%u", val);
            COUNTER_DBG(("val:%d val_u:%u uport:%d\n", val, val, SAL_UPORT_TO_NZUPORT(uport)));
        break;

        case SSPMACRO_STAT_RX1:
            val = stat.RxOctets_lo;
            sal_sprintf(ssputil_shared_buffer, "%u", val);
            COUNTER_DBG(("val:%d val_u:%u uport:%d\n", val, val,  SAL_UPORT_TO_NZUPORT(uport)));
        break;

        case SSPMACRO_STAT_TX2:
            if (ODD_PORT_NUM && (SAL_UPORT_TO_NZUPORT(uport) == (NUM_PORTS_PER_SYSTEM+1)/2)) {
                sal_strcpy(ssputil_shared_buffer, "&nbsp;");
            } else {
                val = stat2.TxOctets_lo;
                sal_sprintf(ssputil_shared_buffer, "%u", val);
            }
            COUNTER_DBG(("val:%d val_u:%u uport:%d\n", val, val, SAL_UPORT_TO_NZUPORT(uport2)));
        break;

        case SSPMACRO_STAT_RX2:
            if (ODD_PORT_NUM && (SAL_UPORT_TO_NZUPORT(uport) == (NUM_PORTS_PER_SYSTEM+1)/2)) {
                sal_strcpy(ssputil_shared_buffer, "&nbsp;");
            } else {
                val = stat2.RxOctets_lo;
                sal_sprintf(ssputil_shared_buffer, "%u", val);
            }
            COUNTER_DBG(("val:%d val_u:%u uport:%d\n", val, val, SAL_UPORT_TO_NZUPORT(uport2)));
            break;
    }
}

SSP_HANDLER_RETVAL
ssphandler_counters_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    int *p;

    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {

        p = (int *)ssputil_psmem_alloc(psmem,
                                       ssphandler_counters_cgi,
                                       sizeof(int));
        *p = sal_atoi(cxt->pairs[0].value);
        COUNTER_DBG(("port_num = %d\n", *p));
    }
    return SSP_HANDLER_RET_INTACT;
}

SSP_HANDLER_RETVAL
ssphandler_clear_counter_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    sys_error_t rv = SYS_OK;

    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {
        if (cxt->count > 0){    /* To prevent using this URL directly */
            /* ONLY support clear all counters now */
            if (!sal_strcmp(cxt->pairs[0].value, "all")) {
                /* clear all counters */
                rv = board_port_stat_clear_all();
                if(rv != SYS_OK){
                    return show_error(cxt, psmem, rv);
                }
            }
        }
    }

    return SSP_HANDLER_RET_INTACT;
}
#endif
