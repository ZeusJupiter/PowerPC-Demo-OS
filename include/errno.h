/*
 *   File name: errno.h
 *
 *  Created on: 2017年3月4日, 下午9:45:52
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_ERRNO_H__
#define __INCLUDE_ERRNO_H__

#if defined(__cplusplus)

enum class ErrNo : sint
{
    ENONE         = 0,
    EPERM         = 1,

    ENOENT        = 2,

    ESRCH         = 3,

    EINTR         = 4,

    EIO           = 5,

    ENXIO         = 6,

    E2BIG         = 7,

    ENOEXEC       = 8,

    EBADF         = 9,

    ECHILD        = 10,

    EAGAIN        = 11,

    ENOMEM        = 12,

    EACCES        = 13,

    EFAULT        = 14,

    ENOTEMPTY     = 15,

    EBUSY         = 16,

    EEXIST        = 17,

    EXDEV         = 18,

    ENODEV        = 19,

    ENOTDIR       = 20,

    EISDIR        = 21,

    EINVAL        = 22,

    ENFILE        = 23,

    EMFILE        = 24,

    ENOTTY        = 25,

    ENAMETOOLONG  = 26,

    EFBIG         = 27,

    ENOSPC        = 28,

    ESPIPE        = 29,

    EROFS         = 30,

    EMLINK        = 31,

    EPIPE         = 32,

    EDEADLK       = 33,

    EDEADLOCK   =  EDEADLK,
    ENOLCK        = 34,

    ENOTSUP       = 35,

    EMSGSIZE      = 36,

    EDOM    = 37,

    ERANGE  = 38,

    EDESTADDRREQ = 40,

    EPROTOTYPE   = 41,

    ENOPROTOOPT  = 42,

    EPROTONOSUPPORT = 43,

    ESOCKTNOSUPPORT = 44,

    EOPNOTSUPP      = 45,

    EPFNOSUPPORT    = 46,

    EAFNOSUPPORT    = 47,

    EADDRINUSE      = 48,

    EADDRNOTAVAIL   = 49,

    ENOTSOCK        = 50,

    ENETUNREACH  = 51,

    ENETRESET    = 52,

    ECONNABORTED = 53,

    ECONNRESET   = 54,

    ENOBUFS      = 55,

    EISCONN      = 56,

    ENOTCONN     = 57,

    ESHUTDOWN    = 58,

    ETOOMANYREFS = 59,

    ETIMEDOUT    = 60,

    ECONNREFUSED = 61,

    ENETDOWN     = 62,

    ETXTBSY      = 63,

    ELOOP        = 64,

    EHOSTUNREACH = 65,

    ENOTBLK      = 66,

    EHOSTDOWN    = 67,

    EINPROGRESS  = 68,

    EALREADY     = 69,

    EWOULDBLOCK  = 70,

    ENOSYS  = 71,

    ECANCELED = 72,

    ENOSR      = 74,

    ENOSTR     = 75,

    EPROTO     = 76,

    EBADMSG    = 77,

    ENODATA    = 78,

    ETIME      = 79,

    ENOMSG     = 80,

    EFPOS      = 81,

    EILSEQ     = 82,

    EIDRM     = 84,

    EOVERFLOW = 85,

	EWRPROTECT = 90,

	EFORMAT = 91,

	ECHRNG = 93,

	EL2NSYNC = 94,

	EL3HLT = 95,

	EL3RST = 96,

	ELNRNG = 97,

	EUNATCH = 98,

	ENOCSI = 99,

    EL2HLT      =  100,

    EBADE       =  101,

    EBADR       =  102,

    EXFULL      =  103,

    ENOANO      =  104,

    EBADRQC     =  105,

    EBADSLT     =  106,

    EBFONT      =  107,

    ENONET      =  108,

    ENOPKG      =  109,

    EREMOTE     =  110,

    ENOLINK     =  111,

    EADV        =  112,

    ESRMNT      =  113,

    ECOMM       =  114,

    EMULTIHOP   =  115,

    EDOTDOT     =  116,

    EUCLEAN     =  117,

    ENOTUNIQ    =  118,

    EBADFD      =  119,

    EREMCHG     =  120,

    ELIBACC     =  121,

    ELIBBAD     =  122,

    ELIBSCN     =  123,

    ELIBMAX     =  124,

    ELIBEXEC    =  125,

    ERESTART    =  126,

    ESTRPIPE    =  127,

    EUSERS      =  128,

    ESTALE      =  129,

    ENOTNAM     =  130,

    ENAVAIL     =  131,

    EISNAM      =  132,

    EREMOTEIO   =  133,

    EDQUOT      =  134,

    ENOMEDIUM   =  135,

    EMEDIUMTYPE =  136,

	ENODRV   = 200,

	EADDDEV  = 201,

    ERRMAX
};

#else
enum ErrNo

{
	ENONE         = 0,
	EPERM		  = 1,

	ENOENT		  = 2,

	ESRCH		  = 3,

	EINTR		  = 4,

	EIO		      = 5,

	ENXIO		  = 6,

	E2BIG		  = 7,

	ENOEXEC		  = 8,

	EBADF		  = 9,

	ECHILD		  = 10,

	EAGAIN		  = 11,

	ENOMEM		  = 12,

	EACCES		  = 13,

	EFAULT		  = 14,

	ENOTEMPTY	  = 15,

	EBUSY		  = 16,

	EEXIST		  = 17,

	EXDEV		  = 18,

	ENODEV		  = 19,

	ENOTDIR		  = 20,

	EISDIR		  = 21,

	EINVAL		  = 22,

	ENFILE		  = 23,

	EMFILE		  = 24,

	ENOTTY		  = 25,

	ENAMETOOLONG  = 26,

	EFBIG		  = 27,

	ENOSPC		  = 28,

	ESPIPE		  = 29,

	EROFS		  = 30,

	EMLINK		  = 31,

	EPIPE		  = 32,

	EDEADLK		  = 33,

	ENOLCK		  = 34,

	ENOTSUP		  = 35,

	EMSGSIZE	  = 36,

	EDOM	= 37,

	ERANGE	= 38,

	EDESTADDRREQ = 40,

	EPROTOTYPE	 = 41,

	ENOPROTOOPT	 = 42,

	EPROTONOSUPPORT	= 43,

	ESOCKTNOSUPPORT	= 44,

	EOPNOTSUPP	    = 45,

	EPFNOSUPPORT	= 46,

	EAFNOSUPPORT	= 47,

	EADDRINUSE	    = 48,

	EADDRNOTAVAIL   = 49,

	ENOTSOCK	    = 50,

	ENETUNREACH	 = 51,

	ENETRESET	 = 52,

	ECONNABORTED = 53,

	ECONNRESET	 = 54,

	ENOBUFS		 = 55,

	EISCONN		 = 56,

	ENOTCONN	 = 57,

	ESHUTDOWN	 = 58,

	ETOOMANYREFS = 59,

	ETIMEDOUT	 = 60,

	ECONNREFUSED = 61,

	ENETDOWN	 = 62,

	ETXTBSY		 = 63,

	ELOOP		 = 64,

	EHOSTUNREACH = 65,

	ENOTBLK		 = 66,

	EHOSTDOWN	 = 67,

	EINPROGRESS	 = 68,

	EALREADY	 = 69,

	EWOULDBLOCK	 = 70,

	ENOSYS	= 71,

	ECANCELED = 72,

	ENOSR      = 74,

	ENOSTR     = 75,

	EPROTO     = 76,

	EBADMSG    = 77,

	ENODATA    = 78,

	ETIME      = 79,

	ENOMSG     = 80,

    EFPOS      = 81,

    EILSEQ     = 82,

    EIDRM     = 84,

    EOVERFLOW = 85,

	EWRPROTECT = 90,

	EFORMAT = 91,

	ECHRNG = 93,

	EL2NSYNC = 94,

	EL3HLT = 95,

	EL3RST = 96,

	ELNRNG = 97,

	EUNATCH = 98,

	ENOCSI = 99,

    EL2HLT      =  100,

    EBADE       =  101,

    EBADR       =  102,

    EXFULL      =  103,

    ENOANO      =  104,

    EBADRQC     =  105,

    EBADSLT     =  106,

    EBFONT      =  107,

    ENONET      =  108,

    ENOPKG      =  109,

    EREMOTE     =  110,

    ENOLINK     =  111,

    EADV        =  112,

    ESRMNT      =  113,

    ECOMM       =  114,

    EMULTIHOP   =  115,

    EDOTDOT     =  116,

    EUCLEAN     =  117,

    ENOTUNIQ    =  118,

    EBADFD      =  119,

    EREMCHG     =  120,

    ELIBACC     =  121,

    ELIBBAD     =  122,

    ELIBSCN     =  123,

    ELIBMAX     =  124,

    ELIBEXEC    =  125,

    ERESTART    =  126,

    ESTRPIPE    =  127,

    EUSERS      =  128,

    ESTALE      =  129,

    ENOTNAM     =  130,

    ENAVAIL     =  131,

    EISNAM      =  132,

    EREMOTEIO   =  133,

    EDQUOT      =  134,

    ENOMEDIUM   =  135,

    EMEDIUMTYPE =  136,

	ENODRV   = 200,

	EADDDEV  = 201,

    ERRMAX
};

#endif

EXTERN_C errno_t *__errno(void);

#if !defined( __errno )
#define __errno __errno
#endif

#if !defined( errno )
#define errno   (*__errno())
#endif

#endif

