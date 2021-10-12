/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkVx.h"
#include "tests/Test.h"
#include <numeric>

using float2 = skvx::Vec<2,float>;
using float4 = skvx::Vec<4,float>;
using float8 = skvx::Vec<8,float>;

using double2 = skvx::Vec<2,double>;
using double4 = skvx::Vec<4,double>;
using double8 = skvx::Vec<8,double>;

using byte2  = skvx::Vec< 2,uint8_t>;
using byte4  = skvx::Vec< 4,uint8_t>;
using byte8  = skvx::Vec< 8,uint8_t>;
using byte16 = skvx::Vec<16,uint8_t>;

using int2 = skvx::Vec<2,int32_t>;
using int4 = skvx::Vec<4,int32_t>;
using int8 = skvx::Vec<8,int32_t>;

using uint2 = skvx::Vec<2,uint32_t>;
using uint4 = skvx::Vec<4,uint32_t>;
using uint8 = skvx::Vec<8,uint32_t>;

using long2 = skvx::Vec<2,int64_t>;
using long4 = skvx::Vec<4,int64_t>;
using long8 = skvx::Vec<8,int64_t>;

DEF_TEST(SkVx, r) {
    static_assert(sizeof(float2) ==  8, "");
    static_assert(sizeof(float4) == 16, "");
    static_assert(sizeof(float8) == 32, "");

    static_assert(sizeof(byte2) == 2, "");
    static_assert(sizeof(byte4) == 4, "");
    static_assert(sizeof(byte8) == 8, "");

    {
        int4 mask = float4{1,2,3,4} < float4{1,2,4,8};
        REPORTER_ASSERT(r, mask[0] == int32_t( 0));
        REPORTER_ASSERT(r, mask[1] == int32_t( 0));
        REPORTER_ASSERT(r, mask[2] == int32_t(-1));
        REPORTER_ASSERT(r, mask[3] == int32_t(-1));

        REPORTER_ASSERT(r,  any(mask));
        REPORTER_ASSERT(r, !all(mask));
    }

    {
        long4 mask = double4{1,2,3,4} < double4{1,2,4,8};
        REPORTER_ASSERT(r, mask[0] == int64_t( 0));
        REPORTER_ASSERT(r, mask[1] == int64_t( 0));
        REPORTER_ASSERT(r, mask[2] == int64_t(-1));
        REPORTER_ASSERT(r, mask[3] == int64_t(-1));

        REPORTER_ASSERT(r,  any(mask));
        REPORTER_ASSERT(r, !all(mask));
    }

    REPORTER_ASSERT(r, min(float4{1,2,3,4}) == 1);
    REPORTER_ASSERT(r, max(float4{1,2,3,4}) == 4);

    REPORTER_ASSERT(r, all(int4{1,2,3,4,5} == int4{1,2,3,4}));
    REPORTER_ASSERT(r, all(int4{1,2,3,4}   == int4{1,2,3,4}));
    REPORTER_ASSERT(r, all(int4{1,2,3}     == int4{1,2,3,0}));
    REPORTER_ASSERT(r, all(int4{1,2}       == int4{1,2,0,0}));
    REPORTER_ASSERT(r, all(int4{1}         == int4{1,0,0,0}));
    REPORTER_ASSERT(r, all(int4(1)         == int4{1,1,1,1}));
    REPORTER_ASSERT(r, all(int4{}          == int4{0,0,0,0}));
    REPORTER_ASSERT(r, all(int4()          == int4{0,0,0,0}));

    REPORTER_ASSERT(r, all(int4{1,2,2,1} == min(int4{1,2,3,4}, int4{4,3,2,1})));
    REPORTER_ASSERT(r, all(int4{4,3,3,4} == max(int4{1,2,3,4}, int4{4,3,2,1})));

    REPORTER_ASSERT(r, all(if_then_else(float4{1,2,3,2} <= float4{2,2,2,2}, float4(42), float4(47))
                           == float4{42,42,47,42}));

    REPORTER_ASSERT(r, all(floor(float4{-1.5f,1.5f,1.0f,-1.0f}) == float4{-2.0f,1.0f,1.0f,-1.0f}));
    REPORTER_ASSERT(r, all( ceil(float4{-1.5f,1.5f,1.0f,-1.0f}) == float4{-1.0f,2.0f,1.0f,-1.0f}));
    REPORTER_ASSERT(r, all(trunc(float4{-1.5f,1.5f,1.0f,-1.0f}) == float4{-1.0f,1.0f,1.0f,-1.0f}));
    REPORTER_ASSERT(r, all(round(float4{-1.5f,1.5f,1.0f,-1.0f}) == float4{-2.0f,2.0f,1.0f,-1.0f}));


    REPORTER_ASSERT(r, all(abs(float4{-2,-1,0,1}) == float4{2,1,0,1}));

    // TODO(mtklein): these tests could be made less loose.
    REPORTER_ASSERT(r, all( sqrt(float4{2,3,4,5}) < float4{2,2,3,3}));
    REPORTER_ASSERT(r, all( sqrt(float2{2,3}) < float2{2,2}));

    REPORTER_ASSERT(r, all(skvx::cast<int>(float4{-1.5f,0.5f,1.0f,1.5f}) == int4{-1,0,1,1}));

    float buf[] = {1,2,3,4,5,6};
    REPORTER_ASSERT(r, all(float4::Load(buf) == float4{1,2,3,4}));
    float4{2,3,4,5}.store(buf);
    REPORTER_ASSERT(r, buf[0] == 2
                    && buf[1] == 3
                    && buf[2] == 4
                    && buf[3] == 5
                    && buf[4] == 5
                    && buf[5] == 6);
    REPORTER_ASSERT(r, all(float4::Load(buf+0) == float4{2,3,4,5}));
    REPORTER_ASSERT(r, all(float4::Load(buf+2) == float4{4,5,5,6}));

    REPORTER_ASSERT(r, all(skvx::shuffle<2,1,0,3>        (float4{1,2,3,4}) == float4{3,2,1,4}));
    REPORTER_ASSERT(r, all(skvx::shuffle<2,1>            (float4{1,2,3,4}) == float2{3,2}));
    REPORTER_ASSERT(r, all(skvx::shuffle<3,3,3,3>        (float4{1,2,3,4}) == float4{4,4,4,4}));
    REPORTER_ASSERT(r, all(skvx::shuffle<2,1,2,1,2,1,2,1>(float4{1,2,3,4})
                           == float8{3,2,3,2,3,2,3,2}));

    // Test that mixed types can be used where they make sense.  Mostly about ergonomics.
    REPORTER_ASSERT(r, all(float4{1,2,3,4} < 5));
    REPORTER_ASSERT(r, all( byte4{1,2,3,4} < 5));
    REPORTER_ASSERT(r, all(  int4{1,2,3,4} < 5.0f));
    float4 five = 5;
    REPORTER_ASSERT(r, all(five == 5.0f));
    REPORTER_ASSERT(r, all(five == 5));

    REPORTER_ASSERT(r, all(max(2, min(float4{1,2,3,4}, 3)) == float4{2,2,3,3}));

    for (int x = 0; x < 256; x++)
    for (int y = 0; y < 256; y++) {
        uint8_t want = (uint8_t)( 255*(x/255.0 * y/255.0) + 0.5 );

        {
            uint8_t got = skvx::div255(skvx::Vec<8, uint16_t>(x) *
                                       skvx::Vec<8, uint16_t>(y) )[0];
            REPORTER_ASSERT(r, got == want);
        }

        {
            uint8_t got = skvx::approx_scale(skvx::Vec<8,uint8_t>(x),
                                             skvx::Vec<8,uint8_t>(y))[0];

            REPORTER_ASSERT(r, got == want-1 ||
                               got == want   ||
                               got == want+1);
            if (x == 0 || y == 0 || x == 255 || y == 255) {
                REPORTER_ASSERT(r, got == want);
            }
        }
    }

    for (int x = 0; x < 256; x++)
    for (int y = 0; y < 256; y++) {
        uint16_t xy = x*y;

        // Make sure to cover implementation cases N=8, N<8, and N>8.
        REPORTER_ASSERT(r, all(mull(byte2 (x), byte2 (y)) == xy));
        REPORTER_ASSERT(r, all(mull(byte4 (x), byte4 (y)) == xy));
        REPORTER_ASSERT(r, all(mull(byte8 (x), byte8 (y)) == xy));
        REPORTER_ASSERT(r, all(mull(byte16(x), byte16(y)) == xy));
    }

    {
        // Intentionally not testing -0, as we don't care if it's 0x0000 or 0x8000.
        float8 fs = {+0.0f,+0.5f,+1.0f,+2.0f,
                     -4.0f,-0.5f,-1.0f,-2.0f};
        skvx::Vec<8,uint16_t> hs = {0x0000,0x3800,0x3c00,0x4000,
                                    0xc400,0xb800,0xbc00,0xc000};
        REPORTER_ASSERT(r, all(skvx::  to_half(fs) == hs));
        REPORTER_ASSERT(r, all(skvx::from_half(hs) == fs));
    }
}

