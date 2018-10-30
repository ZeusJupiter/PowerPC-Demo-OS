/*
 *   File name: spinlock.h
 *
 *  Created on: 2017年3月4日, 上午4:21:05
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_ARCH_POWERPC_SPINLOCK_H__
#define __INCLUDE_ARCH_POWERPC_SPINLOCK_H__

inline void Spinlock::lock(void)
{
	word_t old;
	__asm__ __volatile__ (
		" 1: lwarx  %0, 0, %1  \n"
		"   cmpwi  0, %0, 0   \n"
		"   bne-   1b   \n"
		"  stwcx.  %2, 0, %1   \n"
		"   bne-   1b   \n"
		"  isync \n"
		: "=&r"(old)
		: "r"(&_key), "r"(1)
		: "cr0", "memory"
	);
}
inline bool Spinlock::trylock(void)
{
	word_t old;
	__asm__ __volatile__(
		" 1: lwarx %0, 0, %1 \n"
		"    cmpwi 0, %0, 0 \n"
		"    bne- 2f \n"
		"   stwcx. %2, 0, %1 \n"
		"    isync \n"
		" 2: \n"
		: "=&r"(old)
		: "r"(&_key), "r"(1)
		: "cr0", "memory"
	);

	return (!old && _key);
}
inline void Spinlock::unlock(void)
{
	__asm__ __volatile__ ( "sync; eieio"::: "memory");
	_key = 0;
}

#endif

