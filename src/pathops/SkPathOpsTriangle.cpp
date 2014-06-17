/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathOpsTriangle.h"

// http://www.blackpawn.com/texts/pointinpoly/default.html
// return true if pt is inside triangle; false if outside or on the line
bool SkDTriangle::contains(const SkDPoint& pt) const {
// Compute vectors
    SkDVector v0 = fPts[2] - fPts[0];
    SkDVector v1 = fPts[1] - fPts[0];
    SkDVector v2 = pt - fPts[0];

// Compute dot products
    double dot00 = v0.dot(v0);
    double dot01 = v0.dot(v1);
    double dot02 = v0.dot(v2);
    double dot11 = v1.dot(v1);
    double dot12 = v1.dot(v2);

// original code doesn't handle degenerate input; isn't symmetric with inclusion of corner pts;
// introduces error with divide; doesn't short circuit on early answer
#if 0
// Compute barycentric coordinates
    double invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
    double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

// Check if point is in triangle
    return (u >= 0) && (v >= 0) && (u + v <= 1);
#else
    double w = dot00 * dot11 - dot01 * dot01;
    if (w == 0) {
        return false;
    }
    double wSign = w < 0 ? -1 : 1;
    double u = (dot11 * dot02 - dot01 * dot12) * wSign;
    if (u <= 0) {
        return false;
    }
    double v = (dot00 * dot12 - dot01 * dot02) * wSign;
    if (v <= 0) {
        return false;
    }
    return u + v < w * wSign;
#endif
}
