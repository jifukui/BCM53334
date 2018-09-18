/*
 * $Id: nvram_utils.c,v 1.17 Broadcom SDK $
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
#include "utils/nvram.h"
#include "utils/net.h"

#if defined(CFG_NVRAM_SUPPORT_INCLUDED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED)

/* Hash chains are linked lists of tuples, where each tuple has the
   ASCII representation of the binding appended.  Note that the
   basic declaration provides space for the null-termination only. */

typedef struct nvram_tuple_s {
    struct nvram_tuple_s *next;
    char chksum[4];
    char binding[1];
} nvram_tuple_t;

/*  *********************************************************************
    *  Globals
    ********************************************************************* */

#define HASH_SIZE   1


static nvram_header_t nv_header;
static nvram_tuple_t *hashtab[HASH_SIZE];

/* Local hash function (simplistic for now) */
static uint32
nvram_hash(const char *name)
{
#if HASH_SIZE != 1
    const char *p;
    uint32 sum;

    p = name;
    sum = 0;
    while (*p && *p != '=') sum = 7*sum + *p++;
    return sum % HASH_SIZE;
#else
    return 0;
#endif
}


static nvram_tuple_t *
nvram_lookup(const char *name, size_t len)
{
    nvram_tuple_t *p;
    uint32 hash = nvram_hash(name);
    
    for (p = hashtab[hash]; p != NULL; p = p->next) {
        if (sal_memcmp(name, p->binding, len) == 0 && p->binding[len] == '=')
            return p;           
    }
    return NULL;
}

static void
nvram_insert(uint32 hash, nvram_tuple_t *tuple)
{
    tuple->next = hashtab[hash];
    hashtab[hash] = tuple;
}

static sys_error_t
nvram_internalize(nvram_header_t *hdr)
{
    void *buffer;
    uint32 *dst;
    uint32 hash;
    nvram_tuple_t *tuple;
    const char *base;
    uint32 offset;
    size_t len;
    uint16 chksum = 0;
    uint8 *buf;
    char tuple_chksum[5];
    buffer = (void *)sal_malloc(hdr->len + sizeof(nvram_header_t));
    if (!buffer) {
        return SYS_ERR_OUT_OF_RESOURCE;
    }

    dst = (uint32 *)buffer;
    buf = (uint8 *) buffer;
    if (flash_read((hsaddr_t)NVRAM_BASE + NVRAM_OFFSET, buffer, hdr->len + sizeof(nvram_header_t)) != SYS_OK) {
        return SYS_ERR;
    }

    dst += sizeof(nvram_header_t)/4;

    chksum = sal_checksum(0, 
                           &buf[sizeof(hdr->magic) + sizeof(hdr->chksum)], 
                           hdr->len + sizeof(nvram_header_t) - sizeof(hdr->magic) - sizeof(hdr->chksum)
                          );         

    if (chksum != hdr->chksum) {
        sal_printf("nvram: global chksum error\n");
        return SYS_ERR;
    }
    base = (const char *)buffer;
    offset = sizeof(nvram_header_t);

    while (offset < (hdr->len + sizeof(nvram_header_t)) && *(base+offset) != 0) {

        len = sal_strlen(base+offset);

        tuple = (nvram_tuple_t *)sal_malloc(sizeof(nvram_tuple_t) + len + 1);

        if (tuple == NULL) {
            /* Out of memory */
            return SYS_ERR_OUT_OF_RESOURCE;
        }        

        sal_strcpy(tuple->chksum, base+offset);

        sprintf(tuple_chksum, "%04X", sal_checksum(0, tuple->binding, sal_strlen(tuple->binding)));

        if (sal_memcmp(tuple_chksum, tuple->chksum, 4) == 0) {

            hash = nvram_hash(tuple->binding);
         
            /* Check for duplicates here? */
            nvram_insert(hash, tuple);
        } else {
           sal_printf("item chksum fail :%s\n", tuple->chksum);

        }
        offset += len + 1;
    }

    sal_free(buffer);
    return SYS_OK;
}


