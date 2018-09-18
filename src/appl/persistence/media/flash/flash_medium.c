/*
 * $Id: flash_medium.c,v 1.10 Broadcom SDK $
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
#include "flash_medium.h"

#if (CFG_FLASH_SUPPORT_ENABLED && CFG_PERSISTENCE_SUPPORT_ENABLED)

/* Open types */
#define MED_TYPE_NOT_OPENED             0
#define MED_TYPE_OPEN_FOR_READ          1
#define MED_TYPE_OPEN_FOR_WRITE         2

/* 
 * Symbols to be defined for medium customization 
 */
#ifndef MEDIUM_FLASH_SIZE
#define MEDIUM_FLASH_SIZE               1024
#endif
#ifndef MEDIUM_FLASH_MAGIC1
#define MEDIUM_FLASH_MAGIC1             "FMv2"
#endif
#ifndef MEDIUM_FLASH_MAX_ITEMS
#define MEDIUM_FLASH_MAX_ITEMS          (40)
#endif

/* Forwards */
STATICCBK int32 medium_open(BOOL write, struct _smedium_s *ps) REENTRANT;
STATICCBK int32 medium_close(struct _smedium_s *ps) REENTRANT;
STATICCBK BOOL medium_item_begin(const char *name, struct _smedium_s *ps, int32 version) REENTRANT;
STATICCBK BOOL medium_item_end(struct _smedium_s *ps) REENTRANT;
STATICCBK int32 medium_item_size(struct _smedium_s *ps) REENTRANT;
STATICCBK int32 medium_item_version(struct _smedium_s *ps) REENTRANT;
STATICCBK int32 medium_read(uint32 len, uint8 *buf, struct _smedium_s *ps) REENTRANT;
STATICCBK int32 medium_write(uint32 len, uint8 *buf, struct _smedium_s *ps) REENTRANT;
#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
STATICCBK const char * medium_read_string(struct _smedium_s *ps) REENTRANT;
STATICCBK BOOL medium_write_string(const char *str, struct _smedium_s *ps) REENTRANT;
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
STATICCBK BOOL medium_seek(uint32 pos, struct _smedium_s *ps) REENTRANT;
STATICCBK uint32 medium_medium_properties(struct _smedium_s *ps) REENTRANT;
STATICCBK const char *medium_get_medium_identification(struct _smedium_s *ps) REENTRANT;
STATICCBK int32 medium_get_total_data_size(struct _smedium_s *ps) REENTRANT;
STATICCBK uint32 medium_get_valid_groups(struct _smedium_s *ps) REENTRANT;

/* Convenient macros */
#define THIS                ((MEDIUM_FLASH *)ps)

static void
cleanup(struct _smedium_s *ps) REENTRANT
{
    if (THIS->databuf != NULL) {
        sal_free(THIS->databuf);
    }
    
    if (THIS->header != NULL) {
        sal_free(THIS->header);
    }
    
    if (THIS->items != NULL) {
        sal_free(THIS->items);
    }
    
    THIS->open_type = MED_TYPE_NOT_OPENED;
}

