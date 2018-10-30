/*
 *   File name: ksymbols.h
 *
 *  Created on: 2017年6月1日, 下午10:53:27
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_KSYMBOLS_H__
#define __INCLUDE_KSYMBOLS_H__

#include "mutex.h"

#define SYMBOL_FLAG_S   0x80000000
#define SYMBOL_FLAG_R   0x00000001
#define SYMBOL_FLAG_W   0x00000002
#define SYMBOL_FLAG_X   0x00000004

class KSymbols
{
public:
	struct Symbol
	{
		addr_t address;
		const char* funcName;
	};

	static void print(void);
	static Symbol* getSymbolByAddr(addr_t addr);
	static Symbol* getSymbolByName(const char* funcName);

};

#endif

