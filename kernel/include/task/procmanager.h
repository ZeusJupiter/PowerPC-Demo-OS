/*
 *   File name: procmanager.h
 *
 *  Created on: 2017年4月1日, 下午4:06:44
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_PROCMANAGER_H__
#define __KERNEL_INCLUDE_PROCMANAGER_H__

#include <spinlock.h>
#include "process.h"

namespace Kernel
{
	namespace MM
	{
		class Vmm;
	}

	class Scheduler;
	class ProcManager
	{
		friend class Scheduler;
		static word_t ProcessId;
		static word_t getNewProcessId(void)
		{
			register word_t ret = ProcessId++;
			return ret;
		}

		static DoubleList _usedProcListHeader;
		static DoubleList _freeInherentProcListHeader;

		static Spinlock _usedProcListSpinlock;
		static Spinlock _inherentProcListSpinlock;

		static word_t _curUsedProcNum;
		static word_t _maxUsedProcNum;

		static Procedure::GeneralProc _rootProc;
		static Procedure::Process _inherentProcs[];

		static Procedure::Process* allocate(Procedure::ProcAttr procAttr);
		static void deallocate(Procedure::Process* const process);

		explicit ProcManager(void) = delete;
		ProcManager(const ProcManager& ) = delete;
		ProcManager & operator=(const ProcManager&) = delete;

	public:
		static void init(void);
		static Procedure::Process* createProcess(const char* filename, Procedure::ProcAttr procAttr);

		static void destroyProc(Procedure::Process* desProc);
	};
}

#if !defined(_KERNEL_PROCMANAGER_ALIAS_)
#define _KERNEL_PROCMANAGER_ALIAS_ 1
#define KProcMng Kernel::ProcManager

#endif

#endif

