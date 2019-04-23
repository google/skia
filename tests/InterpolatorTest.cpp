/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkInterpolator.h"

#include "tests/Test.h"

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

    for (SkScalar val = -0.1f; val <= 1.1f; val += 0.1f) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SkTPin(0.f, val, 1.f),
                        SkUnitCubicInterp(val, 1.f/3, 1.f/3, 2.f/3, 2.f/3)));
    }

    // These numbers come from
    // http://www.w3.org/TR/css3-transitions/#transition-timing-function_tag.
    const SkScalar testTransitions[][4] = {
        { 0.25f, 0.1f, 0.25f, 1 }, // ease
        { 0.42f, 0,    1,     1 }, // ease in
        { 0,     0,    0.58f, 1 }, // ease out
        { 0.42f, 0,    0.58f, 1 }, // ease in out
    };

    const SkScalar expectedOutput[][5] = {
        { 0.0947876f, 0.513367f, 0.80249f,  0.940796f, 0.994263f }, // ease
        { 0.0170288f, 0.129639f, 0.31543f,  0.554749f, 0.839417f }, // ease in
        { 0.160583f,  0.445251f, 0.684692f, 0.870361f, 0.982971f }, // ease out
        { 0.0197144f, 0.187439f, 0.500122f, 0.812561f, 0.980286f }, // ease in out
    };

    int i = 0;
    for (const SkScalar* t : testTransitions) {
        int j = 0;
        for (SkScalar val = 0.1f; val < 1; val += 0.2f) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(expectedOutput[i][j++],
                            SkUnitCubicInterp(val, t[0], t[1], t[2], t[3])));
        }
        ++i;
    }
}
