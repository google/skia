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

static void PathOpsLineIntersectionTest(skiatest::Reporter* reporter) {
    size_t index;
    for (index = 0; index < tests_count; ++index) {
        const SkDLine& line1 = tests[index][0];
        const SkDLine& line2 = tests[index][1];
        SkIntersections ts;
        int pts = ts.intersect(line1, line2);
        REPORTER_ASSERT(reporter, pts);
        for (int i = 0; i < pts; ++i) {
            SkDPoint result1 = line1.xyAtT(ts[0][i]);
            SkDPoint result2 = line2.xyAtT(ts[1][i]);
            if (!result1.approximatelyEqual(result2)) {
                REPORTER_ASSERT(reporter, pts != 1);
                result2 = line2.xyAtT(ts[1][i ^ 1]);
                REPORTER_ASSERT(reporter, result1.approximatelyEqual(result2));
            }
        }
    }
    for (index = 0; index < noIntersect_count; ++index) {
        const SkDLine& line1 = noIntersect[index][0];
        const SkDLine& line2 = noIntersect[index][1];
        SkIntersections ts;
        int pts = ts.intersect(line1, line2);
        REPORTER_ASSERT(reporter, !pts);    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsLineIntersectionTest)
