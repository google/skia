/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkPathOpsLine.h"
#include "Test.h"

static const SkDLine tests[] = {
    {{{2, 1}, {2, 1}}},
    {{{2, 1}, {1, 1}}},
    {{{2, 1}, {2, 2}}},
    {{{1, 1}, {2, 2}}},
    {{{3, 0}, {2, 1}}},
    {{{3, 2}, {1, 1}}},
};

static const SkDPoint left[] = {
    {2, 1},
    {1, 0},
    {1, 1},
    {1, 2},
    {2, 0},
    {2, 1}
};

static const size_t tests_count = SK_ARRAY_COUNT(tests);

DEF_TEST(PathOpsLineUtilities, reporter) {
    for (size_t index = 0; index < tests_count; ++index) {
        const SkDLine& line = tests[index];
        SkASSERT(ValidLine(line));
        SkDLine line2;
        SkPoint pts[2] = {line[0].asSkPoint(), line[1].asSkPoint()};
        line2.set(pts);
        REPORTER_ASSERT(reporter, line[0] == line2[0] && line[1] == line2[1]);
        const SkDPoint& pt = left[index];
        SkASSERT(ValidPoint(pt));
        double result = line.isLeft(pt);
        if ((result <= 0 && index >= 1) || (result < 0 && index == 0)) {
            SkDebugf("%s [%d] expected left\n", __FUNCTION__, index);
            REPORTER_ASSERT(reporter, 0);
        }
        SkDPoint mid = line.ptAtT(.5);
        REPORTER_ASSERT(reporter, approximately_equal((line[0].fX + line[1].fX) / 2, mid.fX));
        REPORTER_ASSERT(reporter, approximately_equal((line[0].fY + line[1].fY) / 2, mid.fY));
    }
}
