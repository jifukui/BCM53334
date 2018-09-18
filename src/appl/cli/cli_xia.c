/*
 * $Id: cli.c,v 1.5 Broadcom SDK $
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
#pragma userclass (code = cli)
#endif /* CODE_USERCLASS */
#endif /* __C51__ */

#include "system.h"
#include "utils/ui.h"
#include "appl/cli.h"
#include "utils/net.h"
#if CFG_CLI_ENABLED

#define MAX_CLI_COMMANDS        (26+26+10)  /* Lower + upper + digit */

/* Commands forward */
APISTATIC void cli_cmd_list_commands(CLI_CMD_OP op) REENTRANT;
APISTATIC void cli_cmd_command_help(CLI_CMD_OP op) REENTRANT;
APISTATIC void cli_cmd_set_static_network(CLI_CMD_OP op) REENTRANT;
extern void APIFUNC(cli_init)(void) REENTRANT;

/* 
 * Command list 
 *
 * Note: We use STATIC initializer because commands could be added during
 *       system initialization phase.
 */
STATIC CLI_CMD_FUNC cmds[MAX_CLI_COMMANDS];

APISTATIC void
APIFUNC(cli_cmd_list_commands)(CLI_CMD_OP op) REENTRANT
{
    if (op == CLI_CMD_OP_HELP) {
        sal_printf("Help - List all available commands\n");
    } else if (op == CLI_CMD_OP_DESC) {
        sal_printf("List all commands");
    } else {
        uint8 i;
        CLI_CMD_FUNC *pcmd = &cmds[0];
        for(i=0; i<MAX_CLI_COMMANDS; i++, pcmd++) {
            if (*pcmd) {
                char c = i < 26 ? 
                    ('a' + i) : (i < 52? ('A' + i - 26) : '0' + i - 52);
                sal_printf("  %c - ", c);
                (*(*pcmd))(CLI_CMD_OP_DESC);
                sal_printf("\n");
            }
        }
    }
}

APISTATIC void
APIFUNC(cli_cmd_command_help)(CLI_CMD_OP op) REENTRANT
{
    if (op == CLI_CMD_OP_HELP) {
        sal_printf("Help - Show help information for a command\n");
    } else if (op == CLI_CMD_OP_DESC) {
        sal_printf("Help for a command");
    } else {
        char cmd;
        CLI_CMD_FUNC *pcmd;

        sal_printf("Command: ");
        cmd = sal_getchar();
        sal_printf("\n");
        if (cmd >= 'a' && cmd <= 'z') {
            pcmd = &cmds[cmd - 'a'];
        } else if (cmd >= 'A' && cmd <= 'Z') {
            pcmd = &cmds[26 + cmd - 'A'];
        } else if (cmd >= '0' && cmd <= '9') {
            pcmd = &cmds[52 + cmd - '0'];
        } else if (cmd == '\r' || cmd == '\n') {
            return;
        } else {
            sal_printf("Invalid command\n");
            return;
        }
        
        if (*pcmd == NULL) {
            sal_printf("Command not available\n");
            return;
        }

        sal_printf("\n");
        (*(*pcmd))(CLI_CMD_OP_HELP);
    }
}
APISTATIC void APIFUNC (cli_cmd_set_static_network)(CLI_CMD_OP op)REENTRANT
{
	uint8 ip[4]={192,168,20,115};
	uint8 mask[4]={255,255,0,0};
	uint8 gateway[4]={192,168,20,1};
	set_network_interface_config(1,ip,mask,gateway);
}
void
APIFUNC(cli)(void) REENTRANT
{
    for(;;) {
        char cmd;
        CLI_CMD_FUNC *pcmd;

        sal_printf("\n" CFG_CLI_PROMPT);
        cmd = sal_getchar();
        if (cmd >= 'a' && cmd <= 'z') {
            pcmd = &cmds[cmd - 'a'];
        } else if (cmd >= 'A' && cmd <= 'Z') {
            pcmd = &cmds[26 + cmd - 'A'];
        } else if (cmd >= '0' && cmd <= '9') {
            pcmd = &cmds[52 + cmd - '0'];
        } else if (cmd == UI_KB_LF || cmd == UI_KB_ESC ||
                   cmd == UI_KB_BS || cmd == UI_KB_CR) {
            ui_backspace();
            sal_putchar('\n');
            continue;
#ifdef __LINUX__			
        } else if (cmd == UI_KB_CTRL_C) {
            sal_reset(0);
#endif		  
        } else {
            sal_printf("\nInvalid command\n");
            continue;
        }
        
        if (*pcmd == NULL) {
            sal_printf("\nCommand not available\n");
            continue;
        }

        sal_printf(" - ");
        (*(*pcmd))(CLI_CMD_OP_DESC);
        sal_printf("\n");
        (*(*pcmd))(CLI_CMD_OP_EXEC);
    }
}

BOOL
APIFUNC(cli_add_cmd)(char cmd, CLI_CMD_FUNC func) REENTRANT
{
    CLI_CMD_FUNC *pcmd = NULL;
    
    SAL_ASSERT(func);
    if (func == NULL) {
        return FALSE;
    }
    
    if (cmd >= 'a' && cmd <= 'z') {
        pcmd = &cmds[cmd - 'a'];
    } else if (cmd >= 'A' && cmd <= 'Z') {
        pcmd = &cmds[26 + cmd - 'A'];
    } else if (cmd >= '0' && cmd <= '9') {
        pcmd = &cmds[52 + cmd - '0'];
    } else {
        SAL_ASSERT(FALSE);
        return FALSE;
    }
    
    SAL_ASSERT(*pcmd == NULL);
    if (*pcmd != NULL) {
        return FALSE;
    }
    
    *pcmd = func;
    return TRUE;
}

void
APIFUNC(cli_remove_cmd)(char cmd) REENTRANT
{
    CLI_CMD_FUNC *pcmd = NULL;
    
    if (cmd >= 'a' && cmd <= 'z') {
        pcmd = &cmds[cmd - 'a'];
    } else if (cmd >= 'A' && cmd <= 'Z') {
        pcmd = &cmds[26 + cmd - 'A'];
    } else if (cmd >= '0' && cmd <= '9') {
        pcmd = &cmds[52 + cmd - '0'];
    } else {
        SAL_ASSERT(FALSE);
        return;
    }
    
    *pcmd = NULL;
}

void
APIFUNC(cli_init)(void) REENTRANT
{
    uint8 i;
    for(i=0; i<MAX_CLI_COMMANDS; i++) {
        cmds[i] = NULL;
    }
    cli_add_cmd('h', cli_cmd_list_commands);
    cli_add_cmd('H', cli_cmd_command_help);
    //jifukui
    cli_add_cmd('J', cli_cmd_set_static_network);
}

#endif /* CFG_CLI_ENABLED */
