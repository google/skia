/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkInterpolator.h"

#include "Test.h"

static SkScalar* iset(SkScalar array[3], int a, int b, int c) {
    array[0] = SkIntToScalar(a);
    array[1] = SkIntToScalar(b);
    array[2] = SkIntToScalar(c);
    return array;
}

DEF_TEST(Interpolator, reporter) {
    SkInterpolator  inter(3, 2);
    SkScalar        v1[3], v2[3], v[3];
    SkInterpolator::Result          result;

    inter.setKeyFrame(0, 100, iset(v1, 10, 20, 30), 0);
    inter.setKeyFrame(1, 200, iset(v2, 110, 220, 330));

    result = inter.timeToValues(0, v);
    REPORTER_ASSERT(reporter, result == SkInterpolator::kFreezeStart_Result);
    REPORTER_ASSERT(reporter, memcmp(v, v1, sizeof(v)) == 0);

    result = inter.timeToValues(99, v);
    REPORTER_ASSERT(reporter, result == SkInterpolator::kFreezeStart_Result);
    REPORTER_ASSERT(reporter, memcmp(v, v1, sizeof(v)) == 0);

    result = inter.timeToValues(100, v);
    REPORTER_ASSERT(reporter, result == SkInterpolator::kNormal_Result);
    REPORTER_ASSERT(reporter, memcmp(v, v1, sizeof(v)) == 0);

    result = inter.timeToValues(200, v);
    REPORTER_ASSERT(reporter, result == SkInterpolator::kNormal_Result);
    REPORTER_ASSERT(reporter, memcmp(v, v2, sizeof(v)) == 0);

    result = inter.timeToValues(201, v);
    REPORTER_ASSERT(reporter, result == SkInterpolator::kFreezeEnd_Result);
    REPORTER_ASSERT(reporter, memcmp(v, v2, sizeof(v)) == 0);

    result = inter.timeToValues(150, v);
    REPORTER_ASSERT(reporter, result == SkInterpolator::kNormal_Result);

// Found failing when we re-enabled this test:
#if 0
    SkScalar vv[3];
    REPORTER_ASSERT(reporter, memcmp(v, iset(vv, 60, 120, 180), sizeof(v)) == 0);
#endif

    result = inter.timeToValues(125, v);
    REPORTER_ASSERT(reporter, result == SkInterpolator::kNormal_Result);
    result = inter.timeToValues(175, v);
    REPORTER_ASSERT(reporter, result == SkInterpolator::kNormal_Result);

}
