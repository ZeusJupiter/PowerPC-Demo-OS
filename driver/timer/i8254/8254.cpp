/*
 *   File name: 8254.cpp
 *
 *  Created on: 2017年4月23日, 上午6:21:01
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <platform/platform.h>

#include <macros.h>
#include <types.h>
#include <io.h>
#include "8254.h"

namespace Driver
{
	void I8254::init(addr_t base, u32 freq, u32 initFreq)
	{
		_base = base;
		_freq = freq;

		u32 divisor = freq/initFreq;
		out8(_base + 3,  (3 << 4) | (3 << 1));

		out8(_base, divisor & 0xff);

		out8(_base, (divisor >> 8) & 0xff);
	}
}
