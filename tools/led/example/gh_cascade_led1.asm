;
; $Id: gh_cascade_led1.asm,v 1.2.6.1 Broadcom SDK $
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
; This is the default program for Greyhound SKUs:
;       BCM53456 Option1
;
; Assume 2 LED per port. And 2 bits stream per LED.
;       Link-Down = Off (b'00 b'00)
;       Link-Up = On (b'01 b'00)
;       traffic = (b'01 b'01) (b'01 b'00)  switched
;       loop    = (b'01 b'01) (b'00 b'00)  switched
;
; Totally 28 ports/56 LEDs will be outputed.
;
; Link up/down info cannot be derived from LINKEN or LINKUP, as the LED
; processor does not always have access to link status.  This program
; assumes link status is kept current in bit 0 of RAM byte (0xA0 + portnum).
; Generally, a program running on the main CPU must update these
; locations on link change; 
;

;
; Constants
;
NUM_PORTS   equ 28
MIN_PORT_0 equ 2
MAX_PORT_0 equ 26
MIN_PORT_1 equ 28
MAX_PORT_1 equ 28
MIN_PORT_2 equ 30
MAX_PORT_2 equ 30
MIN_PORT_3 equ 32
MAX_PORT_3 equ 32

TICKS           EQU     1
SECOND_TICKS    EQU     (30*TICKS)
TXRX_ALT_TICKS  EQU     (SECOND_TICKS/8)
; The TX/RX activity lights will be extended for ACT_EXT_TICKS
; so they will be more visible.
ACT_EXT_TICKS   EQU     (SECOND_TICKS/20)

;
; LED process
;
start:
    ld  a, MIN_PORT_0

iter_sec0:
    cmp a, MAX_PORT_0+1
    jnc  iter_sec1

    cmp a, MIN_PORT_0
    jz  iter_sec0_init
    jc  iter_sec0_init
    jmp iter_sec0_start
iter_sec0_init:
    ld  a, MIN_PORT_0
    ld  b, MAX_PORT_0
    ld  (SEC_MAX), b
iter_sec0_start:
    jmp iter_common

iter_sec1:
    cmp a, MAX_PORT_1+1
    jnc  iter_sec2

    cmp a, MIN_PORT_1
    jz  iter_sec1_init
    jc  iter_sec1_init
    jmp iter_sec1_start
iter_sec1_init:
    ld  a, MIN_PORT_1
    ld  b, MAX_PORT_1
    ld  (SEC_MAX), b
iter_sec1_start:
    jmp iter_common

iter_sec2:
    cmp a, MAX_PORT_2+1
    jnc  iter_sec3

    cmp a, MIN_PORT_2
    jz  iter_sec2_init
    jc  iter_sec2_init
    jmp iter_sec2_start
iter_sec2_init:
    ld  a, MIN_PORT_2
    ld  b, MAX_PORT_2
    ld  (SEC_MAX), b
iter_sec2_start:
    jmp iter_common

iter_sec3:
    cmp a, MAX_PORT_3+1
    jnc  end

    cmp a, MIN_PORT_3
    jz  iter_sec3_init
    jc  iter_sec3_init
    jmp iter_sec3_start
iter_sec3_init:
    ld  a, MIN_PORT_3
    ld  b, MAX_PORT_3
    ld  (SEC_MAX), b
iter_sec3_start:
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
    inc a
    ld b, (SEC_MAX)
    inc b
    cmp a, b
    jz iter_sec0
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
	send    4*NUM_PORTS

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
	    jmp   get_link

act2:                           
        ld      b,(TXRX_ALT_COUNT)
        cmp     b,TXRX_ALT_TICKS/2
        jnc     led_link_noact        ; Fast alternation of green on right
        jmp     led_green        ; keep green on left
    
;
; get_link
;
;  This routine finds the link status LED for a port.
;  Link info is in bit 0 of the byte read from PORTDATA[port]
;  Inputs: (PORT_NUM)
;  Outputs: Carry flag set if link is up, clear if link is down.
;  Destroys: a, b
get_link:
    ld  b, PORTDATA
    add b, (PORT_NUM)
    ld  a, (b)
    tst a, 0
    
    jnc     led_black             ; no link, all black
    jmp     led_link_noact         ; link up, left keep green


; loop_detect
;
;  This routine finds the link status LED for a port.
;  Link info is in bit 1 of the byte read from PORTDATA[port]
;
;  Inputs: (PORT_NUM)
;        
loop_detect:
        ld      b,PORTDATA
        add     b,a
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
        add     b,a
        ld      b,(b)
        tst     b,1
        ret
     
;
; led_green
;
;  Outputs: Bits to the LED stream indicating ON
;
led_green:
    push 0
    pack
    push 1
    pack
    push 0
    pack
    push 1
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
