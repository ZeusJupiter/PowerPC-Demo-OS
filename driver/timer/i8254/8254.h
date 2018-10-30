/*
 *   File name: 8254.h
 *
 *  Created on: 2017年4月23日, 上午6:20:47
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __DRIVER_TIMER_8254_H__
#define __DRIVER_TIMER_8254_H__

namespace Hardware
{
	class MotherBoardImp;
}

namespace Driver
{
	class I8254 final
	{
		friend class Hardware::MotherBoardImp;

		addr_t _base;
		u32 _freq;

		explicit I8254(void){}
		I8254(const I8254&) = delete;
		I8254& operator=(const I8254&) = delete;

		void init(addr_t base, u32 freq, u32 initFreq);

	public:
		static void isr(I8254* tm, pvoid arg){}

	};
}

#endif

