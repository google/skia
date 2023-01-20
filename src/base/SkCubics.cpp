/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkCubics.h"

#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTPin.h"
#include "src/base/SkQuads.h"

#include <cmath>

static constexpr double PI = 3.141592653589793;

int SkCubics::RootsReal(double A, double B, double C, double D, double solution[3]) {
    if (sk_double_nearly_zero(A)) {  // we're just a quadratic
        return SkQuads::RootsReal(B, C, D, solution);
    }
    if (sk_double_nearly_zero(D)) {  // 0 is one root
        int num = SkQuads::RootsReal(A, B, C, solution);
        for (int i = 0; i < num; ++i) {
            if (sk_double_nearly_zero(solution[i])) {
                return num;
            }
        }
        solution[num++] = 0;
        return num;
    }
    if (sk_double_nearly_zero(A + B + C + D)) {  // 1 is one root
        int num = SkQuads::RootsReal(A, A + B, -D, solution);
        for (int i = 0; i < num; ++i) {
            if (sk_doubles_nearly_equal_ulps(solution[i], 1)) {
                return num;
            }
        }
        solution[num++] = 1;
        return num;
    }
    double a, b, c;
    {
        double invA = 1 / A;
        a = B * invA;
        b = C * invA;
        c = D * invA;
    }
    double a2 = a * a;
    double Q = (a2 - b * 3) / 9;
    double R = (2 * a2 * a - 9 * a * b + 27 * c) / 54;
    double R2 = R * R;
    double Q3 = Q * Q * Q;
    double R2MinusQ3 = R2 - Q3;
    double adiv3 = a / 3;
    double r;
    double* roots = solution;
    if (R2MinusQ3 < 0) {   // we have 3 real roots
        // the divide/root can, due to finite precisions, be slightly outside of -1...1
        double theta = acos(SkTPin(R / sqrt(Q3), -1., 1.));
        double neg2RootQ = -2 * sqrt(Q);

        r = neg2RootQ * cos(theta / 3) - adiv3;
        *roots++ = r;

        r = neg2RootQ * cos((theta + 2 * PI) / 3) - adiv3;
        if (!sk_doubles_nearly_equal_ulps(solution[0], r)) {
            *roots++ = r;
        }
        r = neg2RootQ * cos((theta - 2 * PI) / 3) - adiv3;
        if (!sk_doubles_nearly_equal_ulps(solution[0], r) && (roots - solution == 1 ||
            !sk_doubles_nearly_equal_ulps(solution[1], r))) {
            *roots++ = r;
        }
    } else {  // we have 1 real root
        double sqrtR2MinusQ3 = sqrt(R2MinusQ3);
        A = fabs(R) + sqrtR2MinusQ3;
        A = std::cbrt(A); // cube root
        if (R > 0) {
            A = -A;
        }
        if (A != 0) {
            A += Q / A;
        }
        r = A - adiv3;
        *roots++ = r;
        if (sk_doubles_nearly_equal_ulps(R2, Q3)) {
            r = -A / 2 - adiv3;
            if (!sk_doubles_nearly_equal_ulps(solution[0], r)) {
                *roots++ = r;
            }
        }
    }
    return static_cast<int>(roots - solution);
}

int SkCubics::RootsValidT(double A, double B, double C, double D,
                          double t[3]) {
    double solution[3] = {0, 0, 0};
    int realRoots = SkCubics::RootsReal(A, B, C, D, solution);
    int foundRoots = 0;
    for (int index = 0; index < realRoots; ++index) {
        double tValue = solution[index];
        if (tValue >= 1.0 && tValue <= 1.00005) {
            // Make sure we do not already have 1 (or something very close) in the list of roots.
            if ((foundRoots < 1 || !sk_doubles_nearly_equal_ulps(t[0], 1)) &&
                (foundRoots < 2 || !sk_doubles_nearly_equal_ulps(t[1], 1))) {
                t[foundRoots++] = 1;
            }
        } else if (tValue >= -0.00005 && tValue <= 0.0) {
            // Make sure we do not already have 0 (or something very close) in the list of roots.
            if ((foundRoots < 1 || !sk_double_nearly_zero(t[0])) &&
                (foundRoots < 2 || !sk_double_nearly_zero(t[1]))) {
                t[foundRoots++] = 0;
            }
        } else if (tValue > 0.0 && tValue < 1.0) {
            t[foundRoots++] = tValue;
        }
    }
    return foundRoots;
}
