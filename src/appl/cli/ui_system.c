/*
 * $Id: ui_system.c,v 1.18 Broadcom SDK $
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

#ifdef __C51__
#ifdef CODE_USERCLASS
#pragma userclass (code = uisys)
#endif /* CODE_USERCLASS */
#endif /* __C51__ */

#ifdef __C51__
#include "8051.h"
#endif  /* __C51__ */

#include "system.h"
#include "appl/cli.h"
#include "utils/ui.h"
#include "utils/net.h"

#if CFG_CLI_ENABLED

/* Forwards */
APISTATIC void cli_cmd_show_tick(CLI_CMD_OP op) REENTRANT;
APISTATIC void cli_cmd_memory_dump(CLI_CMD_OP op) REENTRANT;
APISTATIC void cli_cmd_memory_edit(CLI_CMD_OP op) REENTRANT;
#if CFG_UIP_STACK_ENABLED
APISTATIC void cli_cmd_access_ctrl(CLI_CMD_OP op) REENTRANT;
#endif
#ifdef __C51__
APISTATIC void cli_cmd_8051_sfr_get(CLI_CMD_OP op) REENTRANT;
#endif /* __C51__ */

extern void APIFUNC(ui_system_init)(void) REENTRANT;

APISTATIC void
APIFUNC(cli_cmd_show_tick)(CLI_CMD_OP op) REENTRANT
{
    if (op == CLI_CMD_OP_HELP) {
        sal_printf("Ticks - Show current system ticks.\n");
    } else if (op == CLI_CMD_OP_DESC) {
        sal_printf("Show current system ticks");
    } else {
        sal_printf("System ticks: %lu / 0x%lX\n",
            (uint32)sal_get_ticks(), (uint32)sal_get_ticks());
        sal_printf("  - Every tick is %lu micro seconds\n",
            sal_get_us_per_tick());
    }
}

void
APISTATIC APIFUNC(cli_cmd_memory_dump)(CLI_CMD_OP op) REENTRANT
{
    if (op == CLI_CMD_OP_HELP) {
        sal_printf("This command displays data from memory as bytes in hex.\n"
                    "ASCII text, if present, will appear to the right of "
                    "the hex data.\n"
                    "To stop dumping, press ESC or Ctrl-C.\n");
    } else if (op == CLI_CMD_OP_DESC) {
        sal_printf("Dump memory");
    } else {
        uint8 *addr;
        uint16 len;
        if (ui_get_address(&addr, "Address: ") == UI_RET_OK &&
            ui_get_word(&len, "Length: ") == UI_RET_OK &&
            len > 0) {

            if (len > 16) {
                sal_printf(" Press ESC or Ctrl-C to stop.");
            }
            ui_dump_memory(addr, len);
        }
    }
}

APISTATIC void
APIFUNC(cli_cmd_memory_edit)(CLI_CMD_OP op) REENTRANT
{
    if (op == CLI_CMD_OP_HELP) {
        sal_printf("This command edits the contents of memory in a byte "
                    "by byte manner.\n"
                    "Press ESC or Ctrl-C to quit editing.\n");
    } else if (op == CLI_CMD_OP_DESC) {
        sal_printf("Edit contents of memory");
    } else {
        uint8 *addr;
        uint8 val;
        ui_ret_t r;
        if (ui_get_address(&addr, "Address: ") == UI_RET_OK) {
            sal_printf(" Press ESC or Ctrl-C to quit editing.\n");
            for(;; addr++) {
                sal_printf(" %08lX [%02bX]: ",
                    (uint32)DATAPTR2MSADDR(addr), *addr);
                r = ui_get_byte(&val, NULL);
                if (r == UI_RET_OK) {
                    *(volatile uint8 XDATA *)addr = val;
                } else if (r == UI_RET_EMPTY) {
                    continue;
                } else {
                    break;
                }
            }
        }
    }
}

