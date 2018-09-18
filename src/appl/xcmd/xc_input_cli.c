/*
 * $Id: xc_input_cli.c,v 1.3 Broadcom SDK $
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
 
#ifdef CFG_XCOMMAND_INCLUDED

#include "appl/xcmd/xcmd_internal.h"
#include "xcmd_core.h"
#include "xc_input_cli.h"

#define THIS ((XCIN_CLI *)pstream)

#define CYG_CMD_BUF_SIZE               256
#define CYG_MAX_CMDS                   10
#define SHELL_TIMEOUT                  400
#define CYG_CONSOLE_TIMEOUT_MS         300
enum {
    RET_FIND_MATCH = -1,
    RET_TAB_COMPLETE = -2,
    RET_ERR = 0,
    RET_OK,
};


static BOOL 
cyg_console_read_char(char* c)
{

    *c = get_char();

    return TRUE;

}

static void 
cyg_console_write_char(char c)
{

    put_char(c);
    
    return;
}


static int 
cyg_console_read_char_with_timeout(char *c, int msec) {

    int i;

    for (i = msec; i >= 0; i--) {
        if (cyg_console_read_char(c) == TRUE) {
            break;
        }
        sal_usleep(1000);
    } 

    if ( i < 0) {
        return FALSE;    
    }

    return TRUE;

}

static int
cyg_shell_cmd_get(char* cmd, uint32_t size, BOOL reuse)
{
    char c;
    int ret = RET_ERR;
    static char cmd_cache[CYG_MAX_CMDS][CYG_CMD_BUF_SIZE];
    static int lst_cmd_idx = -1;
    static int ttl_cmd_idx = -1;
    static char insert_on = 1;
    char *curidx, *endidx, *tmpidx;
    int cur_cmd_idx = lst_cmd_idx;
    
    /*
         * XXX: length checking (both cmd/size and CYG_CMD_BUF_SIZE)
         */
    
    curidx = endidx = cmd;
    
    /* FINDMATCH */
    if (reuse) {
        while (*curidx != '\0')
            cyg_console_write_char(*curidx++);
        *curidx = 0;
        endidx = curidx;
    }
    
    for(;;) {

        if (cyg_console_read_char(&c) != TRUE)
            continue;

        *endidx = '\0';
        
        switch (c)
        {
        case 0xb: /* Ctrl-K */
            tmpidx = curidx;
            while(tmpidx != endidx) {
               cyg_console_write_char(' ');
               tmpidx++;
            }
            while(curidx != endidx) {
               cyg_console_write_char('\b');
               endidx--;
            }
        break;    
            
        /* FINDMATCH */
        case '?':
            cyg_console_write_char('\r');
            cyg_console_write_char('\n');
            ret = RET_FIND_MATCH;
            break;
        
        /* AUTOCOMPLETE */
        case '\t':
            cyg_console_write_char('\r');
            cyg_console_write_char('\n');
            ret = RET_TAB_COMPLETE;
            break;
        
        case '\n':
        case '\r':
            cyg_console_write_char('\r');
            cyg_console_write_char('\n');
            if (curidx != cmd) {
                if (++lst_cmd_idx == CYG_MAX_CMDS)
                    lst_cmd_idx = 0;
                if (lst_cmd_idx > ttl_cmd_idx)
                    ttl_cmd_idx = lst_cmd_idx;
                
                sal_strcpy(cmd_cache[lst_cmd_idx], cmd);
            }
            ret = RET_OK;
            break;
            
        case '\b': /* backspace */
            if (curidx != cmd) {
                if (curidx != endidx)
                {
                    curidx--;
                    cyg_console_write_char('\b');
                    tmpidx = curidx;
                    while (tmpidx != endidx -1)
                    {
                        *tmpidx = *(tmpidx + 1);
                        cyg_console_write_char(*tmpidx++);
                    }
                    cyg_console_write_char(' ');
                    cyg_console_write_char('\b');
                    while (tmpidx-- != curidx)
                        cyg_console_write_char('\b');
                    endidx--;
                } else {
                    cyg_console_write_char('\b');
                    cyg_console_write_char(' ');
                    cyg_console_write_char('\b');
                    curidx--; endidx--;
                }
            } 
            break;
        case 0x7F: /* delete */
             if (curidx != endidx) {
                 tmpidx = curidx;
                 while (tmpidx != endidx -1) {
                     *tmpidx = *(tmpidx + 1);
                     cyg_console_write_char(*tmpidx++);
                 }
                 cyg_console_write_char(' ');
                 cyg_console_write_char('\b');
                 while (tmpidx-- != curidx) {
                     cyg_console_write_char('\b');
                 }
                 endidx--;
            } 
            break;
        case 0x1b: /* ESC */

            if (cyg_console_read_char_with_timeout(&c, CYG_CONSOLE_TIMEOUT_MS) == FALSE) {
                break;
            }
            if (c == 0x5b) {

                if (cyg_console_read_char_with_timeout(&c, CYG_CONSOLE_TIMEOUT_MS) == FALSE) {
                     break;
                }


                switch (c)
                {
                case 'A': /* up arrow */
                    if (cur_cmd_idx >= 0)
                    {
                        while(curidx != endidx) {
                            cyg_console_write_char(' ');
                            curidx++;
                        }

                        while(curidx != cmd)
                        {
                            cyg_console_write_char('\b');
                            cyg_console_write_char(' ');
                            cyg_console_write_char('\b');
                            curidx--;
                        }
                        sal_strcpy(cmd, cmd_cache[cur_cmd_idx]);
                        while (*curidx)
                            cyg_console_write_char(*curidx++);
                        endidx = curidx;
                        if (--cur_cmd_idx < 0)
                            cur_cmd_idx = ttl_cmd_idx;
                    }
                    break;
                    
                case 'B': /* down arrow */
                    if (cur_cmd_idx >= 0)
                    {
                        if (++cur_cmd_idx > ttl_cmd_idx)
                            cur_cmd_idx = 0;

                        while(curidx != endidx) {
                            cyg_console_write_char(' ');
                            curidx++;
                        }

                        while(curidx != cmd)
                        {
                            cyg_console_write_char('\b');
                            cyg_console_write_char(' ');
                            cyg_console_write_char('\b');
                            curidx--;
                        }
                        sal_strcpy(cmd, cmd_cache[cur_cmd_idx]);
                        while (*curidx)
                            cyg_console_write_char(*curidx++);
                        endidx = curidx;                        
                    }
                    break;

                case 'C': /* right arrow */
                    if (curidx != endidx)
                        cyg_console_write_char(*curidx++);
                    break;                                      
                    
                case 'D': /* left  arrow */
                    if (curidx != cmd)
                    {                       
                        cyg_console_write_char('\b');
                        curidx--;
                    }                   
                    break; 
                /* for console is non-VT100 or ANSI mode */    
                case '1': /* home */                    
                {
                    if (cyg_console_read_char_with_timeout(&c, CYG_CONSOLE_TIMEOUT_MS) == FALSE &&
                        c == 0x7e ) {
                        break;
                    }
                    while(curidx != cmd) {
                          cyg_console_write_char('\b');
                          curidx--;
                    }
                    break;
                }    

                case '2': /* Insert */
                {
                    if (cyg_console_read_char_with_timeout(&c, CYG_CONSOLE_TIMEOUT_MS) == FALSE &&
                        c == 0x7e ) {
                        break;
                    }

                    if (insert_on == 0) {
                        insert_on = 1;
                    } else {
                        insert_on = 0;
                    }

                    break;
                }    
                case '3': /* delete */
                {
                    if (cyg_console_read_char_with_timeout(&c, CYG_CONSOLE_TIMEOUT_MS) == FALSE &&
                        c == 0x7e ) {
                        break;
                    }
                    if (curidx != endidx)
                    {
                        tmpidx = curidx;
                        while (tmpidx != endidx -1)
                        {
                          *tmpidx = *(tmpidx + 1);
                          cyg_console_write_char(*tmpidx++);
                        }
                        cyg_console_write_char(' ');
                        cyg_console_write_char('\b');
                        while (tmpidx-- != curidx)
                            cyg_console_write_char('\b');
                        endidx--;
                    }                
                    break;
                }
                case '4': /* End */
                {
                    if (cyg_console_read_char_with_timeout(&c, CYG_CONSOLE_TIMEOUT_MS) == FALSE &&
                        c == 0x7e ) {
                        break;
                    }
                    while (curidx != endidx) {
                          cyg_console_write_char(*curidx++);
                    }
                    break;
                }
                case '5': /* Page Up */
                case '6': /* Page Down */
                    if (cyg_console_read_char_with_timeout(&c, CYG_CONSOLE_TIMEOUT_MS) == FALSE &&
                        c == 0x7e ) {
                        break;
                    }
                    break;
                default:
                    break;
                }
            }
            break;  
        default:
            if (curidx != endidx && insert_on == 1)
            {

                tmpidx = endidx;                
                *++endidx = '\0';
                while (tmpidx !=  curidx)
                {
                    *tmpidx = *(tmpidx -1);
                    --tmpidx;
                }
            }
            
            cyg_console_write_char(c);

            if (curidx != endidx)
            {
                *curidx++ = c;
                /* insert off case */
                tmpidx = curidx;
                while (tmpidx != endidx)
                    cyg_console_write_char(*tmpidx++);
                while (tmpidx-- != curidx)
                    cyg_console_write_char('\b');                   
            } else {
                *curidx = c;
                curidx++; 
                endidx++;
            }
            break;
            
        }

        if (curidx == cmd + size - 1)
        {
            *curidx = '\0';
            ret = RET_OK;
        }
        if (ret == RET_OK || ret == RET_FIND_MATCH || ret == RET_TAB_COMPLETE)
            return ret;
    }
    return ret;
}



