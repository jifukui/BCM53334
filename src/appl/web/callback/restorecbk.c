/*
 * $Id: restorecbk.c,v 1.4 Broadcom SDK $
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
#include "appl/httpd.h"
#include "appl/ssp.h"
#include "appl/mdns.h"
#include "appl/persistence.h"
#include "../content/sspmacro_system.h"
#include "../content/sspmacro_system_name.h"

/* Utility: show web page of error messages.
 *          Used only in SSP handlers. 
 *          Must return SSP_HANDLER_RET_MODIFIED after calling this.
 */
void webutil_show_error(SSP_HANDLER_CONTEXT *cxt, 
           SSP_PSMH psmem,
           const char *title,
           const char *message,
           const char *button,
           const char *action
           );
           
/* Common string used in webutil_show_error() */
extern const char err_button_retry[];
extern const char err_action_back[];


/*

      Callback Function of configure backup and restore page

*/

SSP_DATA_ENTRY sspfile_switch_config[1] = {
  { 0x80, 0, 0, 0, 0, NULL },
};

SSP_HANDLER_RETVAL 
ssphandler_config_data_save_handler(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT
{
    SSP_HANDLER_CONTEXT_EXT *pcxt = (SSP_HANDLER_CONTEXT_EXT *)cxt;
    void *cfg_data;
    char buf[64];
    uint8   ver[4];
    UNREFERENCED_PARAMETER(psmem);
    
    if (cxt->type == SSP_HANDLER_OPEN) {
          cfg_data = ssputil_psmem_get(psmem, ssphandler_config_data_save_handler);
          if (cfg_data == NULL) {
              cfg_data = ssputil_psmem_alloc(psmem,ssphandler_config_data_save_handler, MEDIUM_FLASH_SIZE + MEDIUM_FLASH_SIZE/2);
          }
          if (cfg_data == NULL) {
                sal_printf("Out of memory \n");
                webutil_show_error(
                     cxt, psmem,
                     "System",
                     "Out of memory. ",
                     err_button_retry,
                     "window.location.assign('restore.htm')");
                     /* We don't want to process it more */
                     /* cxt->flags = 0; */
                 return SSP_HANDLER_RET_MODIFIED;    
          }  
          ((char *)cfg_data)[0] = 0;
#if CFG_PERSISTENCE_SUPPORT_ENABLED          
          persistence_backup_to_memory(cfg_data);
#endif
          sspfile_switch_config[0].dataptr = cfg_data;
          sspfile_switch_config[0].param1 = sal_strlen(cfg_data);
    }
    

    if (cxt->type == SSP_HANDLER_SET_HEADER) {
   
      /* Copy strings to buffer to avoid banking issues */
      get_system_name(buf,63);
      board_firmware_version_get(ver, ver+1, ver+2, ver+3);

      sal_strcpy(ssputil_shared_buffer, "Content-Type");
      sal_sprintf(ssputil_shared_buffer + 20, "application/octet-stream\r\nContent-Disposition: attachment; filename=\"%s_%02d%02d%02d%02d.cfg\"", buf, ver[0], ver[1], ver[2], ver[3]);

      pcxt->url_data.string_pair.name = ssputil_shared_buffer;
      pcxt->url_data.string_pair.value = ssputil_shared_buffer + 20;
      pcxt->flags &= ~SSPF_SET_HEADER_H;
      return SSP_HANDLER_RET_MODIFIED;
    }

    if (cxt->type == SSP_HANDLER_CLOSE) {
          ssputil_psmem_free(psmem, ssphandler_config_data_save_handler);
    }


    return SSP_HANDLER_RET_INTACT;

}


#ifdef HTTPD_TIMER_SUPPORT
STATICCBK void
restore_timer_reboot(void *in_data) REENTRANT
{
    httpd_delete_timers_by_callback(restore_timer_reboot);

    board_reset(in_data);
    
}
#endif /* HTTPD_TIMER_SUPPORT */


static uint8 restore_in_progress = 0;
static uint32 restore_total_size;
static uint32 restore_current_received;

SSP_HANDLER_RETVAL 
ssphandler_config_data_restore_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT
{

   
    SSP_HANDLER_CONTEXT_EXT *cxte = (SSP_HANDLER_CONTEXT_EXT *)cxt;
    unsigned char *pbuff = NULL;
    static BOOL    hard_reset;

    if (cxt->type == SSP_HANDLER_CLOSE) {
            
            hard_reset = TRUE;
            if (restore_in_progress == 2) {
#ifdef HTTPD_TIMER_SUPPORT
            httpd_create_timer(3, restore_timer_reboot, (void *)&hard_reset);
#else /* HTTPD_TIMER_SUPPORT */
                /* For no HTTPD timer case, the specific design must be added here 
                 *  to reset device 
                 */
#endif /* HTTPD_TIMER_SUPPORT */
            }
            
            restore_in_progress = 0;
            return SSP_HANDLER_RET_INTACT;
    }
    
    if (cxt->type != SSP_HANDLER_FILE_UPLOAD) {
            return SSP_HANDLER_RET_INTACT;
    }
    
        /* The first call to this handler is the indication of file length */
    if (cxte->url_data.upload.buf == NULL && cxte->url_data.upload.index == -1) {
               
            /* Check if someone has already started */
            if (restore_in_progress) {
              //  return show_error(cxt, psmem, ERROR_ALLOC_MUTEX);
              return SSP_HANDLER_RET_INTACT;
            }

            restore_total_size = cxte->url_data.upload.length;
             
            /* Allocate sector buffer */
            pbuff = (unsigned char *)ssputil_psmem_alloc(
                        psmem, 
                        ssphandler_config_data_restore_cgi, 
                        restore_total_size + 4
                        );
            if (pbuff == NULL) {
                sal_printf("Out of memory \n");
                webutil_show_error(
                     cxt, psmem,
                     "System",
                     "Out of memory. ",
                     err_button_retry,
                     "window.location.assign('restore.htm')");
                     /* We don't want to process it more */
                     /* cxt->flags = 0; */
                 return SSP_HANDLER_RET_MODIFIED;    
            }
            
            restore_in_progress = 1;

            restore_current_received = 0;

            return SSP_HANDLER_RET_INTACT;
            
     } else if (cxte->url_data.upload.buf == NULL && cxte->url_data.upload.length == 0) {

            /* Check if someone has already started */
            if (restore_in_progress == 0) {
              //  return show_error(cxt, psmem, ERROR_ALLOC_MUTEX);
              return SSP_HANDLER_RET_MODIFIED;
            }

            /* Get sector buffer previous allocated/updated */
            pbuff = (unsigned char *)ssputil_psmem_get(psmem, ssphandler_config_data_restore_cgi);
            
#if CFG_PERSISTENCE_SUPPORT_ENABLED
            if (persistence_validate_data_in_memory(pbuff) == FALSE) {

                webutil_show_error(
                    cxt, psmem,
                    "System",
                    "Wrong file format.",
                    err_button_retry,
                    "window.location.assign('restore.htm')");
                    /* We don't want to process it more */
                    /* cxt->flags = 0; */
                    return SSP_HANDLER_RET_MODIFIED;    
            }

            persistence_restore_from_memory(pbuff);

            persistence_save_all_current_settings();
#endif            
            /* persistence write done */
            ssputil_psmem_free(psmem, ssphandler_config_data_restore_cgi);
                
            /* Mark it as it'd completed successfully */
            restore_in_progress = 2;

            return SSP_HANDLER_RET_INTACT;   
      } else {
    
             /* Get sector buffer previous allocated/updated */
             pbuff = (unsigned char *)ssputil_psmem_get(psmem, ssphandler_config_data_restore_cgi);

             /* Check if someone has already started */
             if (restore_in_progress == 0) {
                 return SSP_HANDLER_RET_MODIFIED;
             }

             sal_memcpy(pbuff + cxte->url_data.upload.index, cxte->url_data.upload.buf, (size_t)cxte->url_data.upload.length);               
    
             restore_current_received += cxte->url_data.upload.length;

             pbuff[restore_current_received] = 0;
     }
             
    
     return SSP_HANDLER_RET_INTACT;

}


