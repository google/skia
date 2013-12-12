/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkRandom.h"
#include "SkRect.h"

#ifdef SK_SCALAR_IS_FLOAT
static float make_zero() {
    return sk_float_sin(0);
}
#endif

struct RectCenter {
    SkIRect  fRect;
    SkIPoint fCenter;
};

static void test_center(skiatest::Reporter* reporter) {
    static const RectCenter gData[] = {
        { { 0, 0, 0, 0 }, { 0, 0 } },
        { { 0, 0, 1, 1 }, { 0, 0 } },
        { { -1, -1, 0, 0 }, { -1, -1 } },
        { { 0, 0, 10, 7 }, { 5, 3 } },
        { { 0, 0, 11, 6 }, { 5, 3 } },
    };
    for (size_t index = 0; index < SK_ARRAY_COUNT(gData); ++index) {
        REPORTER_ASSERT(reporter,
                        gData[index].fRect.centerX() == gData[index].fCenter.x());
        REPORTER_ASSERT(reporter,
                        gData[index].fRect.centerY() == gData[index].fCenter.y());
    }

    SkRandom rand;
    for (int i = 0; i < 10000; ++i) {
        SkIRect r;

        r.set(rand.nextS() >> 2, rand.nextS() >> 2,
              rand.nextS() >> 2, rand.nextS() >> 2);
        int cx = r.centerX();
        int cy = r.centerY();
        REPORTER_ASSERT(reporter, ((r.left() + r.right()) >> 1) == cx);
        REPORTER_ASSERT(reporter, ((r.top() + r.bottom()) >> 1) == cy);
    }
}

static void check_invalid(skiatest::Reporter* reporter,
                          SkScalar l, SkScalar t, SkScalar r, SkScalar b) {
    SkRect rect;
    rect.set(l, t, r, b);
    REPORTER_ASSERT(reporter, !rect.isFinite());
}

// Tests that isFinite() will reject any rect with +/-inf values
// as one of its coordinates.
DEF_TEST(InfRect, reporter) {
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

    test_center(reporter);
}

// need tests for SkStrSearch