static XCMD_ERROR
get_line(void *pstream, const char **pline, unsigned int *plen, XCMDI_OP *pop)
{
    BOOL linef;
    
    if (pstream == NULL || pline == NULL || plen == NULL || pop == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_STREAM_CALL;
    }
    
    if (THIS->depth < 0) {
        return XCMD_ERR_INTERNAL_INVALID_STREAM_CALL;
    }
    
    linef = (*pop == XCMDI_OP_FINDMATCH);
    
    for(;;) {
        
        unsigned int i;
        char *str;
        int bf;
        
        /* FINDMATCH && AUTOCOMPLETE */
        if (linef) {
            cyg_console_write_char('\r');
            cyg_console_write_char('\n');
            linef = FALSE;
        }
        
        sal_printf(THIS->prompt);
        bf = cyg_shell_cmd_get(
                &THIS->buffer[THIS->curpos], 
                sizeof(THIS->buffer) - THIS->curpos,
                (*pline != NULL)
                );

        if (bf == RET_ERR) {
            *plen = 0;
            return XCMD_ERR_EXIT;
        }
        
        *plen = sal_strlen(&THIS->buffer[THIS->curpos]);
        if (bf == RET_OK && *plen == 0) {
            continue;
        }

        /* Skip leading spaces */
        i = 0;
        str = &THIS->buffer[THIS->curpos];
        while(str[i] == ' ') {
            i++;
        }
        
        /* Check if it's comment */
        if (str[i] == DEFAULT_CLI_COMMENT_BEGIN) {
            if (bf == RET_OK) {
                *pline = NULL;
            }
            continue;
        }

        /* Check empty line */
        if (bf == RET_OK && str[i] == '\0') {
            *pline = NULL;
            continue;
        }
        
        *pline = &THIS->buffer[THIS->curpos];
        *pop = XCMDI_OP_EXECUTE;
        
        /* FINDMATCH */
        if (bf == RET_FIND_MATCH) {
            *pop = XCMDI_OP_FINDMATCH;
        }

        /* AUTOCOMPLETE */
        if (bf == RET_TAB_COMPLETE) {
            /*
             * The length returned by plen is the buffer size,
             * while the current command input is terminated by '\0'.
             */
            *plen = sizeof(THIS->buffer) - THIS->curpos;

            *pop = XCMDI_OP_COMPLETE;
        }

        break;
    }

    return XCMD_ERR_OK;
}

