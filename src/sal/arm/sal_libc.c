/*
 * $Id: sal_libc.c,v 1.7 Broadcom SDK $
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

int lib_xtoi(const char *dest)
{
    int x = 0;
    int digit;

    if ((*dest == '0') && (*(dest+1) == 'x')) dest += 2;

    while (*dest) {
    if ((*dest >= '0') && (*dest <= '9')) {
        digit = *dest - '0';
        }
    else if ((*dest >= 'A') && (*dest <= 'F')) {
        digit = 10 + *dest - 'A';
        }
    else if ((*dest >= 'a') && (*dest <= 'f')) {
        digit = 10 + *dest - 'a';
        }
    else {
        break;
        }
    x *= 16;
    x += digit;
    dest++;
    }

    return x;
}

int lib_memcmp(const void *dest,const void *src,size_t cnt)
{
    const unsigned char *d;
    const unsigned char *s;

    d = (const unsigned char *) dest;
    s = (const unsigned char *) src;

    while (cnt) {
    if (*d < *s) return -1;
    if (*d > *s) return 1;
    d++; s++; cnt--;
    }

    return 0;
}

void *lib_memcpy(void *dest,const void *src,size_t cnt)
{
    unsigned char *d;
    const unsigned char *s;
    d = (unsigned char *) dest;
    s = (const unsigned char *) src;

    while (cnt) {
    *d++ = *s++;
    cnt--;
    }

    return dest;
}

void *lib_memmove(void *dst, const void *src, size_t n)
{
    register char *dst1 = (char *)dst;

    if (dst > src && dst < src + n) {
        src += n;
        dst1 += n;
        while (n-- > 0) {
            *--dst1 = *(char *)(--src);
        }
    } else {
        while (n-- > 0) {
            *dst1++ = *(char *)(src++);
        }
    }
    return dst;
}

void *lib_memset(void *dest,int c,size_t cnt)
{
    unsigned char *d;

    d = dest;

    while (cnt) {
    *d++ = (unsigned char) c;
    cnt--;
    }

    return d;
}

size_t lib_strlen(const char *str)
{
    size_t cnt = 0;

    while (*str) {
    str++;
    cnt++;
    }

    return cnt;
}

int lib_strcmp(const char *dest,const char *src)
{
    while (*src && *dest) {
    if (*dest < *src) return -1;
    if (*dest > *src) return 1;
    dest++;
    src++;
    }

    if (*dest && !*src) return 1;
    if (!*dest && *src) return -1;
    return 0;
}

int
lib_strncmp(const char *dest, const char *src, size_t cnt)
{
    const unsigned char *d;
    const unsigned char *s;

    d = (const unsigned char *) dest;
    s = (const unsigned char *) src;

    while (cnt) {
    if (*d < *s) return -1;
    if (*d > *s) return 1;
    if (!*d && !*s) return 0;
    d++; s++; cnt--;
    }

    return 0;
}

char
sal_toupper(char c)
{
    if ((c >= 'a') && (c <= 'z'))
        c -= 32;
    return c;
}

int
sal_stricmp(const char *s1, const char *s2)
{
    char dc, sc;

    while(*s2 && *s1) {
        dc = sal_toupper(*s1);
        sc = sal_toupper(*s2);
        if (dc < sc)
            return -1;
        if (dc > sc)
            return 1;
        s1++;
        s2++;
    }

    if (*s1 && !*s2)
        return 1;
    if (!*s1 && *s2)
        return -1;
    return 0;
}

int
sal_strncmp(const char *dest, const char *src, size_t cnt)
{
    const unsigned char *d;
    const unsigned char *s;

    d = (const unsigned char *) dest;
    s = (const unsigned char *) src;

    while (cnt) {
    if (*d < *s) return -1;
    if (*d > *s) return 1;
    if (!*d && !*s) return 0;
    d++; s++; cnt--;
    }

    return 0;
}

size_t
sal_strcspn(const char *s1, const char *s2)
{
    const char *s = s1;
    const char *c;

    while(*s1) {
        for (c = s2; *c; c++) {
            if (*s1 == *c) {
                break;
            }
        }
        if (*c) {
            break;
        }
        s1++;
    }

    return s1 - s;
}

char *lib_strchr(const char *dest,int c)
{
    while (*dest) {
    if (*dest == c) return (char *) dest;
    dest++;
    }
    return NULL;
}

char *lib_strcpy(char *dest,const char *src)
{
    char *ptr = dest;

    while (*src) *ptr++ = *src++;
    *ptr = '\0';

    return dest;
}

char *lib_strcat(char *dest,const char *src)
{
    char *ptr = dest;

    while (*ptr) ptr++;
    while (*src) *ptr++ = *src++;
    *ptr = '\0';

    return dest;
}

int lib_atoi(const char *dest)
{
    int x = 0;
    int digit;

    while (*dest) {
        if ((*dest >= '0') && (*dest <= '9')) {
            digit = *dest - '0';
        } else {
            break;
        }
        x *= 10;
        x += digit;
        dest++;
    }
    return x;
}

char *lib_strncpy(char *dest,const char *src,size_t cnt)
{
    char *ptr = dest;

    while (*src && (cnt > 0)) {
    *ptr++ = *src++;
    cnt--;
    }

    while (cnt > 0) {
    *ptr++ = 0;
    cnt--;
    }

    return dest;
}


char *lib_strnchr(const char *dest,int c,size_t cnt)
{
    while (*dest && (cnt > 0)) {
    if (*dest == c) return (char *) dest;
    dest++;
    cnt--;
    }
    return NULL;
}

char *lib_strrchr(const char *dest,int c)
{
    char *ret = NULL;

    while (*dest) {
    if (*dest == c) ret = (char *) dest;
    dest++;
    }

    return ret;
}

char *
sal_strstr(const char *s1, const char *s2)
{
    if (*s1 == '\0') {
        if (*s2) {
            return (char *) NULL;
        } else {
            return (char *) s1;
        }
    }

    while(*s1) {
        int i;

        for (i=0; ; i++) {
            if (s2[i] == '\0') {
                return (char *) s1;
            }

            if (s2[i] != s1[i]) {
                break;
            }
        }
        s1++;
    }

    return (char *) NULL;
}

/*
 * A simple random number generator without floating pointer operations
 */
