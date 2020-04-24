/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkNx.h"
#include "include/utils/SkRandom.h"
#include "src/core/Sk4px.h"
#include "tests/Test.h"

template <int N>
static void test_Nf(skiatest::Reporter* r) {

    auto assert_nearly_eq = [&](float eps, const SkNx<N, float>& v,
                                float a, float b, float c, float d) {
        auto close = [=](float a, float b) { return fabsf(a-b) <= eps; };
        float vals[4];
        v.store(vals);
        bool ok = close(vals[0], a) && close(vals[1], b)
               && close(   v[0], a) && close(   v[1], b);
        REPORTER_ASSERT(r, ok);
        if (N == 4) {
            ok = close(vals[2], c) && close(vals[3], d)
              && close(   v[2], c) && close(   v[3], d);
            REPORTER_ASSERT(r, ok);
        }
    };
    auto assert_eq = [&](const SkNx<N, float>& v, float a, float b, float c, float d) {
        return assert_nearly_eq(0, v, a,b,c,d);
    };

    float vals[] = {3, 4, 5, 6};
    SkNx<N,float> a = SkNx<N,float>::Load(vals),
                  b(a),
                  c = a;
    SkNx<N,float> d;
    d = a;

    assert_eq(a, 3, 4, 5, 6);
    assert_eq(b, 3, 4, 5, 6);
    assert_eq(c, 3, 4, 5, 6);
    assert_eq(d, 3, 4, 5, 6);

    assert_eq(a+b, 6, 8, 10, 12);
    assert_eq(a*b, 9, 16, 25, 36);
    assert_eq(a*b-b, 6, 12, 20, 30);
    assert_eq((a*b).sqrt(), 3, 4, 5, 6);
    assert_eq(a/b, 1, 1, 1, 1);
    assert_eq(SkNx<N,float>(0)-a, -3, -4, -5, -6);

    SkNx<N,float> fours(4);

    assert_eq(fours.sqrt(), 2,2,2,2);
    assert_nearly_eq(0.001f, fours.rsqrt(), 0.5, 0.5, 0.5, 0.5);

    assert_nearly_eq(0.001f, fours.invert(), 0.25, 0.25, 0.25, 0.25);

    assert_eq(SkNx<N,float>::Min(a, fours), 3, 4, 4, 4);
    assert_eq(SkNx<N,float>::Max(a, fours), 4, 4, 5, 6);

    // Test some comparisons.  This is not exhaustive.
    REPORTER_ASSERT(r, (a == b).allTrue());
    REPORTER_ASSERT(r, (a+b == a*b-b).anyTrue());
    REPORTER_ASSERT(r, !(a+b == a*b-b).allTrue());
    REPORTER_ASSERT(r, !(a+b == a*b).anyTrue());
    REPORTER_ASSERT(r, !(a != b).anyTrue());
    REPORTER_ASSERT(r, (a < fours).anyTrue());
    REPORTER_ASSERT(r, (a <= fours).anyTrue());
    REPORTER_ASSERT(r, !(a > fours).allTrue());
    REPORTER_ASSERT(r, !(a >= fours).allTrue());
}

DEF_TEST(SkNf, r) {
    test_Nf<2>(r);
    test_Nf<4>(r);
}

