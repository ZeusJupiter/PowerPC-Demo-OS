/*
 *   File name: powerpc.cpp
 *
 *  Created on: 2017年4月8日, 下午8:39:45
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <string.h>

#include <cpu.h>

#include <mm/vmm.h>
#include <mm/heap.h>

#include <arch/powerpc/bat.h>
#include <arch/powerpc/cache.h>
#include <arch/powerpc/tlb.h>
#include <arch/powerpc/mmu.h>
#include <arch/powerpc/powerpc.h>

EXTERN_C const char __heap_start[], __heap_end[];

namespace Powerpc
{
	void init(void)
	{
		kdebugln("Hello " PROCESSOR_NAME);

		addr_t heapStart = (addr_t) __heap_start;
		addr_t heapEnd = (addr_t) __heap_end;
		addr_t tabStart, tabEnd;

		kvmm.firstInit();

		u32 tabSize = PHTab::optimalSize(kvmm.getFreePageNum() * KMM::PageSize);

		tabStart = alignUp(heapStart, tabSize);
		tabEnd = ((addr_t) tabStart + tabSize);

		if ((addr_t) tabStart >= heapEnd || tabEnd >= heapEnd)
		{
			while (1) ;
		}

		phtab.init((pvoid)tabStart, tabSize);

		kheap.addHeapBlock(heapStart, tabStart);
		kheap.addHeapBlock(tabEnd, heapEnd);

		if(kvmm.secondInit() != ErrNo::ENONE)
			CPU::halt();

		kdebugln("Kernel Heap has been initialized");

		MMU::init();
		MMU::enableMMU();
		setMSR(PPC_KERNEL_MSR);

		Cache::init();

		Cache::invalidateCacheAll();
		MMU::invalidateTlbAll();
		kdebugln("Cache and MMU has been invalidated");

		zero_block((word_t*) tabStart, tabSize);
	}
}