DEF_TEST(SkVx_xy, r) {
    float2 f = float2(1,2);
    REPORTER_ASSERT(r, all(f == float2{1,2}));
    REPORTER_ASSERT(r, f.x() == 1);
    REPORTER_ASSERT(r, f.y() == 2);
    f.y() = 9;
    REPORTER_ASSERT(r, all(f == float2{1,9}));
    f.x() = 0;
    REPORTER_ASSERT(r, all(f == float2(0,9)));
    f[0] = 8;
    REPORTER_ASSERT(r, f.x() == 8);
    f[1] = 6;
    REPORTER_ASSERT(r, f.y() == 6);
    REPORTER_ASSERT(r, all(f == float2(8,6)));
    f = f.yx();
    REPORTER_ASSERT(r, all(f == float2(6,8)));
    REPORTER_ASSERT(r, skvx::bit_pun<SkPoint>(f) == SkPoint::Make(6,8));
    SkPoint p;
    f.store(&p);
    REPORTER_ASSERT(r, p == SkPoint::Make(6,8));
    f.yx().store(&p);
    REPORTER_ASSERT(r, p == SkPoint::Make(8,6));
    REPORTER_ASSERT(r, all(f.xyxy() == float4(6,8,6,8)));
    REPORTER_ASSERT(r, all(f.xyxy() == float4(f,f)));
    REPORTER_ASSERT(r, all(skvx::join(f,f) == f.xyxy()));
    REPORTER_ASSERT(r, all(skvx::join(f.yx(),f) == float4(f.y(),f.x(),f)));
    REPORTER_ASSERT(r, all(skvx::join(f.yx(),f) == float4(f.yx(),f.x(),f.y())));
    REPORTER_ASSERT(r, all(skvx::join(f,f.yx()) == float4(f.x(),f.y(),f.yx())));
    REPORTER_ASSERT(r, all(skvx::join(f.yx(),f.yx()) == float4(f.yx(),f.yx())));
}