template <int N, typename T>
void test_Ni(skiatest::Reporter* r) {
    auto assert_eq = [&](const SkNx<N,T>& v, T a, T b, T c, T d, T e, T f, T g, T h) {
        T vals[8];
        v.store(vals);

        switch (N) {
          case 8: REPORTER_ASSERT(r, vals[4] == e && vals[5] == f && vals[6] == g && vals[7] == h);
          case 4: REPORTER_ASSERT(r, vals[2] == c && vals[3] == d);
          case 2: REPORTER_ASSERT(r, vals[0] == a && vals[1] == b);
        }
        switch (N) {
          case 8: REPORTER_ASSERT(r, v[4] == e && v[5] == f &&
                                     v[6] == g && v[7] == h);
          case 4: REPORTER_ASSERT(r, v[2] == c && v[3] == d);
          case 2: REPORTER_ASSERT(r, v[0] == a && v[1] == b);
        }
    };

    T vals[] = { 1,2,3,4,5,6,7,8 };
    SkNx<N,T> a = SkNx<N,T>::Load(vals),
              b(a),
              c = a;
    SkNx<N,T> d;
    d = a;

    assert_eq(a, 1,2,3,4,5,6,7,8);
    assert_eq(b, 1,2,3,4,5,6,7,8);
    assert_eq(c, 1,2,3,4,5,6,7,8);
    assert_eq(d, 1,2,3,4,5,6,7,8);

    assert_eq(a+a, 2,4,6,8,10,12,14,16);
    assert_eq(a*a, 1,4,9,16,25,36,49,64);
    assert_eq(a*a-a, 0,2,6,12,20,30,42,56);

    assert_eq(a >> 2, 0,0,0,1,1,1,1,2);
    assert_eq(a << 1, 2,4,6,8,10,12,14,16);

    REPORTER_ASSERT(r, a[1] == 2);
}

DEF_TEST(SkNx, r) {
    test_Ni<2, uint16_t>(r);
    test_Ni<4, uint16_t>(r);
    test_Ni<8, uint16_t>(r);

    test_Ni<2, int>(r);
    test_Ni<4, int>(r);
    test_Ni<8, int>(r);
}

DEF_TEST(SkNi_min_lt, r) {
    // Exhaustively check the 8x8 bit space.
    for (int a = 0; a < (1<<8); a++) {
    for (int b = 0; b < (1<<8); b++) {
        Sk16b aw(a), bw(b);
        REPORTER_ASSERT(r, Sk16b::Min(aw, bw)[0] == SkTMin(a, b));
        REPORTER_ASSERT(r, !(aw < bw)[0] == !(a < b));
    }}

    // Exhausting the 16x16 bit space is kind of slow, so only do that in release builds.
#ifdef SK_DEBUG
    SkRandom rand;
    for (int i = 0; i < (1<<16); i++) {
        uint16_t a = rand.nextU() >> 16,
                 b = rand.nextU() >> 16;
        REPORTER_ASSERT(r, Sk16h::Min(Sk16h(a), Sk16h(b))[0] == SkTMin(a, b));
    }
#else
    for (int a = 0; a < (1<<16); a++) {
    for (int b = 0; b < (1<<16); b++) {
        REPORTER_ASSERT(r, Sk16h::Min(Sk16h(a), Sk16h(b))[0] == SkTMin(a, b));
    }}
#endif
}

DEF_TEST(SkNi_saturatedAdd, r) {
    for (int a = 0; a < (1<<8); a++) {
    for (int b = 0; b < (1<<8); b++) {
        int exact = a+b;
        if (exact > 255) { exact = 255; }
        if (exact <   0) { exact =   0; }

        REPORTER_ASSERT(r, Sk16b(a).saturatedAdd(Sk16b(b))[0] == exact);
    }
    }
}

DEF_TEST(SkNi_mulHi, r) {
    // First 8 primes.
    Sk4u a{ 0x00020000, 0x00030000, 0x00050000, 0x00070000 };
    Sk4u b{ 0x000b0000, 0x000d0000, 0x00110000, 0x00130000 };

    Sk4u q{22, 39, 85, 133};

    Sk4u c = a.mulHi(b);
    REPORTER_ASSERT(r, c[0] == q[0]);
    REPORTER_ASSERT(r, c[1] == q[1]);
    REPORTER_ASSERT(r, c[2] == q[2]);
    REPORTER_ASSERT(r, c[3] == q[3]);
}

DEF_TEST(Sk4px_muldiv255round, r) {
    for (int a = 0; a < (1<<8); a++) {
    for (int b = 0; b < (1<<8); b++) {
        int exact = (a*b+127)/255;

        // Duplicate a and b 16x each.
        Sk4px av = Sk16b(a),
              bv = Sk16b(b);

        // This way should always be exactly correct.
        int correct = (av * bv).div255()[0];
        REPORTER_ASSERT(r, correct == exact);

        // We're a bit more flexible on this method: correct for 0 or 255, otherwise off by <=1.
        int fast = av.approxMulDiv255(bv)[0];
        REPORTER_ASSERT(r, fast-exact >= -1 && fast-exact <= 1);
        if (a == 0 || a == 255 || b == 0 || b == 255) {
            REPORTER_ASSERT(r, fast == exact);
        }
    }
    }
}

