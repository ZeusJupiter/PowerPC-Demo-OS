/*
 *   File name: tty.cpp
 *
 *  Created on: 2017年5月3日, 下午5:20:13
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <lockguard.h>
#include <user.h>
#include <sys/stat.h>

#include <tty/tty.h>

#include <kernel.h>
#include <task/generalproc.h>
#include <vfs/vfsnode.h>

#define PERCENT(percent, value)  (((percent) * (value) + 99) / 100)

#define getCtrlChar(C) (c8)((C) - '@')

#define XON  getCtrlChar('Q')
#define XOFF getCtrlChar('S')

namespace Driver
{
	S32FuncPtrVoid TtyBase::globalAbortFunc;
	ErrNo TtyBase::init(TtyType ttyType, Driver::DriverInterface* driver, uint rBufSize, uint wBufSize)
	{
		if(rBufSize > (MaxInput + 1)) rBufSize = MaxInput + 1;

		if(!_rBuffer.init(rBufSize) || !_wBuffer.init(wBufSize))
		{
			return ErrNo::ENOMEM;
		}

		_ttyType = ttyType;

		_rSem.init(0);
		_wSem.init(0);
		_mutex.init();

		_ctrlChars[static_cast<u8>(CtrlCharIndex::VINTR)]  = getCtrlChar('C');
		_ctrlChars[static_cast<u8>(CtrlCharIndex::VQUIT)]  = getCtrlChar('X');
		_ctrlChars[static_cast<u8>(CtrlCharIndex::VERASE)] = getCtrlChar('H');
		_ctrlChars[static_cast<u8>(CtrlCharIndex::VKILL)]  = getCtrlChar('U');
		_ctrlChars[static_cast<u8>(CtrlCharIndex::VEOF)]   = getCtrlChar('D');
		_ctrlChars[static_cast<u8>(CtrlCharIndex::VSTART)] = XON;
		_ctrlChars[static_cast<u8>(CtrlCharIndex::VSTOP)]  = XOFF;
		_ctrlChars[static_cast<u8>(CtrlCharIndex::VTIME)] = (c8)0;
		_ctrlChars[static_cast<u8>(CtrlCharIndex::VMIN)]  = (c8)1;

		_winSize._rows = 24;
		_winSize._cols = 80;
		_winSize._rowpixel = 24 * 16;

		_winSize._colpixel = 80 * 8;

		_winColor = { 0, 0, 0, 0 };

		_curWinCursor = { 0, 0 };
		_lastWinCursor = { 0, 0 };

		_ctrlCHookFunc = nullptr;
		_ctrlCArg = nullptr;

		_createdTime = 0;

		_protoHookFunc = nullptr;

		_protoArg = 0;

		_rState = {0, 0 ,0, 0};
		_wState = {0, 0, 0, 0, 0};

		_xoffThreshold = PERCENT(100 - XoffPercent, rBufSize);
		_xonThreshold = PERCENT(100 - XonPercent, rBufSize);

		_wrtThreshold = WriteThreshold;

		_xoffHookFunc = nullptr;
		_xoffArg = 0;

		_bytesInUnfinishedLine = 0;
		_leftBytesInUnfinishedLine = 0;
		_options = 0;

		_spinlock.init(0);

		flushReadBuffer();
		flushWriteBuffer();

		return ErrNo::ENONE;
	}

	void TtyBase::txStartup(void)
	{
		if(!_wState._busy)
		{
			_spinlock.lock();
			if(!_wState._busy)
			{
				_wState._busy = true;
				_spinlock.unlock();
				_txStartUpHookFunc(this);
				return;
			}
			_spinlock.unlock();
		}
	}

	ErrNo TtyBase::setXoffHook(VoidFuncPtrVar xoffHook, sint xoffHookArg)
	{
		if(xoffHook == nullptr)
			return ErrNo::EFAULT;

		{
			LockGuard<Mutex> g(_mutex);
			tyRdXoff(false);
		}

		xoffHook(xoffHookArg, false);

		_xoffHookFunc = xoffHook;
		_xoffArg = xoffHookArg;

		return ErrNo::ENONE;
	}

	void TtyBase::flushReadBuffer(void)
	{
		LockGuard<Mutex> g(_mutex);

		_rState._flushingRBuffer = true;
		{
			_rBuffer.reset();

			_bytesInUnfinishedLine = 0;
			_leftBytesInUnfinishedLine = 0;
		}

		_rSem.clear(0);

		tyRdXoff(false);

		_rState._flushingRBuffer = false;
	}
	void TtyBase::flushWriteBuffer(void)
	{
		LockGuard<Mutex> g(_mutex);
		_wState._flushingWBuffer = true;
		_wBuffer.reset();
		_wState._flushingWBuffer = false;
		_wSem.unlock();
	}

	void TtyBase::tyRdXoff(register bool xoff)
	{
		_spinlock.lock();

		if(_rState._xoff != xoff)
		{
			_rState._xoff = xoff;
			if(_xoffHookFunc == nullptr)
			{
				_rState._pending = true;
				if(!_wState._busy)
				{
					_wState._busy = true;

					_spinlock.unlock();

					_txStartUpHookFunc(this);
					return;
				}
			}
			else
			{
				_spinlock.unlock();
				_xoffHookFunc(_xoffArg, xoff);
				return;
			}
		}

		_spinlock.unlock();
	}

	void TtyBase::tyWrtXoff(register bool xoff)
	{
		_spinlock.lock();

		if(_wState._xoff != xoff)
		{
			_wState._xoff = xoff;

			if(!xoff && !_wState._busy)
			{
				_wState._busy = true;
				_spinlock.unlock();
				_txStartUpHookFunc(this);
				return;
			}
		}

		_spinlock.unlock();
	}

	void TtyBase::teminate(void)
	{
		LockGuard<Mutex> g(_mutex);

		_rState._canceled = true;
		_wState._canceled = true;

		_rState._flushingRBuffer = true;
		_wState._flushingWBuffer = true;

		_spinlock.unlock();

		_wBuffer.destroy();
	}

	int TtyBase::readIn(const char* buffer, uint bytes)
	{
		register sint outputBytes = 0;
		         sint bytesCount = bytes;
		KScheduler* curSheduler = KCommKernel::curSheduler();
		curSheduler->curThreadSafe();
		while(bytes > 0)
		{
			_wSem.lock();
			_mutex.lock();

			if(_wState._canceled)
			{
				_mutex.unlock();
				curSheduler->curthreadUnsafe();
				return (bytesCount - bytes);
			}

			_wState._busy = true;
			outputBytes = _wBuffer.writen(buffer, bytes);
			_wState._busy = false;

			txStartup();

			buffer += outputBytes;
			bytes -= outputBytes;

			if(_wBuffer.freeSpace())
				_wSem.unlock();

			_mutex.unlock();
		}

		curSheduler->curthreadUnsafe();
		return (bytesCount);
	}

	int TtyBase::writeOut(char* buffer, uint maxByteSize)
	{
		register sint nbytes = 0;
		register sint freeBytes = 0;

		_rSem.lock();

		while(true)
		{
			_mutex.lock();

			if(_rState._canceled)
			{
				_mutex.unlock();
				_rSem.unlock();

				return nbytes;
			}

			if(!_rBuffer.isEmpty()) break;

			_mutex.unlock();
		}

		if(_options & static_cast<word_t>(OptionFlags::OPT_LINE))
		{
			if(_bytesInUnfinishedLine == 0)
				_bytesInUnfinishedLine = maxByteSize;

			freeBytes = min<uint>(_bytesInUnfinishedLine, maxByteSize);
			nbytes = _rBuffer.readn(buffer, freeBytes);

			_bytesInUnfinishedLine = freeBytes - nbytes;
		}
		else
		{
			nbytes = _rBuffer.readn(buffer, maxByteSize);
		}

		if((_options & static_cast<word_t>(OptionFlags::OPT_TANDEM)) && _rState._xoff)
		{
			freeBytes = _rBuffer.freeSpace();
			if(_options & static_cast<word_t>(OptionFlags::OPT_LINE))
			{
				freeBytes -= _bytesInUnfinishedLine + 1;
			}

			if(freeBytes > _xonThreshold)
				tyRdXoff(false);
		}

		if(!_rBuffer.isEmpty())

			_rSem.unlock();

		_mutex.unlock();

		return nbytes;
	}

	ErrNo TtyBase::ioctl(IoctlSet request, slong arg)
	{
		register ErrNo ret = ErrNo::ENONE;

		switch(request)
		{
		case IoctlSet::FIONREAD:
			*((sint*)arg) = (sint)_rBuffer.length();
			break;
		case IoctlSet::FIONWRITE:
			*((sint*)arg) = (sint)_wBuffer.length();
			break;
		case IoctlSet::FIOFLUSH:
			flushReadBuffer();
			flushWriteBuffer();
			break;
		case IoctlSet::FIOWFLUSH:
			flushWriteBuffer();
			break;
		case IoctlSet::FIORFLUSH:
			flushReadBuffer();
			break;
		case IoctlSet::FIOGETOPTIONS:
			*((slong*)arg) = _options;
			break;
		case IoctlSet::FIOSETOPTIONS:
			{
				s32 oldOpt = _options;
				_options = (s32)arg;
				if((oldOpt & static_cast<word_t>(OptionFlags::OPT_LINE)) != (_options & static_cast<word_t>(OptionFlags::OPT_LINE)))
				{
					flushReadBuffer();
				}

				if((oldOpt & static_cast<word_t>(OptionFlags::OPT_TANDEM)) &&
						(_options & static_cast<word_t>(OptionFlags::OPT_TANDEM)))
				{
					LockGuard<Mutex> g(_mutex);
					tyRdXoff(false);
					tyWrtXoff(false);
				}
			}
			break;
		case IoctlSet::FIOCANCEL:
			{
				LockGuard<Mutex> g(_mutex);

				_rState._canceled = _rSem.blkTaskCount() > 0 ? true : false;
				if(_rState._canceled)
					_rSem.unlock();

				_wState._canceled = _wSem.blkTaskCount() > 0 ? true : false;
				if(_wState._canceled)
					_wSem.unlock();
			}
			break;
		case IoctlSet::FIOISATTY:
			*((bool*)arg) = true;
			break;
		case IoctlSet::FIOPROTOHOOK:
			_protoHookFunc = (S32FuncPtrVar)arg;
			break;
		case IoctlSet::FIOPROTOARG:
			_protoArg = (s32)arg;
			break;
		case IoctlSet::FIORBUFSET:
			{
				if((sint)arg > MaxInput)
					arg = MaxInput;

				if((sint)arg != _rBuffer.capacity())
				{
					LockGuard<Mutex> g(_mutex);
					_rState._flushingRBuffer = true;

					_rBuffer.destroy();
					if(_rBuffer.init((sint)arg))
					{
						_xoffThreshold = PERCENT(100 - XoffPercent, (sint)arg);
						_xonThreshold = PERCENT(100 - XonPercent, (sint)arg);
					}
					else
					{
						ret = ErrNo::ENOMEM;
					}

					_rState._flushingRBuffer = false;
				}
			}
			break;
		case IoctlSet::FIOWBUFSET:
			{
				if((sint)arg != _wBuffer.capacity())
				{
					LockGuard<Mutex> g(_mutex);
					_wState._flushingWBuffer = true;
					_wBuffer.destroy();
					if(!_wBuffer.init((sint)arg))
					{
						ret = ErrNo::ENOMEM;
					}
					_wState._flushingWBuffer = false;
				}
			}
			break;

		case IoctlSet::FIOFSTATGET:
				ret = ErrNo::EINVAL;
			break;

		case IoctlSet::FIOABORTFUNC:
			_ctrlCHookFunc = (S32FuncPtrVar)arg;
			break;
		case IoctlSet::FIOABORTARG:
			_ctrlCArg = (pvoid)arg;
			break;
		case IoctlSet::FIOSETCC:
			{
				c8* ctrlCharsPtr = (c8*)arg;
				if(ctrlCharsPtr)
				{
					LockGuard<Mutex> g(_mutex);
					memcpy(_ctrlChars, ctrlCharsPtr, static_cast<u8>(CtrlCharIndex::CtrlCharNum));
				}
				else
					ret = ErrNo::EFAULT;
			}
			break;
		case IoctlSet::FIOGETCC:
			{
				c8* ctrlCharsPtr = (c8*)arg;
				if(ctrlCharsPtr)
				{
					LockGuard<Mutex> g(_mutex);
					memcpy(ctrlCharsPtr, _ctrlChars, static_cast<u8>(CtrlCharIndex::CtrlCharNum));
				}
				else
					ret = ErrNo::EFAULT;
			}
			break;
		case IoctlSet::TIOCGWINSZ:
			{
				WinSize* ws = (WinSize*)arg;
				if(ws)
				{
					*ws = _winSize;
				}
				else
					ret = ErrNo::EFAULT;
			}
			break;
		case IoctlSet::TIOCSWINSZ:
			{
				WinSize* ws = (WinSize*)arg;
				if(ws != nullptr)
				{
					_winSize = *ws;
				}
				else
					ret = ErrNo::EFAULT;
			}
			break;

		default:
			ret = ErrNo::EINVAL;
		}

		return ret;
	}

	ErrNo TtyBase::iSendDataToTty(TtyBase* tyDev, c8 inChar)
	{
		register ErrNo ret = ErrNo::ENONE;

		if(tyDev->_protoHookFunc && (tyDev->_protoHookFunc)(tyDev->_protoArg) == 0)
		{
			return ret;
		}

		register s32 opts = tyDev->_options;

		if(opts & static_cast<word_t>(OptionFlags::OPT_7_BIT))
		{
			inChar &= 0x7f;
		}

		register Mutex& m = tyDev->_mutex;
		register Spinlock& s = tyDev->_spinlock;

		m.lock();
		s.lock();
		if(tyDev->_rState._flushingRBuffer)
		{
			s.unlock();
			m.unlock();
			return ret;
		}

		if( (inChar == tyDev->_ctrlChars[static_cast<u8>(CtrlCharIndex::VINTR)]) &&
				(opts & static_cast<word_t>(OptionFlags::OPT_ABORT)) &&
				(globalAbortFunc || tyDev->_ctrlCHookFunc) )
		{
			s.unlock();
			m.unlock();
			if(tyDev->_ctrlCHookFunc)
			{
				(tyDev->_ctrlCHookFunc)(tyDev->_ctrlCArg);
			}
			if(globalAbortFunc)
				globalAbortFunc();
		}
		else if( (inChar == tyDev->_ctrlChars[static_cast<u8>(CtrlCharIndex::VQUIT)]) &&
				(opts & static_cast<word_t>(OptionFlags::OPT_MON_TRAP)) )
		{
			s.unlock();
			m.unlock();
	

		}
		else if( ( (inChar == XOFF) || (inChar == XON) ) &&
				(opts & static_cast<word_t>(OptionFlags::OPT_TANDEM)) )
		{
			s.unlock();
			m.unlock();
			tyDev->tyWrtXoff(inChar == XOFF);
		}
		else
		{
			if( (opts & static_cast<word_t>(OptionFlags::OPT_CRMOD)) &&
				(opts & static_cast<word_t>(OptionFlags::OPT_LINE)) &&
				(inChar == '\0') )
			{
				s.unlock();
				m.unlock();
			}

			if( (opts & static_cast<word_t>(OptionFlags::OPT_CRMOD)) &&
				(inChar == '\r') )
			{
				inChar = '\n';
			}

			register SRingBuffer<char>& buf = tyDev->_rBuffer;
			register sint freeBytes ;

			bool releaseTask = false;
			bool hasEraseCharOrKillLine = false;

			if( !(opts & static_cast<word_t>(OptionFlags::OPT_LINE)) )

			{
				if( !(buf.write(inChar)) )
				{
					ret = ErrNo::EFAULT;
				}

				if(buf.length() == 1)
					releaseTask = true;
			}
			else

			{
				freeBytes = buf.freeSpace();
				if(inChar == 127 || (inChar == tyDev->_ctrlChars[static_cast<u8>(CtrlCharIndex::VERASE)]))
				{
					if(tyDev->_bytesInUnfinishedLine)
					{
						tyDev->_bytesInUnfinishedLine--;
						hasEraseCharOrKillLine = true;
					}
					else
					{
						ret = ErrNo::EFAULT;
					}
				}
				else if( inChar == tyDev->_ctrlChars[static_cast<u8>(CtrlCharIndex::VKILL)] )
				{
					if(tyDev->_bytesInUnfinishedLine)
					{
						tyDev->_bytesInUnfinishedLine = 0;
						hasEraseCharOrKillLine = true;
					}
					else
					{
						ret = ErrNo::EFAULT;
					}
				}
				else if( inChar == tyDev->_ctrlChars[static_cast<u8>(CtrlCharIndex::VEOF)] )
				{
					if(freeBytes > 0)
					{
						releaseTask = true;
					}
				}
				else
				{
					if(freeBytes >= 2)
					{
						if(freeBytes >= (tyDev->_bytesInUnfinishedLine + 3) && \
						   (tyDev->_bytesInUnfinishedLine < (MaxCanOn - 1)) )
						{
							tyDev->_bytesInUnfinishedLine++;
						}
						else
							ret = ErrNo::EFAULT;

						if( ret == ErrNo::ENONE || ( inChar == '\n' && tyDev->_bytesInUnfinishedLine++ ) )
						{
							buf.write(inChar);
						}

						if(inChar == '\n') releaseTask = true;
					}
					else
					{
						ret = ErrNo::EFAULT;
					}
				}

				if(releaseTask)
				{
					tyDev->_bytesInUnfinishedLine = 0;
				}
			}

			bool charEchoOk = false;

			if( (opts & static_cast<word_t>(OptionFlags::OPT_ECHO)) &&

				(ret == ErrNo::ENONE) && (!tyDev->_wState._busy) &&
				(!tyDev->_wState._flushingWBuffer))
			{
				buf = tyDev->_wBuffer;
				if(opts & static_cast<word_t>(OptionFlags::OPT_LINE))

				{
					if(hasEraseCharOrKillLine)
					{
						if(inChar == tyDev->_ctrlChars[static_cast<u8>(CtrlCharIndex::VKILL)])
						{
							charEchoOk = buf.write('\n');
							charEchoOk = true;
						}
						else
						{
							buf.write(inChar);
							buf.write(' ');
							buf.write(inChar);
							charEchoOk = true;
						}
					}
					else if( (inChar < 0x20) && (inChar != '\n') )

					{
						buf.write('^');
						buf.write(inChar + '@');
						charEchoOk = true;
					}
					else
					{
						buf.write(inChar);
						charEchoOk = true;
					}
				}
				else

				{
					buf.write(inChar);
					charEchoOk = true;
				}
			}

			s.unlock();
			m.unlock();

			if(charEchoOk)
			{
				tyDev->txStartup();
				ret = ErrNo::ENONE;
			}

			if( (opts & static_cast<word_t>(OptionFlags::OPT_TANDEM)) &&
				!(opts & static_cast<word_t>(OptionFlags::OPT_LINE)) )
			{
				s.lock();
				if(tyDev->_rState._flushingRBuffer)
				{
					s.unlock();
					ret = ErrNo::EFAULT;
				}
				else
				{
					freeBytes = tyDev->_rBuffer.freeSpace();
					s.unlock();
					if(!tyDev->_rState._xoff)
					{
						if(freeBytes < tyDev->_xoffThreshold)
						{
							tyDev->tyRdXoff(true);
							releaseTask = true;
						}
					}
					else
					{
						if(freeBytes > tyDev->_xoffThreshold)
						{
							tyDev->tyRdXoff(false);
						}
					}
				}
			}

			if(releaseTask)
			{
				tyDev->_rSem.unlock();
			}
		}

		return ret;
	}
	ErrNo TtyBase::iTransmitDataOfTty(TtyBase* tyDev, c8* outChar)
	{
		register Mutex& m = tyDev->_mutex;
		register Spinlock& s = tyDev->_spinlock;
		register SRingBuffer<char>& buf = tyDev->_wBuffer;

		m.lock();
		s.lock();

		if(tyDev->_rState._pending)
		{
			tyDev->_rState._pending = false;
			*outChar = (char)(tyDev->_rState._xoff ? XOFF : XON);
		}
		else if(tyDev->_wState._xoff || tyDev->_wState._flushingWBuffer)
		{
			tyDev->_wState._busy = false;
		}
		else if(tyDev->_wState._cr)
		{
			*outChar = '\n';
			tyDev->_wState._cr = false;
			tyDev->_wState._busy = true;
			goto there;
		}
		else if(!buf.read(outChar))
		{
			tyDev->_wState._busy = false;
		}
		else
		{
			tyDev->_wState._busy = true;
			if( (tyDev->_options & static_cast<word_t>(OptionFlags::OPT_CRMOD)) &&
			   (*outChar == '\n') )
			{
				*outChar = '\r';
				tyDev->_wState._cr = true;
			}

		there:
			if(buf.freeSpace() >= tyDev->_wrtThreshold)
			{
				s.unlock();
				m.unlock();

				tyDev->_wSem.unlock();

				return ErrNo::ENONE;
			}
		}

		ErrNo ret = (tyDev->_wState._busy) ? (ErrNo::ENONE) : (ErrNo::EFAULT);

		s.unlock();
		m.unlock();

		return ret;
	}
}
