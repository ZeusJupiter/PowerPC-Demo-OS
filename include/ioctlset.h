/*
 *   File name: ioctlset.h
 *
 *  Created on: 2017年5月24日, 下午9:25:15
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_IOCTLSET_H__
#define __INCLUDE_IOCTLSET_H__

enum class IoctlSet : sint
{
	FIONREAD        =   1,
	FIOFLUSH        =   2,
	FIOOPTIONS      =   3,
	FIOBAUDRATE     =   4,
	FIODISKFORMAT   =   5,
	FIODISKINIT     =   6,
	FIOSEEK         =   7,
	FIOWHERE        =   8,
	FIORENAME       =   10,
	FIOREADYCHANGE  =   11,
	FIONWRITE     = 12,
	FIODISKCHANGE = 13,
	FIOCANCEL     = 14,
	FIOSQUEEZE    = 15,
	FIONBIO       = 16,
	FIONMSGS      = 17,
	FIOGETNAME    = 18,
	FIOGETOPTIONS = 19,
	FIOSETOPTIONS = FIOOPTIONS,
	FIOISATTY     = 20,
	FIOSYNC      = 21,
	FIOPROTOHOOK = 22,
	FIOPROTOARG  = 23,
	FIORBUFSET   = 24,
	FIOWBUFSET   = 25,
	FIORFLUSH    = 26,
	FIOWFLUSH    = 27,
	FIOSELECT       = 28,
	FIOUNSELECT     = 29,
	FIONFREE        = 30,
	FIOMKDIR        = 31,
	FIORMDIR        = 32,
	FIOLABELGET     = 33,
	FIOLABELSET     = 34,
	FIOATTRIBSET    = 35,
	FIOCONTIG       = 36,
	FIOREADDIR      = 37,
	FIOFSTATGET_OLD = 38,
	FIOUNMOUNT      = 39,
	FIOSCSICOMMAND  = 40,
	FIONCONTIG      = 41,
	FIOTRUNC        = 42,
	FIOGETFL        = 43,
	FIOTIMESET      = 44,
	FIOINODETONAME  = 45,
	FIOFSTATFSGET   = 46,
	FIOMOVE         = 47,

	FIOCHKDSK     = 48,
	FIOCONTIG64   = 49,
	FIONCONTIG64  = 50,
	FIONFREE64    = 51,
	FIONREAD64    = 52,
	FIOSEEK64     = 53,
	FIOWHERE64    = 54,
	FIOTRUNC64    = 55,

	FIOCOMMITFS   = 56,
	FIODATASYNC   = 57,

	FIOLINK       = 58,
	FIOUNLINK     = 59,
	FIOACCESS     = 60,
	FIOPATHCONF   = 61,
	FIOFCNTL      = 62,
	FIOCHMOD      = 63,
	FIOFSTATGET   = 64,
	FIOUPDATE     = 65,

	FIOABORTFUNC  = 67,
	FIOABORTARG   = 68,
	FIOSETCC      = 69,
	FIOGETCC      = 70,
	TIOCGWINSZ    = 71,
	TIOCSWINSZ    = 72,

	FIOFSTYPE ,
	FIOGETFORCEDEL ,
};

#endif

