/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGeometry.h"
#include "Test.h"
#include "SkRandom.h"

static bool nearly_equal(const SkPoint& a, const SkPoint& b) {
    return SkScalarNearlyEqual(a.fX, b.fX) && SkScalarNearlyEqual(a.fY, b.fY);
}

static void testChopCubic(skiatest::Reporter* reporter) {
    /*
        Inspired by this test, which used to assert that the tValues had dups

        <path stroke="#202020" d="M0,0 C0,0 1,1 2190,5130 C2190,5070 2220,5010 2205,4980" />
     */
    const SkPoint src[] = {
        { SkIntToScalar(2190), SkIntToScalar(5130) },
        { SkIntToScalar(2190), SkIntToScalar(5070) },
        { SkIntToScalar(2220), SkIntToScalar(5010) },
        { SkIntToScalar(2205), SkIntToScalar(4980) },
    };
    SkPoint dst[13];
    SkScalar tValues[3];
    // make sure we don't assert internally
    int count = SkChopCubicAtMaxCurvature(src, dst, tValues);
    if (false) { // avoid bit rot, suppress warning
        REPORTER_ASSERT(reporter, count);
    }
}

static void check_pairs(skiatest::Reporter* reporter, int index, SkScalar t, const char name[],
                        SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1) {
    bool eq = SkScalarNearlyEqual(x0, x1) && SkScalarNearlyEqual(y0, y1);
    if (!eq) {
        SkDebugf("%s [%d %g] p0 [%10.8f %10.8f] p1 [%10.8f %10.8f]\n",
                 name, index, t, x0, y0, x1, y1);
        REPORTER_ASSERT(reporter, eq);
    }
}

static void test_evalquadat(skiatest::Reporter* reporter) {
    SkRandom rand;
    for (int i = 0; i < 1000; ++i) {
        SkPoint pts[3];
        for (int j = 0; j < 3; ++j) {
            pts[j].set(rand.nextSScalar1() * 100, rand.nextSScalar1() * 100);
        }
        const SkScalar dt = SK_Scalar1 / 128;
        SkScalar t = dt;
        for (int j = 1; j < 128; ++j) {
            SkPoint r0;
            SkEvalQuadAt(pts, t, &r0);
            SkPoint r1 = SkEvalQuadAt(pts, t);
            check_pairs(reporter, i, t, "quad-pos", r0.fX, r0.fY, r1.fX, r1.fY);
            
            SkVector v0;
            SkEvalQuadAt(pts, t, NULL, &v0);
            SkVector v1 = SkEvalQuadTangentAt(pts, t);
            check_pairs(reporter, i, t, "quad-tan", v0.fX, v0.fY, v1.fX, v1.fY);

            t += dt;
        }
    }
}

static void test_conic_eval_pos(skiatest::Reporter* reporter, const SkConic& conic, SkScalar t) {
    SkPoint p0, p1;
    conic.evalAt(t, &p0, NULL);
    p1 = conic.evalAt(t);
    check_pairs(reporter, 0, t, "conic-pos", p0.fX, p0.fY, p1.fX, p1.fY);
}

static void test_conic_eval_tan(skiatest::Reporter* reporter, const SkConic& conic, SkScalar t) {
    SkVector v0, v1;
    conic.evalAt(t, NULL, &v0);
    v1 = conic.evalTangentAt(t);
    check_pairs(reporter, 0, t, "conic-tan", v0.fX, v0.fY, v1.fX, v1.fY);
}

static void test_conic(skiatest::Reporter* reporter) {
    SkRandom rand;
    for (int i = 0; i < 1000; ++i) {
        SkPoint pts[3];
        for (int j = 0; j < 3; ++j) {
            pts[j].set(rand.nextSScalar1() * 100, rand.nextSScalar1() * 100);
        }
        for (int k = 0; k < 10; ++k) {
            SkScalar w = rand.nextUScalar1() * 2;
            SkConic conic(pts, w);

            const SkScalar dt = SK_Scalar1 / 128;
            SkScalar t = dt;
            for (int j = 1; j < 128; ++j) {
                test_conic_eval_pos(reporter, conic, t);
                test_conic_eval_tan(reporter, conic, t);
                t += dt;
            }
        }
    }
}

DEF_TEST(Geometry, reporter) {
    SkPoint pts[3], dst[5];

    pts[0].set(0, 0);
    pts[1].set(100, 50);
    pts[2].set(0, 100);

    int count = SkChopQuadAtMaxCurvature(pts, dst);
    REPORTER_ASSERT(reporter, count == 1 || count == 2);

    pts[0].set(0, 0);
    pts[1].set(3, 0);
    pts[2].set(3, 3);
    SkConvertQuadToCubic(pts, dst);
    const SkPoint cubic[] = {
        { 0, 0, }, { 2, 0, }, { 3, 1, }, { 3, 3 },
    };
    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, nearly_equal(cubic[i], dst[i]));
    }

    testChopCubic(reporter);
    test_evalquadat(reporter);
    test_conic(reporter);
}
