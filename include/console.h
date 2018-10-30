/*
 *   File name: console.h
 *
 *  Created on: 2016年11月25日, 下午7:22:31
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_CONSOLE_H__
#define __INCLUDE_CONSOLE_H__

#include <mk/hardware.h>
#include <uart/uart.h>
#include <motherboard.h>

namespace Hardware
{
	namespace Uart
	{
		enum class UartEnum : word_t;
	}
}

class Console
{
	const c8 * _name;
	Driver::Uart* _com;

	Console(const Console&) = delete;
	Console operator=(const Console&) = delete;
	explicit Console(void) = delete;
public:
	inline explicit Console(const char* name, HUartEnum uartEnum):_name(name),
			_com(Hardware::MotherBoard::enumUart(uartEnum))
	{}
	void putc(const c8 ch){ _com->putcByPolling(ch); }
	c8 getc(bool b){ return _com->getcByPolling(b); }
	void puts(const c8* str){ _com->putsByPolling(str); }
};

extern Console debugConsole;

#endif

