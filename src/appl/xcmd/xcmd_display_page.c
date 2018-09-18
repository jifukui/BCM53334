/*
 * $Id: xcmd_display_page.c,v 1.5 Broadcom SDK $
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


#define XCDEBUG 0

#if XCDEBUG
#define XCDEBUG_PRINT(...)  do { sal_printf("XCDEBUG:"); sal_printf(__VA_ARGS__); sal_printf("\n"); } while(0)  
#else
#define XCDEBUG_PRINT(str, ...)  
#endif 



/*  *********************************************************************
    *  xc_display_pages_write()
    *
    *  Console output function
    *
    ********************************************************************* */

int 
xc_display_pages_write(char *buffer,int length)
{
    int blen = length;
    char *bptr = buffer;
    while (blen > 0) {            
        put_char(*bptr);
        bptr++;
        blen--;
    }
    return 0;
}

#if CFG_CONSOLE_ENABLED
/*

   disp: the displaying pages context 
   buf : the string buffer 
   return:  the length of the line or segment 

*/
static int 
xc_display_pages_line_boundary(XCMD_DISP_PAGES_CONTEXT *disp, char *buf) {

    int pos = 0;
    
    /* search for \r \n 0 */
    while(1) {
            switch (buf[pos]) {
                case '\r':
                    if (buf[pos+1] == '\n') {
                        disp->has_cr = 1;
                        disp->rows ++;
                        return (pos + 2);
                    } else {
                        disp->has_cr = 1;
                        disp->rows ++;
                        return (pos + 1);
                    }
                case '\n': 
                    disp->has_cr = 1;
                    disp->rows ++;
                    return (pos + 1);
                case 0:
                    return pos;
                default:
                    disp->cols++;
                    if (XCMD_DISP_PAGE_COLS_PER_ROW == disp->cols) {
                        disp->cols = 0;
                        disp->rows ++;
                        return (pos + 1);
                    }
                break;
                     
            }             
            pos++;            
    }
}
#endif /* CFG_CONSOLE_ENABLED */
    
XCMD_ERROR 
xc_display_pages_getchar(XCMD_DISP_PAGES_CONTEXT *disp) {

    char ch;
    int exit = 0;//disp->non_stop;
    sal_printf("[Press key to continue, Press '?' to show key description  (page %02d)]", disp->page_no);
    char page_sel[5];
    int page_sel_index = 0;

    page_sel[0] = 0;
    
    while (exit == 0) {
        ch = get_char();
        if ((ch <= '9' && ch >= '0') && (page_sel_index < (sizeof(page_sel)-1))) {
             page_sel[page_sel_index++] = ch;
             page_sel[page_sel_index] = 0;
             sal_printf("%c", ch);
             continue;
        }
        switch (ch) {
            case 'a': // non-stop
            {
              disp->max_rows = XCMD_DISP_PAGE_ROWS_PER_PAGE;
              disp->state = XCMD_DISP_PAGES_STATE_NORMAL_OUTPUT;
              disp->non_stop = 1;
              exit = 1;
              break;
            }
            case ' ':
            case 'N': // next page
            {
              disp->state = XCMD_DISP_PAGES_STATE_NORMAL_OUTPUT;  
              disp->max_rows = XCMD_DISP_PAGE_ROWS_PER_PAGE;              
              exit = 1;
              break;
            }
            case 'P': // previous page
            {
               disp->max_rows = XCMD_DISP_PAGE_ROWS_PER_PAGE;
               if (disp->page_no == 1) {
                  disp->target_page_no = disp->page_no;
               } else {
                  disp->target_page_no = disp->page_no - 1;
               }
               disp->state = XCMD_DISP_PAGES_STATE_CANCEL;
               exit = 1;
               break;
            }
            case 0x1b:
            case 'Q':
            case 0x03:  // Ctrl-C
            {
                disp->max_rows = XCMD_DISP_PAGE_ROWS_PER_PAGE;
                disp->state = XCMD_DISP_PAGES_STATE_CANCEL;
                disp->target_page_no = 0;
                disp->non_stop = 1;
                exit = 1;           
                break;
            }
            case '\r': // Enter , Carrier Return
                disp->state = XCMD_DISP_PAGES_STATE_NORMAL_OUTPUT;  
                disp->max_rows = (disp->rows % XCMD_DISP_PAGE_ROWS_PER_PAGE) + 1;
                XCDEBUG_PRINT("disp->max_rows %x\n", disp->max_rows);
                exit = 1;           
            break; 
            case 'H':  // Goto the top of display page
                disp->target_page_no = 1;
                disp->non_stop = 0;
                disp->state = XCMD_DISP_PAGES_STATE_CANCEL;
                exit = 1;                
            break;    
            case 'R':  // Refresh Current Pages 
                disp->target_page_no = disp->page_no;
                disp->non_stop = 0;
                disp->state = XCMD_DISP_PAGES_STATE_CANCEL;
                exit = 1;                
            break;           
            case 'G':
                if (page_sel_index) {
                    disp->target_page_no = sal_strtoul(page_sel, 0, 10);
                    disp->non_stop = 0;
                    disp->state = XCMD_DISP_PAGES_STATE_CANCEL;
                    sal_printf("Goto page %d\n",  disp->target_page_no);
                    exit = 1;                                        
                }
            break;
            case '?':
                sal_printf("\n\n\n");
                sal_printf("List of key and function description\n\n");                
                sal_printf("    P : Display previous page.\n");
                sal_printf("    N or Space : Display next page.\n");
                sal_printf("    R : Refresh current page.\n");
                sal_printf("    H : Display from first page.\n");
                sal_printf("    a : Display remaining pages.\n");
                sal_printf("    Enter : Display next line.\n");
                sal_printf("    ESC, Q or CTRL+C : Escape from display mode.\n\n");
                sal_printf("    <page no>G : Goto selected page. Ex: goto page 6, you can type \"6G\" \n\n");
                sal_printf("[Press key to continue, Press '?' to show key description  (page %02d)]", disp->page_no);
                page_sel_index = 0;
                page_sel[0] = 0;
            break;
            default:
            break;            
         }
    }
    sal_printf("\r                                                                          \r");
    
    return XCMD_ERR_OK;
}



