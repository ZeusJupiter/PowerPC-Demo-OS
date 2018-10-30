/*
 *   File name: barrier.h
 *
 *  Created on: 2017年4月10日, 下午9:22:31
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_BARRIER_H__
#define __INCLUDE_BARRIER_H__

inline void barrier(void)
{
    __asm__ __volatile__ ("" : : : "memory");
}

#endif

