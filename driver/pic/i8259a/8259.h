/*
 *   File name: 8259.h
 *
 *  Created on: 2017年4月13日, 上午1:54:24
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 *  Driver of i8259 PIC
 *  base is the base address of the registers of i8259
 *
 */

#ifndef __DRIVER_UART_8259_H__
#define __DRIVER_UART_8259_H__

namespace Hardware
{
	class MotherBoardImp;
}

namespace Driver
{
	class I8259 final
	{
		friend class Hardware::MotherBoardImp;
	private:
		addr_t _base;
		u8 _mask;

		I8259(const I8259&) = delete;
		I8259& operator=(const I8259&) = delete;
		explicit I8259(void){}

		void init(addr_t baseAddr, word_t vectorBase, u8 slaveInfo);

	public:

		s32 irr(void)const;
		void unmask(word_t irq);
		void mask(word_t irq);
		void ackSpecEOI(word_t irq);
		void ackNormalEOI(void);
		word_t isr(void);
		bool isMasked(word_t irq);
	};
}

#endif

