/*
 *   File name: process.cpp
 *
 *  Created on: 2017年4月1日, 下午6:30:49
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>

#include <task/process.h>

namespace Kernel
{
	namespace Procedure
	{
		void Process::setParent(GeneralProc* parent)
		{
			if(_parent)
				_parent->removeChild(this);
			if(parent)
				parent->addChild(this);
			_parent = parent;
		}
		void Process::init(GeneralProc* parent, uint priority, Thread* mainThread)
		{
			GeneralProc::init(parent, priority);

			_mainThread = mainThread;

			if(parent)
				parent->addChild(this);

			if(mainThread)
				addThread(mainThread);
		}
	}

}
