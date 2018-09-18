/*
 * $Id: arm.h,v 1.1 Broadcom SDK $
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
 * File:    arm.h
 * Purpose: ARM-specific macros
 */

#ifndef MOS_ARM_H
#define MOS_ARM_H

#ifdef __ASSEMBLER__
/*
 * LEAF - declare leaf routine
 */
#define LEAF(function)              \
        .section .text.function, "ax";  \
        .global function;       \
        .func   function;       \
function:

#define THUMBLEAF(function)         \
        .section .text.function, "ax";  \
        .global function;       \
        .func   function;       \
        .thumb;             \
        .thumb_func;            \
function:

/*
 * END - mark end of function
 */
#define END(function)               \
        .ltorg;             \
        .endfunc;           \
        .size   function, . - function
#endif

#ifndef __ASSEMBLER__
#include "types.h"
#include "board.h"

void membar(void);
void dcache_invalidate(void * addr, uint32 len);
void dcache_flush(void * addr, uint32 len);
void dcache_flush_and_inv(void * addr, uint32 len);
void dcache_flush_all(void);
void dcache_flush_and_inv_all(void);

void icache_invalidate(void * addr, uint32 len);

#define ARM_CYCLE_CNTR_MONITOR_BIT 0x80000000

#define ARM_MODE asm volatile(                                  \
    "   ldr r3,=99997f;"                                        \
    "   bx  r3;"                                                \
    "   .pool;"                                                 \
    "   .code 32;"                                              \
    "99997:;"                                                   \
    : : : "r3");


#define THUMB_MODE asm volatile(                                \
    "   ldr r3,=99996f+1;"                                      \
    "   bx  r3;"                                                \
    "   .pool;"                                                 \
    "   .code 16;"                                              \
    "99996: "                                                   \
    : : : "r3");

/* Define macros that switch to ARM mode around T2 instructions if
   this processor does not support T2  */
#if defined(THUMB2) || defined(TOOLCHAIN_arm)
#define THUMB2_PRE(reg)
#define THUMB2_POST(reg)

#else
#define THUMB2_PRE(reg)                                         \
    "   ldr " #reg ",=99997f;"                                  \
    "   bx  " #reg ";"                                          \
    "   .pool;"                                                 \
    "   .code 32;"                                              \
    "99997:;"

#define THUMB2_POST(reg)                                        \
    "   ldr " #reg ",=99996f+1;"                                \
    "   bx  " #reg ";"                                          \
    "   .pool;"                                                 \
    "   .code 16;"                                              \
    "99996:; "
#endif

#endif

/* Mask interrupts */
#define WRITE_LOW_PRIO_RUPT_MASK(mask) \
    WRITE_REG(CMICM_IRQ_MASK_REG, mask);

#define WRITE_HIGH_PRIO_RUPT_MASK(val) \
    WRITE_REG(CMICM_FIQ_MASK_REG, val);

#define MEM_BAR                                                 \
    asm volatile (                                              \
       THUMB2_PRE(r3)                                           \
       "MCR p15, 0, r0, c7, c10, 5;"                            \
       THUMB2_POST(r3)                                          \
       : : : "r3");

#define SYNC_BAR                                                \
    asm volatile (                                              \
       THUMB2_PRE(r3)                                           \
       "MCR p15, 0, r0, c7, c10, 4;"                            \
       THUMB2_POST(r3)                                          \
       : : : "r3");


/*  Cache operations */

#define CACHELINE_SIZE          (8*4)
#define CACHELINE_MASK          (~((CACHELINE_SIZE) - 1))

#ifdef ICACHE
#define ICACHE_INVALIDATE_ALL                                   \
    asm volatile (                                              \
       THUMB2_PRE(r3)                                           \
       "MCR p15, 0, %0, c7, c5, 0;" /* Icache*/                 \
       "MCR p15, 0, %0, c7, c5, 6;" /* Branch predictor */      \
       THUMB2_POST(r3)                                          \
       : : "r" (0) : "r3");
