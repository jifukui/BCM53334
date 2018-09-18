/*
 * $Id: fupgcbk_fw.c,v 1.16 Broadcom SDK $
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

#include "appl/ssp.h"
#include "appl/httpd.h"
#include "../content/sspmacro_upgrade.h"
#include "uip.h"

#define BYTES_PER_WRITE  (4096)    

/* Upgrade protection and status */
#ifdef __BOOTLOADER__
static int upgrade_in_progress = 0;
static int32 current_received = 0;
static int32 total_size = 0;
static hsaddr_t base_addr = 0;
static hsaddr_t base_addr_start = 0;
const char *error_str = NULL;
#endif /* __BOOTLOADER__ */

enum 
{
    ERROR_INIT_FAIL,
    ERROR_INVALID_FIRMWARE,
    ERROR_INVALID_FIRMWARE2,
    ERROR_WRITE_FAIL,
    ERROR_ALLOC_MUTEX,
    ERROR_ALLOC_FAIL
};

#ifdef __BOOTLOADER__

extern void uip_arp_update(u16_t *ipaddr, struct uip_eth_addr *ethaddr);

SSP_HANDLER_RETVAL ssphandler_fupgrade_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
SSP_HANDLER_RETVAL ssphandler_fupgrade_htm(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT;
void sspvar_upgrade_tag_info(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT;


STATICFN SSP_HANDLER_RETVAL
show_error(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem, int error) 
{
    const char *str = NULL;
    
    /* fix build complain */
    UNREFERENCED_PARAMETER(psmem);
    UNREFERENCED_PARAMETER(cxt);
    UNREFERENCED_PARAMETER(str);    
    
    switch(error) {
    case ERROR_INIT_FAIL: 
        str = "Failed to detect flash memory type!"; 
        break;
    case ERROR_INVALID_FIRMWARE: 
        str = "Invalid firmware image! Please double check!"; 
        break;
    case ERROR_INVALID_FIRMWARE2: 
        str = "Invalid firmware image has been written! "
              "Please update a correct firmware immediately!"; 
        break;
    case ERROR_WRITE_FAIL: 
        str = "Failed to erase/write flash memory!"; 
        break;
    case ERROR_ALLOC_FAIL: 
        str = "Internal server error! Failed to allocate memory!"; 
        break;
    case ERROR_ALLOC_MUTEX: 
        str = "An upgrade has been already started (by someone else)!"; 
        break;
    default: 
        str = "Internal server error!"; 
        break;
    }

    sal_printf("%s\n", str);

    error_str = str;

    upgrade_in_progress = -1;
    
    return SSP_HANDLER_RET_MODIFIED;
}

#ifdef HTTPD_TIMER_SUPPORT
STATICCBK void
fupgrade_timer_reboot(void *in_data) REENTRANT
{
    httpd_delete_timers_by_callback(fupgrade_timer_reboot);

    board_reset(in_data);
    
}
#endif /* HTTPD_TIMER_SUPPORT */

SSP_HANDLER_RETVAL
ssphandler_fupg_abort_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT
{
    BOOL    hard_reset;

    UNREFERENCED_PARAMETER(psmem);
    UNREFERENCED_PARAMETER(cxt);

    /* Clear bookkeeping data here. */
    
    hard_reset = TRUE;

    /* Clear bookkeeping data here. */
    board_loader_mode_get(NULL, TRUE);

#ifdef HTTPD_TIMER_SUPPORT
    httpd_create_timer(1, fupgrade_timer_reboot, (void *)&hard_reset);
#else /* HTTPD_TIMER_SUPPORT */
    /* For no HTTPD timer case, the specific design must be added here 
     *  to reset device 
     */
#endif /* HTTPD_TIMER_SUPPORT */

    return SSP_HANDLER_RET_INTACT;
}

SSP_HANDLER_RETVAL
ssphandler_fupgrade_cgi(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT
{
    SSP_HANDLER_CONTEXT_EXT *cxte = (SSP_HANDLER_CONTEXT_EXT *)cxt;
    unsigned char *pbuff = NULL;
    int32 gidx, glen, coff;
    int32 off, len;
    BOOL    hard_reset;
    bookkeeping_t bk_data;
#ifdef CFG_DUAL_IMAGE_INCLUDED
    flash_imghdr_t *hdr;
    uint16 ts;
#endif

    if (cxt->type == SSP_HANDLER_CLOSE) {
        
        hard_reset = TRUE;
        if (upgrade_in_progress == 2) {
#ifdef HTTPD_TIMER_SUPPORT
            httpd_create_timer(1, fupgrade_timer_reboot, (void *)&hard_reset);
#else /* HTTPD_TIMER_SUPPORT */
            /* For no HTTPD timer case, the specific design must be added here 
             *  to reset device 
             */
#endif /* HTTPD_TIMER_SUPPORT */
        }
        
        upgrade_in_progress = 0;
        return SSP_HANDLER_RET_INTACT;
    }

    if (cxt->type != SSP_HANDLER_FILE_UPLOAD) {
        return SSP_HANDLER_RET_INTACT;
    }

    /* The first call to this handler is the indication of file length */
    if (cxte->url_data.upload.buf == NULL && cxte->url_data.upload.index == -1) {

        if (board_loader_mode_get(&bk_data, FALSE) == LM_UPGRADE_FIRMWARE){
#ifdef CFG_DUAL_IMAGE_INCLUDED
            base_addr = (ACTIVE_IMAGE_GET(bk_data.active_image) == 1) ? 
                         BOARD_SECONDARY_FIRMWARE_ADDR : BOARD_FIRMWARE_ADDR;
            sal_printf("Upgrading firware at partition %d address 0x%08x\n",
                        (ACTIVE_IMAGE_GET(bk_data.active_image) == 1) ? 2 : 1, base_addr);
#else
            base_addr = BOARD_FIRMWARE_ADDR;
            /* Clear bookkeeping data here. */
            board_loader_mode_get(NULL, TRUE);
#endif

        } else {
            base_addr = BOARD_FIRMWARE_ADDR;
        }
        
        /* Check if someone has already started */
        if (upgrade_in_progress) {
            return show_error(cxt, psmem, ERROR_ALLOC_MUTEX);
        }
        upgrade_in_progress = 1;
        base_addr_start = base_addr;
        /* Allocate sector buffer */
        pbuff = (unsigned char *)ssputil_psmem_alloc(
                    psmem, 
                    ssphandler_fupgrade_cgi, 
                    BYTES_PER_WRITE
                    );              
        total_size = cxte->url_data.upload.length;
        current_received = 0;
        return SSP_HANDLER_RET_INTACT;
    }

    /* Get sector buffer previous allocated/updated */
    pbuff = (unsigned char *)ssputil_psmem_get(psmem, ssphandler_fupgrade_cgi);
    if (pbuff == NULL) {
        /* SSP_HANDLER_RET_MODIFIED should be set at the occurance
         * of error found. */
        return SSP_HANDLER_RET_INTACT;
    }

    coff = 0; /* Offset of the incoming upload buffer */
    gidx = cxte->url_data.upload.index; /* Start index of data to process */
    glen = cxte->url_data.upload.length; /* Length of data not yet processed */
    while(glen > 0) {
        off = gidx % BYTES_PER_WRITE;
        len = glen;
        if (len > BYTES_PER_WRITE - off) {
            len = BYTES_PER_WRITE - off;
        }

        sal_memcpy(pbuff + off, cxte->url_data.upload.buf + coff, (size_t)len);

        gidx += len;
        glen -= len;
        coff += len;

        if (off + len == BYTES_PER_WRITE) {

            /* Validate image header */
            if (base_addr == base_addr_start) {
                if (!board_check_imageheader((msaddr_t)pbuff)) {
#if defined(CFG_DUAL_IMAGE_INCLUDED) && CFG_CONSOLE_ENABLED
                     sal_printf("Upload image header is not valid\n");
#endif
                    ssputil_psmem_free(psmem, ssphandler_fupgrade_cgi);
                    return show_error(cxt, psmem, ERROR_INVALID_FIRMWARE);
                }
            }

#ifdef CFG_DUAL_IMAGE_INCLUDED
            /* Add timestamp here */
            if (board_loader_mode_get(&bk_data, FALSE) == LM_UPGRADE_FIRMWARE){
                /* Clear bookkeeping data here. */
                board_loader_mode_get(NULL, TRUE);
                /* 
                 * New timestamp: old timestamp of the other partition plus 1
                 */
                ts = (uint16)ACTIVE_TIMESTAMP_GET(bk_data.active_image);
                if (ts == 0xFFFF) {
                    ts = 1;
                } else {
                    ts += 1;
                }
                hdr = (flash_imghdr_t *)pbuff;
                hdr->timestamp[0] = TIMESTAMP_MAGIC_START;
                hdr->timestamp[3] = TIMESTAMP_MAGIC_END;
                hdr->timestamp[1] = (ts >> 8) & 0xFF;
                hdr->timestamp[2] = ts & 0xFF;
            }
#endif /* CFG_DUAL_IMAGE_INCLUDED */

            /* Write it to the flash */
            if (flash_erase(base_addr , BYTES_PER_WRITE) < 0) {
                ssputil_psmem_free(psmem, ssphandler_fupgrade_cgi);
                return show_error(cxt, psmem, ERROR_WRITE_FAIL);
            }
            if (flash_program(base_addr, (const void*)pbuff, BYTES_PER_WRITE) 
                < 0) {
                ssputil_psmem_free(psmem, ssphandler_fupgrade_cgi);
                return show_error(cxt, psmem, ERROR_WRITE_FAIL);
            }
            base_addr += BYTES_PER_WRITE;
        }
    }

    /* End of File: write incomplete sector */
    if (cxte->url_data.upload.buf == NULL && cxte->url_data.upload.length == 0) {
        off = gidx % BYTES_PER_WRITE;

        /* Write it to the flash */
        if (flash_erase(base_addr , off) < 0) {
            ssputil_psmem_free(psmem, ssphandler_fupgrade_cgi);
            return show_error(cxt, psmem, ERROR_WRITE_FAIL);
        }
        if (flash_program(base_addr, (const void*)pbuff, off) 
            < 0) {
            ssputil_psmem_free(psmem, ssphandler_fupgrade_cgi);
            return show_error(cxt, psmem, ERROR_WRITE_FAIL);
        }

        /* Flash writing done */
        ssputil_psmem_free(psmem, ssphandler_fupgrade_cgi);
        
        /* Mark it as it'd completed successfully */
        upgrade_in_progress = 2;
    }

    current_received += cxte->url_data.upload.length;

    return SSP_HANDLER_RET_INTACT;
}

void
sspvar_upgrade_tag_info(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT
{
    int time = 10;
    
    /* fix build complain */
    UNREFERENCED_PARAMETER(psmem);

    ret->type = SSPVAR_RET_STRING;
    sal_strcpy(ssputil_shared_buffer, "");
    switch(params[0]) {
    case SSPMACRO_UPGRADE_RSTSUBJECT:
        if (upgrade_in_progress == -1) {
            sal_sprintf(ssputil_shared_buffer,": %s", error_str);
        } else {
            sal_sprintf(ssputil_shared_buffer,"%s", 
                (upgrade_in_progress == 0) ? "Aborted" : "Completed" );
        }
        ret->val_data.string = ssputil_shared_buffer;
        break;
    case SSPMACRO_UPGRADE_RSTSTR:
        if (upgrade_in_progress == -1) {
            sal_sprintf(ssputil_shared_buffer,"Please click 'Continue' to upgrade firmware again.");
        } else {
            sal_sprintf(ssputil_shared_buffer,"Please wait while system booting up and then click 'Continue'.");
        }
        ret->val_data.string = ssputil_shared_buffer;
        break;
    case SSPMACRO_UPGRADE_RSTTIME:
        ret->type = SSPVAR_RET_INTEGER;
        if (upgrade_in_progress == -1) {
            time = 1;
        } 
        ret->val_data.integer = time; 
        break;
    default:
        sal_sprintf(ssputil_shared_buffer," (Unknown)");
        ret->val_data.string = ssputil_shared_buffer;
        break;
    }
    return;
}

SSP_HANDLER_RETVAL
ssphandler_fupgrade_htm(SSP_HANDLER_CONTEXT *cxt, SSP_PSMH psmem) REENTRANT
{
    /* fix build complain */
    UNREFERENCED_PARAMETER(psmem);

    if (cxt->type != SSP_HANDLER_QUERY_STRINGS) {
        return SSP_HANDLER_RET_INTACT;
    }

    if (upgrade_in_progress) {    
        return SSP_HANDLER_RET_MODIFIED;
    } else {
        current_received = 0;
        total_size = 0;
    }
    
    return SSP_HANDLER_RET_INTACT;
}

#endif /* _BOOTLOADER__ */


