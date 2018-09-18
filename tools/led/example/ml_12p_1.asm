;
; $Id: ml_12p_1.asm,v 1.1.2.1 Broadcom SDK $
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
; This is the default program for the (12 port) BCM956270K
; reference board.
;
; There are 4 bits per fast Ethernet LED with the following colors:
;       ZERO, ZERO, ZERO, ZERO      Black on left and right
;       ONE,  ONE,  ZERO, ZERO      Green on both left and right
;       ONE,  ZERO, ZERO, ZERO      Green on left
;       ZERO, ONE,  ZERO, ZERO      Green on right 
;

;
; Link up/down info cannot be derived from LINKEN or LINKUP, as the LED
; processor does not always have access to link status.  This program
; assumes "link status" and "loop status" are kept in bit 0 and 1 of RAM byte 
; (0xa0 + portnum) separately.
; Generally, a program running on the main CPU must update these
; locations on link change;
;
; Current implementation:
;
;       Black on left and right: no link
;       Green on left: link up
;       Alternating green on right: both RX and TX
;       Alternating green on both left and right: Loop detected.
;
;

TICKS           EQU     1
SECOND_TICKS    EQU     (30*TICKS)

MIN_PORT     EQU     1
MAX_PORT     EQU     12 

NUM_PORT        EQU     12 

; The TX/RX activity lights will be extended for ACT_EXT_TICKS
; so they will be more visible.
ACT_EXT_TICKS   EQU     (SECOND_TICKS/20)

TXRX_ALT_TICKS  EQU     (SECOND_TICKS/8)

;
; Main Update Routine
;
;  This routine is called once per tick.
;

update:
        ld      a, MIN_PORT

up1:
        port    a
        ld      (PORT_NUM),a

        call    get_loop
        jnc     up4             ; no loop, check tx/rx
        call    loop_detect
        jmp     up3        

up4:        
        call    activity       

up3:        
        ld      a,(PORT_NUM)
        inc     a
        cmp     a, MAX_PORT+1
        jnz     up1

        ; Update various timers

        ld      b,TXRX_ALT_COUNT
        inc     (b)
        ld      a,(b)
        cmp     a,TXRX_ALT_TICKS
        jc      up2
        ld      (b),0
up2:

        send    48 ; 4 * 12

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
	    call   get_link
	    ;pushst  LINKUP
	    ;pop 
	    jnc     led_nolink             ; no link, all black
        jmp     led_link_noact         ; link up, left keep green

act2:                           
        ld      b,(TXRX_ALT_COUNT)
        cmp     b,TXRX_ALT_TICKS/2
        jc      led_link_noact        ; Fast alternation of green on right
        jmp     led_link_green        ; keep green on left


;
; get_link
;
;  This routine finds the link status LED for a port.
;  Link info is in bit 0 of the byte read from PORTDATA[port]
;  Inputs: (PORT_NUM)
;
;


get_link:
        ld      b,PORTDATA
        add     b,a
        ld      b,(b)
        tst     b,0
        ret

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
        jc      led_loop_black
        jmp     led_loop_green       ; Fast alternation of green on left and right
        
get_loop:        
        ld      b,PORTDATA
        add     b,a
        ld      b,(b)
        tst     b,1
        ret
;
; led_loop_black, led_loop_green, led_link_noact, led_link_green, led_nolink
;
;  Inputs: None
;  Outputs: Four bits to the LED stream indicating color
;  Destroys: None
;
led_loop_black:
        pushst  ZERO
        pack
        pushst  ZERO
        pack
        pushst  ZERO
        pack
        pushst  ZERO
        pack
        ret;
led_loop_green:
        pushst  ONE
        pack
        pushst  ONE
        pack
        pushst  ZERO
        pack
        pushst  ZERO
        pack
        ret;
led_link_noact:
        pushst  ONE
        pack
        pushst  ZERO
        pack
        pushst  ZERO
        pack
        pushst  ZERO
        pack
        ret;
led_link_green:
        pushst  ONE
        pack
        pushst  ONE
        pack
        pushst  ZERO
        pack
        pushst  ZERO
        pack
        ret;
led_nolink:
        pushst  ZERO
        pack
        pushst  ZERO
        pack
        pushst  ZERO
        pack
        pushst  ZERO
        pack
        ret;        
;
; Variables (UM software initializes LED memory from 0xa0-0xff to 0)
;

TXRX_ALT_COUNT  equ     0xe0
HD_COUNT        equ     0xe1
GIG_ALT_COUNT   equ     0xe2
PORT_NUM        equ     0xe3

;
; Port data, which must be updated continually by main CPU's
; linkscan task.  
; In this program, bit 0 and bit 1 are assumed to contain the link up/down status 
; and loop detected seperately.
;

PORTDATA        equ     0xa0    

;
; LED extension timers
;

TXRX_TIMERS      equ     0xe4         

;
; Symbolic names for the bits of the port status fields
;

RX              equ     0x0     ; received packet
TX              equ     0x1     ; transmitted packet
COLL            equ     0x2     ; collision indicator
SPEED_C         equ     0x3     ; 100 Mbps
SPEED_M         equ     0x4     ; 1000 Mbps
DUPLEX          equ     0x5     ; half/full duplex
FLOW            equ     0x6     ; flow control capable
LINKUP          equ     0x7     ; link down/up status
LINKEN          equ     0x8     ; link disabled/enabled status
ZERO            equ     0xE     ; always 0
ONE             equ     0xF     ; always 1