#if CFG_UIP_STACK_ENABLED
APISTATIC void
APIFUNC(cli_cmd_access_ctrl)(CLI_CMD_OP op) REENTRANT
{
    char c;

    if (op == CLI_CMD_OP_HELP) {
        sal_printf("Access Control/Admin Privilege configuration.\n");
    } else if (op == CLI_CMD_OP_DESC) {
        sal_printf("Acces Control/Admin Privilege");
    } else {
        sal_printf("  d - Not to set access control IP address\n"
                   "  g - Get current access control IP address\n"
                   "  m - Not to limit single user access\n"
                   "Enter your choice: ");
        c = sal_getchar();
        sal_putchar('\n');
        if (c == 'd') {
            BOOL valid;
            
            valid = FALSE;
            set_accessip_addr(valid, 0, NULL, NULL);
            sal_printf("Set unlimited access control IP limitation");
        }
        if (c == 'g') {
            BOOL acc_valid = FALSE;
            int acc_num = 0;
            uint8 accessctrlip[MAX_ACCESSCONTROL_IP][4], accmaskip[MAX_ACCESSCONTROL_IP][4];
            int i;
        
            get_accessip_addr(&acc_valid, &acc_num, accessctrlip, accmaskip);
            if (acc_valid == TRUE) {
                sal_printf("\n Access Control: ");
                for (i = 0; i < acc_num; i++) {
                    sal_printf("\nIP %d:%03d.%03d.%03d.%03d---MASK:%03d.%03d.%03d.%03d",i, 
                    accessctrlip[i][0],accessctrlip[i][1],accessctrlip[i][2],
                    accessctrlip[i][3],accmaskip[i][0],accmaskip[i][1],
                    accmaskip[i][2],accmaskip[i][3]);
                }
            } else {
                sal_printf("No access IP limitation");
            }
        }
        if (c == 'm') {
            BOOL valid;
            
            valid = FALSE;
            set_adminpv(valid);
            sal_printf("Not to limit single user access");
        }
    }
}
#endif /* CFG_UIP_STACK_ENABLED */

#ifdef __C51__
APISTATIC void
APIFUNC(cli_cmd_8051_sfr_get)(CLI_CMD_OP op) REENTRANT
{
    if (op == CLI_CMD_OP_HELP) {
        sal_printf("This command displays 8051 SFRs (Special Function "
                   "Registers).\n");
    } else if (op == CLI_CMD_OP_DESC) {
        sal_printf("Show 8051 SFRs");
    } else {
        sal_printf(" IEN0 / IE: %02bX\n", IE);
        sal_printf(" IEN1 / IP: %02bX\n", IP);
        sal_printf(" IEN2:      %02bX\n", IEN2);
        sal_printf(" IP0:       %02bX\n", IP0);
        sal_printf(" IP1:       %02bX\n", IP1);
        sal_printf(" PSW:       %02bX\n", PSW);
        sal_printf(" TCON:      %02bX\n", TCON);
        sal_printf(" TMOD:      %02bX\n", TMOD);
        sal_printf(" SCON:      %02bX\n", SCON);
        sal_printf(" PCON:      %02bX\n", PCON);
    }
}
#endif /* __C51__ */

#ifdef CFG_DUAL_IMAGE_INCLUDED
APISTATIC void
APIFUNC(cli_cmd_dual_image)(CLI_CMD_OP op) REENTRANT
{
    uint32 partition;
    uint8  ver[4];
    flash_imghdr_t *hdr;

    if (op == CLI_CMD_OP_HELP) {
        sal_printf("This command displays status of active and backup images.\n");
    } else if (op == CLI_CMD_OP_DESC) {
        sal_printf("Show dual image info");
    } else {
        partition = board_active_image_get();
        board_firmware_version_get(ver, ver+1, ver+2, ver+3);
        
        sal_printf("Image   Version   Active \n"
                   "-------------------------\n");

        if (1 == ACTIVE_IMAGE_GET(partition)) {
            sal_printf("  1     %u.%u.%u       Y\n", ver[0], ver[1], ver[2]);
            if (IS_BACKUP_IMAGE_VALID(partition)) {
                hdr = (flash_imghdr_t *)BOARD_SECONDARY_FIRMWARE_ADDR;
                sal_printf("  2     %c.%c.%c       N\n",
                            hdr->majver, hdr->minver, hdr->ecover);
            } else {
                sal_printf("  2     NA          -\n");
            }
        } else {
            if (IS_BACKUP_IMAGE_VALID(partition)) {
                hdr = (flash_imghdr_t *)BOARD_FIRMWARE_ADDR;
                sal_printf("  1     %c.%c.%c       N\n",
                            hdr->majver, hdr->minver, hdr->ecover);
            } else {
                sal_printf("  1     NA          -\n");
            }
            sal_printf("  2     %u.%u.%u       Y\n", ver[0], ver[1], ver[2]);
        }
    }
}
#endif /* CFG_DUAL_IMAGE_INCLUDED */