DEF_TEST(SkNx_abs, r) {
    auto fs = Sk4f(0.0f, -0.0f, 2.0f, -4.0f).abs();
    REPORTER_ASSERT(r, fs[0] == 0.0f);
    REPORTER_ASSERT(r, fs[1] == 0.0f);
    REPORTER_ASSERT(r, fs[2] == 2.0f);
    REPORTER_ASSERT(r, fs[3] == 4.0f);
    auto fshi = Sk2f(0.0f, -0.0f).abs();
    auto fslo = Sk2f(2.0f, -4.0f).abs();
    REPORTER_ASSERT(r, fshi[0] == 0.0f);
    REPORTER_ASSERT(r, fshi[1] == 0.0f);
    REPORTER_ASSERT(r, fslo[0] == 2.0f);
    REPORTER_ASSERT(r, fslo[1] == 4.0f);
}

DEF_TEST(Sk4i_abs, r) {
    auto is = Sk4i(0, -1, 2, -2147483647).abs();
    REPORTER_ASSERT(r, is[0] == 0);
    REPORTER_ASSERT(r, is[1] == 1);
    REPORTER_ASSERT(r, is[2] == 2);
    REPORTER_ASSERT(r, is[3] == 2147483647);
}

DEF_TEST(Sk4i_minmax, r) {
    auto a = Sk4i(0, 2, 4, 6);
    auto b = Sk4i(1, 1, 3, 7);
    auto min = Sk4i::Min(a, b);
    auto max = Sk4i::Max(a, b);
    for(int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(r, min[i] == SkTMin(a[i], b[i]));
        REPORTER_ASSERT(r, max[i] == SkTMax(a[i], b[i]));
    }
}

DEF_TEST(SkNx_floor, r) {
    auto fs = Sk4f(0.4f, -0.4f, 0.6f, -0.6f).floor();
    REPORTER_ASSERT(r, fs[0] ==  0.0f);
    REPORTER_ASSERT(r, fs[1] == -1.0f);
    REPORTER_ASSERT(r, fs[2] ==  0.0f);
    REPORTER_ASSERT(r, fs[3] == -1.0f);

    auto fs2 = Sk2f(0.4f, -0.4f).floor();
    REPORTER_ASSERT(r, fs2[0] ==  0.0f);
    REPORTER_ASSERT(r, fs2[1] == -1.0f);

    auto fs3 = Sk2f(0.6f, -0.6f).floor();
    REPORTER_ASSERT(r, fs3[0] ==  0.0f);
    REPORTER_ASSERT(r, fs3[1] == -1.0f);
}

DEF_TEST(SkNx_shuffle, r) {
    Sk4f f4(0,10,20,30);

    Sk2f f2 = SkNx_shuffle<2,1>(f4);
    REPORTER_ASSERT(r, f2[0] == 20);
    REPORTER_ASSERT(r, f2[1] == 10);

    f4 = SkNx_shuffle<0,1,1,0>(f2);
    REPORTER_ASSERT(r, f4[0] == 20);
    REPORTER_ASSERT(r, f4[1] == 10);
    REPORTER_ASSERT(r, f4[2] == 10);
    REPORTER_ASSERT(r, f4[3] == 20);
}

DEF_TEST(SkNx_int_float, r) {
    Sk4f f(-2.3f, 1.0f, 0.45f, 0.6f);

    Sk4i i = SkNx_cast<int>(f);
    REPORTER_ASSERT(r, i[0] == -2);
    REPORTER_ASSERT(r, i[1] ==  1);
    REPORTER_ASSERT(r, i[2] ==  0);
    REPORTER_ASSERT(r, i[3] ==  0);

    f = SkNx_cast<float>(i);
    REPORTER_ASSERT(r, f[0] == -2.0f);
    REPORTER_ASSERT(r, f[1] ==  1.0f);
    REPORTER_ASSERT(r, f[2] ==  0.0f);
    REPORTER_ASSERT(r, f[3] ==  0.0f);
}

