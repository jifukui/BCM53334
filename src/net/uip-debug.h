/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * $Id: uip-debug.h,v 1.4 Broadcom SDK $
 */
/**
 * \file
 *         A set of debugging macros.
 *
 * \author Nicolas Tsiftes <nvt@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#ifndef UIP_DEBUG_H
#define UIP_DEBUG_H

#include "uip.h"


#define DEBUG_NONE              0
#define DEBUG_PRINT             1
#define DEBUG_ANNOTATE          2
#define DEBUG_FULL              DEBUG_ANNOTATE | DEBUG_PRINT

#if (DEBUG) & DEBUG_ANNOTATE
#define ANNOTATE(format, arg)   sal_printf(format, arg)
#else
#define ANNOTATE(format, arg)   do { } while(0)
#endif /* (DEBUG) & DEBUG_ANNOTATE */

#if (DEBUG) & DEBUG_PRINT
#define PRINTF(x)               do { sal_printf x; } while(0)
#define PRINT6ADDR(addr)        do {                            \
    sal_printf(" %X:%X:%X:%X:%X:%X:%X:%X ",                     \
        (int)((u16_t *)(addr))[0], (int)((u16_t *)(addr))[1],   \
        (int)((u16_t *)(addr))[2], (int)((u16_t *)(addr))[3],   \
        (int)((u16_t *)(addr))[4], (int)((u16_t *)(addr))[5],   \
        (int)((u16_t *)(addr))[6], (int)((u16_t *)(addr))[7]);  \
    } while(0)
#define PRINTLLADDR(lladdr)     do {                            \
    sal_printf(                                                 \
        " %02X:%02X:%02X:%02X:%02X:%02X ",                      \
            (int)(lladdr)->addr[0], (int)(lladdr)->addr[1],     \
            (int)(lladdr)->addr[2], (int)(lladdr)->addr[3],     \
            (int)(lladdr)->addr[4], (int)(lladdr)->addr[5]);    \
    } while(0)
#else /* (DEBUG) & DEBUG_PRINT */
#define PRINTF(x)               do { } while(0)
#define PRINT6ADDR(addr)        do { } while(0)
#define PRINTLLADDR(lladdr)     do { } while(0)
#endif /* (DEBUG) & DEBUG_PRINT */

#endif /* UIP_DEBUG_H*/
