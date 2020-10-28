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

using namespace grvx;
using skvx::bit_pun;

#if GRVX_FAST_RCP_PRECISION_BITS
DEF_TEST(grvx_fast_rcp, r) {
    constexpr static float kTolerance = 1.f / (1 << GRVX_FAST_RCP_PRECISION_BITS);
    int4 sign = int4{0,0,1,1} << 31;
    int4 randomExpValues = int4{126, 127, 128, 129};
    // Test literally every mantissa.
    for (int4 mantissa = {0,1,2,3}; skvx::all(mantissa < (1 << 23)); mantissa += 4) {
        int4 exp = skvx::pin(randomExpValues & 0xff, int4(1), int4(252));
        float4 x = bit_pun<float4>(sign | (exp << 23) | mantissa);
        float4 fast_rcp_x = fast_rcp(x);
        float4 error = 1 - x * fast_rcp_x;
        if (!(skvx::all(skvx::abs(error) <= kTolerance))) {
            float4 one_over_x = 1/x;
            ERRORF(r, "Larger-than-expected error from grvx::fast_rcp\n"
                      "  x=           {%f, %f, %f, %f}\n"
                      "  exp=         {%i, %i, %i, %i}\n"
                      "  fast_rcp_x=  {%f, %f, %f, %f}\n"
                      "  one_over_x=  {%f, %f, %f, %f}\n"
                      "  error=       {%f, %f, %f, %f}\n"
                      "  tolerance=   %i precision bits (%f)\n\n",
                      x[0], x[1], x[2], x[3],
                      exp[0], exp[1], exp[2], exp[3],
                      fast_rcp_x[0], fast_rcp_x[1], fast_rcp_x[2], fast_rcp_x[3],
                      one_over_x[0], one_over_x[1], one_over_x[2], one_over_x[3],
                      error[0], error[1], error[2], error[3],
                      GRVX_FAST_RCP_PRECISION_BITS, kTolerance);
            return;
        }
        // Adding primes makes sure we test every exponent before we repeat.
        randomExpValues += int4{79, 83, 199, 7};
    }
}
#endif

#if GRVX_FAST_RSQRT_PRECISION_BITS
DEF_TEST(grvx_fast_rsqrt, r) {
    constexpr static float kTolerance = 1.f / (1 << GRVX_FAST_RSQRT_PRECISION_BITS);
    int4 randomExpValues = int4{126, 127, 128, 129};
    // Test literally every mantissa.
    for (int4 mantissa = {0,1,2,3}; skvx::all(mantissa < (1 << 23)); mantissa += 4) {
        int4 exp = skvx::pin(randomExpValues & 0xff, int4(1), int4(254));
        float4 x = bit_pun<float4>((exp << 23) | mantissa);
        float4 fast_rsqrt_x = fast_rsqrt(x);
        float4 error = 1 - skvx::sqrt(x) * fast_rsqrt_x;
        if (!(skvx::all(skvx::abs(error) <= kTolerance))) {
            float4 one_over_sqrt_x = 1/sqrt(x);
            ERRORF(r, "Larger-than-expected error from grvx::fast_rsqrt\n"
                      "  x=                {%f, %f, %f, %f}\n"
                      "  exp=              {%i, %i, %i, %i}\n"
                      "  fast_rsqrt_x=     {%f, %f, %f, %f}\n"
                      "  one_over_sqrt_x=  {%f, %f, %f, %f}\n"
                      "  error=            {%f, %f, %f, %f}\n"
                      "  tolerance=        %i precision bits (%f)\n\n",
                      x[0], x[1], x[2], x[3],
                      exp[0], exp[1], exp[2], exp[3],
                      fast_rsqrt_x[0], fast_rsqrt_x[1], fast_rsqrt_x[2], fast_rsqrt_x[3],
                      one_over_sqrt_x[0], one_over_sqrt_x[1], one_over_sqrt_x[2],one_over_sqrt_x[3],
                      error[0], error[1], error[2], error[3],
                      GRVX_FAST_RSQRT_PRECISION_BITS, kTolerance);
            return;
        }
        // Adding primes makes sure we test every exponent before we repeat.
        randomExpValues += int4{79, 83, 199, 7};
    }
}
#endif

static bool check_fast_acos(skiatest::Reporter* r, float x, float fast_acos_x) {
    float acosf_x = acosf(x);
    float error = acosf_x - fast_acos_x;
    if (!(fabsf(error) <= GRVX_FAST_ACOS_MAX_ERROR)) {
        ERRORF(r, "Larger-than-expected error from grvx::fast_acos\n"
                  "  x=            %f\n"
                  "  fast_acos_x=  %f  (%f degrees\n"
                  "  acosf_x=      %f  (%f degrees\n"
                  "  error=        %f  (%f degrees)\n"
                  "  tolerance=    %f  (%f degrees)\n\n",
                  x, fast_acos_x, SkRadiansToDegrees(fast_acos_x), acosf_x,
                  SkRadiansToDegrees(acosf_x), error, SkRadiansToDegrees(error),
                  GRVX_FAST_ACOS_MAX_ERROR, SkRadiansToDegrees(GRVX_FAST_ACOS_MAX_ERROR));
        return false;
    }
    return true;
}