#include "include/utils/SkRandom.h"

DEF_TEST(SkNx_u16_float, r) {
    {
        // u16 --> float
        auto h4 = Sk4h(15, 17, 257, 65535);
        auto f4 = SkNx_cast<float>(h4);
        REPORTER_ASSERT(r, f4[0] == 15.0f);
        REPORTER_ASSERT(r, f4[1] == 17.0f);
        REPORTER_ASSERT(r, f4[2] == 257.0f);
        REPORTER_ASSERT(r, f4[3] == 65535.0f);
    }
    {
        // float -> u16
        auto f4 = Sk4f(15, 17, 257, 65535);
        auto h4 = SkNx_cast<uint16_t>(f4);
        REPORTER_ASSERT(r, h4[0] == 15);
        REPORTER_ASSERT(r, h4[1] == 17);
        REPORTER_ASSERT(r, h4[2] == 257);
        REPORTER_ASSERT(r, h4[3] == 65535);
    }

    // starting with any u16 value, we should be able to have a perfect round-trip in/out of floats
    //
    SkRandom rand;
    for (int i = 0; i < 10000; ++i) {
        const uint16_t s16[4] {
            (uint16_t)(rand.nextU() >> 16), (uint16_t)(rand.nextU() >> 16),
            (uint16_t)(rand.nextU() >> 16), (uint16_t)(rand.nextU() >> 16),
        };
        auto u4_0 = Sk4h::Load(s16);
        auto f4 = SkNx_cast<float>(u4_0);
        auto u4_1 = SkNx_cast<uint16_t>(f4);
        uint16_t d16[4];
        u4_1.store(d16);
        REPORTER_ASSERT(r, !memcmp(s16, d16, sizeof(s16)));
    }
}

// The SSE2 implementation of SkNx_cast<uint16_t>(Sk4i) is non-trivial, so worth a test.
DEF_TEST(SkNx_int_u16, r) {
    // These are pretty hard to get wrong.
    for (int i = 0; i <= 0x7fff; i++) {
        uint16_t expected = (uint16_t)i;
        uint16_t actual = SkNx_cast<uint16_t>(Sk4i(i))[0];

        REPORTER_ASSERT(r, expected == actual);
    }

    // A naive implementation with _mm_packs_epi32 would succeed up to 0x7fff but fail here:
    for (int i = 0x8000; (1) && i <= 0xffff; i++) {
        uint16_t expected = (uint16_t)i;
        uint16_t actual = SkNx_cast<uint16_t>(Sk4i(i))[0];

        REPORTER_ASSERT(r, expected == actual);
    }
}

DEF_TEST(SkNx_4fLoad4Store4, r) {
    float src[] = {
         0.0f,  1.0f,  2.0f,  3.0f,
         4.0f,  5.0f,  6.0f,  7.0f,
         8.0f,  9.0f, 10.0f, 11.0f,
        12.0f, 13.0f, 14.0f, 15.0f
    };

    Sk4f a, b, c, d;
    Sk4f::Load4(src, &a, &b, &c, &d);
    REPORTER_ASSERT(r,  0.0f == a[0]);
    REPORTER_ASSERT(r,  4.0f == a[1]);
    REPORTER_ASSERT(r,  8.0f == a[2]);
    REPORTER_ASSERT(r, 12.0f == a[3]);
    REPORTER_ASSERT(r,  1.0f == b[0]);
    REPORTER_ASSERT(r,  5.0f == b[1]);
    REPORTER_ASSERT(r,  9.0f == b[2]);
    REPORTER_ASSERT(r, 13.0f == b[3]);
    REPORTER_ASSERT(r,  2.0f == c[0]);
    REPORTER_ASSERT(r,  6.0f == c[1]);
    REPORTER_ASSERT(r, 10.0f == c[2]);
    REPORTER_ASSERT(r, 14.0f == c[3]);
    REPORTER_ASSERT(r,  3.0f == d[0]);
    REPORTER_ASSERT(r,  7.0f == d[1]);
    REPORTER_ASSERT(r, 11.0f == d[2]);
    REPORTER_ASSERT(r, 15.0f == d[3]);

    float dst[16];
    Sk4f::Store4(dst, a, b, c, d);
    REPORTER_ASSERT(r, 0 == memcmp(dst, src, 16 * sizeof(float)));
}

