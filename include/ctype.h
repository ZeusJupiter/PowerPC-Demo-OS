/*
 *   File name: ctype.h
 *
 *  Created on: 2017年6月26日, 上午1:44:23
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_CTYPE_H__
#define __INCLUDE_CTYPE_H__

enum {
	CT_NUMERIC	= 1,
	CT_LOWER	= 2,
	CT_UPPER	= 4,
	CT_SPACE	= 8,
	CT_HEX		= 16,
	CT_PUNCT	= 32,
	CT_CTRL		= 64,
	CT_BLANK	= 128,
};

#ifdef __cplusplus
extern "C" {
#endif

__DeclareFunc__(int, isalnum, (int c));
__DeclareFunc__(int, isalpha, (int c));
__DeclareFunc__(int, iscntrl, (int c));
__DeclareFunc__(int, isdigit, (int c));
__DeclareFunc__(int, isgraph, (int c));
__DeclareFunc__(int, islower, (int c));
__DeclareFunc__(int, isprint, (int c));
__DeclareFunc__(int, ispunct, (int c));
__DeclareFunc__(int, isspace, (int c));
__DeclareFunc__(int, isupper, (int c));
__DeclareFunc__(int, isxdigit, (int c));
__DeclareFunc__(int, isblank, (int c));
__DeclareFunc__(int, toupper, (int c));
__DeclareFunc__(int, tolower, (int c));
__DeclareFunc__(int, isascii, (int c));
__DeclareFunc__(int, toascii, (int c));

#define _tolower(__c) ((unsigned char)(__c) - 'A' + 'a')
#define _toupper(__c) ((unsigned char)(__c) - 'a' + 'A')

#ifdef __cplusplus
}
#endif

#endif

