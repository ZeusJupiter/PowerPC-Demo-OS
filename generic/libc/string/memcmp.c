/*
 *   File name: memcmp.c
 *
 *  Created on: 2017年7月21日, 上午1:12:03
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <string.h>

sint memcmp (const void*  cmp1, const void*  cmp2, size_t  size)
{
    register u8*     tmp1 = (u8*)cmp1;
    register u8*     tmp2 = (u8*)cmp2;
    register size_t        i;

	for (i = 0; i < size; i++)
	{
		if (*tmp1 != *tmp2)
		{
			break;
		}
		tmp1++;
		tmp2++;
	}

	if (i >= size)
	{
		return (0);
	}

	if (*tmp1 > *tmp2)
	{
		return (1);
	}
	else
	{
		return (-1);
	}
}
