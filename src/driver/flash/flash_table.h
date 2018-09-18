/*
 * $Id: flash_table.h,v 1.5 Broadcom SDK $
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


extern struct flash_dev_funs mx25l_funs;

/* Flash device flags */
#define FLAG_FSR_POLL   (1 << (0))

/* Block information */

/* 256Mb flash memory */
//static flash_block_info_t s25fl256_info[2] = {{ 4096, 32 }, { 65536, 510 }};

/*  512Mb flash memory, use 4K subsector for erase */
static flash_block_info_t uniform_4K_512Mb_info = { 4096, 16384 };

/* 256Mb flash memory, use 4K subsector for erase */
static flash_block_info_t uniform_4K_256Mb_info = { 4096, 8192 };

/* 128Mb flash memory, use 4K subsector for erase */
static flash_block_info_t uniform_4K_128Mb_info = { 4096, 4096 };

/* 64Mb flash memory, use 4K sector for erase */
static flash_block_info_t uniform_4K_64Mb_info = { 4096, 2048 };

static flash_block_info_t uniform_4K_16Mb_info = {4096, 512};
/* SPANSION */
#define MANUFACTURER_ID_SPANSION     0x01
/* MXIC (MACRONIX) */
#define MANUFACTURER_ID_MACRONIX     0xC2
/* SST (acquired by Microchip)*/
#define MANUFACTURER_ID_SST          0xBF
/* Winbond*/
#define MANUFACTURER_ID_WINBOND      0xEF
/* Fund by Intel, STM and then acquired by Micron */
#define MANUFACTURER_ID_NUMONYX      0x89     
/* Micron */
#define MANUFACTURER_ID_MICRON       0x20     
/* EON */
#define MANUFACTURER_ID_EON          0x1C
/* UNKNOW flash type */
#define MANUFACTURER_ID_UNKNOWN      0x00

const flash_dev_t n25q256_dev =  { {MANUFACTURER_ID_MICRON,   0xba, 0x19, 0x00}, {0xFF, 0xFF, 0xFF, 0x00}, &mx25l_funs, FLAG_FSR_POLL, CFG_FLASH_START_ADDRESS, 0, 1, &uniform_4K_256Mb_info, "N25Q256", NULL};
const flash_dev_t w25q64cv_dev = { {MANUFACTURER_ID_WINBOND,  0x40, 0x17, 0x0},  {0xFF, 0xFF, 0xFF, 0x0}, &mx25l_funs, 0, CFG_FLASH_START_ADDRESS, 0, 1, &uniform_4K_64Mb_info, "W25Q64CV", NULL};


const flash_dev_t flash_dev_support_table[] = {
    { {MANUFACTURER_ID_WINBOND,  0x40, 0x17, 0x0},  {0xFF, 0xFF, 0xFF, 0x0}, &mx25l_funs, 0, CFG_FLASH_START_ADDRESS, 0, 1, &uniform_4K_64Mb_info, "W25Q64CV", NULL}, 
    { {MANUFACTURER_ID_MICRON,   0xba, 0x19, 0x00}, {0xFF, 0xFF, 0xFF, 0x00}, &mx25l_funs, FLAG_FSR_POLL, CFG_FLASH_START_ADDRESS, 0, 1, &uniform_4K_256Mb_info, "N25Q256", NULL}, 
    { {MANUFACTURER_ID_MICRON,   0xba, 0x20, 0x00}, {0xFF, 0xFF, 0xFF, 0x00}, &mx25l_funs, FLAG_FSR_POLL, CFG_FLASH_START_ADDRESS, 0, 1, &uniform_4K_512Mb_info, "N25Q512", NULL},    /* HR2 is not support N25Q512 */ 
    { {MANUFACTURER_ID_MACRONIX, 0x20, 0x18, 0x0},  {0xFF, 0xFF, 0xFF, 0x0}, &mx25l_funs, 0, CFG_FLASH_START_ADDRESS, 0, 1, &uniform_4K_128Mb_info, "MX25L128", NULL},     
    { {MANUFACTURER_ID_MACRONIX, 0x20, 0x19, 0x0},  {0xFF, 0xFF, 0xFF, 0x0}, &mx25l_funs, 0, CFG_FLASH_START_ADDRESS, 0, 1, &uniform_4K_256Mb_info, "MX25L256", NULL}, 
/*    { {MANUFACTURER_ID_MACRONIX, 0x20, 0x20, 0x0},  {0xFF, 0xFF, 0xFF, 0x0}, &mx25l_funs, 0, CFG_FLASH_START_ADDRESS, 0, 1, &uniform_4K_512Mb_info, "MX25L512", NULL},     */
    { {MANUFACTURER_ID_UNKNOWN,  0x0, 0x0, 0x0}, {0x00, 0x0, 0x0, 0x0}, &mx25l_funs, 0, CFG_FLASH_START_ADDRESS, 0, 1, &uniform_4K_64Mb_info, "Unknown Flash", NULL},
    {{MANUFACTURER_ID_MICRON,0x71,0x15,0x10},{0xff,0xff,0xff,0x0},&mx25l_funs,0,CFG_FLASH_START_ADDRESS,0x1c25f000,1,&uniform_4K_16Mb_info,"M25PX16jfk",NULL}
};


