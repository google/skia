/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTime.h"
#include "Test.h"

// Sanity checks for the GetDateTime function.
DEF_TEST(Time_GetDateTime, r) {
    SkTime::DateTime dateTime;
    SkTime::GetDateTime(&dateTime);

    // TODO(future generation): update these values.
    const uint16_t kMinimumSaneYear = 1964;
    const uint16_t kMaximumSaneYear = 2064;

    if (dateTime.fYear < kMinimumSaneYear) {
        ERRORF(r,
               "SkTime::GetDateTime: %u (CurrentYear) < %u (MinimumSaneYear)",
               static_cast<unsigned>(dateTime.fYear),
               static_cast<unsigned>(kMinimumSaneYear));
    }
    if (dateTime.fYear > kMaximumSaneYear) {
        ERRORF(r,
               "SkTime::GetDateTime: %u (CurrentYear) > %u (MaximumSaneYear)",
               static_cast<unsigned>(dateTime.fYear),
               static_cast<unsigned>(kMaximumSaneYear));
    }

    REPORTER_ASSERT(r, dateTime.fMonth >= 1);
    REPORTER_ASSERT(r, dateTime.fMonth <= 12);

    REPORTER_ASSERT(r, dateTime.fDay >= 1);
    REPORTER_ASSERT(r, dateTime.fDay <= 31);

    REPORTER_ASSERT(r, dateTime.fHour <= 23);

    REPORTER_ASSERT(r, dateTime.fMinute <= 59);

    REPORTER_ASSERT(r, dateTime.fSecond <= 60);  // leap seconds are 23:59:60

    // The westernmost timezone is -12:00.
    // The easternmost timezone is +14:00.
    REPORTER_ASSERT(r, abs(SkToInt(dateTime.fTimeZoneMinutes)) <= 14 * 60);

    SkString timeStamp;
    dateTime.toISO8601(&timeStamp);
    REPORTER_ASSERT(r, timeStamp.size() > 0);
    if (r->verbose()) {  // `dm --veryVerbose`
        SkDebugf("\nCurrent Time (ISO-8601 format): \"%s\"\n",
                 timeStamp.c_str());
    }
}