sys_error_t
nvram_enum(sys_error_t (*proc)(const char *tuple))
{
    int i;
    nvram_tuple_t *p;

    for (i = 0; i < HASH_SIZE; i++) {
        for (p = hashtab[i]; p != NULL; p = p->next) {
            if ((*proc)(p->binding) != SYS_OK) {
                return SYS_ERR;
            }
        }
    }
    return SYS_OK;
}
sys_error_t 
nvram_show_tuple(const char *tuple) {
     extern int um_console_print(const char *s);
     um_console_print(tuple);
     um_console_print("\n");
     return SYS_OK;

}

/*  *********************************************************************
    *  Functions
    ********************************************************************* */

/*  *********************************************************************
    *  nvram_init()
    *
    *  Initialize the a-list from flash.
    *
    *  Return value:
    *      error code (0 for success)
    ********************************************************************* */

sys_error_t
nvram_init (void)
{
    int i;
    uint32 *wp;

	for (i = 0; i < HASH_SIZE; i++) {
        hashtab[i] = NULL;
    }

    if (flash_read((hsaddr_t)NVRAM_BASE + NVRAM_OFFSET, &nv_header, sizeof(nvram_header_t)) != SYS_OK) {
        return SYS_ERR;
    }

    /* the nvram data is little endian on storage */
    wp = (uint32 *) &nv_header;    
    for (i = 0; i < sizeof(nvram_header_t); i += 4) {
        *wp = htol32(*wp);
        wp++;
    }	
	
    if (nv_header.magic == NVRAM_MAGIC) {
        if (nvram_internalize(&nv_header) == SYS_OK) {
            return SYS_OK;
		};
    } 
	
#ifdef __BOOTLOADER__
    return SYS_ERR;
#else
    sal_memset(&nv_header, 0, sizeof(nvram_header_t));
    nv_header.magic = NVRAM_MAGIC;
    nv_header.len = sizeof(nvram_header_t);
    nv_header.crc_ver_init = (NVRAM_VERSION << NVRAM_VER_SHIFT);
    return SYS_OK;
#endif /* __BOOTLOADER__ */	
}

const char *
nvram_get(const char *name)
{
    size_t len = sal_strlen(name);
    nvram_tuple_t *p;

    p = nvram_lookup(name, len);
    if (p != NULL) {
        return &p->binding[len + 1]; /* "value" */
    }
    return NULL;
}

/* Remove following API in loader to save space */
#ifndef __BOOTLOADER__
static BOOL
nvram_delete(uint32 hash, nvram_tuple_t *tuple)
{
    nvram_tuple_t *p, *prev;

    prev = NULL;
    for (p = hashtab[hash]; p != NULL; p = p->next) {
        if (p == tuple) {
            if (prev == NULL) {
                hashtab[hash] = p->next;
            } else {
                prev->next = p->next;
            }
            return TRUE;
        }
        prev = p;
    }
    return FALSE; /* not found */
}

sys_error_t
nvram_set(const char *name, const char *value)
{
    unsigned int hash;
    size_t len = sal_strlen(name);
    nvram_tuple_t *tuple, *p;

    if (len == 0) return SYS_ERR;

    if (sal_strlen(value) == 0) return SYS_ERR;

	if (sal_strchr(name, '=')) return SYS_ERR_PARAMETER;
	
    tuple = (nvram_tuple_t *)sal_malloc(
          sizeof(nvram_tuple_t) + len + 1 + sal_strlen(value));
    if (tuple == NULL) {    /* Out of memory */
        return SYS_ERR_OUT_OF_RESOURCE;
    }
    
    sal_strcpy(tuple->binding, name);
    sal_strcat(tuple->binding, "=");
    sal_strcat(tuple->binding, value);

    hash = nvram_hash(name);
    p = nvram_lookup(name, len);
    if (p != NULL) {
        nvram_delete(hash, p);
        sal_free(p);
    }

    nvram_insert(hash, tuple);
    return SYS_OK;
}

