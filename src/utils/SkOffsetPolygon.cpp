/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOffsetPolygon.h"

#include "SkPointPriv.h"
#include "SkTArray.h"
#include "SkTemplates.h"
#include "SkTDPQueue.h"

struct OffsetSegment {
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
        if (dD*dD >= SkPointPriv::DistanceToSqd(p0, p1)) {
            return false;
        }
        SkPoint outerTangentIntersect = SkPoint::Make((p1.fX*d0 - p0.fX*d1) / dD,
                                                      (p1.fY*d0 - p0.fY*d1) / dD);

        SkScalar d0sq = d0*d0;
        SkVector dP = outerTangentIntersect - p0;
        SkScalar dPlenSq = SkPointPriv::LengthSqd(dP);
        SkScalar discrim = SkScalarSqrt(dPlenSq - d0sq);
        offset0->fX = p0.fX + (d0sq*dP.fX - side*d0*dP.fY*discrim) / dPlenSq;
        offset0->fY = p0.fY + (d0sq*dP.fY + side*d0*dP.fX*discrim) / dPlenSq;

        SkScalar d1sq = d1*d1;
        dP = outerTangentIntersect - p1;
        dPlenSq = SkPointPriv::LengthSqd(dP);
        discrim = SkScalarSqrt(dPlenSq - d1sq);
        offset1->fX = p1.fX + (d1sq*dP.fX - side*d1*dP.fY*discrim) / dPlenSq;
        offset1->fY = p1.fY + (d1sq*dP.fY + side*d1*dP.fX*discrim) / dPlenSq;
    }

    return true;
}

// Compute the intersection 'p' between segments s0 and s1, if any.
// 's' is the parametric value for the intersection along 's0' & 't' is the same for 's1'.
// Returns false if there is no intersection.
static bool compute_intersection(const OffsetSegment& s0, const OffsetSegment& s1,
                                 SkPoint* p, SkScalar* s, SkScalar* t) {
    // Common cases for polygon chains -- check if endpoints are touching
    if (SkPointPriv::EqualsWithinTolerance(s0.fP1, s1.fP0)) {
        *p = s0.fP1;
        *s = SK_Scalar1;
        *t = 0;
        return true;
    }
    if (SkPointPriv::EqualsWithinTolerance(s1.fP1, s0.fP0)) {
        *p = s1.fP1;
        *s = 0;
        *t = SK_Scalar1;
        return true;
    }

    SkVector v0 = s0.fP1 - s0.fP0;
    SkVector v1 = s1.fP1 - s1.fP0;
    // We should have culled coincident points before this
    SkASSERT(!SkPointPriv::EqualsWithinTolerance(s0.fP0, s0.fP1));
    SkASSERT(!SkPointPriv::EqualsWithinTolerance(s1.fP0, s1.fP1));

    SkVector d = s1.fP0 - s0.fP0;
    SkScalar perpDot = v0.cross(v1);
    SkScalar localS, localT;
    if (SkScalarNearlyZero(perpDot)) {
        // segments are parallel, but not collinear
        if (!SkScalarNearlyZero(d.dot(d), SK_ScalarNearlyZero*SK_ScalarNearlyZero)) {
            return false;
        }

        // project segment1's endpoints onto segment0
        localS = d.fX / v0.fX;
        localT = 0;
        if (localS < 0 || localS > SK_Scalar1) {
            // the first endpoint doesn't lie on segment0, try the other one
            SkScalar oldLocalS = localS;
            localS = (s1.fP1.fX - s0.fP0.fX) / v0.fX;
            localT = SK_Scalar1;
            if (localS < 0 || localS > SK_Scalar1) {
                // it's possible that segment1's interval surrounds segment0
                // this is false if the params have the same signs, and in that case no collision
                if (localS*oldLocalS > 0) {
                    return false;
                }
                // otherwise project segment0's endpoint onto segment1 instead
                localS = 0;
                localT = -d.fX / v1.fX;
            }
        }
    } else {
        localS = d.cross(v1) / perpDot;
        if (localS < 0 || localS > SK_Scalar1) {
            return false;
        }
        localT = d.cross(v0) / perpDot;
        if (localT < 0 || localT > SK_Scalar1) {
            return false;
        }
    }

    v0 *= localS;
    *p = s0.fP0 + v0;
    *s = localS;
    *t = localT;

    return true;
}