XCMD_ERROR
xc_display_pages(XCMD_HANDLE *xch, const char *fmt, ...)
{
#if CFG_CONSOLE_ENABLED
        va_list arg_ptr;
        char buf[256];
        char *ptr;
        int exit = 0;
        int len;
        XCMD_DISP_PAGES_CONTEXT* disp;
        va_start(arg_ptr, fmt);
        vsprintf(buf, fmt, arg_ptr);
        va_end(arg_ptr);

        if (xch == NULL) {
            return XCMD_ERR_OK;
        }
        
        disp = ((XCEXE_SESSION *) xch)->disp;

        if (disp == NULL) {
            return XCMD_ERR_OK;
        }      
        ptr = buf;

        do {
            switch (disp->state) {
                case XCMD_DISP_PAGES_STATE_CANCEL:
                     exit = 1;
                break;
                case XCMD_DISP_PAGES_STATE_DISABLE:
                {
                     disp->max_rows = XCMD_DISP_PAGE_ROWS_PER_PAGE + 1;
                     disp->state = XCMD_DISP_PAGES_STATE_NORMAL_OUTPUT;
                }
                case XCMD_DISP_PAGES_STATE_NORMAL_OUTPUT:
                {

                    /* get a line from buffer or a segment of text from buf */
                    len = xc_display_pages_line_boundary(disp, ptr);
                
                    if (len == 0) {
                        exit = 1;
                        return XCMD_ERR_OK;
                    }

                    /* output one line or segment into console and buffer */
                    xc_display_pages_write(ptr,len);

                                
                    if (disp->cols == 0 && disp->has_cr == 0) {
                        xc_display_pages_write("\n\r", 2);
                    }

                    if (disp->has_cr == 1) {
                        disp->cols = 0;
                        disp->has_cr = 0;
                    }
                    ptr = &ptr[len];


                    if (disp->max_rows == disp->rows) {
                        if (disp->non_stop == 0) {
                             xc_display_pages_getchar(disp);
                        }


                    }
                    if (disp->rows == XCMD_DISP_PAGE_ROWS_PER_PAGE) {                        
                        disp->page_no ++;
                        disp->rows = 0; 
                    }

                    break;
                }
                case XCMD_DISP_PAGES_STATE_SEEK:
                {     

                    if (disp->page_no == disp->target_page_no) {
                        disp->target_page_no = 0;
                        disp->state = XCMD_DISP_PAGES_STATE_NORMAL_OUTPUT;
                        break;
                    }
                    /* get a line from buffer or a segment of text from buf */
                    len = xc_display_pages_line_boundary(disp, ptr);
                
                    if (len == 0) {
                        exit = 1;
                        return XCMD_ERR_OK;
                    }


                    if (disp->has_cr == 1) {
                        disp->cols = 0;
                        disp->has_cr = 0;
                    }
                    ptr = &ptr[len];

                    if (disp->max_rows == disp->rows) {
                        if (disp->page_no == disp->target_page_no) {
                            disp->state = XCMD_DISP_PAGES_STATE_NORMAL_OUTPUT;
                        } 
                        disp->page_no ++;
                        disp->rows = 0;
                    }
                    break;
                }
                default:
                break;
            }          
        }while ( exit == 0 && (*ptr != 0));
#else
        UNREFERENCED_PARAMETER(fmt);
        UNREFERENCED_PARAMETER(xch);
#endif

     return XCMD_ERR_OK;


}

