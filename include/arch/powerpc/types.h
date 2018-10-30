/*
 *   File name: types.h
 *
 *  Created on: Nov 11, 2016, 9:32:30 PM
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 */

#ifndef __POWERPC_TYPE_H__
#define __POWERPC_TYPE_H__

#if !defined(ASSEMBLY)

#include "stdint.h"

InlineStatic word_t UNUSED equal(word_t arg0, word_t arg1)
{
	word_t result;
	__asm__ __volatile__("eqv %0, %1, %2" : "=r"(result) : "r"(arg0), "r"(arg1));
	return result;
}

InlineStatic word_t UNUSED loadReserve(addr_t addr)
{
	word_t val;
	__asm__ __volatile__("lwarx %0, 0, %1" : "=r"(val) : "r"(addr));
	return val;
}

InlineStatic word_t UNUSED storeReserve(addr_t addr, word_t val)
{
	word_t out;
	__asm__ __volatile__(
		"stwcx. %1, 0, %2 \n"
		"mfcr %0 \n"
		: "=r"(out)
		: "r"(val), "r"(addr)
		: "cr0", "memory"
	);
	return ( out >> 28);

}

InlineStatic void UNUSED sync(void)
{
	__asm__ __volatile__ ("sync");
}
InlineStatic void UNUSED isync(void)
{
	__asm__ __volatile__ ("isync");
}
InlineStatic void UNUSED tlbsync(void)
{
	__asm__ __volatile__ ("tlbsync");
}
InlineStatic void UNUSED eieio(void)
{
	__asm__ __volatile__ ("eieio");
}
InlineStatic word_t UNUSED getTbl(void)
{
	word_t ret;
	__asm__ __volatile__("mftb %0 " : "=r"(ret));
	return ret;
}
InlineStatic word_t UNUSED getTbu(void)
{
	word_t ret;
	__asm__ __volatile__("mftbu %0 " : "=r"(ret));
	return ret;
}
InlineStatic void UNUSED setTbl(word_t val)
{
	__asm__ __volatile__("mttbl %0 " : :"r"(val));
}
InlineStatic void UNUSED setTbu(word_t val)
{
	__asm__ __volatile__("mttbu %0 " : :"r"(val));
}

#endif
#endif
