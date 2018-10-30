/*
 *   File name: Atomic
 *
 *  Created on: 2017年3月4日, 上午1:23:47
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __PLATFORM_POWERPC_ATOMIC__
#define __PLATFORM_POWERPC_ATOMIC__

inline int Atomic::operator ++ (void)
{
	int tmp;
	sync();
	__asm__ __volatile__(
		"1: lwarx  %0, 0, %1 \n"
		"   addic  %0, %0, 1 \n"
		"   stwcx. %0, 0, %1 \n"
		"   bne-   1b"
		: "=&r"(tmp)

		: "r"(&val)
		: "cc"

	);
	isync();
	return tmp;
}

inline int Atomic::operator -- (void)
{
	int tmp;
	sync();
	__asm__ __volatile__(
		"1: lwarx  %0,  0, %1 \n"
		"   addic  %0, %0, -1 \n"
		"  stwcx.  %0,  0, %1 \n"
		"   bne-   1b"
		: "=&r"(tmp)
		: "r" (&val)
		: "cc"
	);
	isync();
	return tmp;
}

inline word_t Atomic::atomicAnd(void volatile *addr, word_t mask)
{
	word_t tmp;
	__asm__ __volatile__(
		"1: lwarx  %0, 0, %1  ;"
		"   and    %0, %0, %2 ;"
		"   stwcx. %0, 0, %1  ;"
		"   bne-   1b ;"
		: "=&r"(tmp)
		: "r"(addr), "r"(mask)
		: "cr0", "memory"
	);

	return tmp;
}

inline word_t Atomic::atmoicOr(void volatile *addr, word_t mask)
{
	word_t tmp;
	__asm__ __volatile__(
		"1: lwarx  %0, 0, %1  \n"
		"     or   %0, %0, %2 \n"
		"   stwcx.  %0, 0, %1  \n"
		"   bne-   1b"
		: "=&r"(tmp)
		: "r"(addr), "r"(mask)
		: "cr0", "memory"
	);

	return tmp;
}

inline word_t Atomic::add(void volatile* add, word_t val)
{
	word_t old;
	__asm__ __volatile__(
		"1: lwarx  %0,  0, %1 \n"
		"   add    %2, %0, %2 \n"
		"   stwcx. %2,  0, %1 \n"
		"   bne-   1b"
		: "=&r"(old)

		: "r"(add), "r"(val)
		: "cc"

	);
	return old;
}

#endif