static XCMD_ERROR
report_error(void *pstream, XCMD_ERROR err, const char *line, unsigned int len)
{
    /* 
     * Error handing
     * - Common strings
     * - Config-file stream error: let developer know
     */
    switch (err) {
    case XCMD_ERR_INTERNAL_INVALID_PRIVILEGE_LEVEL:
    case XCMD_ERR_INVALID_COMMAND_INPUT:
        sal_printf("%% Error: Invalid command input\n");
        break;
    case XCMD_ERR_INCOMPLETE_COMMAND_INPUT:
        sal_printf("%% Error: Incomplete command\n");
        break;
    case XCMD_ERR_INCORRECT_PARAM_LENGTH:
        sal_printf("%% Error: Incorrect length of parameter\n");
        break;
    case XCMD_ERR_INCORRECT_PARAM_RANGE:
        sal_printf("%% Error: Parameter out of range\n");
        break;
    default:
        sal_printf("%% Error: code %d\n", err);
    }
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
open(void *pstream, const char *prompt)
{
    if (pstream == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    if (prompt == NULL) {
        prompt = "Switch> ";
    }
    sal_strncpy(
        &THIS->buffer[THIS->curpos], 
        prompt, 
        sizeof(THIS->buffer) - THIS->curpos
        );
    THIS->prompt = &THIS->buffer[THIS->curpos];
    THIS->stack[THIS->count++] = sal_strlen(prompt) + 1;
    THIS->curpos += sal_strlen(prompt) + 1;
    
    THIS->depth++;

    return XCMD_ERR_OK;
}

static XCMD_ERROR
close(void *pstream)
{
    unsigned int len;
    if (pstream == NULL || THIS->depth < 0) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    THIS->depth--;

    len = THIS->stack[--THIS->count];
    THIS->curpos -= len;
    if (THIS->depth < 0) {
        THIS->prompt = NULL;
    } else {
        THIS->prompt -= THIS->stack[THIS->count - 1];
    }    
    
    return XCMD_ERR_OK;
}

XCMD_ERROR
xcin_cli_init(XCIN_CLI *ps)
{
    if (ps == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    ps->base.open = open;
    ps->base.get_line = get_line;
    ps->base.report_error = report_error;
    ps->base.close = close;

    ps->buffer[0] = 0;
    ps->curpos = 0;
    ps->count = 0;
    ps->prompt = NULL;
    ps->depth = -1;
    
    return XCMD_ERR_OK;
}

#endif /* CFG_XCOMMAND_INCLUDED */
