/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkRandom.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPointPriv.h"
#include "tests/Test.h"

#include <array>
#include <numeric>

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
    // Make sure src and dst can be the same pointer.
    SkPoint pts[7];
    for (int i = 0; i < 7; ++i) {
        pts[i].set(i, i);
    }
    SkChopCubicAt(pts, pts, .5f);
    for (int i = 0; i < 7; ++i) {
        REPORTER_ASSERT(reporter, pts[i].fX == pts[i].fY);
        REPORTER_ASSERT(reporter, pts[i].fX == i * .5f);
    }

    static const float chopTs[] = {
        0, 3/83.f, 3/79.f, 3/73.f, 3/71.f, 3/67.f, 3/61.f, 3/59.f, 3/53.f, 3/47.f, 3/43.f, 3/41.f,
        3/37.f, 3/31.f, 3/29.f, 3/23.f, 3/19.f, 3/17.f, 3/13.f, 3/11.f, 3/7.f, 3/5.f, 1,
    };
    float ones[] = {1,1,1,1,1};

    // Ensure an odd number of T values so we exercise the single chop code at the end of
    // SkChopCubicAt form multiple T.
    static_assert(SK_ARRAY_COUNT(chopTs) % 2 == 1);
    static_assert(SK_ARRAY_COUNT(ones) % 2 == 1);

    SkRandom rand;
    for (int iterIdx = 0; iterIdx < 5; ++iterIdx) {
        SkPoint pts[4] = {{rand.nextF(), rand.nextF()}, {rand.nextF(), rand.nextF()},
                          {rand.nextF(), rand.nextF()}, {rand.nextF(), rand.nextF()}};

        SkPoint allChops[4 + SK_ARRAY_COUNT(chopTs)*3];
        SkChopCubicAt(pts, allChops, chopTs, SK_ARRAY_COUNT(chopTs));
        int i = 3;
        for (float chopT : chopTs) {
            // Ensure we chop at approximately the correct points when we chop an entire list.
            SkPoint expectedPt;
            SkEvalCubicAt(pts, chopT, &expectedPt, nullptr, nullptr);
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(allChops[i].x(), expectedPt.x()));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(allChops[i].y(), expectedPt.y()));
            if (chopT == 0) {
                REPORTER_ASSERT(reporter, allChops[i] == pts[0]);
            }
            if (chopT == 1) {
                REPORTER_ASSERT(reporter, allChops[i] == pts[3]);
            }
            i += 3;

            // Ensure the middle is exactly degenerate when we chop at two equal points.
            SkPoint localChops[10];
            SkChopCubicAt(pts, localChops, chopT, chopT);
            REPORTER_ASSERT(reporter, localChops[3] == localChops[4]);
            REPORTER_ASSERT(reporter, localChops[3] == localChops[5]);
            REPORTER_ASSERT(reporter, localChops[3] == localChops[6]);
            if (chopT == 0) {
                // Also ensure the first curve is exactly p0 when we chop at T=0.
                REPORTER_ASSERT(reporter, localChops[0] == pts[0]);
                REPORTER_ASSERT(reporter, localChops[1] == pts[0]);
                REPORTER_ASSERT(reporter, localChops[2] == pts[0]);
                REPORTER_ASSERT(reporter, localChops[3] == pts[0]);
            }
            if (chopT == 1) {
                // Also ensure the last curve is exactly p3 when we chop at T=1.
                REPORTER_ASSERT(reporter, localChops[6] == pts[3]);
                REPORTER_ASSERT(reporter, localChops[7] == pts[3]);
                REPORTER_ASSERT(reporter, localChops[8] == pts[3]);
                REPORTER_ASSERT(reporter, localChops[9] == pts[3]);
            }
        }

        // Now test what happens when SkChopCubicAt does 0/0 and gets NaN values.
        SkPoint oneChops[4 + SK_ARRAY_COUNT(ones)*3];
        SkChopCubicAt(pts, oneChops, ones, SK_ARRAY_COUNT(ones));
        REPORTER_ASSERT(reporter, oneChops[0] == pts[0]);
        REPORTER_ASSERT(reporter, oneChops[1] == pts[1]);
        REPORTER_ASSERT(reporter, oneChops[2] == pts[2]);
        for (size_t i = 3; i < SK_ARRAY_COUNT(oneChops); ++i) {
            REPORTER_ASSERT(reporter, oneChops[i] == pts[3]);
        }
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

    REPORTER_ASSERT(r, SkPointPriv::AreFinite(qpts, pcount));
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
                             const std::array<SkPoint, 4>& bezierPoints, SkCubicType expectedType,
                             bool undefined = false) {
    // Classify the cubic even if the results will be undefined: check for crashes and asserts.
    SkCubicType actualType = SkClassifyCubic(bezierPoints.data());
    if (!undefined) {
        REPORTER_ASSERT(reporter, actualType == expectedType);
    }
}

