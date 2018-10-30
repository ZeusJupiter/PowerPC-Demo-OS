/*
 *   File name: time.h
 *
 *  Created on: 2017年4月7日, 下午9:39:59
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_TIME_H__
#define __INCLUDE_TIME_H__

#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CLOCKS_PER_SEC	getSystemClockRate()
#define CLOCK_REALTIME			0x0
#define CLOCK_MONOTONIC			0x1
#define CLOCK_PROCESS_CPUTIME_ID  	0x2
#define CLOCK_THREAD_CPUTIME_ID   	0x3
#define TIMER_ABSTIME	0x1	
#define TIMER_RELTIME   (~TIMER_ABSTIME)
#define SECSPERMIN      60
#define MINSPERHOUR     60
#define HOURSPERDAY     24
#define DAYSPERWEEK     7
#define MONSPERYEAR     12
#define SECSPERHOUR     (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY      ((long) SECSPERHOUR * HOURSPERDAY)

#define DAYSPERYEAR  	365
#define DAYSPERLYEAR    366
#define CENTURY	        100
#define TM_THURSDAY	4
#define TM_SUNDAY	0
#define TM_MONDAY	1

#define EPOCH_WDAY      TM_THURSDAY
#define EPOCH_YEAR      1970
#define TM_YEAR_BASE    1900

#define ABBR		1
#define FULL		0

#define ASCBUF          "Day Mon dd hh:mm:ss yyyy\n\0"
#define MaxBufferSize   25
#define ZONEBUFFER      "UTC:UTC:000:ddmmhh:ddmmhh\0"

#define NAME		0
#define NAME2		1

#define TIMEOFFSET   	2
#define DSTON		0
#define DSTOFF		1
#define DATETIME	0
#define DATE   		1
#define TIMEO  		2
#define AMPM   		3
#define ZONE   		4
#define DST   		5

#define TIMEVAL_NANOSEC_MAX     1000000000

typedef struct timespec
{
    time_t tv_sec;
    ulong tv_nsec;

}timespec_t;

typedef struct timeval {
	time_t tv_sec;
	ulong  tv_usec;
} timeval_t;

typedef struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
} timezone_t;

typedef struct itimerspec
{
    struct timespec it_interval;
    struct timespec it_value;
}itimerspec_t;

typedef struct tm
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
} tm_t;

inline bool isleap(u32 y) { return ((((y) & 3) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0); }
__DeclareFunc__(int, getDayOfYear, (int y, int mon, int dayOfMon));
__DeclareFunc__(time_t, tm2time, (struct tm * sttm));
__DeclareFunc__(struct tm*, time2tm, (const time_t tt, struct tm * stm));

__DeclareFunc__( uint, _clocks_per_sec, (void));

__DeclareFunc__( char*, asctime, (const struct tm *_tptr));

__DeclareFunc__( clock_t, clock, (void));

__DeclareFunc__( char *, ctime, (const time_t *_cal));

__DeclareFunc__( double, difftime, (time_t _t1, time_t _t0));

__DeclareFunc__( struct tm *, gmtime, (const time_t *_tod));

__DeclareFunc__( struct tm *, localtime, (const time_t *_tod));

__DeclareFunc__( int, nanosleep, (const struct timespec *rqtp, struct timespec *rmtp));

__DeclareFunc__( time_t, mktime, (struct tm *_tptr));

__DeclareFunc__( size_t, strftime, (char *_s, size_t _n, const char *_format, const struct tm *_tptr));

__DeclareFunc__( time_t, time, (time_t *_tod));

__DeclareFunc__( char *, asctime_r, (const struct tm *_tm, char *_buffer));

__DeclareFunc__( char *, ctime_r, (const time_t *_cal, char *_buffer));

__DeclareFunc__( struct tm *, gmtime_r, (const time_t *_tod, struct tm *_result));

__DeclareFunc__( struct tm *, localtime_r, (const time_t *_tod, struct tm *_result));

__DeclareFunc__( int, clock_gettime, (clockid_t clock_id, struct timespec *tp));
__DeclareFunc__( int, clock_settime, (clockid_t clock_id, const struct timespec *tp));
__DeclareFunc__( int, clock_getres, (clockid_t clock_id, struct timespec *res));
__DeclareFunc__( int, clock_nanosleep, (clockid_t clock_id, int flags,
				const struct timespec * rqtp, struct timespec * rmtp));

__DeclareFunc__( int, _ext_asctime_r, (const struct tm *_tm, char *_buffer, size_t *_buflen));
__DeclareFunc__( char*, _ext_ctime_r, (const time_t *_cal, char *_buffer, size_t *_buflen));
__DeclareFunc__( int, _ext_gmtime_r, (const time_t *_tod, struct tm *_result));
__DeclareFunc__( int, _ext_localtime_r, (const time_t *_tod, struct tm *_result));

__DeclareFunc__( void, timezoneSet, (void));

#if defined(__cplusplus)
}

const s32 secondPreMinute = SECSPERMIN;
const s32 minutePreHour = MINSPERHOUR;
const s32 hoursPreDay = HOURSPERDAY;
const s32 secondPreHour = (secondPreMinute * minutePreHour);
const s32 secondPreDay = (secondPreHour * hoursPreDay);
const s32 daysPreWeek = DAYSPERWEEK;
const s32 monthsPreYear = MONSPERYEAR;
const s32 epochYearBase   =  EPOCH_YEAR;
const s32 timeYearBase  = TM_YEAR_BASE;
const s32 epochWDayBase = EPOCH_WDAY;
const u32 daysPreYear = DAYSPERYEAR;
const u32 daysPreLeapYear = DAYSPERLYEAR;
#endif

InlineStatic bool lessThanByTimespec(const struct timespec *tv1, const struct timespec *tv2)
{
	return (tv1->tv_sec < tv2->tv_sec ||
			(tv1->tv_sec == tv2->tv_sec && tv1->tv_nsec < tv2->tv_nsec));
}

InlineStatic void subTimespec2(struct timespec  *tv, const struct timespec  *tv2)
{
    tv->tv_sec  -= tv2->tv_sec;
    tv->tv_nsec -= tv2->tv_nsec;

	if (tv->tv_nsec >= TIMEVAL_NANOSEC_MAX)
	{
		tv->tv_sec++;
		tv->tv_nsec -= TIMEVAL_NANOSEC_MAX;
	}
	else if (tv->tv_nsec < 0)
	{
		tv->tv_sec--;
		tv->tv_nsec += TIMEVAL_NANOSEC_MAX;
	}
}

#endif