DEF_TEST(SkNx_neg, r) {
    auto fs = -Sk4f(0.0f, -0.0f, 2.0f, -4.0f);
    REPORTER_ASSERT(r, fs[0] == 0.0f);
    REPORTER_ASSERT(r, fs[1] == 0.0f);
    REPORTER_ASSERT(r, fs[2] == -2.0f);
    REPORTER_ASSERT(r, fs[3] == 4.0f);
    auto fshi = -Sk2f(0.0f, -0.0f);
    auto fslo = -Sk2f(2.0f, -4.0f);
    REPORTER_ASSERT(r, fshi[0] == 0.0f);
    REPORTER_ASSERT(r, fshi[1] == 0.0f);
    REPORTER_ASSERT(r, fslo[0] == -2.0f);
    REPORTER_ASSERT(r, fslo[1] == 4.0f);
}

DEF_TEST(SkNx_thenElse, r) {
    auto fs = (Sk4f(0.0f, -0.0f, 2.0f, -4.0f) < 0).thenElse(-1, 1);
    REPORTER_ASSERT(r, fs[0] == 1);
    REPORTER_ASSERT(r, fs[1] == 1);
    REPORTER_ASSERT(r, fs[2] == 1);
    REPORTER_ASSERT(r, fs[3] == -1);
    auto fshi = (Sk2f(0.0f, -0.0f) < 0).thenElse(-1, 1);
    auto fslo = (Sk2f(2.0f, -4.0f) < 0).thenElse(-1, 1);
    REPORTER_ASSERT(r, fshi[0] == 1);
    REPORTER_ASSERT(r, fshi[1] == 1);
    REPORTER_ASSERT(r, fslo[0] == 1);
    REPORTER_ASSERT(r, fslo[1] == -1);
}

DEF_TEST(Sk4f_Load2, r) {
    float xy[8] = { 0,1,2,3,4,5,6,7 };

    Sk4f x,y;
    Sk4f::Load2(xy, &x,&y);

    REPORTER_ASSERT(r, x[0] == 0);
    REPORTER_ASSERT(r, x[1] == 2);
    REPORTER_ASSERT(r, x[2] == 4);
    REPORTER_ASSERT(r, x[3] == 6);

    REPORTER_ASSERT(r, y[0] == 1);
    REPORTER_ASSERT(r, y[1] == 3);
    REPORTER_ASSERT(r, y[2] == 5);
    REPORTER_ASSERT(r, y[3] == 7);
}

DEF_TEST(Sk2f_Load2, r) {
    float xy[4] = { 0,1,2,3 };

    Sk2f x,y;
    Sk2f::Load2(xy, &x,&y);

    REPORTER_ASSERT(r, x[0] == 0);
    REPORTER_ASSERT(r, x[1] == 2);

    REPORTER_ASSERT(r, y[0] == 1);
    REPORTER_ASSERT(r, y[1] == 3);
}

DEF_TEST(Sk2f_Store2, r) {
    Sk2f p0{0, 2};
    Sk2f p1{1, 3};
    float dst[4];
    Sk2f::Store2(dst, p0, p1);
    REPORTER_ASSERT(r, dst[0] == 0);
    REPORTER_ASSERT(r, dst[1] == 1);
    REPORTER_ASSERT(r, dst[2] == 2);
    REPORTER_ASSERT(r, dst[3] == 3);
}

