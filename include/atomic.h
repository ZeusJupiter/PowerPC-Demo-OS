/*
 *   File name: atomic.h
 *
 *  Created on: 2017年3月14日, 下午6:03:44
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_ATOMIC_H__
#define __INCLUDE_ATOMIC_H__

class Atomic{
public:
	inline Atomic(void){}
	Atomic(const Atomic&) = delete;
	Atomic& operator=(const Atomic&) = delete;

	void init(word_t initVal){ val = initVal; }

	int operator ++(void);
	int operator --(void);

	int operator ++ (int){ return operator++() - 1; }
	int operator -- (int){ return operator--() + 1;}

    int operator = (word_t val)
	{ return this->val = val; }

    int operator = (int val)
	{ return this->val = val; }

    bool operator == (word_t val)
	{ return (this->val == val); }

    bool operator == (int val)
	{ return (this->val == (word_t) val); }

    bool operator != (word_t val)
	{ return (this->val != val); }

    bool operator != (int val)
	{ return (this->val != (word_t) val); }

    operator word_t (void) const
	{ return val; }

    static word_t atomicAnd(void volatile *addr, word_t mask);
    static word_t atmoicOr(void volatile *addr, word_t mask);
    static word_t add(void volatile* add, word_t val);

private:
	volatile word_t val;
};

#if ( (CONFIG_CPU_FAMILY == POWERPC) || defined(POWERPC) )
#include "arch/powerpc/atomic.h"
#endif

#endif

