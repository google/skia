/* libs/graphics/animator/SkTime.cpp
**
** Copyright 2006, Google Inc.
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

#ifdef SK_BUILD_FOR_WIN

#ifdef SK_DEBUG
SkMSec gForceTickCount = (SkMSec) -1;
#endif

void SkTime::GetDateTime(DateTime* t)
{
    if (t)
    {
        SYSTEMTIME  syst;

        ::GetLocalTime(&syst);
        t->fYear        = SkToU16(syst.wYear);
        t->fMonth       = SkToU8(syst.wMonth);
        t->fDayOfWeek   = SkToU8(syst.wDayOfWeek);
        t->fDay         = SkToU8(syst.wDay);
        t->fHour        = SkToU8(syst.wHour);
        t->fMinute      = SkToU8(syst.wMinute);
        t->fSecond      = SkToU8(syst.wSecond);
    }
}

SkMSec SkTime::GetMSecs()
{
#ifdef SK_DEBUG
    if (gForceTickCount != (SkMSec) -1)
        return gForceTickCount;
#endif
    return ::GetTickCount();
}

#elif defined(xSK_BUILD_FOR_MAC)

#include <time.h>

void SkTime::GetDateTime(DateTime* t)
{
    if (t)
    {
        tm      syst;
        time_t  tm;
        
        time(&tm);
        localtime_r(&tm, &syst);
        t->fYear        = SkToU16(syst.tm_year);
        t->fMonth       = SkToU8(syst.tm_mon + 1);
        t->fDayOfWeek   = SkToU8(syst.tm_wday);
        t->fDay         = SkToU8(syst.tm_mday);
        t->fHour        = SkToU8(syst.tm_hour);
        t->fMinute      = SkToU8(syst.tm_min);
        t->fSecond      = SkToU8(syst.tm_sec);
    }
}

#include "Sk64.h"

SkMSec SkTime::GetMSecs()
{
    UnsignedWide    wide;
    Sk64            s;

    ::Microseconds(&wide);
    s.set(wide.hi, wide.lo);
    s.div(1000, Sk64::kRound_DivOption);
    return s.get32();
}

#endif