static void check_cubic_around_rect(skiatest::Reporter* reporter,
                                    float x1, float y1, float x2, float y2,
                                    bool undefined = false) {
    static constexpr SkCubicType expectations[24] = {
        SkCubicType::kLoop,
        SkCubicType::kCuspAtInfinity,
        SkCubicType::kLocalCusp,
        SkCubicType::kLocalCusp,
        SkCubicType::kCuspAtInfinity,
        SkCubicType::kLoop,
        SkCubicType::kCuspAtInfinity,
        SkCubicType::kLoop,
        SkCubicType::kCuspAtInfinity,
        SkCubicType::kLoop,
        SkCubicType::kLocalCusp,
        SkCubicType::kLocalCusp,
        SkCubicType::kLocalCusp,
        SkCubicType::kLocalCusp,
        SkCubicType::kLoop,
        SkCubicType::kCuspAtInfinity,
        SkCubicType::kLoop,
        SkCubicType::kCuspAtInfinity,
        SkCubicType::kLoop,
        SkCubicType::kCuspAtInfinity,
        SkCubicType::kLocalCusp,
        SkCubicType::kLocalCusp,
        SkCubicType::kCuspAtInfinity,
        SkCubicType::kLoop,
    };
    SkPoint points[] = {{x1, y1}, {x2, y1}, {x2, y2}, {x1, y2}};
    std::array<SkPoint, 4> bezier;
    for (int i=0; i < 4; ++i) {
        bezier[0] = points[i];
        for (int j=0; j < 3; ++j) {
            int jidx = (j < i) ? j : j+1;
            bezier[1] = points[jidx];
            for (int k=0, kidx=0; k < 2; ++k, ++kidx) {
                for (int n = 0; n < 2; ++n) {
                    kidx = (kidx == i || kidx == jidx) ? kidx+1 : kidx;
                }
                bezier[2] = points[kidx];
                for (int l = 0; l < 4; ++l) {
                    if (l != i && l != jidx && l != kidx) {
                        bezier[3] = points[l];
                        break;
                    }
                }
                check_cubic_type(reporter, bezier, expectations[i*6 + j*2 + k], undefined);
            }
        }
    }
    for (int i=0; i < 4; ++i) {
        bezier[0] = points[i];
        for (int j=0; j < 3; ++j) {
            int jidx = (j < i) ? j : j+1;
            bezier[1] = points[jidx];
            bezier[2] = points[jidx];
            for (int k=0, kidx=0; k < 2; ++k, ++kidx) {
                for (int n = 0; n < 2; ++n) {
                    kidx = (kidx == i || kidx == jidx) ? kidx+1 : kidx;
                }
                bezier[3] = points[kidx];
                check_cubic_type(reporter, bezier, SkCubicType::kSerpentine, undefined);
            }
        }
    }
}

