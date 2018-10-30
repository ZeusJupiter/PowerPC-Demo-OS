/*
 *   File name: nand_bch.h
 *
 *  Created on: 2017年7月25日, 下午7:20:24
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description: from linux source code
 */

#ifndef __INCLUDE_LINUX_MTD_NAND_BCH_H__
#define __INCLUDE_LINUX_MTD_NAND_BCH_H__

#include "mtdcfg.h"

struct mtd_info;
struct nand_bch_control;

static inline int mtd_nand_has_bch(void) { return 1; }

BEG_EXT_C

int nand_bch_calculate_ecc(struct mtd_info *mtd, const uchar *dat, uchar *ecc_code);
int nand_bch_correct_data(struct mtd_info *mtd, uchar *dat, uchar *read_ecc, uchar *calc_ecc);
struct nand_bch_control * nand_bch_init(struct mtd_info *mtd, unsigned int eccsize,
	      unsigned int eccbytes, struct nand_ecclayout **ecclayout);
void nand_bch_free(struct nand_bch_control *nbc);

END_EXT_C

#endif

