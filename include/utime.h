/*
 *   File name: utime.h
 *
 *  Created on: 2017年7月19日, 上午10:13:58
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_UTIME_H__
#define __INCLUDE_UTIME_H__

#include "macros.h"
#include "types.h"

struct utimbuf
{
	time_t actime;
	time_t modtime;

};

sint utime(c8*  fileName, const struct utimbuf *utimbNew);
sint utimes(c8*  fileName, struct timeval tvp[2]);
sint futimes(sint fd, struct timeval tvp[2]);

#endif