// computes the line intersection and then the distance to s0's endpoint
static SkScalar compute_crossing_distance(const OffsetSegment& s0, const OffsetSegment& s1) {
    SkVector v0 = s0.fP1 - s0.fP0;
    SkVector v1 = s1.fP1 - s1.fP0;

    SkScalar perpDot = v0.cross(v1);
    if (SkScalarNearlyZero(perpDot)) {
        // segments are parallel
        return SK_ScalarMax;
    }

    SkVector d = s1.fP0 - s0.fP0;
    SkScalar localS = d.cross(v1) / perpDot;
    if (localS < 0) {
        localS = -localS;
    } else {
        localS -= SK_Scalar1;
    }

    localS *= v0.length();

    return localS;
}

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

struct EdgeData {
    OffsetSegment fInset;
    SkPoint       fIntersection;
    SkScalar      fTValue;
    uint16_t      fStart;
    uint16_t      fEnd;
    uint16_t      fIndex;
    bool          fValid;

    void init() {
        fIntersection = fInset.fP0;
        fTValue = SK_ScalarMin;
        fStart = 0;
        fEnd = 0;
        fIndex = 0;
        fValid = true;
    }

    void init(uint16_t start, uint16_t end) {
        fIntersection = fInset.fP0;
        fTValue = SK_ScalarMin;
        fStart = start;
        fEnd = end;
        fIndex = start;
        fValid = true;
    }
};

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
    SkAutoSTMalloc<64, EdgeData> edgeData(inputPolygonSize);
    for (int i = 0; i < inputPolygonSize; ++i) {
        int j = (i + 1) % inputPolygonSize;
        int k = (i + 2) % inputPolygonSize;
        // check for convexity just to be sure
        if (compute_side(inputPolygonVerts[i], inputPolygonVerts[j],
                         inputPolygonVerts[k])*winding < 0) {
            return false;
        }
        if (!SkOffsetSegment(inputPolygonVerts[i], inputPolygonVerts[j],
                             insetDistanceFunc(i), insetDistanceFunc(j),
                             winding,
                             &edgeData[i].fInset.fP0, &edgeData[i].fInset.fP1)) {
            return false;
        }
        edgeData[i].init();
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
                       SkPointPriv::EqualsWithinTolerance(intersection,
                                                          edgeData[currIndex].fIntersection,
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
    if (insetVertexCount >= 0) {
        insetPolygon->setReserve(insetVertexCount);
    }
    currIndex = -1;
    for (int i = 0; i < inputPolygonSize; ++i) {
        if (edgeData[i].fValid && (currIndex == -1 ||
            !SkPointPriv::EqualsWithinTolerance(edgeData[i].fIntersection,
                                                (*insetPolygon)[currIndex],
                                                kCleanupTolerance))) {
            *insetPolygon->push() = edgeData[i].fIntersection;
            currIndex++;
        }
    }
    // make sure the first and last points aren't coincident
    if (currIndex >= 1 &&
       SkPointPriv::EqualsWithinTolerance((*insetPolygon)[0], (*insetPolygon)[currIndex],
                                          kCleanupTolerance)) {
        insetPolygon->pop();
    }

    return (insetPolygon->count() >= 3 && is_convex(*insetPolygon));
}

// compute the number of points needed for a circular join when offsetting a  reflex vertex
static void compute_radial_steps(const SkVector& v1, const SkVector& v2, SkScalar r,
                                 SkScalar* rotSin, SkScalar* rotCos, int* n) {
    const SkScalar kRecipPixelsPerArcSegment = 0.25f;

    SkScalar rCos = v1.dot(v2);
    SkScalar rSin = v1.cross(v2);
    SkScalar theta = SkScalarATan2(rSin, rCos);

    int steps = SkScalarRoundToInt(SkScalarAbs(r*theta*kRecipPixelsPerArcSegment));

    SkScalar dTheta = theta / steps;
    *rotSin = SkScalarSinCos(dTheta, rotCos);
    *n = steps;
}

// tolerant less-than comparison
static inline bool nearly_lt(SkScalar a, SkScalar b, SkScalar tolerance = SK_ScalarNearlyZero) {
    return a < b - tolerance;
}

// a point is "left" to another if its x coordinate is less, or if equal, its y coordinate
static bool left(const SkPoint& p0, const SkPoint& p1) {
    return nearly_lt(p0.fX, p1.fX) ||
           (SkScalarNearlyEqual(p0.fX, p1.fX) && nearly_lt(p0.fY, p1.fY));
}

struct Vertex {
    static bool Left(const Vertex& qv0, const Vertex& qv1) {
        return left(qv0.fPosition, qv1.fPosition);
    }
    // packed to fit into 16 bytes (one cache line)
    SkPoint  fPosition;
    uint16_t fIndex;       // index in unsorted polygon
    uint16_t fPrevIndex;   // indices for previous and next vertex in unsorted polygon
    uint16_t fNextIndex;
    uint16_t fFlags;
};

enum VertexFlags {
    kPrevLeft_VertexFlag = 0x1,
    kNextLeft_VertexFlag = 0x2,
};

struct Edge {
    // returns true if "this" is above "that"
    bool above(const Edge& that, SkScalar tolerance = SK_ScalarNearlyZero) {
        SkASSERT(nearly_lt(this->fSegment.fP0.fX, that.fSegment.fP0.fX, tolerance) ||
                 SkScalarNearlyEqual(this->fSegment.fP0.fX, that.fSegment.fP0.fX, tolerance));
        // The idea here is that if the vector between the origins of the two segments (dv)
        // rotates counterclockwise up to the vector representing the "this" segment (u),
        // then we know that "this" is above that. If the result is clockwise we say it's below.
        SkVector dv = that.fSegment.fP0 - this->fSegment.fP0;
        SkVector u = this->fSegment.fP1 - this->fSegment.fP0;
        SkScalar cross = dv.cross(u);
        if (cross > tolerance) {
            return true;
        } else if (cross < -tolerance) {
            return false;
        }
        // If the result is 0 then either the two origins are equal or the origin of "that"
        // lies on dv. So then we try the same for the vector from the tail of "this"
        // to the head of "that". Again, ccw means "this" is above "that".
        dv = that.fSegment.fP1 - this->fSegment.fP0;
        return (dv.cross(u) > tolerance);
    }

    bool intersect(const Edge& that) const {
        SkPoint intersection;
        SkScalar s, t;
        // check first to see if these edges are neighbors in the polygon
        if (this->fIndex0 == that.fIndex0 || this->fIndex1 == that.fIndex0 ||
            this->fIndex0 == that.fIndex1 || this->fIndex1 == that.fIndex1) {
            return false;
        }
        return compute_intersection(this->fSegment, that.fSegment, &intersection, &s, &t);
    }

    bool operator==(const Edge& that) const {
        return (this->fIndex0 == that.fIndex0 && this->fIndex1 == that.fIndex1);
    }

    bool operator!=(const Edge& that) const {
        return !operator==(that);
    }

    OffsetSegment fSegment;
    int32_t fIndex0;   // indices for previous and next vertex
    int32_t fIndex1;
};

class EdgeList {
public:
    void reserve(int count) { fEdges.reserve(count); }

    bool insert(const Edge& newEdge) {
        // linear search for now (expected case is very few active edges)
        int insertIndex = 0;
        while (insertIndex < fEdges.count() && fEdges[insertIndex].above(newEdge)) {
            ++insertIndex;
        }
        // if we intersect with the existing edge above or below us
        // then we know this polygon is not simple, so don't insert, just fail
        if (insertIndex > 0 && newEdge.intersect(fEdges[insertIndex - 1])) {
            return false;
        }
        if (insertIndex < fEdges.count() && newEdge.intersect(fEdges[insertIndex])) {
            return false;
        }

        fEdges.push_back();
        for (int i = fEdges.count() - 1; i > insertIndex; --i) {
            fEdges[i] = fEdges[i - 1];
        }
        fEdges[insertIndex] = newEdge;

        return true;
    }

    bool remove(const Edge& edge) {
        SkASSERT(fEdges.count() > 0);

        // linear search for now (expected case is very few active edges)
        int removeIndex = 0;
        while (removeIndex < fEdges.count() && fEdges[removeIndex] != edge) {
            ++removeIndex;
        }
        // we'd better find it or something is wrong
        SkASSERT(removeIndex < fEdges.count());

        // if we intersect with the edge above or below us
        // then we know this polygon is not simple, so don't remove, just fail
        if (removeIndex > 0 && fEdges[removeIndex].intersect(fEdges[removeIndex-1])) {
            return false;
        }
        if (removeIndex < fEdges.count()-1) {
            if (fEdges[removeIndex].intersect(fEdges[removeIndex + 1])) {
                return false;
            }
            // copy over the old entry
            memmove(&fEdges[removeIndex], &fEdges[removeIndex + 1],
                    sizeof(Edge)*(fEdges.count() - removeIndex - 1));
        }

        fEdges.pop_back();
        return true;
    }

private:
    SkSTArray<1, Edge> fEdges;
};

// Here we implement a sweep line algorithm to determine whether the provided points
// represent a simple polygon, i.e., the polygon is non-self-intersecting.
// We first insert the vertices into a priority queue sorting horizontally from left to right.
// Then as we pop the vertices from the queue we generate events which indicate that an edge
// should be added or removed from an edge list. If any intersections are detected in the edge
// list, then we know the polygon is self-intersecting and hence not simple.
static bool is_simple_polygon(const SkPoint* polygon, int polygonSize) {
    SkTDPQueue <Vertex, Vertex::Left> vertexQueue;
    EdgeList sweepLine;

    sweepLine.reserve(polygonSize);
    for (int i = 0; i < polygonSize; ++i) {
        Vertex newVertex;
        newVertex.fPosition = polygon[i];
        newVertex.fIndex = i;
        newVertex.fPrevIndex = (i - 1 + polygonSize) % polygonSize;
        newVertex.fNextIndex = (i + 1) % polygonSize;
        newVertex.fFlags = 0;
        if (left(polygon[newVertex.fPrevIndex], polygon[i])) {
            newVertex.fFlags |= kPrevLeft_VertexFlag;
        }
        if (left(polygon[newVertex.fNextIndex], polygon[i])) {
            newVertex.fFlags |= kNextLeft_VertexFlag;
        }
        vertexQueue.insert(newVertex);
    }

    // pop each vertex from the queue and generate events depending on
    // where it lies relative to its neighboring edges
    while (vertexQueue.count() > 0) {
        const Vertex& v = vertexQueue.peek();

        // check edge to previous vertex
        if (v.fFlags & kPrevLeft_VertexFlag) {
            Edge edge{ { polygon[v.fPrevIndex], v.fPosition }, v.fPrevIndex, v.fIndex };
            if (!sweepLine.remove(edge)) {
                break;
            }
        } else {
            Edge edge{ { v.fPosition, polygon[v.fPrevIndex] }, v.fIndex, v.fPrevIndex };
            if (!sweepLine.insert(edge)) {
                break;
            }
        }

        // check edge to next vertex
        if (v.fFlags & kNextLeft_VertexFlag) {
            Edge edge{ { polygon[v.fNextIndex], v.fPosition }, v.fNextIndex, v.fIndex };
            if (!sweepLine.remove(edge)) {
                break;
            }
        } else {
            Edge edge{ { v.fPosition, polygon[v.fNextIndex] }, v.fIndex, v.fNextIndex };
            if (!sweepLine.insert(edge)) {
                break;
            }
        }

        vertexQueue.pop();
    }

    return (vertexQueue.count() == 0);
}

// TODO: assuming a constant offset here -- do we want to support variable offset?
bool SkOffsetSimplePolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                           SkScalar offset, SkTDArray<SkPoint>* offsetPolygon,
                           SkTDArray<int>* polygonIndices) {
    if (inputPolygonSize < 3) {
        return false;
    }

    if (!is_simple_polygon(inputPolygonVerts, inputPolygonSize)) {
        return false;
    }

    // compute area and use sign to determine winding
    // do initial pass to build normals
    SkAutoSTMalloc<64, SkVector> normals(inputPolygonSize);
    SkScalar quadArea = 0;
    for (int curr = 0; curr < inputPolygonSize; ++curr) {
        int next = (curr + 1) % inputPolygonSize;
        SkVector tangent = inputPolygonVerts[next] - inputPolygonVerts[curr];
        SkVector normal = SkVector::Make(-tangent.fY, tangent.fX);
        normals[curr] = normal;
        quadArea += inputPolygonVerts[curr].cross(inputPolygonVerts[next]);
    }
    // 1 == ccw, -1 == cw
    int winding = (quadArea > 0) ? 1 : -1;
    if (0 == winding) {
        return false;
    }

    // resize normals to match offset
    for (int curr = 0; curr < inputPolygonSize; ++curr) {
        normals[curr].setLength(winding*offset);
    }

    // build initial offset edge list
    SkSTArray<64, EdgeData> edgeData(inputPolygonSize);
    int prevIndex = inputPolygonSize - 1;
    int currIndex = 0;
    int nextIndex = 1;
    while (currIndex < inputPolygonSize) {
        int side = compute_side(inputPolygonVerts[prevIndex],
                                inputPolygonVerts[currIndex],
                                inputPolygonVerts[nextIndex]);

        // if reflex point, fill in curve
        if (side*winding*offset < 0) {
            SkScalar rotSin, rotCos;
            int numSteps;
            SkVector prevNormal = normals[prevIndex];
            compute_radial_steps(prevNormal, normals[currIndex], SkScalarAbs(offset),
                                 &rotSin, &rotCos, &numSteps);
            for (int i = 0; i < numSteps - 1; ++i) {
                SkVector currNormal = SkVector::Make(prevNormal.fX*rotCos - prevNormal.fY*rotSin,
                                                     prevNormal.fY*rotCos + prevNormal.fX*rotSin);
                EdgeData& edge = edgeData.push_back();
                edge.fInset.fP0 = inputPolygonVerts[currIndex] + prevNormal;
                edge.fInset.fP1 = inputPolygonVerts[currIndex] + currNormal;
                edge.init(currIndex, currIndex);
                prevNormal = currNormal;
            }
            EdgeData& edge = edgeData.push_back();
            edge.fInset.fP0 = inputPolygonVerts[currIndex] + prevNormal;
            edge.fInset.fP1 = inputPolygonVerts[currIndex] + normals[currIndex];
            edge.init(currIndex, currIndex);
        }

        // Add the edge
        EdgeData& edge = edgeData.push_back();
        edge.fInset.fP0 = inputPolygonVerts[currIndex] + normals[currIndex];
        edge.fInset.fP1 = inputPolygonVerts[nextIndex] + normals[currIndex];
        edge.init(currIndex, nextIndex);

        prevIndex = currIndex;
        currIndex++;
        nextIndex = (nextIndex + 1) % inputPolygonSize;
    }

    int edgeDataSize = edgeData.count();
    prevIndex = edgeDataSize - 1;
    currIndex = 0;
    int insetVertexCount = edgeDataSize;
    while (prevIndex != currIndex) {
        if (!edgeData[prevIndex].fValid) {
            prevIndex = (prevIndex + edgeDataSize - 1) % edgeDataSize;
            continue;
        }
        if (!edgeData[currIndex].fValid) {
            currIndex = (currIndex + 1) % edgeDataSize;
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
                prevIndex = (prevIndex + edgeDataSize - 1) % edgeDataSize;
                // we've already considered this intersection, we're done
            } else if (edgeData[currIndex].fTValue > SK_ScalarMin &&
                       SkPointPriv::EqualsWithinTolerance(intersection,
                                                          edgeData[currIndex].fIntersection,
                                                          1.0e-6f)) {
                break;
            } else {
                // add intersection
                edgeData[currIndex].fIntersection = intersection;
                edgeData[currIndex].fTValue = t;
                edgeData[currIndex].fIndex = edgeData[prevIndex].fEnd;

                // go to next segment
                prevIndex = currIndex;
                currIndex = (currIndex + 1) % edgeDataSize;
            }
        } else {
            // If there is no intersection, we want to minimize the distance between
            // the point where the segment lines cross and the segments themselves.
            SkScalar prevPrevIndex = (prevIndex + edgeDataSize - 1) % edgeDataSize;
            SkScalar currNextIndex = (currIndex + 1) % edgeDataSize;
            SkScalar dist0 = compute_crossing_distance(edgeData[currIndex].fInset,
                                                       edgeData[prevPrevIndex].fInset);
            SkScalar dist1 = compute_crossing_distance(edgeData[prevIndex].fInset,
                                                       edgeData[currNextIndex].fInset);
            if (dist0 < dist1) {
                edgeData[prevIndex].fValid = false;
                prevIndex = prevPrevIndex;
            } else {
                edgeData[currIndex].fValid = false;
                currIndex = currNextIndex;
            }
            --insetVertexCount;
        }
    }

    // store all the valid intersections that aren't nearly coincident
    // TODO: look at the main algorithm and see if we can detect these better
    static constexpr SkScalar kCleanupTolerance = 0.01f;

    offsetPolygon->reset();
    offsetPolygon->setReserve(insetVertexCount);
    currIndex = -1;
    for (int i = 0; i < edgeData.count(); ++i) {
        if (edgeData[i].fValid && (currIndex == -1 ||
                                   !SkPointPriv::EqualsWithinTolerance(edgeData[i].fIntersection,
                                                                       (*offsetPolygon)[currIndex],
                                                                       kCleanupTolerance))) {
            *offsetPolygon->push() = edgeData[i].fIntersection;
            if (polygonIndices) {
                *polygonIndices->push() = edgeData[i].fIndex;
            }
            currIndex++;
        }
    }
    // make sure the first and last points aren't coincident
    if (currIndex >= 1 &&
        SkPointPriv::EqualsWithinTolerance((*offsetPolygon)[0], (*offsetPolygon)[currIndex],
                                           kCleanupTolerance)) {
        offsetPolygon->pop();
        if (polygonIndices) {
            polygonIndices->pop();
        }
    }

    // compute signed area to check winding (it should be same as the original polygon)
    quadArea = 0;
    for (int curr = 0; curr < offsetPolygon->count(); ++curr) {
        int next = (curr + 1) % offsetPolygon->count();
        quadArea += (*offsetPolygon)[curr].cross((*offsetPolygon)[next]);
    }

    return (winding*quadArea > 0 &&
            is_simple_polygon(offsetPolygon->begin(), offsetPolygon->count()));
}

