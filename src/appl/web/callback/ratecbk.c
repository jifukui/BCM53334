/*
 * $Id: ratecbk.c,v 1.18 Broadcom SDK $
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
#include "utilcbk.h"
#include "appl/persistence.h"
#include "boardapi/rate.h"
#include "../src/appl/web/content/sspmacro_rate.h"
#include "../src/appl/web/content/sspmacro_storm.h"

SSP_HANDLER_RETVAL ssphandler_setrate_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem);
SSP_HANDLER_RETVAL ssphandler_rate_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem);

#ifdef CFG_SWITCH_RATE_INCLUDED
/* Define Storm Control Type */
#define STORM_DISABLE       1
#define STORM_BCST_ONLY     2
#define STORM_BCST_MCST     3
#define STORM_BCST_DLF      4
#define STORM_BCST_MCST_DLF 5


/* need to be defined per chip */
#define NUM_VALUES_FOR_STORM_CONTROL    11

char *storm_control_value_captions[] =
{
    "512 Kbps",
    "1 Mbps",
    "2 Mbps",
    "4 Mbps",
    "8 Mbps",
    "16 Mbps",
    "32 Mbps",
    "64 Mbps",
    "128 Mbps",
    "256 Mbps",
    "512 Mbps"
};

int storm_control_values1[] =
{
    512, 1, 2, 4, 8, 16, 32, 64, 128, 256, 5120
};

int storm_control_values2[] =
{
      512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288
};

/* for rate */
#define NUM_VALUES_FOR_RATE             12

static char *rate_limit_value_captions[] =
{
    "No Limit",
    "512 Kbps",
    "1 Mbps",
    "2 Mbps",
    "4 Mbps",
    "8 Mbps",
    "16 Mbps",
    "32 Mbps",
    "64 Mbps",
    "128 Mbps",
    "256 Mbps",
    "512 Mbps"
};

int rate_limit_value_values[] =
{
    0, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288
};

const char STRING_COMMENT_START1[] = "<!--";
const char STRING_NULL1[] = "";
const char STRING_NA1[] = "--";
static char cbk_port_num[5];

char *port_number(int nzport)
{
    sal_sprintf(cbk_port_num, "%02u", nzport);
    return cbk_port_num;
}
#endif /* CFG_SWITCH_RATE_INCLUDED */

