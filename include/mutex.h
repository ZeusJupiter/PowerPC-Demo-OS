/*
 *   File name: mutex.h
 *
 *  Created on: 2017年4月10日, 上午1:59:41
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_MUTEX_H__
#define __INCLUDE_MUTEX_H__

#include <semaphore.h>

class Mutex
{
public:

	void lock(void);
	bool trylock(void);
	void unlock(void);
	s32  getValue(void)const{ return _sem.getValue(); }
	void destroy(void);

	explicit Mutex(void){}

	void init(void)
	{
		_sem.init(1);
		_depth = 0;
		_holder = INVALID;
	}

private:

	Mutex(const Mutex&) = delete;
	Mutex operator=(const Mutex&) = delete;

	Semaphore _sem;
	word_t _holder;
	sint _depth;

	friend class Kernel::SemManager;
	static const word_t INVALID = 0xFFFF;
};

#endif