DEF_TEST(SkVx_xyzw, r) {
    float4 f = float4{1,2,3,4};
    REPORTER_ASSERT(r, all(f == float4(1,2,3,4)));
    REPORTER_ASSERT(r, all(f == float4(1,2,float2(3,4))));
    REPORTER_ASSERT(r, all(f == float4(float2(1,2),3,4)));
    REPORTER_ASSERT(r, all(f == float4(float2(1,2),float2(3,4))));
    f.xy() = float2(9,8);
    REPORTER_ASSERT(r, all(f == float4(9,8,3,4)));
    f.zw().x() = 7;
    f.zw().y() = 6;
    REPORTER_ASSERT(r, all(f == float4(9,8,7,6)));
    f.x() = 5;
    f.y() = 4;
    f.z() = 3;
    f.w() = 2;
    REPORTER_ASSERT(r, all(f == float4(5,4,3,2)));
    f[0] = 0;
    REPORTER_ASSERT(r, f.x() == 0);
    f[1] = 1;
    REPORTER_ASSERT(r, f.y() == 1);
    f[2] = 2;
    REPORTER_ASSERT(r, f.z() == 2);
    f[3] = 3;
    REPORTER_ASSERT(r, f.w() == 3);
    REPORTER_ASSERT(r, skvx::all(f.xy() == float2(0,1)));
    REPORTER_ASSERT(r, skvx::all(f.zw() == float2{2,3}));
    REPORTER_ASSERT(r, all(f == float4(0,1,2,3)));
    REPORTER_ASSERT(r, all(f.yxwz().lo == skvx::shuffle<1,0>(f)));
    REPORTER_ASSERT(r, all(f.yxwz().hi == skvx::shuffle<3,2>(f)));
    REPORTER_ASSERT(r, all(f.zwxy().lo.lo == f.z()));
    REPORTER_ASSERT(r, all(f.zwxy().lo.hi == f.w()));
    REPORTER_ASSERT(r, all(f.zwxy().hi.lo == f.x()));
    REPORTER_ASSERT(r, all(f.zwxy().hi.hi == f.y()));
    REPORTER_ASSERT(r, f.yxwz().lo.lo.val == f.y());
    REPORTER_ASSERT(r, f.yxwz().lo.hi.val == f.x());
    REPORTER_ASSERT(r, f.yxwz().hi.lo.val == f.w());
    REPORTER_ASSERT(r, f.yxwz().hi.hi.val == f.z());

    REPORTER_ASSERT(r, all(skvx::naive_if_then_else(int2(0,~0),
                                                    skvx::shuffle<3,2>(float4(0,1,2,3)),
                                                    float4(4,5,6,7).xy()) == float2(4,2)));
    REPORTER_ASSERT(r, all(skvx::if_then_else(int2(0,~0),
                                              skvx::shuffle<3,2>(float4(0,1,2,3)),
                                              float4(4,5,6,7).xy()) == float2(4,2)));
    REPORTER_ASSERT(r, all(skvx::naive_if_then_else(int2(0,~0).xyxy(),
                                                    float4(0,1,2,3).zwxy(),
                                                    float4(4,5,6,7)) == float4(4,3,6,1)));
    REPORTER_ASSERT(r, all(skvx::if_then_else(int2(0,~0).xyxy(),
                                              float4(0,1,2,3).zwxy(),
                                              float4(4,5,6,7)) == float4(4,3,6,1)));

    REPORTER_ASSERT(r, all(skvx::pin(float4(0,1,2,3).yxwz(),
                                     float2(1).xyxy(),
                                     float2(2).xyxy()) == float4(1,1,2,2)));
}

