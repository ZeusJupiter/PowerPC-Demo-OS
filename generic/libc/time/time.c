/*
 *   File name: time.c
 *
 *  Created on: 2017年6月24日, 下午3:41:32
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <time.h>

time_t time(time_t *t)
{
	struct timespec tp;

	if (clock_gettime(CLOCK_REALTIME, &tp) == 0)
	{
		if (t != NULL)
			*t = (time_t) tp.tv_sec;
		return (time_t) (tp.tv_sec);
	}
	else
		return (time_t) (-1);
}

