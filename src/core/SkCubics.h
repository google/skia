/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkCubics_DEFINED
#define SkCubics_DEFINED

#include <cmath>

/**
 * Utilities for dealing with cubic formulas with one variable:
 *   f(t) = A*t^3 + B*t^2 + C*t + d
 */
class SkCubics {
public:
    /**
     * Puts up to 3 real solutions to the equation
     *   A*t^3 + B*t^2 + C*t + d = 0
     * in the provided array and returns how many roots that was.
     */
    static int RootsReal(double A, double B, double C, double D,
                         double solution[3]);

    /**
     * Puts up to 3 real solutions to the equation
     *   A*t^3 + B*t^2 + C*t + D = 0
     * in the provided array, with the constraint that t is in the range [0.0, 1.0],
     * and returns how many roots that was.
     */
    static int RootsValidT(double A, double B, double C, double D,
                           double solution[3]);


    /**
     * Puts up to 3 real solutions to the equation
     *   A*t^3 + B*t^2 + C*t + D = 0
     * in the provided array, with the constraint that t is in the range [0.0, 1.0],
     * and returns how many roots that was.
     * This is a slower method than RootsValidT, but more accurate in circumstances
     * where floating point error gets too big.
     */
    static int BinarySearchRootsValidT(double A, double B, double C, double D,
                                       double solution[3]);

    /**
     * Evaluates the cubic function with the 4 provided coefficients and the
     * provided variable.
     */
    static double EvalAt(double A, double B, double C, double D, double t) {
        return std::fma(t, std::fma(t, std::fma(t, A, B), C), D);
    }

    static double EvalAt(double coefficients[4], double t) {
        return EvalAt(coefficients[0], coefficients[1], coefficients[2], coefficients[3], t);
    }
};

#endif
