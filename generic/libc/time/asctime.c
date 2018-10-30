/*
 *   File name: asctime.c
 *
 *  Created on: 2017年6月24日, 下午4:04:18
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include "macros.h"
#include "types.h"
#include "time.h"

char * asctime(const struct tm *timeptr)

{
	static char asctimeBuf[sizeof(ASCBUF)];

	asctime_r(timeptr, asctimeBuf);

	return (asctimeBuf);
}

char* asctime_r(const struct tm *timeptr,

                char * asctimeBuf

               )
{
	size_t size;
	size = strftime(asctimeBuf, sizeof(ASCBUF), "%a %b %d %H:%M:%S %Y\n\0", timeptr);
	if (size == 0L)
		return NULL;

	return (asctimeBuf);
}
