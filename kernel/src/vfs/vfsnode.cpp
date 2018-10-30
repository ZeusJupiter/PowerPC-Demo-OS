/*
 *   File name: vfsnode.cpp
 *
 *  Created on: 2017年5月8日, 下午3:30:41
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <string.h>
#include <atomic.h>
#include <lockguard.h>
#include <device.h>

#include <utime.h>

#include <sys/stat.h>

#include <mm/heap.h>
#include <vfs/vfs.h>
#include <vfs/vfsdevice.h>
#include <vfs/vfsdir.h>
#include <vfs/vfsfile.h>
#include <vfs/vfslink.h>

namespace FS
{
	void VFSNode::init(pid_t ower, VFSDir* parent, char* name, uint mode)
	{
		_name = name;
		_nameLen = strlen(name);
		_ower = ower;
		_mode = mode;
		_refCount = 1;

		_needReleased = true;

		_modTime = _accTime = _crtTime = 0;

		_nodeSpinlock.init(0);

		_dev = nullptr;

		_parent = parent;
		if(_parent)
		{
			_parent->addChild(this);
		}
	}

	bool VFSNode::isDeletable(void)const
	{

		return _ower != KERNEL_PID;
	}

	void VFSNode::getStat(pid_t pid, struct stat *info)
	{
		info->st_size = 0;
		info->st_blksize = 0;
		info->st_blocks = 0;

		if(_nodeType == VFSNodeType::Device)
		{
			info->st_dev = _dev->_devId;
		}
		else
		{
			info->st_dev = VFS_DEV_NO;
			info->st_size = getSize(pid);
			info->st_blksize = 512;
			info->st_blocks = info->st_size / 512;
		}

		info->st_atime = _accTime;
		info->st_mtime = _modTime;
		info->st_ctime = _crtTime;
		info->st_ino = getNodeId();
		info->st_nlink = 1;
		info->st_uid = _uid;
		info->st_gid = _gid;
		info->st_mode = _mode;
	}

	ErrNo VFSNode::getPathTo(char *dst, uint size)const
	{
		uint nlen, len = 0, total = 0;
		const VFSNode* n = this;

		while(n->_parent != nullptr)
		{
			total += n->_nameLen + 1;
			n = n->_parent;
		}

		if(total > size)
		{
			return ErrNo::ENAMETOOLONG;
		}

		n = this;
		len = total;
		while(n->_parent != nullptr)
		{
			nlen = n->_nameLen + 1;
			*(dst + total - nlen) = '/';

			memcpy(dst + total + 1 - nlen, n->_name, nlen - 1);
			total -= nlen;
			n = n->_parent;
		}
		*(dst + len) = '\0';

		return ErrNo::ENONE;
	}

	ErrNo VFSNode::chmod(pid_t pid, mode_t mode)
	{
		ErrNo res = ErrNo::ENONE;

		if(ErrNo::ENONE == res)
			_mode = (_mode & ~MODE_PERM) | (mode & MODE_PERM);

		return res;
	}

	ErrNo VFSNode::chown(pid_t pid, uid_t uid, gid_t gid)
	{
		ErrNo res = ErrNo::ENONE;

		if(ErrNo::ENONE == res)
		{
			if(uid != (uid_t)-1) _uid = uid;
			if(gid != (gid_t)-1) _gid = gid;
		}
		return res;
	}

	ErrNo VFSNode::utime(pid_t pid, const struct utimbuf *utime)
	{
		ErrNo res = ErrNo::ENONE;

		if(ErrNo::ENONE == res)
		{
			_modTime = utime->modtime;
			_accTime = utime->actime;
		}

		return res;
	}

	ErrNo VFSNode::link(pid_t pid, VFSDir* dir, const char* name)
	{
		if(_nodeType == VFSNodeType::Directory) return ErrNo::EISDIR;

		uint nameLen = strlen(name);
		if(dir->findFileInDir(name, nameLen) != nullptr)
		{
			return ErrNo::EEXIST;
		}

		char* namecpy = (char*)kheap.allocate(nameLen + 1);
		if(namecpy == nullptr)
			return ErrNo::ENOMEM;

		memcpy(namecpy, name, nameLen + 1);

		if(nullptr == VFS::createObjNode<VFSLink>(pid, dir, namecpy, this))
		{
			kheap.free(namecpy);
			return ErrNo::ENOMEM;
		}

		return ErrNo::ENONE;
	}

	ErrNo VFSNode::unlink(pid_t pid, const char* name)
	{
		if(_nodeType != VFSNodeType::Directory) return ErrNo::ENOTDIR;

		VFS::lockTree();
		VFSNode* n = const_cast<VFSNode*>(static_cast<VFSDir*>(this)->findFileInDir(name, strlen(name), false));
		if(!n || !canRemove(pid, n))
		{
			VFS::unlockTree();
			return n ? ErrNo::EPERM : ErrNo::ENOENT;
		}

		n->ref();
		VFS::unlockTree();

		n->destroy();
		n->unref();

		return ErrNo::ENONE;
	}

	ErrNo VFSNode::mkdir(pid_t pid, const char *name,mode_t mode)
	{

		assert(name != nullptr);

		if(VFSNodeType::Directory != _nodeType) return ErrNo::ENOTDIR;

		uint nameLen = strlen(name);

		if(static_cast<VFSDir*>(this)->findFileInDir(name, nameLen) != NULL)
		{
			return ErrNo::EEXIST;
		}

		char* namecpy = (char*)kheap.allocate(nameLen + 1);
		if(namecpy == nullptr)
			return ErrNo::ENOMEM;

		memcpy(namecpy, name, nameLen + 1);

		if(VFS::createObjNode<VFSDir>(pid, static_cast<VFSDir*>(this), namecpy, S_IFDIR | (mode & MODE_PERM)) == nullptr)
		{
			kheap.free(namecpy);
			return ErrNo::ENOMEM;
		}

		return ErrNo::ENONE;
	}

	ErrNo VFSNode::rmdir(pid_t pid,const char *name)
	{
		assert(name != nullptr);

		if(VFSNodeType::Directory != _nodeType) return ErrNo::ENOTDIR;

		VFS::lockTree();
		VFSNode* dir = const_cast<VFSNode*>(static_cast<VFSDir*>(this)->findFileInDir(name, strlen(name), false));
		if(dir == nullptr)
		{
			VFS::unlockTree();
			return ErrNo::ENOENT;
		}

		dir->ref();
		VFS::unlockTree();

		ErrNo res = ErrNo::ENONE;

		if(VFSNodeType::Directory !=  dir->_nodeType)
		{
			res = ErrNo::ENOTDIR;
			goto error;
		}

		if(dir->_ower == KERNEL_PID || !canRemove(pid, dir))
		{
			res = ErrNo::EPERM;
			goto error;
		}

		if(static_cast<VFSDir*>(dir)->isEmptyDir())
			dir->destroy();
		else
			res = ErrNo::ENOTEMPTY;

	error:
		dir->unref();
		return res;
	}

	ErrNo VFSNode::symlink(pid_t pid, const char* name, const char* target)
	{
		if(VFSNodeType::Directory != _nodeType) return ErrNo::ENOTDIR;

		if(static_cast<VFSDir*>(this)->findFileInDir(name, strlen(name)) != nullptr)
			return ErrNo::EEXIST;

		VFSNode* symlink;
		ErrNo res = createFile(pid, name, static_cast<VFSDir*>(this), &symlink, VFS_SYMLINK, LNK_DEF_MODE);
		if(res > ErrNo::ENONE)
			return res;

		symlink->write(pid, target, 0, strlen(target));

		return res;
	}

	ErrNo VFSNode::createdev(pid_t pid, const char* name, mode_t mode, VFSDevice** node, BaseDevice* phyDevice, bool needCopy)
	{
		assert(name!=nullptr);
		if(VFSNodeType::Directory != _nodeType) return ErrNo::ENOTDIR;

		uint nameLen = strlen(name);

		if(static_cast<VFSDir*>(this)->findFileInDir(name, nameLen) != nullptr)
		{
			return ErrNo::EEXIST;
		}

		char* namecpy = const_cast<char*>(name);
		if(needCopy)
		{
			namecpy = (char*)kheap.allocate(nameLen + 1);
			if(namecpy == nullptr)
				return ErrNo::ENOMEM;

			memcpy(namecpy, name, nameLen + 1);
		}

		*node = VFS::createObjNode<VFSDevice>(pid, static_cast<VFSDir*>(this), namecpy, mode);
		if(*node == nullptr)
		{
			kheap.free(namecpy);
			return ErrNo::ENOMEM;
		}

		(*node)->_needReleased = needCopy;

		(*node)->_dev = phyDevice;
		phyDevice->_node = *node;

		return ErrNo::ENONE;
	}

	void VFSNode::ref(void)
	{
		Atomic::add(&_refCount, 1);
	}

	void VFSNode::unref(void)
	{
		Atomic::add(&_refCount, -1);
		doUnref(false);
	}

	void VFSNode::destroy(void)
	{
		LockGuard<Spinlock> g(_nodeSpinlock);
		_dev = nullptr;
		doUnref(true);
	}

	sint VFSNode::open(pid_t pid, const char* path, sint *sympos, ino_t root, uint flags, mode_t mode)
	{}

	sint VFSNode::getSize(pid_t pid)const
	{
		switch(nodeType())
		{
		case VFSNodeType::Directory:
			return static_cast<const VFSDir*>(this)->getSize(pid);

		case VFSNodeType::HardLink:
			return static_cast<const VFSLink*>(this)->getSize(pid);

		case VFSNodeType::SymLink:
		case VFSNodeType::File:
			return static_cast<const VFSFile*>(this)->getSize();

		case VFSNodeType::Device:
			return static_cast<const VFSDevice*>(this)->getSize(pid);

		case VFSNodeType::Invalid:
		default:
			return 0;
		}
	}

	sint VFSNode::read(pid_t pid, void *buffer, off_t offset, uint count)
	{}

	sint VFSNode::write(pid_t pid, const void* buffer, off_t offset, uint count)
	{}

	off_t VFSNode::seek(pid_t pid, off_t position, off_t offset, uint whence) const
	{}

	int VFSNode::truncate(pid_t pid, off_t lenth)
	{}

	void VFSNode::close(pid_t pid )
	{}

	ErrNo VFSNode::createFile(pid_t pid, const char* fileName, VFSDir* dir, VFSNode** child, uint flags, mode_t mode, bool needCopy)
	{
		uint nameLen = strlen(fileName);

		char* namecpy = const_cast<char*>(fileName);
		if(needCopy)
		{
			namecpy = (char*)kheap.allocate(nameLen + 1);
			if(nullptr == namecpy)
				return ErrNo::ENOMEM;
			memcpy(namecpy, fileName, nameLen + 1);
		}

		if(flags & VFS_SYMLINK)
			*child = VFS::createObjNode<VFSFile>(pid, dir, namecpy, S_IFLNK | LNK_DEF_MODE);
		else
			*child = VFS::createObjNode<VFSFile>(pid, dir, namecpy, S_IFREG | (mode & MODE_PERM));

		if(*child == nullptr)
		{
			kheap.free(namecpy);
			return ErrNo::ENOMEM;
		}

		(*child)->_needReleased = needCopy;

		return ErrNo::ENONE;
	}

	bool VFSNode::canRemove(pid_t pid, const VFSNode* node)const
	{
		if(pid == KERNEL_PID) return true;
		if(!node->isDeletable()) return false;

		return true;
	}

	void VFSNode::detach(bool force)
	{
		if(_refCount == 0 || force)
		{
			_parent->removeChild(this);
			_parent = nullptr;
			invalidate();
			if(_needReleased)
			{
				kheap.free(const_cast<char*>(_name));
				_name = nullptr;
			}
		}
	}

	void VFSNode::doUnref(bool force)
	{
		if(force)
			_refCount = 0;

		if(_refCount == 0)
		{
			if(VFSNodeType::Directory == _nodeType)
			{
				static_cast<VFSDir*>(this)->removeChildren();
			}

			detach(force);

			VFS::destroyObjNode(this);
		}
	}
}
