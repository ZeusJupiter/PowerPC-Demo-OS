/*
 *   File name: kconst.h
 *
 *  Created on: 2017年4月28日, 上午9:01:12
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description: all constants related kernel will be placed here now.
 */

#ifndef __KERNEL_INCLUDE_KCONST_H__
#define __KERNEL_INCLUDE_KCONST_H__

namespace Kernel
{
    namespace MM
	{
		const word_t PageSize = (1 << 12);
		const word_t PageDirectoryBits = 10;
		const word_t PageDirectoryShift = 22;
		const word_t PageTableEntryNumber = 1024;
		const word_t PageDirectoryEntryNumber = 1024;
		const word_t PageTableEntryRpnMask  = 0xFFFFF000;
		const word_t PageDirectoryEntryMask = 0xFFFFF000;
		const word_t PageTableEntryTransMask = 0xFFFFF1FB;
		const word_t PageTableEntryAttrMask  = 0x000001FB;
	}

	namespace Procedure
	{
		const s32 MaxPriority = 255;

		const u32 ContextAlignment = 8;
		const u32 KernelContextSize = 1024;

		const u32 InherentThreadNum = 200;
		const u32 InherentProcessNum = InherentThreadNum >> 2;

		const u32 InherentSemaphoreNum = InherentThreadNum;
		const u32 InherentMutexNum = InherentThreadNum;
	}

	namespace IPC
	{
		const u32 SignalContextCount = Procedure::InherentThreadNum ;
	}
}

#if !defined(__KERNEL_IPC_ALIAS_)
#define __KERNEL_IPC_ALIAS_ 1
#define KIPC Kernel::IPC
#endif

#if !defined(__KERNEL_MM_ALIAS_)
#define __KERNEL_MM_ALIAS_ 1
#define KMM Kernel::MM
#endif

#if !defined(__KERNEL_Procedure_ALIAS_)
#define __KERNEL_Procedure_ALIAS_ 1
#define KProcedure Kernel::Procedure
#endif

#endif

