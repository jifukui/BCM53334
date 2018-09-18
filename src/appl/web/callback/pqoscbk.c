/*
 * $Id: pqoscbk.c,v 1.11 Broadcom SDK $
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
#include "../content/sspmacro_pqos.h"

SSPLOOP_RETVAL ssploop_tbl_tag_pqos(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem);

#define PQOS_DEBUG   0

#if PQOS_DEBUG
#define PQOS_DBG(x)    do { sal_printf("PQOS: "); sal_printf x; sal_printf("\n"); } while(0);
#else
#define PQOS_DBG(x)
#endif

SSP_HANDLER_RETVAL ssphandler_pqos_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem);
void sspvar_pqos_tag_info(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem);

#ifdef CFG_SWITCH_QOS_INCLUDED
#define MAX_PRIORITY_NUM 8
#define MAX_QUEUE_NUM 4
#define MAX_WEIGHT 8

typedef enum bcm_cosq_schedule_e {
    BCM_COSQ_WEIGHTED_ROUND_ROBIN,
} bcm_cosq_schedule_t;


#define MAX_MODE_NUM   1
static char *mode_str[] = {"Weighted Round Robin"};
static int mode_val[] = {BCM_COSQ_WEIGHTED_ROUND_ROBIN};


#ifdef QOS_ENHANCE
static char *enhance_mode_str[] = {"Disabled",  "DSCP Based QoS", "ToS Based QoS", "Port Based QoS", "IP Address Based QoS"};
static int enhance_mode_val[] = {ENHANCE_MODE_DISABLED, ENHANCE_MODE_DSCP_BASED_QOS,
                                      ENHANCE_MODE_TOS_BASED_QOS, ENHANCE_MODE_PORT_BASED_QOS, ENHANCE_MODE_IP_BASED_QOS};

#ifdef BCM_ESW_SUPPORT
static int enhance_mode_min_val[] = {ENHANCE_MODE_MIN_DISABLED, ENHANCE_MODE_MIN_DSCP_BASED_QOS,
                                      ENHANCE_MODE_MIN_TOS_BASED_QOS};
#endif
#endif

static int current_queue_num = 4;

static int
get_current_queue_num(SSP_PSMH psmem)
{
    int *pn;
    pn = ssputil_psmem_get(psmem, sspvar_pqos_tag_info);
    if (pn == NULL) {
        /* Not yet called */
        pn = (int *)ssputil_psmem_alloc(psmem, sspvar_pqos_tag_info, sizeof(int));
        *pn = current_queue_num;
    }

    return *pn;
}
#endif /* CFG_SWITCH_QOS_INCLUDED */

