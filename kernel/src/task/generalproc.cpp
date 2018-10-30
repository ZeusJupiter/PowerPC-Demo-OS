/*
 *   File name: generalproc.cpp
 *
 *  Created on: 2017年4月8日, 下午11:03:44
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <assert.h>

#include <task/process.h>

namespace Kernel
{
	namespace Procedure
	{
		void GeneralProc::addChild(Procedure::Process* newChild)
		{
			addNodeToRingDoubleListTail(&_childHeader, &newChild->_pList);
		}

		void GeneralProc::removeChild(Procedure::Process* delChild)
		{
			delNodeFromRingDoubleList(&delChild->_pList);
		}
	}
}