#else
#define ICACHE_INVALIDATE_ALL                   do { } while (0)
#endif

#define FLUSH_PREFETCH_BUFFER                                   \
    asm volatile (                                              \
       THUMB2_PRE(r3)                                           \
       "MCR p15, 0, %0, c7, c5, 4;"                             \
       THUMB2_POST(r3)                                          \
       : : "r" (0) : "r3");

#ifdef DCACHE

#if 0
#define DCACHE_INVALIDATE(addr, len)                                    \
    for (uint32_t _local_addr = (addr) & CACHELINE_MASK;;) {            \
        asm volatile (                                                  \
          THUMB2_PRE(r3)                                                \
          "   mcr p15, 0, %0, c7, c6, 1;"                               \
          THUMB2_POST(r3)                                               \
          : : "r" (_local_addr) : "r3");                                \
        _local_addr += CACHELINE_SIZE;                                  \
        if (_local_addr >= (((addr) + (len) - 1) & CACHELINE_MASK)) {   \
            break;                                                      \
        }                                                               \
    }                                                                   \
    MEM_BAR;
#else
#define DCACHE_INVALIDATE(addr, len)                                    \
    dcache_invalidate((void *) ((uint32_t) addr & CACHELINE_MASK),      \
                      (len + CACHELINE_SIZE - 1) & CACHELINE_MASK);
#endif

#if 0
#define DCACHE_FLUSH(addr, len)                                         \
    for (uint32_t _local_addr = (addr) & CACHELINE_MASK;;) {            \
        asm volatile (                                                  \
          THUMB2_PRE(r3)                                                \
          "    mcr p15, 0, %0, c7, c11, 1;"                             \
          THUMB2_POST(r3)                                               \
          : : "r" (_local_addr) : "r3");                                \
        _local_addr += CACHELINE_SIZE;                                  \
        if (_local_addr >= (((addr) + (len) - 1) & CACHELINE_MASK)) {   \
            break;                                                      \
        }                                                               \
    }                                                                   \
    MEM_BAR;
#else
#define DCACHE_FLUSH(addr, len)                                         \
    dcache_flush((void *) ((uint32_t) addr & CACHELINE_MASK),           \
                 (len + CACHELINE_SIZE - 1) & CACHELINE_MASK);
#endif


#if 0
#define DCACHE_FLUSH_AND_INV(addr, len)                                 \
    for (uint32_t _local_addr = (addr) & CACHELINE_MASK;;) {            \
        asm volatile (                                                  \
          THUMB2_PRE(r3)                                                \
          "MCR p15, 0, %0, c7, c14, 1;"                                 \
          THUMB2_POST(r3)                                               \
          : : "r" (_local_addr) : "r3");                                \
        _local_addr += CACHELINE_SIZE;                                  \
        if (_local_addr >= (((addr) + (len) - 1) & CACHELINE_MASK)) {   \
            break;                                                      \
        }                                                               \
    }                                                                   \
    MEM_BAR;
#else
#define DCACHE_FLUSH_AND_INV(addr, len)                                 \
    dcache_flush_and_inv((void *) ((uint32_t) addr & CACHELINE_MASK),   \
                         (len + CACHELINE_SIZE - 1) & CACHELINE_MASK);
#endif

#else

#define DCACHE_INVALIDATE(addr, len)            do { } while (0)
#define DCACHE_FLUSH(addr, len)                 do { } while (0)
#define DCACHE_FLUSH_AND_INV(addr, len)         do { } while (0)

#endif

#define ICACHE_ENABLE                                                   \
    asm volatile(                                                       \
     THUMB2_PRE(r1)                                                     \
    "MRC p15, 0, r1, c1, c0, 0;"        /* Readsysctl*/                 \
    "ORR R1, R1, #0x1000;"              /* Set icache enable */         \
    "MCR p15, 0, r0, c7, c5, 0;"         /* Invalidate icache */        \
    "MCR p15, 0, r1, c1, c0, 0;"        /* Write sys ctl */             \
    "MCR p15, 0, r0, c7, c5, 6;"         /* Branch predictor array */   \
     THUMB2_POST(r1)                                                    \
    ::: "r1");