void sspvar_rate_tag_info(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_RATE_INCLUDED
    int uport = SAL_ZUPORT_TO_UPORT(0);
    char *pbuf;
    uint32 bits_sec;
    int i;
    if (params[2] == SSPMACRO_RATE_LEFT_HALF) {
        uport = SAL_ZUPORT_TO_UPORT(params[1]);
    } else if(params[2] == SSPMACRO_RATE_RIGHT_HALF) {
        uport = SAL_ZUPORT_TO_UPORT((params[1] + (board_uport_count()/2 + board_uport_count()%2)));
    }

    
    /* get value according to tag */
    switch (params[0]) {
        case SSPMACRO_RATE_EGRESS:
            sal_strcpy(ssputil_shared_buffer, STRING_NA1);
            if (board_port_rate_egress_get(uport, &bits_sec) != SYS_OK) {
                break;
            }
            if (bits_sec == 0) {
                sal_strcpy(ssputil_shared_buffer, "No Limit");
            } else {
                  for(i=1; i<NUM_VALUES_FOR_RATE; i++) {
                      if ((uint32)rate_limit_value_values[i] == (bits_sec / 1000)) {
                        sal_sprintf(ssputil_shared_buffer, "%s", rate_limit_value_captions[i]);
                        break;
                    }
                }
            }
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            break;
        case SSPMACRO_RATE_INGRESS:
            sal_strcpy(ssputil_shared_buffer, STRING_NA1);
            if (board_port_rate_ingress_get(uport, &bits_sec) != SYS_OK) {
                break;
            }
            if (bits_sec == 0) {
                sal_strcpy(ssputil_shared_buffer, "No Limit");
            } else {
                  for(i=1; i<NUM_VALUES_FOR_RATE; i++) {
                      if ((uint32)rate_limit_value_values[i] == (bits_sec / 1000)) {
                        sal_sprintf(ssputil_shared_buffer, "%s", rate_limit_value_captions[i]);
                        break;
                    }
                }
            }
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            break;
        case SSPMACRO_RATE_CAPS:
            pbuf = ssputil_shared_buffer;
            for (i=0; i<NUM_VALUES_FOR_RATE; i++) {
                if (rate_limit_value_values[i] < 0)
                    continue;
                *(pbuf++) = '"';
                sal_strcpy(pbuf, rate_limit_value_captions[i]);
                pbuf += sal_strlen(pbuf);
                *(pbuf++) = '"';
                if (i < NUM_VALUES_FOR_RATE-1) {
                    *(pbuf++) = ',';
                }
            }
            *pbuf = 0;
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            break;
        case SSPMACRO_RATE_VALS:
            pbuf = ssputil_shared_buffer;
            for(i=0; i<NUM_VALUES_FOR_RATE; i++) {
                if (rate_limit_value_values[i] < 0)
                    continue;
                sal_sprintf(pbuf, "%u", (int)rate_limit_value_values[i]);
                pbuf += sal_strlen(pbuf);
                if (i < NUM_VALUES_FOR_RATE-1) {
                    *(pbuf++) = ',';
                }
            }
            *pbuf = 0;
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            break;
        case SSPMACRO_RATE_SPS:
            if (board_port_rate_ingress_get(uport, &bits_sec) == SYS_OK) {
                sal_strcpy(ssputil_shared_buffer, STRING_NULL1);
            } else {
                sal_strcpy(ssputil_shared_buffer, STRING_COMMENT_START1);
            }
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            break;
        case SSPMACRO_RATE_SPE:
            if (board_port_rate_ingress_get(uport, &bits_sec) == SYS_OK) {
                sal_strcpy(ssputil_shared_buffer, STRING_NULL1);
            } else {
                sal_strcpy(ssputil_shared_buffer, STRING_COMMENT_START1);
            }
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            break;
        default:
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = "";
            break;
    }
#endif /* CFG_SWITCH_RATE_INCLUDED */
}


void sspvar_rate_tag_string(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_RATE_INCLUDED
    int *p, uport=0, i;
    uint32 bits_sec = 0;

    p = (int *)ssputil_psmem_get(psmem, ssphandler_rate_cgi);

    uport = SAL_NZUPORT_TO_UPORT(*p);

    switch (params[0]) {
        case SSPMACRO_RATE_EGRESS:
              if (board_port_rate_egress_get(uport, &bits_sec) != SYS_OK) {
                break;
            }
            for (i=0 ; i<NUM_VALUES_FOR_RATE ; i++) {
                if ((uint32)rate_limit_value_values[i] == (bits_sec / 1000))
                    break;
            }
            if (i == NUM_VALUES_FOR_RATE) {
                i = 0;
            }
            sal_sprintf(ssputil_shared_buffer, "%u", rate_limit_value_values[i]);
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            break;
        case SSPMACRO_RATE_INGRESS:
              if (board_port_rate_ingress_get(uport, &bits_sec) != SYS_OK) {
                break;
            }
            for (i=0 ; i<NUM_VALUES_FOR_RATE ; i++) {
                if ((uint32)rate_limit_value_values[i] == (bits_sec / 1000))
                    break;
            }
            if (i == NUM_VALUES_FOR_RATE) {
                i = 0;
            }
            sal_sprintf(ssputil_shared_buffer, "%u", rate_limit_value_values[i]);
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            break;
        case SSPMACRO_RATE_PORT:
            sal_strcpy(ssputil_shared_buffer, port_number(SAL_UPORT_TO_NZUPORT(uport)));
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            break;
        case SSPMACRO_RATE_PORT1:
            ret->type = SSPVAR_RET_INTEGER;
            ret->val_data.integer = SAL_UPORT_TO_NZUPORT(uport);
            break;
        default:
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = "";
            break;
    }
#endif /* CFG_SWITCH_RATE_INCLUDED */
}


