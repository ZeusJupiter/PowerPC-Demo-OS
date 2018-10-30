/*
 *   File name: strtoumax.c
 *
 *  Created on: 2017年7月14日, 下午5:14:16
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <inttypes.h>
#include <ctype.h>
#include <errno.h>

uintmax_t strtoumax (const char *nptr, char **endptr, int base)
{
        const char *s;
        uintmax_t acc, cutoff;
        int c;
        int neg, any, cutlim;

        s = nptr;
        do {
                c = (unsigned char) *s++;
        } while (isspace(c));
        if (c == '-') {
                neg = 1;
                c = *s++;
        } else {
                neg = 0;
                if (c == '+')
                        c = *s++;
        }
        if ((base == 0 || base == 16) &&
            c == '0' && (*s == 'x' || *s == 'X')) {
                c = s[1];
                s += 2;
                base = 16;
        }
        if (base == 0)
                base = c == '0' ? 8 : 10;

#define  CASE_BASE(x)                            \
            case x: cutoff = UINTMAX_MAX / x;    \
                    cutlim = UINTMAX_MAX % x;    \
                    break

        switch (base) {
        CASE_BASE(8);
        CASE_BASE(10);
        CASE_BASE(16);
        default:
            cutoff = UINTMAX_MAX / base;
            cutlim = UINTMAX_MAX % base;
        }

        for (acc = 0, any = 0;; c = (unsigned char) *s++) {
                if (isdigit(c))
                        c -= '0';
                else if (isalpha(c))
                        c -= isupper(c) ? 'A' - 10 : 'a' - 10;
                else
                        break;
                if (c >= base)
                        break;
                if (any < 0)
                        continue;
                if (acc > cutoff || (acc == cutoff && c > cutlim)) {
                        any = -1;
                        acc = UINTMAX_MAX;
                        errno = ERANGE;
                } else {
                        any = 1;
                        acc *= (uintmax_t)base;
                        acc += c;
                }
        }
        if (neg && any > 0)
                acc = -acc;
        if (endptr != 0)
                *endptr = (char *) (any ? s - 1 : nptr);
        return (acc);
}

