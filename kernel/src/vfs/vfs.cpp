/*
 *   File name: vfs.cpp
 *
 *  Created on: 2017年5月9日, 下午10:06:40
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

#include <dirent.h>

#include <sys/stat.h>
#include <device.h>

#include <kernel.h>

#include <driverinterface.h>

#include <vfs/vfs.h>
#include <vfs/vfsdevice.h>
#include <vfs/vfsdir.h>
#include <vfs/vfslink.h>
#include <vfs/stdfilemanager.h>

namespace FS
{
	const c8 VFS::_fileNameIllegalCharStr[] = "\\*?<>:\"|\t\r\n";
	VFSDir* VFS::rootNode;
	VFSDir* VFS::rootUserNode;
	VFSDir* VFS::homeNode;
	VFSDir* VFS::tmpNode;
	VFSDir* VFS::devNode;

	void VFS::init(void)
	{
		VFSManager::init();
		StdFileMng::init();

		rootNode = static_cast<VFSDir*>(VFSManager::allocate());
		rootUserNode = static_cast<VFSDir*>(VFSManager::allocate());
		homeNode = static_cast<VFSDir*>(VFSManager::allocate());
		devNode = static_cast<VFSDir*>(VFSManager::allocate());
		tmpNode = static_cast<VFSDir*>(VFSManager::allocate());

		VFSManager::verify();

		rootNode->init(KERNEL_PID, nullptr, (char*) "", DIR_DEF_MODE);
		rootUserNode->init(KERNEL_PID, rootNode, (char*) "root", DIR_DEF_MODE);
		homeNode->init(KERNEL_PID, rootNode, (char*) "home", DIR_DEF_MODE);
		devNode->init(KERNEL_PID, rootNode, (char*) "dev", DIR_DEF_MODE);
		tmpNode->init(KERNEL_PID, rootNode, (char*) "tmp",
				S_IFDIR | S_ISSTICKY | 0777);

		kdebugln("VFS has been initialized");
	}

	bool VFS::isValid(ino_t nodeId)
	{
		return nodeId < (200);
	}

	char* VFS::absolutePath(char* dst, uint dstSize, const char* relativePath)
	{
		assert(dstSize > 0);

		KThread* curThread = KCommKernel::curThread();
		uint n = sstrncpy(dst, curThread->cwd(), dstSize);

		if(n >= dstSize) return nullptr;

		dstSize -= n;

		strncat(dst, relativePath, dstSize);

		return dst;
	}

	BaseDevice* VFS::findDevice(const c8* name)
	{
		assert(name != nullptr && *name != EOS);

		register BaseDevice * baseDev = nullptr;
		uint nameLen = strlen(name);
		c8* subName = baseName(name, nameLen);

		VFS::lockTree();

		register const VFSDevice* node = (VFSDevice*)(devNode->findFileInDir(subName, nameLen, false));
		if(node)
		{
			baseDev = node->_dev;
		}

		VFS::unlockTree();

		return (baseDev);
	}

#define isSeparator(ch) ((ch) == PATH_SEPARATOR || ch == '\\')
#define isComponentEnd(ch) ((ch) == EOS || isSeparator(ch))

	void VFS::compactPath(c8* absolutPathName)
	{
		if ((absolutPathName == nullptr) || (*absolutPathName == EOS) || (*absolutPathName != PATH_SEPARATOR))
		{
			return;
		}

		char *erasePoint;
		char *startPoint;
		char *in;
		char *out;
		char c;
		bool checkDot;

		in = out = absolutPathName;
		in++;
		out++;

		erasePoint = startPoint = out;
		checkDot = true;

		while ((c = *in++) != EOS)
		{
			if (isSeparator(c))
			{
				checkDot = true;
				continue;
			}

			if (checkDot)
			{
				if (c == '.')
				{
					if (isComponentEnd(*in))
					{
						continue;
					}

					if (*in == '.' && isComponentEnd(in[1]))
					{
						in++;

						if (out > erasePoint)
						{
							do
								out--;
							while (out > erasePoint && *out != PATH_SEPARATOR);
							continue;
						}

					}
				}
				if (out > startPoint)
					*out++ = PATH_SEPARATOR;
				checkDot = false;
			}
			*out++ = c;
		}
		*out = EOS;
	}

#undef isSeparator
#undef isComponentEnd

	void VFS::dirName(const char* path, uint len)
	{
		char *p = const_cast<char*>(path + len - 1);

		while (*p == PATH_SEPARATOR)
		{
			p--;
			len--;
		}

		if (len == 0)
			return;

		while (len > 0 && *p != PATH_SEPARATOR)
		{
			p--;
			len--;
		}

		*(p) = EOS;
	}

	ErrNo VFS::mkdir(const char* dirName, mode_t mode)
	{}

	ErrNo VFS::rmdir(const char* dirName)
	{}

	int VFS::dirfd(const struct DIR* pdir)
	{}

	struct DIR *VFS::opendir(const char* dirName)
	{}

	void VFS::closedir(struct DIR* dir)
	{}

	void VFS::rewinddir(struct DIR* dir)
	{}

	char* VFS::baseName(const char* path, uint len)
	{
		char* p = const_cast<char*>(path + len - 1);
		while (*p == PATH_SEPARATOR)
		{
			--p;
			--len;
		}
		*(p + 1) = EOS;

		if ( len != 0 && (p = strrchr(path, PATH_SEPARATOR)) == nullptr )
			return (char*) path;
		return p + 1;
	}

	ino_t VFS::openfile(User* user, const char* fileName, mode_t mode)
	{}

	void VFS::closefile(ino_t fd)
	{}

	int VFS::stat(ino_t fd, struct stat* info)
	{}

	int VFS::link(pid_t user, ino_t dst, ino_t src, const char*)
	{}

	int VFS::unlink(pid_t user, ino_t, const char*)
	{}

	int VFS::symlink(pid_t user, const char* dst, const char* src)
	{}

	ErrNo VFS::rename(pid_t user, const char* newfullPathName, const char* oldName, VFSDir* oldDir)
	{
		c8* newName = baseName(newfullPathName, strlen(newfullPathName));
		*(newName - 1) = EOS;

		VFSDir* newDir = const_cast<VFSDir*>(findSubDirFromCurDir(rootNode, newfullPathName, false));

		if(newDir == nullptr) return ErrNo::ENOENT;

		VFSNode* renamedNode = oldDir->renameChild(user, oldName, newDir, newName);
		if(renamedNode)
		{
	

		}
	}

	int VFS::chmod(pid_t user, ino_t, mode_t)
	{}

	int VFS::chown(pid_t user, ino_t, uid_t uid, gid_t gid)
	{}

	ErrNo VFS::utime(pid_t user, const char* fileName, const struct utimebuf *timeNew)
	{}

	ErrNo VFS::truncate(pid_t user, ino_t, off_t)
	{}

	void VFS::sync(void)
	{}

	bool VFS::fileNameIsLegal(const c8* fileName)
	{
		if (fileName == nullptr || *fileName == EOS)
		{
			return (false);
		}

	    register const c8*  pcTemp;

	    pcTemp = strrchr(fileName, PATH_SEPARATOR);
		if (pcTemp)
		{
			pcTemp++;
			if (*pcTemp == EOS)
			{
				return (false);
			}

			if (*pcTemp == '.')
			{
				if ( (*(pcTemp + 1) == EOS) ||
					 (*(pcTemp + 1) == '.' && *(pcTemp + 2) == EOS) )
				{
					return false;
				}
			}
		}

	    pcTemp = fileName;
		for (; *pcTemp != EOS; pcTemp++)
		{
			if (strchr(_fileNameIllegalCharStr, *pcTemp))
			{
				return (false);
			}
		}

	    return  (true);
	}

	void VFS::print(ino_t fd)
	{}

	void VFS::destroyObjNode(VFSNode* node)
	{
		VFSManager::deallocate(static_cast<VFSFile*>(node));
	}

	ErrNo VFS::addDev(BaseDevice* dev, const c8* name, bool needCopy)
	{
		VFSDevice* newDev = nullptr;
		ErrNo ret = devNode->createdev(devNode->_ower, name, MODE_PERM, &newDev, dev, needCopy);

		if (ret != ErrNo::ENONE)
			return ret;

		return ret;
	}

	ErrNo VFS::deleteDev(const c8* name)
	{
		if(name == nullptr) return ErrNo::EINVAL;

		u32 nameLen = strlen(name);
		VFSDevice* vDev = (VFSDevice*)(devNode->findFileInDir(name, nameLen));

		if(!vDev)
		{
			return ErrNo::ENODEV;
		}

		if(vDev->_dev->_driver->_release)
			(vDev->_dev->*(vDev->_dev->_driver->_release))();

		vDev->destroy();

		return ErrNo::ENONE;
	}

	VFSNode* VFS::getNodeById(ino_t nodeId)
	{
		VFSNode* ret = nullptr;

		if(VFS::isValid(nodeId))
		{
			ret = VFSManager::fsDescTable[nodeId]._node;
		}

		return ret;
	}

	const VFSDir* VFS::findSubDirFromCurDir(const VFSDir* curDir, const char* subDirPathName, bool lockNeeded)
	{
		if(*subDirPathName == PATH_SEPARATOR) ++subDirPathName;
		while(*subDirPathName != EOS && curDir)
		{
			c8* subName = strchr(subDirPathName, PATH_SEPARATOR);
			if(subName != nullptr)
			{
				curDir = curDir->findDirInDir(subDirPathName, subName - subDirPathName , lockNeeded);
				subDirPathName = subName + 1;
			}
			else
			{
				curDir = curDir->findDirInDir(subDirPathName, strlen(subDirPathName) , lockNeeded);
				return curDir;
			}
		}
		return nullptr;
	}
}

#undef EOS
