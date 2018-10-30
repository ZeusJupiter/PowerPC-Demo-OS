/*
 *   File name: nandcore.cpp
 *
 *  Created on: 2017年7月28日, 下午3:38:00
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <io.h>

#include <mk/oscfg.h>

#include <cpu.h>
#include <kernel.h>
#include <mm/heap.h>

#include <linux/port.h>
#include <linux/bitops.h>
#include <linux/mtd/bbm.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_bch.h>
#include <linux/mtd/nand_ecc.h>

#define WATCHDOG_RESET()

static u32 get_timer (u32 base)
{
    u32 t = (u32)KCommKernel::ticks();

    if (t >= base) {
        return  (t - base);
    } else {
        return  (0xffffffff - base + t);
    }
}

#ifndef CONFIG_SYS_NAND_RESET_CNT
#define CONFIG_SYS_NAND_RESET_CNT 200000
#endif

static struct nand_ecclayout nand_oob_8 = { 3, { 1, 2 }, 0,
		{ { 3, 2 }, { 6, 2 } } };

static struct nand_ecclayout nand_oob_16 = { 6, { 1, 2, 3, 6, 7 }, 0,
		{ { 8, 8 } } };

static struct nand_ecclayout nand_oob_64 = { 24, { 40, 41, 42, 43, 44, 45, 46,
		47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 }, 0,
		{ { 2, 35 } } };

static struct nand_ecclayout nand_oob_128 = { 48, { 80, 81, 82, 83, 84, 85, 86,
		87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103,
		104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117,
		118, 119, 120, 121, 122, 123, 124, 125, 126, 127 }, 0, { { 2, 78 } } };

static int nand_get_device(struct nand_chip *chip, struct mtd_info *mtd,
			   int new_state);

static int nand_do_write_oob(struct mtd_info *mtd, loff_t to,
			     struct mtd_oob_ops *ops);

static int nand_wait(struct mtd_info *mtd, struct nand_chip *thisChip);

static int check_offs_len(struct mtd_info *mtd,
					loff_t ofs, uint64_t len)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	int ret = 0;

	if (ofs & ((1 << chip->phys_erase_shift) - 1)) {
		kformatln("%s: Unaligned address", __func__);
		ret = -static_cast<sint>(ErrNo::EINVAL);
	}

	if (len & ((1 << chip->phys_erase_shift) - 1)) {
		kformatln("%s: Length not block aligned", __func__);
		ret = -static_cast<sint>(ErrNo::EINVAL);
	}

	return ret;
}

static void nand_release_device(struct mtd_info *mtd)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);

	chip->select_chip(mtd, -1);
}

static uint8_t nand_read_byte16(struct mtd_info *mtd)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	return (uint8_t) htole16(read16((addr_t)chip->IO_ADDR_R));
}

static u16 nand_read_word(struct mtd_info *mtd)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	return read16((addr_t)chip->IO_ADDR_R);
}

static void nand_select_chip(struct mtd_info *mtd, int chipnr)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);

	switch (chipnr)
	{
	case -1:
		chip->cmd_ctrl(mtd, NAND_CMD_NONE, 0 | NAND_CTRL_CHANGE);
		break;
	case 0:
		break;

	default:
		kformatln("BUG at %s:%d!", __FILE__, __LINE__);
	}
}

static int nand_verify_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);

	for (int i = 0; i < len; i++)
		if (buf[i] != read8((addr_t)chip->IO_ADDR_R))
			return -static_cast<sint>(ErrNo::EFAULT);
	return 0;
}

#define readw(x)   read16((addr_t)(x))
#define writew(v, a) write16((addr_t)(a), v)

static int nand_verify_buf16(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	u16 *p = (u16 *) buf;
	len >>= 1;

	for (int i = 0; i < len; i++)
		if (p[i] != readw(chip->IO_ADDR_R))
			return -static_cast<sint>(ErrNo::EFAULT);

	return 0;
}

static int nand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	int page, chipnr, res = 0, i = 0;
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	u16 bad;

	if (chip->bbt_options & NAND_BBT_SCANLASTPAGE)
		ofs += mtd->erasesize - mtd->writesize;

	page = (int)(ofs >> chip->page_shift) & chip->pagemask;

	if (getchip) {
		chipnr = (int)(ofs >> chip->chip_shift);

		nand_get_device(chip, mtd, FCS_READING);

		chip->select_chip(mtd, chipnr);
	}

	do {
		if (chip->options & NAND_BUSWIDTH_16) {
			chip->cmdfunc(mtd, NAND_CMD_READOOB,
					chip->badblockpos & 0xFE, page);
			bad = htole16(chip->read_word(mtd));
			if (chip->badblockpos & 0x1)
				bad >>= 8;
			else
				bad &= 0xFF;
		} else {
			chip->cmdfunc(mtd, NAND_CMD_READOOB, chip->badblockpos,
					page);
			bad = chip->read_byte(mtd);
		}

		if (likely(chip->badblockbits == 8))
			res = bad != 0xFF;
		else
			res = hweight8(bad) < chip->badblockbits;
		ofs += mtd->writesize;
		page = (int)(ofs >> chip->page_shift) & chip->pagemask;
		i++;
	} while (!res && i < 2 && (chip->bbt_options & NAND_BBT_SCAN2NDPAGE));

	if (getchip)
		nand_release_device(mtd);

	return res;
}

static int nand_default_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	uint8_t buf[2] = { 0, 0 };
	int block, res, ret = 0, i = 0;
	int write_oob = !(chip->bbt_options & NAND_BBT_NO_OOB_BBM);

	if (write_oob) {
		struct erase_info einfo;

		memset(&einfo, 0, sizeof(einfo));
		einfo.mtd = mtd;
		einfo.addr = ofs;
		einfo.len = 1 << chip->phys_erase_shift;
		nand_erase_nand(mtd, &einfo, 0);
	}

	block = (int)(ofs >> chip->bbt_erase_shift);

	if (chip->bbt)
		chip->bbt[block >> 2] |= 0x01 << ((block & 0x03) << 1);

	if (write_oob) {
		struct mtd_oob_ops ops;
		loff_t wr_ofs = ofs;

		nand_get_device(chip, mtd, FCS_WRITING);

		ops.datbuf = NULL;
		ops.oobbuf = buf;
		ops.ooboffs = chip->badblockpos;
		if (chip->options & NAND_BUSWIDTH_16) {
			ops.ooboffs &= ~0x01;
			ops.len = ops.ooblen = 2;
		} else {
			ops.len = ops.ooblen = 1;
		}
		ops.mode = MTD_OPS_PLACE_OOB;

		if (chip->bbt_options & NAND_BBT_SCANLASTPAGE)
			wr_ofs += mtd->erasesize - mtd->writesize;
		do {
			res = nand_do_write_oob(mtd, wr_ofs, &ops);
			if (!ret)
				ret = res;

			i++;
			wr_ofs += mtd->writesize;
		} while ((chip->bbt_options & NAND_BBT_SCAN2NDPAGE) && i < 2);

		nand_release_device(mtd);
	}

	if (chip->bbt_options & NAND_BBT_USE_FLASH) {
		res = nand_update_bbt(mtd, ofs);
		if (!ret)
			ret = res;
	}

	if (!ret)
		mtd->ecc_stats.badblocks++;

	return ret;
}

static int nand_check_wp(struct mtd_info *mtd)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);

	if (chip->options & NAND_BROKEN_XD)
		return 0;

	chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
	return (chip->read_byte(mtd) & NAND_STATUS_WP) ? 0 : 1;
}

static int nand_block_checkbad(struct mtd_info *mtd, loff_t ofs, int getchip,
			       int allowbbt)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);

	if (!(chip->options & NAND_BBT_SCANNED)) {
		chip->options |= NAND_BBT_SCANNED;
		chip->scan_bbt(mtd);
	}

	if (!chip->bbt)
		return chip->block_bad(mtd, ofs, getchip);

	return nand_isbad_bbt(mtd, ofs, allowbbt);
}

static void nand_command(struct mtd_info *mtd, unsigned int command,
			 int column, int page_addr)
{
	register struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	int ctrl = NAND_CTRL_CLE | NAND_CTRL_CHANGE;
	uint32_t rst_sts_cnt = CONFIG_SYS_NAND_RESET_CNT;

	if (command == NAND_CMD_SEQIN) {
		int readcmd;

		if (column >= mtd->writesize) {
	

			column -= mtd->writesize;
			readcmd = NAND_CMD_READOOB;
		} else if (column < 256) {
	

			readcmd = NAND_CMD_READ0;
		} else {
			column -= 256;
			readcmd = NAND_CMD_READ1;
		}
		chip->cmd_ctrl(mtd, readcmd, ctrl);
		ctrl &= ~NAND_CTRL_CHANGE;
	}
	chip->cmd_ctrl(mtd, command, ctrl);

	ctrl = NAND_CTRL_ALE | NAND_CTRL_CHANGE;

	if (column != -1) {

		if (chip->options & NAND_BUSWIDTH_16)
			column >>= 1;
		chip->cmd_ctrl(mtd, column, ctrl);
		ctrl &= ~NAND_CTRL_CHANGE;
	}
	if (page_addr != -1) {
		chip->cmd_ctrl(mtd, page_addr, ctrl);
		ctrl &= ~NAND_CTRL_CHANGE;
		chip->cmd_ctrl(mtd, page_addr >> 8, ctrl);

		if (chip->chipsize > (32 << 20))
			chip->cmd_ctrl(mtd, page_addr >> 16, ctrl);
	}
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

	switch (command) {

	case NAND_CMD_PAGEPROG:
	case NAND_CMD_ERASE1:
	case NAND_CMD_ERASE2:
	case NAND_CMD_SEQIN:
	case NAND_CMD_STATUS:
		return;

	case NAND_CMD_RESET:
		if (chip->dev_ready)
			break;
		CPU::microsecondDelay(chip->chip_delay);
		chip->cmd_ctrl(mtd, NAND_CMD_STATUS,
			       NAND_CTRL_CLE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd,
			       NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
		while (!(chip->read_byte(mtd) & NAND_STATUS_READY) &&
			(rst_sts_cnt--));
		return;

	default:

		if (!chip->dev_ready) {
			CPU::microsecondDelay(chip->chip_delay);
			return;
		}
	}

	CPU::nanosecondDelay(100);

	nand_wait_ready(mtd);
}

#define udelay(x) CPU::microsecondDelay(x)
#define ndelay(x) CPU::nanosecondDelay(x)

static void nand_command_lp(struct mtd_info *mtd, unsigned int command,
			    int column, int page_addr)
{
	register struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	uint32_t rst_sts_cnt = CONFIG_SYS_NAND_RESET_CNT;

	if (command == NAND_CMD_READOOB) {
		column += mtd->writesize;
		command = NAND_CMD_READ0;
	}

	chip->cmd_ctrl(mtd, command & 0xff,
		       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);

	if (column != -1 || page_addr != -1) {
		int ctrl = NAND_CTRL_CHANGE | NAND_NCE | NAND_ALE;

		if (column != -1) {
	

			if (chip->options & NAND_BUSWIDTH_16)
				column >>= 1;
			chip->cmd_ctrl(mtd, column, ctrl);
			ctrl &= ~NAND_CTRL_CHANGE;
			chip->cmd_ctrl(mtd, column >> 8, ctrl);
		}
		if (page_addr != -1) {
			chip->cmd_ctrl(mtd, page_addr, ctrl);
			chip->cmd_ctrl(mtd, page_addr >> 8,
				       NAND_NCE | NAND_ALE);
	

			if (chip->chipsize > (128 << 20))
				chip->cmd_ctrl(mtd, page_addr >> 16,
					       NAND_NCE | NAND_ALE);
		}
	}
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

	switch (command) {

	case NAND_CMD_CACHEDPROG:
	case NAND_CMD_PAGEPROG:
	case NAND_CMD_ERASE1:
	case NAND_CMD_ERASE2:
	case NAND_CMD_SEQIN:
	case NAND_CMD_RNDIN:
	case NAND_CMD_STATUS:
	case NAND_CMD_DEPLETE1:
		return;

	case NAND_CMD_STATUS_ERROR:
	case NAND_CMD_STATUS_ERROR0:
	case NAND_CMD_STATUS_ERROR1:
	case NAND_CMD_STATUS_ERROR2:
	case NAND_CMD_STATUS_ERROR3:

		udelay(chip->chip_delay);
		return;

	case NAND_CMD_RESET:
		if (chip->dev_ready)
			break;
		udelay(chip->chip_delay);
		chip->cmd_ctrl(mtd, NAND_CMD_STATUS,
			       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, NAND_CMD_NONE,
			       NAND_NCE | NAND_CTRL_CHANGE);
		while (!(chip->read_byte(mtd) & NAND_STATUS_READY) &&
			(rst_sts_cnt--));
		return;

	case NAND_CMD_RNDOUT:

		chip->cmd_ctrl(mtd, NAND_CMD_RNDOUTSTART,
			       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, NAND_CMD_NONE,
			       NAND_NCE | NAND_CTRL_CHANGE);
		return;

	case NAND_CMD_READ0:
		chip->cmd_ctrl(mtd, NAND_CMD_READSTART,
			       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, NAND_CMD_NONE,
			       NAND_NCE | NAND_CTRL_CHANGE);

	default:

		if (!chip->dev_ready) {
			udelay(chip->chip_delay);
			return;
		}
	}

	ndelay(100);

	nand_wait_ready(mtd);
}

static int
nand_get_device(struct nand_chip *chip, struct mtd_info *mtd, int new_state)
{
	chip->state = new_state;
	return 0;
}

static int nand_wait(struct mtd_info *mtd, struct nand_chip *chip)
{
	unsigned long	timeo;
	int state = chip->state;
	u32 time_start;

	if (state == FCS_ERASING)
		timeo = (OSCfg::Clock::SystemTicksPreSecond * 400) / 1000;
	else
		timeo = (OSCfg::Clock::SystemTicksPreSecond * 20) / 1000;

	if ((state == FCS_ERASING) && (chip->options & NAND_IS_AND))
		chip->cmdfunc(mtd, NAND_CMD_STATUS_MULTI, -1, -1);
	else
		chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);

	time_start = get_timer(0);

	while (1) {
		if (get_timer(time_start) > timeo) {
			printf("Timeout!\r\n");
			return 0x01;
		}

		if (chip->dev_ready) {
			if (chip->dev_ready(mtd))
				break;
		} else {
			if (chip->read_byte(mtd) & NAND_STATUS_READY)
				break;
		}
	}
#ifdef PPCHAMELON_NAND_TIMER_HACK
	time_start = get_timer(0);
	while (get_timer(time_start) < 10)
		;
#endif

	return (int)chip->read_byte(mtd);
}

static int nand_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
			      uint8_t *buf, int oob_required, int page)
{
	chip->read_buf(mtd, buf, mtd->writesize);
	if (oob_required)
		chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);
	return 0;
}

static int nand_read_page_raw_syndrome(struct mtd_info *mtd,
				       struct nand_chip *chip, uint8_t *buf,
				       int oob_required, int page)
{
	int eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	uint8_t *oob = chip->oob_poi;
	int steps, size;

	for (steps = chip->ecc.steps; steps > 0; steps--) {
		chip->read_buf(mtd, buf, eccsize);
		buf += eccsize;

		if (chip->ecc.prepad) {
			chip->read_buf(mtd, oob, chip->ecc.prepad);
			oob += chip->ecc.prepad;
		}

		chip->read_buf(mtd, oob, eccbytes);
		oob += eccbytes;

		if (chip->ecc.postpad) {
			chip->read_buf(mtd, oob, chip->ecc.postpad);
			oob += chip->ecc.postpad;
		}
	}

	size = mtd->oobsize - (oob - chip->oob_poi);
	if (size)
		chip->read_buf(mtd, oob, size);

	return 0;
}

static int nand_read_page_swecc(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *p = buf;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	uint8_t *ecc_code = chip->buffers->ecccode;
	uint32_t *eccpos = chip->ecc.layout->eccpos;

	chip->ecc.read_page_raw(mtd, chip, buf, 1, page);

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize)
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);

	for (i = 0; i < chip->ecc.total; i++)
		ecc_code[i] = chip->oob_poi[eccpos[i]];

	eccsteps = chip->ecc.steps;
	p = buf;

	for (i = 0 ; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		int stat;

		stat = chip->ecc.correct(mtd, p, &ecc_code[i], &ecc_calc[i]);
		if (stat < 0)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;
	}
	return 0;
}

static int nand_read_subpage(struct mtd_info *mtd, struct nand_chip *chip,
			uint32_t data_offs, uint32_t readlen, uint8_t *bufpoi)
{
	int start_step, end_step, num_steps;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	uint8_t *p;
	int data_col_addr, i, gaps = 0;
	int datafrag_len, eccfrag_len, aligned_len, aligned_pos;
	int busw = (chip->options & NAND_BUSWIDTH_16) ? 2 : 1;
	int index = 0;

	start_step = data_offs / chip->ecc.size;
	end_step = (data_offs + readlen - 1) / chip->ecc.size;
	num_steps = end_step - start_step + 1;

	datafrag_len = num_steps * chip->ecc.size;
	eccfrag_len = num_steps * chip->ecc.bytes;

	data_col_addr = start_step * chip->ecc.size;

	if (data_col_addr != 0)
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, data_col_addr, -1);

	p = bufpoi + data_col_addr;
	chip->read_buf(mtd, p, datafrag_len);

	for (i = 0; i < eccfrag_len ; i += chip->ecc.bytes, p += chip->ecc.size)
		chip->ecc.calculate(mtd, p, &chip->buffers->ecccalc[i]);

	for (i = 0; i < eccfrag_len - 1; i++) {
		if (eccpos[i + start_step * chip->ecc.bytes] + 1 !=
			eccpos[i + start_step * chip->ecc.bytes + 1]) {
			gaps = 1;
			break;
		}
	}
	if (gaps) {
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, mtd->writesize, -1);
		chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);
	} else {

		index = start_step * chip->ecc.bytes;

		aligned_pos = eccpos[index] & ~(busw - 1);
		aligned_len = eccfrag_len;
		if (eccpos[index] & (busw - 1))
			aligned_len++;
		if (eccpos[index + (num_steps * chip->ecc.bytes)] & (busw - 1))
			aligned_len++;

		chip->cmdfunc(mtd, NAND_CMD_RNDOUT,
					mtd->writesize + aligned_pos, -1);
		chip->read_buf(mtd, &chip->oob_poi[aligned_pos], aligned_len);
	}

	for (i = 0; i < eccfrag_len; i++)
		chip->buffers->ecccode[i] = chip->oob_poi[eccpos[i + index]];

	p = bufpoi + data_col_addr;
	for (i = 0; i < eccfrag_len ; i += chip->ecc.bytes, p += chip->ecc.size) {
		int stat;

		stat = chip->ecc.correct(mtd, p,
			&chip->buffers->ecccode[i], &chip->buffers->ecccalc[i]);
		if (stat < 0)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;
	}
	return 0;
}

static int nand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *p = buf;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	uint8_t *ecc_code = chip->buffers->ecccode;
	uint32_t *eccpos = chip->ecc.layout->eccpos;

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		chip->ecc.hwctl(mtd, NAND_ECC_READ);
		chip->read_buf(mtd, p, eccsize);
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);
	}
	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	for (i = 0; i < chip->ecc.total; i++)
		ecc_code[i] = chip->oob_poi[eccpos[i]];

	eccsteps = chip->ecc.steps;
	p = buf;

	for (i = 0 ; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		int stat;

		stat = chip->ecc.correct(mtd, p, &ecc_code[i], &ecc_calc[i]);
		if (stat < 0)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;
	}
	return 0;
}

static int nand_read_page_hwecc_oob_first(struct mtd_info *mtd,
	struct nand_chip *chip, uint8_t *buf, int oob_required, int page)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *p = buf;
	uint8_t *ecc_code = chip->buffers->ecccode;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	uint8_t *ecc_calc = chip->buffers->ecccalc;

	chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page);
	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);

	for (i = 0; i < chip->ecc.total; i++)
		ecc_code[i] = chip->oob_poi[eccpos[i]];

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		int stat;

		chip->ecc.hwctl(mtd, NAND_ECC_READ);
		chip->read_buf(mtd, p, eccsize);
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);

		stat = chip->ecc.correct(mtd, p, &ecc_code[i], NULL);
		if (stat < 0)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;
	}
	return 0;
}

static int nand_read_page_syndrome(struct mtd_info *mtd, struct nand_chip *chip,
				   uint8_t *buf, int oob_required, int page)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *p = buf;
	uint8_t *oob = chip->oob_poi;

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		int stat;

		chip->ecc.hwctl(mtd, NAND_ECC_READ);
		chip->read_buf(mtd, p, eccsize);

		if (chip->ecc.prepad) {
			chip->read_buf(mtd, oob, chip->ecc.prepad);
			oob += chip->ecc.prepad;
		}

		chip->ecc.hwctl(mtd, NAND_ECC_READSYN);
		chip->read_buf(mtd, oob, eccbytes);
		stat = chip->ecc.correct(mtd, p, oob, NULL);

		if (stat < 0)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;

		oob += eccbytes;

		if (chip->ecc.postpad) {
			chip->read_buf(mtd, oob, chip->ecc.postpad);
			oob += chip->ecc.postpad;
		}
	}

	i = mtd->oobsize - (oob - chip->oob_poi);
	if (i)
		chip->read_buf(mtd, oob, i);

	return 0;
}

static uint8_t *nand_transfer_oob(struct nand_chip *chip, uint8_t *oob,
				  struct mtd_oob_ops *ops, size_t len)
{
	switch (ops->mode) {

	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_RAW:
		memcpy(oob, chip->oob_poi + ops->ooboffs, len);
		return oob + len;

	case MTD_OPS_AUTO_OOB: {
		struct nand_oobfree *free = chip->ecc.layout->oobfree;
		uint32_t boffs = 0, roffs = ops->ooboffs;
		size_t bytes = 0;

		for (; free->length && len; free++, len -= bytes) {
	

			if (unlikely(roffs)) {
				if (roffs >= free->length) {
					roffs -= free->length;
					continue;
				}
				boffs = free->offset + roffs;
				bytes = min_t(size_t, len,
					      (free->length - roffs));
				roffs = 0;
			} else {
				bytes = min_t(size_t, len, free->length);
				boffs = free->offset;
			}
			memcpy(oob, chip->oob_poi + boffs, bytes);
			oob += bytes;
		}
		return oob;
	}
	default:
		kformatln("BUG at %s:%d", __FILE__, __LINE__);
	}
	return nullptr;
}

static int nand_do_read_ops(struct mtd_info *mtd, loff_t from,
			    struct mtd_oob_ops *ops)
{
	int chipnr, page, realpage, col, bytes, aligned, oob_required;
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	struct mtd_ecc_stats stats;
	int ret = 0;
	uint32_t readlen = ops->len;
	uint32_t oobreadlen = ops->ooblen;
	uint32_t max_oobsize = ops->mode == MTD_OPS_AUTO_OOB ?
		mtd->oobavail : mtd->oobsize;

	uint8_t *bufpoi, *oob, *buf;
	unsigned int max_bitflips = 0;

	stats = mtd->ecc_stats;

	chipnr = (int)(from >> chip->chip_shift);
	chip->select_chip(mtd, chipnr);

	realpage = (int)(from >> chip->page_shift);
	page = realpage & chip->pagemask;

	col = (int)(from & (mtd->writesize - 1));

	buf = ops->datbuf;
	oob = ops->oobbuf;
	oob_required = oob ? 1 : 0;

	while (1) {
		WATCHDOG_RESET();

		bytes = min(mtd->writesize - col, readlen);
		aligned = (bytes == mtd->writesize);

		if (realpage != chip->pagebuf || oob) {
			bufpoi = aligned ? buf : chip->buffers->databuf;

			chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, page);

	

			if (unlikely(ops->mode == MTD_OPS_RAW))
				ret = chip->ecc.read_page_raw(mtd, chip, bufpoi,
							      oob_required,
							      page);
			else if (!aligned && NAND_HAS_SUBPAGE_READ(chip) &&
			    !oob)
				ret = chip->ecc.read_subpage(mtd, chip,
							col, bytes, bufpoi);
			else
				ret = chip->ecc.read_page(mtd, chip, bufpoi,
							  oob_required, page);
			if (ret < 0) {
				if (!aligned)
			

					chip->pagebuf = -1;
				break;
			}

			max_bitflips = max_t(unsigned int, max_bitflips, ret);

	

			if (!aligned) {
				if (!NAND_HAS_SUBPAGE_READ(chip) && !oob &&
				    !(mtd->ecc_stats.failed - stats.failed) &&
				    (ops->mode != MTD_OPS_RAW)) {
					chip->pagebuf = realpage;
					chip->pagebuf_bitflips = ret;
				} else {
			

					chip->pagebuf = -1;
				}
				memcpy(buf, chip->buffers->databuf + col, bytes);
			}

			buf += bytes;

			if (unlikely(oob)) {
				int toread = min(oobreadlen, max_oobsize);

				if (toread) {
					oob = nand_transfer_oob(chip,
						oob, ops, toread);
					oobreadlen -= toread;
				}
			}
		} else {
			memcpy(buf, chip->buffers->databuf + col, bytes);
			buf += bytes;
			max_bitflips = max_t(unsigned int, max_bitflips,
					     chip->pagebuf_bitflips);
		}

		readlen -= bytes;

		if (!readlen)
			break;

		col = 0;

		realpage++;

		page = realpage & chip->pagemask;

		if (!page) {
			chipnr++;
			chip->select_chip(mtd, -1);
			chip->select_chip(mtd, chipnr);
		}
	}

	ops->retlen = ops->len - (size_t) readlen;
	if (oob)
		ops->oobretlen = ops->ooblen - oobreadlen;

	if (ret)
		return ret;

	if (mtd->ecc_stats.failed - stats.failed)
		return -static_cast<sint>(ErrNo::EBADMSG);

	return max_bitflips;
}

static int nand_read(struct mtd_info *mtd, loff_t from, size_t len,
		     size_t *retlen, uint8_t *buf)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	struct mtd_oob_ops ops;
	int ret;

	nand_get_device(chip, mtd, FCS_READING);
	ops.len = len;
	ops.datbuf = buf;
	ops.oobbuf = NULL;
	ops.mode = MTD_OPS_PLACE_OOB;
	ret = nand_do_read_ops(mtd, from, &ops);
	*retlen = ops.retlen;
	nand_release_device(mtd);
	return ret;
}

static int nand_read_oob_std(struct mtd_info *mtd, struct nand_chip *chip,
			     int page)
{
	chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page);
	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);
	return 0;
}

static int nand_read_oob_syndrome(struct mtd_info *mtd, struct nand_chip *chip,
				  int page)
{
	uint8_t *buf = chip->oob_poi;
	int length = mtd->oobsize;
	int chunk = chip->ecc.bytes + chip->ecc.prepad + chip->ecc.postpad;
	int eccsize = chip->ecc.size;
	uint8_t *bufpoi = buf;
	int i, toread, sndrnd = 0, pos;

	chip->cmdfunc(mtd, NAND_CMD_READ0, chip->ecc.size, page);
	for (i = 0; i < chip->ecc.steps; i++) {
		if (sndrnd) {
			pos = eccsize + i * (eccsize + chunk);
			if (mtd->writesize > 512)
				chip->cmdfunc(mtd, NAND_CMD_RNDOUT, pos, -1);
			else
				chip->cmdfunc(mtd, NAND_CMD_READ0, pos, page);
		} else
			sndrnd = 1;
		toread = min_t(int, length, chunk);
		chip->read_buf(mtd, bufpoi, toread);
		bufpoi += toread;
		length -= toread;
	}
	if (length > 0)
		chip->read_buf(mtd, bufpoi, length);

	return 0;
}

static int nand_write_oob_std(struct mtd_info *mtd, struct nand_chip *chip,
			      int page)
{
	int status = 0;
	const uint8_t *buf = chip->oob_poi;
	int length = mtd->oobsize;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, mtd->writesize, page);
	chip->write_buf(mtd, buf, length);

	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

	status = chip->waitfunc(mtd, chip);

	return status & NAND_STATUS_FAIL ? -static_cast<sint>(ErrNo::EIO) : 0;
}

static int nand_write_oob_syndrome(struct mtd_info *mtd,
				   struct nand_chip *chip, int page)
{
	int chunk = chip->ecc.bytes + chip->ecc.prepad + chip->ecc.postpad;
	int eccsize = chip->ecc.size, length = mtd->oobsize;
	int i, len, pos, status = 0, sndcmd = 0, steps = chip->ecc.steps;
	const uint8_t *bufpoi = chip->oob_poi;

	if (!chip->ecc.prepad && !chip->ecc.postpad) {
		pos = steps * (eccsize + chunk);
		steps = 0;
	} else
		pos = eccsize;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, pos, page);
	for (i = 0; i < steps; i++) {
		if (sndcmd) {
			if (mtd->writesize <= 512) {
				uint32_t fill = 0xFFFFFFFF;

				len = eccsize;
				while (len > 0) {
					int num = min_t(int, len, 4);
					chip->write_buf(mtd, (uint8_t *)&fill,
							num);
					len -= num;
				}
			} else {
				pos = eccsize + i * (eccsize + chunk);
				chip->cmdfunc(mtd, NAND_CMD_RNDIN, pos, -1);
			}
		} else
			sndcmd = 1;
		len = min_t(int, length, chunk);
		chip->write_buf(mtd, bufpoi, len);
		bufpoi += len;
		length -= len;
	}
	if (length > 0)
		chip->write_buf(mtd, bufpoi, length);

	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
	status = chip->waitfunc(mtd, chip);

	return status & NAND_STATUS_FAIL ? -static_cast<sint>(ErrNo::EIO) : 0;
}

static int nand_do_read_oob(struct mtd_info *mtd, loff_t from,
			    struct mtd_oob_ops *ops)
{
	int page, realpage, chipnr;
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	struct mtd_ecc_stats stats;
	int readlen = ops->ooblen;
	int len;
	uint8_t *buf = ops->oobbuf;
	int ret = 0;

	kformatln("%s: from = 0x%08Lx, len = %i",
			__func__, (unsigned long long)from, readlen);

	stats = mtd->ecc_stats;

	if (ops->mode == MTD_OPS_AUTO_OOB)
		len = chip->ecc.layout->oobavail;
	else
		len = mtd->oobsize;

	if (unlikely(ops->ooboffs >= len)) {
		kformatln("%s: Attempt to start read "
					"outside oob", __func__);
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	if (unlikely(from >= mtd->size ||
	 ops->ooboffs + readlen > ((mtd->size >> chip->page_shift) - (from >> chip->page_shift)) * len))
	{
		kformatln("%s: Attempt read beyond end " "of device", __func__);
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	chipnr = (int)(from >> chip->chip_shift);
	chip->select_chip(mtd, chipnr);

	realpage = (int)(from >> chip->page_shift);
	page = realpage & chip->pagemask;

	while (1) {
		WATCHDOG_RESET();
		if (ops->mode == MTD_OPS_RAW)
			ret = chip->ecc.read_oob_raw(mtd, chip, page);
		else
			ret = chip->ecc.read_oob(mtd, chip, page);

		if (ret < 0)
			break;

		len = min(len, readlen);
		buf = nand_transfer_oob(chip, buf, ops, len);

		readlen -= len;
		if (!readlen)
			break;

		realpage++;

		page = realpage & chip->pagemask;

		if (!page) {
			chipnr++;
			chip->select_chip(mtd, -1);
			chip->select_chip(mtd, chipnr);
		}
	}

	ops->oobretlen = ops->ooblen - readlen;

	if (ret < 0)
		return ret;

	if (mtd->ecc_stats.failed - stats.failed)
		return -static_cast<sint>(ErrNo::EBADMSG);

	return  mtd->ecc_stats.corrected - stats.corrected ? -static_cast<sint>(ErrNo::EUCLEAN) : 0;
}

static int nand_read_oob(struct mtd_info *mtd, loff_t from,
			 struct mtd_oob_ops *ops)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	int ret = -static_cast<sint>(ErrNo::ENOTSUP);

	ops->retlen = 0;

	if (ops->datbuf && (from + ops->len) > mtd->size) {
		kformatln("%s: Attempt read beyond end of device", __func__);
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	nand_get_device(chip, mtd, FCS_READING);

	switch (ops->mode) {
	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_AUTO_OOB:
	case MTD_OPS_RAW:
		break;

	default:
		goto out;
	}

	if (!ops->datbuf)
		ret = nand_do_read_oob(mtd, from, ops);
	else
		ret = nand_do_read_ops(mtd, from, ops);

out:
	nand_release_device(mtd);
	return ret;
}

static int nand_write_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
				const uint8_t *buf, int oob_required)
{
	chip->write_buf(mtd, buf, mtd->writesize);
	if (oob_required)
		chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);

	return 0;
}

static int nand_write_page_raw_syndrome(struct mtd_info *mtd,
					struct nand_chip *chip,
					const uint8_t *buf, int oob_required)
{
	int eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	uint8_t *oob = chip->oob_poi;
	int steps, size;

	for (steps = chip->ecc.steps; steps > 0; steps--) {
		chip->write_buf(mtd, buf, eccsize);
		buf += eccsize;

		if (chip->ecc.prepad) {
			chip->write_buf(mtd, oob, chip->ecc.prepad);
			oob += chip->ecc.prepad;
		}

		chip->read_buf(mtd, oob, eccbytes);
		oob += eccbytes;

		if (chip->ecc.postpad) {
			chip->write_buf(mtd, oob, chip->ecc.postpad);
			oob += chip->ecc.postpad;
		}
	}

	size = mtd->oobsize - (oob - chip->oob_poi);
	if (size)
		chip->write_buf(mtd, oob, size);

	return 0;
}

static int nand_write_page_swecc(struct mtd_info *mtd, struct nand_chip *chip,
				  const uint8_t *buf, int oob_required)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	const uint8_t *p = buf;
	uint32_t *eccpos = chip->ecc.layout->eccpos;

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize)
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);

	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ecc_calc[i];

	return chip->ecc.write_page_raw(mtd, chip, buf, 1);
}

static int nand_write_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
				  const uint8_t *buf, int oob_required)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	const uint8_t *p = buf;
	uint32_t *eccpos = chip->ecc.layout->eccpos;

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
		chip->write_buf(mtd, p, eccsize);
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);
	}

	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ecc_calc[i];

	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);

	return 0;
}

static int nand_write_page_syndrome(struct mtd_info *mtd,
				    struct nand_chip *chip,
				    const uint8_t *buf, int oob_required)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	const uint8_t *p = buf;
	uint8_t *oob = chip->oob_poi;

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {

		chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
		chip->write_buf(mtd, p, eccsize);

		if (chip->ecc.prepad) {
			chip->write_buf(mtd, oob, chip->ecc.prepad);
			oob += chip->ecc.prepad;
		}

		chip->ecc.calculate(mtd, p, oob);
		chip->write_buf(mtd, oob, eccbytes);
		oob += eccbytes;

		if (chip->ecc.postpad) {
			chip->write_buf(mtd, oob, chip->ecc.postpad);
			oob += chip->ecc.postpad;
		}
	}

	i = mtd->oobsize - (oob - chip->oob_poi);
	if (i)
		chip->write_buf(mtd, oob, i);

	return 0;
}

static int nand_write_page(struct mtd_info *mtd, struct nand_chip *chip,
			   const uint8_t *buf, int oob_required, int page,
			   int cached, int raw)
{
	int status;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, page);

	if (unlikely(raw))
		status = chip->ecc.write_page_raw(mtd, chip, buf, oob_required);
	else
		status = chip->ecc.write_page(mtd, chip, buf, oob_required);

	if (status < 0)
		return status;

	cached = 0;

	if (!cached || !(chip->options & NAND_CACHEPRG)) {

		chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
		status = chip->waitfunc(mtd, chip);

		if ((status & NAND_STATUS_FAIL) && (chip->errstat))
			status = chip->errstat(mtd, chip, FCS_WRITING, status,
					       page);

		if (status & NAND_STATUS_FAIL)
			return -static_cast<sint>(ErrNo::EIO);
	} else {
		chip->cmdfunc(mtd, NAND_CMD_CACHEDPROG, -1, -1);
		status = chip->waitfunc(mtd, chip);
	}

#ifdef CONFIG_MTD_NAND_VERIFY_WRITE

	chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);

	if (chip->verify_buf(mtd, buf, mtd->writesize))
		return -EIO;

	chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
#endif
	return 0;
}

static uint8_t *nand_fill_oob(struct mtd_info *mtd, uint8_t *oob, size_t len,
			      struct mtd_oob_ops *ops)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);

	memset(chip->oob_poi, 0xff, mtd->oobsize);

	switch (ops->mode) {

	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_RAW:
		memcpy(chip->oob_poi + ops->ooboffs, oob, len);
		return oob + len;

	case MTD_OPS_AUTO_OOB: {
		struct nand_oobfree *free = chip->ecc.layout->oobfree;
		uint32_t boffs = 0, woffs = ops->ooboffs;
		size_t bytes = 0;

		for (; free->length && len; free++, len -= bytes) {
	

			if (unlikely(woffs)) {
				if (woffs >= free->length) {
					woffs -= free->length;
					continue;
				}
				boffs = free->offset + woffs;
				bytes = min_t(size_t, len,
					      (free->length - woffs));
				woffs = 0;
			} else {
				bytes = min_t(size_t, len, free->length);
				boffs = free->offset;
			}
			memcpy(chip->oob_poi + boffs, oob, bytes);
			oob += bytes;
		}
		return oob;
	}
	default:
		kformatln("BUG at %s:%d", __FILE__, __LINE__);
	}
	return nullptr;
}

#define NOTALIGNED(x)	((x & (chip->subpagesize - 1)) != 0)

static int nand_do_write_ops(struct mtd_info *mtd, loff_t to,
			     struct mtd_oob_ops *ops)
{
	int chipnr, realpage, page, blockmask, column;
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	uint32_t writelen = ops->len;

	uint32_t oobwritelen = ops->ooblen;
	uint32_t oobmaxlen = ops->mode == MTD_OPS_AUTO_OOB ?
				mtd->oobavail : mtd->oobsize;

	uint8_t *oob = ops->oobbuf;
	uint8_t *buf = ops->datbuf;
	int ret, subpage;
	int oob_required = oob ? 1 : 0;

	ops->retlen = 0;
	if (!writelen)
		return 0;

	column = to & (mtd->writesize - 1);
	subpage = column || (writelen & (mtd->writesize - 1));

	if (subpage && oob)
		return -static_cast<sint>(ErrNo::EINVAL);

	chipnr = (int)(to >> chip->chip_shift);
	chip->select_chip(mtd, chipnr);

	if (nand_check_wp(mtd)) {
		printf ("nand_do_write_ops: Device is write protected");
		return -static_cast<sint>(ErrNo::EIO);
	}

	realpage = (int)(to >> chip->page_shift);
	page = realpage & chip->pagemask;
	blockmask = (1 << (chip->phys_erase_shift - chip->page_shift)) - 1;

	if (to <= (chip->pagebuf << chip->page_shift) &&
	    (chip->pagebuf << chip->page_shift) < (to + ops->len))
		chip->pagebuf = -1;

	if (oob && ops->ooboffs && (ops->ooboffs + ops->ooblen > oobmaxlen))
		return -static_cast<sint>(ErrNo::EINVAL);

	while (1) {
		WATCHDOG_RESET();

		int bytes = mtd->writesize;
		int cached = writelen > bytes && page != blockmask;
		uint8_t *wbuf = buf;

		if (unlikely(column || writelen < mtd->writesize)) {
			cached = 0;
			bytes = min_t(int, bytes - column, (int) writelen);
			chip->pagebuf = -1;
			memset(chip->buffers->databuf, 0xff, mtd->writesize);
			memcpy(&chip->buffers->databuf[column], buf, bytes);
			wbuf = chip->buffers->databuf;
		}

		if (unlikely(oob)) {
			size_t len = min(oobwritelen, oobmaxlen);
			oob = nand_fill_oob(mtd, oob, len, ops);
			oobwritelen -= len;
		} else {
	

			memset(chip->oob_poi, 0xff, mtd->oobsize);
		}

		ret = chip->write_page(mtd, chip, wbuf, oob_required, page,
				       cached, (ops->mode == MTD_OPS_RAW));
		if (ret)
			break;

		writelen -= bytes;
		if (!writelen)
			break;

		column = 0;
		buf += bytes;
		realpage++;

		page = realpage & chip->pagemask;

		if (!page) {
			chipnr++;
			chip->select_chip(mtd, -1);
			chip->select_chip(mtd, chipnr);
		}
	}

	ops->retlen = ops->len - writelen;
	if (unlikely(oob))
		ops->oobretlen = ops->ooblen;
	return ret;
}

static int nand_write(struct mtd_info *mtd, loff_t to, size_t len,
			  size_t *retlen, const uint8_t *buf)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	struct mtd_oob_ops ops;
	int ret;

	nand_get_device(chip, mtd, FCS_WRITING);
	ops.len = len;
	ops.datbuf = (uint8_t *)buf;
	ops.oobbuf = nullptr;
	ops.mode = MTD_OPS_PLACE_OOB;
	ret = nand_do_write_ops(mtd, to, &ops);
	*retlen = ops.retlen;
	nand_release_device(mtd);
	return ret;
}

static int nand_do_write_oob(struct mtd_info *mtd, loff_t to,
			     struct mtd_oob_ops *ops)
{
	int chipnr, page, status, len;
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);

	kformatln("%s: to = 0x%08x, len = %i",
			 __func__, (unsigned int)to, (int)ops->ooblen);

	if (ops->mode == MTD_OPS_AUTO_OOB)
		len = chip->ecc.layout->oobavail;
	else
		len = mtd->oobsize;

	if ((ops->ooboffs + ops->ooblen) > len) {
		kformatln("%s: Attempt to write past end of page", __func__);
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	if (unlikely(ops->ooboffs >= len)) {
		kformatln("%s: Attempt to start write outside oob", __func__);
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	if (unlikely(to >= mtd->size ||
		     ops->ooboffs + ops->ooblen >
			((mtd->size >> chip->page_shift) -
			 (to >> chip->page_shift)) * len)) {
		kformatln("%s: Attempt write beyond end of device", __func__);
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	chipnr = (int)(to >> chip->chip_shift);
	chip->select_chip(mtd, chipnr);

	page = (int)(to >> chip->page_shift);

	chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);

	if (nand_check_wp(mtd))
		return -static_cast<sint>(ErrNo::EROFS);

	if (page == chip->pagebuf)
		chip->pagebuf = -1;

	nand_fill_oob(mtd, ops->oobbuf, ops->ooblen, ops);

	if (ops->mode == MTD_OPS_RAW)
		status = chip->ecc.write_oob_raw(mtd, chip, page & chip->pagemask);
	else
		status = chip->ecc.write_oob(mtd, chip, page & chip->pagemask);

	if (status)
		return status;

	ops->oobretlen = ops->ooblen;

	return 0;
}

static int nand_write_oob(struct mtd_info *mtd, loff_t to,
			  struct mtd_oob_ops *ops)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	int ret = -static_cast<sint>(ErrNo::ENOTSUP);

	ops->retlen = 0;

	if (ops->datbuf && (to + ops->len) > mtd->size) {
		kformatln("%s: Attempt write beyond end of device", __func__);
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	nand_get_device(chip, mtd, FCS_WRITING);

	switch (ops->mode) {
	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_AUTO_OOB:
	case MTD_OPS_RAW:
		break;

	default:
		goto out;
	}

	if (!ops->datbuf)
		ret = nand_do_write_oob(mtd, to, ops);
	else
		ret = nand_do_write_ops(mtd, to, ops);

out:
	nand_release_device(mtd);
	return ret;
}

static void single_erase_cmd(struct mtd_info *mtd, int page)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);

	chip->cmdfunc(mtd, NAND_CMD_ERASE1, -1, page);
	chip->cmdfunc(mtd, NAND_CMD_ERASE2, -1, -1);
}

static void multi_erase_cmd(struct mtd_info *mtd, int page)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);

	chip->cmdfunc(mtd, NAND_CMD_ERASE1, -1, page++);
	chip->cmdfunc(mtd, NAND_CMD_ERASE1, -1, page++);
	chip->cmdfunc(mtd, NAND_CMD_ERASE1, -1, page++);
	chip->cmdfunc(mtd, NAND_CMD_ERASE1, -1, page);
	chip->cmdfunc(mtd, NAND_CMD_ERASE2, -1, -1);
}

static int nand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	return nand_erase_nand(mtd, instr, 0);
}

static void nand_sync(struct mtd_info *mtd)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);

	kformatln("%s: called", __func__);

	nand_get_device(chip, mtd, FCS_SYNCING);

	nand_release_device(mtd);
}

static int nand_block_isbad(struct mtd_info *mtd, loff_t offs)
{
	return nand_block_checkbad(mtd, offs, 1, 0);
}

static int nand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	int ret;

	ret = nand_block_isbad(mtd, ofs);
	if (ret) {

		if (ret > 0)
			return 0;
		return ret;
	}

	return chip->block_markbad(mtd, ofs);
}

static int nand_onfi_set_features(struct mtd_info *mtd, struct nand_chip *chip,
			int addr, uint8_t *subfeature_param)
{
	int status;

	if (!chip->onfi_version)
		return -static_cast<sint>(ErrNo::EINVAL);

	chip->cmdfunc(mtd, NAND_CMD_SET_FEATURES, addr, -1);
	chip->write_buf(mtd, subfeature_param, ONFI_SUBFEATURE_PARAM_LEN);
	status = chip->waitfunc(mtd, chip);
	if (status & NAND_STATUS_FAIL)
		return -static_cast<sint>(ErrNo::EIO);
	return 0;
}

static int nand_onfi_get_features(struct mtd_info *mtd, struct nand_chip *chip,
			int addr, uint8_t *subfeature_param)
{
	if (!chip->onfi_version)
		return -static_cast<sint>(ErrNo::EINVAL);

	memset(subfeature_param, 0, ONFI_SUBFEATURE_PARAM_LEN);

	chip->cmdfunc(mtd, NAND_CMD_GET_FEATURES, addr, -1);
	chip->read_buf(mtd, subfeature_param, ONFI_SUBFEATURE_PARAM_LEN);
	return 0;
}

static void nand_set_defaults(struct nand_chip *chip, int busw)
{

	if (!chip->chip_delay)
		chip->chip_delay = 20;

	if (chip->cmdfunc == nullptr)
		chip->cmdfunc = nand_command;

	if (chip->waitfunc == nullptr)
		chip->waitfunc = nand_wait;

	if (!chip->select_chip)
		chip->select_chip = nand_select_chip;
	if (!chip->read_byte)
		chip->read_byte = busw ? nand_read_byte16 : nand_read_byte;
	if (!chip->read_word)
		chip->read_word = nand_read_word;
	if (!chip->block_bad)
		chip->block_bad = nand_block_bad;
	if (!chip->block_markbad)
		chip->block_markbad = nand_default_block_markbad;
	if (!chip->write_buf)
		chip->write_buf = busw ? nand_write_buf16 : nand_write_buf;
	if (!chip->read_buf)
		chip->read_buf = busw ? nand_read_buf16 : nand_read_buf;
	if (!chip->verify_buf)
		chip->verify_buf = busw ? nand_verify_buf16 : nand_verify_buf;
	if (!chip->scan_bbt)
		chip->scan_bbt = nand_default_bbt;
	if (!chip->controller)
		chip->controller = &chip->hwcontrol;
}

#ifdef CONFIG_SYS_NAND_ONFI_DETECTION

static void sanitize_string(char *s, size_t len)
{
	ssize_t i;

	s[len - 1] = 0;

	for (i = 0; i < len - 1; i++) {
		if (s[i] < ' ' || s[i] > 127)
			s[i] = '?';
	}

	strim(s);
}

static u16 onfi_crc16(u16 crc, u8 const *p, size_t len)
{
	int i;
	while (len--) {
		crc ^= *p++ << 8;
		for (i = 0; i < 8; i++)
			crc = (crc << 1) ^ ((crc & 0x8000) ? 0x8005 : 0);
	}

	return crc;
}

static int nand_flash_detect_onfi(struct mtd_info *mtd, struct nand_chip *chip,
					int *busw)
{
	struct nand_onfi_params *p = &chip->onfi_params;
	int i;
	int val;

	chip->cmdfunc(mtd, NAND_CMD_READID, 0x20, -1);
	if (chip->read_byte(mtd) != 'O' || chip->read_byte(mtd) != 'N' ||
		chip->read_byte(mtd) != 'F' || chip->read_byte(mtd) != 'I')
		return 0;

	chip->cmdfunc(mtd, NAND_CMD_PARAM, 0, -1);
	for (i = 0; i < 3; i++) {
		chip->read_buf(mtd, (uint8_t *)p, sizeof(*p));
		if (onfi_crc16(ONFI_CRC_BASE, (uint8_t *)p, 254) ==
				le16toh(p->crc)) {
			kformatln("ONFI param page %d valid", i);
			break;
		}
	}

	if (i == 3)
		return 0;

	val = le16toh(p->revision);
	if (val & (1 << 5))
		chip->onfi_version = 23;
	else if (val & (1 << 4))
		chip->onfi_version = 22;
	else if (val & (1 << 3))
		chip->onfi_version = 21;
	else if (val & (1 << 2))
		chip->onfi_version = 20;
	else if (val & (1 << 1))
		chip->onfi_version = 10;
	else
		chip->onfi_version = 0;

	if (!chip->onfi_version) {
		kformatln("%s: unsupported ONFI version: %d", __func__, val);
		return 0;
	}

	sanitize_string(p->manufacturer, sizeof(p->manufacturer));
	sanitize_string(p->model, sizeof(p->model));
	if (!mtd->name)
		mtd->name = p->model;
	mtd->writesize = le32toh(p->byte_per_page);
	mtd->erasesize = le32toh(p->pages_per_block) * mtd->writesize;
	mtd->oobsize = le16toh(p->spare_bytes_per_page);
	chip->chipsize = le32toh(p->blocks_per_lun);
	chip->chipsize *= (uint64_t)mtd->erasesize * p->lun_count;
	*busw = 0;
	if (le16toh(p->features) & 1)
		*busw = NAND_BUSWIDTH_16;

	kdebugln("ONFI flash detected");
	return 1;
}
#else
static inline int nand_flash_detect_onfi(struct mtd_info *mtd,
					struct nand_chip *chip,
					int *busw)
{
	return 0;
}
#endif

static int nand_id_has_period(u8 *id_data, int arrlen, int period)
{
	int i, j;
	for (i = 0; i < period; i++)
		for (j = i + period; j < arrlen; j += period)
			if (id_data[i] != id_data[j])
				return 0;
	return 1;
}

static int nand_id_len(u8 *id_data, int arrlen)
{
	int last_nonzero, period;

	for (last_nonzero = arrlen - 1; last_nonzero >= 0; last_nonzero--)
		if (id_data[last_nonzero])
			break;

	if (last_nonzero < 0)
		return 0;

	for (period = 1; period < arrlen; period++)
		if (nand_id_has_period(id_data, arrlen, period))
			break;

	if (period < arrlen)
		return period;

	if (last_nonzero < arrlen - 1)
		return last_nonzero + 1;

	return arrlen;
}

static void nand_decode_ext_id(struct mtd_info *mtd, struct nand_chip *chip,
				u8 id_data[8], int *busw)
{
	int extid, id_len;

	chip->cellinfo = id_data[2];

	extid = id_data[3];

	id_len = nand_id_len(id_data, 8);

	if (id_len == 6 && id_data[0] == NAND_MFR_SAMSUNG &&
			(chip->cellinfo & NAND_CI_CELLTYPE_MSK) &&
			id_data[5] != 0x00) {

		mtd->writesize = 2048 << (extid & 0x03);
		extid >>= 2;

		switch (((extid >> 2) & 0x04) | (extid & 0x03)) {
		case 1:
			mtd->oobsize = 128;
			break;
		case 2:
			mtd->oobsize = 218;
			break;
		case 3:
			mtd->oobsize = 400;
			break;
		case 4:
			mtd->oobsize = 436;
			break;
		case 5:
			mtd->oobsize = 512;
			break;
		case 6:
		default:

			mtd->oobsize = 640;
			break;
		}
		extid >>= 2;

		mtd->erasesize = (128 * 1024) <<
			(((extid >> 1) & 0x04) | (extid & 0x03));
		*busw = 0;
	} else if (id_len == 6 && id_data[0] == NAND_MFR_HYNIX &&
			(chip->cellinfo & NAND_CI_CELLTYPE_MSK)) {
		unsigned int tmp;

		mtd->writesize = 2048 << (extid & 0x03);
		extid >>= 2;

		switch (((extid >> 2) & 0x04) | (extid & 0x03)) {
		case 0:
			mtd->oobsize = 128;
			break;
		case 1:
			mtd->oobsize = 224;
			break;
		case 2:
			mtd->oobsize = 448;
			break;
		case 3:
			mtd->oobsize = 64;
			break;
		case 4:
			mtd->oobsize = 32;
			break;
		case 5:
			mtd->oobsize = 16;
			break;
		default:
			mtd->oobsize = 640;
			break;
		}
		extid >>= 2;

		tmp = ((extid >> 1) & 0x04) | (extid & 0x03);
		if (tmp < 0x03)
			mtd->erasesize = (128 * 1024) << tmp;
		else if (tmp == 0x03)
			mtd->erasesize = 768 * 1024;
		else
			mtd->erasesize = (64 * 1024) << tmp;
		*busw = 0;
	} else {

		mtd->writesize = 1024 << (extid & 0x03);
		extid >>= 2;

		mtd->oobsize = (8 << (extid & 0x01)) *
			(mtd->writesize >> 9);
		extid >>= 2;

		mtd->erasesize = (64 * 1024) << (extid & 0x03);
		extid >>= 2;

		*busw = (extid & 0x01) ? NAND_BUSWIDTH_16 : 0;
	}
}

static void nand_decode_id(struct mtd_info *mtd, struct nand_chip *chip,
				const struct nand_flash_dev *type, u8 id_data[8],
				int *busw)
{
	int maf_id = id_data[0];

	mtd->erasesize = type->erasesize;
	mtd->writesize = type->pagesize;
	mtd->oobsize = mtd->writesize / 32;
	*busw = type->options & NAND_BUSWIDTH_16;

	if (maf_id == NAND_MFR_AMD && id_data[4] != 0x00 && id_data[5] == 0x00
			&& id_data[6] == 0x00 && id_data[7] == 0x00
			&& mtd->writesize == 512) {
		mtd->erasesize = 128 * 1024;
		mtd->erasesize <<= ((id_data[3] & 0x03) << 1);
	}
}

static void nand_decode_bbm_options(struct mtd_info *mtd,
				    struct nand_chip *chip, u8 id_data[8])
{
	int maf_id = id_data[0];

	if (mtd->writesize > 512 || (chip->options & NAND_BUSWIDTH_16))
		chip->badblockpos = NAND_LARGE_BADBLOCK_POS;
	else
		chip->badblockpos = NAND_SMALL_BADBLOCK_POS;

	if ((chip->cellinfo & NAND_CI_CELLTYPE_MSK) &&
			(maf_id == NAND_MFR_SAMSUNG ||
			 maf_id == NAND_MFR_HYNIX))
		chip->bbt_options |= NAND_BBT_SCANLASTPAGE;
	else if ((!(chip->cellinfo & NAND_CI_CELLTYPE_MSK) &&
				(maf_id == NAND_MFR_SAMSUNG ||
				 maf_id == NAND_MFR_HYNIX ||
				 maf_id == NAND_MFR_TOSHIBA ||
				 maf_id == NAND_MFR_AMD ||
				 maf_id == NAND_MFR_MACRONIX)) ||
			(mtd->writesize == 2048 &&
			 maf_id == NAND_MFR_MICRON))
		chip->bbt_options |= NAND_BBT_SCAN2NDPAGE;
}

static const struct nand_flash_dev *nand_get_flash_type(struct mtd_info *mtd,
						  struct nand_chip *chip,
						  int busw,
						  int *maf_id, int *dev_id,
						  const struct nand_flash_dev *type)
{
	const char *name;
	int i, maf_idx;
	u8 id_data[8];

	chip->select_chip(mtd, 0);

	chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);

	chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);

	*maf_id = chip->read_byte(mtd);
	*dev_id = chip->read_byte(mtd);

	chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);

	for (i = 0; i < 8; i++)
		id_data[i] = chip->read_byte(mtd);

	if (id_data[0] != *maf_id || id_data[1] != *dev_id) {
		kformatln("%s: second ID read did not match %02x,%02x against %02x,%02x", __func__,
			*maf_id, *dev_id, id_data[0], id_data[1]);

		return nullptr;
	}

	if (!type)
		type = nand_flash_ids;

	for (; type->name != nullptr; type++)
		if (*dev_id == type->id)
			break;

	chip->onfi_version = 0;
	if (!type->name || !type->pagesize) {

		if (nand_flash_detect_onfi(mtd, chip, &busw))
			goto ident_done;
	}

	if (!type->name)

		return nullptr;

	if (!mtd->name)
		mtd->name = type->name;

	chip->chipsize = (uint64_t)type->chipsize << 20;

	if (!type->pagesize && chip->init_size) {

		busw = chip->init_size(mtd, chip, id_data);
	} else if (!type->pagesize) {

		nand_decode_ext_id(mtd, chip, id_data, &busw);
	} else {
		nand_decode_id(mtd, chip, type, id_data, &busw);
	}

	chip->options |= type->options;

	if (*maf_id != NAND_MFR_SAMSUNG && !type->pagesize)
		chip->options &= ~NAND_SAMSUNG_LP_OPTIONS;
ident_done:

	for (maf_idx = 0; nand_manuf_ids[maf_idx].id != 0x0; maf_idx++) {
		if (nand_manuf_ids[maf_idx].id == *maf_id)
			break;
	}

	if (busw != (chip->options & NAND_BUSWIDTH_16)) {
		kformatln("NAND device: Manufacturer ID: 0x%02x, Chip ID: 0x%02x (%s %s)", *maf_id,
			*dev_id, nand_manuf_ids[maf_idx].name, mtd->name);
		kformatln("NAND bus width %d instead %d bit", (chip->options & NAND_BUSWIDTH_16) ? 16 : 8,
			   busw ? 16 : 8);

		return nullptr;
	}

	nand_decode_bbm_options(mtd, chip, id_data);

	chip->page_shift = ffs(mtd->writesize) - 1;

	chip->pagemask = (chip->chipsize >> chip->page_shift) - 1;

	chip->bbt_erase_shift = chip->phys_erase_shift =
		ffs(mtd->erasesize) - 1;
	if (chip->chipsize & 0xffffffff)
		chip->chip_shift = ffs((unsigned)chip->chipsize) - 1;
	else {
		chip->chip_shift = ffs((unsigned)(chip->chipsize >> 32));
		chip->chip_shift += 32 - 1;
	}

	chip->badblockbits = 8;

	if (chip->options & NAND_4PAGE_ARRAY)
		chip->erase_cmd = multi_erase_cmd;
	else
		chip->erase_cmd = single_erase_cmd;

	if (mtd->writesize > 512 && chip->cmdfunc == nand_command)
		chip->cmdfunc = nand_command_lp;

	name = type->name;
#ifdef CONFIG_SYS_NAND_ONFI_DETECTION
	if (chip->onfi_version)
		name = chip->onfi_params.model;
#endif
	kformatln("NAND device: Manufacturer ID: 0x%02x, Chip ID: 0x%02x (%s %s), "
			"page size: %d, OOB size: %d", *maf_id, *dev_id, nand_manuf_ids[maf_idx].name,
			name, mtd->writesize, mtd->oobsize);

	return type;
}

BEG_EXT_C

uint8_t nand_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	return read8((addr_t)chip->IO_ADDR_R);
}

void nand_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	int i;
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);

	for (i = 0; i < len; i++)
		write8((addr_t)chip->IO_ADDR_W, buf[i]);
}

void nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	int i;
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);

	for (i = 0; i < len; i++)
		buf[i] = read8((addr_t)chip->IO_ADDR_R);
}

void nand_write_buf16(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	int i;
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	u16 *p = (u16 *) buf;
	len >>= 1;

	for (i = 0; i < len; i++)
		write16((addr_t)chip->IO_ADDR_W, p[i]);

}

void nand_read_buf16(struct mtd_info *mtd, uint8_t *buf, int len)
{
	int i;
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	u16 *p = (u16 *) buf;
	len >>= 1;

	for (i = 0; i < len; i++)
		p[i] = read16((addr_t)chip->IO_ADDR_R);
}

void nand_wait_ready(struct mtd_info *mtd)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	u32 timeo = (OSCfg::Clock::SystemTicksPreSecond * 20) / 1000;
	u32 time_start;

	time_start = get_timer(0);

	while (get_timer(time_start) < timeo) {
		if (chip->dev_ready)
			if (chip->dev_ready(mtd))
				break;
	}
}

#define BBT_PAGE_MASK	0xffffff3f

int nand_erase_nand(struct mtd_info *mtd, struct erase_info *instr,
		    int allowbbt)
{
	int page, status, pages_per_block, ret, chipnr;
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	loff_t rewrite_bbt[CONFIG_SYS_NAND_MAX_CHIPS] = {0};
	unsigned int bbt_masked_page = 0xffffffff;
	loff_t len;

	kformatln("%s: start = 0x%012llx, len = %llu",
				__func__, (unsigned long long)instr->addr,
				(unsigned long long)instr->len);

	if (check_offs_len(mtd, instr->addr, instr->len))
		return -static_cast<sint>(ErrNo::EINVAL);

	nand_get_device(chip, mtd, FCS_ERASING);

	page = (int)(instr->addr >> chip->page_shift);
	chipnr = (int)(instr->addr >> chip->chip_shift);

	pages_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);

	chip->select_chip(mtd, chipnr);

	if (nand_check_wp(mtd)) {
		kformatln("%s: Device is write protected!!!",
					__func__);
		instr->state = MTD_ERASE_FAILED;
		goto erase_exit;
	}

	if ((chip->options & BBT_AUTO_REFRESH) && !allowbbt)
		bbt_masked_page = chip->bbt_td->pages[chipnr] & BBT_PAGE_MASK;

	len = instr->len;

	instr->state = MTD_ERASING;

	while (len) {
		WATCHDOG_RESET();

		if (!instr->scrub && nand_block_checkbad(mtd, ((loff_t) page) <<
					chip->page_shift, 0, allowbbt)) {
			kformatln("%s: attempt to erase a bad block at page 0x%08x",
				   __func__, page);
			instr->state = MTD_ERASE_FAILED;
			goto erase_exit;
		}

		if (page <= chip->pagebuf && chip->pagebuf <
		    (page + pages_per_block))
			chip->pagebuf = -1;

		chip->erase_cmd(mtd, page & chip->pagemask);

		status = chip->waitfunc(mtd, chip);

		if ((status & NAND_STATUS_FAIL) && (chip->errstat))
			status = chip->errstat(mtd, chip, FCS_ERASING,
					       status, page);

		if (status & NAND_STATUS_FAIL) {
			kformatln("%s: Failed erase, page 0x%08x", __func__, page);
			instr->state = MTD_ERASE_FAILED;
			instr->fail_addr =
				((loff_t)page << chip->page_shift);
			goto erase_exit;
		}

		if (bbt_masked_page != 0xffffffff &&
		    (page & BBT_PAGE_MASK) == bbt_masked_page)
			rewrite_bbt[chipnr] =
				((loff_t)page << chip->page_shift);

		len -= (1 << chip->phys_erase_shift);
		page += pages_per_block;

		if (len && !(page & chip->pagemask)) {
			chipnr++;
			chip->select_chip(mtd, -1);
			chip->select_chip(mtd, chipnr);

	

			if (bbt_masked_page != 0xffffffff &&
			    (chip->bbt_td->options & NAND_BBT_PERCHIP))
				bbt_masked_page = chip->bbt_td->pages[chipnr] &
					BBT_PAGE_MASK;
		}
	}
	instr->state = MTD_ERASE_DONE;

erase_exit:

	ret = instr->state == MTD_ERASE_DONE ? 0 : -static_cast<sint>(ErrNo::EIO);

	nand_release_device(mtd);

	if (!ret)
		mtd_erase_callback(instr);

	if (bbt_masked_page == 0xffffffff || ret)
		return ret;

	for (chipnr = 0; chipnr < chip->numchips; chipnr++) {
		if (!rewrite_bbt[chipnr])
			continue;

		kformatln("%s: nand_update_bbt (%d:0x%0llx 0x%0x)", __func__, chipnr,
			rewrite_bbt[chipnr], chip->bbt_td->pages[chipnr]);
		nand_update_bbt(mtd, rewrite_bbt[chipnr]);
	}

	return ret;
}

int nand_scan_ident(struct mtd_info *mtd, int maxchips,
		    const struct nand_flash_dev *table)
{
	int i, busw, nand_maf_id, nand_dev_id;
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	const struct nand_flash_dev *type;

	busw = chip->options & NAND_BUSWIDTH_16;

	nand_set_defaults(chip, busw);

	type = nand_get_flash_type(mtd, chip, busw,
				&nand_maf_id, &nand_dev_id, table);

	if (type == nullptr) {
#ifndef CONFIG_SYS_NAND_QUIET_TEST
		kdebugln("No NAND device found");
#endif
		chip->select_chip(mtd, -1);

		return -static_cast<sint>(ErrNo::ENODEV);
	}

	for (i = 1; i < maxchips; i++) {
		chip->select_chip(mtd, i);

		chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);

		chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);

		if (nand_maf_id != chip->read_byte(mtd) ||
		    nand_dev_id != chip->read_byte(mtd))
			break;
	}
#ifdef DEBUG
	if (i > 1)
		kformatln("%d NAND chips detected", i);
#endif

	chip->numchips = i;
	mtd->size = i * chip->chipsize;

	return 0;
}

int nand_scan_tail(struct mtd_info *mtd)
{
	int i;
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);

	assert((chip->bbt_options & NAND_BBT_NO_OOB_BBM) &&
			!(chip->bbt_options & NAND_BBT_USE_FLASH));

	if (!(chip->options & NAND_OWN_BUFFERS))
		chip->buffers = (struct nand_buffers *)kheap.allocate(sizeof(*chip->buffers), 8);
	if (!chip->buffers)
		return -static_cast<sint>(ErrNo::ENOMEM);

	chip->oob_poi = chip->buffers->databuf + mtd->writesize;

	if (!chip->ecc.layout && (chip->ecc.mode != NAND_ECC_SOFT_BCH)) {
		switch (mtd->oobsize) {
		case 8:
			chip->ecc.layout = &nand_oob_8;
			break;
		case 16:
			chip->ecc.layout = &nand_oob_16;
			break;
		case 64:
			chip->ecc.layout = &nand_oob_64;
			break;
		case 128:
			chip->ecc.layout = &nand_oob_128;
			break;
		default:
			kformatln("No oob scheme defined for oobsize %d",
				   mtd->oobsize);
		}
	}

	if (!chip->write_page)
		chip->write_page = nand_write_page;

	if (!chip->onfi_set_features)
		chip->onfi_set_features = nand_onfi_set_features;
	if (!chip->onfi_get_features)
		chip->onfi_get_features = nand_onfi_get_features;

	switch (chip->ecc.mode) {
	case NAND_ECC_HW_OOB_FIRST:

		if (!chip->ecc.calculate || !chip->ecc.correct ||
		     !chip->ecc.hwctl) {
			kdebugln("No ECC functions supplied; hardware ECC not possible");
			kformatln("BUG at %s:%d", __FILE__, __LINE__);
		}
		if (!chip->ecc.read_page)
			chip->ecc.read_page = nand_read_page_hwecc_oob_first;

	case NAND_ECC_HW:

		if (!chip->ecc.read_page)
			chip->ecc.read_page = nand_read_page_hwecc;
		if (!chip->ecc.write_page)
			chip->ecc.write_page = nand_write_page_hwecc;
		if (!chip->ecc.read_page_raw)
			chip->ecc.read_page_raw = nand_read_page_raw;
		if (!chip->ecc.write_page_raw)
			chip->ecc.write_page_raw = nand_write_page_raw;
		if (!chip->ecc.read_oob)
			chip->ecc.read_oob = nand_read_oob_std;
		if (!chip->ecc.write_oob)
			chip->ecc.write_oob = nand_write_oob_std;

	case NAND_ECC_HW_SYNDROME:
		if ((!chip->ecc.calculate || !chip->ecc.correct ||
		     !chip->ecc.hwctl) &&
		    (!chip->ecc.read_page ||
		     chip->ecc.read_page == nand_read_page_hwecc ||
		     !chip->ecc.write_page ||
		     chip->ecc.write_page == nand_write_page_hwecc)) {
			kdebugln("No ECC functions supplied; hardware ECC not possible");
			kformatln("BUG at %s:%d", __FILE__, __LINE__);
		}

		if (!chip->ecc.read_page)
			chip->ecc.read_page = nand_read_page_syndrome;
		if (!chip->ecc.write_page)
			chip->ecc.write_page = nand_write_page_syndrome;
		if (!chip->ecc.read_page_raw)
			chip->ecc.read_page_raw = nand_read_page_raw_syndrome;
		if (!chip->ecc.write_page_raw)
			chip->ecc.write_page_raw = nand_write_page_raw_syndrome;
		if (!chip->ecc.read_oob)
			chip->ecc.read_oob = nand_read_oob_syndrome;
		if (!chip->ecc.write_oob)
			chip->ecc.write_oob = nand_write_oob_syndrome;

		if (mtd->writesize >= chip->ecc.size) {
			if (!chip->ecc.strength) {
				kdebugln("Driver must set ecc.strength when using hardware ECC");
				kformatln("BUG at %s:%d", __FILE__, __LINE__);
			}
			break;
		}
		kformatln("%d byte HW ECC not possible on %d byte page size, fallback to SW ECC",
			   chip->ecc.size, mtd->writesize);
		chip->ecc.mode = NAND_ECC_SOFT;

	case NAND_ECC_SOFT:
		chip->ecc.calculate = nand_calculate_ecc;
		chip->ecc.correct = nand_correct_data;
		chip->ecc.read_page = nand_read_page_swecc;
		chip->ecc.read_subpage = nand_read_subpage;
		chip->ecc.write_page = nand_write_page_swecc;
		chip->ecc.read_page_raw = nand_read_page_raw;
		chip->ecc.write_page_raw = nand_write_page_raw;
		chip->ecc.read_oob = nand_read_oob_std;
		chip->ecc.write_oob = nand_write_oob_std;
		if (!chip->ecc.size)
			chip->ecc.size = 256;
		chip->ecc.bytes = 3;
		chip->ecc.strength = 1;
		break;

	case NAND_ECC_SOFT_BCH:
		if (!mtd_nand_has_bch()) {
			kdebugln("CONFIG_MTD_ECC_BCH not enabled");
			return -static_cast<sint>(ErrNo::EINVAL);
		}
		chip->ecc.calculate = nand_bch_calculate_ecc;
		chip->ecc.correct = nand_bch_correct_data;
		chip->ecc.read_page = nand_read_page_swecc;
		chip->ecc.read_subpage = nand_read_subpage;
		chip->ecc.write_page = nand_write_page_swecc;
		chip->ecc.read_page_raw = nand_read_page_raw;
		chip->ecc.write_page_raw = nand_write_page_raw;
		chip->ecc.read_oob = nand_read_oob_std;
		chip->ecc.write_oob = nand_write_oob_std;

		if (!chip->ecc.size && (mtd->oobsize >= 64)) {
			chip->ecc.size = 512;
			chip->ecc.bytes = 7;
		}
		chip->ecc.priv = nand_bch_init(mtd,
					       chip->ecc.size,
					       chip->ecc.bytes,
					       &chip->ecc.layout);
		if (!chip->ecc.priv)
			kdebugln("BCH ECC initialization failed!");
 		chip->ecc.strength =
			chip->ecc.bytes * 8 / fls(8 * chip->ecc.size);
		break;

	case NAND_ECC_NONE:
		kdebugln("NAND_ECC_NONE selected by board driver. This is not recommended !!");
		chip->ecc.read_page = nand_read_page_raw;
		chip->ecc.write_page = nand_write_page_raw;
		chip->ecc.read_oob = nand_read_oob_std;
		chip->ecc.read_page_raw = nand_read_page_raw;
		chip->ecc.write_page_raw = nand_write_page_raw;
		chip->ecc.write_oob = nand_write_oob_std;
		chip->ecc.size = mtd->writesize;
		chip->ecc.bytes = 0;
		break;

	default:
		kformatln("Invalid NAND_ECC_MODE %d", chip->ecc.mode);
		kformatln("BUG at %s:%d", __FILE__, __LINE__);
	}

	if (!chip->ecc.read_oob_raw)
		chip->ecc.read_oob_raw = chip->ecc.read_oob;
	if (!chip->ecc.write_oob_raw)
		chip->ecc.write_oob_raw = chip->ecc.write_oob;

	chip->ecc.layout->oobavail = 0;
	for (i = 0; chip->ecc.layout->oobfree[i].length
			&& i < ARRAY_SIZE(chip->ecc.layout->oobfree); i++)
		chip->ecc.layout->oobavail +=
			chip->ecc.layout->oobfree[i].length;
	mtd->oobavail = chip->ecc.layout->oobavail;

	chip->ecc.steps = mtd->writesize / chip->ecc.size;
	if (chip->ecc.steps * chip->ecc.size != mtd->writesize) {
		kdebugln("Invalid ECC parameters");
		kformatln("BUG at %s:%d", __FILE__, __LINE__);
	}
	chip->ecc.total = chip->ecc.steps * chip->ecc.bytes;

	if (!(chip->options & NAND_NO_SUBPAGE_WRITE) &&
	    !(chip->cellinfo & NAND_CI_CELLTYPE_MSK)) {
		switch (chip->ecc.steps) {
		case 2:
			mtd->subpage_sft = 1;
			break;
		case 4:
		case 8:
		case 16:
			mtd->subpage_sft = 2;
			break;
		}
	}
	chip->subpagesize = mtd->writesize >> mtd->subpage_sft;

	chip->state = FCS_READY;

	chip->select_chip(mtd, -1);

	chip->pagebuf = -1;

	if ((chip->ecc.mode == NAND_ECC_SOFT) && (chip->page_shift > 9))
		chip->options |= NAND_SUBPAGE_READ;

	mtd->type = MTD_NANDFLASH;
	mtd->flags = (chip->options & NAND_ROM) ? MTD_CAP_ROM :
						MTD_CAP_NANDFLASH;
	mtd->_erase = nand_erase;
	mtd->_point = nullptr;
	mtd->_unpoint = nullptr;
	mtd->_read = nand_read;
	mtd->_write = nand_write;
	mtd->_read_oob = nand_read_oob;
	mtd->_write_oob = nand_write_oob;
	mtd->_sync = nand_sync;
	mtd->_lock = nullptr;
	mtd->_unlock = nullptr;
	mtd->_block_isbad = nand_block_isbad;
	mtd->_block_markbad = nand_block_markbad;

	mtd->ecclayout = chip->ecc.layout;
	mtd->ecc_strength = chip->ecc.strength;

	if (!mtd->bitflip_threshold)
		mtd->bitflip_threshold = mtd->ecc_strength;

	if (chip->options & NAND_SKIP_BBTSCAN)
		chip->options |= NAND_BBT_SCANNED;

	return 0;
}

int nand_scan(struct mtd_info *mtd, int maxchips)
{
	int ret;

	ret = nand_scan_ident(mtd, maxchips, nullptr);
	if (!ret)
		ret = nand_scan_tail(mtd);
	return ret;
}

void nand_release(struct mtd_info *mtd)
{
	struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);

	if (chip->ecc.mode == NAND_ECC_SOFT_BCH)
		nand_bch_free((struct nand_bch_control *)chip->ecc.priv);

#ifdef CONFIG_MTD_PARTITIONS

	del_mtd_partitions(mtd);
#endif

	kheap.free(chip->bbt);
	if (!(chip->options & NAND_OWN_BUFFERS))
		kheap.free(chip->buffers);

	if (chip->badblock_pattern && chip->badblock_pattern->options
			& NAND_BBT_DYNAMICSTRUCT)
		kheap.free(chip->badblock_pattern);
}

END_EXT_C

#undef udelay
#undef ndelay

#undef readw
#undef writew