DEF_TEST(Sk2f_Store3, r) {
    Sk2f p0{0, 3};
    Sk2f p1{1, 4};
    Sk2f p2{2, 5};
    float dst[6];
    Sk2f::Store3(dst, p0, p1, p2);
    REPORTER_ASSERT(r, dst[0] == 0);
    REPORTER_ASSERT(r, dst[1] == 1);
    REPORTER_ASSERT(r, dst[2] == 2);
    REPORTER_ASSERT(r, dst[3] == 3);
    REPORTER_ASSERT(r, dst[4] == 4);
    REPORTER_ASSERT(r, dst[5] == 5);
}

DEF_TEST(Sk2f_Store4, r) {
    Sk2f p0{0, 4};
    Sk2f p1{1, 5};
    Sk2f p2{2, 6};
    Sk2f p3{3, 7};

    float dst[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
    Sk2f::Store4(dst, p0, p1, p2, p3);
    REPORTER_ASSERT(r, dst[0] == 0);
    REPORTER_ASSERT(r, dst[1] == 1);
    REPORTER_ASSERT(r, dst[2] == 2);
    REPORTER_ASSERT(r, dst[3] == 3);
    REPORTER_ASSERT(r, dst[4] == 4);
    REPORTER_ASSERT(r, dst[5] == 5);
    REPORTER_ASSERT(r, dst[6] == 6);
    REPORTER_ASSERT(r, dst[7] == 7);

    // Ensure transposing to Sk4f works.
    Sk4f dst4f[2] = {{-1, -1, -1, -1}, {-1, -1, -1, -1}};
    Sk2f::Store4(dst4f, p0, p1, p2, p3);
    REPORTER_ASSERT(r, dst4f[0][0] == 0);
    REPORTER_ASSERT(r, dst4f[0][1] == 1);
    REPORTER_ASSERT(r, dst4f[0][2] == 2);
    REPORTER_ASSERT(r, dst4f[0][3] == 3);
    REPORTER_ASSERT(r, dst4f[1][0] == 4);
    REPORTER_ASSERT(r, dst4f[1][1] == 5);
    REPORTER_ASSERT(r, dst4f[1][2] == 6);
    REPORTER_ASSERT(r, dst4f[1][3] == 7);

}

DEF_TEST(Sk4f_minmax, r) {
    REPORTER_ASSERT(r,  3 == Sk4f(0,1,2,3).max());
    REPORTER_ASSERT(r,  2 == Sk4f(1,-5,2,-1).max());
    REPORTER_ASSERT(r, -1 == Sk4f(-2,-1,-6,-3).max());
    REPORTER_ASSERT(r,  3 == Sk4f(3,2,1,0).max());

    REPORTER_ASSERT(r,  0 == Sk4f(0,1,2,3).min());
    REPORTER_ASSERT(r, -5 == Sk4f(1,-5,2,-1).min());
    REPORTER_ASSERT(r, -6 == Sk4f(-2,-1,-6,-3).min());
    REPORTER_ASSERT(r,  0 == Sk4f(3,2,1,0).min());
}

DEF_TEST(SkNf_anyTrue_allTrue, r) {
    REPORTER_ASSERT(r,  (Sk2f{1,2} < Sk2f{3,4}).anyTrue());
    REPORTER_ASSERT(r,  (Sk2f{1,2} < Sk2f{3,4}).allTrue());
    REPORTER_ASSERT(r,  (Sk2f{3,2} < Sk2f{1,4}).anyTrue());
    REPORTER_ASSERT(r, !(Sk2f{3,2} < Sk2f{1,4}).allTrue());
    REPORTER_ASSERT(r, !(Sk2f{3,4} < Sk2f{1,2}).anyTrue());

    REPORTER_ASSERT(r,  (Sk4f{1,2,3,4} < Sk4f{3,4,5,6}).anyTrue());
    REPORTER_ASSERT(r,  (Sk4f{1,2,3,4} < Sk4f{3,4,5,6}).allTrue());
    REPORTER_ASSERT(r,  (Sk4f{1,2,3,4} < Sk4f{1,4,1,1}).anyTrue());
    REPORTER_ASSERT(r, !(Sk4f{1,2,3,4} < Sk4f{1,4,1,1}).allTrue());
    REPORTER_ASSERT(r, !(Sk4f{3,4,5,6} < Sk4f{1,2,3,4}).anyTrue());
}
