/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkPathOpsBounds.h"
#include "Test.h"

static const SkRect sectTests[][2] = {
    {{2, 0, 4, 1}, {4, 0, 6, 1}},
    {{2, 0, 4, 1}, {3, 0, 5, 1}},
    {{2, 0, 4, 1}, {3, 0, 5, 0}},
    {{2, 0, 4, 1}, {3, 1, 5, 2}},
    {{2, 1, 4, 2}, {1, 0, 5, 3}},
    {{2, 1, 5, 3}, {3, 1, 4, 2}},
    {{2, 0, 4, 1}, {3, 0, 3, 0}},  // intersecting an empty bounds is OK
    {{2, 0, 4, 1}, {4, 1, 5, 2}},  // touching just on a corner is OK
};

static const size_t sectTestsCount = SK_ARRAY_COUNT(sectTests);

static const SkRect noSectTests[][2] = {
    {{2, 0, 4, 1}, {5, 0, 6, 1}},
    {{2, 0, 4, 1}, {3, 2, 5, 2}},
};

static const size_t noSectTestsCount = SK_ARRAY_COUNT(noSectTests);

static const SkRect reallyEmpty[] = {
    {0, 0, 0, 0},
    {1, 1, 1, 0},
    {1, 1, 0, 1},
    {1, 1, 0, 0},
    {1, 2, 3, SK_ScalarNaN},
};

static const size_t emptyTestsCount = SK_ARRAY_COUNT(reallyEmpty);

static const SkRect notReallyEmpty[] = {
    {0, 0, 1, 0},
    {0, 0, 0, 1},
    {0, 0, 1, 1},
};

static const size_t notEmptyTestsCount = SK_ARRAY_COUNT(notReallyEmpty);

DEF_TEST(PathOpsBounds, reporter) {
    for (size_t index = 0; index < sectTestsCount; ++index) {
        const SkPathOpsBounds& bounds1 = static_cast<const SkPathOpsBounds&>(sectTests[index][0]);
        SkASSERT(ValidBounds(bounds1));
        const SkPathOpsBounds& bounds2 = static_cast<const SkPathOpsBounds&>(sectTests[index][1]);
        SkASSERT(ValidBounds(bounds2));
        bool touches = SkPathOpsBounds::Intersects(bounds1, bounds2);
        REPORTER_ASSERT(reporter, touches);
    }
    for (size_t index = 0; index < noSectTestsCount; ++index) {
        const SkPathOpsBounds& bounds1 = static_cast<const SkPathOpsBounds&>(noSectTests[index][0]);
        SkASSERT(ValidBounds(bounds1));
        const SkPathOpsBounds& bounds2 = static_cast<const SkPathOpsBounds&>(noSectTests[index][1]);
        SkASSERT(ValidBounds(bounds2));
        bool touches = SkPathOpsBounds::Intersects(bounds1, bounds2);
        REPORTER_ASSERT(reporter, !touches);
    }
    SkPathOpsBounds bounds;
    bounds.setEmpty();
    bounds.add(1, 2, 3, 4);
    SkPathOpsBounds expected;
    expected.set(0, 0, 3, 4);
    REPORTER_ASSERT(reporter, bounds == expected);
    bounds.setEmpty();
    SkPathOpsBounds ordinal;
    ordinal.set(1, 2, 3, 4);
    bounds.add(ordinal);
    REPORTER_ASSERT(reporter, bounds == expected);
    SkPoint topLeft = {0, 0};
    bounds.setPointBounds(topLeft);
    SkPoint botRight = {3, 4};
    bounds.add(botRight);
    REPORTER_ASSERT(reporter, bounds == expected);
    for (size_t index = 0; index < emptyTestsCount; ++index) {
        const SkPathOpsBounds& bounds = static_cast<const SkPathOpsBounds&>(reallyEmpty[index]);
        // SkASSERT(ValidBounds(bounds));  // don't check because test may contain nan
        bool empty = bounds.isReallyEmpty();
        REPORTER_ASSERT(reporter, empty);
    }
    for (size_t index = 0; index < notEmptyTestsCount; ++index) {
        const SkPathOpsBounds& bounds = static_cast<const SkPathOpsBounds&>(notReallyEmpty[index]);
        SkASSERT(ValidBounds(bounds));
        bool empty = bounds.isReallyEmpty();
        REPORTER_ASSERT(reporter, !empty);
    }
    const SkPoint curvePts[] = {{0, 0}, {1, 2}, {3, 4}, {5, 6}};
    bounds.setLineBounds(curvePts);
    expected.set(0, 0, 1, 2);
    REPORTER_ASSERT(reporter, bounds == expected);
    (bounds.*SetCurveBounds[1])(curvePts);
    REPORTER_ASSERT(reporter, bounds == expected);
    bounds.setQuadBounds(curvePts);
    expected.set(0, 0, 3, 4);
    REPORTER_ASSERT(reporter, bounds == expected);
    (bounds.*SetCurveBounds[2])(curvePts);
    REPORTER_ASSERT(reporter, bounds == expected);
    bounds.setCubicBounds(curvePts);
    expected.set(0, 0, 5, 6);
    REPORTER_ASSERT(reporter, bounds == expected);
    (bounds.*SetCurveBounds[3])(curvePts);
    REPORTER_ASSERT(reporter, bounds == expected);
}