#define DCACHE_ENABLE                                                   \
    asm volatile(                                                       \
    THUMB2_PRE(r1)                                                      \
    "MRC p15, 0, r1, c1, c0, 0;"        /* Read sys ctl */              \
    "ORR R1,R1,#0x0004;"                /* Set cache enable */          \
    "DSB;"                              /* Data sync bar */             \
    "MCR p15, 0, r0, c15, c5, 0;"       /* Invalidate dcache */         \
    "MCR p15, 0, r1, c1, c0, 0;"        /* Write sys ctl */             \
    THUMB2_POST(r1)                                                     \
    ::: "r1");

/*  MPU Operations */


/* Size plus enable bits */
#define MPU_SIZE_DISABLE        0x00
#define MPU_SIZE_32             0x09            /* 0b001001 */
#define MPU_SIZE_64             0x0b            /* 0b001011 */
#define MPU_SIZE_128            0x0d            /* 0b001101 */
#define MPU_SIZE_256            0x0f            /* 0b001111 */
#define MPU_SIZE_512            0x11            /* 0b010001 */
#define MPU_SIZE_1K             0x13            /* 0b010011 */
#define MPU_SIZE_2K             0x15            /* 0b010101 */
#define MPU_SIZE_4K             0x17            /* 0b010111 */
#define MPU_SIZE_8K             0x19            /* 0b011001 */
#define MPU_SIZE_16K            0x1b            /* 0b011011 */
#define MPU_SIZE_32K            0x1d            /* 0b011101 */
#define MPU_SIZE_64K            0x1f            /* 0b011111 */
#define MPU_SIZE_128K           0x21            /* 0b100001 */
#define MPU_SIZE_256K           0x23            /* 0b100011 */
#define MPU_SIZE_512K           0x25            /* 0b100101 */
#define MPU_SIZE_1M             0x27            /* 0b100111 */
#define MPU_SIZE_2M             0x29            /* 0b101001 */
#define MPU_SIZE_4M             0x2b            /* 0b101011 */
#define MPU_SIZE_8M             0x2d            /* 0b101101 */
#define MPU_SIZE_16M            0x2f            /* 0b101111 */
#define MPU_SIZE_32M            0x31            /* 0b110001 */
#define MPU_SIZE_64M            0x33            /* 0b110011 */
#define MPU_SIZE_128M           0x35            /* 0b110101 */
#define MPU_SIZE_256M           0x37            /* 0b110111 */
#define MPU_SIZE_512M           0x39            /* 0b111001 */
#define MPU_SIZE_1G             0x3b            /* 0b111011 */
#define MPU_SIZE_2G             0x3d            /* 0b111101 */
#define MPU_SIZE_4G             0x3f            /* 0b111111 */

#ifndef __ASSEMBLER__
typedef struct mpu_region_size_s {
    unsigned short reserved1:16;
    unsigned short subregion:8;
    unsigned short reserved2:2;
    unsigned short size_enable:6;
} mpu_region_size_t;

/* Endian-invariant */
#define MPU_REGION_SIZE(r) (((r).subregion << 8) | (r).size_enable)

/* Convert from MPU_SIZE_X to X: take bits 1..5, add one, to get lg(X). Return 2^lg(X). */
#define MPU_SIZE_FROM_FIELD(f) (1<<(((f)>>1)+1))
#endif

/* CPSR defines */
#define CPSR_MODE_BITS  0x1f
#define CPSR_MODE_IRQ   0x12
#define CPSR_MODE_SVC   0x13

/* Access bits definition (other values apply to user-mode) */
#define MPU_ACCESS_NONE 0x00            /* 0b000 */
#define MPU_ACCESS_FULL 0x03            /* 0b011 */

