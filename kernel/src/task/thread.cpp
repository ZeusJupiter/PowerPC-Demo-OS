/*
 *   File name: thread.cpp
 *
 *  Created on: 2017年4月8日, 下午11:38:03
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>

#include <arch/powerpc/context.h>
#include <task/thread.h>
#include <kernel.h>
#include <task/threadmanager.h>
#include <vfs/iocontext.h>

namespace Kernel
{
	namespace Procedure
	{
		void Thread::start(const word_t ms)
		{}

		void Thread::setPriority(const s32 priority)
		{
			_priority = priority;
		}

		const char* Thread::cwd(void)
		{
			static char path[256];
			_io->_cwd->getPathTo(path, sizeof(path));
			return path;
		}

		void Thread::suspend(word_t tid)
		{
			Thread* that = ( tid == _id ) ? this : kthreadMng.getThreadById(tid);
			assert(that->getPriority() <= _priority);

			LockGuard<Spinlock> g(_spinLock);
			KCommKernel::curSheduler()->remove(that);
			that->_state = Procedure::State::Waiting;
		}

		void Thread::restart(word_t tid)
		{}

		void Thread::sleep(const word_t ms)
		{}

		void Thread::wakeup(word_t tid)
		{
			Thread* that = ( tid == _id ) ? this : kthreadMng.getThreadById(tid);
			switch(_state)
			{
			case Procedure::State::Waiting:
				{
					LockGuard<Spinlock> g(_spinLock);
					that->_state = Procedure::State::Ready;
					KCommKernel::curSheduler()->append(this);
				}
				break;
			default:
				break;
			}

			return;
		}

#if defined(_DEBUG) || defined(_RELEASE_WITH_MSG)
		void Thread::show(void)
		{
			kformatln("-------Thread Infor: id=%d--------", this->id());

			kformatln("this=0x%x", (addr_t)this);
			kformatln("_tlist=0x%x", (addr_t)(&_commList));
			kformatln("this id=0x%x", (addr_t)(&_id));

			kformatln("Priority=%d", _priority);
			kformatln("State=%d", state());
			kformatln("context=0x%x", (addr_t)_context);

			kformatln("StartAddr=0x%x", (addr_t)_startAddr);
			kformatln("EndAddr=0x%x", (addr_t)_endAddr);

			printContext(_context);

			kdebugln("-----------Thread Infor------------");
		}
#endif

	}
}