static std::array<SkPoint, 4> kSerpentines[] = {
    {{{149.325f, 107.705f}, {149.325f, 103.783f}, {151.638f, 100.127f}, {156.263f, 96.736f}}},
    {{{225.694f, 223.15f}, {209.831f, 224.837f}, {195.994f, 230.237f}, {184.181f, 239.35f}}},
    {{{4.873f, 5.581f}, {5.083f, 5.2783f}, {5.182f, 4.8593f}, {5.177f, 4.3242f}}},
    {{{285.625f, 499.687f}, {411.625f, 808.188f}, {1064.62f, 135.688f}, {1042.63f, 585.187f}}}
};

static std::array<SkPoint, 4> kLoops[] = {
    {{{635.625f, 614.687f}, {171.625f, 236.188f}, {1064.62f, 135.688f}, {516.625f, 570.187f}}},
    {{{653.050f, 725.049f}, {663.000f, 176.000f}, {1189.000f, 508.000f}, {288.050f, 564.950f}}},
    {{{631.050f, 478.049f}, {730.000f, 302.000f}, {870.000f, 350.000f}, {905.050f, 528.950f}}},
    {{{631.050f, 478.0499f}, {221.000f, 230.000f}, {1265.000f, 451.000f}, {905.050f, 528.950f}}}
};

static std::array<SkPoint, 4> kLinearCubics[] = {
    {{{0, 0}, {0, 1}, {0, 2}, {0, 3}}},  // 0-degree flat line.
    {{{0, 0}, {1, 0}, {1, 0}, {0, 0}}},  // 180-degree flat line
    {{{0, 1}, {0, 0}, {0, 2}, {0, 3}}},  // 180-degree flat line
    {{{0, 1}, {0, 0}, {0, 3}, {0, 2}}},  // 360-degree flat line
    {{{0, 0}, {2, 0}, {1, 0}, {64, 0}}},  // 360-degree flat line
    {{{1, 0}, {0, 0}, {3, 0}, {-64, 0}}}  // 360-degree flat line
};

static void test_classify_cubic(skiatest::Reporter* reporter) {
    for (const auto& serp : kSerpentines) {
        check_cubic_type(reporter, serp, SkCubicType::kSerpentine);
    }
    for (const auto& loop : kLoops) {
        check_cubic_type(reporter, loop, SkCubicType::kLoop);
    }
    for (const auto& loop : kLinearCubics) {
        check_cubic_type(reporter, loop, SkCubicType::kLineOrPoint);
    }
    check_cubic_around_rect(reporter, 0, 0, 1, 1);
    check_cubic_around_rect(reporter,
                            -std::numeric_limits<float>::max(),
                            -std::numeric_limits<float>::max(),
                            +std::numeric_limits<float>::max(),
                            +std::numeric_limits<float>::max());
    check_cubic_around_rect(reporter, 1, 1,
                            +std::numeric_limits<float>::min(),
                            +std::numeric_limits<float>::max());
    check_cubic_around_rect(reporter,
                            -std::numeric_limits<float>::min(),
                            -std::numeric_limits<float>::min(),
                            +std::numeric_limits<float>::min(),
                            +std::numeric_limits<float>::min());
    check_cubic_around_rect(reporter, +1, -std::numeric_limits<float>::min(), -1, -1);
    check_cubic_around_rect(reporter,
                            -std::numeric_limits<float>::infinity(),
                            -std::numeric_limits<float>::infinity(),
                            +std::numeric_limits<float>::infinity(),
                            +std::numeric_limits<float>::infinity(),
                            true);
    check_cubic_around_rect(reporter, 0, 0, 1, +std::numeric_limits<float>::infinity(), true);
    check_cubic_around_rect(reporter,
                            -std::numeric_limits<float>::quiet_NaN(),
                            -std::numeric_limits<float>::quiet_NaN(),
                            +std::numeric_limits<float>::quiet_NaN(),
                            +std::numeric_limits<float>::quiet_NaN(),
                            true);
    check_cubic_around_rect(reporter, 0, 0, 1, +std::numeric_limits<float>::quiet_NaN(), true);
}