sys_error_t
nvram_unset(const char *name)
{
    uint32 hash;
    size_t len = sal_strlen(name);
    nvram_tuple_t *p;

    hash = nvram_hash(name);
    p = nvram_lookup(name, len);
    if (p == NULL)
      return SYS_ERR;  /* Not currently bound */

    nvram_delete(hash, p);
    sal_free(p);

    return SYS_OK;
}

sys_error_t
nvram_commit()
{
    int i;
    nvram_tuple_t *p;
    size_t total;
    nvram_header_t *hdr;
    uint8 *cp;
    uint32 *wp;
    sys_error_t res;
    uint8 *buf;
    uint32 block_size;
    char chksum[5];
    block_size = flash_block_size(NVRAM_BASE);
    /* Allocate and copy block data */
    buf = sal_malloc(block_size);
    if (!buf) {
#if CFG_CONSOLE_ENABLED
        sal_printf("nvram_commit: malloc failed!\n");
#endif
        return SYS_ERR_OUT_OF_RESOURCE;
    }
  
    flash_read(NVRAM_BASE, buf, block_size);
    
    total = 0;

    for (i = 0; i < HASH_SIZE; i++) {
        for (p = hashtab[i]; p != NULL; p = p->next) {
            total += sal_strlen(p->binding) + 1 + sizeof(p->chksum);
        }
    }

    if (total > (NVRAM_SPACE + NVRAM_OFFSET)) {
        return SYS_ERR;
    }

    hdr = (nvram_header_t *) (buf + NVRAM_OFFSET);

    sal_memcpy(hdr, &nv_header, sizeof(nvram_header_t));
    hdr->len = total;
    nv_header.len = total;
    hdr->crc_ver_init &= ~NVRAM_CRC_MASK;
    
    cp = (uint8 *)hdr + sizeof(nvram_header_t);

    for (i = 0; i < HASH_SIZE; i++) {
        for (p = hashtab[i]; p != NULL; p = p->next) {
            size_t n = sal_strlen(p->binding) + 1 + sizeof(p->chksum);
            sprintf(chksum, "%04X", sal_checksum(0, p->binding, sal_strlen(p->binding)));
            sal_memcpy(p->chksum, chksum, 4);
            sal_memcpy(cp, p->chksum, n);
            cp += n;
        }
    }
    while (cp < (uint8 *)hdr + hdr->len) {
        *cp++ = 0; /* terminate and pad */
    }
	
    /* the nvram data is little endian on storage */
    wp = (uint32 *) hdr;    
    for (i = 0; i < sizeof(nvram_header_t); i += 4) {
        *wp = htol32(*wp);
        wp++;
    }	
    hdr->chksum = sal_checksum(0, &hdr->len, nv_header.len + sizeof(nvram_header_t) - sizeof(hdr->magic) - sizeof(hdr->chksum));  
    hdr->chksum = htol32(hdr->chksum);
	
    res = flash_erase((hsaddr_t)NVRAM_BASE, block_size);
    if (res != SYS_OK) {
#if CFG_CONSOLE_ENABLED
        sal_printf("nvram_commit: failed to erase region 0x%x\n", (hsaddr_t)NVRAM_BASE);
#endif
        goto done;
    }

    res = flash_program((hsaddr_t)NVRAM_BASE, (const void *)buf, block_size);

    if (res != SYS_OK) {
#if CFG_CONSOLE_ENABLED
        sal_printf("nvram_commit: failed to program region 0x%x\n", (hsaddr_t)NVRAM_BASE);
#endif
        goto done;
    }
done:
    sal_free(buf);
    return SYS_OK;
}
#endif /* !__BOOTLOADER__ */

#endif /* defined(CFG_NVRAM_SUPPORT_INCLUDED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED) */

