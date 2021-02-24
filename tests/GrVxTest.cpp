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

static bool check_approx_acos(skiatest::Reporter* r, float x, float approx_acos_x) {
    float acosf_x = acosf(x);
    float error = acosf_x - approx_acos_x;
    if (!(fabsf(error) <= GRVX_APPROX_ACOS_MAX_ERROR)) {
        ERRORF(r, "Larger-than-expected error from grvx::approx_acos\n"
                  "  x=              %f\n"
                  "  approx_acos_x=  %f  (%f degrees\n"
                  "  acosf_x=        %f  (%f degrees\n"
                  "  error=          %f  (%f degrees)\n"
                  "  tolerance=      %f  (%f degrees)\n\n",
                  x, approx_acos_x, SkRadiansToDegrees(approx_acos_x), acosf_x,
                  SkRadiansToDegrees(acosf_x), error, SkRadiansToDegrees(error),
                  GRVX_APPROX_ACOS_MAX_ERROR, SkRadiansToDegrees(GRVX_APPROX_ACOS_MAX_ERROR));
        return false;
    }
    return true;
}

DEF_TEST(grvx_approx_acos, r) {
    float4 boundaries = approx_acos(float4{-1, 0, 1, 0});
    check_approx_acos(r, -1, boundaries[0]);
    check_approx_acos(r, 0, boundaries[1]);
    check_approx_acos(r, +1, boundaries[2]);

    // Select a distribution of starting points around which to begin testing approx_acos. These
    // fall roughly around the known minimum and maximum errors. No need to include -1, 0, or 1
    // since those were just tested above. (Those are tricky because 0 is an inflection and the
    // derivative is infinite at 1 and -1.)
    constexpr static int N = 8;
    vec<8> x = {-.99f, -.8f, -.4f, -.2f, .2f, .4f, .8f, .99f};

    // Converge at the various local minima and maxima of "approx_acos(x) - cosf(x)" and verify that
    // approx_acos is always within "kTolerance" degrees of the expected answer.
    vec<N> err_;
    for (int iter = 0; iter < 10; ++iter) {
        // Run our approximate inverse cosine approximation.
        vec<N> approx_acos_x = approx_acos(x);

        // Find d/dx(error)
        //    = d/dx(approx_acos(x) - acos(x))
        //    = (f'g - fg')/gg + 1/sqrt(1 - x^2), [where f = bx^3 + ax, g = dx^4 + cx^2 + 1]
        vec<N> xx = x*x;
        vec<N> a = -0.939115566365855f;
        vec<N> b =  0.9217841528914573f;
        vec<N> c = -1.2845906244690837f;
        vec<N> d =  0.295624144969963174f;
        vec<N> f = (b*xx + a)*x;
        vec<N> f_ = 3*b*xx + a;
        vec<N> g = (d*xx + c)*xx + 1;
        vec<N> g_ = (4*d*xx + 2*c)*x;
        vec<N> gg = g*g;
        vec<N> q = skvx::sqrt(1 - xx);
        err_ = (f_*g - f*g_)/gg + 1/q;

        // Find d^2/dx^2(error)
        //    = ((f''g - fg'')g^2 - (f'g - fg')2gg') / g^4 + x(1 - x^2)^(-3/2)
        //    = ((f''g - fg'')g - (f'g - fg')2g') / g^3 + x(1 - x^2)^(-3/2)
        vec<N> f__ = 6*b*x;
        vec<N> g__ = 12*d*xx + 2*c;
        vec<N> err__ = ((f__*g - f*g__)*g - (f_*g - f*g_)*2*g_) / (gg*g) + x/((1 - xx)*q);

#if 0
        SkDebugf("\n\niter %i\n", iter);
#endif
        // Ensure each lane's approximation is within maximum error.
        for (int j = 0; j < N; ++j) {
#if 0
            SkDebugf("x=%f  err=%f  err'=%f  err''=%f\n",
                     x[j], SkRadiansToDegrees(approx_acos_x[j] - acosf(x[j])),
                     SkRadiansToDegrees(err_[j]), SkRadiansToDegrees(err__[j]));
#endif
            if (!check_approx_acos(r, x[j], approx_acos_x[j])) {
                return;
            }
        }

        // Use Newton's method to update the x values to locations closer to their local minimum or
        // maximum. (This is where d/dx(error) == 0.)
        x -= err_/err__;
        x = skvx::pin(x, vec<N>(-.99f), vec<N>(.99f));
    }

    // Ensure each lane converged to a local minimum or maximum.
    for (int j = 0; j < N; ++j) {
        REPORTER_ASSERT(r, SkScalarNearlyZero(err_[j]));
    }

    // Make sure we found all the actual known locations of local min/max error.
    for (float knownRoot : {-0.983536f, -0.867381f, -0.410923f, 0.410923f, 0.867381f, 0.983536f}) {
        REPORTER_ASSERT(r, skvx::any(skvx::abs(x - knownRoot) < SK_ScalarNearlyZero));
    }
}