/* Type extensions */
#define MPU_TE_STRONGLY_ORDERED         0x00    /* 0b000000 */
#define MPU_TE_SHARABLE_DEVICE          0x03    /* 0b000101 */
#define MPU_TE_UNSHARABLE_DEVICE        0x10    /* 0b010000 */
#define MPU_TE_UNCACHABLE_MEMORY        0x08    /* 0b001000 */
#define MPU_TE_CACHABLE_MEMORY_WB_WA    0x29    /* 0b101001 */
#define MPU_TE_CACHABLE_MEMORY_WT       0x32    /* 0b110010 */
#define MPU_TE_CACHABLE_MEMORY_WB_NWA   0x3b    /* 0b111011 */

#define MPU_RC_EXECUTE 0
#define MPU_RC_NO_EXECUTE 1

#ifndef __ASSEMBLER__
typedef struct mpu_region_control_s {
    unsigned int reserved1:19;
    unsigned short no_execute:1;
    unsigned short reserved2:1;
    unsigned short access:3;
    unsigned short reserved3:2;
    /* type_extension (s, c, b bits combined) */
    unsigned short type_extension:6;
} mpu_region_control_t;

/* Endian-invariant */
#define MPU_REGION_CONTROL(r) (((r).no_execute << 12) | ((r).access << 8) | (r).type_extension)
#endif

/* Locks should be set before calling these;  However, they are used
   early enough that we don't bother (no threads running). */
#ifdef TOOLCHAIN_gnu

#define CLZ32(val) __builtin_clzl(val)

#define MPU_REGION_SELECT(region)                                       \
    asm volatile (                                                      \
                  THUMB2_PRE(r3)                                        \
                  "MCR p15, 0, %0, c6, c2, 0;"                          \
                  THUMB2_POST(r3)                                       \
                  : : "r" (region) : "r3");

#define MPU_DREGION_DEFINE(base, size, control)                         \
    asm volatile (                                                      \
           THUMB2_PRE(r3)                                               \
           "MCR p15, 0, %0, c6, c1, 0;"                                 \
           "MCR p15, 0, %1, c6, c1, 4;"                                 \
           /* Enable bit in size reg so do that last */                 \
           "MCR p15, 0, %2, c6, c1, 2;"                                 \
           THUMB2_POST(r3)                                              \
           : : "r" (base),                                              \
           "r"(control), "r"(size) : "r3");

#define MPU_DREGION_DISABLE                                             \
    asm volatile (                                                      \
           THUMB2_PRE(r3)                                               \
           "MCR p15, 0, %0, c6, c1, 2;"                                 \
           THUMB2_POST(r3)                                              \
           : : "r" (0) : "r3");

#define MPU_IREGION_DEFINE(base, size, control)                         \
    asm volatile (                                                      \
           THUMB2_PRE(r3)                                               \
           "MCR p15, 0, %0, c6, c1, 1;"                                 \
           "MCR p15, 0, %1, c6, c1, 5;"                                 \
           /* Enable bit in size reg so do that last */                 \
           "MCR p15, 0, %2, c6, c1, 3;"                                 \
           THUMB2_POST(r3)                                              \
           : : "r" (base),                                              \
           "r"(control), "r"(size) : "r3");

#define MPU_IREGION_DISABLE                                             \
    asm volatile (                                                      \
           THUMB2_PRE(r3)                                               \
           "MCR p15, 0, %0, c6, c1, 3;"                                 \
           THUMB2_POST(r3)                                              \
           : : "r" (0) : "r3");

#define MPU_ENABLE                                                      \
    asm volatile(                                                       \
        THUMB2_PRE(r1)                                                  \
        "MRC p15, 0, r1, c1, c0, 0;"                                    \
        "ORR r1,r1,#0x0001;"                                            \
        "MCR p15, 0, r1, c1, c0, 0;"                                    \
        "ISB;"                          /* instruction sync bar */      \
        THUMB2_POST(r1)                                                 \
        ::: "r1");

