/*
 *   File name: stdfilemanager.h
 *
 *  Created on: 2017年7月20日, 下午11:01:48
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_SRC_VFS_STDFILEMANAGER_H__
#define __KERNEL_SRC_VFS_STDFILEMANAGER_H__

#include <stdio.h>
#include <list.h>
#include <mutex.h>

namespace FS
{
	class StdFileMng
	{
		struct StdFileNode
		{
			SingleList _slinkList;
			FILE _file;
		};
	public:
		static void init(void);
		static FILE* createStdFile(void);
		static FILE* initStdFile(FILE* stdFile);
		static void  destroyStdFile(FILE* stdFile);
	private:
		static Mutex _stdFileLock;
		static SingleList _stdFileListHeader;
		static StdFileNode _stdFileArray[];
	};
}

#endif

