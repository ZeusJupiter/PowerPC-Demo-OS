/*
 *   File name: semaphore.h
 *
 *  Created on: 2017年4月10日, 上午1:59:33
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_SEMAPHORE_H__
#define __INCLUDE_SEMAPHORE_H__

#include <list.h>
#include <spinlock.h>

namespace Kernel
{
	class SemManager;
}
class Mutex;
class Semaphore
{
	friend class Kernel::SemManager;
	friend class Mutex;
private:
	DoubleList _waiterList;
	s32        _val;
	Spinlock   _spinlock;

	Semaphore(const Semaphore&) = delete;
	Semaphore& operator=(const Semaphore&) = delete;

public:
	inline explicit Semaphore(void){}

	void init(int val)
	{
		_val = val;
		initRingDoubleList(&_waiterList);
	}

	void clear(int val){ _val = val; }

	void lock(void);

	bool trylock(void);

	void unlock(void);

	s32 getValue(void)const{ return _val; }

	s32 blkTaskCount(void)const{ return _val >= 0 ? 0 : (-_val);}

	void destroy(void);
};

#endif