STATICCBK int32 
medium_open(BOOL write, struct _smedium_s *ps) REENTRANT
{
    BOOL valid = TRUE;
    uint16 size, offset;
    med_item_t *item;
    if (write) {
        if (THIS->open_type == MED_TYPE_OPEN_FOR_READ) {
            /* Already opened with READ */
            return -1; 
        } else if (THIS->open_type == MED_TYPE_OPEN_FOR_WRITE) {
            return 0;
        }
        
        THIS->open_type = MED_TYPE_OPEN_FOR_WRITE;
    } else {
        if (THIS->open_type == MED_TYPE_OPEN_FOR_WRITE) {
            /* Already opened with WRITE */
            return -1;
        } else if (THIS->open_type == MED_TYPE_OPEN_FOR_READ) {
            return 0;
        }

        THIS->open_type = MED_TYPE_OPEN_FOR_READ;
    }
    
    THIS->header = NULL;
    THIS->items = NULL;
    THIS->databuf = NULL;
    THIS->newbuf = NULL;
    THIS->item = NULL;
    
    /* Allocate memory for item holders */
    THIS->items = 
        (med_item_t *)sal_malloc(sizeof(med_item_t) * MEDIUM_FLASH_MAX_ITEMS);
    if (THIS->items == NULL) {
        goto err;
    }
    THIS->items[0].header = NULL;
    
    /* Allocate memory for header */
    THIS->header = sal_malloc(sizeof(med_header_t));
    if (THIS->header == NULL) {
        goto err;
    }
    
    /* Try to read header from flash */
    if (flash_read(THIS->start, THIS->header, sizeof(med_header_t)) != SYS_OK) {
        goto err;
    }
    
    /* Check magic */
    if (sal_memcmp(THIS->header->magic, MEDIUM_FLASH_MAGIC1, 4) != 0) {
        valid = FALSE;
    }
    
    /* Check size */
    size = THIS->header->size;
    if (valid && size + sizeof(med_header_t) > MEDIUM_FLASH_SIZE) {
        valid = FALSE;
    }
    
    /* Do checksum verification on existing data if valid */
    if (valid) {
        uint16 chksum;
    
        /* Allocate buffer for data */
        THIS->databuf = sal_malloc(size);
        if (THIS->databuf == NULL) {
	   goto err;
        }

        if (flash_read(THIS->start + sizeof(med_header_t), 
                       THIS->databuf, size) != SYS_OK) {
            sal_free(THIS->databuf);
            goto err;
        }

        /* Verify checksum */
        chksum = sal_checksum(0, THIS->databuf, size);
        if (chksum != THIS->header->checksum) {
            valid = FALSE;
        }
    }
    
    /* For read, don't go further if it's not valid */
    if (!valid) {
        if (!write) {
            goto err;
        }
        size = 0;
        THIS->header->size = 0;
    }
    
    /* Parse and read all items */
    item = THIS->items;
    for(offset=0; offset < size; item++) {
        med_item_hdr_t *header;
        
        header = (med_item_hdr_t *)&THIS->databuf[offset];
        item->header = header;
        
        offset += sizeof(med_item_hdr_t) - 1 + header->datalen;
#ifndef  __C51__
        /* 4 bytes alignment */
        offset = (offset + 0x3) & ~0x3;
#endif /* !__C51__ */
    }
    if (size != offset) {
        /* Size unmatched */
        if (!write) {
            goto err; 
        }
         
        /* Drop all items (for write) */
        item = &THIS->items[0];
    }
    item->header = NULL;
    
    return 0;
    
err:
    cleanup(ps);
    return -1;
}

STATICCBK int32 
medium_close(struct _smedium_s *ps) REENTRANT
{
    if (THIS->open_type == MED_TYPE_NOT_OPENED) {
        return 0;
    }
    
    if (THIS->open_type == MED_TYPE_OPEN_FOR_WRITE) {

        hsaddr_t addr = THIS->start;
        uint16 size;
        med_item_t *item;
        uint8 *buffer;

        /* Erase flash */
        flash_erase(addr, sizeof(med_header_t) + THIS->header->size);

        /* Write item one by one */
        addr += sizeof(med_header_t);
        for(item = THIS->items; item->header != NULL; item++) {
            flash_program(addr, item->header, 
                          sizeof(med_item_hdr_t) - 1 + item->header->datalen);
            addr += sizeof(med_item_hdr_t) - 1 + item->header->datalen;
#ifndef  __C51__
            /* 4 bytes alignment */
            addr = (addr + 0x3) & ~0x3;
#endif /* !__C51__ */
        }
        size = addr - THIS->start - sizeof(med_header_t);
        
        /* Fill up fields in header */
        sal_memcpy(THIS->header->magic, MEDIUM_FLASH_MAGIC1, 4);
        THIS->header->size = size;
        
        /* Calculation checksum */
        if (THIS->newbuf != NULL) {
            sal_free(THIS->newbuf);
        }
        buffer = (uint8 *)sal_malloc(size);
        if (buffer == NULL) {
            cleanup(ps);
            return -1;
        }
        flash_read(THIS->start + sizeof(med_header_t), buffer, size);
        THIS->header->checksum = sal_checksum(0, buffer, size);
        sal_free(buffer);
        
        /* Write header to the flash */
        flash_program(THIS->start, THIS->header, sizeof(med_header_t));
    }

    cleanup(ps);
    return 0;
}

