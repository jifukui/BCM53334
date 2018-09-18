/*
 * $Id: sal_alloc.c,v 1.6 Broadcom SDK $
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

extern void sal_alloc_init(void *addr, uint16 len);
extern void sal_dma_alloc_init(void *addr, uint16 len);

typedef uint32 uintptr_t;

/*  *********************************************************************
    *  Constants
    ********************************************************************* */

#define MEMNODE_SEAL 0xFAAFA123     /* just some random constant */
#define MINBLKSIZE 64

/*  *********************************************************************
    *  Types
    ********************************************************************* */

typedef enum { memnode_free = 0, memnode_alloc } memnode_status_t;

typedef struct memnode_s {
    uint32 seal;
    struct memnode_s *next;     /* pointer to next node */
    uint32 length;              /* length of the entire data section */
    memnode_status_t status;    /* alloc/free status */
    uint8  *data;               /* points to actual user data */
    void   *memnodeptr;         /* memnode back pointer (see comments) */
} memnode_t;

struct mempool_s {
    memnode_t *root;            /* pointer to root node */
    uint8     *base;            /* base of memory region */
    uint32    length;           /* size of memory region */
};

typedef struct mempool_s mempool_t;

#define memnode_data(t,m) (t) (((memnode_t *) (m))+1)

/*  *********************************************************************
    *  Globals
    ********************************************************************* */

static mempool_t kmempool;          /* default pool */
static mempool_t dmamempool;        /* DMA pool */

static void
_sal_alloc_init(mempool_t *p, void *addr, uint16 len)
{
    p->root = (memnode_t *)addr;
    p->root->seal = MEMNODE_SEAL;
    p->root->length = (uint32)len - sizeof(memnode_t);
    p->root->data = memnode_data(uint8 *,p->root);
    p->root->status = memnode_free;
    p->root->next = NULL;

    p->base = (uint8 *)addr;
    p->length = len;
}

void
sal_alloc_init(void *addr, uint16 len)
{
    _sal_alloc_init(&kmempool, addr, len);
}

void
sal_dma_alloc_init(void *addr, uint16 len)
{
    _sal_alloc_init(&dmamempool, addr, len);
}

/*  *********************************************************************
    *  kmemcompact(pool)
    *
    *  Compact the memory blocks, coalescing consectutive free blocks
    *  on the list.
    *
    *  Input parameters:
    *      pool - pool descriptor
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

static void kmemcompact(mempool_t *p)
{
    memnode_t *m;
    int compacted;

    do {
        compacted = 0;

        for (m = p->root; m; m = m->next) {

            /* Check seal to be sure that we're doing ok */

            if (m->seal != MEMNODE_SEAL) {
    #ifdef TESTPROG
                printf("Memory list corrupted!\n");
    #endif
                return;
            }

            /*
             * If we're not on the last block and both this
             * block and the next one are free, combine them
             */

            if (m->next &&
               (m->status == memnode_free) &&
               (m->next->status == memnode_free)) {
                m->length += sizeof(memnode_t) + m->next->length;
                m->next->seal = 0;
                m->next = m->next->next;
                compacted++;
            }
            /* Keep going till we make a pass without doing anything. */
        }
    } while (compacted > 0);
}


/*  *********************************************************************
    *  kfree(ptr)
    *
    *  Return some memory to the pool.
    *
    *  Input parameters:
    *      ptr - pointer to something allocated via kmalloc()
    *
    *  Return value:
    *      nothing
    ********************************************************************* */
static
void _sal_free(mempool_t *p, void *ptr)
{
    memnode_t **backptr;
    memnode_t *m;

    if (ptr == NULL) {
        return;
    }

    if (((uint8 *) ptr < p->base) ||
    ((uint8 *) ptr >= (p->base + p->length))) {
#ifdef TESTPROG
        sal_printf("Pointer %08X does not belong to pool %08X\n",ptr,pool);
#endif
        return;
    }

    backptr = (memnode_t **) (((uint8 *) ptr) - sizeof(memnode_t *));
    m = *backptr;

    if (m->seal != MEMNODE_SEAL) {
#ifdef TESTPROG
    printf("Invalid node freed: %08X\n",m);
#endif
        return;
    }

    m->status = memnode_free;

    kmemcompact(p);
}

