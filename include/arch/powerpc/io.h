/*
 *   File name: io.h
 *
 *  Created on: 2016年11月16日, 下午4:23:39
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:

 */

#ifndef __POWERPC_IO_H__
#define __POWERPC_IO_H__

#define _DEF_IOREAD(FUNC, PTYPE) \
	InlineStatic PTYPE FUNC (addr_t addr) \
	{ \
		sync(); \
		PTYPE val = *(volatile PTYPE *)addr; \
		isync(); \
		return val;  \
	}

#define _DEF_IOWRITE(FUNC, PTYPE)  \
	InlineStatic void FUNC(addr_t addr, const PTYPE data) \
	{ \
		sync(); \
		*(volatile PTYPE *)addr = data; \
		eieio(); \
	}

_DEF_IOREAD(read8, u8)
_DEF_IOREAD(read16, u16)
_DEF_IOREAD(read32, u32)

_DEF_IOWRITE(write8, u8)
_DEF_IOWRITE(write16, u16)
_DEF_IOWRITE(write32, u32)

#define in8  read8
#define in16 read16
#define in32 read32
#define out8 write8
#define out16 write16
#define out32 write32

#undef _DEF_IOREAD
#undef _DEF_IOWRITE
#endif