DEF_TEST(grvx_fast_acos, r) {
    float4 boundaries = fast_acos(float4{-1, 0, 1, 0});
    check_fast_acos(r, -1, boundaries[0]);
    check_fast_acos(r, 0, boundaries[1]);
    check_fast_acos(r, +1, boundaries[2]);

    // Select a distribution of starting points around which to begin testing fast_acos. These fall
    // roughly around the known minimum and maximum errors. No need to include -1, 0, or 1 since
    // those were just tested above. (Those are tricky because 0 is an inflection and the derivative
    // is infinite at 1 and -1.)
    constexpr static int N = 8;
    vec<8> x = {-.99f, -.8f, -.4f, -.2f, .2f, .4f, .8f, .99f};

    // Converge at the various local minima and maxima of "fast_acos(x) - cosf(x)" and verify that
    // fast_acos is always within "kTolerance" degrees of the expected answer.
    vec<N> err_;
    for (int iter = 0; iter < 10; ++iter) {
        // Run our fast inverse cosine approximation.
        vec<N> fast_acos_x = fast_acos(x);

        // Find d/dx(error)
        //    = d/dx(fast_acos(x) - acos(x))
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
            SkDebugf("err=%f  err'=%f  err''=%f\n",
                     x[j], fast_acos_x[j], SkRadiansToDegrees(fast_acos_x[j] - acosf(x[j])),
                     SkRadiansToDegrees(err_[j]), SkRadiansToDegrees(err__[j]));
#endif
            if (!check_fast_acos(r, x[j], fast_acos_x[j])) {
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

    // Now walk around the roots in case nearby values have worse error due to fast_rcp precision.
    for (auto incr = skvx::if_then_else((x > 0), ivec<N>(1), ivec<N>(-1));
         skvx::all(x <= 1);
         x = skvx::bit_pun<vec<N>>(skvx::bit_pun<ivec<N>>(x) + incr)) {
        vec<N> fast_acos_x = fast_acos(x);
        for (int j = 0; j < N; ++j) {
            if (!check_fast_acos(r, x[j], fast_acos_x[j])) {
                return;
            }
        }
    }

#if 0
    // Brute-force approach: test every possible float inside 0..1. This is not necessary to verify
    // accuracy of the polynomial approximation, but it can settle concerns about fast_rcp
    // precision.
    for (float4 x = skvx::bit_pun<float4>(skvx::bit_pun<ivec<4>>(float4(0)) + ivec<4>{0,1,2,3});
         skvx::all(x <= 1);
         x = skvx::bit_pun<float4>(skvx::bit_pun<ivec<4>>(x) + 1)) {
        float4 fast_acos_x = fast_acos(x);
        for (int j = 0; j < 4; ++j) {
            if (!check_fast_acos(r, x[j], fast_acos_x[j])) {
                break;
            }
        }
    }
#endif
}

static bool check_fast_angle_between_vectors(skiatest::Reporter* r, SkVector a, SkVector b,
                                            float fastTheta) {
    float expectedTheta = SkMeasureAngleBetweenVectors(a, b);
    float error = expectedTheta - fastTheta;
    if (!(fabsf(error) <= GRVX_FAST_ACOS_MAX_ERROR)) {
        ERRORF(r, "Larger-than-expected error from grvx::fast_angle_between_vectors\n"
                  "  a=              {%f, %f}\n"
                  "  b=              {%f, %f}\n"
                  "  fastTheta=      %f  (%f degrees\n"
                  "  expectedTheta=  %f  (%f degrees)\n"
                  "  error=          %f  (%f degrees)\n"
                  "  tolerance=      %f  (%f degrees)\n\n",
                  a.fX, a.fY, b.fX, b.fY, fastTheta, SkRadiansToDegrees(fastTheta), expectedTheta,
                  SkRadiansToDegrees(expectedTheta), error, SkRadiansToDegrees(error),
                  GRVX_FAST_ACOS_MAX_ERROR, SkRadiansToDegrees(GRVX_FAST_ACOS_MAX_ERROR));
        return false;
    }
    return true;
}

static bool check_fast_angle_between_vectors(skiatest::Reporter* r, SkVector a, SkVector b) {
    float fastTheta = grvx::fast_angle_between_vectors<1>(a.fX, a.fY, b.fX, b.fY).val;
    return check_fast_angle_between_vectors(r, a, b, fastTheta);
}

DEF_TEST(grvx_fast_angle_between_vectors, r) {
    // Test when a and/or b are zero.
    REPORTER_ASSERT(r, SkScalarNearlyZero(grvx::fast_angle_between_vectors<1>(0,0,0,0).val));
    REPORTER_ASSERT(r, SkScalarNearlyZero(grvx::fast_angle_between_vectors<1>(1,1,0,0).val));
    REPORTER_ASSERT(r, SkScalarNearlyZero(grvx::fast_angle_between_vectors<1>(0,0,1,1).val));
    check_fast_angle_between_vectors(r, {0,0}, {0,0});
    check_fast_angle_between_vectors(r, {1,1}, {0,0});
    check_fast_angle_between_vectors(r, {0,0}, {1,1});

    // Test infinities.
    REPORTER_ASSERT(r, SkScalarNearlyZero(grvx::fast_angle_between_vectors<1>(
            std::numeric_limits<float>::infinity(),1,2,3).val));
    check_fast_angle_between_vectors(r, {std::numeric_limits<float>::infinity(),1}, {2,3});
    check_fast_angle_between_vectors(r, {0,-std::numeric_limits<float>::infinity()}, {2,3});
    check_fast_angle_between_vectors(r, {0,1}, {std::numeric_limits<float>::infinity(),3});
    check_fast_angle_between_vectors(r, {0,1}, {2,-std::numeric_limits<float>::infinity()});

    // Test NaNs.
    REPORTER_ASSERT(r, SkScalarNearlyZero(grvx::fast_angle_between_vectors<1>(
            std::numeric_limits<float>::quiet_NaN(),1,2,3).val));
    check_fast_angle_between_vectors(r, {std::numeric_limits<float>::quiet_NaN(),1}, {2,3});
    check_fast_angle_between_vectors(r, {0,std::numeric_limits<float>::quiet_NaN()}, {2,3});
    check_fast_angle_between_vectors(r, {0,1}, {std::numeric_limits<float>::quiet_NaN(),3});
    check_fast_angle_between_vectors(r, {0,1}, {2,std::numeric_limits<float>::quiet_NaN()});

    // Test demorms.
    // NOTE: there isn't a floating point value large enough to multiply a denormalized value to 1,
    // but these should behave the same as SkMeasureAngleBetweenVectors.
    float epsilon = std::numeric_limits<float>::denorm_min();
    REPORTER_ASSERT(r, SkScalarNearlyZero(grvx::fast_angle_between_vectors<1>(
            epsilon, epsilon, epsilon, epsilon).val));
    check_fast_angle_between_vectors(r, {epsilon, epsilon}, {epsilon, epsilon});
    check_fast_angle_between_vectors(r, {epsilon, epsilon}, {-epsilon, -epsilon});
    check_fast_angle_between_vectors(r, {epsilon, -epsilon*2}, {-epsilon*3, epsilon*4});

    // Test random floats of all types.
    int4 mantissas = {0,0,0,0};
    int4 exp = int4{126, 127, 128, 129};
    for (int i = 0; i < (1 << 12); ++i) {
        int a=exp[0], b=exp[1], c=exp[2], d=exp[3];
        // Run two tests where both components of both vectors have the same exponent, one where
        // both components of a given vector have the same exponent, and one where all components of
        // all vectors have different exponents.
        int4 x0exp = int4{a,b,a,a} << 23;
        int4 y0exp = int4{a,b,a,b} << 23;
        int4 x1exp = int4{a,b,b,c} << 23;
        int4 y1exp = int4{a,b,b,d} << 23;
        int4 signs = int4{i<<31, i<<30, i<<29, i<<28} & (1<<31);
        float4 x0 = bit_pun<float4>(signs | x0exp | mantissas[0]);
        float4 y0 = bit_pun<float4>(signs | y0exp | mantissas[1]);
        float4 x1 = bit_pun<float4>(signs | x1exp | mantissas[2]);
        float4 y1 = bit_pun<float4>(signs | y1exp | mantissas[3]);
        float4 rads = fast_angle_between_vectors(x0, y0, x1, y1);
        for (int j = 0; j < 4; ++j) {
            if (!check_fast_angle_between_vectors(r, {x0[j], y0[j]}, {x1[j], y1[j]}, rads[j])) {
                return;
            }
        }
        // Adding primes makes sure we test every value before we repeat.
        mantissas = (mantissas + int4{123456791, 201345691, 198765433, 156789029}) & ((1<<23) - 1);
        exp = (exp + int4{79, 83, 199, 7}) & 0xff;
    }
}

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
