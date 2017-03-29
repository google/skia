/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkInsetConvexPolygon.h"

#include "SkTemplates.h"

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
static int get_winding(const SkPoint* polygonVerts, int polygonSize) {
    SkPoint p0 = polygonVerts[0];
    SkPoint p1 = polygonVerts[1];

    for (int i = 2; i < polygonSize; ++i) {
        SkPoint p2 = polygonVerts[i];

        // determine if cw or ccw
        int side = compute_side(p0, p1, p2);
        if (0 != side) {
            return ((side > 0) ? 1 : -1);
        }

        // if nearly collinear, treat as straight line and continue
        p1 = p2;
    }

    return 0;
}

// Perpendicularly offset line segment p0-p1 'distance' units in the direction specified by 'dir'
static void inset_edge(const SkPoint& p0, const SkPoint& p1, SkScalar distance, int dir,
                       InsetSegment* inset) {
    SkASSERT(dir == -1 || dir == 1);
    // compute perpendicular
    SkVector perp;
    perp.fX = p0.fY - p1.fY;
    perp.fY = p1.fX - p0.fX;
    perp.setLength(distance*dir);
    inset->fP0 = p0 + perp;
    inset->fP1 = p1 + perp;
}

// Compute the intersection 'p' between segments s0 and s1, if any.
// 's' is the parametric value for the intersection along 's0' & 't' is the same for 's1'.
// Returns false if there is no intersection.
static bool compute_intersection(const InsetSegment& s0, const InsetSegment& s1,
                                 SkPoint* p, SkScalar* s, SkScalar* t) {
    SkVector v0 = s0.fP1 - s0.fP0;
    SkVector v1 = s1.fP1 - s1.fP0;

    SkScalar perpDot = v0.cross(v1);
    if (SkScalarNearlyZero(perpDot)) {
        // segments are parallel
        // check if endpoints are touching
        if (s0.fP1.equalsWithinTolerance(s1.fP0)) {
            *p = s0.fP1;
            *s = SK_Scalar1;
            *t = 0;
            return true;
        }
        if (s1.fP1.equalsWithinTolerance(s0.fP0)) {
            *p = s1.fP1;
            *s = 0;
            *t = SK_Scalar1;
            return true;
        }

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

#ifdef SK_DEBUG
static bool is_convex(const SkTDArray<SkPoint>& poly) {
    if (poly.count() <= 3) {
        return true;
    }

    SkVector v0 = poly[0] - poly[poly.count() - 1];
    SkVector v1 = poly[1] - poly[poly.count() - 1];
    SkScalar winding = v0.cross(v1);

    for (int i = 0; i < poly.count() - 1; ++i) {
        int j = i + 1;
        int k = (i + 2) % poly.count();

        SkVector v0 = poly[j] - poly[i];
        SkVector v1 = poly[k] - poly[i];
        SkScalar perpDot = v0.cross(v1);
        if (winding*perpDot < 0) {
            return false;
        }
    }

    return true;
}
#endif

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
bool SkInsetConvexPolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                          SkScalar insetDistance, SkTDArray<SkPoint>* insetPolygon) {
    if (inputPolygonSize < 3) {
        return false;
    }

    int winding = get_winding(inputPolygonVerts, inputPolygonSize);
    if (0 == winding) {
        return false;
    }

    // set up
    struct EdgeData {
        InsetSegment fInset;
        SkPoint      fIntersection;
        SkScalar     fTValue;
        bool         fValid;
    };

    SkAutoSTMalloc<64, EdgeData> edgeData(inputPolygonSize);
    for (int i = 0; i < inputPolygonSize; ++i) {
        int j = (i + 1) % inputPolygonSize;
        inset_edge(inputPolygonVerts[i], inputPolygonVerts[j], insetDistance, winding,
                   &edgeData[i].fInset);
        edgeData[i].fIntersection = edgeData[i].fInset.fP0;
        edgeData[i].fTValue = SK_ScalarMin;
        edgeData[i].fValid = true;
    }

    int prevIndex = inputPolygonSize - 1;
    int currIndex = 0;
    int insetVertexCount = inputPolygonSize;
    while (prevIndex != currIndex) {
        if (!edgeData[prevIndex].fValid) {
            prevIndex = (prevIndex + inputPolygonSize - 1) % inputPolygonSize;
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
                --insetVertexCount;
                // go back one segment
                prevIndex = (prevIndex + inputPolygonSize - 1) % inputPolygonSize;
            // we've already considered this intersection, we're done
            } else if (edgeData[currIndex].fTValue > SK_ScalarMin &&
                       intersection.equalsWithinTolerance(edgeData[currIndex].fIntersection,
                                                          1.0e-6f)) {
                break;
            } else {
                // add intersection
                edgeData[currIndex].fIntersection = intersection;
                edgeData[currIndex].fTValue = t;

                // go to next segment
                prevIndex = currIndex;
                currIndex = (currIndex + 1) % inputPolygonSize;
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
                --insetVertexCount;
                // go back one segment
                prevIndex = (prevIndex + inputPolygonSize - 1) % inputPolygonSize;
            } else {
                // move to next segment
                edgeData[currIndex].fValid = false;
                --insetVertexCount;
                currIndex = (currIndex + 1) % inputPolygonSize;
            }
        }
    }

    // store all the valid intersections
    insetPolygon->reset();
    insetPolygon->setReserve(insetVertexCount);
    for (int i = 0; i < inputPolygonSize; ++i) {
        if (edgeData[i].fValid) {
            *insetPolygon->push() = edgeData[i].fIntersection;
        }
    }
    SkASSERT(is_convex(*insetPolygon));

    return (insetPolygon->count() >= 3);
}
