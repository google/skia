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

static float length(const Sk2f& v) {
    Sk2f vv = v*v;
    return SkScalarSqrt(vv[0] + vv[1]);
}

static float wangs_formula_quadratic_reference_impl(float intolerance, const SkPoint pts[4]) {
    Sk2f p0 = Sk2f::Load(pts);
    Sk2f p1 = Sk2f::Load(pts + 1);
    Sk2f p2 = Sk2f::Load(pts + 2);
    float k = GrWangsFormula::quadratic_k(intolerance);
    return SkScalarSqrt(k * length(p0 - p1*2 + p2));
}

static float wangs_formula_cubic_reference_impl(float intolerance, const SkPoint pts[4]) {
    Sk2f p0 = Sk2f::Load(pts);
    Sk2f p1 = Sk2f::Load(pts + 1);
    Sk2f p2 = Sk2f::Load(pts + 2);
    Sk2f p3 = Sk2f::Load(pts + 3);
    float k = GrWangsFormula::cubic_k(intolerance);
    return SkScalarSqrt(k * length(Sk2f::Max((p0 - p1*2 + p2).abs(),
                                             (p1 - p2*2 + p3).abs())));
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
                               std::function<void(const SkPoint[])> f) {
    SkASSERT(numPoints <= 4);
    SkPoint pts[4];
    for (int i = -10; i <= 30; ++i) {
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
