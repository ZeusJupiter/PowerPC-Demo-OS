/*
 *   File name: local.h
 *
 *  Created on: 2017年6月26日, 上午1:42:03
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_LOCAL_H__
#define __INCLUDE_LOCAL_H__

#include "macros.h"

#ifdef __cplusplus
extern "C" {
#endif

struct lconv {
    char *decimal_point;
    char *thousands_sep;
    char *grouping;
    char *int_curr_symbol;
    char *currency_symbol;
    char *mon_decimal_point;
    char *mon_thousands_sep;
    char *mon_grouping;
    char *positive_sign;
    char *negative_sign;

    char int_frac_digits;
    char frac_digits;

    char p_cs_precedes;
    char p_sep_by_space;
    char n_cs_precedes;
    char n_sep_by_space;
    char p_sign_posn;
    char n_sign_posn;

    char int_p_cs_precedes;
	char int_n_cs_precedes;
	char int_p_sep_by_space;
	char int_n_sep_by_space;
	char int_p_sign_posn;
	char int_n_sign_posn;
};

#define	LC_ALL		0
#define	LC_COLLATE	1
#define	LC_CTYPE	2
#define	LC_MONETARY	3
#define	LC_NUMERIC	4
#define	LC_TIME		5
#define LC_MESSAGES	6

#define	_LC_LAST	7

__DeclareFunc__( struct lconv *, localeconv, (void));
__DeclareFunc__( char *, setlocale, (int category, const char *locale));

#ifdef __cplusplus
}
#endif

#endif