static std::array<SkPoint, 4> kCusps[] = {
    {{{0, 0}, {1, 1}, {1, 0}, {0, 1}}},
    {{{0, 0}, {1, 1}, {0, 1}, {1, 0}}},
    {{{0, 1}, {1, 0}, {0, 0}, {1, 1}}},
    {{{0, 1}, {1, 0}, {1, 1}, {0, 0}}},
};

static void test_cubic_cusps(skiatest::Reporter* reporter) {
    std::array<SkPoint, 4> noCusps[] = {
        {{{0, 0}, {1, 1}, {2, 2}, {3, 3}}},
        {{{0, 0}, {1, 0}, {1, 1}, {0, 1}}},
        {{{0, 0}, {1, 0}, {2, 1}, {2, 2}}},
        {{{0, 0}, {1, 0}, {1, 1}, {2, 1}}},
    };
    for (auto noCusp : noCusps) {
        REPORTER_ASSERT(reporter, SkFindCubicCusp(noCusp.data()) < 0);
    }
    for (auto cusp : kCusps) {
        REPORTER_ASSERT(reporter, SkFindCubicCusp(cusp.data()) > 0);
    }
}

static SkMatrix kSkewMatrices[] = {
    SkMatrix::MakeAll(1,0,0, 0,1,0, 0,0,1),
    SkMatrix::MakeAll(1,-1,0, 1,1,0, 0,0,1),
    SkMatrix::MakeAll(.889f,.553f,0, -.443f,.123f,0, 0,0,1),
};

static void test_chop_quad_at_midtangent(skiatest::Reporter* reporter, const SkPoint pts[3]) {
    constexpr float kTolerance = 1e-3f;
    for (const SkMatrix& m : kSkewMatrices) {
        SkPoint mapped[3];
        m.mapPoints(mapped, pts, 3);
        float fullRotation = SkMeasureQuadRotation(pts);
        SkPoint chopped[5];
        SkChopQuadAtMidTangent(pts, chopped);
        float leftRotation = SkMeasureQuadRotation(chopped);
        float rightRotation = SkMeasureQuadRotation(chopped+2);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(leftRotation, fullRotation/2, kTolerance));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(rightRotation, fullRotation/2, kTolerance));
    }
}

static void test_chop_cubic_at_midtangent(skiatest::Reporter* reporter, const SkPoint pts[4],
                                          SkCubicType cubicType) {
    constexpr float kTolerance = 1e-3f;
    int n = SK_ARRAY_COUNT(kSkewMatrices);
    if (cubicType == SkCubicType::kLocalCusp || cubicType == SkCubicType::kLineOrPoint) {
        // FP precision isn't always enough to get the exact correct T value of the mid-tangent on
        // cusps and lines. Only test the identity matrix and the matrix with all 1's.
        n = 2;
    }
    for (int i = 0; i < n; ++i) {
        SkPoint mapped[4];
        kSkewMatrices[i].mapPoints(mapped, pts, 4);
        float fullRotation = SkMeasureNonInflectCubicRotation(mapped);
        SkPoint chopped[7];
        SkChopCubicAtMidTangent(mapped, chopped);
        float leftRotation = SkMeasureNonInflectCubicRotation(chopped);
        float rightRotation = SkMeasureNonInflectCubicRotation(chopped+3);
        if (cubicType == SkCubicType::kLineOrPoint &&
            (SkScalarNearlyEqual(fullRotation, 2*SK_ScalarPI, kTolerance) ||
             SkScalarNearlyEqual(fullRotation, 0, kTolerance))) {
            // 0- and 360-degree flat lines don't have single points of midtangent.
            // (tangent == midtangent at every point on these curves except the cusp points.)
            // Instead verify the promise from SkChopCubicAtMidTangent that neither side will rotate
            // more than 180 degrees.
            REPORTER_ASSERT(reporter, std::abs(leftRotation) - kTolerance <= SK_ScalarPI);
            REPORTER_ASSERT(reporter, std::abs(rightRotation) - kTolerance <= SK_ScalarPI);
            continue;
        }
        float expectedChoppedRotation = fullRotation/2;
        if (cubicType == SkCubicType::kLocalCusp ||
            (cubicType == SkCubicType::kLineOrPoint &&
             SkScalarNearlyEqual(fullRotation, SK_ScalarPI, kTolerance))) {
            // If we chop a cubic at a cusp, we lose 180 degrees of rotation.
            expectedChoppedRotation = (fullRotation - SK_ScalarPI)/2;
        }
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(leftRotation, expectedChoppedRotation,
                                                      kTolerance));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(rightRotation, expectedChoppedRotation,
                                                      kTolerance));
    }
}