SSP_HANDLER_RETVAL ssphandler_rate_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_RATE_INCLUDED
    int *p;

    if (cxt->type == SSP_HANDLER_QUERY_STRINGS) {
        p = (int *)ssputil_psmem_alloc(psmem, ssphandler_rate_cgi, sizeof(int));
        *p = sal_atoi(cxt->pairs[0].value);
    }
#endif /* CFG_SWITCH_RATE_INCLUDED */
    return SSP_HANDLER_RET_INTACT;
}


SSP_HANDLER_RETVAL ssphandler_setrate_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_RATE_INCLUDED
    int uport;
    uint32 bits_sec, val;

    if (cxt->type != SSP_HANDLER_QUERY_STRINGS) {
        return SSP_HANDLER_RET_INTACT;
    }

    if (cxt->count < 2) { /* port number and ingress */
        return SSP_HANDLER_RET_INTACT;
    }

    uport = SAL_NZUPORT_TO_UPORT(sal_atoi(cxt->pairs[0].value));

    val = sal_atoi(cxt->pairs[1].value) * 1000; /* Ingress rate value */
    if (board_port_rate_ingress_get(uport, &bits_sec) != SYS_OK) {
        webutil_show_error(
            cxt, psmem,
            "Set Rate",
            "Get egress rate failed",
            err_button_retry,
            err_action_back
        );
        return SSP_HANDLER_RET_MODIFIED;
    }

    if (bits_sec != val) {
        if (board_port_rate_ingress_set(uport, val) != SYS_OK) {
            webutil_show_error(
                cxt, psmem,
                "Set Rate",
                "Set egress rate failed",
                err_button_retry,
                err_action_back
            );
            return SSP_HANDLER_RET_MODIFIED;
        }
    }

    val = sal_atoi(cxt->pairs[2].value) * 1000; /* Egress rate value */
    if (board_port_rate_egress_get(uport, &bits_sec) != SYS_OK) {
        webutil_show_error(
            cxt, psmem,
            "Set Rate",
            "Get egress rate failed",
            err_button_retry,
            err_action_back
        );
        return SSP_HANDLER_RET_MODIFIED;
    }

    if(bits_sec != val) {
        if (board_port_rate_egress_set(uport, val) != SYS_OK) {
            webutil_show_error(
                cxt, psmem,
                "Set Rate",
                "Set egress rate failed",
                err_button_retry,
                err_action_back
            );
            return SSP_HANDLER_RET_MODIFIED;
        }
    }
#if CFG_PERSISTENCE_SUPPORT_ENABLED
    persistence_save_current_settings("rate");
#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */
#endif /* CFG_SWITCH_RATE_INCLUDED */
    return SSP_HANDLER_RET_INTACT;
}


void sspvar_storm_tag_status(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_RATE_INCLUDED
    uint8 temp;

    board_rate_type_get(&temp);
    if (temp != STORM_RATE_NONE) {
        sal_strcpy(ssputil_shared_buffer, "Enable");
    } else 
#endif /* CFG_SWITCH_RATE_INCLUDED */
    {
        sal_strcpy(ssputil_shared_buffer, "Disable");
    }
    ret->type = SSPVAR_RET_STRING;
    ret->val_data.string = ssputil_shared_buffer;
}


