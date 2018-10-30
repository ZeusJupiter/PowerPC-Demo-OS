/*
 *   File name: vfsfile.h
 *
 *  Created on: 2017年5月7日, 下午2:29:31
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_VFS_VFSFILE_H__
#define __INCLUDE_VFS_VFSFILE_H__

#include "vfsnode.h"

namespace FS
{
	class VFSManager;
	class VFS;
	class VFSDir;
	class VFSFile : public VFSNode
	{
		friend class VFSManager;
		friend class VFS;

		explicit VFSFile(void){}
		VFSFile(const VFSFile&) = delete;
		VFSFile& operator=(const VFSFile&) = delete;

		virtual void init(pid_t pid, VFSDir* parent, c8* name, mode_t mode);
		virtual void init(pid_t pid, VFSDir* parent, c8* name, mode_t mode, void* buf, uint bufLen);

	public:

		sint  getSize(void)const;
		off_t seek(pid_t pid, off_t position, off_t offset, uint whence)const;
		sint  read(pid_t pid, void* buffer, off_t offset, uint count);
		sint  write(pid_t pid, const void* buffer, off_t offset, uint count);
		int   truncate(pid_t pid, off_t lenth);

		int reserve(uint newSize);

		off_t filePtr(void)const{ return _offsetPtr; }

		void adjustFilePtr(off_t ofs){ _offsetPtr += ofs; }
		void setFilePtr(off_t ofs){ _offsetPtr = ofs; }

		void adjustSize(off_t ofs){ _fileSize += ofs; }
		void setSize(off_t ofs){ _fileSize = ofs; }

	private:
		int doReserve(uint newSize);

		bool _isDynamic;
		uint _bufSize;

		off_t _fileSize;
		void *_data;

		off_t _offsetPtr;
	};
}

#endif

