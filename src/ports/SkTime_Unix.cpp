
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTime.h"
#include "SkTypes.h"

#include <sys/time.h>
#include <time.h>

void SkTime::GetDateTime(DateTime* dt)
{
    if (dt)
    {
        tzset();  // initialize timezone variable;
        time_t m_time;
        time(&m_time);
        struct tm* tstruct;
        tstruct = localtime(&m_time);
        int offset = tstruct->tm_isdst == 1 ? 60 : 0;

        // http://pubs.opengroup.org/onlinepubs/009695399/basedefs/time.h.html
        dt->fTimeZoneMinutes = SkToS16(offset - timezone / 60);
        dt->fYear       = tstruct->tm_year + 1900;
        dt->fMonth      = SkToU8(tstruct->tm_mon + 1);
        dt->fDayOfWeek  = SkToU8(tstruct->tm_wday);
        dt->fDay        = SkToU8(tstruct->tm_mday);
        dt->fHour       = SkToU8(tstruct->tm_hour);
        dt->fMinute     = SkToU8(tstruct->tm_min);
        dt->fSecond     = SkToU8(tstruct->tm_sec);
    }
}

SkMSec SkTime::GetMSecs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (SkMSec) (tv.tv_sec * 1000 + tv.tv_usec / 1000 ); // microseconds to milliseconds
}
