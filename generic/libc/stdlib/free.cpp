/*
 *   File name: free.cpp
 *
 *  Created on: 2017年7月13日, 上午7:57:54
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
void free (void *freeAddr)
{
	if(freeAddr == nullptr) return;
	kheap.free(freeAddr);
}

