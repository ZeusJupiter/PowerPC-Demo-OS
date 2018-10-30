/*
 *   File name: skipspace.c
 *
 *  Created on: 2017年7月26日, 下午5:00:37
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <ctype.h>

char* UNUSED skip_spaces(const char *str)
{
    while (isspace(*str))
        ++str;
    return (char *)str;
}

