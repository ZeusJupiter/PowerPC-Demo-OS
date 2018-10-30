/*
 *   File name: tty.h
 *
 *  Created on: 2017年5月3日, 下午5:20:06
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 * 		tty is a virtual device
 */

#ifndef __DRIVER_TTY_TTY_H__
#define __DRIVER_TTY_TTY_H__

#include <mk/oscfg.h>
#include <string.h>
#include <ioctlset.h>
#include <mutex.h>
#include <ringbuffer.h>
#include <selecttype.h>
#include <device.h>
#include <errno.h>

namespace Hardware
{
	class MotherBoardImp;
}

struct WinCursor
{

	union
	{
		u16 _x;

		u16 _col;
	};

	union
	{
		u16 _y;
		u16 _row;
	};
};

struct WinSize
{

	union{
		u16 _cols;
		u16 _width;
	};

	union{
		u16 _rows;
		u16 _height;
	};

	union{
		u16 _xpixel;
		u16 _colpixel;
		u16 _widthpixel;
	};

	union{
		u16 _ypixel;
		u16 _rowpixel;
		u16 _heightpixel;
	};
};

struct WinColor
{

	u8 _defForeground;
	u8 _defBackground;
	u8 _foreground;
	u8 _background;
};

namespace Driver
{

	class TtyBase
	{
		static S32FuncPtrVoid globalAbortFunc;
	public:

		static const s32 MaxInput = 255;
		static const s32 MaxCanOn = 255;
		static const u8  WriteThreshold = 20;

		static const u8  XoffPercent = 85;
		static const u8  XonPercent = 50;

		static void setGlobalAbortFunc(S32FuncPtrVoid gAbortFunc){ globalAbortFunc = gAbortFunc; }
		static S32FuncPtrVoid getGlobalAbortFunc(void){ return globalAbortFunc; }

		static ErrNo iSendDataToTty(TtyBase* tyDev, c8 inChar);
		static ErrNo iTransmitDataOfTty(TtyBase* tyDev, c8* outChar);

		enum class OptionFlags : word_t
		{
			OPT_RAW      =  0    ,
			OPT_ECHO     =  0x01 ,
			OPT_CRMOD    =  0x02 ,
			OPT_TANDEM   =  0x04 ,
			OPT_7_BIT    =  0x08 ,
			OPT_MON_TRAP =  0x10 ,
			OPT_ABORT    =  0x20 ,
			OPT_LINE     =  0x40 ,

#if TTY_NOT_7_BIT > 0
			OPT_TERMINAL =  (OPT_ECHO | OPT_CRMOD | OPT_TANDEM | \
                         OPT_MON_TRAP | OPT_ABORT | OPT_LINE),
#else
			OPT_TERMINAL =  (OPT_ECHO | OPT_CRMOD | OPT_TANDEM | \
                         OPT_MON_TRAP | OPT_7_BIT | OPT_ABORT | OPT_LINE),
#endif

		};

		enum class TtyType : u8
		{
			TTY_TYPE_IOSTD = 0,
			TTY_TYPE_SERIAL = 1,
			TTY_TYPE_SCREEN = 2,
			TTY_TYPE_VIRTUAL = 3,
			TTY_TYPE_GUI = 4
		};

		enum class TtyState : u8
		{
			TTY_STATE_RUN = 0,
			TTY_STATE_SWITCH = 1,
			TTY_STATE_ERROR = 2,
			TTY_STATE_PAUSE = 3
		};

		enum class TtyColor : u8
		{
			Black        = 0,
			Blue         = 1,
			Green        = 2,
			Cyan         = 3,
			Red          = 4,
			Magenta      = 5,
			Orange       = 6,
			LightGrey    = 7,
			DarkGrey     = 8,
			LightBlue    = 9,
			LightGreen   = 10,
			LightCyan    = 11,
			LightRed     = 12,
			LightMagenta = 13,
			Yellow       = 14,
			White        = 15
		};

		enum class TtyMode : u8
		{
			Buffered,
			Immediate
		};

		void setWinSize(const WinSize* ws){ _winSize = *ws; }
		void setWinColor(const WinColor* wc){ _winColor = *wc; }
		void setWinCursor(const WinCursor* wcr){ _lastWinCursor = _curWinCursor; _curWinCursor = *wcr; }

		WinSize* winSize(void){ return &_winSize; }
		WinColor* winColor(void){ return &_winColor; }
		WinCursor* winCursor(void){ return &_curWinCursor; }

		void setState(TtyState state){ _state = state; }
		void setType(TtyType ty){ _ttyType = ty; }

		ErrNo setXoffHook(VoidFuncPtrVar xoffHook, sint xoffHookArg);

	protected:
		explicit TtyBase(void){}
		TtyBase(const TtyBase&) = delete;
		TtyBase& operator=(const TtyBase&) = delete;

		ErrNo ioctl(IoctlSet request, slong arg);

		void flushReadBuffer(void);
		void flushWriteBuffer(void);

		void tyRdXoff(register bool xoff);
		void tyWrtXoff(register bool xoff);

		void teminate(void);

		int readIn(const char* buffer, uint bytes);

		int writeOut(char* buffer, uint maxByteSize);

		ErrNo init(TtyType ttyType, Driver::DriverInterface* driver, uint rbufSize, uint wbufSize);

		void txStartup(void);

		enum class CtrlCharIndex : u8
		{
			VINTR    =   0 ,
			VQUIT    ,
			VERASE   ,
			VKILL    ,
			VEOF     ,
			VTIME    ,
			VMIN     ,
			VSWTC    ,
			VSTART   ,
			VSTOP    ,
			VSUSP    ,
			VEOL     ,
			VREPRINT ,
			VDISCARD ,
			VWERASE  ,
			VLNEXT   ,
			VEOL2    ,

			CtrlCharNum
		};

	protected:

		TtyState _state;
		TtyType  _ttyType;
		u8       _flags;
		c8       _ctrlChars[static_cast<u8>(CtrlCharIndex::CtrlCharNum)];

		WinColor _winColor;
		WinSize  _winSize;
		WinCursor    _curWinCursor;
		WinCursor    _lastWinCursor;

		Semaphore _rSem;
		Semaphore _wSem;
		Mutex     _mutex;

		SRingBuffer<char> _rBuffer;
		SRingBuffer<char> _wBuffer;

		S32FuncPtrVar _ctrlCHookFunc;
		pvoid         _ctrlCArg;
		time_t        _createdTime;

		VoidFuncPtrPvoid _txStartUpHookFunc;
		S32FuncPtrVar   _protoHookFunc;
		s32             _protoArg;

		struct
		{
			u8  _canceled;
			u8  _xoff;
			u8  _pending;
			u8  _flushingRBuffer;
		}_rState;

		struct
		{
			u8 _canceled;
			u8 _busy;
			u8 _xoff;
			u8 _cr;
			u8 _flushingWBuffer;
		}_wState;

		u8  _xoffThreshold;
		u8  _xonThreshold;
		u8  _wrtThreshold;

		VoidFuncPtrVar _xoffHookFunc;
		sint _xoffArg;

		u16  _bytesInUnfinishedLine;
		u16  _leftBytesInUnfinishedLine;
		s32 _options;

		Spinlock _spinlock;
	};
}

#endif

