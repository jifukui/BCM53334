/*
 * $Id: network.c,v 1.28 Broadcom SDK $
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
#include "utils/net.h"
#include "appl/persistence.h"

#if CFG_PERSISTENCE_SUPPORT_ENABLED
/* Longest format: <type:1> <IP:4> <netmask:4> <gateway:4> */
#define MAX_SERIALIZED_LEN (13)

#define PWD_ENC_XOR     (0x74)

extern int32
eth_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;

extern int32
password_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;

#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
extern int32
autoip_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;
#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */

extern int32
access_control_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;

#if CFG_UIP_STACK_ENABLED
int32
eth_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    uint8 ip[4], netmask[4], gateway[4];
    uint8 type;
#ifdef	CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
    const char *ipconfig = NULL;
#endif
    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {

        medium->read(&type, 1);
        medium->read(ip, 4);
        medium->read(netmask, 4);
        medium->read(gateway, 4);

        set_network_interface_config(
            (INET_CONFIG)type,
            (const uint8 *)ip,
            (const uint8 *)netmask,
            (const uint8 *)gateway);

    } else if (op == SERIALIZE_OP_SAVE) {

        type = (uint8)get_network_interface_config(ip, netmask, gateway);

        medium->write(&type, 1);
        medium->write(ip, 4);
        medium->write(netmask, 4);
        medium->write(gateway, 4);

    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {
    
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
       /* ip configuation format : 
                 ifconfig=dhcp
                 ifconfig=192.168.0.239/255.255.255.0/192.168.0.254 */
       ipconfig = sal_config_get(SAL_CONFIG_UIP_IFCONFIG);

	   if (ipconfig == NULL) {
           goto default_dhcp;
	   };

	   if (sal_strcmp(ipconfig, "dhcp") == 0) {
           goto default_dhcp;
	   };

       if (parse_ip(ipconfig, ip)) {
           goto default_dhcp;
	   };

	   ipconfig = sal_strchr(ipconfig, '/') + 1;
	   
       if (ipconfig == NULL || parse_ip(ipconfig, netmask)) {
		   goto default_dhcp;
	   };
   
	   ipconfig = sal_strchr(ipconfig, '/') + 1;
      
       if (ipconfig == NULL || parse_ip(ipconfig, gateway)) {
		   goto default_dhcp;
	   };

	   set_network_interface_config(INET_CONFIG_STATIC, 
	   	                            ip, 
	   	                            netmask, 
	   	                            gateway);
       return 0;
	   
       default_dhcp:
#endif	   	
	   set_network_interface_config(
				 INET_CONFIG_DHCP_FALLBACK,
				 NULL,
				 NULL,
				 NULL);
       return 0;			
			
    }
    return MAX_SERIALIZED_LEN;
}
#endif /* CFG_UIP_STACK_ENABLED */


#ifdef CFG_SYSTEM_PASSWORD_INCLUDED

int32
password_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    unsigned char buf[MAX_PASSWORD_LEN + 1];
    unsigned char buf2[MAX_PASSWORD_LEN + 1];
    unsigned char carry, temp, shift, bytes, bits;
    int8 i;

    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {

        medium->read(buf, MAX_PASSWORD_LEN);
        medium->read(&shift, 1);

        /*
         * XOR back
         */
        for(i=0; i<MAX_PASSWORD_LEN; i++) {
            buf[i] ^= PWD_ENC_XOR + i * 5;
        }

        /*
         * Shift back bits
         */
        carry = 0;
        bits = (shift % 7) + 1;
        for(i=MAX_PASSWORD_LEN - 1; i>=0; i--) {
            temp = buf[i] >> (8 - bits);
            buf[i] = (buf[i] << bits) | carry;
            carry = temp;
        }
        buf[MAX_PASSWORD_LEN - 1] |= carry;

        /*
         * Shift back bytes
         */
        bytes = (shift % (MAX_PASSWORD_LEN - 1)) + 1;
        for(i=0; i<MAX_PASSWORD_LEN; i++) {
            buf2[i] = buf[(i + bytes) % MAX_PASSWORD_LEN];
        }

        /*
         * Make sure it's null terminated
         */
        buf2[MAX_PASSWORD_LEN] = 0;

        /*
         * Now we've gotten it.
         */
        set_login_password((char *)buf2);

    } else if (op == SERIALIZE_OP_SAVE) {

        /*
         * Characters after null terminator ('\0') will also be copied
         * and this is what we want (to be randomized).
         */
        get_login_password((char *)buf2, sizeof(buf2));

        /*
         * Decide how many bits we are going to shift.
         */
        shift = (unsigned char)(sal_rand() % 256);

        /*
         * Shift bytes
         */
        bytes = (shift % (MAX_PASSWORD_LEN - 1)) + 1;
        for(i=0; i<MAX_PASSWORD_LEN; i++) {
            buf[(i + bytes) % MAX_PASSWORD_LEN] = buf2[i];
        }

        /*
         * Shift bits
         */
        carry = 0;
        bits = (shift % 7) + 1;
        for(i=0; i<MAX_PASSWORD_LEN; i++) {
            temp = buf[i] << (8 - bits);
            buf[i] = (buf[i] >> bits) | carry;
            carry = temp;
        }
        buf[0] |= carry;

        /*
         * Do some XOR operations
         */
        for(i=0; i<MAX_PASSWORD_LEN; i++) {
            buf[i] ^= PWD_ENC_XOR + i * 5;
        }

        medium->write(buf, MAX_PASSWORD_LEN);
        medium->write(&shift, 1);
    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {

        set_login_password((char *)DEFAULT_PASSWORD);
        return 0;
    }
    return MAX_PASSWORD_LEN + 1;
}

