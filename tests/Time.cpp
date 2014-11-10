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
    const uint16_t kMinimumSaneYear = 2014;
    const uint16_t kMaximumSaneYear = 2064;
    REPORTER_ASSERT(r, dateTime.fYear >= kMinimumSaneYear);
    REPORTER_ASSERT(r, dateTime.fYear <= kMaximumSaneYear);

    REPORTER_ASSERT(r, dateTime.fMonth >= 1);
    REPORTER_ASSERT(r, dateTime.fMonth <= 12);

    REPORTER_ASSERT(r, dateTime.fDay >= 1);
    REPORTER_ASSERT(r, dateTime.fDay <= 31);

    REPORTER_ASSERT(r, dateTime.fHour <= 23);

    REPORTER_ASSERT(r, dateTime.fMinute <= 59);

    REPORTER_ASSERT(r, dateTime.fSecond <= 60);  // leap seconds are 23:59:60
}
