/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGeometry.h"
#include "Test.h"
#include "SkRandom.h"
#include <array>

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
            SkEvalQuadAt(pts, t, nullptr, &v0);
            SkVector v1 = SkEvalQuadTangentAt(pts, t);
            check_pairs(reporter, i, t, "quad-tan", v0.fX, v0.fY, v1.fX, v1.fY);

            t += dt;
        }
    }
}

static void test_conic_eval_pos(skiatest::Reporter* reporter, const SkConic& conic, SkScalar t) {
    SkPoint p0, p1;
    conic.evalAt(t, &p0, nullptr);
    p1 = conic.evalAt(t);
    check_pairs(reporter, 0, t, "conic-pos", p0.fX, p0.fY, p1.fX, p1.fY);
}

static void test_conic_eval_tan(skiatest::Reporter* reporter, const SkConic& conic, SkScalar t) {
    SkVector v0, v1;
    conic.evalAt(t, nullptr, &v0);
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

static void test_quad_tangents(skiatest::Reporter* reporter) {
    SkPoint pts[] = {
        {10, 20}, {10, 20}, {20, 30},
        {10, 20}, {15, 25}, {20, 30},
        {10, 20}, {20, 30}, {20, 30},
    };
    int count = (int) SK_ARRAY_COUNT(pts) / 3;
    for (int index = 0; index < count; ++index) {
        SkConic conic(&pts[index * 3], 0.707f);
        SkVector start = SkEvalQuadTangentAt(&pts[index * 3], 0);
        SkVector mid = SkEvalQuadTangentAt(&pts[index * 3], .5f);
        SkVector end = SkEvalQuadTangentAt(&pts[index * 3], 1);
        REPORTER_ASSERT(reporter, start.fX && start.fY);
        REPORTER_ASSERT(reporter, mid.fX && mid.fY);
        REPORTER_ASSERT(reporter, end.fX && end.fY);
        REPORTER_ASSERT(reporter, SkScalarNearlyZero(start.cross(mid)));
        REPORTER_ASSERT(reporter, SkScalarNearlyZero(mid.cross(end)));
    }
}

static void test_conic_tangents(skiatest::Reporter* reporter) {
    SkPoint pts[] = {
        { 10, 20}, {10, 20}, {20, 30},
        { 10, 20}, {15, 25}, {20, 30},
        { 10, 20}, {20, 30}, {20, 30}
    };
    int count = (int) SK_ARRAY_COUNT(pts) / 3;
    for (int index = 0; index < count; ++index) {
        SkConic conic(&pts[index * 3], 0.707f);
        SkVector start = conic.evalTangentAt(0);
        SkVector mid = conic.evalTangentAt(.5f);
        SkVector end = conic.evalTangentAt(1);
        REPORTER_ASSERT(reporter, start.fX && start.fY);
        REPORTER_ASSERT(reporter, mid.fX && mid.fY);
        REPORTER_ASSERT(reporter, end.fX && end.fY);
        REPORTER_ASSERT(reporter, SkScalarNearlyZero(start.cross(mid)));
        REPORTER_ASSERT(reporter, SkScalarNearlyZero(mid.cross(end)));
    }
}

static void test_this_conic_to_quad(skiatest::Reporter* r, const SkPoint pts[3], SkScalar w) {
    SkAutoConicToQuads quadder;
    const SkPoint* qpts = quadder.computeQuads(pts, w, 0.25);
    const int qcount = quadder.countQuads();
    const int pcount = qcount * 2 + 1;

    REPORTER_ASSERT(r, SkPointsAreFinite(qpts, pcount));
}

/**
 *  We need to ensure that when a conic is approximated by quads, that we always return finite
 *  values in the quads.
 *
 *  Inspired by crbug_627414
 */
static void test_conic_to_quads(skiatest::Reporter* reporter) {
    const SkPoint triples[] = {
        { 0, 0 }, { 1, 0 }, { 1, 1 },
        { 0, 0 }, { 3.58732e-43f, 2.72084f }, { 3.00392f, 3.00392f },
        { 0, 0 }, { 100000, 0 }, { 100000, 100000 },
        { 0, 0 }, { 1e30f, 0 }, { 1e30f, 1e30f },
    };
    const int N = sizeof(triples) / sizeof(SkPoint);

    for (int i = 0; i < N; i += 3) {
        const SkPoint* pts = &triples[i];

        SkRect bounds;
        bounds.set(pts, 3);

        SkScalar w = 1e30f;
        do {
            w *= 2;
            test_this_conic_to_quad(reporter, pts, w);
        } while (SkScalarIsFinite(w));
        test_this_conic_to_quad(reporter, pts, SK_ScalarNaN);
    }
}

static void test_cubic_tangents(skiatest::Reporter* reporter) {
    SkPoint pts[] = {
        { 10, 20}, {10, 20}, {20, 30}, {30, 40},
        { 10, 20}, {15, 25}, {20, 30}, {30, 40},
        { 10, 20}, {20, 30}, {30, 40}, {30, 40},
    };
    int count = (int) SK_ARRAY_COUNT(pts) / 4;
    for (int index = 0; index < count; ++index) {
        SkConic conic(&pts[index * 3], 0.707f);
        SkVector start, mid, end;
        SkEvalCubicAt(&pts[index * 4], 0, nullptr, &start, nullptr);
        SkEvalCubicAt(&pts[index * 4], .5f, nullptr, &mid, nullptr);
        SkEvalCubicAt(&pts[index * 4], 1, nullptr, &end, nullptr);
        REPORTER_ASSERT(reporter, start.fX && start.fY);
        REPORTER_ASSERT(reporter, mid.fX && mid.fY);
        REPORTER_ASSERT(reporter, end.fX && end.fY);
        REPORTER_ASSERT(reporter, SkScalarNearlyZero(start.cross(mid)));
        REPORTER_ASSERT(reporter, SkScalarNearlyZero(mid.cross(end)));
    }
}

static void check_cubic_type(skiatest::Reporter* reporter,
                             const std::array<SkPoint, 4>& bezierPoints, SkCubicType expectedType) {
    SkCubicType actualType = SkClassifyCubic(bezierPoints.data());
    REPORTER_ASSERT(reporter, actualType == expectedType);
}

static void test_classify_cubic(skiatest::Reporter* reporter) {
    check_cubic_type(reporter, {{{149.325f, 107.705f}, {149.325f, 103.783f},
                                 {151.638f, 100.127f}, {156.263f, 96.736f}}},
                     SkCubicType::kQuadratic);
    check_cubic_type(reporter, {{{225.694f, 223.15f}, {209.831f, 224.837f},
                                 {195.994f, 230.237f}, {184.181f, 239.35f}}},
                     SkCubicType::kQuadratic);
    check_cubic_type(reporter, {{{4.873f, 5.581f}, {5.083f, 5.2783f},
                                 {5.182f, 4.8593f}, {5.177f, 4.3242f}}},
                     SkCubicType::kSerpentine);
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
    test_cubic_tangents(reporter);
    test_quad_tangents(reporter);
    test_conic_tangents(reporter);
    test_conic_to_quads(reporter);
    test_classify_cubic(reporter);
}
