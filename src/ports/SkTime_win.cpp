
/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTime.h"

void SkTime::GetDateTime(DateTime* dt)
{
    if (dt)
    {
        SYSTEMTIME      st;
        TIME_ZONE_INFORMATION timeZoneInfo;
        int tz_bias;
        GetLocalTime(&st);
        // https://gist.github.com/wrl/8924636
        switch (GetTimeZoneInformation(&timeZoneInfo)) {
            case TIME_ZONE_ID_STANDARD:
                tz_bias = -timeZoneInfo.Bias - timeZoneInfo.StandardBias;
                break;
            case TIME_ZONE_ID_DAYLIGHT:
                tz_bias = -timeZoneInfo.Bias - timeZoneInfo.DaylightBias;
                break;
            default:
                tz_bias = -timeZoneInfo.Bias;
                break;
        }
        dt->fTimeZoneMinutes = SkToS16(tz_bias);
        dt->fYear       = st.wYear;
        dt->fMonth      = SkToU8(st.wMonth);
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
