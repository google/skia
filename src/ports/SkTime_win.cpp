/* libs/graphics/ports/SkTime_Unix.cpp
**
** Copyright 2009, The Android Open Source Project
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

void SkTime::GetDateTime(DateTime* dt)
{
    if (dt)
    {
        SYSTEMTIME      st;
        GetSystemTime(&st);

        dt->fYear       = st.wYear;
        dt->fMonth      = SkToU8(st.wMonth + 1);
        dt->fDayOfWeek  = SkToU8(st.wDayOfWeek);
        dt->fDay        = SkToU8(st.wDay);
        dt->fHour       = SkToU8(st.wHour);
        dt->fMinute     = SkToU8(st.wMinute);
        dt->fSecond     = SkToU8(st.wSecond);
    }
}

SkMSec SkTime::GetMSecs()
{
    FILETIME        ft;
    LARGE_INTEGER   li;
    GetSystemTimeAsFileTime(&ft);
    li.LowPart  = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    __int64 t  = li.QuadPart;       /* In 100-nanosecond intervals */
    return (SkMSec)(t / 10000);               /* In milliseconds */
}
