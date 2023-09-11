/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPoint.h"
#include "src/base/SkRandom.h"
#include "src/base/SkUtils.h"
#include "src/base/SkVx.h"
#include "tests/Test.h"

#include <numeric>

namespace skvx {

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

    {
        // Tests that any/all work with non-zero values, not just full bit lanes.
        REPORTER_ASSERT(r,  all(int4{1,2,3,4}));
        REPORTER_ASSERT(r, !all(int4{1,2,3}));
        REPORTER_ASSERT(r,  any(int4{1,2}));
        REPORTER_ASSERT(r, !any(int4{}));
    }

    REPORTER_ASSERT(r, min(float4{1,2,3,4}) == 1);
    REPORTER_ASSERT(r, max(float4{1,2,3,4}) == 4);

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

    REPORTER_ASSERT(r, all(cast<int>(float4{-1.5f,0.5f,1.0f,1.5f}) == int4{-1,0,1,1}));

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

    REPORTER_ASSERT(r, all(shuffle<2,1,0,3>        (float4{1,2,3,4}) == float4{3,2,1,4}));
    REPORTER_ASSERT(r, all(shuffle<2,1>            (float4{1,2,3,4}) == float2{3,2}));
    REPORTER_ASSERT(r, all(shuffle<3,3,3,3>        (float4{1,2,3,4}) == float4{4,4,4,4}));
    REPORTER_ASSERT(r, all(shuffle<2,1,2,1,2,1,2,1>(float4{1,2,3,4})
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
            uint8_t got = div255(Vec<8, uint16_t>(x) * Vec<8, uint16_t>(y) )[0];
            REPORTER_ASSERT(r, got == want);
        }

        {
            uint8_t got = approx_scale(Vec<8,uint8_t>(x), Vec<8,uint8_t>(y))[0];

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
        Vec<8,uint16_t> hs = {0x0000,0x3800,0x3c00,0x4000,
                              0xc400,0xb800,0xbc00,0xc000};
        REPORTER_ASSERT(r, all(  to_half(fs) == hs));
        REPORTER_ASSERT(r, all(from_half(hs) == fs));
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
    REPORTER_ASSERT(r, sk_bit_cast<SkPoint>(f) == SkPoint::Make(6,8));
    SkPoint p;
    f.store(&p);
    REPORTER_ASSERT(r, p == SkPoint::Make(6,8));
    f.yx().store(&p);
    REPORTER_ASSERT(r, p == SkPoint::Make(8,6));
    REPORTER_ASSERT(r, all(f.xyxy() == float4(6,8,6,8)));
    REPORTER_ASSERT(r, all(f.xyxy() == float4(f,f)));
    REPORTER_ASSERT(r, all(join(f,f) == f.xyxy()));
    REPORTER_ASSERT(r, all(join(f.yx(),f) == float4(f.y(),f.x(),f)));
    REPORTER_ASSERT(r, all(join(f.yx(),f) == float4(f.yx(),f.x(),f.y())));
    REPORTER_ASSERT(r, all(join(f,f.yx()) == float4(f.x(),f.y(),f.yx())));
    REPORTER_ASSERT(r, all(join(f.yx(),f.yx()) == float4(f.yx(),f.yx())));
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
    REPORTER_ASSERT(r, all(f.xy() == float2(0,1)));
    REPORTER_ASSERT(r, all(f.zw() == float2{2,3}));
    REPORTER_ASSERT(r, all(f == float4(0,1,2,3)));
    REPORTER_ASSERT(r, all(f.yxwz().lo == shuffle<1,0>(f)));
    REPORTER_ASSERT(r, all(f.yxwz().hi == shuffle<3,2>(f)));
    REPORTER_ASSERT(r, all(f.zwxy().lo.lo == f.z()));
    REPORTER_ASSERT(r, all(f.zwxy().lo.hi == f.w()));
    REPORTER_ASSERT(r, all(f.zwxy().hi.lo == f.x()));
    REPORTER_ASSERT(r, all(f.zwxy().hi.hi == f.y()));
    REPORTER_ASSERT(r, f.yxwz().lo.lo.val == f.y());
    REPORTER_ASSERT(r, f.yxwz().lo.hi.val == f.x());
    REPORTER_ASSERT(r, f.yxwz().hi.lo.val == f.w());
    REPORTER_ASSERT(r, f.yxwz().hi.hi.val == f.z());

    REPORTER_ASSERT(r, all(naive_if_then_else(int2(0,~0),
                                              shuffle<3,2>(float4(0,1,2,3)),
                                              float4(4,5,6,7).xy()) == float2(4,2)));
    REPORTER_ASSERT(r, all(if_then_else(int2(0,~0),
                                        shuffle<3,2>(float4(0,1,2,3)),
                                        float4(4,5,6,7).xy()) == float2(4,2)));
    REPORTER_ASSERT(r, all(naive_if_then_else(int2(0,~0).xyxy(),
                                              float4(0,1,2,3).zwxy(),
                                              float4(4,5,6,7)) == float4(4,3,6,1)));
    REPORTER_ASSERT(r, all(if_then_else(int2(0,~0).xyxy(),
                                        float4(0,1,2,3).zwxy(),
                                        float4(4,5,6,7)) == float4(4,3,6,1)));

    REPORTER_ASSERT(r, all(pin(float4(0,1,2,3).yxwz(),
                               float2(1).xyxy(),
                               float2(2).xyxy()) == float4(1,1,2,2)));
}

DEF_TEST(SkVx_cross_dot, r) {
    REPORTER_ASSERT(r, cross(int2{0,1}, int2{0,1}) == 0);
    REPORTER_ASSERT(r, cross(int2{1,0}, int2{1,0}) == 0);
    REPORTER_ASSERT(r, cross(int2{1,1}, int2{1,1}) == 0);
    REPORTER_ASSERT(r, cross(int2{1,1}, int2{1,-1}) == -2);
    REPORTER_ASSERT(r, cross(int2{1,1}, int2{-1,1}) == 2);

    REPORTER_ASSERT(r, dot(int2{0,1}, int2{1,0}) == 0);
    REPORTER_ASSERT(r, dot(int2{1,0}, int2{0,1}) == 0);
    REPORTER_ASSERT(r, dot(int2{1,1}, int2{1,-1}) == 0);
    REPORTER_ASSERT(r, dot(int2{1,1}, int2{1,1}) == 2);
    REPORTER_ASSERT(r, dot(int2{1,1}, int2{-1,-1}) == -2);

    SkRandom rand;
    for (int i = 0; i < 100; ++i) {
        float a=rand.nextRangeF(-1,1), b=rand.nextRangeF(-1,1), c=rand.nextRangeF(-1,1),
              d=rand.nextRangeF(-1,1);
        constexpr static float kTolerance = 1.f / (1 << 20);
        REPORTER_ASSERT(r, SkScalarNearlyEqual(
                cross(float2{a,b}, float2{c,d}), SkPoint::CrossProduct({a,b}, {c,d}), kTolerance));
        REPORTER_ASSERT(r, SkScalarNearlyEqual(
                dot(float2{a,b}, float2{c,d}), SkPoint::DotProduct({a,b}, {c,d}), kTolerance));
    }

    auto assertDoublesEqual = [&](double left, double right) {
        REPORTER_ASSERT(r, SkScalarNearlyEqual(left, right), "%f != %f", left, right);
    };
    assertDoublesEqual(cross(double2{1.2, 3.4}, double2{3.4, -1.2}),          -13.000000);
    assertDoublesEqual(cross(double2{12.34, 5.6}, double2{7.8, -9.0}),       -154.740000);
    assertDoublesEqual(cross(double2{12.34, 5.6}, double2{7.8, 9.012345678}),  67.532346);
}

template<int N, typename T> void check_strided_loads(skiatest::Reporter* r) {
    using Vec = Vec<N,T>;
    T values[N*4];
    std::iota(values, values + N*4, 0);
    Vec a, b, c, d;
    strided_load2(values, a, b);
    for (int i = 0; i < N; ++i) {
        REPORTER_ASSERT(r, a[i] == values[i*2]);
        REPORTER_ASSERT(r, b[i] == values[i*2 + 1]);
    }
    strided_load4(values, a, b, c, d);
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

DEF_TEST(SkVx_ScaledDividerU32, r) {
    static constexpr uint32_t kMax = std::numeric_limits<uint32_t>::max();

    auto errorBounds = [&](uint32_t actual, uint32_t expected) {
        uint32_t lowerLimit = expected == 0 ? 0 : expected - 1,
                 upperLimit = expected == kMax ? kMax : expected + 1;
        return lowerLimit <= actual && actual <= upperLimit;
    };

    auto test = [&](uint32_t denom) {
        // half == 1 so, the max to check is kMax-1
        ScaledDividerU32 d(denom);
        uint32_t maxCheck = static_cast<uint32_t>(
                std::floor((double)(kMax - d.half()) / denom + 0.5));
        REPORTER_ASSERT(r, errorBounds(d.divide((kMax))[0], maxCheck));
        for (uint32_t i = 0; i < kMax - d.half(); i += 65535) {
            uint32_t expected = static_cast<uint32_t>(std::floor((double)i / denom + 0.5));
            auto actual = d.divide(i + d.half());
            if (!errorBounds(actual[0], expected)) {
                SkDebugf("i: %u expected: %u actual: %u\n", i, expected, actual[0]);
            }
            // Make sure all the lanes are the same.
            for (int e = 1; e < 4; e++) {
                SkASSERT(actual[0] == actual[e]);
            }
        }
    };

    test(2);
    test(3);
    test(5);
    test(7);
    test(27);
    test(65'535);
    test(15'485'863);
    test(512'927'377);
}

DEF_TEST(SkVx_saturated_add, r) {
    for (int a = 0; a < (1<<8); a++) {
        for (int b = 0; b < (1<<8); b++) {
            int exact = a+b;
            if (exact > 255) { exact = 255; }
            if (exact <   0) { exact =   0; }

            REPORTER_ASSERT(r, saturated_add(skvx::byte16(a), skvx::byte16(b))[0] == exact);
        }
    }
}

DEF_TEST(SkVx_length, r) {
    auto assertFloatsEqual = [&](float left, float right) {
        REPORTER_ASSERT(r, SkScalarNearlyEqual(left, right), "%f != %f", left, right);
    };
    auto assertDoublesEqual = [&](double left, double right) {
        REPORTER_ASSERT(r, SkScalarNearlyEqual(left, right), "%f != %f", left, right);
    };

    assertFloatsEqual(length(float2{0, 1}),       1.000000f);
    assertFloatsEqual(length(float2{2, 0}),       2.000000f);
    assertFloatsEqual(length(float2{3, 4}),       5.000000f);
    assertFloatsEqual(length(float2{1, 1}),       1.414214f);
    assertFloatsEqual(length(float2{2.5f, 2.5f}), 3.535534f);
    assertFloatsEqual(length(float4{1, 2, 3, 4}), 5.477226f);

    assertDoublesEqual(length(double2{2.5, 2.5}),           3.535534);
    assertDoublesEqual(length(double4{1.5, 2.5, 3.5, 4.5}), 6.403124);
}

DEF_TEST(SkVx_normalize, r) {
    auto assertFloatsEqual = [&](float left, float right) {
        REPORTER_ASSERT(r, SkScalarNearlyEqual(left, right), "%f != %f", left, right);
    };
    auto assertDoublesEqual = [&](double left, double right) {
        REPORTER_ASSERT(r, SkScalarNearlyEqual(left, right), "%f != %f", left, right);
    };

    skvx::float2 twoFloats = normalize(skvx::float2{1.2f, 3.4f});
    assertFloatsEqual(twoFloats[0], 0.332820f);
    assertFloatsEqual(twoFloats[1], 0.942990f);

    skvx::double2 twoDoubles = normalize(skvx::double2{2.3, -4.5});
    assertDoublesEqual(twoDoubles[0],  0.455111);
    assertDoublesEqual(twoDoubles[1], -0.890435);

    skvx::double4 fourDoubles = normalize(skvx::double4{1.2, 3.4, 5.6, 7.8});
    assertDoublesEqual(fourDoubles[0],  0.116997);
    assertDoublesEqual(fourDoubles[1],  0.331490);
    assertDoublesEqual(fourDoubles[2],  0.545984);
    assertDoublesEqual(fourDoubles[3],  0.760478);
}

DEF_TEST(SkVx_normalize_infinity_and_nan, r) {
    skvx::float2 zeroLenVec = normalize(skvx::float2{0, 0});
    REPORTER_ASSERT(r, std::isnan(zeroLenVec[0]), "%f is not nan", zeroLenVec[0]);
    REPORTER_ASSERT(r, std::isnan(zeroLenVec[1]), "%f is not nan", zeroLenVec[1]);
    REPORTER_ASSERT(r, !isfinite(zeroLenVec));

    skvx::float2 tooBigVec = normalize(skvx::float2{std::numeric_limits<float>::max(),
                                                    std::numeric_limits<float>::max()});
    REPORTER_ASSERT(r, tooBigVec[0] == 0, "%f != 0", tooBigVec[0]);
    REPORTER_ASSERT(r, tooBigVec[1] == 0, "%f != 0", tooBigVec[1]);

    skvx::double2 tooBigVecD = normalize(skvx::double2{std::numeric_limits<double>::max(),
                                                       std::numeric_limits<double>::max()});
    REPORTER_ASSERT(r, tooBigVecD[0] == 0, "%f != 0", tooBigVecD[0]);
    REPORTER_ASSERT(r, tooBigVecD[1] == 0, "%f != 0", tooBigVecD[1]);
}

DEF_TEST(SkVx_isfinite, r) {
    REPORTER_ASSERT(r, isfinite(skvx::float2{0, 0}));
    REPORTER_ASSERT(r, isfinite(skvx::double4{1.2, 3.4, 5.6, 7.8}));
    REPORTER_ASSERT(r, isfinite(skvx::float8{8, 7, 6, 5, 4, 3, 2, 1}));

    REPORTER_ASSERT(r, !isfinite(skvx::float2{0, NAN}));
    REPORTER_ASSERT(r, !isfinite(skvx::float2{INFINITY, 10}));
    REPORTER_ASSERT(r, !isfinite(skvx::float2{NAN, INFINITY}));

    for (int i = 0; i < 4; i++) {
        auto v = skvx::double4{4, 3, 2, 1};
        v[i] = INFINITY;
        REPORTER_ASSERT(r, !isfinite(v), "index %d INFINITY", i);
        v[i] = NAN;
        REPORTER_ASSERT(r, !isfinite(v), "index %d NAN", i);
    }

    for (int i = 0; i < 8; i++) {
        auto v = skvx::float8{8, 7, 6, 5, 4, 3, 2, 1};
        v[i] = INFINITY;
        REPORTER_ASSERT(r, !isfinite(v), "index %d INFINITY", i);
        v[i] = NAN;
        REPORTER_ASSERT(r, !isfinite(v), "index %d NAN", i);
    }
}

}  // namespace skvx
