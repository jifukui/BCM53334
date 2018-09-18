;
; $Id: hr3_embedded_16x1g_2_B0.asm,v 1.2.2.1 Broadcom SDK $
;
; $Copyright: Copyright 2016 Broadcom Corporation.
; This program is the proprietary software of Broadcom Corporation
; and/or its licensors, and may only be used, duplicated, modified
; or distributed pursuant to the terms and conditions of a separate,
; written license agreement executed between you and Broadcom
; (an "Authorized License").  Except as set forth in an Authorized
; License, Broadcom grants no license (express or implied), right
; to use, or waiver of any kind with respect to the Software, and
; Broadcom expressly reserves all rights in and to the Software
; and all intellectual property rights therein.  IF YOU HAVE
; NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
; IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
; ALL USE OF THE SOFTWARE.  
;  
; Except as expressly set forth in the Authorized License,
;  
; 1.     This program, including its structure, sequence and organization,
; constitutes the valuable trade secrets of Broadcom, and you shall use
; all reasonable efforts to protect the confidentiality thereof,
; and to use this information only in connection with your use of
; Broadcom integrated circuit products.
;  
; 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
; PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
; REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
; OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
; DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
; NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
; ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
; CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
; OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
; 
; 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
; BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
; INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
; ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
; TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
; POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
; THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
; WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
; ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
;
;
; This is the default program for Foxhound2 embedded gphy:
;       Right LED : Link,   Left LED : TX/RX activity  
;
; Use on-chip serial-to-parallel circuit
; Assume 2 LED per port. 4 bits per port. 
;       LED Left  = (b'1 b'0 b'0 b'0) 
;       LED Right = (b'0 b'1 b'0 b'0)
;       LED Right Left = (b'1 b'1 b'0 b'0)
;
; No mattter how much port used, on-chip serial-to-parallel need 96 bits output

;
; Constants
;
NUM_PORTS     equ 16
START_PORT_0  equ 10
END_PORT_0    equ 17
START_PORT_1  equ 26
END_PORT_1    equ 33

TICKS           EQU     1
SECOND_TICKS    EQU     (30*TICKS)
TXRX_ALT_TICKS  EQU     (SECOND_TICKS/6)
; The TX/RX activity lights will be extended for ACT_EXT_TICKS
; so they will be more visible.
ACT_EXT_TICKS   EQU     (SECOND_TICKS/3)

;
; LED process
;
start:
    ld  b, START_PORT_0
    ld  (SEC_MAX), b    
       
iter_sec0:
    ld  b, (SEC_MAX)
    cmp b, END_PORT_1
    jz  end

    cmp b, END_PORT_0
    jz  iter_sec1

    ld  a, START_PORT_0
    ld  b, END_PORT_0
    ld  (SEC_MAX), b    
    jmp iter_common
    
iter_sec1:
    ld  a, START_PORT_1
    ld  b, END_PORT_1
    ld  (SEC_MAX), b    
    jmp iter_common

iter_common:
    port    a
    ld  (PORT_NUM), a
    
        call    get_loop
        jnc     up4             ; no loop, check tx/rx
        call    loop_detect
        jmp     up3

up4:        
        call    activity       

up3:        
    ld  a, (PORT_NUM)
    ld  b, (SEC_MAX)
    cmp a, b
    ; a = b
    jz  iter_sec0 
    ; a > b then up_dec
    jnc up_dec    
    

up_add:
    inc a
    jmp iter_common    

up_dec:
    dec a
    jmp iter_common 
        
    
end:
    ; Update various timers
    ld      b,TXRX_ALT_COUNT
    inc     (b)
    ld      a,(b)
    cmp     a,TXRX_ALT_TICKS
    jc      end1
    ld      (b),0
    
end1:
	 send    96

;
; activity
;
;  This routine calculates the activity LED for the current port.
;  It extends the activity lights using timers (new activity overrides
;  and resets the timers).
;
;  Inputs: (PORT_NUM)
;  Outputs: Four bits sent to LED stream
;

activity:
        pushst  RX
        pushst  TX
        tor
        pop
        jnc     act1

        ld      b,TXRX_TIMERS     ; Start TXRX LED extension timer
        add     b,(PORT_NUM)
        ld      a,ACT_EXT_TICKS
        ld      (b),a

act1:
        ld      b,TXRX_TIMERS     ; Check TXRX LED extension timer
        add     b,(PORT_NUM)

        dec     (b)
        jnc     act2            ; TXRX active?
        inc     (b)

        ld     a,(PORT_NUM)
        jmp    get_link

act2:                           
        ld      b,(TXRX_ALT_COUNT)
        cmp     b,TXRX_ALT_TICKS/2
        jnc     led_link_noact        ; Fast alternation of green on right
        jmp     led_green        ; keep green on left
    
;
; get_link
;
get_link:
    pushst  LINKEN
    pop

    jnc     led_black             ; no link, all black
    jmp     led_link_noact         ; link up, left keep green


; loop_detect
;
loop_detect:
        
        ld      b,PORTDATA
        add     b,(PORT_NUM)
        ld      b,(b)
        tst     b,1
        jnc     loop1
        
        ld      b,TXRX_TIMERS     ; Start TXRX LED extension timer
        add     b,(PORT_NUM)
        ld      a,ACT_EXT_TICKS
        ld      (b),a

loop1:
        ld      b,TXRX_TIMERS     ; Check TXRX LED extension timer
        add     b,(PORT_NUM)

        dec     (b)
        
        jnc     loop2            ; TXRX active?
        inc     (b)

        ret

loop2:                           ; Both TX and RX active
        ld      b,(TXRX_ALT_COUNT)
        cmp     b,TXRX_ALT_TICKS/2
        jnc     led_green       ; Fast alternation of green on left and right
        jmp      led_black
        

get_loop:        
        ld      b,PORTDATA
        add     b,(PORT_NUM)
        ld      b,(b)
        tst     b,1
        ret
     
;
; led_green
;
;  Outputs: Bits to the LED stream indicating ON
;
led_green:
    push 1
    pack
    push 1
    pack
    push 0
    pack
    push 0 
    pack
    ret

;
; led_black
;
;  Outputs: Bits to the LED stream indicating OFF
;
led_black:
    push 0
    pack
    push 0
    pack
    push 0
    pack
    push 0
    pack
    ret

led_link_noact:
    push  0
    pack
    push  1
    pack
    push  0
    pack
    push  0
    pack
    ret

; Variables (UM software initializes LED memory from 0xA0-0xff to 0)
PORTDATA    equ 0xA0
PORT_NUM    equ 0xD0
SEC_MAX equ 0xD1
;
; LED extension timers
;
TXRX_ALT_COUNT  equ     0xD2
TXRX_TIMERS      equ     0xD4         

; Symbolic names for the bits of the port status fields
RX      equ 0x0 ; received packet
TX      equ 0x1 ; transmitted packet
COLL    equ 0x2 ; collision indicator
SPEED_C equ 0x3 ; 100 Mbps
SPEED_M equ 0x4 ; 1000 Mbps
DUPLEX  equ 0x5 ; half/full duplex
FLOW    equ 0x6 ; flow control capable
LINKUP  equ 0x7 ; link down/up status
LINKEN  equ 0x8 ; link disabled/enabled status
ZERO    equ 0xE ; always 0
ONE     equ 0xF ; always 1