static float precise_angle_between_vectors(SkPoint a, SkPoint b) {
    if (a.isZero() || b.isZero()) {
        return 0;
    }
    double ax=a.fX, ay=a.fY, bx=b.fX, by=b.fY;
    double theta = (ax*bx + ay*by) / sqrt(ax*ax + ay*ay) / sqrt(bx*bx + by*by);
    return (float)acos(theta);
}

static bool check_approx_angle_between_vectors(skiatest::Reporter* r, SkVector a, SkVector b,
                                               float approxTheta) {
    float expectedTheta = precise_angle_between_vectors(a, b);
    float error = expectedTheta - approxTheta;
    if (!(fabsf(error) <= GRVX_APPROX_ACOS_MAX_ERROR + SK_ScalarNearlyZero)) {
        int expAx = SkFloat2Bits(a.fX) >> 23 & 0xff;
        int expAy = SkFloat2Bits(a.fY) >> 23 & 0xff;
        int expBx = SkFloat2Bits(b.fX) >> 23 & 0xff;
        int expBy = SkFloat2Bits(b.fY) >> 23 & 0xff;
        ERRORF(r, "Larger-than-expected error from grvx::approx_angle_between_vectors\n"
                  "  a=                {%f, %f}\n"
                  "  b=                {%f, %f}\n"
                  "  expA=             {%u, %u}\n"
                  "  expB=             {%u, %u}\n"
                  "  approxTheta=      %f  (%f degrees\n"
                  "  expectedTheta=     %f  (%f degrees)\n"
                  "  error=             %f  (%f degrees)\n"
                  "  tolerance=         %f  (%f degrees)\n\n",
                  a.fX, a.fY, b.fX, b.fY, expAx, expAy, expBx, expBy, approxTheta,
                  SkRadiansToDegrees(approxTheta), expectedTheta, SkRadiansToDegrees(expectedTheta),
                  error, SkRadiansToDegrees(error), GRVX_APPROX_ACOS_MAX_ERROR,
                  SkRadiansToDegrees(GRVX_APPROX_ACOS_MAX_ERROR));
        return false;
    }
    return true;
}

static bool check_approx_angle_between_vectors(skiatest::Reporter* r, SkVector a, SkVector b) {
    float approxTheta = grvx::approx_angle_between_vectors(bit_pun<float2>(a),
                                                           bit_pun<float2>(b)).val;
    return check_approx_angle_between_vectors(r, a, b, approxTheta);
}

