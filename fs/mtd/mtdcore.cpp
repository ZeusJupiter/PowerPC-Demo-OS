/*
 *   File name: mtdcore.cpp
 *
 *  Created on: 2017年7月27日, 下午1:00:21
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description: from linux source code
 */

#include <macros.h>
#include <types.h>
#include <stdio.h>
#include <debug.h>
#include <assert.h>
#include <string.h>

#include <linux/mtd/mtd.h>

static struct mtd_info *mtd_table[MAX_MTD_DEVICES];

BEG_EXT_C

int add_mtd_device(struct mtd_info *mtd)
{
	int i;

	assert(mtd->writesize == 0);

	for (i = 0; i < MAX_MTD_DEVICES; i++)
		if (!mtd_table[i]) {
			mtd_table[i] = mtd;
			mtd->index = i;
			mtd->usecount = 0;

			if (mtd->bitflip_threshold == 0)
				mtd->bitflip_threshold = mtd->ecc_strength;

			return 0;
		}

	return 1;
}

int del_mtd_device(struct mtd_info *mtd)
{
	int ret;

	if (mtd_table[mtd->index] != mtd) {
		ret = -static_cast<sint>(ErrNo::ENODEV);
	} else if (mtd->usecount) {
		kformatln("Removing MTD device #%d (%s) with use count %d\n",
				mtd->index, mtd->name, mtd->usecount);
		ret = -static_cast<sint>(ErrNo::EBUSY);
	} else {

		mtd_table[mtd->index] = nullptr;
		ret = static_cast<sint>(ErrNo::ENONE);
	}

	return ret;
}

struct mtd_info *get_mtd_device(struct mtd_info *mtd, int num)
{
	struct mtd_info *ret = nullptr;

	if (num == -1) {
		for (int i = 0; i < MAX_MTD_DEVICES; i++)
			if (mtd_table[i] == mtd)
				ret = mtd_table[i];
	} else if (num < MAX_MTD_DEVICES) {
		ret = mtd_table[num];
		if (mtd && mtd != ret)
			ret = nullptr;
	}

	if (ret)
		ret->usecount++;

	return ret;
}

struct mtd_info *get_mtd_device_nm(const char *name)
{
	int i;
	struct mtd_info *mtd = nullptr;

	for (i = 0; i < MAX_MTD_DEVICES; i++) {
		if (mtd_table[i] && !strcmp(name, mtd_table[i]->name)) {
			mtd = mtd_table[i];
			break;
		}
	}

	if (mtd)
		mtd->usecount++;
	return mtd;
}

void put_mtd_device(struct mtd_info *mtd)
{
	int c;

	c = --mtd->usecount;
	assert(c < 0);
}

#if defined(CONFIG_CMD_MTDPARTS_SPREAD)

void mtd_get_len_incl_bad(struct mtd_info *mtd, uint64_t offset,
			  const uint64_t length, uint64_t *len_incl_bad,
			  int *truncated)
{
	*truncated = 0;
	*len_incl_bad = 0;

	if (!mtd->block_isbad) {
		*len_incl_bad = length;
		return;
	}

	uint64_t len_excl_bad = 0;
	uint64_t block_len;

	while (len_excl_bad < length) {
		if (offset >= mtd->size) {
			*truncated = 1;
			return;
		}

		block_len = mtd->erasesize - (offset & (mtd->erasesize - 1));

		if (!mtd->block_isbad(mtd, offset & ~(mtd->erasesize - 1)))
			len_excl_bad += block_len;

		*len_incl_bad += block_len;
		offset       += block_len;
	}
}
#endif

int mtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	if (instr->addr > mtd->size || instr->len > mtd->size - instr->addr)
		return -static_cast<sint>(ErrNo::EINVAL);
	if (!(mtd->flags & MTD_WRITEABLE))
		return -static_cast<sint>(ErrNo::EROFS);
	instr->fail_addr = MTD_FAIL_ADDR_UNKNOWN;
	if (!instr->len) {
		instr->state = MTD_ERASE_DONE;
		mtd_erase_callback(instr);
		return 0;
	}
	return mtd->_erase(mtd, instr);
}

int mtd_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, uchar *buf)
{
	if (from < 0 || from > mtd->size || len > mtd->size - from)
		return -static_cast<sint>(ErrNo::EINVAL);
	if (!len)
		return 0;

	int ret_code = mtd->_read(mtd, from, len, retlen, buf);
	if (unlikely(ret_code < 0))
		return ret_code;
	if (mtd->ecc_strength == 0)
		return 0;

	return ret_code >= mtd->bitflip_threshold ? -static_cast<sint>(ErrNo::EUCLEAN) : 0;
}

