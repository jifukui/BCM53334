/*
 * $Id: system.c,v 1.3 Broadcom SDK $
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
#include "utils/system.h"
#include "appl/persistence.h"

#if CFG_PERSISTENCE_SUPPORT_ENABLED
extern int32
system_name_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;

extern int32
port_desc_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;

extern int32
port_enable_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;

#ifdef CFG_PRODUCT_REGISTRATION_INCLUDED
extern int32
registration_status_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;
#endif /* CFG_PRODUCT_REGISTRATION_INCLUDED */

int32
system_name_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    unsigned char buf[MAX_SYSTEM_NAME_LEN + 1];

    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {

        medium->read(buf, MAX_SYSTEM_NAME_LEN);
        /*
         * Make sure it's null terminated
         */
        buf[MAX_SYSTEM_NAME_LEN] = 0;

        set_system_name((char *)buf);

    } else if (op == SERIALIZE_OP_SAVE) {

        get_system_name((char *)buf, sizeof(buf));
        medium->write(buf, MAX_SYSTEM_NAME_LEN);

    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {

        set_system_name((char *)DEFAULT_SYSTEM_NAME);
        return 0;
    }

    return MAX_SYSTEM_NAME_LEN;
}

int32
port_desc_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    unsigned char buf[WEB_PORT_DESC_LEN + 1];
    uint16 uport;

    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {

        SAL_UPORT_ITER(uport) {
            sal_memset(buf, 0, sizeof(buf));
            medium->read(buf, WEB_PORT_DESC_LEN);
            /*
             * Make sure it's null terminated
             */
            buf[WEB_PORT_DESC_LEN] = 0; 
            set_port_desc(uport, (char *)buf);
        }
    } else if (op == SERIALIZE_OP_SAVE) {

        SAL_UPORT_ITER(uport) {
            sal_memset(buf, 0, sizeof(buf)); /* Get_port_desc may fail, clear buf before get_port_desc()*/
            get_port_desc(uport, (char *)buf, WEB_PORT_DESC_LEN);                             
            medium->write(buf, WEB_PORT_DESC_LEN);
        }
    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {

        SAL_UPORT_ITER(uport) {          
            set_port_desc(uport, "");                     
        }
        
        return 0;
    }

    return (WEB_PORT_DESC_LEN * board_uport_count());

}

int32
port_enable_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    uint16 uport;
    BOOL porten;
    uint16 port_en;

    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {

        SAL_UPORT_ITER(uport) {
        
            medium->read_uint16(&port_en);
            porten = 0;
            if (port_en) {
                porten = 1;
            }
            board_port_enable_set(uport, porten);
        }
    } else if (op == SERIALIZE_OP_SAVE) {

        SAL_UPORT_ITER(uport) {
            board_port_enable_get(uport, &porten);
            medium->write_uint16(porten);
        }
    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {
        SAL_UPORT_ITER(uport) {
            board_port_enable_set(uport, 1);
        }

        return 0;
    }

    return (board_uport_count() * 2);

}

#ifdef CFG_PRODUCT_REGISTRATION_INCLUDED
int32
registration_status_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    unsigned char reg_status;

    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {

        medium->read(&reg_status, MAX_REG_STATUS_LEN);

        set_registration_status(reg_status);

    } else if (op == SERIALIZE_OP_SAVE) {

        get_registration_status(&reg_status);
        medium->write(&reg_status, MAX_REG_STATUS_LEN);

    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {

        reg_status = DEFAULT_REGISTRATION_STATUS;
        set_registration_status(reg_status);
        return 0;
    }

    return MAX_REG_STATUS_LEN;
}
#endif /* CFG_PRODUCT_REGISTRATION_INCLUDED */
#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */
