/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkRandom.h"
#include "src/core/SkGeometry.h"
#include "src/gpu/tessellate/GrWangsFormula.h"
#include "tests/Test.h"

constexpr static int kIntolerance = 4;  // 1/4 pixel max error.

const SkPoint kSerp[4] = {
        {285.625f, 499.687f}, {411.625f, 808.188f}, {1064.62f, 135.688f}, {1042.63f, 585.187f}};

const SkPoint kLoop[4] = {
        {635.625f, 614.687f}, {171.625f, 236.188f}, {1064.62f, 135.688f}, {516.625f, 570.187f}};

const SkPoint kQuad[4] = {
        {460.625f, 557.187f}, {707.121f, 209.688f}, {779.628f, 577.687f}};

static float wangs_formula_quadratic_reference_impl(float intolerance, const SkPoint p[3]) {
    float k = (2 * 1) / 8.f * intolerance;
    return sqrtf(k * (p[0] - p[1]*2 + p[2]).length());
}

static float wangs_formula_cubic_reference_impl(float intolerance, const SkPoint p[4]) {
    float k = (3 * 2) / 8.f * intolerance;
    return sqrtf(k * std::max((p[0] - p[1]*2 + p[2]).length(),
                              (p[1] - p[2]*2 + p[3]).length()));
}

// Returns number of segments for linearized cubic rational. This is an analogue
// to Wang's formula, taken from:
//   J. Zheng, T. Sederberg. "Estimating Tessellation Parameter Intervals for
//   Rational Curves and Surfaces." ACM Transactions on Graphics 19(1). 2000.
// See Thm 3, Corollary 1.
static float wangs_formula_rational_cubic_reference_impl(float intolerance,
                                                         const SkPoint P[4],
                                                         const float wts[4]) {
    using float4x2 = std::array<std::array<float, 2>, 4>;
    using vec2 = std::array<float, 2>;

    // Some helper functions for readability
    const auto bb_center = [](const float4x2& P) {
        float min_x = std::numeric_limits<float>::max(),
              max_x = std::numeric_limits<float>::lowest(),
              min_y = std::numeric_limits<float>::max(),
              max_y = std::numeric_limits<float>::lowest();
        for (auto [x, y] : P) {
            min_x = std::min(min_x, x);
            max_x = std::max(max_x, x);
            min_y = std::min(min_y, y);
            max_y = std::max(max_y, y);
        }
        return SkPoint::Make(0.5f * (min_x + max_x), 0.5f * (min_y + max_y));
    };

    const auto length = [](const vec2& p) {
        const SkPoint pt{p[0], p[1]};
        return pt.length();
    };

    const auto max_len = [&length](const float4x2& P) {
        float max = std::numeric_limits<float>::lowest();
        for (size_t j = 0; j < P.size(); j++) {
            max = std::max(max, length(P[j]));
        }
        return max;
    };

    const auto fwdiff2 = [](const vec2& p0, const vec2& p1, const vec2& p2) {
        return vec2{p2[0] - 2 * p1[0] + p0[0], p2[1] - 2 * p1[1] + p0[1]};
    };

    const auto fwdiff2_scalar = [](float p0, float p1, float p2) { return p2 - 2 * p1 + p0; };

    // Get center point of bounding box in projected space
    SkASSERT(wts[0] == 1 && wts[1] == wts[2] && wts[3] == 1 && wts[1] > 0);
    const float w = wts[1];
    const float4x2 projP = {{{P[0].fX, P[0].fY},
                             {P[1].fX / w, P[1].fY / w},
                             {P[2].fX / w, P[2].fY / w},
                             {P[3].fX, P[3].fY}}};
    const SkPoint C = bb_center(projP);

    // Translate by the center point and re-project
    const float4x2 transP = {{{projP[0][0] - C.fX, projP[0][1] - C.fY},
                              {projP[1][0] - C.fX, projP[1][1] - C.fY},
                              {projP[2][0] - C.fX, projP[2][1] - C.fY},
                              {projP[3][0] - C.fX, projP[3][1] - C.fY}}};
    const float4x2 wTransP = {{{transP[0][0], transP[0][1]},
                               {transP[1][0] * w, transP[1][1] * w},
                               {transP[2][0] * w, transP[2][1] * w},
                               {transP[3][0], transP[3][1]}}};

    // Compute forward differences
    const vec2 av = fwdiff2(wTransP[0], wTransP[1], wTransP[2]),
               bv = fwdiff2(wTransP[1], wTransP[2], wTransP[3]);
    const float aw = fwdiff2_scalar(wts[0], wts[1], wts[2]),
                bw = fwdiff2_scalar(wts[1], wts[2], wts[3]);

    // Compute delta = parametric step size of linearization
    const float eps = 1.f / intolerance;
    const float min_w = std::min(w, 1.f);
    const float r = max_len(transP);
    const float k = std::max(r - eps, 0.f);
    const float a = length(av) + k * std::abs(aw), b = length(bv) + k * std::abs(bw);
    const float delta = sqrtf((4 * min_w * eps) / (3 * std::max(a, b)));

    constexpr float tmin = 0, tmax = 1;
    return (tmax - tmin) / delta;
}

