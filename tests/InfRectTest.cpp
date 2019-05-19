/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRect.h"
#include "include/private/SkFloatingPoint.h"
#include "include/utils/SkRandom.h"
#include "tests/Test.h"

static void check_invalid(skiatest::Reporter* reporter,
                          SkScalar l, SkScalar t, SkScalar r, SkScalar b) {
    SkRect rect;
    rect.set(l, t, r, b);
    REPORTER_ASSERT(reporter, !rect.isFinite());
}

// Tests that isFinite() will reject any rect with +/-inf values
// as one of its coordinates.
DEF_TEST(InfRect, reporter) {
    float inf = SK_FloatInfinity;
    float nan = SK_FloatNaN;
    SkASSERT(!(nan == nan));
    SkScalar small = SkIntToScalar(10);
    SkScalar big = SkIntToScalar(100);

    REPORTER_ASSERT(reporter, SkRect::MakeEmpty().isFinite());

    SkRect rect = SkRect::MakeXYWH(small, small, big, big);
    REPORTER_ASSERT(reporter, rect.isFinite());

    const SkScalar invalid[] = { nan, inf, -inf };
    for (size_t i = 0; i < SK_ARRAY_COUNT(invalid); ++i) {
        check_invalid(reporter, small, small, big, invalid[i]);
        check_invalid(reporter, small, small, invalid[i], big);
        check_invalid(reporter, small, invalid[i], big, big);
        check_invalid(reporter, invalid[i], small, big, big);
    }
}

// need tests for SkStrSearch
