/*
 * $Id: qoscbk.c,v 1.15 Broadcom SDK $
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
#include "appl/persistence.h"
#include "../content/sspmacro_qos.h"
#include "../content/qos_htm.h"
#include "../content/pqos_htm.h"

SSPLOOP_RETVAL ssploop_tbl_tag_qos(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem);

#define QOS_DEBUG   0

#if QOS_DEBUG
#define QOS_DBG(x)    do { sal_printf("QOS: "); sal_printf x; sal_printf("\n"); } while(0);
#else
#define QOS_DBG(x)
#endif

SSP_HANDLER_RETVAL ssphandler_qos_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem);
void sspvar_qos_tag_info(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem);

#ifdef CFG_SWITCH_QOS_INCLUDED
#define MAX_QUEUE_NUM 4
#define MAX_WEIGHT 8

typedef enum bcm_qos_type_s {
    BCM_QT_DOT1P_PRIORITY, /*html default value 0 is 802.1p*/
    BCM_QT_PORT_BASED,
    BCM_QT_COUNT
} bcm_qos_type_t;

typedef enum bcm_cosq_schedule_e {
    BCM_COSQ_WEIGHTED_ROUND_ROBIN,
} bcm_cosq_schedule_t;

const char SINGLE_SCH[] = "false";
const char THRESHOLD_MODE[] = "false";
const char FIX_WEIGHT[] = "false";
const char queue_priority_low[] = "&nbsp;&nbsp;(Low)";
const char queue_priority_high[] = "&nbsp;&nbsp;(High)";
#define MAX_MODE_NUM   1

static char *mode_str[] = {"Weighted Round Robin"};
static int mode_val[] = {BCM_COSQ_WEIGHTED_ROUND_ROBIN};
static int current_queue_num = 4;

static int
get_current_queue_num(SSP_PSMH psmem)
{
    int *pn;

    pn = ssputil_psmem_get(psmem, sspvar_qos_tag_info);
    if (pn == NULL) {
        /* Not yet called */
        pn = (int *)ssputil_psmem_alloc(psmem, sspvar_qos_tag_info, sizeof(int));

        *pn = current_queue_num;
    }

    return *pn;
}
#endif /* CFG_SWITCH_QOS_INCLUDED */

void
sspvar_qos_tag_info(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_QOS_INCLUDED
    int i, mode = BCM_COSQ_WEIGHTED_ROUND_ROBIN, weights[MAX_QUEUE_NUM] = {1,2,4,8};
    char *pbuf;
    int *p;
    qos_type_t qos_type;

    switch (params[0]) {
        case SSPMACRO_QOS_VALUES:
            pbuf = ssputil_shared_buffer;
            /*deafault value*/
            sal_sprintf(pbuf," 1, 0, 0, 1, 2, 2, 3, 3");

            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            break;
        case SSPMACRO_QOS_SCH:
            ret->type = SSPVAR_RET_STRING;
            sal_strcpy(ssputil_shared_buffer, SINGLE_SCH);
            ret->val_data.string = ssputil_shared_buffer;
            break;
        case SSPMACRO_QOS_THRES:
            ret->type = SSPVAR_RET_STRING;
            sal_strcpy(ssputil_shared_buffer, THRESHOLD_MODE);
            ret->val_data.string = ssputil_shared_buffer;
            break;
        case SSPMACRO_QOS_MAXWEIGHT:
            ret->type = SSPVAR_RET_INTEGER;
            ret->val_data.integer = MAX_WEIGHT;
            break;
        case SSPMACRO_QOS_WEIGHT:
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = (char *)FIX_WEIGHT;
            break;
        case SSPMACRO_QOS_WEIGHTS:
            pbuf = ssputil_shared_buffer;

            for (i=0; i<get_current_queue_num(psmem); i++) {

                sal_sprintf(pbuf, "%u", weights[i]);
                pbuf += sal_strlen(pbuf);
                if (i < get_current_queue_num(psmem) - 1) {
                    *(pbuf++) = ',';
                }
            }
            *pbuf = 0;

            /*
             * Special case: even if one queue,
             * we still need to give it two values
             * because new Array(1) not working in javascript.
             */
            if (get_current_queue_num(psmem) == 1) {
                sal_strcat(ssputil_shared_buffer, ",0");
            }

            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            break;
        case SSPMACRO_QOS_QUEUE:
            ret->type = SSPVAR_RET_INTEGER;
            ret->val_data.integer = params[1];
            break;

        case SSPMACRO_QOS_ORDER:
            /* Don't show (low)/(high) if only one queue */
            if (get_current_queue_num(psmem) == 1) {
                ret->type = SSPVAR_RET_NULL;
                return;
            }

            ret->type = SSPVAR_RET_STRING;
            if(params[1] == 0) {
                sal_strcpy(ssputil_shared_buffer, queue_priority_low);
            }

            else if(params[1] == get_current_queue_num(psmem) - 1) {
                sal_strcpy(ssputil_shared_buffer, queue_priority_high);
            }
            else {
                sal_strcpy(ssputil_shared_buffer, "");
            }
            ret->val_data.string = ssputil_shared_buffer;
            break;

        case SSPMACRO_QOS_MODE_VAL:
            ret->type = SSPVAR_RET_INTEGER;
            ret->val_data.integer = mode_val[params[1]];
            return;

        case SSPMACRO_QOS_MODE_SEL:
            p = (int *)ssputil_psmem_get(psmem, ssphandler_qos_cgi);
            if (p == NULL) {
               /*default value:wrr*/
                mode = BCM_COSQ_WEIGHTED_ROUND_ROBIN;
            }
            else
                mode = *p;

            if (mode == (mode_val[params[1]]))
              sal_strcpy(ssputil_shared_buffer, "selected");
            else
              sal_strcpy(ssputil_shared_buffer, "");

            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            return;

        case SSPMACRO_QOS_MODE_STRING:
            sal_strcpy(ssputil_shared_buffer, mode_str[params[1]]);
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            return;

        case SSPMACRO_QOS_QNUM:
            ret->type = SSPVAR_RET_INTEGER;
            ret->val_data.integer = get_current_queue_num(psmem);
            return;

        case SSPMACRO_QOS_NUMBER:
            ret->type = SSPVAR_RET_INTEGER;
            ret->val_data.integer = (int)params[1] + 1;
            return;

        case SSPMACRO_QOS_TYPE:
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            board_qos_type_get(&qos_type);
            QOS_DBG(("wss_vlanmgr_qostype_get = %d\n", i));
            
            if (qos_type == QT_PORT_BASED) {
                sal_sprintf(ssputil_shared_buffer, "port-based");
            } else if(qos_type == QT_DOT1P_PRIORITY){
                sal_sprintf(ssputil_shared_buffer, "8021p");
            }
          break;

        default:
            QOS_DBG(("**** DEFAULT in sspvar_qos_tag_info : params[0]=%d", params[0]));
        }
    return;
#endif /* #ifdef CFG_SWITCH_QOS_INCLUDED */
}