DEF_TEST(grvx_approx_angle_between_vectors, r) {
    // Test when a and/or b are zero.
    REPORTER_ASSERT(r, SkScalarNearlyZero(grvx::approx_angle_between_vectors<2>({0,0}, {0,0}).val));
    REPORTER_ASSERT(r, SkScalarNearlyZero(grvx::approx_angle_between_vectors<2>({1,1}, {0,0}).val));
    REPORTER_ASSERT(r, SkScalarNearlyZero(grvx::approx_angle_between_vectors<2>({0,0}, {1,1}).val));
    check_approx_angle_between_vectors(r, {0,0}, {0,0});
    check_approx_angle_between_vectors(r, {1,1}, {0,0});
    check_approx_angle_between_vectors(r, {0,0}, {1,1});

    // Test infinities.
    REPORTER_ASSERT(r, SkScalarNearlyZero(grvx::approx_angle_between_vectors<2>(
            {std::numeric_limits<float>::infinity(),1}, {2,3}).val));

    // Test NaNs.
    REPORTER_ASSERT(r, SkScalarNearlyZero(grvx::approx_angle_between_vectors<2>(
            {std::numeric_limits<float>::quiet_NaN(),1}, {2,3}).val));

    // Test demorms.
    float epsilon = std::numeric_limits<float>::denorm_min();
    REPORTER_ASSERT(r, SkScalarNearlyZero(grvx::approx_angle_between_vectors<2>(
            {epsilon, epsilon}, {epsilon, epsilon}).val));

    // Test random floats of all types.
    uint4 mantissas = {0,0,0,0};
    uint4 exp = uint4{126, 127, 128, 129};
    for (uint32_t i = 0; i < (1 << 12); ++i) {
        // approx_angle_between_vectors is only valid for absolute values < 2^31.
        uint4 exp_ = skvx::min(exp, 127 + 30);
        uint32_t a=exp_[0], b=exp_[1], c=exp_[2], d=exp_[3];
        // approx_angle_between_vectors is only valid if at least one vector component's magnitude
        // is >2^-31.
        a = std::max(a, 127u - 30);
        c = std::max(a, 127u - 30);
        // Run two tests where both components of both vectors have the same exponent, one where
        // both components of a given vector have the same exponent, and one where all components of
        // all vectors have different exponents.
        uint4 x0exp = uint4{a,c,a,a} << 23;
        uint4 y0exp = uint4{a,c,a,b} << 23;
        uint4 x1exp = uint4{a,c,c,c} << 23;
        uint4 y1exp = uint4{a,c,c,d} << 23;
        uint4 signs = uint4{i<<31, i<<30, i<<29, i<<28} & (1u<<31);
        float4 x0 = bit_pun<float4>(signs | x0exp | mantissas[0]);
        float4 y0 = bit_pun<float4>(signs | y0exp | mantissas[1]);
        float4 x1 = bit_pun<float4>(signs | x1exp | mantissas[2]);
        float4 y1 = bit_pun<float4>(signs | y1exp | mantissas[3]);
        float4 rads = approx_angle_between_vectors(skvx::join(x0, y0), skvx::join(x1, y1));
        for (int j = 0; j < 4; ++j) {
            if (!check_approx_angle_between_vectors(r, {x0[j], y0[j]}, {x1[j], y1[j]}, rads[j])) {
                return;
            }
        }
        // Adding primes makes sure we test every value before we repeat.
        mantissas = (mantissas + uint4{123456791, 201345691, 198765433, 156789029}) & ((1<<23) - 1);
        exp = (exp + uint4{79, 83, 199, 7}) & 0xff;
    }
}

template<int N, typename T> void check_strided_loads(skiatest::Reporter* r) {
    using Vec = skvx::Vec<N,T>;
    T values[N*4];
    std::iota(values, values + N*4, 0);
    Vec a, b, c, d;
    grvx::strided_load2(values, a, b);
    for (int i = 0; i < N; ++i) {
        REPORTER_ASSERT(r, a[i] == values[i*2]);
        REPORTER_ASSERT(r, b[i] == values[i*2 + 1]);
    }
    grvx::strided_load4(values, a, b, c, d);
    for (int i = 0; i < N; ++i) {
        REPORTER_ASSERT(r, a[i] == values[i*4]);
        REPORTER_ASSERT(r, b[i] == values[i*4 + 1]);
        REPORTER_ASSERT(r, c[i] == values[i*4 + 2]);
        REPORTER_ASSERT(r, d[i] == values[i*4 + 3]);
    }
}

template<typename T> void check_strided_loads(skiatest::Reporter* r) {
    check_strided_loads<1,T>(r);
    check_strided_loads<2,T>(r);
    check_strided_loads<4,T>(r);
    check_strided_loads<8,T>(r);
    check_strided_loads<16,T>(r);
    check_strided_loads<32,T>(r);
}

DEF_TEST(GrVx_strided_loads, r) {
    check_strided_loads<uint32_t>(r);
    check_strided_loads<uint16_t>(r);
    check_strided_loads<uint8_t>(r);
    check_strided_loads<int32_t>(r);
    check_strided_loads<int16_t>(r);
    check_strided_loads<int8_t>(r);
    check_strided_loads<float>(r);
}
