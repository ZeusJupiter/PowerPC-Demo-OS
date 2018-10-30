/*
 *   File name: nandbbt.cpp
 *
 *  Created on: 2017年7月28日, 下午3:37:47
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description: drivers/mtd/nand_bbt.c
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <mm/heap.h>

#include <linux/mtd/bbm.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>

static void *check_bytes8(const u8 *start, u8 value, unsigned int bytes)
{
    while (bytes)
    {
        if (*start != value)
            return (void *)start;
        start++;
        bytes--;
    }
    return nullptr;
}

static void *memchr_inv(const void *start, int c, size_t bytes)
{
    u8 value = (u8)c;
    u64 value64;
    unsigned int words, prefix;

    if (bytes <= 16)
        return check_bytes8(static_cast<const u8 *>(start), value, bytes);

    value64 = value;
    value64 |= value64 << 8;
    value64 |= value64 << 16;
    value64 |= value64 << 32;

    prefix = (unsigned long)start % 8;
    if (prefix)
    {
        prefix = 8 - prefix;
        u8 *r = (u8*)check_bytes8(static_cast<const u8 *>(start), value, prefix);
        if (r)
            return r;
        start += prefix;
        bytes -= prefix;
    }

    words = bytes / 8;

    while (words) {
        if (*(u64 *)start != value64)
            return check_bytes8(static_cast<const u8 *>(start), value, 8);
        start += 8;
        words--;
    }

    return check_bytes8(static_cast<const u8 *>(start), value, bytes % 8);
}

static int check_pattern_no_oob(uint8_t *buf, struct nand_bbt_descr *td)
{
	if (memcmp(buf, td->pattern, td->len))
		return -1;
	return 0;
}

static int check_pattern(uint8_t *buf, int len, int paglen, struct nand_bbt_descr *td)
{
	if (td->options & NAND_BBT_NO_OOB)
		return check_pattern_no_oob(buf, td);

	int end = 0;
	uint8_t *p = buf;
	end = paglen + td->offs;
	if (td->options & NAND_BBT_SCANEMPTY)
		if (memchr_inv(p, 0xff, end))
			return -1;
	p += end;

	if (memcmp(p, td->pattern, td->len))
		return -1;

	if (td->options & NAND_BBT_SCANEMPTY) {
		p += td->len;
		end += td->len;
		if (memchr_inv(p, 0xff, len - end))
			return -1;
	}
	return 0;
}

static int check_short_pattern(uint8_t *buf, struct nand_bbt_descr *td)
{

	if (memcmp(buf + td->offs, td->pattern, td->len))
		return -1;
	return 0;
}

static u32 add_marker_len(struct nand_bbt_descr *td)
{
	u32 len;

	if (!(td->options & NAND_BBT_NO_OOB))
		return 0;

	len = td->len;
	if (td->options & NAND_BBT_VERSION)
		len++;
	return len;
}

static int read_bbt(struct mtd_info *mtd, uint8_t *buf, int page, int num,
		struct nand_bbt_descr *td, int offs)
{
	int ret = 0;
	struct nand_chip *thisChip = static_cast<struct nand_chip*>(mtd->priv);
	size_t totlen;
	loff_t from;
	int bits = td->options & NAND_BBT_NRBITS_MSK;
	uint8_t msk = (uint8_t)((1 << bits) - 1);
	u32 marker_len;
	int reserved_block_code = td->reserved_block_code;

	totlen = (num * bits) >> 3;
	marker_len = add_marker_len(td);
	from = ((loff_t)page) << thisChip->page_shift;

	while (totlen)
	{
		size_t len = min(totlen, (size_t)(1 << thisChip->bbt_erase_shift));
		if (marker_len)
		{
			len -= marker_len;
			from += marker_len;
			marker_len = 0;
		}
		size_t retlen;
		int res = mtd_read(mtd, from, len, &retlen, buf);
		if (res < 0)
		{
			if (mtd_is_eccerr(res))
			{
				kformatln("nand_bbt: ECC error in BBT at " "0x%012llx",
						from & ~mtd->writesize);
				return res;
			}
			else if (mtd_is_bitflip(res))
			{
				kformatln("nand_bbt: corrected error in BBT at " "0x%012llx",
						from & ~mtd->writesize);
				ret = res;
			}
			else
			{
				kdebugln("nand_bbt: error reading BBT");
				return res;
			}
		}

		for (size_t i = 0; i < len; i++)
		{
			uint8_t dat = buf[i];
			int act = 0;
			for (int j = 0; j < 8; j += bits, act += 2)
			{
				uint8_t tmp = (dat >> j) & msk;
				if (tmp == msk)
					continue;
				if (reserved_block_code && (tmp == reserved_block_code))
				{
					kformatln("nand_read_bbt: reserved block at 0x%012llx",
							(loff_t)((offs << 2) + (act >> 1)) << thisChip->bbt_erase_shift);
					thisChip->bbt[offs + (act >> 3)] |= 0x2 << (act & 0x06);
					mtd->ecc_stats.bbtblocks++;
					continue;
				}
				kformatln("nand_read_bbt: Bad block at 0x%012llx",
						(loff_t)((offs << 2) + (act >> 1)) << thisChip->bbt_erase_shift);
		
				if (tmp == 0)
					thisChip->bbt[offs + (act >> 3)] |= 0x3 << (act & 0x06);
				else
					thisChip->bbt[offs + (act >> 3)] |= 0x1 << (act & 0x06);
				mtd->ecc_stats.badblocks++;
			}
		}
		totlen -= len;
		from += len;
	}

	return ret;
}

static int read_abs_bbt(struct mtd_info *mtd, uint8_t *buf, struct nand_bbt_descr *td, int chip)
{
	struct nand_chip *thisChip = static_cast<struct nand_chip*>(mtd->priv);
	int res = 0, i;

	if (td->options & NAND_BBT_PERCHIP)
	{
		int offs = 0;
		for (i = 0; i < thisChip->numchips; i++)
		{
			if (chip == -1 || chip == i)
				res = read_bbt(mtd, buf, td->pages[i],
						thisChip->chipsize >> thisChip->bbt_erase_shift, td,
						offs);
			if (res)
				return res;
			offs += thisChip->chipsize >> (thisChip->bbt_erase_shift + 2);
		}
	}
	else
	{
		res = read_bbt(mtd, buf, td->pages[0],
				mtd->size >> thisChip->bbt_erase_shift, td, 0);
		if (res)
			return res;
	}
	return 0;
}

static int scan_read_data(struct mtd_info *mtd, uint8_t *buf, loff_t offs,
			 struct nand_bbt_descr *td)
{
	size_t retlen;
	size_t len;

	len = td->len;
	if (td->options & NAND_BBT_VERSION)
		len++;

	return mtd_read(mtd, offs, len, &retlen, buf);
}

static int scan_read_oob(struct mtd_info *mtd, uint8_t *buf, loff_t offs,
			 size_t len)
{
	struct mtd_oob_ops ops;
	int res, ret = 0;

	ops.mode = MTD_OPS_PLACE_OOB;
	ops.ooboffs = 0;
	ops.ooblen = mtd->oobsize;

	while (len > 0) {
		ops.datbuf = buf;
		ops.len = min(len, (size_t)mtd->writesize);
		ops.oobbuf = buf + ops.len;

		res = mtd_read_oob(mtd, offs, &ops);
		if (res) {
			if (!mtd_is_bitflip_or_eccerr(res))
				return res;
			else if (mtd_is_eccerr(res) || !ret)
				ret = res;
		}

		buf += mtd->oobsize + mtd->writesize;
		len -= mtd->writesize;
		offs += mtd->writesize;
	}
	return ret;
}

static int scan_read(struct mtd_info *mtd, uint8_t *buf, loff_t offs,
			 size_t len, struct nand_bbt_descr *td)
{
	if (td->options & NAND_BBT_NO_OOB)
		return scan_read_data(mtd, buf, offs, td);
	else
		return scan_read_oob(mtd, buf, offs, len);
}

static int scan_write_bbt(struct mtd_info *mtd, loff_t offs, size_t len,
			  uint8_t *buf, uint8_t *oob)
{
	struct mtd_oob_ops ops;

	ops.mode = MTD_OPS_PLACE_OOB;
	ops.ooboffs = 0;
	ops.ooblen = mtd->oobsize;
	ops.datbuf = buf;
	ops.oobbuf = oob;
	ops.len = len;

	return mtd_write_oob(mtd, offs, &ops);
}

static u32 bbt_get_ver_offs(struct mtd_info *mtd, struct nand_bbt_descr *td)
{
	u32 ver_offs = td->veroffs;

	if (!(td->options & NAND_BBT_NO_OOB))
		ver_offs += mtd->writesize;
	return ver_offs;
}

static void read_abs_bbts(struct mtd_info *mtd, uint8_t *buf,
			  struct nand_bbt_descr *td, struct nand_bbt_descr *md)
{
	struct nand_chip *thisChip = static_cast<struct nand_chip*>(mtd->priv);

	if (td->options & NAND_BBT_VERSION) {
		scan_read(mtd, buf, (loff_t)td->pages[0] << thisChip->page_shift,
			      mtd->writesize, td);
		td->version[0] = buf[bbt_get_ver_offs(mtd, td)];
		kformatln("Bad block table at page %d, version 0x%02X",
			 td->pages[0], td->version[0]);
	}

	if (md && (md->options & NAND_BBT_VERSION)) {
		scan_read(mtd, buf, (loff_t)md->pages[0] << thisChip->page_shift,
			      mtd->writesize, md);
		md->version[0] = buf[bbt_get_ver_offs(mtd, md)];
		kformatln("Bad block table at page %d, version 0x%02X",
			 md->pages[0], md->version[0]);
	}
}

static int scan_block_full(struct mtd_info *mtd, struct nand_bbt_descr *bd,
			   loff_t offs, uint8_t *buf, size_t readlen,
			   int scanlen, int numpages)
{
	int ret, j;

	ret = scan_read_oob(mtd, buf, offs, readlen);

	if (ret && !mtd_is_bitflip_or_eccerr(ret))
		return ret;

	for (j = 0; j < numpages; j++, buf += scanlen) {
		if (check_pattern(buf, scanlen, mtd->writesize, bd))
			return 1;
	}
	return 0;
}

static int scan_block_fast(struct mtd_info *mtd, struct nand_bbt_descr *bd,
			   loff_t offs, uint8_t *buf, int numpages)
{
	struct mtd_oob_ops ops;
	int j, ret;

	ops.ooblen = mtd->oobsize;
	ops.oobbuf = buf;
	ops.ooboffs = 0;
	ops.datbuf = nullptr;
	ops.mode = MTD_OPS_PLACE_OOB;

	for (j = 0; j < numpages; j++)
	{

		ret = mtd_read_oob(mtd, offs, &ops);

		if (ret && !mtd_is_bitflip_or_eccerr(ret))
			return ret;

		if (check_short_pattern(buf, bd))
			return 1;

		offs += mtd->writesize;
	}
	return 0;
}

static int create_bbt(struct mtd_info *mtd, uint8_t *buf,
	struct nand_bbt_descr *bd, int chip)
{
	struct nand_chip *thisChip = static_cast<struct nand_chip*>(mtd->priv);
	int i, numblocks, numpages, scanlen;
	int startblock;
	loff_t from;
	size_t readlen;

	kdebugln("Scanning device for bad blocks");

	if (bd->options & NAND_BBT_SCANALLPAGES)
		numpages = 1 << (thisChip->bbt_erase_shift - thisChip->page_shift);
	else if (bd->options & NAND_BBT_SCAN2NDPAGE)
		numpages = 2;
	else
		numpages = 1;

	if (!(bd->options & NAND_BBT_SCANEMPTY))
	{

		scanlen = 0;
		readlen = bd->len;
	}
	else
	{
		scanlen = mtd->writesize + mtd->oobsize;
		readlen = numpages * mtd->writesize;
	}

	if (chip == -1)
	{
		numblocks = mtd->size >> (thisChip->bbt_erase_shift - 1);
		startblock = 0;
		from = 0;
	}
	else
	{
		if (chip >= thisChip->numchips)
		{
			kformatln("create_bbt(): chipnr (%d) > available chips (%d)",
					chip + 1, thisChip->numchips);
			return -static_cast<sint>(ErrNo::EINVAL);
		}
		numblocks = thisChip->chipsize >> (thisChip->bbt_erase_shift - 1);
		startblock = chip * numblocks;
		numblocks += startblock;
		from = (loff_t) startblock << (thisChip->bbt_erase_shift - 1);
	}

	if (thisChip->bbt_options & NAND_BBT_SCANLASTPAGE)
		from += mtd->erasesize - (mtd->writesize * numpages);

	for (i = startblock; i < numblocks;)
	{
		int ret;

		assert(bd->options & NAND_BBT_NO_OOB);

		if (bd->options & NAND_BBT_SCANALLPAGES)
			ret = scan_block_full(mtd, bd, from, buf, readlen, scanlen,
					numpages);
		else
			ret = scan_block_fast(mtd, bd, from, buf, numpages);

		if (ret < 0)
			return ret;

		if (ret)
		{
			thisChip->bbt[i >> 3] |= 0x03 << (i & 0x6);
			kformatln("Bad eraseblock %d at 0x%012llx", i >> 1,
					(unsigned long long)from);
			mtd->ecc_stats.badblocks++;
		}

		i += 2;
		from += (1 << thisChip->bbt_erase_shift);
	}
	return 0;
}

static int search_bbt(struct mtd_info *mtd, uint8_t *buf, struct nand_bbt_descr *td)
{
	struct nand_chip *thisChip = static_cast<struct nand_chip*>(mtd->priv);
	int i, chips;
	int startblock, block, dir;
	int scanlen = mtd->writesize + mtd->oobsize;
	int bbtblocks;
	int blocktopage = thisChip->bbt_erase_shift - thisChip->page_shift;

	if (td->options & NAND_BBT_LASTBLOCK)
	{
		startblock = (mtd->size >> thisChip->bbt_erase_shift) - 1;
		dir = -1;
	}
	else
	{
		startblock = 0;
		dir = 1;
	}

	if (td->options & NAND_BBT_PERCHIP)
	{
		chips = thisChip->numchips;
		bbtblocks = thisChip->chipsize >> thisChip->bbt_erase_shift;
		startblock &= bbtblocks - 1;
	}
	else
	{
		chips = 1;
		bbtblocks = mtd->size >> thisChip->bbt_erase_shift;
	}

	for (i = 0; i < chips; i++)
	{
		td->version[i] = 0;
		td->pages[i] = -1;

		for (block = 0; block < td->maxblocks; block++)
		{
			int actblock = startblock + dir * block;
			loff_t offs = (loff_t) actblock << thisChip->bbt_erase_shift;

			scan_read(mtd, buf, offs, mtd->writesize, td);
			if (!check_pattern(buf, scanlen, mtd->writesize, td))
			{
				td->pages[i] = actblock << blocktopage;
				if (td->options & NAND_BBT_VERSION)
				{
					offs = bbt_get_ver_offs(mtd, td);
					td->version[i] = buf[offs];
				}
				break;
			}
		}
		startblock += thisChip->chipsize >> thisChip->bbt_erase_shift;
	}

	for (i = 0; i < chips; i++)
	{
		if (td->pages[i] == -1)
			kformatln("Bad block table not found for chip %d", i);
		else
			kformatln("Bad block table found at page %d, version 0x%02X",
					td->pages[i], td->version[i]);
	}
	return 0;
}

static void search_read_bbts(struct mtd_info *mtd, uint8_t *buf,
			     struct nand_bbt_descr *td, struct nand_bbt_descr *md)
{
	search_bbt(mtd, buf, td);

	if (md)
		search_bbt(mtd, buf, md);
}

static int write_bbt(struct mtd_info *mtd, uint8_t *buf,
		     struct nand_bbt_descr *td, struct nand_bbt_descr *md,
		     int chipsel)
{
	struct nand_chip *thisChip = static_cast<struct nand_chip*>(mtd->priv);
	struct erase_info einfo;
	int i, j, res, chip = 0;
	int bits, startblock, dir, page, offs, numblocks, sft, sftmsk;
	int nrchips, bbtoffs, pageoffs, ooboffs;
	uint8_t msk[4];
	uint8_t rcode = td->reserved_block_code;
	size_t retlen, len = 0;
	loff_t to;
	struct mtd_oob_ops ops;

	ops.ooblen = mtd->oobsize;
	ops.ooboffs = 0;
	ops.datbuf = nullptr;
	ops.mode = MTD_OPS_PLACE_OOB;

	if (!rcode)
		rcode = 0xff;

	if (td->options & NAND_BBT_PERCHIP)
	{
		numblocks = (int) (thisChip->chipsize >> thisChip->bbt_erase_shift);

		if (chipsel == -1)
		{
			nrchips = thisChip->numchips;
		}
		else
		{
			nrchips = chipsel + 1;
			chip = chipsel;
		}
	}
	else
	{
		numblocks = (int) (mtd->size >> thisChip->bbt_erase_shift);
		nrchips = 1;
	}

	for (; chip < nrchips; chip++)
	{

		if (td->pages[chip] != -1)
		{
			page = td->pages[chip];
			goto write;
		}

		if (td->options & NAND_BBT_LASTBLOCK)
		{
			startblock = numblocks * (chip + 1) - 1;
			dir = -1;
		}
		else
		{
			startblock = chip * numblocks;
			dir = 1;
		}

		for (i = 0; i < td->maxblocks; i++)
		{
			int block = startblock + dir * i;

			switch ((thisChip->bbt[block >> 2] >> (2 * (block & 0x03))) & 0x03)
			{
			case 0x01:
			case 0x03:
				continue;
			}
			page = block << (thisChip->bbt_erase_shift - thisChip->page_shift);

			if (!md || md->pages[chip] != page)
				goto write;
		}
		kdebugln("No space left to write bad block table");
		return -static_cast<sint>(ErrNo::ENOSPC);
		write:

		bits = td->options & NAND_BBT_NRBITS_MSK;
		msk[2] = ~rcode;
		switch (bits)
		{
		case 1:
			sft = 3;
			sftmsk = 0x07;
			msk[0] = 0x00;
			msk[1] = 0x01;
			msk[3] = 0x01;
			break;
		case 2:
			sft = 2;
			sftmsk = 0x06;
			msk[0] = 0x00;
			msk[1] = 0x01;
			msk[3] = 0x03;
			break;
		case 4:
			sft = 1;
			sftmsk = 0x04;
			msk[0] = 0x00;
			msk[1] = 0x0C;
			msk[3] = 0x0f;
			break;
		case 8:
			sft = 0;
			sftmsk = 0x00;
			msk[0] = 0x00;
			msk[1] = 0x0F;
			msk[3] = 0xff;
			break;
		default:
			return -static_cast<sint>(ErrNo::EINVAL);
		}

		bbtoffs = chip * (numblocks >> 2);

		to = ((loff_t) page) << thisChip->page_shift;

		if (td->options & NAND_BBT_SAVECONTENT)
		{
	

			to &= ~((loff_t) ((1 << thisChip->bbt_erase_shift) - 1));
			len = 1 << thisChip->bbt_erase_shift;
			res = mtd_read(mtd, to, len, &retlen, buf);
			if (res < 0)
			{
				if (retlen != len)
				{
					kdebugln(
							"nand_bbt: error reading block " "for writing the bad block table");
					return res;
				}
				kdebugln(
						"nand_bbt: ECC error while reading " "block for writing bad block table");
			}

			ops.ooblen = (len >> thisChip->page_shift) * mtd->oobsize;
			ops.oobbuf = &buf[len];
			res = mtd_read_oob(mtd, to + mtd->writesize, &ops);
			if (res < 0 || ops.oobretlen != ops.ooblen)
				goto outerr;

			pageoffs = page - (int) (to >> thisChip->page_shift);
			offs = pageoffs << thisChip->page_shift;
	

			memset(&buf[offs], 0xff, (size_t) (numblocks >> sft));
			ooboffs = len + (pageoffs * mtd->oobsize);

		}
		else if (td->options & NAND_BBT_NO_OOB)
		{
			ooboffs = 0;
			offs = td->len;

			if (td->options & NAND_BBT_VERSION)
				offs++;
	

			len = (size_t) (numblocks >> sft);
			len += offs;

			len = alignUp(len, mtd->writesize);

			memset(buf, 0xff, len);
			memcpy(buf, td->pattern, td->len);
		}
		else
		{
			len = (size_t) (numblocks >> sft);
			len = alignUp(len, mtd->writesize);

			memset(buf, 0xff,
					len + (len >> thisChip->page_shift) * mtd->oobsize);
			offs = 0;
			ooboffs = len;
			memcpy(&buf[ooboffs + td->offs], td->pattern, td->len);
		}

		if (td->options & NAND_BBT_VERSION)
			buf[ooboffs + td->veroffs] = td->version[chip];

		for (i = 0; i < numblocks;)
		{
			uint8_t dat;
			dat = thisChip->bbt[bbtoffs + (i >> 2)];
			for (j = 0; j < 4; j++, i++)
			{
				int sftcnt = (i << (3 - sft)) & sftmsk;
				buf[offs + (i >> sft)] &= ~(msk[dat & 0x03] << sftcnt);
				dat >>= 2;
			}
		}

		memset(&einfo, 0, sizeof(einfo));
		einfo.mtd = mtd;
		einfo.addr = to;
		einfo.len = 1 << thisChip->bbt_erase_shift;
		res = nand_erase_nand(mtd, &einfo, 1);
		if (res < 0)
			goto outerr;

		res = scan_write_bbt(mtd, to, len, buf,
				td->options & NAND_BBT_NO_OOB ? nullptr : &buf[len]);
		if (res < 0)
			goto outerr;

		kformatln("Bad block table written to 0x%012llx, version 0x%02X",
				(unsigned long long)to, td->version[chip]);

		td->pages[chip] = page;
	}
	return 0;

 outerr:
	kformatln("nand_bbt: error while writing bad block table %d", res);
	return res;
}

static inline int nand_memory_bbt(struct mtd_info *mtd, struct nand_bbt_descr *bd)
{
	struct nand_chip *thisChip = static_cast<struct nand_chip*>(mtd->priv);

	bd->options &= ~NAND_BBT_SCANEMPTY;
	return create_bbt(mtd, thisChip->buffers->databuf, bd, -1);
}

static int check_create(struct mtd_info *mtd, uint8_t *buf, struct nand_bbt_descr *bd)
{
	int i, chips, writeops, create, chipsel, res, res2;
	struct nand_chip *thisChip = static_cast<struct nand_chip*>(mtd->priv);
	struct nand_bbt_descr *td = thisChip->bbt_td;
	struct nand_bbt_descr *md = thisChip->bbt_md;
	struct nand_bbt_descr *rd, *rd2;

	if (td->options & NAND_BBT_PERCHIP)
		chips = thisChip->numchips;
	else
		chips = 1;

	for (i = 0; i < chips; i++)
	{
		writeops = 0;
		create = 0;
		rd = nullptr;
		rd2 = nullptr;
		res = res2 = 0;

		chipsel = (td->options & NAND_BBT_PERCHIP) ? i : -1;

		if (md)
		{
			if (td->pages[i] == -1 && md->pages[i] == -1)
			{
				create = 1;
				writeops = 0x03;
			}
			else if (td->pages[i] == -1)
			{
				rd = md;
				writeops = 0x01;
			}
			else if (md->pages[i] == -1)
			{
				rd = td;
				writeops = 0x02;
			}
			else if (td->version[i] == md->version[i])
			{
				rd = td;
				if (!(td->options & NAND_BBT_VERSION))
					rd2 = md;
			}
			else if (((int8_t) (td->version[i] - md->version[i])) > 0)
			{
				rd = td;
				writeops = 0x02;
			}
			else
			{
				rd = md;
				writeops = 0x01;
			}
		}
		else
		{
			if (td->pages[i] == -1)
			{
				create = 1;
				writeops = 0x01;
			}
			else
			{
				rd = td;
			}
		}

		if (create)
		{
			if (!(td->options & NAND_BBT_CREATE))
				continue;

			if (!(thisChip->bbt_options & NAND_BBT_CREATE_EMPTY))
				create_bbt(mtd, buf, bd, chipsel);

			td->version[i] = 1;
			if (md)
				md->version[i] = 1;
		}

		if (rd)
		{
			res = read_abs_bbt(mtd, buf, rd, chipsel);
			if (mtd_is_eccerr(res))
			{
				rd->pages[i] = -1;
				rd->version[i] = 0;
				i--;
				continue;
			}
		}

		if (rd2)
		{
			res2 = read_abs_bbt(mtd, buf, rd2, chipsel);
			if (mtd_is_eccerr(res2))
			{
				rd2->pages[i] = -1;
				rd2->version[i] = 0;
				i--;
				continue;
			}
		}

		if (mtd_is_bitflip(res) || mtd_is_bitflip(res2))
			writeops = 0x03;

		if (md)
		{
			td->version[i] = ::max<u8>(td->version[i], md->version[i]);
			md->version[i] = td->version[i];
		}

		if ((writeops & 0x01) && (td->options & NAND_BBT_WRITE))
		{
			res = write_bbt(mtd, buf, td, md, chipsel);
			if (res < 0)
				return res;
		}

		if ((writeops & 0x02) && md && (md->options & NAND_BBT_WRITE))
		{
			res = write_bbt(mtd, buf, md, td, chipsel);
			if (res < 0)
				return res;
		}
	}

	return 0;
}

static void mark_bbt_region(struct mtd_info *mtd, struct nand_bbt_descr *td)
{
	struct nand_chip *thisChip = static_cast<struct nand_chip*>(mtd->priv);
	int chips, nrblocks;

	if (td->options & NAND_BBT_PERCHIP)
	{
		chips = thisChip->numchips;
		nrblocks = (int) (thisChip->chipsize >> thisChip->bbt_erase_shift);
	}
	else
	{
		chips = 1;
		nrblocks = (int) (mtd->size >> thisChip->bbt_erase_shift);
	}

	for (int i = 0; i < chips; i++)
	{
		int block, update;
		uint8_t oldval, newval;
		if ((td->options & NAND_BBT_ABSPAGE) || !(td->options & NAND_BBT_WRITE))
		{
			if (td->pages[i] == -1)
				continue;
			block = td->pages[i]
					>> (thisChip->bbt_erase_shift - thisChip->page_shift);
			block <<= 1;
			oldval = thisChip->bbt[(block >> 3)];
			newval = oldval | (0x2 << (block & 0x06));
			thisChip->bbt[(block >> 3)] = newval;
			if ((oldval != newval) && td->reserved_block_code)
				nand_update_bbt(mtd,
						(loff_t) block << (thisChip->bbt_erase_shift - 1));
			continue;
		}
		update = 0;
		if (td->options & NAND_BBT_LASTBLOCK)
			block = ((i + 1) * nrblocks) - td->maxblocks;
		else
			block = i * nrblocks;
		block <<= 1;
		for (int j = 0; j < td->maxblocks; j++)
		{
			oldval = thisChip->bbt[(block >> 3)];
			newval = oldval | (0x2 << (block & 0x06));
			thisChip->bbt[(block >> 3)] = newval;
			if (oldval != newval)
				update = 1;
			block += 2;
		}

		if (update && td->reserved_block_code)
			nand_update_bbt(mtd, (loff_t)(block - 2) << (thisChip->bbt_erase_shift - 1));
	}
}

static void verify_bbt_descr(struct mtd_info *mtd, struct nand_bbt_descr *bd)
{
	if (!bd)
		return;

	u32 pattern_len;
	u32 bits;
	struct nand_chip *thisChip = static_cast<struct nand_chip*>(mtd->priv);

	pattern_len = bd->len;
	bits = bd->options & NAND_BBT_NRBITS_MSK;

	assert((thisChip->bbt_options & NAND_BBT_NO_OOB) &&
			!(thisChip->bbt_options & NAND_BBT_USE_FLASH));
	assert(!bits);

	if (bd->options & NAND_BBT_VERSION)
		pattern_len++;

	if (bd->options & NAND_BBT_NO_OOB) {
		assert(!(thisChip->bbt_options & NAND_BBT_USE_FLASH));
		assert(!(thisChip->bbt_options & NAND_BBT_NO_OOB));
		assert(bd->offs);
		if (bd->options & NAND_BBT_VERSION)
			assert(bd->veroffs != bd->len);
		assert(bd->options & NAND_BBT_SAVECONTENT);
	}

	u32 table_size;

	if (bd->options & NAND_BBT_PERCHIP)
		table_size = thisChip->chipsize >> thisChip->bbt_erase_shift;
	else
		table_size = mtd->size >> thisChip->bbt_erase_shift;

	table_size >>= 3;
	table_size *= bits;

	if (bd->options & NAND_BBT_NO_OOB)
		table_size += pattern_len;

	assert(table_size > (1u << thisChip->bbt_erase_shift));
}

static uint8_t scan_ff_pattern[] = { 0xff, 0xff };

static uint8_t scan_agand_pattern[] = { 0x1C, 0x71, 0xC7, 0x1C, 0x71, 0xC7 };

static uint8_t bbt_pattern[] = {'B', 'b', 't', '0' };
static uint8_t mirror_pattern[] = {'1', 't', 'b', 'B' };

static struct nand_bbt_descr agand_flashbased;

static struct nand_bbt_descr bbt_main_descr;

static struct nand_bbt_descr bbt_mirror_descr;

static struct nand_bbt_descr bbt_main_no_oob_descr;

static struct nand_bbt_descr bbt_mirror_no_oob_descr;

#define BADBLOCK_SCAN_MASK (~NAND_BBT_NO_OOB)

static int nand_create_badblock_pattern(struct nand_chip *thisChip)
{
	if (thisChip->badblock_pattern) {
		kdebugln("Bad block pattern already allocated; not replacing");
		return -static_cast<sint>(ErrNo::EINVAL);
	}
	struct nand_bbt_descr *bd;
	bd = (struct nand_bbt_descr *)kheap.allocate(sizeof(*bd));
	if (!bd)
		return -static_cast<sint>(ErrNo::ENOMEM);
	zero_block(bd, sizeof(*bd));

	bd->options = thisChip->bbt_options & BADBLOCK_SCAN_MASK;
	bd->offs = thisChip->badblockpos;
	bd->len = (thisChip->options & NAND_BUSWIDTH_16) ? 2 : 1;
	bd->pattern = scan_ff_pattern;
	bd->options |= NAND_BBT_DYNAMICSTRUCT;
	thisChip->badblock_pattern = bd;
	return 0;
}

BEG_EXT_C

int nand_scan_bbt(struct mtd_info *mtd, struct nand_bbt_descr *bd)
{
	struct nand_chip *thisChip = static_cast<struct nand_chip*>(mtd->priv);
	int len;
	len = mtd->size >> (thisChip->bbt_erase_shift + 2);

	thisChip->bbt = (u8*)kheap.allocate(len);
	if (!thisChip->bbt)
		return -static_cast<sint>(ErrNo::ENOMEM);

	zero_block(thisChip->bbt, len);

	int res = 0;
	struct nand_bbt_descr *td = thisChip->bbt_td;

	if (!td) {
		if ((res = nand_memory_bbt(mtd, bd))) {
			kdebugln("nand_bbt: can't scan flash and build the RAM-based BBT");
			kheap.free(thisChip->bbt);
			thisChip->bbt = nullptr;
		}
		return res;
	}
	struct nand_bbt_descr *md = thisChip->bbt_md;
	verify_bbt_descr(mtd, td);
	verify_bbt_descr(mtd, md);

	len = (1 << thisChip->bbt_erase_shift);
	len += (len >> thisChip->page_shift) * mtd->oobsize;

	uint8_t *buf;
	buf = (u8*)kheap.allocate(len);
	if (!buf) {
		kheap.free(thisChip->bbt);
		thisChip->bbt = nullptr;
		return -static_cast<sint>(ErrNo::ENOMEM);
	}

	if (td->options & NAND_BBT_ABSPAGE) {
		read_abs_bbts(mtd, buf, td, md);
	} else {

		search_read_bbts(mtd, buf, td, md);
	}

	res = check_create(mtd, buf, bd);

	mark_bbt_region(mtd, td);
	if (md)
		mark_bbt_region(mtd, md);

	kheap.free(buf);
	return res;
}

int nand_update_bbt(struct mtd_info *mtd, loff_t offs)
{
	struct nand_chip *thisChip = static_cast<struct nand_chip*>(mtd->priv);
	struct nand_bbt_descr *td = thisChip->bbt_td;

	if (!thisChip->bbt || !td)
		return -static_cast<sint>(ErrNo::EINVAL);

	uint8_t *buf;

	int len;

	len = (1 << thisChip->bbt_erase_shift);
	len += (len >> thisChip->page_shift) * mtd->oobsize;
	buf = (u8*)kheap.allocate(len);
	if (!buf)
		return -static_cast<sint>(ErrNo::ENOMEM);

	int chip, chipsel;

	if (td->options & NAND_BBT_PERCHIP) {
		chip = (int)(offs >> thisChip->chip_shift);
		chipsel = chip;
	} else {
		chip = 0;
		chipsel = -1;
	}

	td->version[chip]++;
	struct nand_bbt_descr *md = thisChip->bbt_md;
	if (md)
		md->version[chip]++;

	int res = 0;

	if (td->options & NAND_BBT_WRITE) {
		res = write_bbt(mtd, buf, td, md, chipsel);
		if (res < 0)
			goto out;
	}

	if (md && (md->options & NAND_BBT_WRITE)) {
		res = write_bbt(mtd, buf, md, td, chipsel);
	}

 out:
	kheap.free(buf);
	return res;
}

int nand_default_bbt(struct mtd_info *mtd)
{
	struct nand_chip *thisChip = static_cast<struct nand_chip*>(mtd->priv);

	if (thisChip->options & NAND_IS_AND) {

		if (!thisChip->bbt_td) {
			thisChip->bbt_td = &bbt_main_descr;
			thisChip->bbt_md = &bbt_mirror_descr;
		}
		thisChip->bbt_options |= NAND_BBT_USE_FLASH;
		return nand_scan_bbt(mtd, &agand_flashbased);
	}

	if (thisChip->bbt_options & NAND_BBT_USE_FLASH) {

		if (!thisChip->bbt_td) {
			if (thisChip->bbt_options & NAND_BBT_NO_OOB) {
				thisChip->bbt_td = &bbt_main_no_oob_descr;
				thisChip->bbt_md = &bbt_mirror_no_oob_descr;
			} else {
				thisChip->bbt_td = &bbt_main_descr;
				thisChip->bbt_md = &bbt_mirror_descr;
			}
		}
	} else {
		thisChip->bbt_td = nullptr;
		thisChip->bbt_md = nullptr;
	}

	if (!thisChip->badblock_pattern)
		nand_create_badblock_pattern(thisChip);

	return nand_scan_bbt(mtd, thisChip->badblock_pattern);
}

int nand_isbad_bbt(struct mtd_info *mtd, loff_t offs, int allowbbt)
{
	struct nand_chip *thisChip = static_cast<struct nand_chip*>(mtd->priv);
	int block;
	uint8_t res;

	block = (int)(offs >> (thisChip->bbt_erase_shift - 1));
	res = (thisChip->bbt[block >> 3] >> (block & 0x06)) & 0x03;

	kformatln("nand_isbad_bbt(): bbt info for offs 0x%08x: (block %d) 0x%02x",
	      (unsigned int)offs, block >> 1, res);

	switch ((int)res) {
	case 0x00:
		return 0;
	case 0x01:
		return 1;
	case 0x02:
		return allowbbt ? 0 : 1;
	}
	return 1;
}

END_EXT_C

