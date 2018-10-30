/*
 *   File name: stat.h
 *
 *  Created on: 2017年5月11日, 下午5:40:58
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_SYS_STAT_H__
#define __INCLUDE_SYS_STAT_H__

#define S_IFMT		0170000
#define S_IFDEVMEM  0150000
#define S_IFSOCK	0140000
#define S_IFSHM		0130000
#define S_IFLNK		0120000
#define S_IFMS		0110000
#define S_IFREG		0100000
#define S_IFBLK		0060000
#define S_IFDIR		0040000
#define S_IFCHR		0020000
#define S_IFIFO		0010000
#define S_ISUID		0004000
#define S_ISGID		0002000
#define S_ISSTICKY	0001000
#define S_IRWXU		0000700
#define S_IRUSR		0000400
#define S_IWUSR		0000200
#define S_IXUSR		0000100
#define S_IRWXG		0000070
#define S_IRGRP		0000040
#define S_IWGRP		0000020
#define S_IXGRP		0000010
#define S_IRWXO		0000007
#define S_IROTH		0000004
#define S_IWOTH		0000002
#define S_IXOTH		0000001

#define S_ISDIR(mode) 		(((mode) & S_IFMT) == S_IFDIR)
#define S_ISREG(mode) 		(((mode) & S_IFMT) == S_IFREG)
#define S_ISLNK(mode)		(((mode) & S_IFMT) == S_IFLNK)
#define S_ISCHR(mode) 		(((mode) & S_IFMT) == S_IFCHR)
#define S_ISBLK(mode)		(((mode) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(mode)		(((mode) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(mode)		(((mode) & S_IFMT) == S_IFSOCK)
#define S_ISMS(mode)		(((mode) & S_IFMT) == S_IFMS)
#define S_ISSHM(mode)		(((mode) & S_IFMT) == S_IFSHM)

#define MODE_READ			(S_IRUSR | S_IRGRP | S_IROTH)
#define MODE_WRITE			(S_IWUSR | S_IWGRP | S_IWOTH)
#define MODE_EXEC			(S_IXUSR | S_IXGRP | S_IXOTH)
#define MODE_PERM			(MODE_READ | MODE_WRITE | MODE_EXEC | \
							S_ISUID | S_ISGID | S_ISSTICKY)

#define FILE_DEF_MODE		(S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define DIR_DEF_MODE		(S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR | \
								S_IRGRP | S_IXGRP | \
								S_IROTH | S_IXOTH)
#define LNK_DEF_MODE		(S_IFLNK | S_IRUSR | S_IXUSR | \
								S_IRGRP | S_IXGRP | \
								S_IROTH | S_IXOTH)

#define SF_RDONLY   0x1
#define SF_NOSUID   0x2

struct stat {
	dev_t     st_dev;
	ino_t     st_ino;
	mode_t    st_mode;
	nlink_t   st_nlink;
	uid_t     st_uid;
	gid_t     st_gid;
	off_t     st_size;
	time_t    st_atime;
	time_t    st_mtime;
	time_t    st_ctime;
	dev_t     st_rdev;
	blksize_t st_blksize;
	blkcnt_t  st_blocks;
};

typedef struct fsid {
    int32_t       val[2];
} fsid_t;

struct statfs {
    long          f_type;
    long          f_bsize;
    long          f_blocks;
    long          f_bfree;
    long          f_bavail;
    long          f_files;
    long          f_ffree;
    fsid_t        f_fsid;
    long          f_flag;
    long          f_namelen;
    long          f_spare[7];
};

#if defined( __cplusplus )
extern "C" {
#endif

extern int fstat (int filedes, struct stat *buf);

#if defined( __cplusplus )
}
#endif

#endif

