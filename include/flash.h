/*
 * $Id: flash.h,v 1.9 Broadcom SDK $
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

#ifndef _FLASH_H_
#define _FLASH_H_


/* Forward reference of the device structure */
struct flash_dev_s;

/* Structure of pointers to functions in the device driver */
struct flash_dev_funs {
  sys_error_t (*flash_init) (struct flash_dev_s *dev) REENTRANT;
  sys_error_t (*flash_erase_block) (struct flash_dev_s *dev, 
                                    hsaddr_t block_base) REENTRANT;
  sys_error_t (*flash_program) (struct flash_dev_s *dev, 
                                hsaddr_t base, 
                                const void* ram_base, size_t len) REENTRANT;
  sys_error_t (*flash_read) (struct flash_dev_s *dev, 
                             const hsaddr_t base, 
                             void* ram_base, size_t len) REENTRANT;
};

typedef struct flash_block_info_s 
{
  size_t  block_size;
  uint32  blocks;
} flash_block_info_t;

/* Information about what one device driver drives */
typedef struct {
  hsaddr_t                  start;              /* First address */
  hsaddr_t                  end;                /* Last address */
  uint32                    num_block_infos;    /* Number of entries */
  const flash_block_info_t* block_info;         /* Info about block sizes */
} flash_info_t;

/* 
 * flash device class
 */
typedef struct flash_dev_s{
  uint8 jedec_id[4];                          /* JEDEC ID for flash probe */
  uint8 jedec_id_mask[4];                     /* JEDEC ID mask to mask unwant field of JEDEC ID */ 
  const struct flash_dev_funs *funs;          /* Function pointers */
  uint32                      flags;          /* Device characteristics */
  hsaddr_t                    start;          /* First address */
  hsaddr_t                    end;            /* Last address */
  uint32                      num_block_infos;/* Number of entries */
  const flash_block_info_t    *block_info;    /* Info about one block size */
  const char *                name;
  const void                  *priv;          /* Devices private data */
} flash_dev_t;

extern sys_error_t flash_erase(hsaddr_t flash_base, size_t len);
extern sys_error_t flash_program(hsaddr_t flash_base, const void *ram_base, 
                                 size_t len);
extern sys_error_t flash_read(hsaddr_t flash_base, 
                              void *ram_base, size_t len);
extern size_t flash_block_size(const hsaddr_t addr);
extern flash_dev_t *flash_dev_get(void);
extern sys_error_t flash_init(flash_dev_t *dev);

#endif /* _FLASH_H_ */