void sspvar_storm_tag_info(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_RATE_INCLUDED
    char *pbuf;
    int i;
    uint8 temp;
    uint32 bits_sec;

    switch (params[0]) {
        case SSPMACRO_STORM_CAPS:
            pbuf = ssputil_shared_buffer;
            for(i=0; i<NUM_VALUES_FOR_STORM_CONTROL; i++) {
                if (storm_control_values1[i] < 0)
                    continue;
                *(pbuf++) = '"';
                sal_strcpy(pbuf, storm_control_value_captions[i]);
                pbuf += sal_strlen(pbuf);
                *(pbuf++) = '"';
                if (i < NUM_VALUES_FOR_STORM_CONTROL-1) {
                    *(pbuf++) = ',';
                }
            }
            *pbuf = 0;
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            break;
        case SSPMACRO_STORM_RATE2:
            board_rate_get(SAL_ZUPORT_TO_UPORT(0), &bits_sec);
            for(i=0; i<NUM_VALUES_FOR_STORM_CONTROL; i++) {
                  if ((bits_sec / (storm_control_values2[i] * 1000)) == 1)
                       break;
            }
            ret->type = SSPVAR_RET_INTEGER;
            ret->val_data.integer = storm_control_values1[i];
            break;
        case SSPMACRO_STORM_TYPE2:
            board_rate_type_get(&temp);
            ret->type = SSPVAR_RET_INTEGER;
            if (temp == STORM_RATE_ALL)
                  ret->val_data.integer = STORM_BCST_MCST_DLF;
            else
                ret->val_data.integer = STORM_DISABLE;
            break;
        case SSPMACRO_STORM_VALS:
            pbuf = ssputil_shared_buffer;
            for(i=0; i<NUM_VALUES_FOR_STORM_CONTROL; i++) {
                if (storm_control_values1[i] < 0)
                    continue;
                sal_sprintf(pbuf, "%u", (int)storm_control_values1[i]);
                pbuf += sal_strlen(pbuf);
                if (i < NUM_VALUES_FOR_STORM_CONTROL-1) {
                    *(pbuf++) = ',';
                }
            }
            *pbuf = 0;
            ret->type = SSPVAR_RET_STRING;
            ret->val_data.string = ssputil_shared_buffer;
            break;
        default:
            ret->val_data.string = "";
            break;
    }

    return;
#endif /* CFG_SWITCH_RATE_INCLUDED */
}


SSP_HANDLER_RETVAL ssphandler_storm_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem)
{
#ifdef CFG_SWITCH_RATE_INCLUDED
    uint8 flags;
    int i, temp=0;
    uint16 uport;
    uint32 rate=0;

    if (cxt->type != SSP_HANDLER_QUERY_STRINGS) {
        return SSP_HANDLER_RET_INTACT;
    }

    if (cxt->count < 1) {
        return SSP_HANDLER_RET_INTACT;
    }

    /* The storm_type order should follow the API */
    temp = sal_atoi(cxt->pairs[0].value);
    if (temp == STORM_DISABLE) {
        flags = STORM_RATE_NONE;
    } else if (temp == STORM_BCST_ONLY) {
        flags = STORM_RATE_BCAST;
    } else if (temp == STORM_BCST_MCST) {
        flags = (STORM_RATE_BCAST | STORM_RATE_MCAST);
    } else if (temp == STORM_BCST_DLF) {
        flags = STORM_RATE_DLF;
    } else {
        flags = STORM_RATE_ALL;
    }

    board_rate_type_set(flags);
    if(temp != STORM_DISABLE) {
          for(i=0; i<NUM_VALUES_FOR_STORM_CONTROL; i++) {
           if (storm_control_values1[i] == sal_atoi(cxt->pairs[1].value))
              break;
        }
        rate = storm_control_values2[i] * 1000;
    } else {
        rate = 0;
    }

    SAL_UPORT_ITER(uport) {
        if (board_rate_set(uport, rate) != SYS_OK) {
            webutil_show_error(
                cxt, psmem,
                "Set Storm",
                "Set storm failed",
                err_button_retry,
                err_action_back
            );
            return SSP_HANDLER_RET_MODIFIED;
        }
    }
#if CFG_PERSISTENCE_SUPPORT_ENABLED
    persistence_save_current_settings("storm");
#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */
#endif /* CFG_SWITCH_RATE_INCLUDED */
    return SSP_HANDLER_RET_INTACT;
}

