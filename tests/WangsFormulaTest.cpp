/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkRandom.h"
#include "src/core/SkGeometry.h"
#include "src/gpu/tessellate/WangsFormula.h"
#include "tests/Test.h"

namespace skgpu {

constexpr static float kPrecision = 4;  // 1/4 pixel max error.

const SkPoint kSerp[4] = {
        {285.625f, 499.687f}, {411.625f, 808.188f}, {1064.62f, 135.688f}, {1042.63f, 585.187f}};

const SkPoint kLoop[4] = {
        {635.625f, 614.687f}, {171.625f, 236.188f}, {1064.62f, 135.688f}, {516.625f, 570.187f}};

const SkPoint kQuad[4] = {
        {460.625f, 557.187f}, {707.121f, 209.688f}, {779.628f, 577.687f}};

static float wangs_formula_quadratic_reference_impl(float precision, const SkPoint p[3]) {
    float k = (2 * 1) / 8.f * precision;
    return sqrtf(k * (p[0] - p[1]*2 + p[2]).length());
}

static float wangs_formula_cubic_reference_impl(float precision, const SkPoint p[4]) {
    float k = (3 * 2) / 8.f * precision;
    return sqrtf(k * std::max((p[0] - p[1]*2 + p[2]).length(),
                              (p[1] - p[2]*2 + p[3]).length()));
}

// Returns number of segments for linearized quadratic rational. This is an analogue
// to Wang's formula, taken from:
//
//   J. Zheng, T. Sederberg. "Estimating Tessellation Parameter Intervals for
//   Rational Curves and Surfaces." ACM Transactions on Graphics 19(1). 2000.
// See Thm 3, Corollary 1.
//
// Input points should be in projected space.
static float wangs_formula_conic_reference_impl(float precision,
                                                const SkPoint P[3],
                                                const float w) {
    // Compute center of bounding box in projected space
    float min_x = P[0].fX, max_x = min_x,
          min_y = P[0].fY, max_y = min_y;
    for (int i = 1; i < 3; i++) {
        min_x = std::min(min_x, P[i].fX);
        max_x = std::max(max_x, P[i].fX);
        min_y = std::min(min_y, P[i].fY);
        max_y = std::max(max_y, P[i].fY);
    }
    const SkPoint C = SkPoint::Make(0.5f * (min_x + max_x), 0.5f * (min_y + max_y));

    // Translate control points and compute max length
    SkPoint tP[3] = {P[0] - C, P[1] - C, P[2] - C};
    float max_len = 0;
    for (int i = 0; i < 3; i++) {
        max_len = std::max(max_len, tP[i].length());
    }
    SkASSERT(max_len > 0);

    // Compute delta = parametric step size of linearization
    const float eps = 1 / precision;
    const float r_minus_eps = std::max(0.f, max_len - eps);
    const float min_w = std::min(w, 1.f);
    const float numer = 4 * min_w * eps;
    const float denom =
            (tP[2] - tP[1] * 2 * w + tP[0]).length() + r_minus_eps * std::abs(1 - 2 * w + 1);
    const float delta = sqrtf(numer / denom);

    // Return corresponding num segments in the interval [tmin,tmax]
    constexpr float tmin = 0, tmax = 1;
    SkASSERT(delta > 0);
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
DEF_TEST(wangs_formula_log2, r) {
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

    // wangs_formula_cubic and wangs_formula_quadratic both use rsqrt instead of sqrt for speed.
    // Linearization is all approximate anyway, so as long as we are within ~1/2 tessellation
    // segment of the reference value we are good enough.
    constexpr static float kTessellationTolerance = 1/128.f;

    for (int level = 0; level < 30; ++level) {
        float epsilon = std::ldexp(SK_ScalarNearlyZero, level * 2);
        SkPoint pts[4];

        {
            // Test cubic boundaries.
            //     f = sqrt(k * length(max(abs(p0 - p1*2 + p2),
            //                             abs(p1 - p2*2 + p3))));
            constexpr static float k = (3 * 2) / (8 * (1.f/kPrecision));
            float x = std::ldexp(1, level * 2) / k;
            setupCubicLengthTerm(level << 1, pts, x - epsilon);
            float referenceValue = wangs_formula_cubic_reference_impl(kPrecision, pts);
            REPORTER_ASSERT(r, std::ceil(std::log2(referenceValue)) == level);
            float c = wangs_formula::cubic(kPrecision, pts);
            REPORTER_ASSERT(r, SkScalarNearlyEqual(c/referenceValue, 1, kTessellationTolerance));
            REPORTER_ASSERT(r, wangs_formula::cubic_log2(kPrecision, pts) == level);
            setupCubicLengthTerm(level << 1, pts, x + epsilon);
            referenceValue = wangs_formula_cubic_reference_impl(kPrecision, pts);
            REPORTER_ASSERT(r, std::ceil(std::log2(referenceValue)) == level + 1);
            c = wangs_formula::cubic(kPrecision, pts);
            REPORTER_ASSERT(r, SkScalarNearlyEqual(c/referenceValue, 1, kTessellationTolerance));
            REPORTER_ASSERT(r, wangs_formula::cubic_log2(kPrecision, pts) == level + 1);
        }

        {
            // Test quadratic boundaries.
            //     f = std::sqrt(k * Length(p0 - p1*2 + p2));
            constexpr static float k = 2 / (8 * (1.f/kPrecision));
            float x = std::ldexp(1, level * 2) / k;
            setupQuadraticLengthTerm(level << 1, pts, x - epsilon);
            float referenceValue = wangs_formula_quadratic_reference_impl(kPrecision, pts);
            REPORTER_ASSERT(r, std::ceil(std::log2(referenceValue)) == level);
            float q = wangs_formula::quadratic(kPrecision, pts);
            REPORTER_ASSERT(r, SkScalarNearlyEqual(q/referenceValue, 1, kTessellationTolerance));
            REPORTER_ASSERT(r, wangs_formula::quadratic_log2(kPrecision, pts) == level);
            setupQuadraticLengthTerm(level << 1, pts, x + epsilon);
            referenceValue = wangs_formula_quadratic_reference_impl(kPrecision, pts);
            REPORTER_ASSERT(r, std::ceil(std::log2(referenceValue)) == level+1);
            q = wangs_formula::quadratic(kPrecision, pts);
            REPORTER_ASSERT(r, SkScalarNearlyEqual(q/referenceValue, 1, kTessellationTolerance));
            REPORTER_ASSERT(r, wangs_formula::quadratic_log2(kPrecision, pts) == level + 1);
        }
    }

    auto check_cubic_log2 = [&](const SkPoint* pts) {
        float f = std::max(1.f, wangs_formula_cubic_reference_impl(kPrecision, pts));
        int f_log2 = wangs_formula::cubic_log2(kPrecision, pts);
        REPORTER_ASSERT(r, SkScalarCeilToInt(std::log2(f)) == f_log2);
        float c = std::max(1.f, wangs_formula::cubic(kPrecision, pts));
        REPORTER_ASSERT(r, SkScalarNearlyEqual(c/f, 1, kTessellationTolerance));
    };

    auto check_quadratic_log2 = [&](const SkPoint* pts) {
        float f = std::max(1.f, wangs_formula_quadratic_reference_impl(kPrecision, pts));
        int f_log2 = wangs_formula::quadratic_log2(kPrecision, pts);
        REPORTER_ASSERT(r, SkScalarCeilToInt(std::log2(f)) == f_log2);
        float q = std::max(1.f, wangs_formula::quadratic(kPrecision, pts));
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
DEF_TEST(wangs_formula_vectorXforms, r) {
    auto check_cubic_log2_with_transform = [&](const SkPoint* pts, const SkMatrix& m){
        SkPoint ptsXformed[4];
        m.mapPoints(ptsXformed, pts, 4);
        int expected = wangs_formula::cubic_log2(kPrecision, ptsXformed);
        int actual = wangs_formula::cubic_log2(kPrecision, pts, wangs_formula::VectorXform(m));
        REPORTER_ASSERT(r, actual == expected);
    };

    auto check_quadratic_log2_with_transform = [&](const SkPoint* pts, const SkMatrix& m) {
        SkPoint ptsXformed[3];
        m.mapPoints(ptsXformed, pts, 3);
        int expected = wangs_formula::quadratic_log2(kPrecision, ptsXformed);
        int actual = wangs_formula::quadratic_log2(kPrecision, pts, wangs_formula::VectorXform(m));
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

DEF_TEST(wangs_formula_worst_case_cubic, r) {
    {
        SkPoint worstP[] = {{0,0}, {100,100}, {0,0}, {0,0}};
        REPORTER_ASSERT(r, wangs_formula::worst_case_cubic(kPrecision, 100, 100) ==
                           wangs_formula_cubic_reference_impl(kPrecision, worstP));
        REPORTER_ASSERT(r, wangs_formula::worst_case_cubic_log2(kPrecision, 100, 100) ==
                           wangs_formula::cubic_log2(kPrecision, worstP));
    }
    {
        SkPoint worstP[] = {{100,100}, {100,100}, {200,200}, {100,100}};
        REPORTER_ASSERT(r, wangs_formula::worst_case_cubic(kPrecision, 100, 100) ==
                           wangs_formula_cubic_reference_impl(kPrecision, worstP));
        REPORTER_ASSERT(r, wangs_formula::worst_case_cubic_log2(kPrecision, 100, 100) ==
                           wangs_formula::cubic_log2(kPrecision, worstP));
    }
    auto check_worst_case_cubic = [&](const SkPoint* pts) {
        SkRect bbox;
        bbox.setBoundsNoCheck(pts, 4);
        float worst = wangs_formula::worst_case_cubic(kPrecision, bbox.width(), bbox.height());
        int worst_log2 = wangs_formula::worst_case_cubic_log2(kPrecision, bbox.width(),
                                                               bbox.height());
        float actual = wangs_formula_cubic_reference_impl(kPrecision, pts);
        REPORTER_ASSERT(r, worst >= actual);
        REPORTER_ASSERT(r, std::ceil(std::log2(std::max(1.f, worst))) == worst_log2);
    };
    SkRandom rand;
    for (int i = 0; i < 100; ++i) {
        for_random_beziers(4, &rand, [&](const SkPoint pts[]) {
            check_worst_case_cubic(pts);
        });
    }
    // Make sure overflow saturates at infinity (not NaN).
    constexpr static float inf = std::numeric_limits<float>::infinity();
    REPORTER_ASSERT(r, wangs_formula::worst_case_cubic_pow4(kPrecision, inf, inf) == inf);
    REPORTER_ASSERT(r, wangs_formula::worst_case_cubic(kPrecision, inf, inf) == inf);
}

// Ensure Wang's formula for quads produces max error within tolerance.
DEF_TEST(wangs_formula_quad_within_tol, r) {
    // Wang's formula and the quad math starts to lose precision with very large
    // coordinate values, so limit the magnitude a bit to prevent test failures
    // due to loss of precision.
    constexpr int maxExponent = 15;
    SkRandom rand;
    for_random_beziers(3, &rand, [&r](const SkPoint pts[]) {
        const int nsegs = static_cast<int>(
                std::ceil(wangs_formula_quadratic_reference_impl(kPrecision, pts)));

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
            REPORTER_ASSERT(r, d <= (1.f / kPrecision) + SK_ScalarNearlyZero);
        }
    }, maxExponent);
}

// Ensure the specialized version for rational quads reduces to regular Wang's
// formula when all weights are equal to one
DEF_TEST(wangs_formula_rational_quad_reduces, r) {
    constexpr static float kTessellationTolerance = 1 / 128.f;

    SkRandom rand;
    for (int i = 0; i < 100; ++i) {
        for_random_beziers(3, &rand, [&r](const SkPoint pts[]) {
            const float rational_nsegs = wangs_formula::conic(kPrecision, pts, 1.f);
            const float integral_nsegs = wangs_formula_quadratic_reference_impl(kPrecision, pts);
            REPORTER_ASSERT(
                    r, SkScalarNearlyEqual(rational_nsegs, integral_nsegs, kTessellationTolerance));
        });
    }
}

// Ensure the rational quad version (used for conics) produces max error within tolerance.
DEF_TEST(wangs_formula_conic_within_tol, r) {
    constexpr int maxExponent = 24;

    // Single-precision functions in SkConic/SkGeometry lose too much accuracy with
    // large-magnitude curves and large weights for this test to pass.
    using Sk2d = skvx::Vec<2, double>;
    const auto eval_conic = [](const SkPoint pts[3], float w, float t) -> Sk2d {
        const auto eval = [](Sk2d A, Sk2d B, Sk2d C, float t) -> Sk2d {
            return (A * t + B) * t + C;
        };

        const Sk2d p0 = {pts[0].fX, pts[0].fY};
        const Sk2d p1 = {pts[1].fX, pts[1].fY};
        const Sk2d p1w = p1 * w;
        const Sk2d p2 = {pts[2].fX, pts[2].fY};
        Sk2d numer = eval(p2 - p1w * 2 + p0, (p1w - p0) * 2, p0, t);

        Sk2d denomC = {1, 1};
        Sk2d denomB = {2 * (w - 1), 2 * (w - 1)};
        Sk2d denomA = {-2 * (w - 1), -2 * (w - 1)};
        Sk2d denom = eval(denomA, denomB, denomC, t);
        return numer / denom;
    };

    const auto dot = [](const Sk2d& a, const Sk2d& b) -> double {
        return a[0] * b[0] + a[1] * b[1];
    };

    const auto length = [](const Sk2d& p) -> double { return sqrt(p[0] * p[0] + p[1] * p[1]); };

    SkRandom rand;
    for (int i = -10; i <= 10; ++i) {
        const float w = std::ldexp(1 + rand.nextF(), i);
        for_random_beziers(
                3, &rand,
                [&](const SkPoint pts[]) {
                    const int nsegs = SkScalarCeilToInt(wangs_formula::conic(kPrecision, pts, w));

                    const float tdelta = 1.f / nsegs;
                    for (int j = 0; j < nsegs; ++j) {
                        const float tmin = j * tdelta, tmax = (j + 1) * tdelta,
                                    tmid = 0.5f * (tmin + tmax);

                        Sk2d p0, p1, p2;
                        p0 = eval_conic(pts, w, tmin);
                        p1 = eval_conic(pts, w, tmid);
                        p2 = eval_conic(pts, w, tmax);

                        // Get distance of p1 to baseline (p0, p2).
                        const Sk2d n = {p2[1] - p0[1], p0[0] - p2[0]};
                        SkASSERT(length(n) != 0);
                        const double d = std::abs(dot(p1 - p0, n)) / length(n);

                        // Check distance is within tolerance
                        REPORTER_ASSERT(r, d <= (1.0 / kPrecision) + SK_ScalarNearlyZero);
                    }
                },
                maxExponent);
    }
}

// Ensure the vectorized conic version equals the reference implementation
DEF_TEST(wangs_formula_conic_matches_reference, r) {
    SkRandom rand;
    for (int i = -10; i <= 10; ++i) {
        const float w = std::ldexp(1 + rand.nextF(), i);
        for_random_beziers(3, &rand, [&r, w](const SkPoint pts[]) {
            const float ref_nsegs = wangs_formula_conic_reference_impl(kPrecision, pts, w);
            const float nsegs = wangs_formula::conic(kPrecision, pts, w);

            // Because the Gr version may implement the math differently for performance,
            // allow different slack in the comparison based on the rough scale of the answer.
            const float cmpThresh = ref_nsegs * (1.f / (1 << 20));
            REPORTER_ASSERT(r, SkScalarNearlyEqual(ref_nsegs, nsegs, cmpThresh));
        });
    }
}

// Ensure using transformations gives the same result as pre-transforming all points.
DEF_TEST(wangs_formula_conic_vectorXforms, r) {
    auto check_conic_with_transform = [&](const SkPoint* pts, float w, const SkMatrix& m) {
        SkPoint ptsXformed[3];
        m.mapPoints(ptsXformed, pts, 3);
        float expected = wangs_formula::conic(kPrecision, ptsXformed, w);
        float actual = wangs_formula::conic(kPrecision, pts, w, wangs_formula::VectorXform(m));
        REPORTER_ASSERT(r, SkScalarNearlyEqual(actual, expected));
    };

    SkRandom rand;
    for (int i = -10; i <= 10; ++i) {
        const float w = std::ldexp(1 + rand.nextF(), i);
        for_random_beziers(3, &rand, [&](const SkPoint pts[]) {
            check_conic_with_transform(pts, w, SkMatrix::I());
            check_conic_with_transform(
                    pts, w, SkMatrix::Scale(rand.nextRangeF(-10, 10), rand.nextRangeF(-10, 10)));

            // Random 2x2 matrix
            SkMatrix m;
            m.setScaleX(rand.nextRangeF(-10, 10));
            m.setSkewX(rand.nextRangeF(-10, 10));
            m.setSkewY(rand.nextRangeF(-10, 10));
            m.setScaleY(rand.nextRangeF(-10, 10));
            check_conic_with_transform(pts, w, m);
        });
    }
}

}  // namespace skgpu
