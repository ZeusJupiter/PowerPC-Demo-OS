/*
 *   File name: spinlock.h
 *
 *  Created on: 2017年3月17日, 上午1:37:53
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_SPINLOCK_H__
#define __INCLUDE_SPINLOCK_H__

#if !defined(ASSEMBLY)

class Spinlock
{
public:
	inline explicit Spinlock(void){}
	Spinlock(const Spinlock&) = delete;
	Spinlock& operator=(const Spinlock&) = delete;

	void lock(void);
	bool trylock(void);
	void unlock(void);
	bool isLocked() { return _key != 0 ;}

	void init(word_t initVal){ _key = initVal; }

private:
	volatile word_t _key;
};

#if ( defined(POWERPC) && (CONFIG_CPU_FAMILY == POWERPC) )
#include "arch/powerpc/spinlock.h"

#endif

#endif

#endif

