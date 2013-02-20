/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "CurveUtilities.h"
#include "LineParameters.h"

#define DEBUG_BEZIER_CLIP 1

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
        SkASSERT(0);
        return false;
    }

    double distance = endLine.controlPtDistance(q1);

    // find fat line
    double top = 0;
    double bottom = distance / 2; // http://students.cs.byu.edu/~tom/557/text/cic.pdf (7.6)
    if (top > bottom) {
        SkTSwap(top, bottom);
    }

    // compute intersecting candidate distance
    Quadratic distance2y; // points with X of (0, 1/2, 1)
    endLine.quadDistanceY(q2, distance2y);

    int flags = 0;
    if (approximately_lesser_or_equal(distance2y[0].y, top)) {
        flags |= kFindTopMin;
    } else if (approximately_greater_or_equal(distance2y[0].y, bottom)) {
        flags |= kFindBottomMin;
    } else {
        minT = 0;
    }

    if (approximately_lesser_or_equal(distance2y[2].y, top)) {
        flags |= kFindTopMax;
    } else if (approximately_greater_or_equal(distance2y[2].y, bottom)) {
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
#if DEBUG_BEZIER_CLIP
    _Rect r1, r2;
    r1.setBounds(q1);
    r2.setBounds(q2);
    _Point testPt = {0.487, 0.337};
    if (r1.contains(testPt) && r2.contains(testPt)) {
        printf("%s q1=(%1.9g,%1.9g %1.9g,%1.9g %1.9g,%1.9g)"
                " q2=(%1.9g,%1.9g %1.9g,%1.9g %1.9g,%1.9g) minT=%1.9g maxT=%1.9g\n",
                __FUNCTION__, q1[0].x, q1[0].y, q1[1].x, q1[1].y, q1[2].x, q1[2].y,
                q2[0].x, q2[0].y, q2[1].x, q2[1].y, q2[2].x, q2[2].y, minT, maxT);
    }
#endif
    return minT < maxT; // returns false if distance shows no intersection
}
