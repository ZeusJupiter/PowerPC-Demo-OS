/*
 *   File name: mc146818.cpp
 *
 *  Created on: 2017年4月21日, 下午8:51:38
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <platform/platform.h>

#include <macros.h>
#include <types.h>
#include <debug.h>
#include "mc146818.h"
#include <mk/hardware.h>

namespace Driver
{
	void MC146818::init(addr_t base)
	{
		_base = base;
		w8(base + 0x0B, 0x80);

		w8(0x0A, Hardware::MC146818::Frequency | Hardware::MC146818::DividerRate);
		w8(0x0D, 0x80);
		r8(0x0C);
		w8(0x0B, Hardware::MC146818::TimeFormate);

	}

	void MC146818::read(tm_t& tt)const
	{
		while(isUpdating())
			kdebugln("----Real time Clock is updating----");

		u8 s, m, h, dow, dom, mon, y;

		s = r8(0);
		m = r8(2);
		h = r8(4);
		dow = r8(6);
		dom = r8(7);
		mon = r8(8);
		y = r8(9);

		tt.tm_sec = bcd2bin(s & 0x7f);
		tt.tm_min = bcd2bin(m & 0x7f);
		tt.tm_hour = bcd2bin(h & 0x3f);
		tt.tm_mday = bcd2bin(dom & 0x3f);
		tt.tm_mon = bcd2bin(mon & 0x1f) - 1;
		tt.tm_year = bcd2bin(y);
		tt.tm_wday = bcd2bin(dow & 0x07);

		if(tt.tm_year < 70) tt.tm_year += 100;

		tt.tm_yday = getDayOfYear(tt.tm_year, tt.tm_mon, tt.tm_mday);
		tt.tm_isdst = 0;
	}

	void MC146818::write(const tm_t& tm)const
	{
		w8(0x0B, 0x82);

		w8(9, bin2bcd(tm.tm_year));
		w8(8, bin2bcd(tm.tm_mon));
		w8(7, bin2bcd(tm.tm_mday));
		w8(6, bin2bcd(tm.tm_wday));
		w8(4, bin2bcd(tm.tm_hour));
		w8(2, bin2bcd(tm.tm_min));
		w8(0, bin2bcd(tm.tm_sec));

		w8(0x0B, 0x02);
	}

#if defined(_DEBUG) || defined(_RELEASE_WITH_MSG)
		void MC146818::show(void)const
		{
			tm_t tt;
			read(tt);
			kformatln("year=%d", tt.tm_year);
			kformatln("mon=%d", tt.tm_mon);
			kformatln("Day=%d", tt.tm_mday);
			kformatln("week=%d", tt.tm_wday);
			kformatln("hour=%d", tt.tm_hour);
			kformatln("min=%d", tt.tm_min);
			kformatln("sec=%d", tt.tm_sec);
		}
#endif
}

