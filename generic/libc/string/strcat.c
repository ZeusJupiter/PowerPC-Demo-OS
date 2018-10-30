/*
 *   File name: strcat.c
 *
 *  Created on: 2017年7月21日, 上午1:25:27
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <string.h>

char*  strcat (char*  to, const char*  from)
{
    register char* tmp = to;

	while (*tmp != EOS)
	{
		tmp++;
	}

	while (*from != EOS)
	{
		*tmp++ = *from++;
	}

    *tmp = EOS;

    return  (to);
}
