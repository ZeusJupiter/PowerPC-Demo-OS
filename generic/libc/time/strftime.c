/*
 *   File name: strftime.c
 *
 *  Created on: 2017年6月25日, 下午10:14:14
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <time.h>
#include <string.h>

extern time_t timezone;

static const char* weekDays[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
};

static const char * abrWeekDays[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};

static const char * monthNames[] = {
	"January", "February", "March", "April", "May", "June", "July",
	"August", "September", "October", "November", "December",
};

static const char * abrMonthNames[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

static size_t gsize;
static char *pt;

static int _add(register const char *str)
{
	for (;; ++pt, --gsize) {
		if (!gsize)
			return(0);
		if (!(*pt = *str++))
			return(1);
	}
}

static int _conv(int n, int digits, int pad)
{
	char tmpBuffer[10];
	register char *p;

	for (p = tmpBuffer + sizeof(tmpBuffer) - 2; n > 0 && p > tmpBuffer; n /= 10, --digits)
		*p-- = n % 10 + '0';
	while (p > tmpBuffer && digits-- > 0)
		*p-- = pad;
	return (_add(++p));
}

static int _secs(const struct tm *t)
{
	char buffer[15];
	register time_t s;
	register char *p;
	struct tm tmp;

	tmp = *t;
	s = mktime(&tmp);
	for (p = buffer + sizeof(buffer) - 2; s > 0 && p > buffer; s /= 10)
		*p-- = s % 10 + '0';
	return (_add(++p));
}

static size_t _format(register const char *format, const struct tm *t)
{
	for (; *format; ++format) {
		if (*format == '%')
			switch(*++format) {
			case '\0':
				--format;
				break;
			case 'A':
				if (t->tm_wday < 0 || t->tm_wday > 6)
					return(0);
				if (!_add(weekDays[t->tm_wday]))
					return(0);
				continue;
			case 'a':
				if (t->tm_wday < 0 || t->tm_wday > 6)
					return(0);
				if (!_add(abrWeekDays[t->tm_wday]))
					return(0);
				continue;
			case 'B':
				if (t->tm_mon < 0 || t->tm_mon > 11)
					return(0);
				if (!_add(monthNames[t->tm_mon]))
					return(0);
				continue;
			case 'b':
			case 'h':
				if (t->tm_mon < 0 || t->tm_mon > 11)
					return(0);
				if (!_add(abrMonthNames[t->tm_mon]))
					return(0);
				continue;
			case 'C':
				if (!_format("%a %b %e %H:%M:%S %Y", t))
					return(0);
				continue;
			case 'c':
				if (!_format("%m/%d/%y %H:%M:%S", t))
					return(0);
				continue;
			case 'D':
				if (!_format("%m/%d/%y", t))
					return(0);
				continue;
			case 'd':
				if (!_conv(t->tm_mday, 2, '0'))
					return(0);
				continue;
			case 'e':
				if (!_conv(t->tm_mday, 2, ' '))
					return(0);
				continue;
			case 'H':
				if (!_conv(t->tm_hour, 2, '0'))
					return(0);
				continue;
			case 'I':
				if (!_conv(t->tm_hour % 12 ?
				    t->tm_hour % 12 : 12, 2, '0'))
					return(0);
				continue;
			case 'j':
				if (!_conv(t->tm_yday + 1, 3, '0'))
					return(0);
				continue;
			case 'k':
				if (!_conv(t->tm_hour, 2, ' '))
					return(0);
				continue;
			case 'l':
				if (!_conv(t->tm_hour % 12 ?
				    t->tm_hour % 12 : 12, 2, ' '))
					return(0);
				continue;
			case 'M':
				if (!_conv(t->tm_min, 2, '0'))
					return(0);
				continue;
			case 'm':
				if (!_conv(t->tm_mon + 1, 2, '0'))
					return(0);
				continue;
			case 'n':
				if (!_add("\n"))
					return(0);
				continue;
			case 'p':
				if (!_add(t->tm_hour >= 12 ? "PM" : "AM"))
					return(0);
				continue;
			case 'R':
				if (!_format("%H:%M", t))
					return(0);
				continue;
			case 'r':
				if (!_format("%I:%M:%S %p", t))
					return(0);
				continue;
			case 'S':
				if (!_conv(t->tm_sec, 2, '0'))
					return(0);
				continue;
			case 's':
				if (!_secs(t))
					return(0);
				continue;
			case 'T':
			case 'X':
				if (!_format("%H:%M:%S", t))
					return(0);
				continue;
			case 't':
				if (!_add("\t"))
					return(0);
				continue;
			case 'U':
				if (!_conv((t->tm_yday + 7 - t->tm_wday) / 7,
				    2, '0'))
					return(0);
				continue;
			case 'W':
				if (!_conv((t->tm_yday + 7 -
				    (t->tm_wday ? (t->tm_wday - 1) : 6))
				    / 7, 2, '0'))
					return(0);
				continue;
			case 'w':
				if (!_conv(t->tm_wday, 1, '0'))
					return(0);
				continue;
			case 'x':
				if (!_format("%m/%d/%y", t))
					return(0);
				continue;
			case 'y':
				if (!_conv((t->tm_year + TM_YEAR_BASE)
				    % 100, 2, '0'))
					return(0);
				continue;
			case 'Y':
				if (!_conv(t->tm_year + TM_YEAR_BASE, 4, '0'))
					return(0);
				continue;
			case 'Z':
                pt += sprintf(pt, "%d", (int)(timezone / 3600));
				continue;
			case '%':
	

			default:
				break;
		}
		if (!gsize--)
			return(0);
		*pt++ = *format;
	}
	return(gsize);
}

size_t strftime(s, maxsize, format, t)
	char *s;
	size_t maxsize;
	const char *format;
	const struct tm *t;
{

	pt = s;
	if ((gsize = maxsize) < 1)
		return(0);
	if (_format(format, t)) {
		*pt = '\0';
		return(maxsize - gsize);
	}
	return(0);
}