#define MPU_DISABLE                                                     \
    asm volatile(                                                       \
        THUMB2_PRE(r1)                                                  \
        "MRC p15, 0, r1, c1, c0, 0;"                                    \
        "BIC r1,r1,#0x0001;"                                            \
        "MCR p15, 0, r1, c1, c0, 0;"                                    \
        "ISB;"                          /* instruction sync bar */      \
        THUMB2_POST(r1)                                                 \
        ::: "r1");

#define LOW_VECTORS                                                     \
    asm volatile(                                                       \
        THUMB2_PRE(r1)                                                  \
        "mcr p15,0,r1,c1,c0,0;"                                         \
        "bic r1,r1,#0x2000;"                                            \
        "mrc p15,0,r1,c1,c0,0;"                                         \
        THUMB2_POST(r1)                                                 \
        ::: "r1");


#endif

#ifdef TOOLCHAIN_arm

#define CLZ32(val) __clz(val)

#define MPU_REGION_SELECT(region)                                       \
    { uint32_t r = (uint32_t)(region);                                  \
    __asm {                                                             \
        MCR p15, 0, r, c6, c2, 0; } }

#define MPU_IREGION_DEFINE(base, size, control)                         \
    { uint32_t b = (uint32_t)(base);                                    \
    uint32_t s = (uint32_t)(size);                                      \
    uint32_t c = (uint32_t)(control);                                   \
    __asm {                                                             \
        MCR p15, 0, b, c6, c1, 1;                                       \
        MCR p15, 0, c, c6, c1, 5;                                       \
        /* Enable bit in size reg so do that last */                    \
        MCR p15, 0, s, c6, c1, 3; } }

#define MPU_IREGION_DISABLE                                             \
    { uint32_t z = 0;                                                   \
    __asm {                                                             \
        MCR p15, 0, z, c6, c1, 3; } }

#define MPU_DREGION_DEFINE(base, size, control)                         \
    { uint32_t b = (uint32_t)(base);                                    \
    uint32_t s = (uint32_t)(size);                                      \
    uint32_t c = (uint32_t)(control);                                   \
    __asm {                                                             \
        MCR p15, 0, b, c6, c1, 0;                                       \
        MCR p15, 0, c, c6, c1, 4;                                       \
        /* Enable bit in size reg so do that last */                    \
        MCR p15, 0, s, c6, c1, 2; } }

#define MPU_DREGION_DISABLE                                             \
    { uint32_t z = 0;                                                   \
    __asm {                                                             \
        MCR p15, 0, z, c6, c1, 2; } }

#define MPU_ENABLE                                                      \
    {                                                                   \
        uint32_t r;                                                     \
        __asm {                                                         \
            MRC p15, 0, r, c1, c0, 0;                                   \
            ORR r,r,#0x0001;                                            \
            MCR p15, 0, r, c1, c0, 0;                                   \
            MCR p15, 0, 0, c7, c10, 4; } }

#define MPU_DISABLE                                                     \
    {                                                                   \
        uint32_t r;                                                     \
        __asm {                                                         \
            MRC p15, 0, r, c1, c0, 0;                                   \
            BIC r,r,#0x0001;                                            \
            MCR p15, 0, r, c1, c0, 0;                                   \
            MCR p15, 0, 0, c7, c10, 4; } }

#define LOW_VECTORS                                                     \
    {                                                                   \
        uint32_t r;                                                     \
        __asm {                                                         \
            MRC p15, 0, r, c1, c0, 0;                                   \
            BIC r,r,#0x2000;                                            \
            MCR p15, 0, r, c1, c0, 0; } }

#endif


#ifndef __ASSEMBLER__
typedef struct mpu_region_entry_s {
    uint32 base;              /* Base address of region */
    mpu_region_size_t size;
    mpu_region_control_t control;
} mpu_region_entry_t;
#endif


#endif
