/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "src/pathops/SkPathOpsPoint.h"
#include "src/pathops/SkPathOpsTypes.h"
#include "tests/PathOpsTestCommon.h"
#include "tests/Test.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

static const SkDPoint tests[] = {
    {0, 0},
    {1, 0},
    {0, 1},
    {2, 1},
    {1, 2},
    {1, 1},
    {2, 2}
};

static const size_t tests_count = std::size(tests);

DEF_TEST(PathOpsDVector, reporter) {
    for (size_t index = 0; index < tests_count - 1; ++index) {
        SkDVector v1 = tests[index + 1] - tests[index];
        SkASSERT(ValidVector(v1));
        SkDVector v2 = tests[index] - tests[index + 1];
        SkASSERT(ValidVector(v2));
        v1 += v2;
        REPORTER_ASSERT(reporter, v1.fX == 0 && v1.fY == 0);
        v2 -= static_cast<decltype(v2)&>(v2);
        REPORTER_ASSERT(reporter, v2.fX == 0 && v2.fY == 0);
        v1 = tests[index + 1] - tests[index];
        v1 /= 2;
        v1 *= 2;
        v1 -= tests[index + 1] - tests[index];
        REPORTER_ASSERT(reporter, v1.fX == 0 && v1.fY == 0);
        SkVector sv = v1.asSkVector();
        REPORTER_ASSERT(reporter, sv.fX == 0 && sv.fY == 0);
        v1 = tests[index + 1] - tests[index];
        double lenSq = v1.lengthSquared();
        double v1Dot = v1.dot(v1);
        REPORTER_ASSERT(reporter, lenSq == v1Dot);
        REPORTER_ASSERT(reporter, approximately_equal(sqrt(lenSq), v1.length()));
        double v1Cross = v1.cross(v1);
        REPORTER_ASSERT(reporter, v1Cross == 0);
    }
}

DEF_TEST(SkDVector_normalize, reporter) {
    // See also SkVx_normalize
    auto assertDoublesEqual = [&](double left, double right) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(left, right), "%f != %f", left, right);
    };
    SkDVector first{1.2, 3.4};
    first.normalize();
    REPORTER_ASSERT(reporter, first.isFinite());
    assertDoublesEqual(first.fX, 0.332820);
    assertDoublesEqual(first.fY, 0.942990);

    SkDVector second{2.3, -4.5};
    second.normalize();
    REPORTER_ASSERT(reporter, second.isFinite());
    assertDoublesEqual(second.fX,  0.455111);
    assertDoublesEqual(second.fY, -0.890435);
}

DEF_TEST(SkDVector_normalize_infinity_and_nan, reporter) {
    // See also SkVx_normalize_infinity_and_nan
    SkDVector first{0, 0};
    first.normalize();
    REPORTER_ASSERT(reporter, !first.isFinite());
    REPORTER_ASSERT(reporter, std::isnan(first.fX), "%f is not nan", first.fX);
    REPORTER_ASSERT(reporter, std::isnan(first.fY), "%f is not nan", first.fY);

    SkDVector second{std::numeric_limits<double>::max(),
                     std::numeric_limits<double>::max()};
    second.normalize();
    REPORTER_ASSERT(reporter, second.isFinite());
    REPORTER_ASSERT(reporter, second.fX == 0, "%f != 0", second.fX);
    REPORTER_ASSERT(reporter, second.fY == 0, "%f != 0", second.fY);
}

