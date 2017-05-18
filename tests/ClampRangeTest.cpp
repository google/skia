/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRandom.h"
#include "Test.h"
#include "gradients/SkClampRange.h"

static skiatest::Reporter* gReporter;
#define R_ASSERT(cond) if (!(cond)) {      \
    SkDebugf("%d: %s\n", __LINE__, #cond); \
    REPORTER_ASSERT(gReporter, cond);      \
}

// Arbitrary sentinel values outside [0, 0xFFFF].
static const int kV0 = -42, kV1 = -53, kRamp = -64;

static void check_value(int64_t bigfx, int expected) {
    if (bigfx < 0) {
        R_ASSERT(expected == kV0);
    } else if (bigfx > kFracMax_SkGradFixed) {
        R_ASSERT(expected == kV1);
    } else if (bigfx == kFracMax_SkGradFixed) {
        // Either one is fine (and we do see both).
        R_ASSERT(expected == kV1 || expected == kRamp);
    } else {
        R_ASSERT(expected == kRamp);
    }
}

static void slow_check(const SkClampRange& range,
                       const SkGradFixed fx, SkGradFixed dx, int count) {
    SkASSERT(range.fCount0 + range.fCount1 + range.fCount2 == count);

    // If dx is large, fx will overflow if updated naively.  So we use more bits.
    int64_t bigfx = fx;

    for (int i = 0; i < range.fCount0; i++) {
        check_value(bigfx, range.fV0);
        bigfx += dx;
    }

    for (int i = 0; i < range.fCount1; i++) {
        check_value(bigfx, kRamp);
        bigfx += dx;
    }

    for (int i = 0; i < range.fCount2; i++) {
        check_value(bigfx, range.fV1);
        bigfx += dx;
    }
}


static void test_range(SkFixed fx, SkFixed dx, int count) {
    const SkGradFixed gfx = SkFixedToGradFixed(fx);
    const SkGradFixed gdx = SkFixedToGradFixed(dx);

    SkClampRange range;
    range.init(gfx, gdx, count, kV0, kV1);
    slow_check(range, gfx, gdx, count);
}

#define ff(x)   SkIntToFixed(x)

DEF_TEST(ClampRange, reporter) {
    gReporter = reporter;

    test_range(0, 0, 20);
    test_range(0xFFFF, 0, 20);
    test_range(-ff(2), 0, 20);
    test_range( ff(2), 0, 20);

    test_range(-10, 1, 20);
    test_range(10, -1, 20);
    test_range(-10, 3, 20);
    test_range(10, -3, 20);

    test_range(ff(1),  ff(16384),  100);
    test_range(ff(-1), ff(-16384), 100);
    test_range(ff(1)/2, ff(16384), 100);
    test_range(ff(1)/2, ff(-16384), 100);

    SkRandom rand;

    // test non-overflow cases
    for (int i = 0; i < 1000000; i++) {
        SkFixed fx = rand.nextS() >> 1;
        SkFixed sx = rand.nextS() >> 1;
        int count = rand.nextU() % 1000 + 1;
        SkFixed dx = (sx - fx) / count;
        test_range(fx, dx, count);
    }

    // TODO(reed): skia:2481, fix whatever bug this is, then uncomment
    /*
    // test overflow cases
    for (int i = 0; i < 100000; i++) {
        SkFixed fx = rand.nextS();
        SkFixed dx = rand.nextS();
        int count = rand.nextU() % 1000 + 1;
        test_range(fx, dx, count);
    }
    */
}
