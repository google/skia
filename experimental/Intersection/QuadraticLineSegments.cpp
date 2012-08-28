/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "QuadraticLineSegments.h"

// http://cagd.cs.byu.edu/~557/text/cagd.pdf 2.7
// A hodograph is the first derivative curve
void hodograph(const Quadratic& quad, _Line& hodo) {
    hodo[0].x = 2 * (quad[1].x - quad[0].x);
    hodo[0].y = 2 * (quad[1].y - quad[0].y);
    hodo[1].x = 2 * (quad[2].x - quad[1].x);
    hodo[1].y = 2 * (quad[2].y - quad[1].y);
}

// A 2nd hodograph is the second derivative curve
void secondHodograph(const Quadratic& quad, _Point& hodo2) {
    _Line hodo;
    hodograph(quad, hodo);
    hodo2.x = hodo[1].x - hodo[0].x;
    hodo2.y = hodo[1].y - hodo[0].y;
}

// The number of line segments required to approximate the quad
// see  http://cagd.cs.byu.edu/~557/text/cagd.pdf 10.6
double subDivisions(const Quadratic& quad) {
    _Point hodo2;
    secondHodograph(quad, hodo2);
    double dist = sqrt(hodo2.x * hodo2.x + hodo2.y * hodo2.y);
    double segments = sqrt(dist / (8 * FLT_EPSILON));
    return segments;
}
