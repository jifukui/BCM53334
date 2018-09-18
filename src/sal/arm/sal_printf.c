/*
 * $Id: sal_printf.c,v 1.6 Broadcom SDK $
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

extern int um_console_print(const char *str);

static const char digits[17] = "0123456789ABCDEF";
static const char ldigits[17] = "0123456789abcdef";

/*  *********************************************************************
    *  __atox(buf,num,radix,width)
    *
    *  Convert a number to a string
    *
    *  Input Parameters:
    *      buf - where to put characters
    *      num - number to convert
    *      radix - radix to convert number to (usually 10 or 16)
    *      width - width in characters
    *
    *  Return Value:
    *      number of digits placed in output buffer
    ********************************************************************* */
static int __atox(char *buf,unsigned int num,unsigned int radix,int width,
             const char *digits)
{
    char buffer[16];
    char *op;
    int retval;

    op = &buffer[0];
    retval = 0;

    do {
    *op++ = digits[num % radix];
    retval++;
    num /= radix;
    } while (num != 0);

    if (width && (width > retval)) {
    width = width - retval;
    while (width) {
        *op++ = '0';
        retval++;
        width--;
        }
    }

    while (op != buffer) {
    op--;
    *buf++ = *op;
    }

    return retval;
}


/*  *********************************************************************
    *  __llatox(buf,num,radix,width)
    *
    *  Convert a long number to a string
    *
    *  Input Parameters:
    *      buf - where to put characters
    *      num - number to convert
    *      radix - radix to convert number to (usually 10 or 16)
    *      width - width in characters
    *
    *  Return Value:
    *      number of digits placed in output buffer
    ********************************************************************* */
static int __llatox(char *buf,unsigned long long num,unsigned int radix,
            int width,const char *digits)
{
    char buffer[16];
    char *op;
    int retval;

    op = &buffer[0];
    retval = 0;

#if CPUCFG_REGS32
    /*
     * To avoid pulling in the helper library that isn't necessarily
     * compatible with PIC code, force radix to 16, use shifts and masks
     */
    do {
    *op++ = digits[num & 0x0F];
    retval++;
    num >>= 4;
    } while (num != 0);
#else
    do {
    *op++ = digits[num % radix];
    retval++;
    num /= radix;
    } while (num != 0);
#endif

    if (width && (width > retval)) {
    width = width - retval;
    while (width) {
        *op++ = '0';
        retval++;
        width--;
        }
    }

    while (op != buffer) {
    op--;
    *buf++ = *op;
    }

    return retval;
}

/*  *********************************************************************
    *  xvsprintf(outbuf,template,arglist)
    *
    *  Format a string into the output buffer
    *
    *  Input Parameters:
    *      outbuf - output buffer
    *      template - template string
    *      arglist - parameters
    *
    *  Return Value:
    *      number of characters copied
    ********************************************************************* */