void
sspvar_pqos_tag_info(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_QOS_INCLUDED
    int mode = BCM_COSQ_WEIGHTED_ROUND_ROBIN;
    char *pbuf;
    int *p;
    uint16 i = 0;
    uint8 que_pri = 0;
    uint8 pri2que_map[MAX_PRIORITY_NUM]={ 1, 0, 0, 1, 2, 2, 3, 3}; /*default value*/

    switch (params[0]) {
        case SSPMACRO_PQOS_VALUES:
            pbuf = ssputil_shared_buffer;
            board_qos_type_set(QT_PORT_BASED);

            SAL_UPORT_ITER(i) {
                board_untagged_priority_get(i, &que_pri);

                PQOS_DBG(("SSPMACRO_PQOS_VALUES: que_pri=%d", que_pri));

                *pbuf = pri2que_map[que_pri]+48;  /*To transfer digital to character*/
                PQOS_DBG(("1SSPMACRO_PQOS_VALUES: pbuf=%s", ssputil_shared_buffer));
                pbuf++;

                *pbuf  = ',';
                pbuf++;
                PQOS_DBG(("3SSPMACRO_PQOS_VALUES: pbuf=%s", ssputil_shared_buffer));
            }
            *(--pbuf)='\0';

            PQOS_DBG(("4SSPMACRO_PQOS_VALUES: pbuf=%s", ssputil_shared_buffer));
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            break;

        case SSPMACRO_PQOS_QUEUE:
            ret->type = SSPVAR_RET_INTEGER;
            ret->val_data.integer = params[1];
            break;

        case SSPMACRO_PQOS_ORDER:

            /* Don't show (low)/(high) if only one queue */
            if (get_current_queue_num(psmem) == 1) {
                ret->type = SSPVAR_RET_NULL;
                return;
            }

            ret->type = SSPVAR_RET_STRING;
            if(params[1] == 0) {
                sal_strcpy(ssputil_shared_buffer, "&nbsp;&nbsp;(Low)");
            }else if(params[1] == get_current_queue_num(psmem) - 1) {
                sal_strcpy(ssputil_shared_buffer, "&nbsp;&nbsp;(High)");
            }else {
                sal_strcpy(ssputil_shared_buffer, "");
            }
            ret->val_data.string = ssputil_shared_buffer;
            break;

        case SSPMACRO_PQOS_MODE_STRING:
            sal_strcpy(ssputil_shared_buffer, mode_str[params[1]]);
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            return;

        case SSPMACRO_PQOS_MODE_VAL:
            ret->type = SSPVAR_RET_INTEGER;
            ret->val_data.integer = mode_val[params[1]];
            return;

        case SSPMACRO_PQOS_MODE_SEL:
            p = (int *)ssputil_psmem_get(psmem, ssphandler_pqos_cgi);
            if (p == NULL) {
                sal_strcpy(ssputil_shared_buffer, "");
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

        case SSPMACRO_PQOS_NUMBER:
            ret->type = SSPVAR_RET_INTEGER;
            ret->val_data.integer = (int)params[1] + 1;
            return;

        default:
            PQOS_DBG(("**** DEFAULT in sspvar_pqos_tag_info : params[0]=%d", params[0]));
        }
    return;
#endif /* CFG_SWITCH_QOS_INCLUDED */
}

SSPLOOP_RETVAL ssploop_tbl_tag_pqos(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_QOS_INCLUDED
    if (index < get_current_queue_num(psmem)) {
        return SSPLOOP_PROCEED;
    }
#endif /* CFG_SWITCH_QOS_INCLUDED */

    return SSPLOOP_STOP;
}

SSP_HANDLER_RETVAL
ssphandler_pqos_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_QOS_INCLUDED
    sys_error_t rv = SYS_OK;
    int i = 0;
    uint16 que_pri = 0;
    char *ptmp = NULL, *pbuf_tmp;
    char buf[5];
    uint8 que2priority_map[MAX_QUEUE_NUM]={ 1, 3, 5,7}; /*default priority seting value: Low->Hight*/


    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {

        PQOS_DBG(("__ssphandler_pqos_cgi:queue value for each port :pairs[0]: %s", cxt->pairs[0].value));
        board_qos_type_set(QT_PORT_BASED);

        ptmp = (char *)(cxt->pairs[0].value);
        pbuf_tmp = (char *)&buf[0];

        while(*ptmp != '\0'){
            PQOS_DBG(("__PVIDSET ptmp:%02x\n", *ptmp));
            if(*ptmp == ','){
                ptmp++;
                *pbuf_tmp = '\0';
                que_pri = sal_atoi(buf);
                sal_memset(buf, 0, sizeof(char)*5);
                pbuf_tmp = &buf[0];
                rv = board_untagged_priority_set(SAL_ZUPORT_TO_UPORT(i), que2priority_map[que_pri]);
                i++;
            } else {
                *pbuf_tmp = *ptmp;
                pbuf_tmp++;
                ptmp++;
            }
        }
    }
    if(SYS_OK == rv){
#if CFG_PERSISTENCE_SUPPORT_ENABLED
        persistence_save_current_settings("qos");
#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */
    }
#endif /* CFG_SWITCH_QOS_INCLUDED */
    return SSP_HANDLER_RET_INTACT;
}

SSPLOOP_RETVAL
ssploop_pqos_tag_weights(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_QOS_INCLUDED
    if (index < MAX_WEIGHT) {
        return SSPLOOP_PROCEED;
    }
#endif /* CFG_SWITCH_QOS_INCLUDED */

    return SSPLOOP_STOP;
}

