/*
 *   File name: interrupt.h
 *
 *  Created on: 2017年3月20日, 下午4:41:40
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_INTERRUPT_H__
#define __KERNEL_INCLUDE_INTERRUPT_H__

#include <macros.h>
#include <types.h>
#include <spinlock.h>
#include <errno.h>

namespace Kernel
{
    namespace Interrupt
	{
    	ErrNo connectInterruptVector(sint vectorNum, VoidFuncPtrVar intptServiceRoutine,
    			pvoid arg, VoidFuncPtrVar clearFunc);

    	void disconnectInterruptVector(sint vectorNum, pvoid arg);
    	void solveInterurpt(sint vecNum);
	}
}

#if !defined(__KERNEL_INTERRUPT_ALIAS_)
#define __KERNEL_INTERRUPT_ALIAS_ 1
#define KInterrupt Kernel::Interrupt
#endif

#endif

