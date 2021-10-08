/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkRandom.h"
#include "src/core/SkGeometry.h"
#include "src/gpu/GrVx.h"
#include "tests/Test.h"
#include <limits>
#include <numeric>

using namespace grvx;
using skvx::bit_pun;

DEF_TEST(grvx_cross_dot, r) {
    REPORTER_ASSERT(r, grvx::cross({0,1}, {0,1}) == 0);
    REPORTER_ASSERT(r, grvx::cross({1,0}, {1,0}) == 0);
    REPORTER_ASSERT(r, grvx::cross({1,1}, {1,1}) == 0);
    REPORTER_ASSERT(r, grvx::cross({1,1}, {1,-1}) == -2);
    REPORTER_ASSERT(r, grvx::cross({1,1}, {-1,1}) == 2);

    REPORTER_ASSERT(r, grvx::dot({0,1}, {1,0}) == 0);
    REPORTER_ASSERT(r, grvx::dot({1,0}, {0,1}) == 0);
    REPORTER_ASSERT(r, grvx::dot({1,1}, {1,-1}) == 0);
    REPORTER_ASSERT(r, grvx::dot({1,1}, {1,1}) == 2);
    REPORTER_ASSERT(r, grvx::dot({1,1}, {-1,-1}) == -2);

    SkRandom rand;
    for (int i = 0; i < 100; ++i) {
        float a=rand.nextRangeF(-1,1), b=rand.nextRangeF(-1,1), c=rand.nextRangeF(-1,1),
              d=rand.nextRangeF(-1,1);
        constexpr static float kTolerance = 1.f / (1 << 20);
        REPORTER_ASSERT(r, SkScalarNearlyEqual(
                grvx::cross({a,b}, {c,d}), SkPoint::CrossProduct({a,b}, {c,d}), kTolerance));
        REPORTER_ASSERT(r, SkScalarNearlyEqual(
                grvx::dot({a,b}, {c,d}), SkPoint::DotProduct({a,b}, {c,d}), kTolerance));
    }
}