/*  *********************************************************************
    *  kmalloc(pool,size,align)
    *
    *  Allocate some memory from the pool.
    *
    *  Input parameters:
    *      pool - pool structure
    *      size - size of item to allocate
    *      align - alignment (must be zero or a power of 2)
    *
    *  Return value:
    *      pointer to data, or NULL if no memory left
    ********************************************************************* */

static void *
_sal_malloc(mempool_t *p, uint32 size)
{
    memnode_t *m;
    memnode_t *newm;
    memnode_t **backptr;
    uintptr_t daddr = 0;
    uintptr_t realsize = 0;
    uintptr_t extra;
    uintptr_t blkend;
    uintptr_t ptralign;

    /*
     * Everything should be aligned by at least the
     * size of an int32
     */

    /* Make it aligned with cache line size. */
    ptralign = 32;

    /*
     * Everything should be at least a multiple of the
     * size of a pointer.
     */

    if (size == 0) {
        size = sizeof(void *);
    }

    if (size & (sizeof(void *)-1)) {
        size += sizeof(void *);
        size &= ~(sizeof(void *)-1);
    }

    /*
     * Find a memnode at least big enough to hold the storage we
     * want.
     */

    for (m = p->root; m; m = m->next) {

        if (m->status == memnode_alloc) continue;

        /*
         * If we wanted a particular alignment, we will
         * need to adjust the size.
         */

        daddr = memnode_data(uintptr_t,m);
        extra = 0;
        if (daddr & (ptralign-1)) {
            extra = ptralign - (daddr & (ptralign-1));
        }
        realsize = size + extra;

        if (m->length < realsize) continue;
        break;
    }

    /*
     * If m is null, there's no memory left.
     */

    if (m == NULL) {
        return NULL;
    }

    /*
     * Otherwise, use this block.  Calculate the address of the data
     * to preserve the alignment.
     */

    if (daddr & (ptralign-1)) {
        daddr += ptralign;
        daddr &= ~(ptralign-1);
    }

    /* Mark this node as allocated. */

    m->data   = (unsigned char *) daddr;
    m->status = memnode_alloc;

    /*
     * Okay, this is ugly.  Store a pointer to the original
     * memnode just before what we've allocated.  It's guaranteed
     * to be aligned at least well enough for this pointer.
     * If for some reason the memnode was already exactly
     * aligned, backing up will put us inside the memnode
     * structure itself... that's why the memnodeptr field
     * is there, as a placeholder for this eventuality.
     */

    backptr   = (memnode_t **) (m->data - sizeof(memnode_t *));
    *backptr  = m;

    /*
     * See if we need to split it.
     * Don't bother to split if the resulting size will be
     * less than MINBLKSIZE bytes
     */

    if (m->length - realsize < MINBLKSIZE) {
        return m->data;
    }

    /*
     * Split this block.  Align the address on a pointer-size
     * boundary.
     */

    daddr += size;
    if (daddr & (uintptr_t)(sizeof(void *)-1)) {
        daddr += (uintptr_t)sizeof(void *);
        daddr &= ~(uintptr_t)(sizeof(void *)-1);
    }

    blkend = memnode_data(uintptr_t,m) + (uintptr_t)(m->length);

    newm = (memnode_t *) daddr;

    newm->next   = m->next;
    m->length    = (uint32) (daddr - memnode_data(uintptr_t,m));
    m->next      = newm;
    m->status    = memnode_alloc;
    newm->seal   = MEMNODE_SEAL;
    newm->data    = memnode_data(uint8 *,newm);
    newm->length = (uint32) (blkend - memnode_data(uintptr_t,newm));
    newm->status = memnode_free;

    return m->data;
}

void *
sal_malloc(uint32 size) {
    return _sal_malloc(&kmempool, size);
}

void sal_free(void *ptr)
{
    _sal_free(&kmempool, ptr);
}

void *
sal_dma_malloc(uint32 size) {
    return _sal_malloc(&dmamempool, size);
}

void sal_dma_free(void *ptr)
{
    _sal_free(&dmamempool, ptr);
}
