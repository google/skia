/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPathOpsTypes.h"

const int UlpsEpsilon = 16;

// from http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
union SkPathOpsUlpsFloat {
    int32_t fInt;
    float fFloat;

    SkPathOpsUlpsFloat(float num = 0.0f) : fFloat(num) {}
    bool negative() const { return fInt < 0; }
};

bool AlmostEqualUlps(float A, float B) {
    SkPathOpsUlpsFloat uA(A);
    SkPathOpsUlpsFloat uB(B);
    // Different signs means they do not match.
    if (uA.negative() != uB.negative())
    {
        // Check for equality to make sure +0 == -0
        return A == B;
    }
    // Find the difference in ULPs.
    int ulpsDiff = abs(uA.fInt - uB.fInt);
    return ulpsDiff <= UlpsEpsilon;
}

// cube root approximation using bit hack for 64-bit float
// adapted from Kahan's cbrt
static  double cbrt_5d(double d) {
    const unsigned int B1 = 715094163;
    double t = 0.0;
    unsigned int* pt = (unsigned int*) &t;
    unsigned int* px = (unsigned int*) &d;
    pt[1] = px[1] / 3 + B1;
    return t;
}

// iterative cube root approximation using Halley's method (double)
static  double cbrta_halleyd(const double a, const double R) {
    const double a3 = a * a * a;
    const double b = a * (a3 + R + R) / (a3 + a3 + R);
    return b;
}

// cube root approximation using 3 iterations of Halley's method (double)
static double halley_cbrt3d(double d) {
    double a = cbrt_5d(d);
    a = cbrta_halleyd(a, d);
    a = cbrta_halleyd(a, d);
    return cbrta_halleyd(a, d);
}

double SkDCubeRoot(double x) {
    if (approximately_zero_cubed(x)) {
        return 0;
    }
    double result = halley_cbrt3d(fabs(x));
    if (x < 0) {
        result = -result;
    }
    return result;
}
