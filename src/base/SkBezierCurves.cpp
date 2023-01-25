/*
 * Copyright 2012 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkBezierCurves.h"

#include "include/private/base/SkAssert.h"

#include <cstddef>

static inline double interpolate(double A, double B, double t) {
    return A + (B - A) * t;
}

// Perform subdivision using De Casteljau's algorithm, that is, repeated linear
// interpolation between adjacent points.
void SkBezierCubic::Subdivide(const double curve[8], double t,
                              double twoCurves[14]) {
    SkASSERT(0.0 <= t && t <= 1.0);
    // We split the curve "in" into two curves "alpha" and "beta"
    const auto in_X = [&curve](size_t n) { return curve[2*n]; };
    const auto in_Y = [&curve](size_t n) { return curve[2*n + 1]; };
    const auto alpha_X = [&twoCurves](size_t n) -> double& { return twoCurves[2*n]; };
    const auto alpha_Y = [&twoCurves](size_t n) -> double& { return twoCurves[2*n + 1]; };
    const auto beta_X = [&twoCurves](size_t n) -> double& { return twoCurves[2*n + 6]; };
    const auto beta_Y = [&twoCurves](size_t n) -> double& { return twoCurves[2*n + 7]; };

    alpha_X(0) = in_X(0);
    alpha_Y(0) = in_Y(0);

    beta_X(3) = in_X(3);
    beta_Y(3) = in_Y(3);

    double x01 = interpolate(in_X(0), in_X(1), t);
    double y01 = interpolate(in_Y(0), in_Y(1), t);
    double x12 = interpolate(in_X(1), in_X(2), t);
    double y12 = interpolate(in_Y(1), in_Y(2), t);
    double x23 = interpolate(in_X(2), in_X(3), t);
    double y23 = interpolate(in_Y(2), in_Y(3), t);

    alpha_X(1) = x01;
    alpha_Y(1) = y01;

    beta_X(2) = x23;
    beta_Y(2) = y23;

    alpha_X(2) = interpolate(x01, x12, t);
    alpha_Y(2) = interpolate(y01, y12, t);

    beta_X(1) = interpolate(x12, x23, t);
    beta_Y(1) = interpolate(y12, y23, t);

    alpha_X(3) /*= beta_X(0) */ = interpolate(alpha_X(2), beta_X(1), t);
    alpha_Y(3) /*= beta_Y(0) */ = interpolate(alpha_Y(2), beta_Y(1), t);
}
