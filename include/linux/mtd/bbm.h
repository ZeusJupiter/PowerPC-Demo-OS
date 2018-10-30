/*
 *   File name: bbm.h
 *
 *  Created on: 2017年7月25日, 下午6:36:57
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description: from linux source code
 */

#include "mtdcfg.h"

#ifndef __INCLUDE_LINUX_MTD_BBM_H__
#define __INCLUDE_LINUX_MTD_BBM_H__

struct nand_bbt_descr {
	int options;
	int pages[CONFIG_SYS_NAND_MAX_CHIPS];
	int offs;
	int veroffs;
	u8 version[CONFIG_SYS_NAND_MAX_CHIPS];
	int len;
	int maxblocks;
	int reserved_block_code;
	const u8 *pattern;
};

#define NAND_BBT_NRBITS_MSK	0x0000000F
#define NAND_BBT_1BIT		0x00000001
#define NAND_BBT_2BIT		0x00000002
#define NAND_BBT_4BIT		0x00000004
#define NAND_BBT_8BIT		0x00000008

#define NAND_BBT_LASTBLOCK	0x00000010

#define NAND_BBT_ABSPAGE	0x00000020

#define NAND_BBT_SEARCH		0x00000040

#define NAND_BBT_PERCHIP	0x00000080

#define NAND_BBT_VERSION	0x00000100

#define NAND_BBT_CREATE		0x00000200

#define NAND_BBT_CREATE_EMPTY	0x00000400

#define NAND_BBT_SCANALLPAGES	0x00000800

#define NAND_BBT_SCANEMPTY	0x00001000

#define NAND_BBT_WRITE		0x00002000

#define NAND_BBT_SAVECONTENT	0x00004000

#define NAND_BBT_SCAN2NDPAGE	0x00008000

#define NAND_BBT_SCANLASTPAGE	0x00010000

#define NAND_BBT_USE_FLASH	0x00020000

#define NAND_BBT_NO_OOB		0x00040000

#define NAND_BBT_NO_OOB_BBM	0x00080000

#define NAND_BBT_DYNAMICSTRUCT	0x80000000

#define NAND_BBT_SCAN_MAXBLOCKS	4

#define NAND_SMALL_BADBLOCK_POS		5
#define NAND_LARGE_BADBLOCK_POS		0
#define ONENAND_BADBLOCK_POS		0

#define ONENAND_BADBLOCK_POS	0

#define ONENAND_BBT_READ_ERROR		1
#define ONENAND_BBT_READ_ECC_ERROR	2
#define ONENAND_BBT_READ_FATAL_ERROR	4

struct bbm_info {
	int bbt_erase_shift;
	int badblockpos;
	int options;

	u8 *bbt;

	int (*isbad_bbt)(struct mtd_info *mtd, loff_t ofs, int allowbbt);
	struct nand_bbt_descr *badblock_pattern;

	void *priv;
};

BEG_EXT_C

extern int onenand_scan_bbt(struct mtd_info *mtd, struct nand_bbt_descr *bd);
extern int onenand_default_bbt(struct mtd_info *mtd);

END_EXT_C

#endif

