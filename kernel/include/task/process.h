/*
 *   File name: process.h
 *
 *  Created on: 2017年3月21日, 上午11:03:14
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_PROCEDURE_PROCESS_H__
#define __KERNEL_PROCEDURE_PROCESS_H__

#include <list.h>
#include <debug.h>
#include <assert.h>
#include "generalproc.h"
#include "state.h"

namespace Kernel
{
	namespace MM
	{

		class Vmm;
	}
	class ProcManager;
	namespace Procedure
	{
		class Thread;
		class Process : public GeneralProc
		{
		public:
			void setParent(GeneralProc* parent);

		private:
			explicit Process(){}
			Process(const Process&) = delete;
			Process& operator=(const Process&) = delete;

			void init(GeneralProc* parent, uint priority, Thread* mainThread);

			void setMainThread(Thread* mainThread)
			{
				assert(_mainThread == nullptr && mainThread != nullptr);
				_mainThread = mainThread;
				addThread(mainThread);
			}

			Thread* _mainThread;

			DoubleList _procList;
			DoubleList _pList;

			addr_t _execStartAddr;
			addr_t _execEndAddr;

			addr_t _bssStartAddr;
			addr_t _bssEndAddr;

			addr_t _heapStartAddr;
			addr_t _heapEndAddr;

			friend class MM::Vmm;
			friend class Kernel::ProcManager;
			friend class GeneralProc;
		} ;

	}
}

#if !defined(_KERNEL_PROCEDURE_ALIAS_)
#define _KERNEL_PROCEDURE_ALIAS_ 1
#define KProcedure Kernel::Procedure
#endif

#if !defined(_KERNEL_PROCESS_ALIAS_)
#define _KERNEL_PROCESS_ALIAS_ 1
#define KProcess Kernel::Procedure::Process
#endif

#endif

