/*
 *   File name: thread.h
 *
 *  Created on: 2017年4月5日, 下午5:59:10
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_THREAD_H__
#define __KERNEL_INCLUDE_THREAD_H__

#include <errno.h>
#include <list.h>
#include <spinlock.h>
#include <lockguard.h>
#include <selectcontext.h>
#include "state.h"

typedef struct stSigaltStack signalstack_t;
EXTERN_C sint sigaltstack(const signalstack_t *, signalstack_t *);

typedef struct stContext Context;
typedef struct stFpContext FpContext;
typedef struct sigcontext sigcontext_t;

typedef	struct __sFILE FILE;

class Semaphore;

namespace FS
{
	struct IoContext;
}

namespace Kernel
{
	class Scheduler;
	class ThreadManager;
	class CommonKernel;
	namespace Procedure
	{
		class Process;
		class GeneralProc;
		class Thread
		{
		public:
			enum class Options : u32
			{
				CheckStack          = 0x00000001,
				ClearStack          = 0x00000002,
				UseFPU          = 0x00000004,
				Suspended          = 0x00000010,
				Unpreemptrive     = 0x00000020,
				Init             = 0x00000040,
				Safe             = 0x00000080,
				Detached         = 0x00000100,
				UnSelect         = 0x00000200,
				NoMonitor       = 0x00000400,
				ScopeProcess    = 0x00000800,
				AlwayesAffinity  = 0x20000000,
				MainThreadStack  = 0x40000000,
			};

			tid_t id(void)const{ return this->_id; }
			Procedure::State state(void)const{ return _state; }
	
			void start(const word_t ms = 0);

			s32 getPriority(void)const
			{
				return _priority;
			}

			const char* cwd(void);
			void suspend(word_t tid);

			void suspendSelf(void){ suspend(_id); }
			void restart(word_t tid);
			void sleep(const word_t ms = 0);
			void wakeup(word_t tid);
#if defined(_DEBUG) || defined(_RELEASE_WITH_MSG)
			void show(void);
#else
			void show(void){}
#endif

			cpuid_t getCPU(void)const{ return _cpu; }

			void lock(void){ _spinLock.lock(); }
			bool tryLock(void){ return _spinLock.trylock(); }
			void unlock(void){ _spinLock.unlock(); }

			void ref(void){ LockGuard<Spinlock> g(_spinLock); ++_refCount; }
			void unref(void){ LockGuard<Spinlock> g(_spinLock); --_refCount; }

			void setLastError(ErrNo err){ _lastError = err; }

			void setStdIn(FILE* stdIn){ _stdFd[0] = stdIn; }
			void setStdOut(FILE* stdOut){ _stdFd[1] = stdOut; }
			void setStdError(FILE* stdError){ _stdFd[2] = stdError; }

			FILE** stdIn(void){ return &_stdFd[0]; }
			FILE** stdOut(void){ return &_stdFd[1]; }
			FILE** stdError(void){ return &_stdFd[2]; }

			clock_t getExecTicks(void)const{ return _execTicks; }
		private:
			explicit Thread(void){}
			Thread(const Thread& other) = delete;
			Thread& operator=(const Thread&) = delete;
			void setPriority(const s32 priority);

			Context* _context;
			addr_t _startAddr;
			addr_t _endAddr;
			GeneralProc* _proc;
			VoidFuncPtrPvoid _entry;
			Procedure::State _state;
			ushort  _priority;
			tid_t   _id;
			ushort  _refCount;

			DoubleList _procList;
			DoubleList _commList;

			Spinlock _spinLock;
			cpuid_t _cpu;
			ErrNo _exitCode;
			ErrNo _lastError;
			addr_t _ipcBuffer;
			FILE*  _stdFd[3];
			SelectContext* _selectContext;
			FS::IoContext* _io;
			FpContext* _fpContext;
			uint _execTicks;
			uint _totalTicks;
			uint _slices;
			u32 _options;

			friend class Kernel::Scheduler;
			friend class Kernel::ThreadManager;
			friend class GeneralProc;
			friend class ::Semaphore;
			friend class Kernel::CommonKernel;
			friend sint ::sigaltstack(const signalstack_t *, signalstack_t *);
		};
	}
}

#if !defined(_KERNEL_PROCEDURE_ALIAS_)
#define _KERNEL_PROCEDURE_ALIAS_ 1
#define KProcedure Kernel::Procedure
#endif

#if !defined(_KERNEL_THREAD_ALIAS_)
#define _KERNEL_THREAD_ALIAS_ 1
#define KThread Kernel::Procedure::Thread
#endif

#endif

