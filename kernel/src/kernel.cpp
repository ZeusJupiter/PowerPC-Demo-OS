/*
 *   File name: kernel.cpp
 *
 *  Created on: 2017年6月24日, 下午8:51:10
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <assert.h>
#include <debug.h>
#include <barrier.h>

#include <mk/hardware.h>

#include <kernel.h>

namespace Kernel
{
	const CommonKernel::VoidFuncPtrSchedulerPtr CommonKernel::switchContext = Powerpc::contextSwitch;
	const U32FuncPtrVoid CommonKernel::closeInterrupt = Powerpc::Interrupt::closeInterrupt;
	const VoidFuncPtrU32 CommonKernel::restoreInterrupt = Powerpc::Interrupt::restoreInterrupt;

	struct timespec CommonKernel::_currentTOD;
	struct timespec CommonKernel::_monoTOD;
	u64 CommonKernel::_lastTime;
	u64 CommonKernel::_tickCounter;

	const VoidFuncPtrVoid CommonKernel::ioLock = FS::VFS::lockTree;
	const VoidFuncPtrVoid CommonKernel::ioUnlock = FS::VFS::unlockTree;

	u32 CommonKernel::_interruptNestingCounter;
	Spinlock CommonKernel::_kernelLock;
	u32 CommonKernel::_kernelCounter;
	pvoid CommonKernel::_kernelOwer;
	c8* CommonKernel::_kernelEnterFunc;
	ErrNo CommonKernel::_errorWhenNotRun;

	Scheduler CommonKernel::_schedulerTable[Hardware::CPU::CPUCount];

	void CommonKernel::handleSysTickInterrupt(void)
	{
		s32 delta = 0;
		do
		{
			u64 newTime = _lastTime + Powerpc::TimerInterval;
			u64 curTime = CPU::timeStamp();
			_lastTime = newTime;
			delta = newTime - curTime;

			++_tickCounter;
		}while(delta < 0);

		Powerpc::setDEC(delta);

		_schedulerTable[CPU::id()].scheduleInInterrupt();
	}

	void CommonKernel::idle(void)
	{
		while(true)
		{
			kdebugln("I am idle");
			CPU::sleep();
		}
	}

	void CommonKernel::idle2(void)
	{
		while(true)
		{
			kdebugln("I am idle2");
			CPU::sleep();
		}
	}

	void CommonKernel::lockKernelAndCloseInterrupt(volatile u32* intVal)
	{
		_kernelLock.lock();

		if (!_interruptNestingCounter)
		{
			_schedulerTable[CPU::id()].curThreadSafe();
		}

		*intVal = closeInterrupt();
	}

	void CommonKernel::enterKernel(c8* func)
	{
	    volatile u32 intVal;
	    lockKernelAndCloseInterrupt(&intVal);
	    _kernelCounter++;
	    ::barrier();

	    _kernelOwer = func;
	    _kernelEnterFunc = func;

	    restoreInterrupt(intVal);
	}

	void CommonKernel::unlockKernelWhenSwitchContext(void)
	{
		_kernelLock.unlock();
		if (!_interruptNestingCounter)
		{
			_schedulerTable[CPU::id()].curThreadSafe();
		}
	}

	void CommonKernel::unlockKernelAndRestoreInterrupt(u32 intVal)
	{
		_kernelLock.unlock();

		if (!_interruptNestingCounter)
		{
			_schedulerTable[CPU::id()].curthreadUnsafe();
		}

		restoreInterrupt(intVal);
	}

	void CommonKernel::justLockKernel(void)
	{
		_kernelLock.lock();
		if (!_interruptNestingCounter)
		{
			_schedulerTable[CPU::id()].curThreadSafe();
		}
	}

	sint CommonKernel::exitKernel(void)
	{
		u32 intVal;
	    sint ret = 0;

	    intVal = closeInterrupt();

	    if (--_kernelCounter == 0) {
		    _kernelOwer = nullptr;
		    _kernelEnterFunc = nullptr;

			ret = _schedulerTable[CPU::id()].scheduleInKernel();
	    }

	    unlockKernelAndRestoreInterrupt(intVal);

	    return  (ret);
	}

}

