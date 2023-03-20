/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkQuads_DEFINED
#define SkQuads_DEFINED

/**
 * Utilities for dealing with quadratic formulas with one variable:
 *   f(t) = A*t^2 + B*t + C
 */
class SkQuads {
public:
    /**
     * Puts up to 2 real solutions to the equation
     *   A*t^2 + B*t + C = 0
     * in the provided array.
     */
    static int RootsReal(double A, double B, double C,
                         double solution[2]);

    /**
     * Evaluates the quadratic function with the 3 provided coefficients and the
     * provided variable.
     */
    static double EvalAt(double A, double B, double C, double t) {
        return A * t * t +
               B * t +
               C;
    }
};

#endif
