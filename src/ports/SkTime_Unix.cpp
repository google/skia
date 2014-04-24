
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTime.h"

#include <sys/time.h>
#include <time.h>

void SkTime::GetDateTime(DateTime* dt) {
    if (dt) {
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

#ifdef __MACH__
#  include <mach/mach_time.h>

namespace {

struct ConversionFactor {
    ConversionFactor() {
        mach_timebase_info_data_t timebase;
        mach_timebase_info(&timebase);
        toNanos = (double) timebase.numer / timebase.denom;
    }
    double toNanos;
};

}  // namespace

SkNSec SkTime::GetNSecs() {
    static ConversionFactor convert;  // Since already know we're on Mac, this is threadsafe.
    return mach_absolute_time() * convert.toNanos;
}

#else  // Linux, presumably all others too

SkNSec SkTime::GetNSecs() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (SkNSec)(time.tv_sec * 1000000000 + time.tv_nsec);
}

#endif
