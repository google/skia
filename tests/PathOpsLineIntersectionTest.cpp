/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkIntersections.h"
#include "SkPathOpsLine.h"
#include "Test.h"

// FIXME: add tests for intersecting, non-intersecting, degenerate, coincident
static const SkDLine tests[][2] = {
    {{{{192, 4}, {243, 4}}}, {{{246, 4}, {189, 4}}}},
    {{{{246, 4}, {189, 4}}}, {{{192, 4}, {243, 4}}}},
    {{{{5, 0}, {0, 5}}}, {{{5, 4}, {1, 4}}}},
    {{{{0, 0}, {1, 0}}}, {{{1, 0}, {0, 0}}}},
    {{{{0, 0}, {0, 0}}}, {{{0, 0}, {1, 0}}}},
    {{{{0, 1}, {0, 1}}}, {{{0, 0}, {0, 2}}}},
    {{{{0, 0}, {1, 0}}}, {{{0, 0}, {2, 0}}}},
    {{{{1, 1}, {2, 2}}}, {{{0, 0}, {3, 3}}}},
    {{{{166.86950047022856, 112.69654129527828}, {166.86948801592692, 112.69655741235339}}},
     {{{166.86960700313026, 112.6965477747386}, {166.86925794355412, 112.69656471103423}}}}
};

static const size_t tests_count = SK_ARRAY_COUNT(tests);

static const SkDLine noIntersect[][2] = {
    {{{{0, 0}, {1, 0}}}, {{{3, 0}, {2, 0}}}},
    {{{{0, 0}, {0, 0}}}, {{{1, 0}, {2, 0}}}},
    {{{{0, 1}, {0, 1}}}, {{{0, 3}, {0, 2}}}},
    {{{{0, 0}, {1, 0}}}, {{{2, 0}, {3, 0}}}},
    {{{{1, 1}, {2, 2}}}, {{{4, 4}, {3, 3}}}},
};

static const size_t noIntersect_count = SK_ARRAY_COUNT(noIntersect);

static void check_results(skiatest::Reporter* reporter, const SkDLine& line1, const SkDLine& line2,
                          const SkIntersections& ts) {
    for (int i = 0; i < ts.used(); ++i) {
        SkDPoint result1 = line1.xyAtT(ts[0][i]);
        SkDPoint result2 = line2.xyAtT(ts[1][i]);
        if (!result1.approximatelyEqual(result2)) {
            REPORTER_ASSERT(reporter, ts.used() != 1);
            result2 = line2.xyAtT(ts[1][i ^ 1]);
            REPORTER_ASSERT(reporter, result1.approximatelyEqual(result2));
            REPORTER_ASSERT(reporter, result1.approximatelyEqual(ts.pt(i).asSkPoint()));
        }
    }
}

static void PathOpsLineIntersectionTest(skiatest::Reporter* reporter) {
    size_t index;
    for (index = 0; index < tests_count; ++index) {
        const SkDLine& line1 = tests[index][0];
        const SkDLine& line2 = tests[index][1];
        SkIntersections ts;
        int pts = ts.intersect(line1, line2);
        REPORTER_ASSERT(reporter, pts);
        REPORTER_ASSERT(reporter, pts == ts.used());
        check_results(reporter, line1, line2, ts);
        if (line1[0] == line1[1] || line2[0] == line2[1]) {
            continue;
        }
        if (line1[0].fY == line1[1].fY) {
            double left = SkTMin(line1[0].fX, line1[1].fX);
            double right = SkTMax(line1[0].fX, line1[1].fX);
            ts.horizontal(line2, left, right, line1[0].fY, line1[0].fX != left);
            check_results(reporter, line2, line1, ts);
        }
        if (line2[0].fY == line2[1].fY) {
            double left = SkTMin(line2[0].fX, line2[1].fX);
            double right = SkTMax(line2[0].fX, line2[1].fX);
            ts.horizontal(line1, left, right, line2[0].fY, line2[0].fX != left);
            check_results(reporter, line1, line2, ts);
        }
        if (line1[0].fX == line1[1].fX) {
            double top = SkTMin(line1[0].fY, line1[1].fY);
            double bottom = SkTMax(line1[0].fY, line1[1].fY);
            ts.vertical(line2, top, bottom, line1[0].fX, line1[0].fY != top);
            check_results(reporter, line2, line1, ts);
        }
        if (line2[0].fX == line2[1].fX) {
            double top = SkTMin(line2[0].fY, line2[1].fY);
            double bottom = SkTMax(line2[0].fY, line2[1].fY);
            ts.vertical(line1, top, bottom, line2[0].fX, line2[0].fY != top);
            check_results(reporter, line1, line2, ts);
        }
    }
    for (index = 0; index < noIntersect_count; ++index) {
        const SkDLine& line1 = noIntersect[index][0];
        const SkDLine& line2 = noIntersect[index][1];
        SkIntersections ts;
        int pts = ts.intersect(line1, line2);
        REPORTER_ASSERT(reporter, !pts);    
        REPORTER_ASSERT(reporter, pts == ts.used());
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsLineIntersectionTest)
