/*
 *   File name: asm.h
 *
 *  Created on: 2016年11月16日, 下午9:28:14
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:

 */

#ifndef __INCLUDE_ASM_H__
#define __INCLUDE_ASM_H__

#if defined( ASSEMBLY )

#if defined( __GNUC__ )
	#include "toolchain/gnu/asm.h"
#endif

#include "../platform/cpucfg.h"

#include "arch/powerpc/asm.h"
#include "arch/powerpc/regs.h"

#endif

#endif

