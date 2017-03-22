/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkInsetConvexPolygon.h"

#include "SkAutoMalloc.h"

struct InsetSegment {
    SkPoint fP0;
    SkPoint fP1;
};

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
                       InsetSegment* inset) {
    // compute perpendicular
    SkVector normal;
    normal.fX = p0.fY - p1.fY;
    normal.fY = p1.fX - p0.fX;
    SkASSERT_RELEASE(normal.normalize());
    normal *= radius*dir;
    inset->fP0 = p0 + normal;
    inset->fP1 = p1 + normal;
}

static bool compute_intersection(const InsetSegment& s0, const InsetSegment& s1,
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
// parameter to reverse this). We detect this by checking whether the second intersection
// on an edge is closer to its tail than the first one.
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
    int polySize = inputPolygon.count();
    insetPolygon->reset();
    insetPolygon->setReserve(polySize);

    struct EdgeData {
        InsetSegment fInset;
        SkPoint      fIntersection;
        SkScalar     fTValue;
        bool         fValid;
    };

    size_t allocSize = sizeof(EdgeData)*polySize;
    SkAutoSMalloc<1024> storage(allocSize);
    EdgeData* edgeData = (EdgeData*) storage.get();
    for (int i = 0; i < inputPolygon.count(); ++i) {
        edgeData[i].fValid = true;
        int j = (i + 1) % polySize;
        inset_edge(inputPolygon[i], inputPolygon[j], insetDistance, winding,
                   &edgeData[i].fInset);
        edgeData[i].fTValue = -1;
    }

    int prevIndex = polySize - 1;
    int currIndex = 0;
    while (prevIndex != currIndex) {
        if (!edgeData[prevIndex].fValid) {
            prevIndex = (prevIndex + polySize - 1) % polySize;
            continue;
        }

        SkScalar s, t;
        SkPoint intersection;
        if (compute_intersection(edgeData[prevIndex].fInset, edgeData[currIndex].fInset,
                                 &intersection, &s, &t)) {
            // if new intersection is further back on previous inset from the prior intersection
            if (s < edgeData[prevIndex].fTValue) {
                // no point in considering this one again
                edgeData[prevIndex].fValid = false;
                // go back one segment
                prevIndex = (prevIndex + polySize - 1) % polySize;
            // we've already considered this intersection, we're done
            } else if (edgeData[currIndex].fTValue > -1 &&
                       intersection.equalsWithinTolerance(edgeData[currIndex].fIntersection,
                                                          1.0e-6f)) {
                break;
            } else {
                // add intersection
                edgeData[currIndex].fIntersection = intersection;
                edgeData[currIndex].fTValue = t;

                // go to next segment
                prevIndex = currIndex;
                currIndex = (currIndex + 1) % polySize;
            }
        } else {
            // if prev to right side of curr
            int side = winding*compute_side(edgeData[currIndex].fInset.fP0,
                                            edgeData[currIndex].fInset.fP1,
                                            edgeData[prevIndex].fInset.fP1);
            if (side < 0 && side == winding*compute_side(edgeData[currIndex].fInset.fP0,
                                                         edgeData[currIndex].fInset.fP1,
                                                         edgeData[prevIndex].fInset.fP0)) {
                // no point in considering this one again
                edgeData[prevIndex].fValid = false;
                // go back one segment
                prevIndex = (prevIndex + polySize - 1) % polySize;
            } else {
                // move to next segment
                edgeData[currIndex].fValid = false;
                currIndex = (currIndex + 1) % polySize;
            }
        }
    }

    // store all the valid intersections
    for (int i = 0; i < polySize; ++i) {
        if (edgeData[i].fValid) {
            *insetPolygon->push() = edgeData[i].fIntersection;
        }
    }

#ifdef SK_DEBUG
    bool convex = true;
    for (int i = 0; i < insetPolygon->count(); ++i) {
        int j = (i + 1) % insetPolygon->count();
        int k = (i + 2) % insetPolygon->count();

        int side = winding*compute_side((*insetPolygon)[i], (*insetPolygon)[j],
                                        (*insetPolygon)[k]);
        if (side < 0) {
            convex = false;
            break;
        }
    }
    SkASSERT(convex);
#endif

    return (insetPolygon->count() >= 3);
}