int mtd_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const uchar *buf)
{
	*retlen = 0;
	if (to < 0 || to > mtd->size || len > mtd->size - to)
		return -static_cast<sint>(ErrNo::EINVAL);
	if (!mtd->_write || !(mtd->flags & MTD_WRITEABLE))
		return -static_cast<sint>(ErrNo::EROFS);
	if (!len)
		return 0;
	return mtd->_write(mtd, to, len, retlen, buf);
}

int mtd_panic_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const uchar *buf)
{
	*retlen = 0;
	if (!mtd->_panic_write)
		return -static_cast<sint>(ErrNo::EOPNOTSUPP);
	if (to < 0 || to > mtd->size || len > mtd->size - to)
		return -static_cast<sint>(ErrNo::EINVAL);
	if (!(mtd->flags & MTD_WRITEABLE))
		return -static_cast<sint>(ErrNo::EROFS);
	if (!len)
		return 0;
	return mtd->_panic_write(mtd, to, len, retlen, buf);
}

int mtd_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
	ops->retlen = ops->oobretlen = 0;
	if (!mtd->_read_oob)
		return -static_cast<sint>(ErrNo::EOPNOTSUPP);
	return mtd->_read_oob(mtd, from, ops);
}

int mtd_get_fact_prot_info(struct mtd_info *mtd, struct otp_info *buf, size_t len)
{
	if (!mtd->_get_fact_prot_info)
		return -static_cast<sint>(ErrNo::EOPNOTSUPP);
	if (!len)
		return 0;
	return mtd->_get_fact_prot_info(mtd, buf, len);
}

int mtd_read_fact_prot_reg(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, uchar *buf)
{
	*retlen = 0;
	if (!mtd->_read_fact_prot_reg)
		return -static_cast<sint>(ErrNo::EOPNOTSUPP);
	if (!len)
		return 0;
	return mtd->_read_fact_prot_reg(mtd, from, len, retlen, buf);
}

int mtd_get_user_prot_info(struct mtd_info *mtd, struct otp_info *buf, size_t len)
{
	if (!mtd->_get_user_prot_info)
		return -static_cast<sint>(ErrNo::EOPNOTSUPP);
	if (!len)
		return 0;
	return mtd->_get_user_prot_info(mtd, buf, len);
}

int mtd_read_user_prot_reg(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, uchar *buf)
{
	*retlen = 0;
	if (!mtd->_read_user_prot_reg)
		return -static_cast<sint>(ErrNo::EOPNOTSUPP);
	if (!len)
		return 0;
	return mtd->_read_user_prot_reg(mtd, from, len, retlen, buf);
}

int mtd_write_user_prot_reg(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, uchar *buf)
{
	*retlen = 0;
	if (!mtd->_write_user_prot_reg)
		return -static_cast<sint>(ErrNo::EOPNOTSUPP);
	if (!len)
		return 0;
	return mtd->_write_user_prot_reg(mtd, to, len, retlen, buf);
}

int mtd_lock_user_prot_reg(struct mtd_info *mtd, loff_t from, size_t len)
{
	if (!mtd->_lock_user_prot_reg)
		return -static_cast<sint>(ErrNo::EOPNOTSUPP);
	if (!len)
		return 0;
	return mtd->_lock_user_prot_reg(mtd, from, len);
}

int mtd_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	if (!mtd->_lock)
		return -static_cast<sint>(ErrNo::EOPNOTSUPP);
	if (ofs < 0 || ofs > mtd->size || len > mtd->size - ofs)
		return -static_cast<sint>(ErrNo::EINVAL);
	if (!len)
		return 0;
	return mtd->_lock(mtd, ofs, len);
}

int mtd_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	if (!mtd->_unlock)
		return -static_cast<sint>(ErrNo::EOPNOTSUPP);
	if (ofs < 0 || ofs > mtd->size || len > mtd->size - ofs)
		return -static_cast<sint>(ErrNo::EINVAL);
	if (!len)
		return 0;
	return mtd->_unlock(mtd, ofs, len);
}

int mtd_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	if (!mtd->_block_isbad)
		return 0;
	if (ofs < 0 || ofs > mtd->size)
		return -static_cast<sint>(ErrNo::EINVAL);
	return mtd->_block_isbad(mtd, ofs);
}

int mtd_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	if (!mtd->_block_markbad)
		return -static_cast<sint>(ErrNo::EOPNOTSUPP);
	if (ofs < 0 || ofs > mtd->size)
		return -static_cast<sint>(ErrNo::EINVAL);
	if (!(mtd->flags & MTD_WRITEABLE))
		return -static_cast<sint>(ErrNo::EROFS);
	return mtd->_block_markbad(mtd, ofs);
}

END_EXT_C