STATICCBK BOOL 
medium_item_begin(const char *name, struct _smedium_s *ps, int32 version) REENTRANT
{
    med_item_t *item;

    if (name == NULL || name[0] == 0) {
        return FALSE;
    }
    
    if (THIS->open_type == MED_TYPE_NOT_OPENED) {
        return FALSE;
    }
    
    if (THIS->item != NULL) {
        medium_item_end(ps);
    }
    
    /* Find the item with matching name */
    for(item = THIS->items; item->header != NULL; item++) {
        if (sal_strcmp(item->header->name, name) == 0) {
            break;
        }
    }
    if (item->header == NULL) {

        /* No matching item found */
        if (THIS->open_type == MED_TYPE_OPEN_FOR_READ) {
            return FALSE;
        }

        /* New item entry used; mark it the last items */
        (item + 1)->header = NULL;

    } else if (item->header->version != (uint16)version) {
        
        /* Version incorrect */
        if (THIS->open_type == MED_TYPE_OPEN_FOR_READ) {
            return FALSE;
        }
        
        /* For write, set header to NULL to use new bufer */
        item->header = NULL;
    }
    
    /* Initial data index */
    THIS->item_index = sal_strlen(name) + 1;
    
    /* Item not found or changed; use new buffer */
    if (item->header == NULL) {

        if (THIS->newbuf == NULL) {
            /* Create new buffer if not yet */
            THIS->newbuf = (uint8 *)sal_malloc(MEDIUM_FLASH_SIZE);
            if (THIS->newbuf == NULL) {
                return FALSE;
            }
            THIS->nbuf_len = 0;
        }
        
        item->header = (med_item_hdr_t *)&THIS->newbuf[THIS->nbuf_len];
        item->header->version = (uint16)version;
        item->header->datalen = 0;  /* This also indicates it's new */
        sal_strcpy(item->header->name, name);
        THIS->nbuf_len += sizeof(med_item_hdr_t) - 1 + THIS->item_index;
    }
    
    THIS->item = item->header;
    THIS->item_ptr = ((uint8 *)item->header) + 
                        sizeof(med_item_hdr_t)  - 1 + THIS->item_index;
    return TRUE;
}

STATICCBK BOOL 
medium_item_end(struct _smedium_s *ps) REENTRANT
{
    if (THIS->open_type == MED_TYPE_NOT_OPENED) {
        return FALSE;
    }
    
    if (THIS->open_type == MED_TYPE_OPEN_FOR_WRITE) {
        if (THIS->item->datalen == 0) {
            THIS->item->datalen = THIS->item_index;
#ifndef __C51__
            /* 4 bytes alignment */
            THIS->nbuf_len = (THIS->nbuf_len + 0x3) & ~0x3;
#endif /* !__C51__ */
        }
    }
    
    THIS->item = NULL;
    return TRUE;
}

STATICCBK int32 
medium_item_size(struct _smedium_s *ps) REENTRANT
{
    if (THIS->open_type != MED_TYPE_OPEN_FOR_READ) {
        return -1;
    }
    
    if (THIS->item == NULL) {
        return -1;
    }
    
    return (int32)THIS->item->datalen - sal_strlen(THIS->item->name) - 1;
}

STATICCBK int32 
medium_item_version(struct _smedium_s *ps) REENTRANT
{
    if (THIS->open_type != MED_TYPE_OPEN_FOR_READ) {
        return -1;
    }
    
    if (THIS->item == NULL) {
        return -1;
    }
    
    return (int32)THIS->item->version;
}

STATICCBK int32 
medium_read(uint32 len, uint8 *buf, struct _smedium_s *ps) REENTRANT
{
    if (len == 0) {
        return 0;
    }
    if (buf == NULL) {
        return -1;
    }
    if (THIS->open_type != MED_TYPE_OPEN_FOR_READ) {
        return -1;
    }
    if (THIS->item == NULL) {
        return -1;
    }
    
    /* Shouldn't exceed max size */
    SAL_ASSERT(THIS->item_index + len <= THIS->item->datalen);
    if (THIS->item_index + len > THIS->item->datalen) {
        return -1;
    }
    
    sal_memcpy(buf, THIS->item_ptr, len);
    THIS->item_ptr += len;
    THIS->item_index += len;
    return len;
}

STATICCBK int32 
medium_write(uint32 len, uint8 *buf, struct _smedium_s *ps) REENTRANT
{
    if (len == 0) {
        return 0;
    }
    if (buf == NULL) {
        return -1;
    }
    if (THIS->open_type != MED_TYPE_OPEN_FOR_WRITE) {
        return -1;
    }
    if (THIS->item == NULL) {
        return -1;
    }
    
    /* Shouldn't exceed max size */
    if (THIS->item->datalen != 0) {
        SAL_ASSERT(THIS->item_index + len <= THIS->item->datalen);
        if (THIS->item_index + len > THIS->item->datalen) {
            return -1;
        }
    } else {
        /* In new buffer */
        SAL_ASSERT(THIS->nbuf_len + len <= MEDIUM_FLASH_SIZE);
        if (THIS->nbuf_len + len > MEDIUM_FLASH_SIZE) {
            return -1;
        }
    }
    
    /* Copy data to buffer */
    sal_memcpy(THIS->item_ptr, buf, len);
    THIS->item_ptr += len;
    THIS->item_index += len;
    if (THIS->item->datalen == 0) {
        THIS->nbuf_len += len;
    }

    return len;
}

