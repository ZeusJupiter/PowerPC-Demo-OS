/*
 *   File name: ksymbols.cpp
 *
 *  Created on: 2017年6月1日, 下午11:29:05
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <ksymbols.h>

#define KERNEL_AREA  0xC0000000

extern KSymbols::Symbol kernelSymbolTalbe[];
extern uint kernelSymbolCount;

