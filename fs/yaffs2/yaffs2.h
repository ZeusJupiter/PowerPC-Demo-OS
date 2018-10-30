/*
 *   File name: yaffs2.h
 *
 *  Created on: 2017年6月23日, 下午8:33:46
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __FS_YAFFS2_YAFFS2_H__
#define __FS_YAFFS2_YAFFS2_H__

#include <errno.h>
#include <device.h>
#include <mutex.h>
#include <list.h>
#include <stddef.h>

namespace FS
{
	class VFSFile;
}

namespace YaffsFS
{
	class YaffsDev
	{
		static sint _yaffsDevDrvIndex;
		static sint  _isCreatedYaffsDev;
		static Mutex _yaffsDrvMutex;

		bool       _isForceDelete;
		DoubleList _nodeListHeader;

		void initDev(void);

		sint flush(FS::VFSNode*);
		sint dataSync(FS::VFSNode*);

		ErrNo seek(FS::VFSFile* node, off_t oftOffset);

		ErrNo nRead(FS::VFSFile* node, sint *piPos);
		ErrNo where(FS::VFSFile* node, off_t *poftPos);

		ErrNo nRead64(FS::VFSFile* node, off_t *poftPos);

		ErrNo format(void);

		ErrNo yaffsRename (FS::VFSNode* node, const c8* newFullPathName);

		ErrNo getStat (FS::VFSNode* node, struct stat *pstat);
		ErrNo getStatfs (FS::VFSNode* node, struct statfs *pstatfs);

		ErrNo chmod (FS::VFSNode* node, sint  iMode);

		ErrNo readDir (FS::VFSNode* node, struct DIR  *dir);

		ErrNo setTime (FS::VFSNode* node, struct utimbuf  *utim);
		ErrNo truncate (FS::VFSFile* node, loff_t  oftSize);

	public:

		virtual sint open(const c8* fileName, sint flags, sint openMode);
		virtual ErrNo remove(const c8* fileName);

		virtual ErrNo lStat(FS::VFSNode* node, c8* devName, struct stat *pstat);
		virtual ErrNo createSymlink(const c8* createdLinkName,const c8* linkDstName);
		virtual ssize_t readSymlink(c8* devName, c8* pcLinkDst, size_t bufferSize);

		virtual void closefile(FS::VFSNode* node);

		virtual ErrNo devClose();

		virtual ssize_t readFile(FS::VFSFile* node, c8* pcBuffer, size_t stMaxBytes);
		virtual ssize_t readFileExt(FS::VFSFile* node, c8* pcBuffer, size_t stMaxBytes, off_t oftPos);
		virtual ssize_t writeFile(FS::VFSFile* node, c8* pcBuffer, size_t stNBytes, sint flags);
		virtual ssize_t writeFileExt(FS::VFSFile* node, c8* pcBuffer, size_t stNBytes, off_t oftPos, sint flags);

		virtual sint ioctl(FS::VFSNode* node, sint iRequest, slong lArg);
	};
}

#endif

