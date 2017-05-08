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

// Offset line segment p0-p1 'd0' and 'd1' units in the direction specified by 'side'
bool SkOffsetSegment(const SkPoint& p0, const SkPoint& p1, SkScalar d0, SkScalar d1,
                     int side, SkPoint* offset0, SkPoint* offset1) {
    SkASSERT(side == -1 || side == 1);
    SkVector perp = SkVector::Make(p0.fY - p1.fY, p1.fX - p0.fX);
    if (SkScalarNearlyEqual(d0, d1)) {
        // if distances are equal, can just outset by the perpendicular
        perp.setLength(d0*side);
        *offset0 = p0 + perp;
        *offset1 = p1 + perp;
    } else {
        // Otherwise we need to compute the outer tangent.
        // See: http://www.ambrsoft.com/TrigoCalc/Circles2/Circles2Tangent_.htm
        if (d0 < d1) {
            side = -side;
        }
        SkScalar dD = d0 - d1;
        // if one circle is inside another, we can't compute an offset
        if (dD*dD >= p0.distanceToSqd(p1)) {
            return false;
        }
        SkPoint outerTangentIntersect = SkPoint::Make((p1.fX*d0 - p0.fX*d1) / dD,
                                                      (p1.fY*d0 - p0.fY*d1) / dD);

        SkScalar d0sq = d0*d0;
        SkVector dP = outerTangentIntersect - p0;
        SkScalar dPlenSq = dP.lengthSqd();
        SkScalar discrim = SkScalarSqrt(dPlenSq - d0sq);
        offset0->fX = p0.fX + (d0sq*dP.fX - side*d0*dP.fY*discrim) / dPlenSq;
        offset0->fY = p0.fY + (d0sq*dP.fY + side*d0*dP.fX*discrim) / dPlenSq;

        SkScalar d1sq = d1*d1;
        dP = outerTangentIntersect - p1;
        dPlenSq = dP.lengthSqd();
        discrim = SkScalarSqrt(dPlenSq - d1sq);
        offset1->fX = p1.fX + (d1sq*dP.fX - side*d1*dP.fY*discrim) / dPlenSq;
        offset1->fY = p1.fY + (d1sq*dP.fY + side*d1*dP.fX*discrim) / dPlenSq;
    }

    return true;
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
                          std::function<SkScalar(int index)> insetDistanceFunc,
                          SkTDArray<SkPoint>* insetPolygon) {
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
        SkOffsetSegment(inputPolygonVerts[i], inputPolygonVerts[j],
                        insetDistanceFunc(i), insetDistanceFunc(j),
                        winding,
                        &edgeData[i].fInset.fP0, &edgeData[i].fInset.fP1);
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

    // store all the valid intersections that aren't nearly coincident
    // TODO: look at the main algorithm and see if we can detect these better
    static constexpr SkScalar kCleanupTolerance = 0.01f;

    insetPolygon->reset();
    insetPolygon->setReserve(insetVertexCount);
    currIndex = -1;
    for (int i = 0; i < inputPolygonSize; ++i) {
        if (edgeData[i].fValid && (currIndex == -1 ||
            !edgeData[i].fIntersection.equalsWithinTolerance((*insetPolygon)[currIndex],
                                                             kCleanupTolerance))) {
            *insetPolygon->push() = edgeData[i].fIntersection;
            currIndex++;
        }
    }
    // make sure the first and last points aren't coincident
    if (currIndex >= 1 &&
        (*insetPolygon)[0].equalsWithinTolerance((*insetPolygon)[currIndex],
                                                 kCleanupTolerance)) {
        insetPolygon->pop();
    }
    SkASSERT(is_convex(*insetPolygon));

    return (insetPolygon->count() >= 3);
}
