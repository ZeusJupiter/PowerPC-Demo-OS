/*
 *   File name: strim.c
 *
 *  Created on: 2017年7月26日, 下午5:00:27
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <ctype.h>
#include <string.h>
#include <linux/port.h>

char* strim(char *s)
{
    size_t size;
    char *end;

    s = skip_spaces(s);
    size = strlen(s);
    if (!size)
        return s;

    end = s + size - 1;
    while (end >= s && isspace(*end))
        --end;
    *(end + 1) = EOS;

    return s;
}

