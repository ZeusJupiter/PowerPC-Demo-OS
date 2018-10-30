/*
 *   File name: version.h
 *
 *  Created on: 2017年6月23日, 下午10:58:15
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_LINUX_VERSION_H__
#define __INCLUDE_LINUX_VERSION_H__

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c)   (((a) << 16) + ((b) << 8) + (c))
#endif
#define LINUX_VERSION_CODE      KERNEL_VERSION(3, 4, 0)

#endif