#endif /* CFG_SYSTEM_PASSWORD_INCLUDED */

#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
int32
autoip_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    uint8   auto_ip[4];
    BOOL    valid;

    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {

        medium->read(&valid, 1);
        medium->read(auto_ip, 4);
        set_autoip_addr(valid, auto_ip);

    } else if (op == SERIALIZE_OP_SAVE) {

        get_autoip_addr(&valid, auto_ip);
        medium->write(&valid, 1);
        medium->write(auto_ip, 4);

    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {

        valid = FALSE;
        set_autoip_addr(valid, auto_ip);
        return 0;
    }

    return MAX_SERIAL_AUTOIP_LEN;
}

#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */

#if CFG_UIP_IPV6_ENABLED
int32
ipv6_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    uip_ip6addr_t ipaddr;

    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {

        medium->read((uint8 *)&ipaddr, sizeof(ipaddr));
        set_manual_ipv6_address(&ipaddr);

    } else if (op == SERIALIZE_OP_SAVE) {

        uint8 count, i;
        ip6addr_type type;

        count = get_ipv6_address_count();
        for(i=0; i<count; i++) {
            if (get_ipv6_address(i, &ipaddr, &type) == SYS_OK) {
                if (type == IP6ADDR_TYPE_MANUAL) {
                    medium->write((uint8 *)&ipaddr, sizeof(ipaddr));
                    break;
                }
            }
        }

        if (i == count) {
            uip_create_unspecified(&ipaddr);
            medium->write((uint8 *)&ipaddr, sizeof(ipaddr));
        }

    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {

        /* Default: no manual address */
        uip_create_unspecified(&ipaddr);
        set_manual_ipv6_address(&ipaddr);
        return 0;
    }

    return sizeof(uip_ip6addr_t);
}
#endif /* CFG_UIP_IPV6_ENABLED */

#if CFG_UIP_STACK_ENABLED
int32
access_control_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    BOOL    valid;
    int     acc_num;
    uint8 accessctrlip[MAX_ACCESSCONTROL_IP][4], accmaskip[MAX_ACCESSCONTROL_IP][4];
    

    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {
        medium->read(&valid, sizeof(valid));
        medium->read_uint32((uint32 *)&acc_num);
        medium->read((uint8 *)accessctrlip, sizeof(accessctrlip));
        medium->read((uint8 *)accmaskip, sizeof(accmaskip));
        set_accessip_addr(valid, acc_num, accessctrlip, accmaskip);

    } else if (op == SERIALIZE_OP_SAVE) {
        get_accessip_addr(&valid, &acc_num, accessctrlip, accmaskip);
        medium->write(&valid, sizeof(valid));
        medium->write_uint32((uint32) acc_num);
        medium->write((uint8 *)accessctrlip, sizeof(accessctrlip));
        medium->write((uint8 *)accmaskip, sizeof(accmaskip));

    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {
        valid = FALSE;
        set_accessip_addr(valid, 0, NULL, NULL);
        return 0;
    }

    return (sizeof(valid) + sizeof(acc_num) + sizeof(accessctrlip) + sizeof(accmaskip));
}

int32
admin_privilege_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    BOOL    valid;
    

    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {
        medium->read(&valid, sizeof(valid));
        set_adminpv(valid);

    } else if (op == SERIALIZE_OP_SAVE) {
        get_adminpv(&valid);
        medium->write(&valid, sizeof(valid));

    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {
        valid = FALSE;
        set_adminpv(valid);
        return 0;
    }

    return (sizeof(valid));
}
#endif /* CFG_UIP_STACK_ENABLED */
#endif /* CFG_PERSISTENCE_SUPPORT_ENABLED */
