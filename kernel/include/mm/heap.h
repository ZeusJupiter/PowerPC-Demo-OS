/*
 *   File name: heap.h
 *
 *  Created on: 2017年3月14日, 下午3:52:47
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_MM_HEAP_H__
#define __KERNEL_INCLUDE_MM_HEAP_H__

#include <spinlock.h>
#include <list.h>

namespace Kernel
{
    namespace MM
	{
    	class Vmm;
    	class Heap
		{
    		friend class Vmm;
			DoubleList _freeRingListHeader;
			Spinlock _heapLock;
			addr_t _start;
			addr_t _end;
#if defined(_DEBUG)
			addr_t phyBlockHeader;
			word_t counter;
#endif
			void* separateFreeBlock(u32 size);
    	public:
	    	void addHeapBlock(addr_t from, addr_t to);
	    	void* allocate(word_t neededSize);
	    	void* allocate(word_t neededSize, word_t neededAlignment);
	    	void  free(pvoid baseAddr);
#if defined(_DEBUG)
	    	bool verifyPhyBlock(addr_t addr);
	    	void verify(void);
#else
	    	void verify(void){}
#endif
		};

    	extern Heap kernelHeap;
	}
}

#if !defined(__KERNEL_MM_ALIAS_)
#define __KERNEL_MM_ALIAS_ 1
#define KMM Kernel::MM
#endif

#if !defined(__KERNEL_MM_HEAP_ALIAS_)
#define __KERNEL_MM_HEAP_ALIAS_ 1
#define kheap Kernel::MM::kernelHeap
#endif

#endif

