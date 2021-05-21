/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkRandom.h"
#include "src/core/SkGeometry.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "tests/Test.h"

static bool is_linear(SkPoint p0, SkPoint p1, SkPoint p2) {
    return SkScalarNearlyZero((p0 - p1).cross(p2 - p1));
}

static bool is_linear(const SkPoint p[4]) {
    return is_linear(p[0],p[1],p[2]) && is_linear(p[0],p[2],p[3]) && is_linear(p[1],p[2],p[3]);
}

static void check_cubic_convex_180(skiatest::Reporter* r, const SkPoint p[4]) {
    bool areCusps = false;
    float inflectT[2], convex180T[2];
    if (int inflectN = SkFindCubicInflections(p, inflectT)) {
        // The curve has inflections. findCubicConvex180Chops should return the inflection
        // points.
        int convex180N = GrPathUtils::findCubicConvex180Chops(p, convex180T, &areCusps);
        REPORTER_ASSERT(r, inflectN == convex180N);
        if (!areCusps) {
            REPORTER_ASSERT(r, inflectN == 1 ||
                            fabsf(inflectT[0] - inflectT[1]) >= SK_ScalarNearlyZero);
        }
        for (int i = 0; i < convex180N; ++i) {
            REPORTER_ASSERT(r, SkScalarNearlyEqual(inflectT[i], convex180T[i]));
        }
    } else {
        float totalRotation = SkMeasureNonInflectCubicRotation(p);
        int convex180N = GrPathUtils::findCubicConvex180Chops(p, convex180T, &areCusps);
        SkPoint chops[10];
        SkChopCubicAt(p, chops, convex180T, convex180N);
        float radsSum = 0;
        for (int i = 0; i <= convex180N; ++i) {
            float rads = SkMeasureNonInflectCubicRotation(chops + i*3);
            SkASSERT(rads < SK_ScalarPI + SK_ScalarNearlyZero);
            radsSum += rads;
        }
        if (totalRotation < SK_ScalarPI - SK_ScalarNearlyZero) {
            // The curve should never chop if rotation is <180 degrees.
            REPORTER_ASSERT(r, convex180N == 0);
        } else if (!is_linear(p)) {
            REPORTER_ASSERT(r, SkScalarNearlyEqual(radsSum, totalRotation));
            if (totalRotation > SK_ScalarPI + SK_ScalarNearlyZero) {
                REPORTER_ASSERT(r, convex180N == 1);
                // This works because cusps take the "inflection" path above, so we don't get
                // non-lilnear curves that lose rotation when chopped.
                REPORTER_ASSERT(r, SkScalarNearlyEqual(
                    SkMeasureNonInflectCubicRotation(chops), SK_ScalarPI));
                REPORTER_ASSERT(r, SkScalarNearlyEqual(
                    SkMeasureNonInflectCubicRotation(chops + 3), totalRotation - SK_ScalarPI));
            }
            REPORTER_ASSERT(r, !areCusps);
        } else {
            REPORTER_ASSERT(r, areCusps);
        }
    }
}

DEF_TEST(GrPathUtils_findCubicConvex180Chops, r) {
    // Test all combinations of corners from the square [0,0,1,1]. This covers every cubic type as
    // well as a wide variety of special cases for cusps, lines, loops, and inflections.
    for (int i = 0; i < (1 << 8); ++i) {
        SkPoint p[4] = {SkPoint::Make((i>>0)&1, (i>>1)&1),
                        SkPoint::Make((i>>2)&1, (i>>3)&1),
                        SkPoint::Make((i>>4)&1, (i>>5)&1),
                        SkPoint::Make((i>>6)&1, (i>>7)&1)};
        check_cubic_convex_180(r, p);
    }

    {
        // This cubic has a convex-180 chop at T=1-"epsilon"
        static const uint32_t hexPts[] = {0x3ee0ac74, 0x3f1e061a, 0x3e0fc408, 0x3f457230,
                                          0x3f42ac7c, 0x3f70d76c, 0x3f4e6520, 0x3f6acafa};
        SkPoint p[4];
        memcpy(p, hexPts, sizeof(p));
        check_cubic_convex_180(r, p);
    }

    // Now test an exact quadratic.
    SkPoint quad[4] = {{0,0}, {2,2}, {4,2}, {6,0}};
    float T[2];
    bool areCusps;
    REPORTER_ASSERT(r, GrPathUtils::findCubicConvex180Chops(quad, T, &areCusps) == 0);

    // Now test that cusps and near-cusps get flagged as cusps.
    SkPoint cusp[4] = {{0,0}, {1,1}, {1,0}, {0,1}};
    REPORTER_ASSERT(r, GrPathUtils::findCubicConvex180Chops(cusp, T, &areCusps) == 1);
    REPORTER_ASSERT(r, areCusps == true);

    // Find the height of the right side of "cusp" at which the distance between its inflection
    // points is kEpsilon (in parametric space).
    constexpr static double kEpsilon = 1.0 / (1 << 11);
    constexpr static double kEpsilonSquared = kEpsilon * kEpsilon;
    double h = (1 - kEpsilonSquared) / (3 * kEpsilonSquared + 1);
    double dy = (1 - h) / 2;
    cusp[1].fY = (float)(1 - dy);
    cusp[2].fY = (float)(0 + dy);
    REPORTER_ASSERT(r, SkFindCubicInflections(cusp, T) == 2);
    REPORTER_ASSERT(r, SkScalarNearlyEqual(T[1] - T[0], (float)kEpsilon, (float)kEpsilonSquared));

    // Ensure two inflection points barely more than kEpsilon apart do not get flagged as cusps.
    cusp[1].fY = (float)(1 - 1.1 * dy);
    cusp[2].fY = (float)(0 + 1.1 * dy);
    REPORTER_ASSERT(r, GrPathUtils::findCubicConvex180Chops(cusp, T, &areCusps) == 2);
    REPORTER_ASSERT(r, areCusps == false);

    // Ensure two inflection points barely less than kEpsilon apart do get flagged as cusps.
    cusp[1].fY = (float)(1 - .9 * dy);
    cusp[2].fY = (float)(0 + .9 * dy);
    REPORTER_ASSERT(r, GrPathUtils::findCubicConvex180Chops(cusp, T, &areCusps) == 1);
    REPORTER_ASSERT(r, areCusps == true);
}

DEF_TEST(GrPathUtils_convertToCubic, r) {
    SkPoint cubic[4];
    GrVertexWriter cubicWriter(cubic);
    GrPathUtils::writeLineAsCubic({0,0}, {3,6}, &cubicWriter);
    REPORTER_ASSERT(r, cubic[0] == SkPoint::Make(0,0));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(cubic[1].fX, 1));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(cubic[1].fY, 2));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(cubic[2].fX, 2));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(cubic[2].fY, 4));
    REPORTER_ASSERT(r, cubic[3] == SkPoint::Make(3,6));

    SkPoint quad[3] = {{0,0}, {3,3}, {6,0}};
    GrPathUtils::convertQuadToCubic(quad, cubic);
    REPORTER_ASSERT(r, cubic[0] == SkPoint::Make(0,0));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(cubic[1].fX, 2));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(cubic[1].fY, 2));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(cubic[2].fX, 4));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(cubic[2].fY, 2));
    REPORTER_ASSERT(r, cubic[3] == SkPoint::Make(6,0));
}
