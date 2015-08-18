/*
 * Copyright (c) 2004,2012 Kustaa Nyholm / SpareTimeLabs
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list 
 * of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice, this 
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *  
 * Neither the name of the Kustaa Nyholm or SpareTimeLabs nor the names of its 
 * contributors may be used to endorse or promote products derived from this software 
 * without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

#include "tfp_printf.h"

typedef void (*putcf)(void*, char, sprintf_state_t*);
static putcf stdout_putf;
static void* stdout_putp;

#ifdef PRINTF_LONG_SUPPORT

static void uli2a(unsigned long int num, unsigned int base, int uc,char * bf)
{
    int n=0;
    unsigned int d=1;
    while (num/d >= base)
    d*=base;
    while (d!=0)
    {
        int dgt = num / d;
        num%=d;
        d/=base;
        if (n || dgt>0|| d==0)
        {
            *bf++ = dgt+(dgt<10 ? '0' : (uc ? 'A' : 'a')-10);
            ++n;
        }
    }
    *bf=0;
}

static void li2a (long num, char * bf)
{
    if (num<0)
    {
        num=-num;
        *bf++ = '-';
    }
    uli2a(num,10,0,bf);
}

#endif

static void ui2a(unsigned int num, unsigned int base, int uc, char * bf)
{
    int n = 0;
    unsigned int d = 1;
    while (num / d >= base)
        d *= base;
    while (d != 0)
    {
        int dgt = num / d;
        num %= d;
        d /= base;
        if (n || dgt > 0 || d == 0)
        {
            *bf++ = dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *bf = 0;
}

static void i2a(int num, char * bf)
{
    if (num < 0)
    {
        num = -num;
        *bf++ = '-';
    }
    ui2a(num, 10, 0, bf);
}

static int a2d(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    else
        return -1;
}

static char a2i(char ch, const char** src, int base, int* nump)
{
    const char* p = *src;
    int num = 0;
    int digit;
    while ((digit = a2d(ch)) >= 0)
    {
        if (digit > base)
            break;
        num = num * base + digit;
        ch = *p++;
    }
    *src = p;
    *nump = num;
    return ch;
}

static void putchw(void* putp, putcf putf, sprintf_state_t* state, int n,
        char z, char* bf)
{
    char fc = z ? '0' : ' ';
    char ch;
    char* p = bf;
    char* pbf = bf;
    while (*p++ && n > 0)
        n--;
    while (n-- > 0)
        putf(putp, fc, state);
    while ((ch = *pbf++))
        putf(putp, ch, state);
}

void tfp_format(void* putp, putcf putf, sprintf_state_t* state, const char *fmt,
        va_list va)
{
    char bf[12];
    const char* cp = fmt;
    char ch;

    while ((ch = *(cp++)))
    {
        if (ch != '%')
            putf(putp, ch, state);
        else
        {
            char lz = 0;
#ifdef 	PRINTF_LONG_SUPPORT
            char lng=0;
#endif
            int w = 0;
            ch = *(cp++);
            if (ch == '0')
            {
                ch = *(cp++);
                lz = 1;
            }
            if (ch >= '0' && ch <= '9')
            {
                ch = a2i(ch, &cp, 10, &w);
            }
#ifdef 	PRINTF_LONG_SUPPORT
            if (ch=='l')
            {
                ch=*(fmt++);
                lng=1;
            }
#endif
            switch (ch)
            {
            case 0:
                goto abort;
            case 'u':
            {
#ifdef 	PRINTF_LONG_SUPPORT
                if (lng)
                uli2a(va_arg(va, unsigned long int),10,0,bf);
                else
#endif
                ui2a(va_arg(va, unsigned int), 10, 0, bf);
                putchw(putp, putf, state, w, lz, bf);
                break;
            }
            case 'd':
            {
#ifdef 	PRINTF_LONG_SUPPORT
                if (lng)
                li2a(va_arg(va, unsigned long int),bf);
                else
#endif
                i2a(va_arg(va, int), bf);
                putchw(putp, putf, state, w, lz, bf);
                break;
            }
            case 'x':
            case 'X':
#ifdef 	PRINTF_LONG_SUPPORT
                if (lng)
                uli2a(va_arg(va, unsigned long int),16,(ch=='X'),bf);
                else
#endif
                ui2a(va_arg(va, unsigned int), 16, (ch == 'X'), bf);
                putchw(putp, putf, state, w, lz, bf);
                break;
            case 'c':
                putf(putp, (char) (va_arg(va, int)), state);
                break;
            case 's':
                putchw(putp, putf, state, w, 0, va_arg(va, char*));
                break;
            case '%':
                putf(putp, ch, state);
                break;
            default:
                break;
            }
        }
    }
    abort: ;
}

void init_printf(void* putp, void (*putf)(void*, char, sprintf_state_t*))
{
    stdout_putf = putf;
    stdout_putp = putp;
}

void tfp_printf(char *fmt, ...)
{
    va_list va;
    sprintf_state_t state =
    { .buf_left = -1, .ctx = NULL }; /* unlimited */
    va_start(va, fmt);

    tfp_format(stdout_putp, stdout_putf, &state, fmt, va);
    va_end(va);
}

/**
 * Output function used for tfp_sprintf/tfp_snprintf
 */
static void putcp(void* p, char c, sprintf_state_t* state)
{
    if ((state->buf_left == -1) || (state->buf_left-- > 0))
    {
        *(*((char**) p))++ = c;
    }
}

void tfp_sprintf(char* s, char *fmt, ...)
{
    va_list va;
    sprintf_state_t state =
    { .buf_left = -1, .ctx = NULL }; /* unlimited */
    va_start(va, fmt);
    tfp_format(&s, putcp, &state, fmt, va);
    putcp(&s, 0, &state);
    va_end(va);
}

void tfp_snprintf(char* s, size_t max_len, char *fmt, ...)
{
    va_list va;
    sprintf_state_t state =
    { .buf_left = max_len, .ctx = NULL };
    va_start(va, fmt);
    tfp_format(&s, putcp, &state, fmt, va);
    putcp(&s, 0, &state);
    va_end(va);
    /* always add null termination at last buffer position */
    s[max_len - 1] = '\0';
}

