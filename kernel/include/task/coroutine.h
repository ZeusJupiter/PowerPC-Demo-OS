/*
 *   File name: coroutine.h
 *
 *  Created on: 2017年6月26日, 下午4:16:47
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_COROUTINE_H__
#define __KERNEL_INCLUDE_COROUTINE_H__

#include <list.h>

typedef struct stContext Context;

namespace Kernel
{
	namespace Procedure
	{
		class CoRoutine
		{
		public:
			Context* _threadStackPtr;
			Context* _mainThreadStackTopPtr;
			Context* _threadStackBottom;
			size_t _threadStackSizeInWord;
			Context* _threadStackLowestAddr;
			DoubleList _ringCoRoutineHeader;
			pvoid _coroutineArg;
			bool _isNeededFree;
		};
	}
}

#if !defined(_KERNEL_PROCEDURE_ALIAS_)
#define _KERNEL_PROCEDURE_ALIAS_ 1
#define KProcedure Kernel::Procedure
#endif

#if !defined(_KERNEL_COROUTINE_ALIAS_)
#define _KERNEL_COROUTINE_ALIAS_ 1
#define KCoRoutine Kernel::Procedure::CoRoutine
#endif

#endif

