/*
 *   File name: serialtty.h
 *
 *  Created on: May 24, 2017, 2:53:54 PM
 *      Author: Victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __DRIVER_TTY_SERIALTTY_H__
#define __DRIVER_TTY_SERIALTTY_H__

#include <mk/oscfg.h>
#include "tty.h"

namespace Hardware
{
	class MotherBoard;
}

namespace Driver
{
	class Uart;
	class SerialTty final : public TtyBase
	{
		static sint driverIndex;
		static SerialTty sttyArray[];

		static void installDriver(void);
		static void startup(SerialTty* stty);

		SerialTty(const SerialTty&) = delete;
		SerialTty& operator=(const SerialTty&) = delete;

		explicit SerialTty(void){}
	public:
		static SerialTty* createStty(OSCfg::Dev::SerialTtyEnum stty, Uart* com, const c8* name, uint rBufSize, uint wBufSize, bool needCopy = true);

		sint release(void);
		sint open(int flags, mode_t mode);
		sint close(void);
		sint ioctl(IoctlSet request, slong arg);

	private:
		void teminate(void);
		void init(Uart* sio, uint rBufSize, uint wBufSize);

		Uart* _sio;

		friend class Hardware::MotherBoard;
	};
}

#endif

