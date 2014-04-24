
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
