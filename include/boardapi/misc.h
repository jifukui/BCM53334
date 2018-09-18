/*
 * $Id: misc.h,v 1.19 Broadcom SDK $
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

#ifndef _BOARDAPI_MISC_H_
#define _BOARDAPI_MISC_H_

#if CFG_FLASH_SUPPORT_ENABLED
/* Get flash device object */
extern flash_dev_t * board_get_flash_dev(void) REENTRANT;
#endif /* CFG_FLASH_SUPPORT_ENABLED */

#define MAX_BOARD_NAME_LEN   32

typedef struct flash_imghdr_s {
    uint8 seal[4];          /* UMHD */
    uint8 size[8];          /* Image size in hex, in bytes, always big-endian */
    uint8 flags[4];         /* Various flags in hex, always big-endian */
    uint8 chksum[4];        /* checksum in hex, always big-endian */
    uint8 boardname[MAX_BOARD_NAME_LEN]; /* Board name */
    uint8 majver,minver,ecover,miscver; /* Firmware version */
    uint8 timestamp[4];     /* Timestamp to be used in dual image boot */
    uint8 reserved[4];
} flash_imghdr_t;            /* should be 64 bytes */

#define UM_IMAGE_HEADER_SEAL    "UMHD"

/*
 * Shared structure between loader and firmware. Currently used when firmware
 * needs to upgrade image and jump back to loader to do it.
 */
typedef struct bookkeeping_s
{
    uint32 magic;
    uint16 agent_ip[2];
    uint16 agent_netmask[2];
    uint16 agent_gateway[2];
    uint16 reserved;

#if CFG_UIP_IPV6_ENABLED
    uint8 agent_ipv6[16];
#endif /* CFG_UIP_IPV6_ENABLED */

#ifdef CFG_DUAL_IMAGE_INCLUDED
    /* [31:16] = timestamp of active image,
     * [15:8]  = backup image
     * [7:0]   = active image 
     */
    uint32 active_image;
#endif
} bookkeeping_t;

#define UM_BOOKKEEPING_SEAL    (0x5441574E)
#define TIMESTAMP_MAGIC_START  (0xAC)
#define TIMESTAMP_MAGIC_END    (0x47)

#define ACTIVE_IMAGE_GET(flag)      ((flag) & 0xFF)
#define ACTIVE_TIMESTAMP_GET(flag)  ((flag) >> 16)
#define IS_BACKUP_IMAGE_VALID(flag) (((flag) & 0xFF00) != 0)

typedef enum loader_mode_s {
    LM_NORMAL,
    LM_UPGRADE_FIRMWARE
} loader_mode_t;

#ifdef __BOOTLOADER__

/* Check integrity of firmware image */
extern BOOL board_check_image(hsaddr_t address, hsaddr_t *outaddr) REENTRANT;

/* Launch program at entry address */
extern void board_load_program(hsaddr_t entry) REENTRANT;

/*
 * For loader to check if requested by firmware to upgrade image.
 */
extern loader_mode_t board_loader_mode_get(bookkeeping_t *bk_data, BOOL reset) REENTRANT;

#else
/*
 * For firmware to request loader to upgrade image.
 */
extern void board_loader_mode_set(loader_mode_t mode, bookkeeping_t *bk_data);
#endif /* __BOOTLOADER__ */

/* Validate firmware image header */
extern BOOL board_check_imageheader(msaddr_t address) REENTRANT;

/*
 * System Reset
 */
extern void board_reset(void *param) REENTRANT;

/*
 * Enable/Disable loop detect functionality.
 */
extern void board_loop_detect_enable(BOOL enable);
extern uint8 board_loop_detect_status_get(void);
extern void board_firmware_version_get(uint8 *major, uint8 *minor,
                                       uint8 *eco, uint8 *misc) REENTRANT;


#ifdef CFG_ZEROCONF_MDNS_INCLUDED
extern sys_error_t board_mdns_enable_set(BOOL enable);
extern sys_error_t board_mdns_enable_get(BOOL *enable);
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

#ifdef CFG_DUAL_IMAGE_INCLUDED
extern void board_active_image_set(uint32 partition);
extern uint32 board_active_image_get(void);
extern BOOL board_select_boot_image(hsaddr_t *outaddr);
#endif /* CFG_DUAL_IMAGE_INCLUDED */

#ifdef CFG_RESET_BUTTON_INCLUDED
extern uint8 sw_simulate_press_reset_button_duration;
extern uint8 reset_button_active_high;
extern uint8 reset_button_gpio_bit;
extern uint8 reset_button_enable;

extern BOOL board_reset_button_get(void);
#endif /* CFG_RESET_BUTTON_INCLUDED */

#endif /* _BOARDAPI_MISC_H_ */
