/*
 *   File name: nand_ecc.h
 *
 *  Created on: 2017年7月25日, 下午7:25:47
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description: from linux source code
 */

#ifndef __INCLUDE_LINUX_MTD_NAND_ECC_H__
#define __INCLUDE_LINUX_MTD_NAND_ECC_H__

struct mtd_info;

BEG_EXT_C

int nand_calculate_ecc(struct mtd_info *mtd, const uchar *dat, uchar *ecc_code);
int nand_correct_data(struct mtd_info *mtd, uchar *dat, uchar *read_ecc, uchar *calc_ecc);

END_EXT_C

#endif

