/*
 *   File name: hid1.h
 *
 *  Created on: 2017年8月1日, 下午9:26:02
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_ARCH_POWERPC_60X_HID1_H__
#define __KERNEL_INCLUDE_ARCH_POWERPC_60X_HID1_H__

namespace
{
	u32 getHid1(void)
	{
		register u32 ret;
		__asm__("mfspr %0, 1009" : "=r"(ret)::);
		return ret;
	}
}

#endif

