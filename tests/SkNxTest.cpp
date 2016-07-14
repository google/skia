/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk4px.h"
#include "SkNx.h"
#include "SkRandom.h"
#include "Test.h"

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

DEF_TEST(Sk4px_muldiv255round, r) {
    for (int a = 0; a < (1<<8); a++) {
    for (int b = 0; b < (1<<8); b++) {
        int exact = (a*b+127)/255;

        // Duplicate a and b 16x each.
        auto av = Sk4px::DupAlpha(a),
             bv = Sk4px::DupAlpha(b);

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

DEF_TEST(Sk4px_widening, r) {
    SkPMColor colors[] = {
        SkPreMultiplyColor(0xff00ff00),
        SkPreMultiplyColor(0x40008000),
        SkPreMultiplyColor(0x7f020406),
        SkPreMultiplyColor(0x00000000),
    };
    auto packed = Sk4px::Load4(colors);

    auto wideLo = packed.widenLo(),
         wideHi = packed.widenHi(),
         wideLoHi    = packed.widenLoHi(),
         wideLoHiAlt = wideLo + wideHi;
    REPORTER_ASSERT(r, 0 == memcmp(&wideLoHi, &wideLoHiAlt, sizeof(wideLoHi)));
}

DEF_TEST(SkNx_abs, r) {
    auto fs = Sk4f(0.0f, -0.0f, 2.0f, -4.0f).abs();
    REPORTER_ASSERT(r, fs[0] == 0.0f);
    REPORTER_ASSERT(r, fs[1] == 0.0f);
    REPORTER_ASSERT(r, fs[2] == 2.0f);
    REPORTER_ASSERT(r, fs[3] == 4.0f);
}

DEF_TEST(SkNx_floor, r) {
    auto fs = Sk4f(0.4f, -0.4f, 0.6f, -0.6f).floor();
    REPORTER_ASSERT(r, fs[0] ==  0.0f);
    REPORTER_ASSERT(r, fs[1] == -1.0f);
    REPORTER_ASSERT(r, fs[2] ==  0.0f);
    REPORTER_ASSERT(r, fs[3] == -1.0f);
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

#include "SkRandom.h"

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
            (uint16_t)rand.nextU16(), (uint16_t)rand.nextU16(),
            (uint16_t)rand.nextU16(), (uint16_t)rand.nextU16(),
        };
        auto u4_0 = Sk4h::Load(s16);
        auto f4 = SkNx_cast<float>(u4_0);
        auto u4_1 = SkNx_cast<uint16_t>(f4);
        uint16_t d16[4];
        u4_1.store(d16);
        REPORTER_ASSERT(r, !memcmp(s16, d16, sizeof(s16)));
    }
}
