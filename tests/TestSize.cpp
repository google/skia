/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkSize.h"

static void TestISize(skiatest::Reporter* reporter) {
    SkISize  a, b;

    a.set(0, 0);
    REPORTER_ASSERT(reporter, a.isEmpty());
    a.set(5, -5);
    REPORTER_ASSERT(reporter, a.isEmpty());
    a.clampNegToZero();
    REPORTER_ASSERT(reporter, a.isEmpty());
    b.set(5, 0);
    REPORTER_ASSERT(reporter, a == b);

    a.set(3, 5);
    REPORTER_ASSERT(reporter, !a.isEmpty());
    b = a;
    REPORTER_ASSERT(reporter, !b.isEmpty());
    REPORTER_ASSERT(reporter, a == b);
    REPORTER_ASSERT(reporter, !(a != b));
    REPORTER_ASSERT(reporter,
                    a.fWidth == b.fWidth && a.fHeight == b.fHeight);
}

DEF_TEST(Size, reporter) {
    TestISize(reporter);

    SkSize a, b;
    int ix = 5;
    int iy = 3;
    SkScalar x = SkIntToScalar(ix);
    SkScalar y = SkIntToScalar(iy);

    a.set(0, 0);
    REPORTER_ASSERT(reporter, a.isEmpty());
    a.set(x, -x);
    REPORTER_ASSERT(reporter, a.isEmpty());
    a.clampNegToZero();
    REPORTER_ASSERT(reporter, a.isEmpty());
    b.set(x, 0);
    REPORTER_ASSERT(reporter, a == b);

    a.set(y, x);
    REPORTER_ASSERT(reporter, !a.isEmpty());
    b = a;
    REPORTER_ASSERT(reporter, !b.isEmpty());
    REPORTER_ASSERT(reporter, a == b);
    REPORTER_ASSERT(reporter, !(a != b));
    REPORTER_ASSERT(reporter,
                    a.fWidth == b.fWidth && a.fHeight == b.fHeight);

    SkISize ia;
    ia.set(ix, iy);
    a.set(x, y);
    REPORTER_ASSERT(reporter, a.toRound() == ia);
}
