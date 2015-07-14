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

template <int N, typename T>
static void test_Nf(skiatest::Reporter* r) {

    auto assert_nearly_eq = [&](double eps, const SkNf<N,T>& v, T a, T b, T c, T d) {
        auto close = [=](T a, T b) { return fabs(a-b) <= eps; };
        T vals[4];
        v.store(vals);
        bool ok = close(vals[0], a) && close(vals[1], b)
               && close(v.template kth<0>(), a) && close(v.template kth<1>(), b);
        REPORTER_ASSERT(r, ok);
        if (N == 4) {
            ok = close(vals[2], c) && close(vals[3], d)
              && close(v.template kth<2>(), c) && close(v.template kth<3>(), d);
            REPORTER_ASSERT(r, ok);
        }
    };
    auto assert_eq = [&](const SkNf<N,T>& v, T a, T b, T c, T d) {
        return assert_nearly_eq(0, v, a,b,c,d);
    };

    T vals[] = {3, 4, 5, 6};
    SkNf<N,T> a = SkNf<N,T>::Load(vals),
              b(a),
              c = a;
    SkNf<N,T> d;
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
    assert_eq(SkNf<N,T>(0)-a, -3, -4, -5, -6);

    SkNf<N,T> fours(4);

    assert_eq(fours.sqrt(), 2,2,2,2);
    assert_nearly_eq(0.001, fours.rsqrt0(), 0.5, 0.5, 0.5, 0.5);
    assert_nearly_eq(0.001, fours.rsqrt1(), 0.5, 0.5, 0.5, 0.5);
    assert_nearly_eq(0.001, fours.rsqrt2(), 0.5, 0.5, 0.5, 0.5);

    assert_eq(              fours.      invert(), 0.25, 0.25, 0.25, 0.25);
    assert_nearly_eq(0.001, fours.approxInvert(), 0.25, 0.25, 0.25, 0.25);

    assert_eq(SkNf<N,T>::Min(a, fours), 3, 4, 4, 4);
    assert_eq(SkNf<N,T>::Max(a, fours), 4, 4, 5, 6);

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
    test_Nf<2, float>(r);
    test_Nf<2, double>(r);

    test_Nf<4, float>(r);
    test_Nf<4, double>(r);
}

template <int N, typename T>
void test_Ni(skiatest::Reporter* r) {
    auto assert_eq = [&](const SkNi<N,T>& v, T a, T b, T c, T d, T e, T f, T g, T h) {
        T vals[8];
        v.store(vals);

        switch (N) {
          case 8: REPORTER_ASSERT(r, vals[4] == e && vals[5] == f && vals[6] == g && vals[7] == h);
          case 4: REPORTER_ASSERT(r, vals[2] == c && vals[3] == d);
          case 2: REPORTER_ASSERT(r, vals[0] == a && vals[1] == b);
        }
        switch (N) {
          case 8: REPORTER_ASSERT(r, v.template kth<4>() == e && v.template kth<5>() == f &&
                                     v.template kth<6>() == g && v.template kth<7>() == h);
          case 4: REPORTER_ASSERT(r, v.template kth<2>() == c && v.template kth<3>() == d);
          case 2: REPORTER_ASSERT(r, v.template kth<0>() == a && v.template kth<1>() == b);
        }
    };

    T vals[] = { 1,2,3,4,5,6,7,8 };
    SkNi<N,T> a = SkNi<N,T>::Load(vals),
              b(a),
              c = a;
    SkNi<N,T> d;
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

    REPORTER_ASSERT(r, a.template kth<1>() == 2);
}

DEF_TEST(SkNi, r) {
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
        REPORTER_ASSERT(r, Sk16b::Min(aw, bw).kth<0>() == SkTMin(a, b));
        REPORTER_ASSERT(r, !(aw < bw).kth<0>() == !(a < b));
    }}

    // Exhausting the 16x16 bit space is kind of slow, so only do that in release builds.
#ifdef SK_DEBUG
    SkRandom rand;
    for (int i = 0; i < (1<<16); i++) {
        uint16_t a = rand.nextU() >> 16,
                 b = rand.nextU() >> 16;
        REPORTER_ASSERT(r, Sk8h::Min(Sk8h(a), Sk8h(b)).kth<0>() == SkTMin(a, b));
    }
#else
    for (int a = 0; a < (1<<16); a++) {
    for (int b = 0; b < (1<<16); b++) {
        REPORTER_ASSERT(r, Sk8h::Min(Sk8h(a), Sk8h(b)).kth<0>() == SkTMin(a, b));
    }}
#endif
}

DEF_TEST(SkNi_saturatedAdd, r) {
    for (int a = 0; a < (1<<8); a++) {
    for (int b = 0; b < (1<<8); b++) {
        int exact = a+b;
        if (exact > 255) { exact = 255; }
        if (exact <   0) { exact =   0; }

        REPORTER_ASSERT(r, Sk16b(a).saturatedAdd(Sk16b(b)).kth<0>() == exact);
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
        int correct = (av * bv).div255().kth<0>();
        REPORTER_ASSERT(r, correct == exact);

        // We're a bit more flexible on this method: correct for 0 or 255, otherwise off by <=1.
        int fast = av.approxMulDiv255(bv).kth<0>();
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
