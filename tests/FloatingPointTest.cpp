/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFloatingPoint.h"
#include "tests/Test.h"

#include <cfloat>
#include <cmath>

DEF_TEST(DoubleNearlyZero, reporter) {
    REPORTER_ASSERT(reporter, sk_double_nearly_zero(0.));
    REPORTER_ASSERT(reporter, sk_double_nearly_zero(-0.));
    REPORTER_ASSERT(reporter, sk_double_nearly_zero(DBL_EPSILON));
    REPORTER_ASSERT(reporter, sk_double_nearly_zero(-DBL_EPSILON));

    double nearly = 1. / 20000000000LL;
    REPORTER_ASSERT(reporter, nearly != 0);
    REPORTER_ASSERT(reporter, sk_double_nearly_zero(nearly));
    REPORTER_ASSERT(reporter, sk_double_nearly_zero(-nearly));

    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(FLT_EPSILON));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(-FLT_EPSILON));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(1));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(-1));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(INFINITY));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(HUGE_VALF));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(HUGE_VAL));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(HUGE_VALL));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(-INFINITY));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(-HUGE_VALF));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(-HUGE_VAL));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(-HUGE_VALL));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(NAN));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(-NAN));
}

DEF_TEST(DoubleNearlyEqualUlps, reporter) {
    // Our tolerance is looser than DBL_EPSILON
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(1., 1.));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(1., 1. - DBL_EPSILON));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(1., 1. + DBL_EPSILON));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(100.5, 100.5));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(100.5, 100.5 - DBL_EPSILON));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(100.5, 100.5 + DBL_EPSILON));


    // Our tolerance is tighter than FLT_EPSILON
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(1., 1. - FLT_EPSILON));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(1., 1. + FLT_EPSILON));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(100.5, 100.5 - FLT_EPSILON));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(100.5, 100.5 + FLT_EPSILON));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(0, 0.1));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(FLT_EPSILON, 0));

    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(INFINITY, INFINITY));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(INFINITY, 10));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(10, INFINITY));

    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(NAN, INFINITY));
}
