/*
 *   File name: tmtime.c
 *
 *  Created on: 2017年4月21日, 下午11:14:28
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <time.h>

BEG_EXT_C

int getDayOfYear(int y, int mon, int day)
{
	static const int __days_array_[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
	int leap;
	leap = mon > 1 ? (isleap(y + timeYearBase)) : 0;

	return (__days_array_[mon] + day + leap - 1);
}

inline int _daysSinceEpoch_(int offsetYear, int curDays)
{
	if(offsetYear >= 0)
		return ((365 * offsetYear) + (offsetYear + 1)/4 + curDays);
	else
		return ((365 * offsetYear) + (offsetYear - 2) / 4  + curDays);
}

time_t tm2time(struct tm * sttm)
{
	assert(sttm != nullptr);
	assert(sttm->tm_year + timeYearBase < epochYearBase);

	time_t t;

	t = _daysSinceEpoch_(sttm->tm_year + (timeYearBase - epochYearBase), sttm->tm_yday);
	t *= (3600 * 24);
	t += sttm->tm_sec + sttm->tm_min * 60 + sttm->tm_hour * 3600;
	return t;
}

#define EPOCH_ADJUSTMENT_DAYS	719468L

struct tm* time2tm(const time_t tt, struct tm * stm)
{
	long days, rem;
	int year, mon;

	days = ((long)tt) / secondPreDay + EPOCH_ADJUSTMENT_DAYS;
	rem = ((long)tt) % secondPreDay ;

	if(rem < 0)
	{
		rem += secondPreDay;
		--days;
	}

	stm->tm_hour = (int)(rem/secondPreHour);
	rem %= secondPreHour;
	stm->tm_min = (int)(rem/secondPreMinute);
	stm->tm_sec = (int)(rem%secondPreMinute);

	stm->tm_wday = (int)((days + epochWDayBase)%daysPreWeek);
	if(stm->tm_wday < 0)
		stm->tm_wday += daysPreWeek;

	year = days/daysPreYear;
	while(_daysSinceEpoch_(year, 0) > days) --year;

	days -= _daysSinceEpoch_(year, 0);
	year += epochYearBase;

	for(mon = 0; (days >= getDayOfYear(year - timeYearBase, mon + 1, 0)) && (mon < 11); ++mon);

	stm->tm_year = year - timeYearBase;
	stm->tm_mon = mon;
	stm->tm_mday = (days - getDayOfYear(stm->tm_year, mon, 0)) + 1;
	stm->tm_yday = getDayOfYear(stm->tm_year, mon, stm->tm_mday) - 1;
	stm->tm_isdst = 0;

	return stm;
}

END_EXT_C

