/*
 *   File name: vfs.h
 *
 *  Created on: 2017年5月7日, 下午2:29:10
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_VFS_VFS_H__
#define __INCLUDE_VFS_VFS_H__

#include <mutex.h>
#include <user.h>
#include <errno.h>
#include "vfsmanager.h"

class BaseDevice;

namespace Kernel
{
	class ProcManager;
	namespace Procedure
	{
		class GeneralProc;
		class Thread;
	}
}

namespace FS
{
	const u32 KERNEL_PID = 0;

	enum GFT
	{
		VFS_NOACCESS 	= 0,
		VFS_MSGS 		= 1,
		VFS_EXEC		= 1,

		VFS_WRITE 		= 2,
		VFS_READ 		= 4,
		VFS_CREATE 		= 8,
		VFS_TRUNCATE 	= 16,
		VFS_APPEND 		= 32,
		VFS_NOBLOCK 	= 64,
		VFS_LONELY 		= 128,
		VFS_EXCL 		= 256,
		VFS_NOCHAN 		= 512,
		VFS_NOFOLLOW	= 1024,
		VFS_SYMLINK		= 2048,
		VFS_DEVICE 		= 4096,
		VFS_SIGNALS 	= 8192,
		VFS_BLOCK 		= 16384,
		VFS_NOLINKRES	= 32768,

		VFS_USER_FLAGS	= VFS_MSGS | VFS_WRITE | VFS_READ | VFS_CREATE | VFS_TRUNCATE |
							VFS_APPEND | VFS_NOBLOCK | VFS_LONELY | VFS_EXCL | VFS_NOCHAN |
							VFS_NOFOLLOW,
	};

	class VFSDir;
	class VFSDevice;
	class VFSLink;

	class VFS
	{
	public:
		static const c8 _fileNameIllegalCharStr[];
		static void init(void);
		static bool isValid(ino_t node);

		static void lockTree(void){   return VFSManager::vfsTreeMutex.lock(); }
		static void unlockTree(void){ return VFSManager::vfsTreeMutex.unlock(); }

		static uint getNodeCount(void){ return VFSManager::allocatedCount; }

		static char* absolutePath(c8* dst, uint dstSize, const c8* relativePath);
		static void compactPath(c8* absolutPathName);

		static BaseDevice* findDevice(const c8* devName);

		static void dirName(const char* path, uint len);
		static ErrNo mkdir(const char* dirName, mode_t mode);
		static ErrNo rmdir(const char* dirName);

		static int dirfd(const struct DIR* pdir);
		static struct DIR *opendir(const char* dirName);
		static void closedir(struct DIR* dir);
		static void rewinddir(struct DIR* dir);

		static char* baseName(const char* path, uint len);
		static ino_t openfile(User* user, const char* fileName, mode_t mode);
		static void  closefile(ino_t fd);
		static int   stat(ino_t fd, struct stat* info);

		static int link(pid_t user, ino_t dst, ino_t src, const char* );
		static int unlink(pid_t user, ino_t , const char* );
		static int symlink(pid_t user, const char* dst, const char* src);

		static ErrNo rename(pid_t user, const char* newfullPathName, const char* oldName, VFSDir* oldDir);

		static int chmod(pid_t user, ino_t, mode_t);
		static int chown(pid_t user, ino_t, uid_t uid, gid_t gid);

		static ErrNo utime(pid_t user, const char* fileName, const struct utimebuf *timeNew);

		static ErrNo truncate(pid_t user, ino_t, off_t);

		static void sync(void);

		static bool fileNameIsLegal(const c8* fileName);

		static void print(ino_t fd);

		static ErrNo addDev(BaseDevice* dev, const c8* devName, bool needCopy);
		static ErrNo deleteDev(const c8* devName);

	private:
		explicit VFS(void) = delete;
		VFS(const VFS&) = delete;
		VFS& operator=(const VFS&) = delete;

		static VFSDir* getRootNode(void){ return rootNode; }
		static VFSDir* getRootUser(void){ return rootUserNode; }

		static VFSNode* getNodeById(ino_t nodeId);

		static void destroyObjNode(VFSNode* node);

		static const VFSDir* findSubDirFromCurDir(const VFSDir* curDir, const char* subDirPathName, bool lockNeeded);

		template<typename T, typename... ARGS>
		static T* createObjNode(ARGS... args)
		{
			T* res = static_cast<T*>(VFSManager::allocate());
			if(res)
			{
				res->init(args...);
			}
			return res;
		}

		static VFSDir* rootNode;
		static VFSDir* rootUserNode;
		static VFSDir* homeNode;
		static VFSDir* tmpNode;
		static VFSDir* devNode;

		friend class VFSNode;
		friend class Kernel::Procedure::Thread;
		friend class Kernel::Procedure::GeneralProc;
		friend class Kernel::ProcManager;
	};
}

#endif

