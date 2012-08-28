/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CubicLineSegments.h"
#include "QuadraticLineSegments.h"
#include <algorithm> // used for std::max

// http://cagd.cs.byu.edu/~557/text/cagd.pdf 2.7
// A hodograph is the first derivative curve
void hodograph(const Cubic& cubic, Quadratic& hodo) {
    hodo[0].x = 3 * (cubic[1].x - cubic[0].x);
    hodo[0].y = 3 * (cubic[1].y - cubic[0].y);
    hodo[1].x = 3 * (cubic[2].x - cubic[1].x);
    hodo[1].y = 3 * (cubic[2].y - cubic[1].y);
    hodo[2].x = 3 * (cubic[3].x - cubic[2].x);
    hodo[2].y = 3 * (cubic[3].y - cubic[2].y);
}

// A 2nd hodograph is the second derivative curve
void secondHodograph(const Cubic& cubic, _Line& hodo2) {
    Quadratic hodo;
    hodograph(cubic, hodo);
    hodograph(hodo, hodo2);
}

// The number of line segments required to approximate the cubic
// see  http://cagd.cs.byu.edu/~557/text/cagd.pdf 10.6
double subDivisions(const Cubic& cubic) {
    _Line hodo2;
    secondHodograph(cubic, hodo2);
    double maxX = std::max(hodo2[1].x, hodo2[1].x);
    double maxY = std::max(hodo2[1].y, hodo2[1].y);
    double dist = sqrt(maxX * maxX + maxY * maxY);
    double segments = sqrt(dist / (8 * FLT_EPSILON));
    return segments;
}
