/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBezierCurves_DEFINED
#define SkBezierCurves_DEFINED

#include <array>

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
};

#endif
