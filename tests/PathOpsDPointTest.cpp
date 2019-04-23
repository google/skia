/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/pathops/SkPathOpsPoint.h"
#include "tests/PathOpsTestCommon.h"
#include "tests/Test.h"

static const SkDPoint tests[] = {
    {0, 0},
    {1, 0},
    {0, 1},
    {2, 1},
    {1, 2},
    {1, 1},
    {2, 2}
};

static const size_t tests_count = SK_ARRAY_COUNT(tests);

DEF_TEST(PathOpsDPoint, reporter) {
    for (size_t index = 0; index < tests_count; ++index) {
        const SkDPoint& pt = tests[index];
        SkASSERT(ValidPoint(pt));
        SkDPoint p = pt;
        REPORTER_ASSERT(reporter, p == pt);
        REPORTER_ASSERT(reporter, !(pt != pt));
        SkDVector v = p - pt;
        p += v;
        REPORTER_ASSERT(reporter, p == pt);
        p -= v;
        REPORTER_ASSERT(reporter, p == pt);
        REPORTER_ASSERT(reporter, p.approximatelyEqual(pt));
        SkPoint sPt = pt.asSkPoint();
        p.set(sPt);
        REPORTER_ASSERT(reporter, p == pt);
        REPORTER_ASSERT(reporter, p.approximatelyEqual(sPt));
        REPORTER_ASSERT(reporter, p.roughlyEqual(pt));
        p.fX = p.fY = 0;
        REPORTER_ASSERT(reporter, p.fX == 0 && p.fY == 0);
        REPORTER_ASSERT(reporter, p.approximatelyZero());
        REPORTER_ASSERT(reporter, pt.distanceSquared(p) == pt.fX * pt.fX + pt.fY * pt.fY);
        REPORTER_ASSERT(reporter, approximately_equal(pt.distance(p),
                sqrt(pt.fX * pt.fX + pt.fY * pt.fY)));
    }
}
