/*
 * Copyright 2012 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkBezierCurves.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkPoint_impl.h"
#include "src/base/SkCubics.h"
#include "src/base/SkQuads.h"

#include <cstddef>

static inline double interpolate(double A, double B, double t) {
    return A + (B - A) * t;
}

std::array<double, 2> SkBezierCubic::EvalAt(const double curve[8], double t) {
    const auto in_X = [&curve](size_t n) { return curve[2*n]; };
    const auto in_Y = [&curve](size_t n) { return curve[2*n + 1]; };

    // Two semi-common fast paths
    if (t == 0) {
        return {in_X(0), in_Y(0)};
    }
    if (t == 1) {
        return {in_X(3), in_Y(3)};
    }
    // X(t) = X_0*(1-t)^3 + 3*X_1*t(1-t)^2 + 3*X_2*t^2(1-t) + X_3*t^3
    // Y(t) = Y_0*(1-t)^3 + 3*Y_1*t(1-t)^2 + 3*Y_2*t^2(1-t) + Y_3*t^3
    // Some compilers are smart enough and have sufficient registers/intrinsics to write optimal
    // code from
    //    double one_minus_t = 1 - t;
    //    double a = one_minus_t * one_minus_t * one_minus_t;
    //    double b = 3 * one_minus_t * one_minus_t * t;
    //    double c = 3 * one_minus_t * t * t;
    //    double d = t * t * t;
    // However, some (e.g. when compiling for ARM) fail to do so, so we use this form
    // to help more compilers generate smaller/faster ASM. https://godbolt.org/z/M6jG9x45c
    double one_minus_t = 1 - t;
    double one_minus_t_squared = one_minus_t * one_minus_t;
    double a = (one_minus_t_squared * one_minus_t);
    double b = 3 * one_minus_t_squared * t;
    double t_squared = t * t;
    double c = 3 * one_minus_t * t_squared;
    double d = t_squared * t;

    return {a * in_X(0) + b * in_X(1) + c * in_X(2) + d * in_X(3),
            a * in_Y(0) + b * in_Y(1) + c * in_Y(2) + d * in_Y(3)};
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

std::array<double, 4> SkBezierCubic::ConvertToPolynomial(const double curve[8], bool yValues) {
    const double* offset_curve = yValues ? curve + 1 : curve;
    const auto P = [&offset_curve](size_t n) { return offset_curve[2*n]; };
    // A cubic Bézier curve is interpolated as follows:
    //  c(t) = (1 - t)^3 P_0 + 3t(1 - t)^2 P_1 + 3t^2 (1 - t) P_2 + t^3 P_3
    //       = (-P_0 + 3P_1 + -3P_2 + P_3) t^3 + (3P_0 - 6P_1 + 3P_2) t^2 +
    //         (-3P_0 + 3P_1) t + P_0
    // Where P_N is the Nth point. The second step expands the polynomial and groups
    // by powers of t. The desired output is a cubic formula, so we just need to
    // combine the appropriate points to make the coefficients.
    std::array<double, 4> results;
    results[0] = -P(0) + 3*P(1) - 3*P(2) + P(3);
    results[1] = 3*P(0) - 6*P(1) + 3*P(2);
    results[2] = -3*P(0) + 3*P(1);
    results[3] = P(0);
    return results;
}

namespace {
struct DPoint {
    DPoint(double x_, double y_) : x{x_}, y{y_} {}
    DPoint(SkPoint p) : x{p.fX}, y{p.fY} {}
    double x, y;
};

DPoint operator- (DPoint a) {
    return {-a.x, -a.y};
}

DPoint operator+ (DPoint a, DPoint b) {
    return {a.x + b.x, a.y + b.y};
}

DPoint operator- (DPoint a, DPoint b) {
    return {a.x - b.x, a.y - b.y};
}

DPoint operator* (double s, DPoint a) {
    return {s * a.x, s * a.y};
}

// Pin to 0 or 1 if within half a float ulp of 0 or 1.
double pinTRange(double t) {
    // The ULPs around 0 are tiny compared to the ULPs around 1. Shift to 1 to use the same
    // size ULPs.
    if (sk_double_to_float(t + 1.0) == 1.0f) {
        return 0.0;
    } else if (sk_double_to_float(t) == 1.0f) {
        return 1.0;
    }
    return t;
}
}  // namespace

SkSpan<const float>
SkBezierCubic::IntersectWithHorizontalLine(
        SkSpan<const SkPoint> controlPoints, float yIntercept, float* intersectionStorage) {
    SkASSERT(controlPoints.size() >= 4);
    const DPoint P0 = controlPoints[0],
                 P1 = controlPoints[1],
                 P2 = controlPoints[2],
                 P3 = controlPoints[3];

    const DPoint A =   -P0 + 3*P1 - 3*P2 + P3,
                 B =  3*P0 - 6*P1 + 3*P2,
                 C = -3*P0 + 3*P1,
                 D =    P0;

    return Intersect(A.x, B.x, C.x, D.x, A.y, B.y, C.y, D.y, yIntercept, intersectionStorage);
}

SkSpan<const float>
SkBezierCubic::Intersect(double AX, double BX, double CX, double DX,
                         double AY, double BY, double CY, double DY,
                         float toIntersect, float intersectionsStorage[3]) {
    double roots[3];
    SkSpan<double> ts = SkSpan(roots,
                               SkCubics::RootsReal(AY, BY, CY, DY - toIntersect, roots));

    int intersectionCount = 0;
    for (double t : ts) {
        const double pinnedT = pinTRange(t);
        if (0 <= pinnedT && pinnedT <= 1) {
            intersectionsStorage[intersectionCount++] = SkCubics::EvalAt(AX, BX, CX, DX, pinnedT);
        }
    }

    return {intersectionsStorage, intersectionCount};
}

SkSpan<const float>
SkBezierQuad::IntersectWithHorizontalLine(SkSpan<const SkPoint> controlPoints, float yIntercept,
                                          float intersectionStorage[2]) {
    SkASSERT(controlPoints.size() >= 3);
    const DPoint p0 = controlPoints[0],
                 p1 = controlPoints[1],
                 p2 = controlPoints[2];

    // Calculate A, B, C using doubles to reduce round-off error.
    const DPoint A = p0 - 2 * p1 + p2,
    // Remember we are generating the polynomial in the form A*t^2 -2*B*t + C, so the factor
    // of 2 is not needed and the term is negated. This term for a Bézier curve is usually
    // 2(p1-p0).
                 B = p0 - p1,
                 C = p0;

    return Intersect(A.x, B.x, C.x, A.y, B.y, C.y, yIntercept, intersectionStorage);
}

SkSpan<const float> SkBezierQuad::Intersect(
        double AX, double BX, double CX, double AY, double BY, double CY,
        double yIntercept, float intersectionStorage[2]) {
    auto [discriminant, r0, r1] = SkQuads::Roots(AY, BY, CY - yIntercept);

    int intersectionCount = 0;
    // Round the roots to the nearest float to generate the values t. Valid t's are on the
    // domain [0, 1].
    const double t0 = pinTRange(r0);
    if (0 <= t0 && t0 <= 1) {
        intersectionStorage[intersectionCount++] = SkQuads::EvalAt(AX, -2 * BX, CX, t0);
    }

    const double t1 = pinTRange(r1);
    if (0 <= t1 && t1 <= 1 && t1 != t0) {
        intersectionStorage[intersectionCount++] = SkQuads::EvalAt(AX, -2 * BX, CX, t1);
    }

    return SkSpan{intersectionStorage, intersectionCount};
}

