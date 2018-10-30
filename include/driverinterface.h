/*
 *   File name: driverinterface.h
 *
 *  Created on: 2017年5月16日, 下午3:53:40
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_DRIVERINTERFACE_H__
#define __INCLUDE_DRIVERINTERFACE_H__

class BaseDevice;

typedef uint (BaseDevice::*BDevU32FuncPtrVar)(...);
typedef void (BaseDevice::*BDevVoidFuncPtrVar)(...);
typedef sint (BaseDevice::*BDevSintFuncPtrVar)(...);
typedef bool (BaseDevice::*BDevBoolFuncPtrVar)(...);

namespace Driver
{
	class DriverInterface
	{
	public:

		BDevSintFuncPtrVar _create;
		BDevSintFuncPtrVar _release;

		BDevSintFuncPtrVar _open;
		BDevSintFuncPtrVar _close;

		BDevSintFuncPtrVar _read;
		BDevSintFuncPtrVar _write;

		BDevSintFuncPtrVar _readEx;
		BDevSintFuncPtrVar _writeEx;

		BDevSintFuncPtrVar _ioctl;
		BDevSintFuncPtrVar _select;
		BDevSintFuncPtrVar _lseek;

		BDevSintFuncPtrVar _fstat;
		BDevSintFuncPtrVar _lstat;

		BDevSintFuncPtrVar _symlink;
		BDevSintFuncPtrVar _readlink;

		BDevSintFuncPtrVar _devMap;
		BDevSintFuncPtrVar _devUnmap;

		bool _isUsed;

		explicit DriverInterface(void){}
		DriverInterface(const DriverInterface&) = delete;
		DriverInterface& operator=(const DriverInterface&) = delete;

		template<typename T>
		static void RegisterDriver(DriverInterface& driver)
		{
			driver._create   = decltype(driver._create)(T::create    );
			driver._release  = decltype(driver._release)(T::release  );
			driver._open     = decltype(driver._create)(&T::open     );
			driver._close    = decltype(driver._create)(&T::close    );
			driver._read     = decltype(driver._create)(&T::read     );
			driver._write    = decltype(driver._create)(&T::write    );
			driver._readEx   = decltype(driver._create)(&T::readEx   );
			driver._writeEx  = decltype(driver._create)(&T::writeEx  );
			driver._ioctl    = decltype(driver._create)(&T::ioctl    );
			driver._select   = decltype(driver._create)(&T::select   );
			driver._lseek    = decltype(driver._create)(&T::lseek    );
			driver._fstat    = decltype(driver._create)(&T::fstat    );
			driver._lstat    = decltype(driver._create)(&T::lstat    );
			driver._symlink  = decltype(driver._create)(&T::symlink  );
			driver._readlink = decltype(driver._create)(&T::readlink );
			driver._devMap   = decltype(driver._create)(&T::devMap   );
			driver._devUnmap = decltype(driver._create)(&T::devUnmap );

			driver._isUsed = true;
		}
	};
}

#endif

