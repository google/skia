
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkRect.h"

#ifdef SK_SCALAR_IS_FLOAT
static float make_zero() {
    return sk_float_sin(0);
}
#endif

static void check_invalid(skiatest::Reporter* reporter,
                          SkScalar l, SkScalar t, SkScalar r, SkScalar b) {
    SkRect rect;
    rect.set(l, t, r, b);
    REPORTER_ASSERT(reporter, !rect.isFinite());
}

// Tests that isFinite() will reject any rect with +/-inf values
// as one of its coordinates.
static void TestInfRect(skiatest::Reporter* reporter) {
#ifdef SK_SCALAR_IS_FLOAT
    float inf = 1 / make_zero();    // infinity
    float nan = inf * 0;
    SkASSERT(!(nan == nan));
#else
    SkFixed inf = SK_FixedNaN;
    SkFixed nan = SK_FixedNaN;
#endif
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

#include "TestClassDef.h"
DEFINE_TESTCLASS("InfRect", InfRectTestClass, TestInfRect)
