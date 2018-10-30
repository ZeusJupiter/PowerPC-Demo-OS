/*
 *   File name: serialtty.cpp
 *
 *  Created on: May 24, 2017, 3:30:52 PM
 *      Author: Victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <interrupt.h>

#include <fcntl.h>
#include <errno.h>

#include <tty/serialtty.h>
#include <motherboard.h>
#include <uart/uart.h>

#include <vfs/vfs.h>
#include <drivermanager.h>

namespace Driver
{
	sint SerialTty::driverIndex;
	SerialTty SerialTty::sttyArray[static_cast<word_t>(OSCfg::Dev::SerialTtyEnum::TotalSerialTtyCount)];

	void SerialTty::installDriver(void)
	{
		DriverInterface* driver = nullptr;
		driverIndex = DriverManager::getFreeDriver(&driver);

		if(driver == nullptr || driver->_isUsed)
		{
			return;
		}

		zero_block(driver, sizeof(Driver::DriverInterface));

		driver->_release  = decltype(driver->_release)(&SerialTty::release) ;

		driver->_open     = decltype(driver->_open )(&SerialTty::open    ) ;
		driver->_close    = decltype(driver->_close)(&SerialTty::close   ) ;
		driver->_read     = decltype(driver->_read )(&SerialTty::writeOut) ;
		driver->_write    = decltype(driver->_write)(&SerialTty::readIn  ) ;

		driver->_ioctl    = decltype(driver->_ioctl)(&SerialTty::ioctl   ) ;

		driver->_isUsed = true;
	}

	void SerialTty::init(Uart* sio, uint rBufSize, uint wBufSize)
	{
		assert(sio != nullptr);
		_sio = sio;

		TtyBase::init(TtyType::TTY_TYPE_SERIAL, DriverManager::getDriver(SerialTty::driverIndex), rBufSize, wBufSize);

		_sio->_outputRxCharFunc = (ErrorNoFuncPtrPvoidVar)SerialTty::iSendDataToTty;
		_sio->_outputRxCharArg = this;
		_sio->_getTxCharFunc = (ErrorNoFuncPtrPvoidVar)SerialTty::iTransmitDataOfTty;
		_sio->_getTxCharArg = this;

		_txStartUpHookFunc = decltype(_txStartUpHookFunc)(startup);

		KInterrupt::connectInterruptVector(_sio->_vecNum, (VoidFuncPtrVar)Uart::uartIsr, _sio, nullptr);
		HMB::peripheralEnableIrq(_sio->_vecNum);
	}

	void SerialTty::startup(SerialTty* stty)
	{
		stty->_sio->startup();
	}

	void SerialTty::teminate(void)
	{
		TtyBase::teminate();
		if(_sio)
		{
			_sio->close();

			HMB::peripheralDisableIrq(_sio->_vecNum);
			KInterrupt::disconnectInterruptVector(_sio->_vecNum, _sio);

			_sio = nullptr;
		}
	}

	SerialTty* SerialTty::createStty(OSCfg::Dev::SerialTtyEnum stty, Uart* com, const c8* name,
			uint rBufSize, uint wBufSize, bool needCopy)
	{
		SerialTty* ret = nullptr;
		if(stty < OSCfg::Dev::SerialTtyEnum::TotalSerialTtyCount)
		{
			ret = &sttyArray[static_cast<word_t>(stty)];
			ret->init(com, rBufSize, wBufSize);
		}
		return ret;
	}

	sint SerialTty::release(void)
	{
		teminate();
		return static_cast<sint>(ErrNo::ENONE);
	}

	sint SerialTty::open(int flags, mode_t )
	{
		_flags = flags + 1;
		_sio->open(0);

		return static_cast<sint>(ErrNo::ENONE);
	}
	sint SerialTty::close(void)
	{
		_sio->hangUp();

		return static_cast<sint>(ErrNo::ENONE);
	}

	sint SerialTty::ioctl(IoctlSet request, slong arg)
	{
		if(request == IoctlSet::FIOBAUDRATE)
		{
			_sio->setBaud(arg);
			return static_cast<sint>(ErrNo::ENONE);
		}

		ErrNo ret = _sio->ioctl(static_cast<Uart::IoctlSet>(request), arg);

		if(ret == ErrNo::ENONE)
			ret = TtyBase::ioctl(request, arg);

		return static_cast<sint>(ret);
	}

}

