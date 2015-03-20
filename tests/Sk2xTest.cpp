/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "Sk2x.h"

template <typename T>
static bool nearly_eq(double eps, const Sk2x<T>& v, double x, double y) {
    T vals[2];
    v.store(vals);
    return fabs(vals[0] - (T)x) <= eps && fabs(vals[1] - (T)y) <= eps;
}

template <typename T>
static bool eq(const Sk2x<T>& v, double x, double y) { return nearly_eq(0, v, x, y); }

template <typename T>
static void test(skiatest::Reporter* r) {
    // Constructors, assignment, etc.
    Sk2x<T> a(4),
            b = a,
            c(a);
    REPORTER_ASSERT(r, eq(a, 4, 4));
    REPORTER_ASSERT(r, eq(b, 4, 4));
    REPORTER_ASSERT(r, eq(c, 4, 4));

    Sk2x<T> d(2, 5);
    Sk2x<T> e;
    e = d;
    T vals[] = { 2, 5 };
    Sk2x<T> f = Sk2x<T>::Load(vals);
    REPORTER_ASSERT(r, eq(d, 2, 5));
    REPORTER_ASSERT(r, eq(e, 2, 5));
    REPORTER_ASSERT(r, eq(f, 2, 5));

    a.store(vals);
    REPORTER_ASSERT(r, vals[0] == 4 && vals[1] == 4);

    // Math
    REPORTER_ASSERT(r, eq(a + d, 6,   9));
    REPORTER_ASSERT(r, eq(a - d, 2,  -1));
    REPORTER_ASSERT(r, eq(a * d, 8,  20));

    REPORTER_ASSERT(r, nearly_eq(0.001, a.rsqrt(), 0.5, 0.5));
    REPORTER_ASSERT(r, eq(a.sqrt(), 2, 2));

    REPORTER_ASSERT(r, eq(Sk2x<T>::Min(a, d), 2, 4));
    REPORTER_ASSERT(r, eq(Sk2x<T>::Max(a, d), 4, 5));

    // A bit of both.
    a += d;
    a *= d;
    a -= d;
    REPORTER_ASSERT(r, eq(a, 10, 40));
}

DEF_TEST(Sk2f, r) { test< float>(r); }
DEF_TEST(Sk2d, r) { test<double>(r); }
