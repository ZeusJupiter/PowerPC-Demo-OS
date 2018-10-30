/*
 *   File name: ctors.cpp
 *
 *  Created on: 2016年11月25日, 下午7:06:59
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>

#include <ctors.h>

BEG_EXT_C

void connectObjects()
{
	extern VoidFuncPtrVoid __globalObjFuncs[];
	for(uint i = 0; __globalObjFuncs[i] != 0; ++i)
		__globalObjFuncs[i]();
}

END_EXT_C