#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
STATICCBK const char * 
medium_read_string(struct _smedium_s *ps) REENTRANT
{
    int len; 

    if (THIS->open_type != MED_TYPE_OPEN_FOR_READ) {
        return NULL;
    }
    if (THIS->item == NULL) {
        return NULL;
    }
    
    len = sal_strlen((const char *)THIS->item_ptr) + 1;
    
    /* Shouldn't exceed max size */
    SAL_ASSERT(THIS->item_index + len <= THIS->item->datalen);
    if (THIS->item_index + len > THIS->item->datalen) {
        return NULL;
    }
    
    THIS->item_ptr += len;
    THIS->index_index += len;
    
    return (const char *)THIS->item_ptr;
}

STATICCBK BOOL 
medium_write_string(const char *str, struct _smedium_s *ps) REENTRANT
{
    uint16 len;

    if (THIS->open_type != MED_TYPE_OPEN_FOR_WRITE) {
        return FALSE;
    }
    if (THIS->item == NULL) {
        return FALSE;
    }
    if (str == NULL) {
        str = "";
    }
    
    len = sal_strlen(str) + 1;
    
    /* Shouldn't exceed max size */
    if (THIS->item->datalen != 0) {
        SAL_ASSERT(THIS->item_index + len <= THIS->item->datalen);
        if (THIS->item_index + len > THIS->item->datalen) {
            return FALSE;
        }
    } else {
        /* In new buffer */
        SAL_ASSERT(THIS->nbuf_len + len <= MEDIUM_FLASH_SIZE);
        if (THIS->nbuf_len + len > MEDIUM_FLASH_SIZE) {
            return FALSE;
        }
    }
    
    sal_strcpy((char *)THIS->item_ptr, str);
    THIS->item_ptr += len;
    THIS->item_index += len;
    if (THIS->item->datalen == 0) {
        THIS->nbuf_len += len;
    }

    return TRUE;
}
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */

STATICCBK BOOL 
medium_seek(uint32 pos, struct _smedium_s *ps) REENTRANT
{
    /* Not supported */
    UNREFERENCED_PARAMETER(ps);
    UNREFERENCED_PARAMETER(pos);
    return FALSE;
}

STATICCBK uint32 
medium_medium_properties(struct _smedium_s *ps) REENTRANT
{
    /* It's a compact medium with "total data size" capability */
    UNREFERENCED_PARAMETER(ps);
    return SMEDIUM_PROP_GET_ITEM_DATA_SIZE | SMEDIUM_PROP_GET_ITEM_VERSION;
}

STATICCBK int32 
medium_get_total_data_size(struct _smedium_s *ps) REENTRANT
{
    /* Not supported */
    UNREFERENCED_PARAMETER(ps);
    return -1;
}

STATICCBK uint32 
medium_get_valid_groups(struct _smedium_s *ps) REENTRANT
{
    return THIS->groups;
}

STATICCBK const char *
medium_get_medium_identification(struct _smedium_s *ps) REENTRANT
{
    UNREFERENCED_PARAMETER(ps);
    return "FLASH";
}

BOOL 
flash_medium_initialize(MEDIUM_FLASH *medium, hsaddr_t addr, uint32 groups) REENTRANT
{
    medium->smedium.open = medium_open;
    medium->smedium.close = medium_close;
    medium->smedium.item_begin = medium_item_begin;
    medium->smedium.item_end = medium_item_end;
    medium->smedium.item_size = medium_item_size;
    medium->smedium.item_version = medium_item_version;
    medium->smedium.read = medium_read;
    medium->smedium.write = medium_write;
#ifdef CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED
    medium->smedium.read_string = medium_read_string;
    medium->smedium.write_string = medium_write_string;
#endif /* CFG_PERSISTENCE_STRING_SUPPORT_INCLUDED */
    medium->smedium.seek = medium_seek;
    medium->smedium.medium_properties = medium_medium_properties;
    medium->smedium.get_medium_identification = medium_get_medium_identification;
    medium->smedium.get_total_data_size = medium_get_total_data_size;
    medium->smedium.get_valid_groups = medium_get_valid_groups;
    
    medium->groups = groups;
    medium->open_type = MED_TYPE_NOT_OPENED;
    medium->start = addr;

    return TRUE;
}
#endif /* CFG_FLASH_SUPPORT_ENABLED && CFG_PERSISTENCE_SUPPORT_ENABLED */
