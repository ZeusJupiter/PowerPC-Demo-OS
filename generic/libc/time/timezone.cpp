/*
 *   File name: timezone.cpp
 *
 *  Created on: 2017年6月26日, 上午12:01:10
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <mk/oscfg.h>

#define TIMEZONE_BUFFER_SIZE  10

BEG_EXT_C

static const char defaultTimeZoneName[] = "CTS";

time_t timezone;

char* tzname[2];

static char timezoneNameBuf[TIMEZONE_BUFFER_SIZE];

void  timezoneSet (void)
{
	char tmpTZBuf[64];
	char* tzBufPtr;
	int tzdiff = 0;
	time_t tmpTimeZone;
	int alphaCount = 0;

    if (getenv_r("TZ", tmpTZBuf, 64) < 0) {
        timezone  = -(3600 * 8);
        strncpy(timezoneNameBuf, defaultTimeZoneName, ARRAY_SIZE(defaultTimeZoneName));
        tzname[0] = timezoneNameBuf;
        return;
    }

    tzBufPtr = tmpTZBuf;
    while (isalpha(*tzBufPtr)) {
        tzBufPtr++;
    }
    alphaCount = tzBufPtr - tmpTZBuf;
    alphaCount = (alphaCount < TIMEZONE_BUFFER_SIZE) ? alphaCount : (TIMEZONE_BUFFER_SIZE - 1);

    strlcpy(timezoneNameBuf,  tmpTZBuf, alphaCount + 1);
    tzname[0] = timezoneNameBuf;

    if (*tzBufPtr == '-') {
        tzdiff++;
        tzBufPtr++;
    }

    tmpTimeZone = (time_t)(atol(tzBufPtr) * 3600);
    while ((*tzBufPtr == '+') || ((*tzBufPtr >= '0') && (*tzBufPtr <= '9'))) {
        tzBufPtr++;
    }

    if ( *tzBufPtr == ':' ) {
        tzBufPtr++;
        tmpTimeZone += (time_t)(atol(tzBufPtr) * 60);

        while ((*tzBufPtr >= '0') && (*tzBufPtr <= '9') ) {
            tzBufPtr++;
        }

        if (*tzBufPtr == ':' ) {
            tzBufPtr++;
            tmpTimeZone += (time_t)atol(tzBufPtr);

            while ((*tzBufPtr >= '0') && (*tzBufPtr <= '9') ) {
                tzBufPtr++;
            }
        }
    }

    if (tzdiff) {
        tmpTimeZone = -tmpTimeZone;
    }

    timezone = tmpTimeZone;
}

END_EXT_C

