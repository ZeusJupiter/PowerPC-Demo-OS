/*
 *   File name: iocontext.h
 *
 *  Created on: 2017年5月31日, 下午5:29:52
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_VFS_IOCONTEXT_H__
#define __INCLUDE_VFS_IOCONTEXT_H__

#include <mutex.h>
#include "vfsdir.h"

namespace FS
{
	struct IoContext
	{
		Mutex	 _ioMutex;
		VFSDir*  _cwd;
		s32		 _refCount;
		u32		 _fdsUsedCount;

		void init(VFSDir* const cwd = nullptr)
		{
			_ioMutex.init();
			_cwd = cwd;
			_refCount = 0;
			_fdsUsedCount = 0;
		}
	};
}

#endif

