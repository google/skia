/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkSafe32.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkPointPriv.h"
#include "tests/Test.h"

#include <algorithm>
#include <array>
#include <cstdint>

/*
   Duplicates lots of code from gpu/src/GrPathUtils.cpp
   It'd be nice not to do so, but that code's set up currently to only have
   a single implementation.
*/

// Sk uses 6, Gr (implicitly) used 10, both apparently arbitrarily.
#define MAX_COEFF_SHIFT     6
static const uint32_t MAX_POINTS_PER_CURVE = 1 << MAX_COEFF_SHIFT;

// max + 0.5 min has error [0.0, 0.12]
// max + 0.375 min has error [-.03, 0.07]
// 0.96043387 max + 0.397824735 min has error [-.06, +.05]
// For determining the maximum possible number of points to use in
// drawing a quadratic, we want to err on the high side.
static inline int cheap_distance(SkScalar dx, SkScalar dy) {
    int idx = SkAbs32(SkScalarRoundToInt(dx));
    int idy = SkAbs32(SkScalarRoundToInt(dy));
    if (idx > idy) {
        idx += idy >> 1;
    } else {
        idx = idy + (idx >> 1);
    }
    return idx;
}

static inline int estimate_distance(const SkPoint points[]) {
    return cheap_distance(points[1].fX * 2 - points[2].fX - points[0].fX,
                          points[1].fY * 2 - points[2].fY - points[0].fY);
}

static inline SkScalar compute_distance(const SkPoint points[]) {
    return SkPointPriv::DistanceToLineSegmentBetween(points[1], points[0], points[2]);
}

static inline uint32_t estimate_pointCount(int distance) {
    // Includes -2 bias because this estimator runs 4x high?
    int shift = 30 - SkCLZ(distance);
    // Clamp to zero if above subtraction went negative.
    shift &= ~(shift>>31);
    if (shift > MAX_COEFF_SHIFT) {
        shift = MAX_COEFF_SHIFT;
    }
    return 1 << shift;
}

static inline uint32_t compute_pointCount(SkScalar d, SkScalar tol) {
    if (d < tol) {
       return 1;
    } else {
       int temp = SkScalarCeilToInt(SkScalarSqrt(d / tol));
       uint32_t count = std::min<uint32_t>(SkNextPow2(temp), MAX_POINTS_PER_CURVE);
       return count;
    }
}

static uint32_t quadraticPointCount_EE(const SkPoint points[]) {
    int distance = estimate_distance(points);
    return estimate_pointCount(distance);
}

static uint32_t quadraticPointCount_EC(const SkPoint points[], SkScalar tol) {
    int distance = estimate_distance(points);
    return compute_pointCount(SkIntToScalar(distance), tol);
}

static uint32_t quadraticPointCount_CE(const SkPoint points[]) {
    SkScalar distance = compute_distance(points);
    return estimate_pointCount(SkScalarRoundToInt(distance));
}

static uint32_t quadraticPointCount_CC(const SkPoint points[], SkScalar tol) {
    SkScalar distance = compute_distance(points);
    return compute_pointCount(distance, tol);
}

// Curve from samplecode/SampleSlides.cpp
static const int gXY[] = {
    4, 0, 0, -4, 8, -4, 12, 0, 8, 4, 0, 4
};

static const int gSawtooth[] = {
    0, 0, 10, 10, 20, 20, 30, 10, 40, 0, 50, -10, 60, -20, 70, -10, 80, 0
};

static const int gOvalish[] = {
    0, 0, 5, 15, 20, 20, 35, 15, 40, 0
};

static const int gSharpSawtooth[] = {
    0, 0, 1, 10, 2, 0, 3, -10, 4, 0
};

// Curve crosses back over itself around 0,10
static const int gRibbon[] = {
   -4, 0, 4, 20, 0, 25, -4, 20, 4, 0
};

static bool one_d_pe(const int* array, const unsigned int count,
                     skiatest::Reporter* reporter) {
    SkPoint path [3];
    path[1] = SkPoint::Make(SkIntToScalar(array[0]), SkIntToScalar(array[1]));
    path[2] = SkPoint::Make(SkIntToScalar(array[2]), SkIntToScalar(array[3]));
    int numErrors = 0;
    for (unsigned i = 4; i < count; i += 2) {
        path[0] = path[1];
        path[1] = path[2];
        path[2] = SkPoint::Make(SkIntToScalar(array[i]),
                                SkIntToScalar(array[i+1]));
        uint32_t computedCount =
            quadraticPointCount_CC(path, SkIntToScalar(1));
        uint32_t estimatedCount =
            quadraticPointCount_EE(path);

        if ((false)) { // avoid bit rot, suppress warning
            computedCount = quadraticPointCount_EC(path, SkIntToScalar(1));
            estimatedCount = quadraticPointCount_CE(path);
        }
        // Allow estimated to be high by a factor of two, but no less than
        // the computed value.
        bool isAccurate = (estimatedCount >= computedCount) &&
            (estimatedCount <= 2 * computedCount);

        if (!isAccurate) {
            ERRORF(reporter, "Curve from %.2f %.2f through %.2f %.2f to "
                   "%.2f %.2f computes %u, estimates %u\n",
                   path[0].fX, path[0].fY, path[1].fX, path[1].fY,
                   path[2].fX, path[2].fY, computedCount, estimatedCount);
            numErrors++;
        }
    }

    return (numErrors == 0);
}



static void TestQuadPointCount(skiatest::Reporter* reporter) {
    one_d_pe(gXY, std::size(gXY), reporter);
    one_d_pe(gSawtooth, std::size(gSawtooth), reporter);
    one_d_pe(gOvalish, std::size(gOvalish), reporter);
    one_d_pe(gSharpSawtooth, std::size(gSharpSawtooth), reporter);
    one_d_pe(gRibbon, std::size(gRibbon), reporter);
}

DEF_TEST(PathCoverage, reporter) {
    TestQuadPointCount(reporter);

}