void 
xc_display_pages_start(XCMD_HANDLE *xch) {
    

    XCMD_DISP_PAGES_CONTEXT *disp;

    
    if (xch == NULL) {
        return;
    }

    disp = ((XCEXE_SESSION *) xch)->disp;

    if (disp == NULL) {
        disp = ((XCEXE_SESSION *) xch)->disp = xcmd_alloc(sizeof(XCMD_DISP_PAGES_CONTEXT),"disp page context");
        if (disp == NULL) {
              return;
        }
    }

    disp->state = XCMD_DISP_PAGES_STATE_NORMAL_OUTPUT;
    disp->cols = 0;
    disp->has_cr = 0;
    disp->rows = 0;
    disp->max_cols_per_row = XCMD_DISP_PAGE_COLS_PER_ROW;
    disp->max_rows = XCMD_DISP_PAGE_ROWS_PER_PAGE;
    disp->page_no = 1;
    disp->target_page_no = 0;
    disp->non_stop = 0;

    
}

XCMD_ERROR 
xc_display_pages_end(XCMD_HANDLE *xch) {

    XCMD_DISP_PAGES_CONTEXT * disp;

    if (xch == NULL) {
        return XCMD_ERR_OK;
    }

    disp = ((XCEXE_SESSION *) xch)->disp;

    if (disp == NULL) {
        return XCMD_ERR_OK;
    }

    if (disp->state == XCMD_DISP_PAGES_STATE_SEEK) { // seek not found !!
        disp->state = XCMD_DISP_PAGES_STATE_NORMAL_OUTPUT;       
        disp->cols = 0;
        disp->has_cr = 0;
        disp->rows = 0; 
        disp->max_cols_per_row = XCMD_DISP_PAGE_COLS_PER_ROW;
        disp->max_rows = XCMD_DISP_PAGE_ROWS_PER_PAGE;
        disp->page_no = 1;
        disp->non_stop = 0;
        return XCMD_ERR_FALSE;
    }
    
    if (disp->target_page_no) { 
        disp->state = XCMD_DISP_PAGES_STATE_SEEK;       
        disp->cols = 0;
        disp->has_cr = 0;
        disp->rows = 0;
        disp->max_cols_per_row = XCMD_DISP_PAGE_COLS_PER_ROW;
        disp->max_rows = XCMD_DISP_PAGE_ROWS_PER_PAGE;
        disp->page_no = 1;
        disp->non_stop = 0;
        return XCMD_ERR_FALSE;
    }

    xcmd_free(disp);
    ((XCEXE_SESSION *) xch)->disp = NULL;
    
    sal_printf("\nEnd of Displaying page\n");
    return XCMD_ERR_OK;
}

#endif /* CFG_XCOMMAND_INCLUDED */
