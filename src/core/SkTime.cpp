/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOncePtr.h"
#include "SkString.h"
#include "SkTime.h"

void SkTime::DateTime::toISO8601(SkString* dst) const {
    if (dst) {
        int timeZoneMinutes = SkToInt(fTimeZoneMinutes);
        char timezoneSign = timeZoneMinutes >= 0 ? '+' : '-';
        int timeZoneHours = SkTAbs(timeZoneMinutes) / 60;
        timeZoneMinutes = SkTAbs(timeZoneMinutes) % 60;
        dst->printf("%04u-%02u-%02uT%02u:%02u:%02u%c%02d:%02d",
                    static_cast<unsigned>(fYear), static_cast<unsigned>(fMonth),
                    static_cast<unsigned>(fDay), static_cast<unsigned>(fHour),
                    static_cast<unsigned>(fMinute),
                    static_cast<unsigned>(fSecond), timezoneSign, timeZoneHours,
                    timeZoneMinutes);
    }
}

#if defined(_MSC_VER)
    // TODO: try std::chrono again with MSVC 2015?
    #include <intrin.h>
    SK_DECLARE_STATIC_ONCE_PTR(double, ns_per_tick);
    double SkTime::GetNSecs() {
        uint64_t ticks = __rdtsc();
        return ticks * *ns_per_tick.get([]{
            LARGE_INTEGER khz;  // The docs say this returns Hz, but it returns KHz.
            QueryPerformanceFrequency(&khz);
            return new double(1e6 / khz.QuadPart);
        });
    }
#else
    // This std::chrono code looks great on Linux, Mac, and Android,
    // but MSVC 2013 returns mostly garbage (0ns times, etc).
    // This is ostensibly fixed in MSVC 2015.
    #include <chrono>
    double SkTime::GetNSecs() {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::nano> ns = now.time_since_epoch();
        return ns.count();
    }
#endif
