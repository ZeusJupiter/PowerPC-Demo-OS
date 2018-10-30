/*
 *   File name: mtdabi.h
 *
 *  Created on: 2017年7月25日, 下午8:05:29
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description: from linux source code
 */

#include "../port.h"
#include "mtdcfg.h"

#ifndef __INCLUDE_LINUX_MTD_MTDABI_H__
#define __INCLUDE_LINUX_MTD_MTDABI_H__

struct erase_info_user
{
	u32 start;
	u32 length;
};

struct erase_info_user64
{
	u64 start;
	u64 length;
};

struct mtd_oob_buf
{
	u32 start;
	u32 length;
	u8 __user *ptr;
};

struct mtd_oob_buf64
{
	u64 start;
	u32 pad;
	u32 length;
	u64 usr_ptr;
};

enum MtdOperationModes
{
	MTD_OPS_PLACE_OOB = 0,
	MTD_OPS_AUTO_OOB = 1,
	MTD_OPS_RAW = 2,
};

#define MTD_ABSENT		0
#define MTD_RAM			1
#define MTD_ROM			2
#define MTD_NORFLASH		3
#define MTD_NANDFLASH		4
#define MTD_DATAFLASH		6
#define MTD_UBIVOLUME		7

#define MTD_WRITEABLE		0x400

#define MTD_BIT_WRITEABLE	0x800

#define MTD_NO_ERASE		0x1000

#define MTD_STUPID_LOCK		0x2000

#define MTD_CAP_ROM		0
#define MTD_CAP_RAM		(MTD_WRITEABLE | MTD_BIT_WRITEABLE | MTD_NO_ERASE)
#define MTD_CAP_NORFLASH	(MTD_WRITEABLE | MTD_BIT_WRITEABLE)
#define MTD_CAP_NANDFLASH	(MTD_WRITEABLE)

#define MTD_NANDECC_OFF		0

#define MTD_NANDECC_PLACE	1

#define MTD_NANDECC_AUTOPLACE	2

#define MTD_NANDECC_PLACEONLY	3

#define MTD_NANDECC_AUTOPL_USR	4

#define MTD_OTP_OFF		0
#define MTD_OTP_FACTORY		1
#define MTD_OTP_USER		2

struct mtd_info_user
{
	u8 type;
	u32 flags;
	u32 size;

	u32 erasesize;
	u32 writesize;
	u32 oobsize;

	u32 ecctype;
	u32 eccsize;

};

struct region_info_user
{
	u32 offset;
	u32 erasesize;
	u32 numblocks;
	u32 regionindex;
};

struct otp_info
{
	u32 start;
	u32 length;
	u32 locked;
};

struct nand_oobinfo
{
	u32 useecc;
	u32 eccbytes;
	u32 oobfree[8][2];
	u32 eccpos[48];
};

struct nand_oobfree
{
	u32 offset;
	u32 length;
};

struct mtd_ecc_stats {
	u32 corrected;
	u32 failed;
	u32 badblocks;
	u32 bbtblocks;
};

enum mtd_file_modes
{
	MTD_MODE_NORMAL = MTD_OTP_OFF,
	MTD_MODE_OTP_FACTORY = MTD_OTP_FACTORY,
	MTD_MODE_OTP_USER = MTD_OTP_USER,
	MTD_MODE_RAW,
};

#endif