static bool check_approx_acos(skiatest::Reporter* r, float x, float approx_acos_x) {
    float acosf_x = acosf(x);
    float error = acosf_x - approx_acos_x;
    if (!(fabsf(error) <= SKVX_APPROX_ACOS_MAX_ERROR)) {
        ERRORF(r, "Larger-than-expected error from skvx::approx_acos\n"
                  "  x=              %f\n"
                  "  approx_acos_x=  %f  (%f degrees\n"
                  "  acosf_x=        %f  (%f degrees\n"
                  "  error=          %f  (%f degrees)\n"
                  "  tolerance=      %f  (%f degrees)\n\n",
                  x, approx_acos_x, SkRadiansToDegrees(approx_acos_x), acosf_x,
                  SkRadiansToDegrees(acosf_x), error, SkRadiansToDegrees(error),
                  SKVX_APPROX_ACOS_MAX_ERROR, SkRadiansToDegrees(SKVX_APPROX_ACOS_MAX_ERROR));
        return false;
    }
    return true;
}

DEF_TEST(SkVx_approx_acos, r) {
    float4 boundaries = skvx::approx_acos(float4{-1, 0, 1, 0});
    check_approx_acos(r, -1, boundaries[0]);
    check_approx_acos(r, 0, boundaries[1]);
    check_approx_acos(r, +1, boundaries[2]);

    // Select a distribution of starting points around which to begin testing approx_acos. These
    // fall roughly around the known minimum and maximum errors. No need to include -1, 0, or 1
    // since those were just tested above. (Those are tricky because 0 is an inflection and the
    // derivative is infinite at 1 and -1.)
    float8 x = {-.99f, -.8f, -.4f, -.2f, .2f, .4f, .8f, .99f};

    // Converge at the various local minima and maxima of "approx_acos(x) - cosf(x)" and verify that
    // approx_acos is always within "kTolerance" degrees of the expected answer.
    float8 err_;
    for (int iter = 0; iter < 10; ++iter) {
        // Run our approximate inverse cosine approximation.
        auto approx_acos_x = skvx::approx_acos(x);

        // Find d/dx(error)
        //    = d/dx(approx_acos(x) - acos(x))
        //    = (f'g - fg')/gg + 1/sqrt(1 - x^2), [where f = bx^3 + ax, g = dx^4 + cx^2 + 1]
        float8 xx = x*x;
        float8 a = -0.939115566365855f;
        float8 b =  0.9217841528914573f;
        float8 c = -1.2845906244690837f;
        float8 d =  0.295624144969963174f;
        float8 f = (b*xx + a)*x;
        float8 f_ = 3*b*xx + a;
        float8 g = (d*xx + c)*xx + 1;
        float8 g_ = (4*d*xx + 2*c)*x;
        float8 gg = g*g;
        float8 q = skvx::sqrt(1 - xx);
        err_ = (f_*g - f*g_)/gg + 1/q;

        // Find d^2/dx^2(error)
        //    = ((f''g - fg'')g^2 - (f'g - fg')2gg') / g^4 + x(1 - x^2)^(-3/2)
        //    = ((f''g - fg'')g - (f'g - fg')2g') / g^3 + x(1 - x^2)^(-3/2)
        float8 f__ = 6*b*x;
        float8 g__ = 12*d*xx + 2*c;
        float8 err__ = ((f__*g - f*g__)*g - (f_*g - f*g_)*2*g_) / (gg*g) + x/((1 - xx)*q);

#if 0
        SkDebugf("\n\niter %i\n", iter);
#endif
        // Ensure each lane's approximation is within maximum error.
        for (int j = 0; j < 8; ++j) {
#if 0
            SkDebugf("x=%f  err=%f  err'=%f  err''=%f\n",
                     x[j], SkRadiansToDegrees(skvx::approx_acos_x[j] - acosf(x[j])),
                     SkRadiansToDegrees(err_[j]), SkRadiansToDegrees(err__[j]));
#endif
            if (!check_approx_acos(r, x[j], approx_acos_x[j])) {
                return;
            }
        }

        // Use Newton's method to update the x values to locations closer to their local minimum or
        // maximum. (This is where d/dx(error) == 0.)
        x -= err_/err__;
        x = skvx::pin<8,float>(x, -.99f, .99f);
    }

    // Ensure each lane converged to a local minimum or maximum.
    for (int j = 0; j < 8; ++j) {
        REPORTER_ASSERT(r, SkScalarNearlyZero(err_[j]));
    }

    // Make sure we found all the actual known locations of local min/max error.
    for (float knownRoot : {-0.983536f, -0.867381f, -0.410923f, 0.410923f, 0.867381f, 0.983536f}) {
        REPORTER_ASSERT(r, skvx::any(skvx::abs(x - knownRoot) < SK_ScalarNearlyZero));
    }
}

template<int N, typename T> void check_strided_loads(skiatest::Reporter* r) {
    using Vec = skvx::Vec<N,T>;
    T values[N*4];
    std::iota(values, values + N*4, 0);
    Vec a, b, c, d;
    skvx::strided_load2(values, a, b);
    for (int i = 0; i < N; ++i) {
        REPORTER_ASSERT(r, a[i] == values[i*2]);
        REPORTER_ASSERT(r, b[i] == values[i*2 + 1]);
    }
    skvx::strided_load4(values, a, b, c, d);
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

DEF_TEST(SkVx_strided_loads, r) {
    check_strided_loads<uint32_t>(r);
    check_strided_loads<uint16_t>(r);
    check_strided_loads<uint8_t>(r);
    check_strided_loads<int32_t>(r);
    check_strided_loads<int16_t>(r);
    check_strided_loads<int8_t>(r);
    check_strided_loads<float>(r);
}
