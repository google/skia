/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "CurveUtilities.h"
#include "LineParameters.h"
#include <algorithm> // used for std::swap

// return false if unable to clip (e.g., unable to create implicit line)
// caller should subdivide, or create degenerate if the values are too small
bool bezier_clip(const Quadratic& q1, const Quadratic& q2, double& minT, double& maxT) {
    minT = 1;
    maxT = 0;
    // determine normalized implicit line equation for pt[0] to pt[3]
    //   of the form ax + by + c = 0, where a*a + b*b == 1

    // find the implicit line equation parameters
    LineParameters endLine;
    endLine.quadEndPoints(q1);
    if (!endLine.normalize()) {
        printf("line cannot be normalized: need more code here\n");
        return false;
    }

    double distance = endLine.controlPtDistance(q1);

    // find fat line
    double top = 0;
    double bottom = distance / 2; // http://students.cs.byu.edu/~tom/557/text/cic.pdf (7.6)
    if (top > bottom) {
        std::swap(top, bottom);
    }

    // compute intersecting candidate distance
    Quadratic distance2y; // points with X of (0, 1/2, 1)
    endLine.quadDistanceY(q2, distance2y);

    int flags = 0;
    if (approximately_lesser(distance2y[0].y, top)) {
        flags |= kFindTopMin;
    } else if (approximately_greater(distance2y[0].y, bottom)) {
        flags |= kFindBottomMin;
    } else {
        minT = 0;
    }

    if (approximately_lesser(distance2y[2].y, top)) {
        flags |= kFindTopMax;
    } else if (approximately_greater(distance2y[2].y, bottom)) {
        flags |= kFindBottomMax;
    } else {
        maxT = 1;
    }
    // Find the intersection of distance convex hull and fat line.
    int idx = 0;
    do {
        int next = idx + 1;
        if (next == 3) {
            next = 0;
        }
        x_at(distance2y[idx], distance2y[next], top, bottom, flags, minT, maxT);
        idx = next;
    } while (idx);
    return minT < maxT; // returns false if distance shows no intersection
}
