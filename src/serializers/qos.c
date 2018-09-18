/*
 * $Id: qos.c,v 1.13 Broadcom SDK $
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
#include "system.h"
#include "appl/persistence.h"

#if CFG_PERSISTENCE_SUPPORT_ENABLED

extern int32
qos_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;

extern int32
rate_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;

extern int32
storm_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;

#ifdef CFG_SWITCH_QOS_INCLUDED

int32
qos_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    uint8 val8;
    qos_type_t type;
    uint16 uport;
    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {
        medium->read(&val8, 1);
        type = (qos_type_t)val8;
        board_qos_type_set(type);
        SAL_UPORT_ITER(uport) {
            medium->read(&val8, 1);
            if (type == QT_PORT_BASED) {
                board_untagged_priority_set(uport, val8);
            }
        }
    } else if (op == SERIALIZE_OP_SAVE) {
        board_qos_type_get(&type);
        val8 = (uint8)type;
        medium->write(&val8, 1);
        SAL_UPORT_ITER(uport) {
            board_untagged_priority_get(uport, &val8);
            medium->write(&val8, 1);
        }
    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {
        board_qos_type_set(QT_DOT1P_PRIORITY);
        return 0;
    }

    return board_uport_count()+1;
}

#endif /* CFG_SWITCH_QOS_INCLUDED */

#ifdef CFG_SWITCH_RATE_INCLUDED

int32
rate_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    uint32 rate;
    uint16 uport;
    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {
        SAL_UPORT_ITER(uport) {
            medium->read_uint32(&rate);
            board_port_rate_ingress_set(uport, rate);
            medium->read_uint32(&rate);
            board_port_rate_egress_set(uport, rate);
        }
    } else if (op == SERIALIZE_OP_SAVE) {
        SAL_UPORT_ITER(uport) {
            board_port_rate_ingress_get(uport, &rate);
            medium->write_uint32(rate);
            board_port_rate_egress_get(uport, &rate);
            medium->write_uint32(rate);
        }
    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {
        SAL_UPORT_ITER(uport) {
            board_port_rate_ingress_set(uport, 0);
            board_port_rate_egress_set(uport, 0);
        }
        return 0;
    }

    return board_uport_count()*8;
}

int32
storm_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    uint8 val8;
    uint32 val32;
    uint16 uport;
    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {
        medium->read_uint32(&val32);
        board_rate_type_set((uint8)val32);
        SAL_UPORT_ITER(uport) {
            medium->read_uint32(&val32);
            board_rate_set(uport, val32);
        }
    } else if (op == SERIALIZE_OP_SAVE) {
        board_rate_type_get(&val8);
        medium->write_uint32((uint32)val8);
        SAL_UPORT_ITER(uport) {
            board_rate_get(uport, &val32);
            medium->write_uint32(val32);
        }
    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {
        board_rate_type_set((uint8)STORM_RATE_NONE);
        SAL_UPORT_ITER(uport) {
            board_rate_set(uport, 0);
        }
        return 0;
    }

    return 4+board_uport_count()*4;
}

#endif /* CFG_SWITCH_RATE_INCLUDED */

#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */
