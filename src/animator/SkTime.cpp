/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTime.h"

#ifdef SK_BUILD_FOR_WIN

#ifdef SK_DEBUG
SkMSec gForceTickCount = (SkMSec) -1;
#endif

void SkTime::GetDateTime(DateTime* t) {
    if (t) {
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

#elif defined(xSK_BUILD_FOR_MAC)

#include <time.h>

void SkTime::GetDateTime(DateTime* t) {
    if (t) {
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

#endif
