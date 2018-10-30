/*
 *   File name: generalproc.h
 *
 *  Created on: 2017年4月8日, 下午11:01:36
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_GENERALPROC_H__
#define __KERNEL_INCLUDE_GENERALPROC_H__

#include <list.h>
#include <spinlock.h>
#include "thread.h"
#include "../vfs/iocontext.h"

namespace Kernel
{
	namespace MM
	{
		class Vmm;
	}

	class Scheduler;
	class ProcManager;
	class ThreadManager;
	namespace Procedure
	{
		class Process;
		class Thread;

		class GeneralProc
		{
			friend class Kernel::Scheduler;
			friend class Kernel::ProcManager;
			friend class Kernel::ThreadManager;
			friend class MM::Vmm;

			protected:
				struct{
					word_t _pad0 : 4 ;
					word_t _processId : 16;
					word_t _threadCounter : 12;
				};

				ushort _priority;
				State _state;

				GeneralProc* _parent;
				Spinlock     _spinlock;

				DoubleList _threadHeader;
				DoubleList _pageHeader;

				DoubleList _childHeader;

				FS::IoContext _io;

			    clock_t _clockUser;
			    clock_t _clockSystem;
			    clock_t _clockCUser;
			    clock_t _clockCSystem;

				GeneralProc(const GeneralProc&) = delete;
				GeneralProc& operator=(const GeneralProc&) = delete;

				void addThread(Thread* thread)
				{
					if(thread->_proc == nullptr)
						thread->_proc = this;
					addNodeToRingDoubleListTail(&_threadHeader, &thread->_procList);
				}

				void delThread(Thread* const t)
				{
					delNodeFromRingDoubleList(&t->_procList);
				}

			protected:
				explicit GeneralProc(void){}

				void init(GeneralProc* parent, uint priority, FS::VFSDir* cwd = nullptr)
				{
					initRingDoubleList(&_threadHeader);
					initRingDoubleList(&_pageHeader);
					initRingDoubleList(&_childHeader);

					_parent = parent;
					_priority = (ushort)priority;
					_state = State::Ready;

					_io.init(cwd);
				}

			public:

				pid_t processId(void)const{ return _processId; }
				word_t threadNum(void)const{ return _threadCounter; }
				word_t priority(void)const{ return _priority; }

				clock_t getClockUser(void)const{ return _clockUser; }
				clock_t getClockSystem(void)const{ return _clockSystem; }

				void addChild(Process* newChild);
				void removeChild(Process* delChild);
		};
	}
}

#if !defined(_KERNEL_GENERALPROCESS_ALIAS_)
#define _KERNEL_GENERALPROCESS_ALIAS_ 1
#define KGProc Kernel::Procedure::GeneralProc
#endif

#endif