static std::array<SkPoint, 3> kQuads[] = {
    {{{10, 20}, {15, 35}, {30, 40}}},
    {{{176.324f, 392.705f}, {719.325f, 205.782f}, {297.263f, 347.735f}}},
    {{{652.050f, 602.049f}, {481.000f, 533.000f}, {288.050f, 564.950f}}},
    {{{460.625f, 557.187f}, {707.121f, 209.688f}, {779.628f, 577.687f}}},
    {{{359.050f, 578.049f}, {759.000f, 274.000f}, {288.050f, 564.950f}}}
};

SkPoint lerp(const SkPoint& a, const SkPoint& b, float t) {
    return a * (1 - t) + b * t;
}

static void test_measure_rotation(skiatest::Reporter* reporter) {
    static SkPoint kFlatCubic[4] = {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(SkMeasureNonInflectCubicRotation(kFlatCubic)));

    static SkPoint kFlatCubic180_1[4] = {{0, 0}, {1, 0}, {3, 0}, {2, 0}};
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SkMeasureNonInflectCubicRotation(kFlatCubic180_1),
                                                  SK_ScalarPI));

    static SkPoint kFlatCubic180_2[4] = {{0, 1}, {0, 0}, {0, 2}, {0, 3}};
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SkMeasureNonInflectCubicRotation(kFlatCubic180_2),
                                                  SK_ScalarPI));

    static SkPoint kFlatCubic360[4] = {{0, 1}, {0, 0}, {0, 3}, {0, 2}};
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SkMeasureNonInflectCubicRotation(kFlatCubic360),
                                                  2*SK_ScalarPI));

    static SkPoint kSquare180[4] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SkMeasureNonInflectCubicRotation(kSquare180),
                                                  SK_ScalarPI));

    auto checkQuadRotation = [=](const SkPoint pts[3], float expectedRotation) {
        float r = SkMeasureQuadRotation(pts);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(r, expectedRotation));

        SkPoint cubic1[4] = {pts[0], pts[0], pts[1], pts[2]};
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SkMeasureNonInflectCubicRotation(cubic1),
                                                      expectedRotation));

        SkPoint cubic2[4] = {pts[0], pts[1], pts[1], pts[2]};
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SkMeasureNonInflectCubicRotation(cubic2),
                                                      expectedRotation));

        SkPoint cubic3[4] = {pts[0], pts[1], pts[2], pts[2]};
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SkMeasureNonInflectCubicRotation(cubic3),
                                                      expectedRotation));
    };

    static SkPoint kFlatQuad[4] = {{0, 0}, {0, 1}, {0, 2}};
    checkQuadRotation(kFlatQuad, 0);

    static SkPoint kFlatQuad180_1[4] = {{1, 0}, {0, 0}, {2, 0}};
    checkQuadRotation(kFlatQuad180_1, SK_ScalarPI);

    static SkPoint kFlatQuad180_2[4] = {{0, 0}, {0, 2}, {0, 1}};
    checkQuadRotation(kFlatQuad180_2, SK_ScalarPI);

    static SkPoint kTri120[3] = {{0, 0}, {.5f, std::sqrt(3.f)/2}, {1, 0}};
    checkQuadRotation(kTri120, 2*SK_ScalarPI/3);
}

