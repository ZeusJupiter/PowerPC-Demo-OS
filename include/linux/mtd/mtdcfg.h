/*
 *   File name: mtdcfg.h
 *
 *  Created on: 2017年7月25日, 下午7:22:16
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_LINUX_MTD_MTDCFG_H__
#define __INCLUDE_LINUX_MTD_MTDCFG_H__

#define CONFIG_SYS_NAND_MAX_CHIPS		1

#define CONFIG_SYS_NAND_ONFI_DETECTION 1
#define CONFIG_MTD_NAND_ECC_BCH 1
#define CONFIG_NAND_ECC_BCH 1

#define MAX_MTD_DEVICES 32
#define MTD_MAX_OOBFREE_ENTRIES_LARGE	32
#define MTD_MAX_ECCPOS_ENTRIES_LARGE	448

#define MTD_MAX_OOBFREE_ENTRIES	8
#define MTD_MAX_ECCPOS_ENTRIES	64

#define NAND_MAX_OOBSIZE	576
#define NAND_MAX_PAGESIZE	8192

#endif

