/*
 *   File name: 8259.cpp
 *
 *  Created on: 2017年4月13日, 上午4:25:37
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <platform/platform.h>

#include <macros.h>
#include <types.h>
#include <debug.h>
#include <io.h>

#include "8259.h"

namespace Driver
{
	void I8259::unmask(word_t irqline)
	{
		_mask &= ~(1 << irqline);
		out8(_base + 1, _mask);
	}

	void I8259::mask(word_t irqline)
	{
		_mask |= (1 << (irqline));
		out8(_base + 1, _mask);
	}

	void I8259::ackSpecEOI(word_t irqline)
	{
		out8(_base, 0x60 + irqline);
	}

	void I8259::ackNormalEOI(void)
	{
		out8(_base, 0x20);
	}

	word_t I8259::isr(void)
	{
		out8(_base, 0x0B);
		return in8(_base);
	}

	bool I8259::isMasked(word_t irqline)
	{
		return (_mask & (1 << irqline));
	}

	void I8259::init(addr_t baseAddr, word_t vectorBase, u8 cascadingInfo)
	{
		_mask = 0xFF;
		_base = baseAddr;

		out8(_base, 0x11);

		out8(_base+1, vectorBase);
		out8(_base+1, cascadingInfo);
		out8(_base+1, 0x01);
		out8(_base+1, _mask);
	}

	s32 I8259::irr(void)const
	{
		out8(_base, 0x0E);
		return in8(_base);
	}
}