#ifdef CFG_RESET_BUTTON_INCLUDED
APISTATIC void
APIFUNC(cli_cmd_reset_button)(CLI_CMD_OP op) REENTRANT
{
    char c;
    uint8 uint8_temp;
    
    if (op == CLI_CMD_OP_HELP) {
        sal_printf("This command provide the test for reset button.\n");
    } else if (op == CLI_CMD_OP_DESC) {
        sal_printf("Test for reset button");
    } else {
        sal_printf("  d - Dump information\n"
                   "  B - Enable GPIO BIT for reset button\n"
                   "  P - Set Polarity for reset button\n"
                   "  S - Simulate reset button is pressed by SW\n"
                   "Enter your choice: ");
        c = sal_getchar();
        sal_putchar('\n');
        if (c == 'd') {
            sal_printf("Reset Button information :\n");
            if (reset_button_enable) {
                sal_printf("  - Function is enabled\n");
            } else {
                sal_printf("  - Function is disable\n");
            }
            sal_printf("  - Using GPIO Bit %d\n", reset_button_gpio_bit);
            if (reset_button_active_high) {
                sal_printf("  - Active hight\n");
            } else {
                sal_printf("  - Active low\n");
            }
        }

        if (c == 'B') {
            sal_printf("Input which bit for reset button\n");
            if (ui_get_byte(&uint8_temp, "BIT (0 ~ 7): ") == UI_RET_OK) {
                reset_button_gpio_bit = uint8_temp;
            }
        }

        if (c == 'P') {
            sal_printf("0: Active low\n");
            sal_printf("1: Active high\n");
            if (ui_get_byte(&uint8_temp, "Please select: ") == UI_RET_OK) {
                if (uint8_temp == 0) {
                    reset_button_active_high = 0;
                } else if (uint8_temp == 1) {
                    reset_button_active_high = 1;
                }
            }

        }

        if (c == 'S') {
            sal_printf("Input seconds for sw_simulate_press_reset_button_duration\n");
            if (ui_get_byte(&uint8_temp, "Duration (1 or 2): ") == UI_RET_OK) {
                sw_simulate_press_reset_button_duration = uint8_temp;
            }
        }
    }
}
#endif /* CFG_RESET_BUTTON_INCLUDED */

void
APIFUNC(ui_system_init)(void) REENTRANT
{
    cli_add_cmd('T', cli_cmd_show_tick);
    cli_add_cmd('d', cli_cmd_memory_dump);
    cli_add_cmd('e', cli_cmd_memory_edit);
#if CFG_UIP_STACK_ENABLED     
    cli_add_cmd('a', cli_cmd_access_ctrl);
#endif
#ifdef __C51__
    cli_add_cmd('f', cli_cmd_8051_sfr_get);
#endif /* __C51__ */
#ifdef CFG_DUAL_IMAGE_INCLUDED
    cli_add_cmd('D', cli_cmd_dual_image);
#endif /* CFG_DUAL_IMAGE_INCLUDED */
#ifdef CFG_RESET_BUTTON_INCLUDED
    cli_add_cmd('B', cli_cmd_reset_button);
#endif /* CFG_RESET_BUTTON_INCLUDED */
}

#endif /* CFG_CLI_ENABLED */
