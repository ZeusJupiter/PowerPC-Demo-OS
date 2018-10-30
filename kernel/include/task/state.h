/*
 *   File name: state.h
 *
 *  Created on: 2017年4月5日, 下午6:14:49
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_STATE_H__
#define __KERNEL_INCLUDE_STATE_H__

namespace Kernel
{
	namespace Procedure
	{
		enum class State : ushort{
			Stopped = 0,
			Ready = 1,
			Waiting = 2,
			Lock = 3,
			WaitingNextActivation = 4,
			DelayedStart = 5,
			Running = 6
		};

		enum class ProcAttr : word_t{
			RealtimeProc,
			NormalProc
		};
	}

}

#endif