static void test_chop_at_midtangent(skiatest::Reporter* reporter) {
    SkPoint chops[10];
    for (const auto& serp : kSerpentines) {
        REPORTER_ASSERT(reporter, SkClassifyCubic(serp.data()) == SkCubicType::kSerpentine);
        int n = SkChopCubicAtInflections(serp.data(), chops);
        for (int i = 0; i < n; ++i) {
            test_chop_cubic_at_midtangent(reporter, chops + i*3, SkCubicType::kSerpentine);
        }
    }
    for (const auto& loop : kLoops) {
        REPORTER_ASSERT(reporter, SkClassifyCubic(loop.data()) == SkCubicType::kLoop);
        test_chop_cubic_at_midtangent(reporter, loop.data(), SkCubicType::kLoop);
    }
    for (const auto& line : kLinearCubics) {
        REPORTER_ASSERT(reporter, SkClassifyCubic(line.data()) == SkCubicType::kLineOrPoint);
        test_chop_cubic_at_midtangent(reporter, line.data(), SkCubicType::kLineOrPoint);
    }
    for (const auto& cusp : kCusps) {
        REPORTER_ASSERT(reporter, SkClassifyCubic(cusp.data()) == SkCubicType::kLocalCusp);
        test_chop_cubic_at_midtangent(reporter, cusp.data(), SkCubicType::kLocalCusp);
    }
    for (const auto& quad : kQuads) {
        test_chop_quad_at_midtangent(reporter, quad.data());
        SkPoint asCubic[4] = {
                quad[0], lerp(quad[0], quad[1], 2/3.f), lerp(quad[1], quad[2], 1/3.f), quad[2]};
        test_chop_cubic_at_midtangent(reporter, asCubic, SkCubicType::kQuadratic);
    }

    static const SkPoint kExactQuad[4] = {{0,0}, {6,2}, {10,2}, {12,0}};
    REPORTER_ASSERT(reporter, SkClassifyCubic(kExactQuad) == SkCubicType::kQuadratic);
    test_chop_cubic_at_midtangent(reporter, kExactQuad, SkCubicType::kQuadratic);

    static const SkPoint kExactCuspAtInf[4] = {{0,0}, {1,0}, {0,1}, {1,1}};
    REPORTER_ASSERT(reporter, SkClassifyCubic(kExactCuspAtInf) == SkCubicType::kCuspAtInfinity);
    int n = SkChopCubicAtInflections(kExactCuspAtInf, chops);
    for (int i = 0; i < n; ++i) {
        test_chop_cubic_at_midtangent(reporter, chops + i*3, SkCubicType::kCuspAtInfinity);
    }
}

DEF_TEST(Geometry, reporter) {
    SkPoint pts[5];

    pts[0].set(0, 0);
    pts[1].set(100, 50);
    pts[2].set(0, 100);

    int count = SkChopQuadAtMaxCurvature(pts, pts);  // Ensure src and dst can be the same pointer.
    REPORTER_ASSERT(reporter, count == 1 || count == 2);

    pts[0].set(0, 0);
    pts[1].set(3, 0);
    pts[2].set(3, 3);
    SkConvertQuadToCubic(pts, pts);
    const SkPoint cubic[] = {
        { 0, 0, }, { 2, 0, }, { 3, 1, }, { 3, 3 },
    };
    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, nearly_equal(cubic[i], pts[i]));
    }

    testChopCubic(reporter);
    test_evalquadat(reporter);
    test_conic(reporter);
    test_cubic_tangents(reporter);
    test_quad_tangents(reporter);
    test_conic_tangents(reporter);
    test_conic_to_quads(reporter);
    test_classify_cubic(reporter);
    test_cubic_cusps(reporter);
    test_measure_rotation(reporter);
    test_chop_at_midtangent(reporter);
}
