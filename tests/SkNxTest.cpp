/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkNx.h"
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
    assert_eq(-a, -3, -4, -5, -6);

    SkNf<N,T> fours(4);

    assert_eq(fours.sqrt(), 2,2,2,2);
    assert_nearly_eq(0.001, fours.rsqrt(), 0.5, 0.5, 0.5, 0.5);

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
