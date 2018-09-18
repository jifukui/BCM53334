/*
 * $Id: ui_flash.c,v 1.21 Broadcom SDK $
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
#include "appl/cli.h"
#include "utils/ui.h"
#include "utils/system.h"
#include "utils/net.h"
#include "utils/nvram.h"
#ifdef CFG_FACTORY_CONFIG_INCLUDED
#include "utils/factory.h"
#endif /* CFG_FACTORY_CONFIG_INCLUDED */
#include "appl/persistence.h"

#if (CFG_FLASH_SUPPORT_ENABLED && CFG_CLI_ENABLED)

/* Forwards */
APISTATIC void cli_cmd_flash(CLI_CMD_OP op) REENTRANT;
extern void ui_flash_init(void) REENTRANT;

APISTATIC void
cli_cmd_flash(CLI_CMD_OP op) REENTRANT
{
    char c;
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
    uint8 select = 0;
    ui_ret_t r;
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */

    if (op == CLI_CMD_OP_HELP) {
        sal_printf("Flash read/write and NVRAM configurations.\n");
    } else if (op == CLI_CMD_OP_DESC) {
        sal_printf("Flash utilities");
    } else {
        char name[32];
#ifdef CFG_PRODUCT_REGISTRATION_INCLUDED        
        char serial_num[21];
        uint8 valid;
#endif /* CFG_PRODUCT_REGISTRATION_INCLUDED */        
#ifdef CFG_FACTORY_CONFIG_INCLUDED
        factory_config_t fcfg;
#endif /* CFG_FACTORY_CONFIG_INCLUDED */
#if defined(CFG_NVRAM_SUPPORT_INCLUDED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED)
#ifndef __BOOTLOADER__
        char value[64];
#endif
        const char *str_ptr;
#endif /* defined(CFG_NVRAM_SUPPORT_INCLUDED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED) */

        sal_printf("  f - Write factory mac address\n");
#if CFG_UIP_STACK_ENABLED
        sal_printf("  i - ip configuration \n");
#endif
#ifdef CFG_PRODUCT_REGISTRATION_INCLUDED        
        sal_printf("  n - Write the serial number\n"
                   "  d - Dump the mac address and serial number\n");
#endif /* CFG_PRODUCT_REGISTRATION_INCLUDED */                   
#if defined(CFG_NVRAM_SUPPORT_INCLUDED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED)
        sal_printf( "  s - Set nvram variable\n"
                   "  g - Get nvram variable\n"
                   "  l - list all nvram variable\n");
#ifndef __BOOTLOADER__
        sal_printf( "  r - Remove nvram variable\n"
                   "  c - Commit nvram variable bindings\n");
#endif
#endif /* defined(CFG_NVRAM_SUPPORT_INCLUDED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED) */
#if defined(CFG_PERSISTENCE_SUPPORT_ENABLED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED)
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
        sal_printf("  e - Erase persistence or vendor config partition\n");
#else
        sal_printf("  e - Erase persistence partition\n");
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
#endif /* defined(CFG_PERSISTENCE_SUPPORT_ENABLED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED) */

       sal_printf("Enter your choice: ");
       c = sal_getchar();
       sal_putchar('\n');
#if CFG_UIP_STACK_ENABLED
       uint8 ip[8], mask[8], gateway[8];
       uint32 val32;
       if (c == 'i') {
            sal_printf("1. Static IP\n");
            sal_printf("2. DHCP \n");
            val32 = get_network_interface_config(ip, mask, gateway);
            sal_printf("Enter the type of ip type (%d)=", (val32 == 1) ? 1 : 2);
            if (ui_get_decimal(&val32, "") == UI_RET_OK) {
                if (val32 > 2 || val32 < 1)  {
                    sal_printf("out of range\n");
                    return;
                }
            }
            if (val32 == 1) {
                sal_printf("Enter default IP (%d.%d.%d.%d):", ip[0],ip[1],ip[2],ip[3]);
                if (ui_get_string(name, 32, "") == UI_RET_OK) {
                     if (parse_ip(name, ip) == SYS_ERR) {
                         return;
                     };                      
                }
                sal_printf("Enter default mask (%d.%d.%d.%d):", mask[0],mask[1],mask[2],mask[3]);                
                if (ui_get_string(name,32, "") == UI_RET_OK) {
                    if (parse_ip(name, mask) == SYS_ERR) {
                        return;
                    };                      
                }

                sal_printf("Enter default gateway (%d.%d.%d.%d):", gateway[0], gateway[1], gateway[2], gateway[3]);                
                if (ui_get_string(name,32, "") == UI_RET_OK) {
                    if (parse_ip(name, gateway) == SYS_ERR) {
                        return;
                    };                      
                }            
                set_network_interface_config(INET_CONFIG_STATIC, ip, mask, gateway);
            }else {
                set_network_interface_config(INET_CONFIG_DHCP_FALLBACK, NULL, NULL, NULL);
            }
            
#if CFG_PERSISTENCE_SUPPORT_ENABLED
          persistence_save_current_settings("ethconfig");                
#endif
        }
#endif       
        if (c == 'f') {
            if (ui_get_string(name, 32, "Mac: ") == UI_RET_OK) {
#ifdef CFG_FACTORY_CONFIG_INCLUDED
                factory_config_get(&fcfg);
                if (parse_mac_address((const char*)name, fcfg.mac) == SYS_OK) {
                    factory_config_set(&fcfg);
                }

                sal_printf("\nPlease reboot the device to allow the new MAC address to take effect.\n");
#endif /* CFG_FACTORY_CONFIG_INCLUDED */
            }
        }
#ifdef CFG_PRODUCT_REGISTRATION_INCLUDED
        if (c == 'n') {
            if (ui_get_string(serial_num, 20, "Serial Number ") == UI_RET_OK) {
                set_serial_num(TRUE, serial_num);
            }
        }

        if (c == 'd') {
            get_system_mac((uint8 *)name);
            sal_printf("MAC address : %02bx-%02bx-%02bx-%02bx-%02bx-%02bx\n",
                name[0], name[1], name[2], name[3], 
                name[4], name[5]);
            get_serial_num(&valid, serial_num);
            if (valid) {
                sal_printf("Serial number : %s\n", serial_num);
            } else {
                sal_printf("Invalid serial number.\n");
            }
        }
#endif /* CFG_PRODUCT_REGISTRATION_INCLUDED */    
        
#if defined(CFG_NVRAM_SUPPORT_INCLUDED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED)
        if (c == 'g') {
            if (ui_get_string(name, 32, "Name: ") == UI_RET_OK) {
                str_ptr = nvram_get(name);
                if (str_ptr) {
                    sal_printf("Value: ");
                    nvram_show_tuple(str_ptr);
                }
            }
        } else if (c == 'l') {
             nvram_enum(nvram_show_tuple);
#ifndef __BOOTLOADER__
        } else if (c == 's') {
            if (ui_get_string(name, 32, "Name: ") == UI_RET_OK &&
                ui_get_string(value, 64, "Value: ") == UI_RET_OK) {
                nvram_set(name, value);
            }
        } else if (c == 'r') {
            if (ui_get_string(name, 32, "Name: ") == UI_RET_OK) {
                nvram_unset(name);
            }
        } else if (c == 'c') {
            sal_printf("The nvram variables you commit may cause persistence size changed.\n");
            sal_printf("System will erase the persisten section automatically.\n");
            sal_printf("Are you sure to continue (y or n): ");
            c = sal_getchar();
            if ((c == 'y') || (c == 'Y')) {
                nvram_commit();
                /* Erase persisten section */
                flash_erase(MEDIUM_FLASH_START_ADDRESS, MEDIUM_FLASH_SIZE);
                sal_printf("\n\nPlease reboot the device to allow the commit to take effect.\n");
            }
#endif
        }
#endif /* defined(CFG_NVRAM_SUPPORT_INCLUDED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED) */
#if defined(CFG_PERSISTENCE_SUPPORT_ENABLED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED)
        if (c == 'e') {
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
            sal_printf("  0: Erase persistencepartition \n"
                       "  1: Erase vendor config partition \n");
            r = ui_get_byte(&select, "select: ");
            if ((r == UI_RET_OK) && (select == 1)) {
                sal_printf("It may cause persistence size changed.\n");
                sal_printf("System will erase persisten section automatically.\n");
            }
#endif /*CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
            sal_printf("Are you sure to continue (y or n): ");
            c = sal_getchar();
            if ((c == 'y') || (c == 'Y')) {
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
                if (r == UI_RET_OK) {
                    if (select == 0) {
                        /* Erase persisten section */
                        flash_erase(MEDIUM_FLASH_START_ADDRESS, MEDIUM_FLASH_SIZE);
                    } else if (select == 1) {
                        /* Erase vendor config section */
                        flash_erase(NVRAM_BASE, NVRAM_SPACE);
                        /* Erase persisten section */
                        flash_erase(MEDIUM_FLASH_START_ADDRESS, MEDIUM_FLASH_SIZE);
                    } 
                }
#else
                /* Erase persisten section */
                flash_erase(MEDIUM_FLASH_START_ADDRESS, MEDIUM_FLASH_SIZE);
#endif /*CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
                sal_printf("\n\nPlease reboot the device immediately !\n");
            }
        }
#endif /* defined(CFG_PERSISTENCE_SUPPORT_ENABLED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED) */
    }
}

void
ui_flash_init(void) REENTRANT
{
    cli_add_cmd('F', cli_cmd_flash);
}

#endif /* CFG_CLI_ENABLED && CFG_FLASH_SUPPORT_ENABLED */

