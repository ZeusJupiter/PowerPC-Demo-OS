/*
 *   File name: mc146818.h
 *
 *  Created on: 2017年4月20日, 下午10:05:17
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __DRIVER_RTC_MC146818_H__
#define __DRIVER_RTC_MC146818_H__

#include <io.h>
#include <time.h>

namespace Hardware
{
	class MotherBoardImp;
}

namespace Driver
{
	class MC146818 final
	{
		friend class Hardware::MotherBoardImp;

		addr_t _base;

		static u32 bcd2bin(u32 val){ return (val & 0x0f) + ((val & 0xff) >> 4) * 10; }
		static u32 bin2bcd(u32 val){ return (((val/10) << 4) | (val % 10)); }

		explicit MC146818(void){}
		MC146818(const MC146818&) = delete;
		MC146818& operator=(const MC146818&) = delete;

		void init(addr_t base);

		u8 r8(u32 reg)const{ out8(_base, reg); return in8(_base + 1); }
		void w8(u32 reg, u32 val)const{ out8(_base, reg), out8(_base + 1, val); }

	public:

		bool isUpdating(void)const{ return (r8(0x0A) & 0x80); }
		void read(tm_t& tt)const;
		void write(const tm_t& tm)const;
#if defined(_DEBUG) || defined(_RELEASE_WITH_MSG)
		void show(void)const;
#else
		void show(void)const{}
#endif
	};
}

#endif

