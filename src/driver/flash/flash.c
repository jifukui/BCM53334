/*
 * $Id: flash.c,v 1.15 Broadcom SDK $
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

#if CFG_FLASH_SUPPORT_ENABLED

extern flash_dev_t current_flash_dev;

/**
 * flash initialization 
 *
 *
 * @param dev (IN)- NULL : To probe flash automaticly
 *                            !NULL: To specify the device we want to use, no auto-probe
 * @return sys_error_t 
 *             SYS_OK : there is no error
 *                
 */

sys_error_t flash_init(flash_dev_t *dev) {

     if (dev == NULL) {                  
         dev = &current_flash_dev;         
         dev->funs->flash_init(NULL);         
         sal_printf("Flash detected: %s \n", dev->name);  
     } else {
         dev->funs->flash_init(dev);              
         sal_memcpy(&current_flash_dev, dev, sizeof(flash_dev_t));
//     	 sal_printf("The device name is %s",current_flash_dev.name);
//         sal_printf("The flash end address is %d",current_flash_dev.end);     
// 	 sal_printf("The demo name is %s",dev->name);
//	 sal_printf("The demo flash end address is %d",dev->end);
//         return SYS_OK;
     }
     
     if (sal_strcmp(current_flash_dev.name, "Unknown Flash")==0) {
         sal_printf("ID: %x %x %x %x\n", current_flash_dev.jedec_id[0], current_flash_dev.jedec_id[1],current_flash_dev.jedec_id[2],current_flash_dev.jedec_id[3]);
         sal_printf("Please check the flash supporting table\n");         
     } 
     return SYS_OK;
}

/**
 * Get the pointer of the current flash driver instance 
 *
 *
 * @return flash_dev_t* the pointer of the current flash driver instance 
 *             NULL : for Unknown flash driver to forbid any write to flash   
 */

flash_dev_t *flash_dev_get(void) {

    if (sal_strcmp(current_flash_dev.name, "Unknown Flash")==0) {
        return NULL;
    }

    return &current_flash_dev;
}
/*
 *  Return the size of the block which is at the given address
 */
size_t
flash_block_size(const hsaddr_t addr)
{
  int16 i;
  size_t offset;
  flash_dev_t * dev;

  dev = board_get_flash_dev();
  if (!dev) {
    return SYS_ERR;
  }

  SAL_ASSERT((addr >= dev->start) && (addr <= dev->end));

  offset = addr - dev->start;
  for (i=0; i < dev->num_block_infos; i++) {
    if (offset < (dev->block_info[i].blocks * dev->block_info[i].block_size))
      return dev->block_info[i].block_size;
    offset = offset -
      (dev->block_info[i].blocks * dev->block_info[i].block_size);
  }
  return 0;
}
/*
 * Return the first address of a block. The flash might not be aligned
 * in terms of its block size. So we have to be careful and use
 * offsets.
 */
STATICFN hsaddr_t
flash_block_begin(hsaddr_t addr, flash_dev_t *dev)
{
  size_t block_size;
  hsaddr_t offset;

  block_size = flash_block_size(addr);

  offset = addr - dev->start;
  offset = (offset / block_size) * block_size;
  return offset + dev->start;
}

sys_error_t
flash_erase(hsaddr_t flash_base, size_t len)
{
    hsaddr_t block, end_addr;
    flash_dev_t * dev;
    int32 erase_count;
    sys_error_t rv = SYS_OK;

    dev = board_get_flash_dev();
    if (!dev) {
        return SYS_ERR;
    }

    if (flash_base < dev->start || (flash_base + len -1) > dev->end) {
        return SYS_ERR_PARAMETER;
    }
    /*
     * Check whether or not we are going past the end of this device, on
     * to the next one. If so the next device will be handled by a
     * recursive call later on.
     */
    if (len > (dev->end + 1 - flash_base)) {
        end_addr = dev->end;
    } else {
        end_addr = flash_base + len - 1;
    }
    /* erase can only happen on a block boundary, so adjust for this */
    block         = flash_block_begin(flash_base, dev);
    erase_count   = (end_addr + 1) - block;

    while (erase_count > 0) {
        size_t block_size = flash_block_size(block);

        /* Pad to the block boundary, if necessary */
        if (erase_count < block_size) {
            erase_count = block_size;
        }

        rv = (*dev->funs->flash_erase_block)(dev,block);

        if (SYS_OK != rv) {
            break;
        }
        block       += block_size;
        erase_count -= block_size;
    }

    return rv;
}

