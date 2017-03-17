/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkInsetConvexPolygon.h"

struct MySegment {
    SkPoint fP0;
    SkPoint fP1;
};

static constexpr SkScalar kDistanceTolerance = (SK_Scalar1 / 16);
static constexpr SkScalar kParamTolerance = 1.0e-3f;

// Computes perpDot for point compared to segment. 
// A positive value means the point is to the left of the segment, 
// negative is to the right, 0 is collinear.
static int compute_side(const SkPoint& s0, const SkPoint& s1, const SkPoint& p) {
    SkVector v0 = s1 - s0;
    SkVector v1 = p - s0;
    SkScalar perpDot = v0.cross(v1);
    if (!SkScalarNearlyZero(perpDot)) {
        return ((perpDot > 0) ? 1 : -1);
    }

    return 0;
}

// returns 1 for ccw, -1 for cw and 0 if degenerate
static int get_winding(const SkTDArray<SkPoint>& polygon) {
    SkPoint p0 = polygon[0];
    SkPoint p1 = polygon[1];

    for (int i = 2; i < polygon.count(); ++i) {
        SkPoint p2 = polygon[i];

        // determine if cw or ccw
        int side = compute_side(p0, p1, p2);
        if (!SkScalarNearlyZero(side)) {
            return ((side > 0) ? 1 : -1);
        }

        // if nearly collinear, treat as straight line and continue
        p1 = p2;
    }

    return 0;
}

static void inset_edge(const SkPoint& p0, const SkPoint& p1, SkScalar radius, SkScalar dir,
                       MySegment* inset) {
    // compute perpendicular
    SkVector normal;
    normal.fX = p0.fY - p1.fY;
    normal.fY = p1.fX - p0.fX;
    SkASSERT_RELEASE(normal.normalize());
    normal *= radius*dir;
    inset->fP0 = p0 + normal;
    inset->fP1 = p1 + normal;
}

static bool compute_intersection(const MySegment& s0, const MySegment& s1,
                                 SkPoint* p, SkScalar* s, SkScalar* t) {
    SkVector v0 = s0.fP1 - s0.fP0;
    SkVector v1 = s1.fP1 - s1.fP0;

    SkScalar perpDot = v0.cross(v1);
    if (SkScalarNearlyZero(perpDot)) {
        // segments are parallel
        return false;
    }

    SkVector d = s1.fP0 - s0.fP0;
    SkScalar localS = d.cross(v1) / perpDot;
    if (localS < 0 || localS > SK_Scalar1) {
        return false;
    }
    SkScalar localT = d.cross(v0) / perpDot;
    if (localT < 0 || localT > SK_Scalar1) {
        return false;
    }

    v0 *= localS;
    *p = s0.fP0 + v0;
    *s = localS;
    *t = localT;

    return true;
}

// The objective here is to inset all of the edges by the given distance, and then 
// remove any invalid inset edges by detecting right-hand turns. In a ccw polygon,
// we should only be making left-hand turns (for cw polygons, we use the winding
// parameter to reverse this).
//
// There are a couple ways to catch this:
// * If the second edge's tip is to the right of the first edge
// * If the second intersection on an edge is closer to its tail than the first one
// 
// We might also have the case that there is no intersection between two neighboring inset edges.
// In this case, one edge will lie to the right of the other and should be discarded along with
// its previous intersection (if any).
//
// Note: the assumption is that inputPolygon is convex and has no coincident points.
//
bool SkInsetConvexPolygon(const SkTDArray<SkPoint>& inputPolygon, SkScalar insetDistance,
                          SkTDArray<SkPoint>* insetPolygon) {
    if (inputPolygon.count() < 3) {
        return false;
    }

    int winding = get_winding(inputPolygon);
    if (0 == winding) {
        return false;
    }

    // set up
    insetPolygon->setReserve(inputPolygon.count());

    bool* okay = new bool[inputPolygon.count()];
    for (int i = 0; i < inputPolygon.count(); ++i) {
        okay[i] = true;
    }

    int polySize = inputPolygon.count();
    int lastCurr = polySize;
    int prevIndex = polySize - 1;
    int currIndex = 0;
    SkScalar lastT = 0;
    while (currIndex < lastCurr && prevIndex != currIndex) {
        if (!okay[prevIndex]) {
            prevIndex = (prevIndex + polySize - 1) % polySize;
            continue;
        }

        int prevIndexPlus = (prevIndex + 1) % polySize;
        // TODO: don't recompute insets
        MySegment inset0;
        inset_edge(inputPolygon[prevIndex], inputPolygon[prevIndexPlus], insetDistance, winding,
                   &inset0);
        int currIndexPlus = (currIndex + 1) % polySize;
        MySegment inset1;
        inset_edge(inputPolygon[currIndex], inputPolygon[currIndexPlus], insetDistance, winding,
                   &inset1);

        //// if inset1.fP1 is to right (if ccw) of inset0, skip this edge
        //if (winding*compute_side(inset0.fP0, inset0.fP1, inset1.fP1) - kParamTolerance < 0) {
        //    ++currIndex;
        //    continue;
        //}

        SkScalar s, t;
        SkPoint intersection;
        if (compute_intersection(inset0, inset1, &intersection, &s, &t)) {
            // if new intersection is further back on inset0 from the previous one
            if (s < lastT) {
                // remove last point (if any)
                if (insetPolygon->count() > 0) {
                    insetPolygon->pop();
                }
                // no point in considering this one again
                if (prevIndex > currIndex) {
                    lastCurr = prevIndex;
                }
                okay[prevIndex] = false;
                // go back one segment
                prevIndex = (prevIndex + polySize - 1) % polySize;
                lastT = 0;
                continue;
            } else {
                *insetPolygon->push() = intersection;

                // go to next segment
                prevIndex = currIndex;
                currIndex++;
                lastT = t;
                continue;
            }
        } else {
            // if prev to right side of curr
            int side = winding*compute_side(inset1.fP0, inset1.fP1, inset0.fP1);
            if (side < 0 && side == winding*compute_side(inset1.fP0, inset1.fP1, inset0.fP0)) {
                // remove last point (if any)
                if (insetPolygon->count() > 0) {
                    insetPolygon->pop();
                }
                // no point in considering this one again
                if (prevIndex > currIndex) {
                    lastCurr = prevIndex;
                }
                okay[prevIndex] = false;
                // go back one segment
                prevIndex = (prevIndex + polySize - 1) % polySize;
                lastT = 0;
                continue;
            } else {
                // move to next segment
                okay[currIndex] = false;
                ++currIndex;
                continue;
            }
        }
    }

    return (insetPolygon->count() >= 3);
}
