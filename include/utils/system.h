/*
 * $Id: system.h,v 1.2 Broadcom SDK $
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

#ifndef _UTILS_SYSTEM_H_
#define _UTILS_SYSTEM_H_

#define __STR(x) #x
#define STR(x) __STR(x)

/*
 * System name/description
 */
#define MAX_SYSTEM_NAME_LEN (64)
#ifdef CFG_ZEROCONF_MDNS_INCLUDED
#if CFG_XGS_CHIP
#define DEFAULT_SYSTEM_NAME  CFG_BOARDNAME
#else /* ROBO */
#define DEFAULT_SYSTEM_NAME  STR(_BOARD_NAME_)
#endif /* CFG_XGS_CHIP */
#else /* !CFG_ZEROCONF_MDNS_INCLUDED */
#define DEFAULT_SYSTEM_NAME  ""
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

/*
 * Default MAC address
 */
#define DEFAULT_MAC_ADDR    { 0x00, 0x10, 0x18, 0x55, 0x44, 0x4b }

typedef enum {
    REGISTRATION_STATUS_TURN_OF_REMIND = 0,
    REGISTRATION_STATUS_REMIND_ME_LATER,
    REGISTRATION_STATUS_REGISTERED
} REGISTRATION_STATUS;

/*
 * Product registration status
 */
#define MAX_REG_STATUS_LEN              (1)
#define DEFAULT_REGISTRATION_STATUS     REGISTRATION_STATUS_REMIND_ME_LATER

/*
 * Product serial number
 */
#define MAX_SERIAL_NUM_MAGIC_LEN        (4) /* magic number for serial number */
#define MAX_SERIAL_NUM_LEN              (20)
#define DEFAULT_SERIAL_NUMBER           ""

/* Parse mac address nvram format to array */
extern sys_error_t parse_mac_address(const char *str, uint8 *macaddr);

/* Set/Get product registration status */
extern sys_error_t set_registration_status(uint8 status);
extern sys_error_t get_registration_status(uint8 *status);

/* Set/Get product serial number */
extern sys_error_t get_serial_num(uint8 *valid, char *serial_num);
extern sys_error_t set_serial_num(uint8 valid, const char *serial_num);

/* Set/Get system name or description */
extern sys_error_t set_system_name(const char *name);
extern sys_error_t get_system_name(char *buf, uint8 len);

/* Get system mac */
extern sys_error_t get_system_mac(uint8 *mac_buf);

extern void system_utils_init(void);

#endif /* _UTILS_SYSTEM_H_ */
