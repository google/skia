/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBezierCurves_DEFINED
#define SkBezierCurves_DEFINED

#include "include/private/base/SkSpan_impl.h"

#include <array>

struct SkPoint;

/**
 * Utilities for dealing with cubic Bézier curves. These have a start XY
 * point, an end XY point, and two control XY points in between. They take
 * a parameter t which is between 0 and 1 (inclusive) which is used to
 * interpolate between the start and end points, via a route dictated by
 * the control points, and return a new XY point.
 *
 * We store a Bézier curve as an array of 8 floats or doubles, where
 * the even indices are the X coordinates, and the odd indices are the Y
 * coordinates.
 */
class SkBezierCubic {
public:

    /**
     * Evaluates the cubic Bézier curve for a given t. It returns an X and Y coordinate
     * following the formula, which does the interpolation mentioned above.
     *     X(t) = X_0*(1-t)^3 + 3*X_1*t(1-t)^2 + 3*X_2*t^2(1-t) + X_3*t^3
     *     Y(t) = Y_0*(1-t)^3 + 3*Y_1*t(1-t)^2 + 3*Y_2*t^2(1-t) + Y_3*t^3
     *
     * t is typically in the range [0, 1], but this function will not assert that,
     * as Bézier curves are well-defined for any real number input.
     */
    static std::array<double, 2> EvalAt(const double curve[8], double t);

    /**
     * Splits the provided Bézier curve at the location t, resulting in two
     * Bézier curves that share a point (the end point from curve 1
     * and the start point from curve 2 are the same).
     *
     * t must be in the interval [0, 1].
     *
     * The provided twoCurves array will be filled such that indices
     * 0-7 are the first curve (representing the interval [0, t]), and
     * indices 6-13 are the second curve (representing [t, 1]).
     */
    static void Subdivide(const double curve[8], double t,
                          double twoCurves[14]);

    /**
     * Converts the provided Bézier curve into the the equivalent cubic
     *    f(t) = A*t^3 + B*t^2 + C*t + D
     * where f(t) will represent Y coordinates over time if yValues is
     * true and the X coordinates if yValues is false.
     *
     * In effect, this turns the control points into an actual line, representing
     * the x or y values.
     */
    static std::array<double, 4> ConvertToPolynomial(const double curve[8], bool yValues);

    static SkSpan<const float> IntersectWithHorizontalLine(
            SkSpan<const SkPoint> controlPoints, float yIntercept,
            float intersectionStorage[3]);

    static SkSpan<const float> Intersect(
            double AX, double BX, double CX, double DX,
            double AY, double BY, double CY, double DY,
            float toIntersect, float intersectionsStorage[3]);
};

class SkBezierQuad {
public:
    static SkSpan<const float> IntersectWithHorizontalLine(
            SkSpan<const SkPoint> controlPoints, float yIntercept,
            float intersectionStorage[2]);

    /**
     * Given
     *    AY*t^2 -2*BY*t + CY = 0 and AX*t^2 - 2*BX*t + CX = 0,
     *
     * Find the t where AY*t^2 - 2*BY*t + CY - y = 0, then return AX*t^2 + - 2*BX*t + CX
     * where t is on [0, 1].
     *
     * - y - is the height of the line which intersects the quadratic.
     * - intersectionStorage - is the array to hold the return data pointed to in the span.
     *
     * Returns a span with the intersections of yIntercept, and the quadratic formed by A, B,
     * and C.
     */
    static SkSpan<const float> Intersect(
            double AX, double BX, double CX,
            double AY, double BY, double CY,
            double yIntercept,
            float intersectionStorage[2]);
};

#endif  // SkBezierCurves_DEFINED