STATIC int rand_is_seeded = 0;
STATIC uint32 rand_c_value, rand_t_value;
#define RAND_MAGIC_1 0x0000444BUL
#define RAND_MAGIC_2 0x88740000UL
#define RAND_MAGIC_3 69069UL

void
sal_srand(uint32 seed)
{
    uint32 time_seed = (seed << 21) + (seed << 14) + (seed << 7);
    rand_c_value = ((time_seed + RAND_MAGIC_1) << 1) + 1;
    rand_t_value = (time_seed << (time_seed & 0xF)) + RAND_MAGIC_2;
    rand_is_seeded = 1;
}

uint16
sal_rand(void)
{
    if (!rand_is_seeded) {
        sal_srand((uint32)sal_get_ticks());
    }
    rand_c_value = rand_c_value * RAND_MAGIC_3;
    rand_t_value ^= rand_t_value >> 15;
    rand_t_value ^= rand_t_value << 17;
    return (uint16)(((rand_t_value ^ rand_c_value) >> 1) & 0x7FFF);
}

int32
lib_strtol(const char *nptr, char **endptr, int base)
{
    int x = 0;
    int digit;
    BOOL negative = FALSE;

    while (isspace(*nptr)){
        nptr++;
    }

    if (*nptr == '-') {
        negative = TRUE;
        nptr++;
    }

    if (base == 0) {
        if ((*nptr == '0') && ((*(nptr+1) == 'x') || (*(nptr+1) == 'X'))) {
            base = 16;
            nptr += 2;
        } else {
            base = 10;
        }
    }

    while (*nptr) {
        if ((*nptr >= '0') && (*nptr <= '9')) {
            digit = *nptr - '0';
        } else if ((*nptr >= 'A') && (*nptr <= 'F')) {
            digit = 10 + *nptr - 'A';
        } else if ((*nptr >= 'a') && (*nptr <= 'f')) {
            digit = 10 + *nptr - 'a';
        } else {
            break;
        }

        if (digit >= base) {
            break;
        }
        x *= base;
        x += digit;
        nptr++;
    }

    *endptr = (char *)nptr;

    if (negative) {
        x = -x;
    }
    
    return x;
}

uint32
lib_strtoul(const char *nptr, const char **endptr, int base)
{
    unsigned int x = 0;
    int digit;
    BOOL negative = FALSE;

    while (isspace(*nptr)){
        nptr++;
    }

    if (*nptr == '-') {
        negative = TRUE;
        nptr++;
    }

    if (base == 0) {
        if ((*nptr == '0') && ((*(nptr+1) == 'x') || (*(nptr+1) == 'X'))) {
            base = 16;
            nptr += 2;
        } else {
            base = 10;
        }
    }

    while (*nptr) {
        if ((*nptr >= '0') && (*nptr <= '9')) {
            digit = *nptr - '0';
        } else if ((*nptr >= 'A') && (*nptr <= 'F')) {
            digit = 10 + *nptr - 'A';
        } else if ((*nptr >= 'a') && (*nptr <= 'f')) {
            digit = 10 + *nptr - 'a';
        } else {
            break;
        }

        if (digit >= base) {
            break;
        }
        x *= base;
        x += digit;
        nptr++;
    }

    *endptr = (char *)nptr;

    if (negative) {
        x = -x;
    }
    
    return x;
}