static void for_random_matrices(SkRandom* rand, std::function<void(const SkMatrix&)> f) {
    SkMatrix m;
    m.setIdentity();
    f(m);

    for (int i = -10; i <= 30; ++i) {
        for (int j = -10; j <= 30; ++j) {
            m.setScaleX(std::ldexp(1 + rand->nextF(), i));
            m.setSkewX(0);
            m.setSkewY(0);
            m.setScaleY(std::ldexp(1 + rand->nextF(), j));
            f(m);

            m.setScaleX(std::ldexp(1 + rand->nextF(), i));
            m.setSkewX(std::ldexp(1 + rand->nextF(), (j + i) / 2));
            m.setSkewY(std::ldexp(1 + rand->nextF(), (j + i) / 2));
            m.setScaleY(std::ldexp(1 + rand->nextF(), j));
            f(m);
        }
    }
}

static void for_random_beziers(int numPoints, SkRandom* rand,
                               std::function<void(const SkPoint[])> f,
                               int maxExponent = 30) {
    SkASSERT(numPoints <= 4);
    SkPoint pts[4];
    for (int i = -10; i <= maxExponent; ++i) {
        for (int j = 0; j < numPoints; ++j) {
            pts[j].set(std::ldexp(1 + rand->nextF(), i), std::ldexp(1 + rand->nextF(), i));
        }
        f(pts);
    }
}

