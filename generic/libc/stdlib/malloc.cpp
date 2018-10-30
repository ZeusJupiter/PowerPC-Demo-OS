/*
 *   File name: malloc.cpp
 *
 *  Created on: 2017年7月13日, 下午4:14:27
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <stdlib.h>

#include <mm/heap.h>

EXTERN_C
void * malloc (size_t  sizeInByte)
{
	 return kheap.allocate(sizeInByte);

}
