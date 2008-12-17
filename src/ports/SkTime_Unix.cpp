/* libs/graphics/ports/SkTime_Unix.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkTime.h"

#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_MAC)
#include <sys/time.h>
#include <time.h>

void SkTime::GetDateTime(DateTime* dt)
{
    if (dt)
    {
        time_t m_time;
        time(&m_time);
        struct tm* tstruct;
        tstruct = localtime(&m_time);

        dt->fYear       = tstruct->tm_year;
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

#endif
