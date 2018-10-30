/*
 *   File name: yaffs2.cpp
 *
 *  Created on: 2017年6月22日, 下午6:55:13
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <lockguard.h>

#include <ioctlset.h>

#include <utime.h>

#include <string.h>
#include <stdio.h>

#include <mk/oscfg.h>

#include <mm/heap.h>
#include <vfs/vfs.h>
#include <vfs/vfsnode.h>
#include <vfs/vfsfile.h>
#include <drivermanager.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include "yaffs2.h"

#include "_private/direct/yaffsfs.h"
#include "_private/yaffs_trace.h"
#include "_private/yaffs_guts.h"
#include "_private/yaffs_nand.h"

#define ROOT_STRING "/"

#define IS_ROOT_PATH(devName) ((devName[0] == EOS) || (strcmp(ROOT_STRING, devName) == 0))
#define IS_YVOL_PAATH(devName) (strrchr(devName, PATH_SEPARATOR) == devName)

EXTERN_C void __setError(ErrNo err);

namespace YaffsFS
{
	sint YaffsDev::_yaffsDevDrvIndex;
	sint  YaffsDev::_isCreatedYaffsDev;
	Mutex YaffsDev::_yaffsDrvMutex;

	void YaffsDev::closefile(FS::VFSNode* node)
	{
		assert(node != nullptr);
		VFSNodeType nodetype = node->nodeType();
		if ((nodetype == VFSNodeType::File) || (nodetype == VFSNodeType::Directory))
		{
			yaffs_close(node->phyFd());
		}
	}

	ErrNo  YaffsDev::devClose ()
	{
		LockGuard<Mutex> g(_yaffsDrvMutex);
		return (ErrNo::ENONE);
	}

	ssize_t YaffsDev::readFile(FS::VFSFile* node, c8* pcBuffer, size_t stMaxBytes)
	{
		ssize_t readSize = 0;
		{
			LockGuard<Mutex> g(_yaffsDrvMutex);

			readSize = (ssize_t) yaffs_pread(node->phyFd(), pcBuffer,
					(unsigned int) stMaxBytes, node->filePtr());

			node->adjustFilePtr(readSize);
		}

		return (readSize);
	}

	ssize_t YaffsDev::readFileExt(FS::VFSFile* node, c8* pcBuffer, size_t stMaxBytes,
			off_t oftPos)
	{
		assert(node != nullptr);

		ssize_t       readSize = 0;
		LockGuard<Mutex> g(_yaffsDrvMutex);
		readSize = (ssize_t) yaffs_pread(node->phyFd(), (pvoid) pcBuffer,
			(unsigned int) stMaxBytes, oftPos);

		return  (readSize);
	}

	ssize_t YaffsDev::writeFile(FS::VFSFile* node, c8* pcBuffer, size_t stNBytes, sint flags)
	{
		assert(node != nullptr);

		ssize_t sstWriteNum = 0;

		{
			LockGuard<Mutex> g(_yaffsDrvMutex);

			if (flags & O_APPEND)
			{
				node->setFilePtr(node->getSize());
			}

			sstWriteNum = (ssize_t) yaffs_pwrite(node->phyFd(),
					(pvoid) pcBuffer, (unsigned int) stNBytes, node->filePtr());
			if (sstWriteNum > 0)
			{
				node->adjustFilePtr(sstWriteNum);
				node->adjustSize(sstWriteNum);
			}
		}

		if (sstWriteNum >= 0)
		{
			if (flags & O_SYNC)
			{
				flush(node);
			}
			else if (flags & O_DSYNC)
			{
				dataSync(node);
			}
		}

		return  (sstWriteNum);
	}

	ssize_t YaffsDev::writeFileExt(FS::VFSFile* node, c8* pcBuffer, size_t stNBytes,
			off_t oftPos, sint flags)
	{
		ssize_t writeSize = 0;

		{
			LockGuard<Mutex> g(_yaffsDrvMutex);
			writeSize = (ssize_t) yaffs_pwrite(node->phyFd(),
					(pvoid) pcBuffer, (unsigned int) stNBytes, oftPos);
			if (writeSize > 0)
			{
				node->adjustSize(writeSize);
			}
		}

		if (writeSize >= 0)
		{
			if (flags & O_SYNC)
			{
				flush(node);
			}
			else if (flags & O_DSYNC)
			{
				dataSync(node);
			}
		}

		return  (writeSize);
	}

	ErrNo YaffsDev::seek(FS::VFSFile* node, off_t oftOffset)
	{
		ErrNo ret = ErrNo::ENONE;

		{
			LockGuard<Mutex> g(_yaffsDrvMutex);
			off_t oftRet = yaffs_lseek(node->phyFd(), oftOffset, SEEK_SET);
			if (oftRet != oftOffset)
			{
				ret = ErrNo::EIO;
			}
			else
			{
				node->setFilePtr(oftOffset);
			}
		}

		return  (ret);
	}

	ErrNo YaffsDev::where(FS::VFSFile* node, off_t *poftPos)
	{

		assert(poftPos != nullptr);

		{
			LockGuard<Mutex> g(_yaffsDrvMutex);
			*poftPos = node->filePtr();
		}

		return ErrNo::ENONE;
	}

	ErrNo  YaffsDev::nRead (FS::VFSFile* node, sint  *piPos)
	{
		assert(piPos != nullptr);

		{
			LockGuard<Mutex> g(_yaffsDrvMutex);
			*piPos = (sint)(node->getSize() - node->filePtr());
		}

		return  ErrNo::ENONE;
	}

	ErrNo  YaffsDev::nRead64 (FS::VFSFile* node, off_t  *poftPos)
	{
		assert(poftPos != nullptr);

		{
			LockGuard<Mutex> g(_yaffsDrvMutex);
			*poftPos = (sint)(node->getSize() - node->filePtr());
		}

		return  ErrNo::ENONE;
	}

	sint  YaffsDev::flush (FS::VFSNode* node)
	{
		sint ret = static_cast<sint>(ErrNo::ENONE);

		{
			LockGuard<Mutex> g(_yaffsDrvMutex);
			if (node->nodeType() == VFSNodeType::File)
			{
				ret = yaffs_fsync(node->phyFd());
			}
			else
			{
				yaffs_sync(node->getName());
			}
		}

		return  (ret);
	}

	sint  YaffsDev::dataSync (FS::VFSNode* node)
	{
		sint ret;

		{
			LockGuard<Mutex> g(_yaffsDrvMutex);
			if (node->nodeType() == VFSNodeType::File)
			{
				ret = yaffs_fdatasync(node->phyFd());
			}
			else
			{
				yaffs_sync(node->getName());
				ret = static_cast<sint>(ErrNo::ENONE);
			}
		}

		return  (ret);
	}

	ErrNo  YaffsDev::format (void)
	{
		struct yaffs_dev *pyaffsDev;

		_yaffsDrvMutex.lock();

		if (pyaffsDev == nullptr)
		{
			_yaffsDrvMutex.unlock();
			__setError(ErrNo::ENXIO);
			return (ErrNo::ENXIO);
		}

		_yaffsDrvMutex.unlock();

		return  (ErrNo::ENONE);
	}

	ErrNo YaffsDev::yaffsRename(FS::VFSNode* node, const c8* pcNewName)
	{
		assert(node != nullptr);
		assert(pcNewName != nullptr);

		if (IS_ROOT_PATH(pcNewName))
		{
			__setError(ErrNo::ENOENT);
			return (ErrNo::ENOENT);
		}

		register ErrNo ret = ErrNo::ENONE;
		c8 oldFullPathName[256];

		node->getPathTo(oldFullPathName, 256);

		_yaffsDrvMutex.lock();
		VFSNodeType nodetype = node->nodeType();
		if ((nodetype == VFSNodeType::File) || (nodetype == VFSNodeType::Directory))
		{
			if (pcNewName[0] == PATH_SEPARATOR) ++pcNewName;
			if(yaffs_rename(oldFullPathName, pcNewName) != 0) ret = ErrNo::EIO ;
		}
		else
		{
			__setError(ErrNo::ENOSYS);
		}
		_yaffsDrvMutex.unlock();

		return  (ret);
	}

	ErrNo  YaffsDev::getStat (FS::VFSNode* node, struct stat *statPtr)
	{
		assert(statPtr != nullptr);

		c8 nodeFullPathName[256];
		node->getPathTo(nodeFullPathName, 256);

		struct yaffs_stat yaffsstat;
		ErrNo ret = ErrNo::ENONE;

		_yaffsDrvMutex.lock();
		if (yaffs_stat(nodeFullPathName, &yaffsstat) == 0)
		{
			statPtr->st_dev = (dev_t) yaffsstat.st_dev;
			statPtr->st_ino = yaffsstat.st_ino;
			statPtr->st_mode = yaffsstat.st_mode;
			statPtr->st_nlink = yaffsstat.st_nlink;
			statPtr->st_uid = (uid_t) yaffsstat.st_uid;
			statPtr->st_gid = (gid_t) yaffsstat.st_gid;
			statPtr->st_rdev = (dev_t) yaffsstat.st_rdev;
			statPtr->st_size = yaffsstat.st_size;
			statPtr->st_blksize = yaffsstat.st_blksize;
			statPtr->st_blocks = yaffsstat.st_blocks;

			statPtr->st_atime = yaffsstat.yst_atime;
			statPtr->st_mtime = yaffsstat.yst_mtime;
			statPtr->st_ctime = yaffsstat.yst_ctime;
		}
		else
		{
			if (IS_ROOT_PATH(nodeFullPathName))
			{
				statPtr->st_dev = (dev_t) this;
				statPtr->st_ino = (ino_t) 0;
				statPtr->st_mode = YAFFS_ROOT_MODE | S_IFDIR;
				statPtr->st_nlink = 1;
				statPtr->st_uid = 0;
				statPtr->st_gid = 0;
				statPtr->st_rdev = 1;
				statPtr->st_size = 0;
				statPtr->st_blksize = 0;
				statPtr->st_blocks = 0;

				statPtr->st_atime = 0;

				statPtr->st_mtime = 0;
				statPtr->st_ctime = 0;

				ret = ErrNo::ENONE;
			}
			else
				ret = ErrNo::EIO;
		}
		_yaffsDrvMutex.unlock();

		return  (ret);
	}

	ErrNo  YaffsDev::lStat (FS::VFSNode* node, c8*  name, struct stat *statPtr)
	{
		assert(name != nullptr);
		assert(statPtr != nullptr);

		ErrNo ret = ErrNo::ENONE;
		struct yaffs_stat yaffsstat;

		_yaffsDrvMutex.lock();
		if (yaffs_lstat(name, &yaffsstat) == 0)
		{
			statPtr->st_dev = (dev_t) yaffsstat.st_dev;
			statPtr->st_ino = yaffsstat.st_ino;
			statPtr->st_mode = yaffsstat.st_mode;
			statPtr->st_nlink = yaffsstat.st_nlink;
			statPtr->st_uid = (uid_t) yaffsstat.st_uid;
			statPtr->st_gid = (gid_t) yaffsstat.st_gid;
			statPtr->st_rdev = (dev_t) yaffsstat.st_rdev;
			statPtr->st_size = yaffsstat.st_size;
			statPtr->st_blksize = yaffsstat.st_blksize;
			statPtr->st_blocks = yaffsstat.st_blocks;

			statPtr->st_atime = yaffsstat.yst_atime;
			statPtr->st_mtime = yaffsstat.yst_mtime;
			statPtr->st_ctime = yaffsstat.yst_ctime;
		}
		else
		{
			if (IS_ROOT_PATH(name))
			{
				statPtr->st_dev = (dev_t) this;
				statPtr->st_ino = (ino_t) 0;
				statPtr->st_mode = YAFFS_ROOT_MODE | S_IFDIR;
				statPtr->st_nlink = 1;
				statPtr->st_uid = 0;
				statPtr->st_gid = 0;
				statPtr->st_rdev = 1;
				statPtr->st_size = 0;
				statPtr->st_blksize = 0;
				statPtr->st_blocks = 0;

				statPtr->st_atime = 0;

				statPtr->st_mtime = 0;
				statPtr->st_ctime = 0;
			}
	        else
	        	ret = ErrNo::EIO;
		}
		_yaffsDrvMutex.unlock();
		return  (ret);
	}

	ErrNo  YaffsDev::getStatfs (FS::VFSNode* node, struct statfs *statfsPtr)
	{
		assert(statfsPtr != nullptr);

		loff_t oftFreeBytes;
		loff_t oftTotalBytes;

		ErrNo ret = ErrNo::ENONE;
		struct yaffs_stat yaffsstat;

		c8 nodeFullPathName[256];
		node->getPathTo(nodeFullPathName, 256);

		_yaffsDrvMutex.lock();
		if ((yaffs_lstat(nodeFullPathName, &yaffsstat) == 0) && (yaffsstat.st_blksize > 0))
		{
			struct yaffs_dev *pyaffsdev;

			oftFreeBytes = yaffs_freespace(nodeFullPathName);
			oftTotalBytes = yaffs_totalspace(nodeFullPathName);

			statfsPtr->f_type = YAFFS_MAGIC;

			statfsPtr->f_bsize = yaffsstat.st_blksize;

			statfsPtr->f_blocks = (long) (oftTotalBytes / yaffsstat.st_blksize);
	

			statfsPtr->f_bfree = (long) (oftFreeBytes / yaffsstat.st_blksize);
			statfsPtr->f_bavail = (long) (oftFreeBytes / yaffsstat.st_blksize);
			statfsPtr->f_files = 0;

			statfsPtr->f_ffree = 0;

			statfsPtr->f_fsid.val[0] = (s32) this;

			statfsPtr->f_fsid.val[1] = 0;

			pyaffsdev = (struct yaffs_dev *) yaffs_getdev(nodeFullPathName);
			if (pyaffsdev && pyaffsdev->read_only)
			{
				statfsPtr->f_flag = SF_RDONLY;
			}
			else
			{
				statfsPtr->f_flag = 0;
			}

			statfsPtr->f_namelen = YAFFS_MAX_NAME_LENGTH;
		}
		else
		{
			ret = ErrNo::EIO;
		}
		_yaffsDrvMutex.unlock();

		return  (ret);
	}

	ErrNo  YaffsDev::chmod (FS::VFSNode* node, sint  iMode)
	{
		ErrNo ret = ErrNo::ENONE;

		VFSNodeType nodetype = node->nodeType();

		if ((nodetype == VFSNodeType::File) || (nodetype == VFSNodeType::Directory))
		{
			_yaffsDrvMutex.lock();
			sint iret = yaffs_chmod(node->getName(), (mode_t) iMode);

			_yaffsDrvMutex.unlock();
			if(iret != 0) ret = ErrNo::EIO;
		}

		return  (ret);
	}

	ErrNo  YaffsDev::readDir (FS::VFSNode* node, struct DIR* dir)
	{
		VFSNodeType nodetype = node->nodeType();
		if (nodetype != VFSNodeType::Directory)
		{
			__setError(ErrNo::ENOTDIR);
			return  (ErrNo::ENOTDIR);
		}

		if (IS_ROOT_PATH(node->getName()))
		{
			return ErrNo::ENONE;
		}

		ErrNo ret = ErrNo::ENONE;
		struct yaffs_dirent *yafdirent = nullptr;

		_yaffsDrvMutex.lock();

		if(dir->dir_pos == 0) dir->dir_eof = false;

		yafdirent = yaffs_readdir_fd(node->phyFd());
		_yaffsDrvMutex.unlock();

		if (yafdirent)
		{
			strcpy(dir->dir_dirent.d_name, yafdirent->d_name);
			dir->dir_pos++;
		}
		else
		{
			ret = ErrNo::EIO;
		}

		return  (ret);
	}

	ErrNo  YaffsDev::setTime (FS::VFSNode* node, struct utimbuf  *utim)
	{
		assert(utim != nullptr);

		register ErrNo ret = ErrNo::ENONE;

		if (node->nodeType() == VFSNodeType::Device)
		{
			ret = ErrNo::ENOSYS;
			__setError(ErrNo::ENOSYS);
		}
		else
		{
			struct yaffs_utimbuf yafutim;
			_yaffsDrvMutex.lock();
			yafutim.actime = (unsigned long) utim->actime;
			yafutim.modtime = (unsigned long) utim->modtime;
			if( yaffs_futime(node->phyFd(), &yafutim) != 0) ret = ErrNo::EIO;
			_yaffsDrvMutex.unlock();
		}

		return  (ret);
	}

	ErrNo  YaffsDev::truncate (FS::VFSFile* node, loff_t  oftSize)
	{
		register ErrNo ret = ErrNo::ENONE;

		_yaffsDrvMutex.lock();
		if (yaffs_ftruncate(node->phyFd(), oftSize) == 0)
		{
			struct yaffs_stat yafstat;
			yaffs_fstat(node->phyFd(), &yafstat);
			node->setSize(yafstat.st_size);
		}
		else
			ret = ErrNo::EIO;
		_yaffsDrvMutex.unlock();

		return (ret);
	}

	ErrNo YaffsDev::createSymlink (const c8*  createdLinkName, const c8*  linkDstName)
	{
		register ErrNo ret = ErrNo::ENONE;

		_yaffsDrvMutex.lock();
		if (yaffs_symlink(linkDstName, createdLinkName) != 0)
			ret = ErrNo::EIO;
		_yaffsDrvMutex.unlock();

		return  (ret);
	}

	ssize_t YaffsDev::readSymlink(c8* rawFileName, c8* dstLinkName, size_t bufferSize)
	{
		register sint err;

		_yaffsDrvMutex.lock();
		err = yaffs_readlink(rawFileName, dstLinkName, bufferSize);
		_yaffsDrvMutex.unlock();

		if (err == 0)
		{
			return ((ssize_t) strnlen(dstLinkName, bufferSize));
		}
		else
		{
			return ((ssize_t) err);
		}
	}

	sint YaffsDev::open(const c8* fileName, sint accFlags, sint creationMode)
	{
		assert(fileName != nullptr);

		if ((accFlags & O_CREAT))
			if (S_ISFIFO(creationMode) || S_ISBLK(creationMode) || S_ISCHR(creationMode))
			{
				__setError(ErrNo::ENOSYS);
				return -static_cast<sint>(ErrNo::ENOSYS);
			}

		_yaffsDrvMutex.lock();

		if (S_ISDIR(creationMode))
		{
			if ((yaffs_mkdir(fileName, creationMode) < 0) && (accFlags & O_EXCL))
			{
				_yaffsDrvMutex.unlock();
				return -static_cast<sint>(ErrNo::EIO);
			}
		}

		sint yaffsFd = yaffs_open(fileName, accFlags, creationMode);

		if (yaffsFd < 0)
		{
			_yaffsDrvMutex.unlock();
			return -static_cast<sint>(ErrNo::EIO);
		}

		_yaffsDrvMutex.unlock();

		return (yaffsFd);
	}

	ErrNo YaffsDev::remove(const c8* fileName)
	{
		assert(fileName != nullptr);

		register ErrNo ret = ErrNo::ENONE;

		{
			_yaffsDrvMutex.lock();

			if (IS_ROOT_PATH(fileName))
			{
				if (!_isForceDelete)
				{

					ret = ErrNo::EBUSY;
				}

                _yaffsDrvMutex.unlock();

				return ret;

			}
			else if (IS_YVOL_PAATH(fileName))
			{
				if (yaffs_unmount(fileName) != 0)
				{
					ret = ErrNo::EIO;
				}

				_yaffsDrvMutex.unlock();
				return (ret);
			}
			else
			{
				struct yaffs_stat yaffsstat;

				if (yaffs_lstat(fileName, &yaffsstat) == 0)

				{
					sint yret = -1;
					if (S_ISDIR(yaffsstat.st_mode))
					{
						yret = yaffs_rmdir(fileName);
					}
					else
					{
						yret = yaffs_unlink(fileName);
					}
					if(yret < 0)
						ret = ErrNo::EIO;
				}
				else
					ret = ErrNo::EIO;

				_yaffsDrvMutex.unlock();

				return (ret);
			}
		}
	}

	sint YaffsDev::ioctl(FS::VFSNode* node, sint iRequest, slong lArg)
	{
		off_t oftTemp;

		IoctlSet request = static_cast<IoctlSet>(iRequest);

		switch (request)
		{
		case IoctlSet::FIOCONTIG:
		case IoctlSet::FIOTRUNC:
		case IoctlSet::FIOLABELSET:
		case IoctlSet::FIOATTRIBSET:
		case IoctlSet::FIOSQUEEZE:
			if ((node->openFlags() & O_ACCMODE) == O_RDONLY)
			{
				__setError(ErrNo::EWRPROTECT);
				return  -static_cast<sint>(ErrNo::EWRPROTECT);
			}
		}
		switch (request)
		{

		case IoctlSet::FIODISKFORMAT:

			return  -static_cast<sint>(format());

		case IoctlSet::FIODISKINIT:

			return  static_cast<sint>(ErrNo::ENONE);

		case IoctlSet::FIOSEEK:

			if(node->nodeType() == VFSNodeType::File)
			{
				oftTemp = *(off_t *)lArg;
				return  -static_cast<sint>(seek(static_cast<FS::VFSFile*>(node), oftTemp));
			}

			return -static_cast<sint>(ErrNo::EISDIR);

		case IoctlSet::FIOWHERE:

			if(node->nodeType() == VFSNodeType::File)
			{
				ErrNo ret = where(static_cast<FS::VFSFile*>(node), &oftTemp);
				if (ret == ErrNo::ENONE) {
					*(off_t *)lArg = oftTemp;
					return  static_cast<sint>(ErrNo::ENONE);
				} else {
					return  -static_cast<sint>(ret);
				}
			}

			return -static_cast<sint>(ErrNo::EISDIR);

		case IoctlSet::FIONREAD:
			if(node->nodeType() == VFSNodeType::File)
			{
				return  -static_cast<sint>(nRead(static_cast<FS::VFSFile*>(node), (sint *)lArg));
			}

			return -static_cast<sint>(ErrNo::EISDIR);

		case IoctlSet::FIONREAD64:
			if(node->nodeType() == VFSNodeType::File)
			{
				ErrNo ret = nRead64(static_cast<FS::VFSFile*>(node), &oftTemp);
				if (ret == ErrNo::ENONE) {
					*(off_t *)lArg = oftTemp;
					return  static_cast<sint>(ErrNo::ENONE);
				} else {
					return  -static_cast<sint>(ret);
				}
			}
			return -static_cast<sint>(ErrNo::EISDIR);

		case IoctlSet::FIORENAME:

			return  -static_cast<sint>(yaffsRename(node, (c8*)lArg));

		case IoctlSet::FIOLABELGET:

		case IoctlSet::FIOLABELSET:

			__setError(ErrNo::ENOSYS);
			return  -static_cast<sint>(ErrNo::ENOSYS);

		case IoctlSet::FIOFSTATGET:

			return  -static_cast<sint>(getStat(node, (struct stat *)lArg));

		case IoctlSet::FIOFSTATFSGET:

			return  -static_cast<sint>(getStatfs(node, (struct statfs *)lArg));

		case IoctlSet::FIOREADDIR:

			if (node->nodeType() == VFSNodeType::Directory)
			{
				return  -static_cast<sint>(readDir(node, (struct DIR *)lArg));
			}

			return  -static_cast<sint>(ErrNo::ENOTDIR);

		case IoctlSet::FIOTIMESET:
			if (node->nodeType() == VFSNodeType::Device)
			{
				return -static_cast<sint>(ErrNo::ENOSYS);
			}
			return  -static_cast<sint>(setTime(node, (struct utimbuf *)lArg));

		case IoctlSet::FIOTRUNC:
			if(node->nodeType() == VFSNodeType::File)
			{
				oftTemp = *(off_t *)lArg;
				return  -static_cast<sint>(truncate(static_cast<FS::VFSFile*>(node), oftTemp));
			}
			return -static_cast<sint>(ErrNo::EISDIR);

		case IoctlSet::FIOSYNC:
		case IoctlSet::FIOFLUSH:
			return  -static_cast<sint>(flush(node));

		case IoctlSet::FIODATASYNC:
			return  -static_cast<sint>(dataSync(node));

		case IoctlSet::FIOCHMOD:
			return  -static_cast<sint>(chmod(node, (sint)lArg));

		case IoctlSet::FIOFSTYPE:
			*(c8**)lArg = "YAFFS FileSystem";
			return  static_cast<sint>(ErrNo::ENONE);

		case IoctlSet::FIOGETFORCEDEL:
			*(bool*)lArg = _isForceDelete;
			return  static_cast<sint>(ErrNo::ENONE);

		default:
			return  -static_cast<sint>(ErrNo::ENOSYS);
		}
	}

	void YaffsDev::initDev(void)
	{
	    _isForceDelete = false;
	    initRingDoubleList(&_nodeListHeader);
	}
}

