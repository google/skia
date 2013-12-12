/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "gradients/SkClampRange.h"
#include "SkRandom.h"

static skiatest::Reporter* gReporter;

static void debug_me() {
    if (NULL == gReporter) {
        SkDebugf("dsfdssd\n");
    }
}

#ifdef USE_REPORTER

#define R_ASSERT(cond)                  \
    do { if (!(cond)) {                 \
    debug_me();                         \
    REPORTER_ASSERT(gReporter, cond);   \
    }} while (0)

#else
#define R_ASSERT(cond)                  \
    do { if (!(cond)) {                 \
    debug_me();                         \
    }} while (0)
#endif

static int classify_value(SkFixed fx, int v0, int v1) {
    if (fx <= 0) {
        return v0;
    }
    if (fx >= 0xFFFF) {
        return v1;
    }
    R_ASSERT(false);
    return 0;
}

#define V0  -42
#define V1  1024

static void slow_check(const SkClampRange& range,
                       SkFixed fx, SkFixed dx, int count) {
    SkASSERT(range.fCount0 + range.fCount1 + range.fCount2 == count);

    int i;
    if (range.fOverflowed) {
        fx = range.fFx1;
        for (i = 0; i < range.fCount1; i++) {
            R_ASSERT(fx >= 0 && fx <= 0xFFFF);
            fx += dx;
        }
    } else {
        for (i = 0; i < range.fCount0; i++) {
            int v = classify_value(fx, V0, V1);
            R_ASSERT(v == range.fV0);
            fx += dx;
        }
        if (range.fCount1 > 0 && fx != range.fFx1) {
            SkDebugf("%x %x\n", fx, range.fFx1);
            R_ASSERT(false); // bad fFx1
            return;
        }
        for (i = 0; i < range.fCount1; i++) {
            R_ASSERT(fx >= 0 && fx <= 0xFFFF);
            fx += dx;
        }
        for (i = 0; i < range.fCount2; i++) {
            int v = classify_value(fx, V0, V1);
            R_ASSERT(v == range.fV1);
            fx += dx;
        }
    }
}


static void test_range(SkFixed fx, SkFixed dx, int count) {
    SkClampRange range;
    range.init(fx, dx, count, V0, V1);
    slow_check(range, fx, dx, count);
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

    // test overflow cases
    for (int i = 0; i < 100000; i++) {
        SkFixed fx = rand.nextS();
        SkFixed dx = rand.nextS();
        int count = rand.nextU() % 1000 + 1;
        test_range(fx, dx, count);
    }
}
