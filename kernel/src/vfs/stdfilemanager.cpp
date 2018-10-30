/*
 *   File name: stdfilemanager.cpp
 *
 *  Created on: 2017年7月20日, 下午11:13:54
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <lockguard.h>
#include <string.h>
#include <mk/oscfg.h>

#include <kernel.h>

#include <vfs/stdfilemanager.h>

extern "C"
{
	int __sclose(void *cookie);
	int __sread(void *cookie, char *buf, int n);
	fpos_t __sseek(void *cookie, fpos_t offset, int whence);
	int __swrite(void *cookie, char const *buf, int n);
	void __sinit(void);
}

namespace FS
{
	class StdFileMng;
	Mutex StdFileMng::_stdFileLock;
	SingleList  StdFileMng::_stdFileListHeader;
	StdFileMng::StdFileNode StdFileMng::_stdFileArray[200];

	void StdFileMng::init(void)
	{
		_stdFileLock.init();
		__sinit();
		_stdFileListHeader.next = &(_stdFileArray[0]._slinkList);
		uint i;
		for(i = 0; i < 199; ++i)
		{
			_stdFileArray[i]._slinkList.next = &(_stdFileArray[i + 1]._slinkList);
		}
		_stdFileArray[i]._slinkList.next = &_stdFileListHeader;
	}

	FILE* StdFileMng::createStdFile(void)
	{
		FILE* ret = nullptr;
		{
			LockGuard<Mutex> g(_stdFileLock);
			if(!ringListIsEmpty<SingleList>(&_stdFileListHeader))
			{
				SingleList* sl = delNodeFromRingSingleListHeader(&_stdFileListHeader);
				ret = &((List_Entry(sl, StdFileNode, _slinkList))->_file);
			}
		}

		return ret;
	}

	FILE* StdFileMng::initStdFile(FILE* stdFile)
	{
		if(stdFile != nullptr)
		{
			zero_block(stdFile, sizeof(FILE));

			stdFile->_flags      = 1;

			stdFile->_file       = -1;

			stdFile->_cookie     = (void *)stdFile;

			stdFile->_close      = __sclose;
			stdFile->_read       = __sread;
			stdFile->_seek       = __sseek;
			stdFile->_write      = __swrite;
		}

		return stdFile;
	}

	void  StdFileMng::destroyStdFile(FILE* stdFile)
	{
		if(stdFile)
		{
			StdFileNode* stdfileNode = List_Entry(stdFile, StdFileNode, _file);
			LockGuard<Mutex> g(_stdFileLock);
			addNodeToRingSingleListHeader(&_stdFileListHeader, &stdfileNode->_slinkList);
		}
	}
}

BEG_EXT_C

FILE ** __stdin__  (void)
{
	KThread * t = KCommKernel::curThread();
	if(t->stdIn() == nullptr)
	{

		t->setStdIn(FS::StdFileMng::initStdFile(FS::StdFileMng::createStdFile()));

	}

	return t->stdIn();
}
FILE ** __stdout__(void)
{
	KThread * t = KCommKernel::curThread();
	if(t->stdOut() == nullptr)
	{

		t->setStdOut(FS::StdFileMng::initStdFile(FS::StdFileMng::createStdFile()));

	}

	return t->stdOut();
}
FILE ** __stderr__(void)
{
	KThread * t = KCommKernel::curThread();
	if(t->stdError() == nullptr)
	{

		t->setStdError(FS::StdFileMng::initStdFile(FS::StdFileMng::createStdFile()));

	}

	return t->stdError();
}

END_EXT_C

