/*
 * $Id: vlan.c,v 1.22 Broadcom SDK $
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

#ifdef CFG_SWITCH_VLAN_INCLUDED

#if CFG_PERSISTENCE_SUPPORT_ENABLED

extern int32
vlan_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;

extern int32
pvid_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;

/*
 * BYTE (0~1)  : vlan type.
 * BYTE (2~3)  : vlan count.
 * BYTE (4~5)  : vlan id.
 * BYTE (6~X)  : logical port list and tagged logical port list
 */
#define MAX_VLAN_SERIALIZED_UNIT_LEN (2 + MAX_UPLIST_WIDTH * 2)

#define MAX_VLAN_SERIALIZED_LEN \
        (4 + BOARD_MAX_NUM_OF_QVLANS * MAX_VLAN_SERIALIZED_UNIT_LEN)

int32
vlan_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    uint16 length = 0;
    uint16 i, val16, count = 0;
    uint16 vlan_id, pvid;
    uint8 j, uplist[MAX_UPLIST_WIDTH], tag_uplist[MAX_UPLIST_WIDTH];
    vlan_type_t type;
    uint16 vlan_count = 0;
    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {
        medium->read_uint16(&val16);
        type = (vlan_type_t)val16;
#if !defined(CFG_SWITCH_PVLAN_INCLUDED)
        type = VT_DOT1Q;
#endif
        board_vlan_type_set(type);
        medium->read_uint16(&count);
        length += 4;
        if (type == VT_PORT_BASED)
        {
#if !defined(CFG_SWITCH_PVLAN_INCLUDED)
            for (i = 0; i < count; i++) {
                 medium->read_uint16(&vlan_id);
                 medium->read(uplist, MAX_UPLIST_WIDTH);
            }
#else 
            /* clear default VLAN , load all VLAN from persistance*/
            board_vlan_destroy(1);
            for (i = 0; i < count; i++) {
                medium->read_uint16(&vlan_id);
                board_vlan_create(vlan_id);
                medium->read(uplist, MAX_UPLIST_WIDTH);
                board_pvlan_port_set(vlan_id, uplist);
            }
#endif            
            length += count * (2 + MAX_UPLIST_WIDTH);
        } else if (type == VT_DOT1Q)
        {
            /* clear default VLAN , load all VLAN from persistance*/
			board_vlan_destroy(VLAN_DEFAULT);
            for (i = 0; i < count; i++) {
                medium->read_uint16(&vlan_id);
                board_vlan_create(vlan_id);
                medium->read(uplist, MAX_UPLIST_WIDTH);
                medium->read(tag_uplist, MAX_UPLIST_WIDTH);
                board_qvlan_port_set(vlan_id, uplist, tag_uplist);
            }
            length += count * MAX_VLAN_SERIALIZED_UNIT_LEN;
        }

        /* Consume remaining bytes */
        for(i=0; i<MAX_VLAN_SERIALIZED_LEN - length; i++) {
            medium->read(&j, 1);
        }

    } else if (op == SERIALIZE_OP_SAVE) {
        board_vlan_type_get(&type);
        val16 = (uint16)type;
        medium->write_uint16(val16);
        count = board_vlan_count();
        medium->write_uint16(count);
        length = 4;
        if (type == VT_PORT_BASED)
        {
            /* Assume index is used as vlan id */
            for (i = 0; i < count; i++) {
                board_qvlan_get_by_index(i, &vlan_id, uplist, tag_uplist);
                board_pvlan_port_get(vlan_id, uplist);
                medium->write_uint16(vlan_id);
                medium->write(uplist, MAX_UPLIST_WIDTH);
            }
            length += count * (2 + MAX_UPLIST_WIDTH);
        } else if (type == VT_DOT1Q) {
            for (i = 0; i < count; i++) {
                board_qvlan_get_by_index(i, &vlan_id, uplist, tag_uplist);
                medium->write_uint16(vlan_id);
                medium->write(uplist, MAX_UPLIST_WIDTH);
                medium->write(tag_uplist, MAX_UPLIST_WIDTH);
            }
            length += count * MAX_VLAN_SERIALIZED_UNIT_LEN;
        }

        /* Consume remaining bytes */
        j = 0;
        for (i = length; i < MAX_VLAN_SERIALIZED_LEN; i++) {
            medium->write(&j, 1);
        }

    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {

        board_vlan_type_get(&type);

        if (SAL_IS_UMPLUS()) {            
            board_vlan_type_set(VT_PORT_BASED); /* set port based vlan as default for umplus solution */
            return 0;
        }
        
        if (VT_DOT1Q == type) {
            vlan_count = board_vlan_count();
            
            
            SAL_UPORT_ITER(i) {
                board_untagged_vlan_get(i, &pvid);
                
                if(VLAN_DEFAULT != pvid){
                    board_untagged_vlan_set(i, VLAN_DEFAULT);
                }
            }
            
            while(vlan_count-- > 0) {
                board_qvlan_get_by_index(vlan_count, &vlan_id, uplist, tag_uplist);
                
                if (VLAN_DEFAULT == vlan_id) {
                    
                    for (i = 0; i < MAX_UPLIST_WIDTH; i++) {
                        
                        if (tag_uplist[i] != 0) {
                            tag_uplist[i] = 0;
                        }
                        if (uplist[i] != 0xFF) {
                            uplist[i] = 0xFF;
                        }
                    }
                    board_qvlan_port_set(vlan_id, uplist, tag_uplist);
                    
                } else {
                    board_vlan_destroy(vlan_id);
                }
            }
            
        }else{
            board_vlan_type_set(VT_DOT1Q);
        }
        
        return 0;
    }

    return MAX_VLAN_SERIALIZED_LEN;
}

int32
pvid_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    uint8 i;
    uint16 vlan_id;
    vlan_type_t type;

    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }
    board_vlan_type_get(&type);

    if (op == SERIALIZE_OP_LOAD) {
        SAL_UPORT_ITER(i) {
            medium->read_uint16(&vlan_id);
            if (type == VT_DOT1Q) {
                board_untagged_vlan_set(i, vlan_id);
            }
        }
    } else if (op == SERIALIZE_OP_SAVE) {
        board_vlan_type_get(&type);
        if (type == VT_DOT1Q){
            SAL_UPORT_ITER(i) {
                board_untagged_vlan_get(i, &vlan_id);
                medium->write_uint16(vlan_id);
            }
        } else {
            vlan_id = 1;
            SAL_UPORT_ITER(i) {
                medium->write_uint16(vlan_id);
            }
        }
    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {
        /* Do nothing since defaults are restored in vlan serializer */
        return 0;
    }

    return board_uport_count() * 2;

}


#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */

#endif /* CFG_SWITCH_VLAN_INCLUDED */
