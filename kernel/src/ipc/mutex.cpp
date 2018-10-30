/*
 *   File name: mutex.cpp
 *
 *  Created on: 2017年4月24日, 下午10:32:04
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>

#include <barrier.h>
#include <mutex.h>

#include <task/thread.h>
#include <kernel.h>

#if !defined(_DEBUG)
#define _DEBUG 1
#define TEST_MUEXT 1
#endif

#include <debug.h>
#include <assert.h>

void Mutex::lock(void)
{
	assert(_sem.getValue() < 2);
	KThread* curThread = KCommKernel::curThread();

	if(curThread->id() != _holder)
	{
		_sem.lock();
		assert(_holder == INVALID && _depth == 0);
		_holder = curThread->id();
	}

	++_depth;
}

bool Mutex::trylock(void)
{
	assert(_sem.getValue() < 2);
	KThread* curThread = KCommKernel::curThread();

	if(_holder == curThread->id())
	{
		++_depth;
		return true;
	}

	bool res = _sem.trylock();
	if(res)
	{
		_holder = curThread->id();
		++_depth;
	}

	return res;
}

void Mutex::unlock(void)
{
#if defined(TEST_MUEXT)
	assert(_sem.getValue() < 1);
	KThread* curThread = KCommKernel::curThread();
	assert((curThread->id() == _holder));
	assert(_depth > 0);
#endif

	if(--_depth == 0)
	{
		_holder = INVALID;
		barrier();
		_sem.unlock();
	}
}

void Mutex::destroy(void)
{
	if(_depth < 0)
	{
		_holder = INVALID;
		_depth = 0;
		barrier();
		_sem.unlock();
	}
}

#if defined(TEST_MUEXT)
#undef _DEBUG
#undef TEST_MUEXT
#endif