SSPLOOP_RETVAL
ssploop_qos_tag_mode_counts(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_QOS_INCLUDED
    if (index < MAX_MODE_NUM){
        return SSPLOOP_PROCEED;
    }
#endif /* CFG_SWITCH_QOS_INCLUDED */

    return SSPLOOP_STOP;
}

SSPLOOP_RETVAL ssploop_tbl_tag_qos(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_QOS_INCLUDED
    if (index < get_current_queue_num(psmem)) {
        return SSPLOOP_PROCEED;
    }
#endif /* CFG_SWITCH_QOS_INCLUDED */

    return SSPLOOP_STOP;
}

SSP_HANDLER_RETVAL
ssphandler_swqospage_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_QOS_INCLUDED
    qos_type_t qos_type;
    uint16 uport;

    QOS_DBG(("ssphandler_swqospage_cgi : qos_type = %d\n", sal_atoi(cxt->pairs[0].value)));
    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {
        qos_type = sal_atoi(cxt->pairs[0].value);
        if (qos_type == (qos_type_t)BCM_QT_DOT1P_PRIORITY) {
            /* Show QOS page */
            board_qos_type_set(QT_DOT1P_PRIORITY);
            cxt->page = sspfile_qos_htm;
        } else {
            /* Show PQOS page */
            board_qos_type_set(QT_PORT_BASED);
            SAL_UPORT_ITER(uport) {
                board_untagged_priority_set(uport, 1); /*default port-base qos:priority is low*/
            }
            cxt->page = sspfile_pqos_htm;
        }
        cxt->flags= 0;

#if CFG_PERSISTENCE_SUPPORT_ENABLED
        persistence_save_current_settings("qos");
#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */
    }
    return SSP_HANDLER_RET_MODIFIED;
#else
    return SSP_HANDLER_RET_INTACT;
#endif /* CFG_SWITCH_QOS_INCLUDED */
}


SSP_HANDLER_RETVAL
ssphandler_qos_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
    if (cxt->type != SSP_HANDLER_QUERY_STRINGS) {
        return SSP_HANDLER_RET_INTACT;
    }

    return SSP_HANDLER_RET_INTACT;
}

SSPLOOP_RETVAL
ssploop_qos_tag_show(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{

    if (index) {
        return SSPLOOP_STOP;
    }

    /*
     * If feature available, set proceed=1.
     * Else remain proceed = 0.
     */
    switch (params[0]) {
        case SSPMACRO_QOS_QNUM:
            return SSPLOOP_STOP;
            break;

        default:
            break;
    }

    return SSPLOOP_PROCEED;
}

SSPLOOP_RETVAL
ssploop_qos_tag_maxqloop(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_QOS_INCLUDED
    if (index < MAX_QUEUE_NUM) {
        return SSPLOOP_PROCEED;
    }
#endif /* CFG_SWITCH_QOS_INCLUDED */

    return SSPLOOP_STOP;
}

SSPLOOP_RETVAL
ssploop_qos_tag_weights(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_QOS_INCLUDED
    if (index < MAX_WEIGHT) {
        return SSPLOOP_PROCEED;
    }
#endif /* CFG_SWITCH_QOS_INCLUDED */

    return SSPLOOP_STOP;
}