// Ensure the optimized "*_log2" versions return the same value as ceil(std::log2(f)).
DEF_TEST(WangsFormula_log2, r) {
    // Constructs a cubic such that the 'length' term in wang's formula == term.
    //
    //     f = sqrt(k * length(max(abs(p0 - p1*2 + p2),
    //                             abs(p1 - p2*2 + p3))));
    auto setupCubicLengthTerm = [](int seed, SkPoint pts[], float term) {
        memset(pts, 0, sizeof(SkPoint) * 4);

        SkPoint term2d = (seed & 1) ?
                SkPoint::Make(term, 0) : SkPoint::Make(.5f, std::sqrt(3)/2) * term;
        seed >>= 1;

        if (seed & 1) {
            term2d.fX = -term2d.fX;
        }
        seed >>= 1;

        if (seed & 1) {
            std::swap(term2d.fX, term2d.fY);
        }
        seed >>= 1;

        switch (seed % 4) {
            case 0:
                pts[0] = term2d;
                pts[3] = term2d * .75f;
                return;
            case 1:
                pts[1] = term2d * -.5f;
                return;
            case 2:
                pts[1] = term2d * -.5f;
                return;
            case 3:
                pts[3] = term2d;
                pts[0] = term2d * .75f;
                return;
        }
    };

    // Constructs a quadratic such that the 'length' term in wang's formula == term.
    //
    //     f = sqrt(k * length(p0 - p1*2 + p2));
    auto setupQuadraticLengthTerm = [](int seed, SkPoint pts[], float term) {
        memset(pts, 0, sizeof(SkPoint) * 3);

        SkPoint term2d = (seed & 1) ?
                SkPoint::Make(term, 0) : SkPoint::Make(.5f, std::sqrt(3)/2) * term;
        seed >>= 1;

        if (seed & 1) {
            term2d.fX = -term2d.fX;
        }
        seed >>= 1;

        if (seed & 1) {
            std::swap(term2d.fX, term2d.fY);
        }
        seed >>= 1;

        switch (seed % 3) {
            case 0:
                pts[0] = term2d;
                return;
            case 1:
                pts[1] = term2d * -.5f;
                return;
            case 2:
                pts[2] = term2d;
                return;
        }
    };

    // GrWangsFormula::cubic and ::quadratic both use rsqrt instead of sqrt for speed. Linearization
    // is all approximate anyway, so as long as we are within ~1/2 tessellation segment of the
    // reference value we are good enough.
    constexpr static float kTessellationTolerance = 1/128.f;

    for (int level = 0; level < 30; ++level) {
        float epsilon = std::ldexp(SK_ScalarNearlyZero, level * 2);
        SkPoint pts[4];

        {
            // Test cubic boundaries.
            //     f = sqrt(k * length(max(abs(p0 - p1*2 + p2),
            //                             abs(p1 - p2*2 + p3))));
            constexpr static float k = (3 * 2) / (8 * (1.f/kIntolerance));
            float x = std::ldexp(1, level * 2) / k;
            setupCubicLengthTerm(level << 1, pts, x - epsilon);
            float referenceValue = wangs_formula_cubic_reference_impl(kIntolerance, pts);
            REPORTER_ASSERT(r, std::ceil(std::log2(referenceValue)) == level);
            float c = GrWangsFormula::cubic(kIntolerance, pts);
            REPORTER_ASSERT(r, SkScalarNearlyEqual(c/referenceValue, 1, kTessellationTolerance));
            REPORTER_ASSERT(r, GrWangsFormula::cubic_log2(kIntolerance, pts) == level);
            setupCubicLengthTerm(level << 1, pts, x + epsilon);
            referenceValue = wangs_formula_cubic_reference_impl(kIntolerance, pts);
            REPORTER_ASSERT(r, std::ceil(std::log2(referenceValue)) == level + 1);
            c = GrWangsFormula::cubic(kIntolerance, pts);
            REPORTER_ASSERT(r, SkScalarNearlyEqual(c/referenceValue, 1, kTessellationTolerance));
            REPORTER_ASSERT(r, GrWangsFormula::cubic_log2(kIntolerance, pts) == level + 1);
        }

        {
            // Test quadratic boundaries.
            //     f = std::sqrt(k * Length(p0 - p1*2 + p2));
            constexpr static float k = 2 / (8 * (1.f/kIntolerance));
            float x = std::ldexp(1, level * 2) / k;
            setupQuadraticLengthTerm(level << 1, pts, x - epsilon);
            float referenceValue = wangs_formula_quadratic_reference_impl(kIntolerance, pts);
            REPORTER_ASSERT(r, std::ceil(std::log2(referenceValue)) == level);
            float q = GrWangsFormula::quadratic(kIntolerance, pts);
            REPORTER_ASSERT(r, SkScalarNearlyEqual(q/referenceValue, 1, kTessellationTolerance));
            REPORTER_ASSERT(r, GrWangsFormula::quadratic_log2(kIntolerance, pts) == level);
            setupQuadraticLengthTerm(level << 1, pts, x + epsilon);
            referenceValue = wangs_formula_quadratic_reference_impl(kIntolerance, pts);
            REPORTER_ASSERT(r, std::ceil(std::log2(referenceValue)) == level+1);
            q = GrWangsFormula::quadratic(kIntolerance, pts);
            REPORTER_ASSERT(r, SkScalarNearlyEqual(q/referenceValue, 1, kTessellationTolerance));
            REPORTER_ASSERT(r, GrWangsFormula::quadratic_log2(kIntolerance, pts) == level + 1);
        }
    }

    auto check_cubic_log2 = [&](const SkPoint* pts) {
        float f = std::max(1.f, wangs_formula_cubic_reference_impl(kIntolerance, pts));
        int f_log2 = GrWangsFormula::cubic_log2(kIntolerance, pts);
        REPORTER_ASSERT(r, SkScalarCeilToInt(std::log2(f)) == f_log2);
        float c = std::max(1.f, GrWangsFormula::cubic(kIntolerance, pts));
        REPORTER_ASSERT(r, SkScalarNearlyEqual(c/f, 1, kTessellationTolerance));
    };

    auto check_quadratic_log2 = [&](const SkPoint* pts) {
        float f = std::max(1.f, wangs_formula_quadratic_reference_impl(kIntolerance, pts));
        int f_log2 = GrWangsFormula::quadratic_log2(kIntolerance, pts);
        REPORTER_ASSERT(r, SkScalarCeilToInt(std::log2(f)) == f_log2);
        float q = std::max(1.f, GrWangsFormula::quadratic(kIntolerance, pts));
        REPORTER_ASSERT(r, SkScalarNearlyEqual(q/f, 1, kTessellationTolerance));
    };

    SkRandom rand;

    for_random_matrices(&rand, [&](const SkMatrix& m) {
        SkPoint pts[4];
        m.mapPoints(pts, kSerp, 4);
        check_cubic_log2(pts);

        m.mapPoints(pts, kLoop, 4);
        check_cubic_log2(pts);

        m.mapPoints(pts, kQuad, 3);
        check_quadratic_log2(pts);
    });

    for_random_beziers(4, &rand, [&](const SkPoint pts[]) {
        check_cubic_log2(pts);
    });

    for_random_beziers(3, &rand, [&](const SkPoint pts[]) {
        check_quadratic_log2(pts);
    });
}

