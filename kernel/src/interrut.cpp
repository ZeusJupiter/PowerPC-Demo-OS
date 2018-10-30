/*
 *   File name: interrut.cpp
 *
 *  Created on: 2017年3月20日, 下午5:12:27
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <lockguard.h>

#include <mk/hardware.h>

#include <kconst.h>
#include <mm/heap.h>

#include <interrupt.h>

namespace
{
	struct ExterIntptCallbackDesc
	{
		ExterIntptCallbackDesc* next;
		VoidFuncPtrVar callbackFunc;
		pvoid          argument;
		VoidFuncPtrVar clearFunc;
	};

	struct EexterIntptSrcDesc
	{
		ExterIntptCallbackDesc* callbackDescHeader;
		word_t interVectorflags;
		Spinlock spinlock;
		word_t intptCounter;
		word_t callbackFunCounter;
	};
	EexterIntptSrcDesc exterIntptSrcTable[128];
}

namespace Kernel
{
    namespace Interrupt
	{
    	ErrNo connectInterruptVector(sint vec, VoidFuncPtrVar intptServiceRoutine, pvoid argument, VoidFuncPtrVar clearFunc)
    	{
    		assert(intptServiceRoutine != nullptr);
    		assert(-1 < vec && vec < 128);

    		ExterIntptCallbackDesc * newCallbackNode = (ExterIntptCallbackDesc*)kheap.allocate(sizeof(ExterIntptCallbackDesc));
    		if(0 == newCallbackNode)
    		{
    			kformatinfoln("%s","Heap has been no enough memory");
    			return ErrNo::ENOMEM;
    		}

    		newCallbackNode->callbackFunc = intptServiceRoutine;
    		newCallbackNode->argument = argument;
    		newCallbackNode->clearFunc = clearFunc;

    		EexterIntptSrcDesc * eIntSrcDesc = &exterIntptSrcTable[vec];
    		eIntSrcDesc->spinlock.lock();

    		eIntSrcDesc->interVectorflags = 0;
    		eIntSrcDesc->callbackFunCounter++;

    		newCallbackNode->next = eIntSrcDesc->callbackDescHeader;
    		eIntSrcDesc->callbackDescHeader = newCallbackNode;

    		eIntSrcDesc->interVectorflags = 1;
    		eIntSrcDesc->spinlock.unlock();

    		return ErrNo::ENONE;
    	}

    	void disconnectInterruptVector(sint vec, pvoid arg)
    	{
    		assert(-1 < vec && vec < 128);

    		EexterIntptSrcDesc * eIntSrcDesc = &exterIntptSrcTable[vec];

    		register ExterIntptCallbackDesc* p;

    		{
				LockGuard<Spinlock> g(eIntSrcDesc->spinlock);
				eIntSrcDesc->interVectorflags = 0;
				register ExterIntptCallbackDesc* pp = nullptr;
				p = eIntSrcDesc->callbackDescHeader;
				for(;p; pp = p, p = p->next)
				{
					if(p->argument == arg)
					{
						if(pp)
						{
							pp->next = p->next;
						}
						else
						{
							eIntSrcDesc->callbackDescHeader = p->next;
						}
						break;
					}
				}
    		}

    		kheap.free(p);
    	}

    	void solveInterurpt(sint vec)
    	{
    		assert(-1 < vec && vec < 128);

    		EexterIntptSrcDesc * eIntSrcDesc = &exterIntptSrcTable[vec];
    		LockGuard<Spinlock> g(eIntSrcDesc->spinlock);

    		++eIntSrcDesc->intptCounter;
    		eIntSrcDesc->interVectorflags = 0;

    		for(ExterIntptCallbackDesc* p = eIntSrcDesc->callbackDescHeader;
    				p; p = p->next)
    		{
    			(p->callbackFunc)(p->argument);
    			if(p->clearFunc)
    				(p->clearFunc)(p->argument);
    		}

    		eIntSrcDesc->interVectorflags = 1;
    	}
	}
}

