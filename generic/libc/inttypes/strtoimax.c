/*
 *   File name: strtoimax.c
 *
 *  Created on: 2017年7月14日, 下午5:09:37
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

intmax_t  strtoimax(const char * RESTRICT nptr, char **RESTRICT endptr, int base)
{
        const char *s;
        intmax_t acc, cutoff;
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

#define  CASE_BASE(x) \
            case x:  \
                if (neg) { \
                    cutlim = INTMAX_MIN % x; \
                    cutoff = INTMAX_MIN / x; \
                } else { \
                    cutlim = INTMAX_MAX % x; \
                    cutoff = INTMAX_MAX / x; \
                 }; \
                 break

        switch (base) {
            case 4:
                if (neg) {
                    cutlim = (int)(INTMAX_MIN % 4);
                    cutoff = INTMAX_MIN / 4;
                } else {
                    cutlim = (int)(INTMAX_MAX % 4);
                    cutoff = INTMAX_MAX / 4;
                }
                break;

            CASE_BASE(8);
            CASE_BASE(10);
            CASE_BASE(16);
            default:
                      cutoff  = neg ? INTMAX_MIN : INTMAX_MAX;
                      cutlim  = cutoff % base;
                      cutoff /= base;
        }
#undef CASE_BASE

        if (neg) {
                if (cutlim > 0) {
                        cutlim -= base;
                        cutoff += 1;
                }
                cutlim = -cutlim;
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
                if (neg) {
                        if (acc < cutoff || (acc == cutoff && c > cutlim)) {
                                any = -1;
                                acc = INTMAX_MIN;
                                errno = ERANGE;
                        } else {
                                any = 1;
                                acc *= base;
                                acc -= c;
                        }
                } else {
                        if (acc > cutoff || (acc == cutoff && c > cutlim)) {
                                any = -1;
                                acc = INTMAX_MAX;
                                errno = ERANGE;
                        } else {
                                any = 1;
                                acc *= base;
                                acc += c;
                        }
                }
        }
        if (endptr != 0)
                *endptr = (char *) (any ? s - 1 : nptr);
        return (acc);
}