// Ensure using transformations gives the same result as pre-transforming all points.
DEF_TEST(WangsFormula_vectorXforms, r) {
    auto check_cubic_log2_with_transform = [&](const SkPoint* pts, const SkMatrix& m){
        SkPoint ptsXformed[4];
        m.mapPoints(ptsXformed, pts, 4);
        int expected = GrWangsFormula::cubic_log2(kIntolerance, ptsXformed);
        int actual = GrWangsFormula::cubic_log2(kIntolerance, pts, GrVectorXform(m));
        REPORTER_ASSERT(r, actual == expected);
    };

    auto check_quadratic_log2_with_transform = [&](const SkPoint* pts, const SkMatrix& m) {
        SkPoint ptsXformed[3];
        m.mapPoints(ptsXformed, pts, 3);
        int expected = GrWangsFormula::quadratic_log2(kIntolerance, ptsXformed);
        int actual = GrWangsFormula::quadratic_log2(kIntolerance, pts, GrVectorXform(m));
        REPORTER_ASSERT(r, actual == expected);
    };

    SkRandom rand;

    for_random_matrices(&rand, [&](const SkMatrix& m) {
        check_cubic_log2_with_transform(kSerp, m);
        check_cubic_log2_with_transform(kLoop, m);
        check_quadratic_log2_with_transform(kQuad, m);

        for_random_beziers(4, &rand, [&](const SkPoint pts[]) {
            check_cubic_log2_with_transform(pts, m);
        });

        for_random_beziers(3, &rand, [&](const SkPoint pts[]) {
            check_quadratic_log2_with_transform(pts, m);
        });
    });
}

DEF_TEST(WangsFormula_worst_case_cubic, r) {
    {
        SkPoint worstP[] = {{0,0}, {100,100}, {0,0}, {0,0}};
        REPORTER_ASSERT(r, GrWangsFormula::worst_case_cubic(kIntolerance, 100, 100) ==
                           wangs_formula_cubic_reference_impl(kIntolerance, worstP));
        REPORTER_ASSERT(r, GrWangsFormula::worst_case_cubic_log2(kIntolerance, 100, 100) ==
                           GrWangsFormula::cubic_log2(kIntolerance, worstP));
    }
    {
        SkPoint worstP[] = {{100,100}, {100,100}, {200,200}, {100,100}};
        REPORTER_ASSERT(r, GrWangsFormula::worst_case_cubic(kIntolerance, 100, 100) ==
                           wangs_formula_cubic_reference_impl(kIntolerance, worstP));
        REPORTER_ASSERT(r, GrWangsFormula::worst_case_cubic_log2(kIntolerance, 100, 100) ==
                           GrWangsFormula::cubic_log2(kIntolerance, worstP));
    }
    auto check_worst_case_cubic = [&](const SkPoint* pts) {
        SkRect bbox;
        bbox.setBoundsNoCheck(pts, 4);
        float worst = GrWangsFormula::worst_case_cubic(kIntolerance, bbox.width(), bbox.height());
        int worst_log2 = GrWangsFormula::worst_case_cubic_log2(kIntolerance, bbox.width(),
                                                               bbox.height());
        float actual = wangs_formula_cubic_reference_impl(kIntolerance, pts);
        REPORTER_ASSERT(r, worst >= actual);
        REPORTER_ASSERT(r, std::ceil(std::log2(std::max(1.f, worst))) == worst_log2);
    };
    SkRandom rand;
    for (int i = 0; i < 100; ++i) {
        for_random_beziers(4, &rand, [&](const SkPoint pts[]) {
            check_worst_case_cubic(pts);
        });
    }
}

