/*
 *   File name: nandbch.cpp
 *
 *  Created on: 2017年7月28日, 下午3:39:10
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

#include <mm/heap.h>

#include <linux/bitops.h>
#include <linux/bch.h>
#include <linux/mtd/nand_bch.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>

struct nand_bch_control {
	struct bch_control *bch;
	struct nand_ecclayout ecclayout;
	unsigned int *errloc;
	unsigned char *eccmask;
};

BEG_EXT_C

int nand_bch_calculate_ecc(struct mtd_info *mtd, const unsigned char *buf,
		unsigned char *code)
{
	const struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	struct nand_bch_control *nbc = (struct nand_bch_control *) chip->ecc.priv;

	memset(code, 0, chip->ecc.bytes);
	encode_bch(nbc->bch, buf, chip->ecc.size, code);

	for (int i = 0; i < chip->ecc.bytes; i++)
		code[i] ^= nbc->eccmask[i];

	return 0;
}

int nand_bch_correct_data(struct mtd_info *mtd, unsigned char *buf,
		unsigned char *read_ecc, unsigned char *calc_ecc)
{
	const struct nand_chip *chip = static_cast<struct nand_chip *>(mtd->priv);
	struct nand_bch_control *nbc =
			static_cast<struct nand_bch_control *>(chip->ecc.priv);
	unsigned int *errloc = nbc->errloc;
	int count;

	count = decode_bch(nbc->bch, NULL, chip->ecc.size, read_ecc, calc_ecc,
	NULL, errloc);
	if (count > 0) {
		for (int i = 0; i < count; i++) {
			if (errloc[i] < (chip->ecc.size * 8))

				buf[errloc[i] >> 3] ^= (1 << (errloc[i] & 7));

			kformatln("%s: corrected bitflip %u", __func__, errloc[i]);
		}
	} else if (count < 0) {
		kdebugln("ecc unrecoverable error");
		count = -1;
	}
	return count;
}

struct nand_bch_control *
nand_bch_init(struct mtd_info *mtd, unsigned int eccsize, unsigned int eccbytes,
		struct nand_ecclayout **ecclayout)
{
	struct nand_bch_control *nbc = nullptr;

	if (!eccsize || !eccbytes || !mtd) {
		kdebugln("ecc parameters not supplied");
		goto fail;
	}

	uint m, t;

	m = fls(1 + 8 * eccsize);
	t = (eccbytes * 8) / m;

	nbc = (struct nand_bch_control *) kheap.allocate(sizeof(*nbc));
	if (!nbc)
		goto fail;

	zero_block(nbc, sizeof(*nbc));

	nbc->bch = init_bch(m, t, 0);
	if (!nbc->bch)
		goto fail;

	if (nbc->bch->ecc_bytes != eccbytes) {
		kformatln("invalid eccbytes %u, should be %u", eccbytes,
				nbc->bch->ecc_bytes);
		goto fail;
	}

	uint eccsteps;
	eccsteps = mtd->writesize / eccsize;

	if (!*ecclayout) {

		if (mtd->oobsize < 64) {
			kformatln("must provide an oob scheme for oobsize %d",
					mtd->oobsize);
			goto fail;
		}

		struct nand_ecclayout * layout = &nbc->ecclayout;
		layout->eccbytes = eccsteps * eccbytes;

		if (layout->eccbytes + 2 > mtd->oobsize) {
			kformatln(
					"no suitable oob scheme available " "for oobsize %d eccbytes %u",
					mtd->oobsize, eccbytes);
			goto fail;
		}

		for (uint i = 0; i < layout->eccbytes; i++)
			layout->eccpos[i] = mtd->oobsize - layout->eccbytes + i;

		layout->oobfree[0].offset = 2;
		layout->oobfree[0].length = mtd->oobsize - 2 - layout->eccbytes;

		*ecclayout = layout;
	}

	if (8 * (eccsize + eccbytes) >= (1 << m)) {
		kformatln("eccsize %u is too large", eccsize);
		goto fail;
	}
	if ((*ecclayout)->eccbytes != (eccsteps * eccbytes)) {
		kdebugln("invalid ecc layout");
		goto fail;
	}

	nbc->eccmask = (unsigned char *) kheap.allocate(eccbytes);
	nbc->errloc = (unsigned int *) kheap.allocate(t*sizeof(*nbc->errloc));
	if (!nbc->eccmask || !nbc->errloc)
		goto fail;

	unsigned char *erased_page;
	erased_page = (unsigned char *) kheap.allocate(eccsize);
	if (!erased_page)
		goto fail;

	memset(erased_page, 0xff, eccsize);
	memset(nbc->eccmask, 0, eccbytes);
	encode_bch(nbc->bch, erased_page, eccsize, nbc->eccmask);
	kheap.free(erased_page);

	for (uint i = 0; i < eccbytes; i++)
		nbc->eccmask[i] ^= 0xff;

	return nbc;
	fail: nand_bch_free(nbc);
	return nullptr;
}

void nand_bch_free(struct nand_bch_control *nbc)
{
	if (nbc) {
		free_bch(nbc->bch);
		kheap.free(nbc->errloc);
		kheap.free(nbc->eccmask);
		kheap.free(nbc);
	}
}

END_EXT_C