int vsprintf(char *outbuf,const char *templat,va_list marker)
{
    char *optr;
    const char *iptr;
    unsigned char *tmpptr;
    long x;
    unsigned long ux;
    unsigned long long ulx;
    int i;
    long long ll;
#if 0
    int leadingzero;
    int leadingnegsign;
#endif
    int islong;
    int width;
    int width2 = 0;

    optr = outbuf;
    iptr = templat;

    while (*iptr) {
    if (*iptr != '%') {*optr++ = *iptr++; continue;}

    iptr++;

    if (*iptr == '#') { iptr++; }
#if 0
    if (*iptr == '-') {
        leadingnegsign = 1;
        iptr++;
        }
    else leadingnegsign = 0;

    if (*iptr == '0') leadingzero = 1;
    else leadingzero = 0;
#endif
    width = 0;
    while (*iptr && isdigit(*iptr)) {
        width += (*iptr - '0');
        iptr++;
        if (isdigit(*iptr)) width *= 10;
        }
    if (*iptr == '.') {
        iptr++;
        width2 = 0;
        while (*iptr && isdigit(*iptr)) {
        width2 += (*iptr - '0');
        iptr++;
        if (isdigit(*iptr)) width2 *= 10;
        }
        }

    islong = 0;
    if (*iptr == 'l') { islong++; iptr++; }
    if (*iptr == 'l') { islong++; iptr++; }

    switch (*iptr) {
        case 'I':
        tmpptr = (unsigned char *) va_arg(marker,unsigned char *);
        optr += __atox(optr,*tmpptr++,10,0,digits);
        *optr++ = '.';
        optr += __atox(optr,*tmpptr++,10,0,digits);
        *optr++ = '.';
        optr += __atox(optr,*tmpptr++,10,0,digits);
        *optr++ = '.';
        optr += __atox(optr,*tmpptr++,10,0,digits);
        break;
        case 's':
        tmpptr = (unsigned char *) va_arg(marker,unsigned char *);
        if (!tmpptr) tmpptr = (unsigned char *) "(null)";
        if ((width == 0) & (width2 == 0)) {
            while (*tmpptr) *optr++ = *tmpptr++;
            break;
            }
        while (width && *tmpptr) {
            *optr++ = *tmpptr++;
            width--;
            }
        while (width) {
            *optr++ = ' ';
            width--;
            }
        break;
        case 'a':
        tmpptr = (unsigned char *) va_arg(marker,unsigned char *);
        for (x = 0; x < 5; x++) {
            optr += __atox(optr,*tmpptr++,16,2,digits);
            *optr++ = '-';
            }
        optr += __atox(optr,*tmpptr++,16,2,digits);
        break;
        case 'd':
        switch (islong) {
            case 0:
            case 1:
            i = va_arg(marker,int);
            if (i < 0) { *optr++='-'; i = -i;}
            optr += __atox(optr,i,10,width,digits);
            break;
            case 2:
            ll = va_arg(marker,long long int);
            if (ll < 0) { *optr++='-'; ll = -ll;}
            optr += __llatox(optr,ll,10,width,digits);
            break;
            }
        break;
        case 'u':
        switch (islong) {
            case 0:
            case 1:
            ux = va_arg(marker,unsigned int);
            optr += __atox(optr,ux,10,width,digits);
            break;
            case 2:
            ulx = va_arg(marker,unsigned long long);
            optr += __llatox(optr,ulx,10,width,digits);
            break;
            }
        break;
        case 'X':
        case 'x':
        switch (islong) {
            case 0:
            case 1:
            ux = va_arg(marker,unsigned int);
            optr += __atox(optr,ux,16,width,
                       (*iptr == 'X') ? digits : ldigits);
            break;
            case 2:
            ulx = va_arg(marker,unsigned long long);
            optr += __llatox(optr,ulx,16,width,
                       (*iptr == 'X') ? digits : ldigits);
            break;
            }
        break;
        case 'p':
        case 'P':
#ifdef __long64
        lx = va_arg(marker,long long);
        optr += __llatox(optr,lx,16,16,
                 (*iptr == 'P') ? digits : ldigits);
#else
        x = va_arg(marker,long);
        optr += __atox(optr,x,16,8,
                   (*iptr == 'P') ? digits : ldigits);
#endif
        break;
        case 'w':
        x = va_arg(marker,unsigned int);
            x &= 0x0000FFFF;
        optr += __atox(optr,x,16,4,digits);
        break;
        case 'b':
        x = va_arg(marker,unsigned int);
            x &= 0x0000FF;
        optr += __atox(optr,x,16,2,digits);
        iptr++;
        break;
        case 'Z':
        x = va_arg(marker,unsigned int);
        tmpptr = va_arg(marker,unsigned char *);
        while (x) {
            optr += __atox(optr,*tmpptr++,16,2,digits);
            x--;
            }
        break;
        case 'c':
        x = va_arg(marker, int);
        *optr++ = x & 0xff;
        break;

        default:
        *optr++ = *iptr;
        break;
        }
    iptr++;
    }

    *optr = '\0';

    return (optr - outbuf);
}


/*  *********************************************************************
    *  xsprintf(buf,template,params..)
    *
    *  format messages from template into a buffer.
    *
    *  Input Parameters:
    *      buf - output buffer
    *      template - template string
    *      params... parameters
    *
    *  Return Value:
    *      number of bytes copied to buffer
    ********************************************************************* */
int sprintf(char *buf,const char *templat,...)
{
    va_list marker;
    int count;

    va_start(marker,templat);
    count = vsprintf(buf,templat,marker);
    va_end(marker);

    return count;
}
