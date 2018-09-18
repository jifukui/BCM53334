/*
 * $Id: um_xip_pic.lds,v 1.3 Broadcom SDK $
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
MEMORY {
       ATCM : ORIGIN = 0x00000000, LENGTH = 64K
       BTCM : ORIGIN = 0x00040000, LENGTH = 128K
}

SECTIONS 
{
/* Could be ROM or FLASH start address */
. = reset_addr;

.text reset_addr : {
        text_start = .;
        low_mem*.o(.text);
        low_mem*.o(.rodata);
        *(.text*);
        *(i.*);
        *(i.__ARM_common*);
        *(.rodata*);
        *(.constdata);
        *(.conststring);
        *(.flashdata);
        *phypkgsrc.a:bcm84846_ucode.o(.data*)
        *phypkgsrc.a:bcmi_tsc_xgxs_ucode.o(.data*)
        . = ALIGN(4);
}

.ARM.exidx . : {
   *(.ARM.exidx*);
}


text_end = .;
/*
 * - 2ram, data and bss
 */
. = 0x00040000;

.data . : AT ( text_end ) {
        data_start = .;
        *(.2ram*);
        . = ALIGN(4);
        data_rel_start = .;
        *(.data.rel*);
        . = ALIGN(4);
        data_rel_end = .;
        . = ALIGN(32);
        *(EXCLUDE_FILE (*phypkgsrc.a:bcm84846_ucode.o *phypkgsrc.a:bcmi_tsc_xgxs_ucode.o) .data.*);
        data_rel2_start = .;
        *(EXCLUDE_FILE (*phypkgsrc.a:bcm84846_ucode.o *phypkgsrc.a:bcmi_tsc_xgxs_ucode.o) .data*);
        data_rel2_end = .;
        . = ALIGN(4);
        data_end = .;
} > BTCM 

.rel.dyn : {
  __rel_dyn_start = .;
  *(.rel*)
  __rel_dyn_end = .;
 }  > BTCM 
 .dynsym : {
  __dynsym_start = .;
  *(.dynsym)
 } > BTCM 

.bss . : {
       . = ALIGN(4);
        bss_start = .;
       *(.bss*)
       *(COMMON)
       . = ALIGN(4);
       end = .;
       bss_end = .;


} > BTCM 

.heap (NOLOAD) : {
       . = ALIGN(32);
       low_mem*.o(mem_heap);
       . = ALIGN(32);
       dtcm_endaddr = .;
} > BTCM

.packet_buf (NOLOAD) : {
   . = . + 0x40; 
   itcm_start = . ;
   . = ALIGN(32);
   packetbuf_baseaddr = .;
   *(.packet_buf)
   . = ALIGN(32);
   low_mem*.o(mem_stack);
   . = ALIGN(32);
   low_mem*.o(mem_dmaheap);
   . = ALIGN(32);
   itcm_end = .;
} > ATCM 

 

}