sys_error_t
flash_program(hsaddr_t flash_base, const void *ram_base, size_t len)
{
    flash_dev_t * dev;
    hsaddr_t addr, end_addr, block;
    const uint8 *ram = ram_base;
    int32 write_count;
    size_t offset;
    sys_error_t rv = SYS_OK;


    dev = board_get_flash_dev();
    if (!dev) {
        return SYS_ERR;
    }

    if (flash_base < dev->start || (flash_base + len - 1) > dev->end) {
        return SYS_ERR_PARAMETER;
    }

    addr = flash_base;
    if (len > (dev->end + 1 - flash_base)) {
      end_addr = dev->end;
    } else {
      end_addr = flash_base + len - 1;
    }
    write_count = (end_addr + 1) - flash_base;


    /*
     * The first write may be in the middle of a block. Do the necessary
     * adjustment here rather than inside the loop.
     */
    block = flash_block_begin(flash_base, dev);
    if (addr == block) {
        offset = 0;
    } else {
        offset = addr - block;
    }

    while (write_count > 0) {
        size_t block_size = flash_block_size(addr);
        size_t this_write;
        if (write_count > (block_size - offset)) {
            this_write = block_size - offset;
        } else {
            this_write = write_count;
        }
        /* Only the first block may need the offset. */
        offset = 0;
        rv = (*dev->funs->flash_program)(dev, addr, ram, this_write);

        if (SYS_OK != rv) {
            break;
        }
        write_count -= this_write;
        addr        += this_write;
        ram         += this_write;
    }
    return rv;
}

sys_error_t
flash_read(hsaddr_t flash_base, void *ram_base, size_t len)
{
    flash_dev_t * dev;
    hsaddr_t addr, end_addr;
    uint8 * ram = (uint8 *)ram_base;
    int32 read_count;
    sys_error_t rv = SYS_OK;

    dev = board_get_flash_dev();
    if (!dev) {
        return SYS_ERR;
    }

    if (flash_base < dev->start || (flash_base + len - 1) > dev->end) {
	sal_printf("The value of end is %x",dev->end);
        return SYS_ERR_PARAMETER;
    }

    addr = flash_base;
    if (len > (dev->end + 1 - flash_base)) {
        end_addr = dev->end;
    } else {
        end_addr = flash_base + len - 1;
    }
    read_count = (end_addr + 1) - flash_base;

    if (!dev->funs->flash_read) {
        sal_memcpy(ram, (void*)addr, read_count);
    } else {
        /*
         * We have to indirect through the device driver.
         * The first read may be in the middle of a block. Do the necessary
         * adjustment here rather than inside the loop.
         */
        size_t offset;
        hsaddr_t  block = flash_block_begin(flash_base, dev);

        if (addr == block) {
            offset = 0;
        } else {
            offset = addr - block;
        }

        while (read_count > 0) {
            size_t block_size = flash_block_size(addr);
            size_t this_read;
            if (read_count > (block_size - offset)) {
                this_read = block_size - offset;
            } else {
                this_read = read_count;
            }
            /* Only the first block may need the offset */
            offset = 0;
            rv = (*dev->funs->flash_read)(dev, addr, ram, this_read);
            if (SYS_OK != rv) {
                break;
            }
            read_count  -= this_read;
            addr        += this_read;
            ram         += this_read;
        }
  }
  return rv;
}
#endif /* CFG_FLASH_SUPPORT_ENABLED */