// Ensure Wang's formula for quads produces max error within tolerance.
DEF_TEST(WangsFormula_quad_within_tol, r) {
    // Wang's formula and the quad math starts to lose precision with very large
    // coordinate values, so limit the magnitude a bit to prevent test failures
    // due to loss of precision.
    constexpr int maxExponent = 15;
    SkRandom rand;
    for_random_beziers(3, &rand, [&r](const SkPoint pts[]) {
        const int nsegs = static_cast<int>(
                std::ceil(wangs_formula_quadratic_reference_impl(kIntolerance, pts)));

        const float tdelta = 1.f / nsegs;
        for (int j = 0; j < nsegs; ++j) {
            const float tmin = j * tdelta, tmax = (j + 1) * tdelta;

            // Get section of quad in [tmin,tmax]
            const SkPoint* sectionPts;
            SkPoint tmp0[5];
            SkPoint tmp1[5];
            if (tmin == 0) {
                if (tmax == 1) {
                    sectionPts = pts;
                } else {
                    SkChopQuadAt(pts, tmp0, tmax);
                    sectionPts = tmp0;
                }
            } else {
                SkChopQuadAt(pts, tmp0, tmin);
                if (tmax == 1) {
                    sectionPts = tmp0 + 2;
                } else {
                    SkChopQuadAt(tmp0 + 2, tmp1, (tmax - tmin) / (1 - tmin));
                    sectionPts = tmp1;
                }
            }

            // For quads, max distance from baseline is always at t=0.5.
            SkPoint p;
            p = SkEvalQuadAt(sectionPts, 0.5f);

            // Get distance of p to baseline
            const SkPoint n = {sectionPts[2].fY - sectionPts[0].fY,
                               sectionPts[0].fX - sectionPts[2].fX};
            const float d = std::abs((p - sectionPts[0]).dot(n)) / n.length();

            // Check distance is within specified tolerance
            REPORTER_ASSERT(r, d <= (1.f / kIntolerance) + SK_ScalarNearlyZero);
        }
    }, maxExponent);
}

// Ensure the specialized version for rational cubics reduces to regular Wang's
// formula when all weights are equal to one
DEF_TEST(WangsFormula_rational_cubic_reduces, r) {
    constexpr static float kTessellationTolerance = 1/128.f;

    SkRandom rand;
    for (int i = 0; i < 100; ++i) {
        for_random_beziers(4, &rand, [&r](const SkPoint pts[]) {
            const float wts[4] = {1, 1, 1, 1};
            const float rational_nsegs =
                    wangs_formula_rational_cubic_reference_impl(kIntolerance, pts, wts);
            const float integral_nsegs = wangs_formula_cubic_reference_impl(kIntolerance, pts);
            REPORTER_ASSERT(
                    r, SkScalarNearlyEqual(rational_nsegs, integral_nsegs, kTessellationTolerance));
        });
    }
}

// Ensure the rational cubic version (used for conics) produces max error within tolerance.
DEF_TEST(WangsFormula_conic_within_tol, r) {
    constexpr int maxExponent = 9;

    // Elevates quad rational -> cubic rational
    const auto conic_to_cubic = [](const SkPoint conic[3], float w, SkPoint* cubic,
                                   float* cubic_w) {
        const SkPoint p0 = conic[0], p1 = conic[1], p2 = conic[2];
        const SkPoint c = p1 * (2 / 3.f * w);

        cubic[0] = p0;
        cubic[1] = p0 * (1 / 3.f) + c;
        cubic[2] = p2 * (1 / 3.f) + c;
        cubic[3] = p2;
        *cubic_w = (2 * w + 1) / 3.f;
    };

    SkRandom rand;
    for (int i = -10; i <= 10; ++i) {
        const float w = std::ldexp(1 + rand.nextF(), i);
        for_random_beziers(
                3, &rand,
                [&conic_to_cubic, &r, w](const SkPoint pts[]) {
                    float cubicW;
                    SkPoint cubicPts[4];
                    conic_to_cubic(pts, w, &cubicPts[0], &cubicW);

                    const float wts[4] = {1, cubicW, cubicW, 1};
                    const int nsegs =
                            static_cast<int>(std::ceil(wangs_formula_rational_cubic_reference_impl(
                                    kIntolerance, cubicPts, wts)));

                    const SkConic conic(pts[0], pts[1], pts[2], w);
                    const float tdelta = 1.f / nsegs;
                    for (int j = 0; j < nsegs; ++j) {
                        const float tmin = j * tdelta, tmax = (j + 1) * tdelta;
                        // Chop original conic to [tmin, tmax] and evaluate at 0.5
                        SkConic dst;
                        SkPoint p;
                        conic.chopAt(tmin, tmax, &dst);
                        dst.evalAt(0.5f, &p);

                        // Get distance of p to baseline
                        const SkPoint n = {dst.fPts[2].fY - dst.fPts[0].fY,
                                           dst.fPts[0].fX - dst.fPts[2].fX};
                        SkASSERT(n.length() != 0);
                        const float d = std::abs((p - dst.fPts[0]).dot(n)) / n.length();

                        // Check distance is within tolerance
                        REPORTER_ASSERT(r, d <= (1.f / kIntolerance) + SK_ScalarNearlyZero);
                    }
                },
                maxExponent);
    }
}
