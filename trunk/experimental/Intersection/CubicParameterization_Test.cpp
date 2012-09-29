/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "Intersection_Tests.h"
#include "Parameterization_Test.h"
#include "TestUtilities.h"

const Quadratic quadratics[] = {
    {{0, 0}, {1, 0}, {1, 1}},
};

const size_t quadratics_count = sizeof(quadratics) / sizeof(quadratics[0]);

int firstCubicCoincidenceTest = 0;

void CubicCoincidence_Test() {
    // split large quadratic
    // upscale quadratics to cubics
    // compare original, parts, to see if the are coincident
    for (size_t index = firstCubicCoincidenceTest; index < quadratics_count; ++index) {
        const Quadratic& test = quadratics[index];
        QuadraticPair split;
        chop_at(test, split, 0.5);
        Quadratic midThird;
        sub_divide(test, 1.0/3, 2.0/3, midThird);
        Cubic whole, first, second, mid;
        quad_to_cubic(test, whole);
        quad_to_cubic(split.first(), first);
        quad_to_cubic(split.second(), second);
        quad_to_cubic(midThird, mid);
        if (!implicit_matches(whole, first)) {
            printf("%s-1 %d\n", __FUNCTION__, (int)index);
        }
        if (!implicit_matches(whole, second)) {
            printf("%s-2 %d\n", __FUNCTION__, (int)index);
        }
        if (!implicit_matches(mid, first)) {
            printf("%s-3 %d\n", __FUNCTION__, (int)index);
        }
        if (!implicit_matches(mid, second)) {
            printf("%s-4 %d\n", __FUNCTION__, (int)index);
        }
        if (!implicit_matches(first, second)) {
            printf("%s-5 %d\n", __FUNCTION__, (int)index);
        }
    }
}

// pairs of coincident cubics
// The on curve points of each cubic should be on both parameterized cubics.
const Cubic cubics[] = {
  {
    { 1,     -1},
    { 1.0/3,  1},
    {-1.0/3, -1},
    {-1,      1}
  },
  {
    {-1,     1},
    {-1.0/3, -1},
    { 1.0/3,  1},
    { 1,     -1}
  },
  {
    {0, 2},
    {0, 1},
    {1, 0},
    {2, 0}
  }, {
    {2, 0},
    {1, 0},
    {0, 1},
    {0, 2}
  },
  {
    {315.74799999999999, 312.83999999999997},
    {312.64400000000001, 318.13400000000001},
    {305.83600000000001, 319.90899999999999},
    {300.54199999999997, 316.80399999999997}
  }, {
    {317.12200000000001, 309.05000000000001},
    {316.11200000000002, 315.10199999999998},
    {310.38499999999999, 319.19},
    {304.33199999999999, 318.17899999999997}
  }
};

const size_t cubics_count = sizeof(cubics) / sizeof(cubics[0]);

int firstCubicParameterizationTest = 0;

void CubicParameterization_Test() {
    for (size_t index = firstCubicParameterizationTest; index < cubics_count; ++index) {
        for (size_t inner = 0; inner < 4; inner += 3) {
            if (!point_on_parameterized_curve(cubics[index], cubics[index][inner])) {
                    printf("%s [%zu,%zu] 1 parameterization failed\n",
                        __FUNCTION__, index, inner);
            }
            if (!point_on_parameterized_curve(cubics[index], cubics[index ^ 1][inner])) {
                    printf("%s [%zu,%zu] 2 parameterization failed\n",
                        __FUNCTION__, index, inner);
            }
        }
        if (!implicit_matches(cubics[index], cubics[index ^ 1])) {
            printf("%s %d\n", __FUNCTION__, (int)index);
        }
    }
}
