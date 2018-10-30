/*
 *   File name: signal.h
 *
 *  Created on: 2017年6月7日, 上午9:38:25
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_SIGNAL_H__
#define __INCLUDE_SIGNAL_H__

#include <stddef.h>

#define	SIGHUP	   1

#define	SIGINT	   2

#define	SIGQUIT	   3

#define	SIGILL	   4

#define	SIGTRAP	   5

#define	SIGABRT    6

#define	SIGEMT	   7

#define	SIGFPE	   8

#define	SIGKILL	   9

#define	SIGBUS	   10

#define	SIGSEGV	   11

#define SIGFMT	   12

#define	SIGPIPE	   13

#define	SIGALRM	   14

#define	SIGTERM	   15

#define SIGCNCL    16

#define	SIGSTOP	   17

#define	SIGTSTP	   18

#define	SIGCONT	   19

#define	SIGCHLD	   20

#define	SIGTTIN	   21

#define	SIGTTOU	   22

#define SIGRES1    23

#define SIGRES2    24

#define SIGRES3    25

#define SIGRES4    26

#define SIGRES5    27

#define SIGRES6    28

#define SIGRES7    29

#define	SIGUSR1    30

#define	SIGUSR2    31

#define	SIGPOLL    32

#define	SIGPROF    33

#define	SIGSYS     34

#define	SIGURG     35

#define	SIGVTALRM  36

#define	SIGXCPU    37

#define	SIGXFSZ    38

#define SIGEVTS    39

#define SIGEVTD    40

#define SIGRTMIN   48

#define SIGRTMAX   63

#define _NSIGS     63

#define SIG_ERR         (void (*)(int))-1
#define SIG_DFL         (void (*)(int))0
#define SIG_IGN         (void (*)(int))1

#define SA_NOCLDSTOP	0x0001

#define SA_SIGINFO	    0x0002

#define SA_ONSTACK	    0x0004

#define SA_INTERRUPT	0x0008

#define SA_RESETHAND	0x0010

#define SA_RESTART	    0x0020

#define SA_NODEFER      0x0040

#define SA_NOCLDWAIT    0x0080

#define SIG_BLOCK	1
#define SIG_UNBLOCK	2
#define SIG_SETMASK	3

#define SI_SYNC		0

#define SI_USER		1

#define SI_QUEUE	2

#define SI_TIMER	3

#define SI_ASYNCIO	4

#define SI_MESGQ	5

#define SI_CHILD	6

#define SI_KILL		SI_USER

#define ILL_ILLOPC      1

#define ILL_ILLOPN      2

#define ILL_ILLADR      3

#define ILL_ILLTRP      4

#define ILL_PRVOPC      5

#define ILL_PRVREG      6

#define ILL_COPROC      7

#define ILL_BADSTK      8

#define FPE_INTDIV      1

#define FPE_INTOVF      2

#define FPE_FLTDIV      3

#define FPE_FLTOVF      4

#define FPE_FLTUND      5

#define FPE_FLTRES      6

#define FPE_FLTSUB      7

#define SEGV_MAPERR     1

#define SEGV_ACCERR     2

#define BUS_ADRALNR     1

#define BUS_ADRERR      2

#define BUS_OBJERR      3

#define TRAP_BRKPT      1

#define TRAP_TRACE      2

#define CLD_EXITED      1

#define CLD_KILLED      2

#define CLD_DUMPED      3

#define CLD_TRAPPED     4

#define CLD_STOPPED     5

#define CLD_CONTINUED   6

#define POLL_IN   1

#define POLL_OUT  2

#define POLL_MSG  3

#define POLL_ERR  4

#define POLL_PRI  5

#define POLL_HUP  6

struct siginfo;
typedef VoidFuncPtrSint VoidSignalFuncPtrSint;
typedef void (*VoidSignalFuncPtrSintSigInfoPvoid)(sint, struct siginfo*, pvoid);

typedef VoidSignalFuncPtrSint sighandler_t;
typedef u64 sigset_t;

typedef union sigval
{
	sint  sival_int;
	pvoid sival_ptr;
} sigval_t;

typedef struct sigaction
{
	union
	{
		sighandler_t _sigFuncPtr;
		VoidSignalFuncPtrSintSigInfoPvoid _sigActionFuncPtr;
	};

	sigset_t _sigMask;
	sint _flags;
	VoidSignalFuncPtrSint _restorerFuncPtr;
}sigaction_t;

typedef struct sigvec
{
	sighandler_t       _sigVecHandler;
    sigset_t           _sigVecMask;
    sint               _sigVecFlags;
} sigvec_t;

struct iovec
{
    pvoid  _iovBufBaseAddr;
    uint   _iovBufLen;
};

typedef struct siginfo
{
	sint _signo;
	sint _errno;
	sint _code;

    union
	{
		sint _pad[5];

        struct
		{
            pid_t  _senderPid;
            uid_t  _senderUid;
        } _kill;

        struct
		{
        	ushort   _pad;
            ushort   _timerId;

            sint     _overrun;

        } _timer;

        struct
		{
            pid_t  	 _senderPid;
            uid_t     _senderUid;

        } _rt;

        struct
		{
            pid_t     _childPid;
            uid_t     _childUid;
            sint      _status;
            clock_t   _utime;
            clock_t   _stime;
        } _sigchld;

		struct
		{
			pvoid  _addr;
			ushort _pad;
			ushort _addr_lsb;

		} _sigfault;

        struct
		{
            sint  _band;
            sint  _fd;
        } _sigpoll;
    };

    sigval_t _value;
} siginfo_t;

typedef struct sigevent
{
	sint _notify;

	sint _signo;

	sigval_t _value;

	void (*_sigEvntNotifyFuncPtr)(sigval_t);

	pvoid _sigEvntnotifyAttributes;

	uint _notifiedThreadId;
	uint  _sigevId;

	uint _pad;
}sigevent_t;

#define sigev_notify_func   _notifyFuncPtr
#define sigev_notify_attr   _notifyAttributes
#define sigev_notify_thread _notifiedThreadId

#define SV_ONSTACK   SA_ONSTACK
#define SV_INTERRUPT SA_INTERRUPT
#define SV_RESETHAND SA_RESETHAND

#define sigmask(m) (((m <= 0) || (m > (_NSIGS + 1))) ? (sigset_t) 0 : ((sigset_t) 1 << ((m) - 1)))
#define SIGMASK(m) (((m <= 0) || (m > (_NSIGS + 1))) ? (sigset_t) 0 : ((sigset_t) 1 << ((m) - 1)))
#define sigIndex(m) ( (m) - 1 )
#define isASignal(m) ( 0 < (m) && (m) <= _NSIGS )

#define SIGEV_NONE                0x0
#define SIGEV_SIGNAL              0x1
#define SIGEV_TASK_SIGNAL         0x2
#define SIGEV_TIMER_SIGNAL        0x4
#define SIGEV_TIMER_TASK_SIGNAL   0x8
#define SIGEV_THREAD              0x10
#define SIGEV_TIMER_THREAD        0x20
#define SIGEV_MASK                0x3f

#define IS_SIGEV_TYPE_SUPPORTED(notify) ((notify) & SIGEV_MASK || !(notify))
#define IS_SIGEV_SIGNAL_TASK(notify) ((notify) & (SIGEV_TASK_SIGNAL | SIGEV_TIMER_TASK_SIGNAL))
#define IS_SIGEV_THREAD(notify) ((notify) & (SIGEV_THREAD | SIGEV_TIMER_THREAD))
#define IS_SIGEV_TIMER(notify) ((notify) & (SIGEV_TIMER_SIGNAL | SIGEV_TIMER_TASK_SIGNAL | SIGEV_TIMER_THREAD))

#define MINSIGSTKSZ             4096
#define	SIGSTKSZ                12288

typedef struct stSigaltStack
{
  pvoid     ss_sp;

  sint      ss_flags;

  size_t    ss_size;

} signalstack_t;

typedef signalstack_t stack_t;

#define SS_ONSTACK      0x00000001
#define SS_DISABLE      0x00000002

typedef struct stSigStack
{
	pvoid ss_sp;

	sint  ss_onstack;

} sigstack_t;

BEG_EXT_C

__DeclareFunc__(sint, sigstack, (sigstack_t * newSigStack, sigstack_t * oldSigStack));

__DeclareFunc__(sint, sigaltstack, (const signalstack_t * newSignalStack, signalstack_t *oldSignalStack));

__DeclareFunc__(sighandler_t, bsdsignal, (sint signum, sighandler_t sigHandler));

__DeclareFunc__(sighandler_t, signal, (sint signum, sighandler_t sigHandler));

__DeclareFunc__(sint, raise, (sint signum));

__DeclareFunc__(sint, kill, (uint Id, sint signum));
__DeclareFunc__(sint, sigqueue, (uint Id, sint signum, const sigval_t sigvalue));

inline sint sigemptyset(sigset_t *sigset) { *sigset = 0; return 0; }

inline sint sigfillset(sigset_t *sigset) { *sigset = ~0; return 0; }

inline sint sigaddset(sigset_t *sigset, sint signum){ *sigset |= sigmask(signum); return 0; }

inline sint sigdelset(sigset_t *sigset, sint signum){ *sigset &= ~(sigmask(signum)); return 0; }

inline sint sigismember(const sigset_t *sigset, sint signum){ return (((*sigset) & sigmask(signum)) != 0); }

__DeclareFunc__(sint, sigaction, (sint signum, const sigaction_t * newSigAction, sigaction_t * oldSigAction));

__DeclareFunc__(sint, siggetmask,(void));
__DeclareFunc__(sint, sigsetmask, (sint sigMask));

__DeclareFunc__(sint, sigblock, (sint mask));
__DeclareFunc__(sint, sighold, (sint signum));
__DeclareFunc__(sint, sigignore, (sint signum));
__DeclareFunc__(sint, sigrelse, (sint signum));

__DeclareFunc__(sint, sigpause, (sint sigMask));
__DeclareFunc__(sighandler_t, sigset, (sint signum, sighandler_t disp));

__DeclareFunc__(sint, siginterrupt, (sint signum, sint iFlag));

__DeclareFunc__(sint, sigprocmask, (sint how, const sigset_t *newSigset, sigset_t *oldSigset));

__DeclareFunc__(sint, sigvec, (sint signum, const sigvec_t *newSigvec, sigvec_t *oldSigvec));

__DeclareFunc__(sint, sigpending, (sigset_t *sigset));

__DeclareFunc__(sint, sigsuspend, (const sigset_t *sigsetMask));
__DeclareFunc__(sint, sigwait, (const sigset_t *sigset, sint *signum));
typedef struct timespec timespec_t;
__DeclareFunc__(sint, sigtimedwait, (const sigset_t *sigset, siginfo_t * value, const timespec_t *));
__DeclareFunc__(sint, sigwaitinfo, (const sigset_t *sigset, siginfo_t * siginfo));

END_EXT_C

#endif

