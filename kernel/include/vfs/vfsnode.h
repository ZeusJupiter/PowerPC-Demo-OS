/*
 *   File name: vfsnode.h
 *
 *  Created on: 2017年5月7日, 下午2:28:43
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_VFS_VFSNODE_H__
#define __INCLUDE_VFS_VFSNODE_H__

#include <mk/oscfg.h>
#include <list.h>
#include <spinlock.h>
#include <devtype.h>
#include <errno.h>
#include "vfsnodetype.h"

#define MODE_TYPE_CHANNEL	0x0010000
#define MODE_TYPE_HARDLINK	0x0020000
#define MODE_TYPE_DEVMASK	0x0700000
#define MODE_TYPE_BLKDEV	(0x0100000 | S_IFBLK)
#define MODE_TYPE_CHARDEV	(0x0200000 | S_IFCHR)
#define MODE_TYPE_FSDEV		(0x0300000 | S_IFFS)
#define MODE_TYPE_FILEDEV	(0x0400000 | S_IFREG)
#define MODE_TYPE_SERVDEV	(0x0500000 | S_IFSERV)

#define VFS_DEV_NO					0

#define IS_DEVICE(mode)				(((mode) & MODE_TYPE_DEVMASK) != 0)
#define IS_CHANNEL(mode)			(((mode) & MODE_TYPE_CHANNEL) != 0)
#define IS_FS(mode)					(((mode) & MODE_TYPE_DEVMASK) == 0x0300000)
#define IS_HDLNK(mode)				(((mode) & MODE_TYPE_HARDLINK) != 0)

struct utimbuf;
struct stat;
class BaseDevice;
namespace FS
{
	class VFSManager;
	class VFS;
	class VFSDir;
	class VFSDevice;

	class VFSNode
	{
		friend class VFSManager;
		friend class VFS;
		friend class VFSDir;
	public:
		virtual bool isDeletable(void)const;
		virtual bool isAlive(void)const{ return _name != nullptr && _nodeType != VFSNodeType::Invalid; }

		virtual ino_t getNodeId(void)const{ return _nodeId; }
		virtual const char* getName(void)const{ return _name; }

		virtual uint getMode(void)const{ return _mode; }

		virtual uint getUid(void)const{ return _uid; }
		virtual uint getGid(void)const{ return _gid; }
		virtual pid_t getOwer(void)const{ return _ower; }
		virtual VFSDir* parent(void){ return _parent;  }

		ushort getRefCount(void)const{ return _refCount; }
		VFSNodeType nodeType(void)const{ return _nodeType; }

		void getStat(pid_t pid, struct stat *info);

		const char* getPath(void)const
		{
			static char path[256];
			if(getPathTo(path, sizeof(path)) == ErrNo::ENONE)
				return path;
			else
				return nullptr;
		}

		virtual ErrNo getPathTo(char *dst, uint size)const;
		virtual ErrNo chmod(pid_t pid, mode_t mode);
		virtual ErrNo chown(pid_t pid, uid_t uid, gid_t gid);

		virtual time_t modTime(void)const{ return _modTime;  }
		virtual time_t accTime(void)const{ return _accTime; }

		virtual ErrNo utime(pid_t pid, const struct utimbuf *utime);

		virtual ErrNo link(pid_t pid, VFSDir* dir, const char* name);
		virtual ErrNo unlink(pid_t pid, const char* name);

		virtual ErrNo mkdir(pid_t pid,const char *name,mode_t mode);

		virtual ErrNo rmdir(pid_t pid,const char *name);

		virtual ErrNo symlink(pid_t pid, const char* name, const char* target);

		virtual void ref(void);

		virtual void unref(void);

		virtual void destroy(void);

		virtual sint open(pid_t pid, const char* path, sint *sympos, ino_t root, uint flags, mode_t mode);

		virtual sint getSize(pid_t pid)const;

		virtual sint read(pid_t pid, void *buffer, off_t offset, uint count);

		virtual sint write(pid_t pid, const void* buffer, off_t offset, uint count);

		virtual off_t seek(pid_t pid, off_t position, off_t offset, uint whence)const;

		virtual int truncate(pid_t pid, off_t lenth);

		virtual void close(pid_t pid );

		bool isDir(void)const{ return VFSNodeType::Directory == _nodeType; }
		bool isFile(void)const{ return VFSNodeType::File == _nodeType; }
		bool isSymLink(void)const{ return VFSNodeType::SymLink == _nodeType; }

		sint phyFd(void)const{ return _phyFd; }
		sint setPhyFd(sint pfd){ return _phyFd = pfd; }

		sint openFlags(void)const{ return _openFlags; }
		void setOpenFlags(sint flags){ _openFlags = flags; }

	private:

		void invalidate(){ _nodeType = VFSNodeType::Invalid; }
		static ErrNo createFile(pid_t pid, const char* fileName, VFSDir* dir, VFSNode** child, uint flags, mode_t mode, bool needCopy = true);

		ErrNo createdev(pid_t pid, const char* name, mode_t mode, VFSDevice** node, BaseDevice* phyDevice, bool needCopy = true);

	protected:
		inline explicit VFSNode(void){}
		VFSNode(const VFSNode&) = delete;
		VFSNode& operator=(const VFSNode&) = delete;

		virtual void init(pid_t ower, VFSDir* parent, char* name, uint mode);

		virtual bool canRemove(pid_t pid, const VFSNode* node) const;
		virtual void detach(bool force);
		virtual void doUnref(bool force);

		const char* _name;
		uint _nameLen;

		mutable ushort _refCount;
		pid_t _ower;

		uid_t _uid;
		gid_t _gid;

		struct
		{
			sint  _phyFd : 16;
			sint         : 15;
			bool _needReleased : 1;
		};

		ino_t _nodeId;
		uint _mode;

		VFSDir* _parent;

		VFSNodeType _nodeType;
		mutable Spinlock _nodeSpinlock;

		time_t _crtTime;
		time_t _modTime;
		time_t _accTime;

		BaseDevice *_dev;
		sint _openFlags;

		DoubleList _pList;

		DoubleList _devLinkList;

	};
}

#if !defined(__KERNEL_VFSNODE_ALIAS_)
#define __KERNEL_VFSNODE_ALIAS_ 1
#define FSNode FS::VFSNode
#endif

#endif

