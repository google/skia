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
    // check if endpoints are touching
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
    bool          fValid;
    int           fWinding;

    void init() {
        fIntersection = fInset.fP0;
        fTValue = SK_ScalarMin;
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
        SkOffsetSegment(inputPolygonVerts[i], inputPolygonVerts[j],
                        insetDistanceFunc(i), insetDistanceFunc(j),
                        winding,
                        &edgeData[i].fInset.fP0, &edgeData[i].fInset.fP1);
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
    insetPolygon->setReserve(insetVertexCount);
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


static void compute_radial_steps(const SkVector& v1, const SkVector& v2, SkScalar r,
                                 SkScalar* rotSin, SkScalar* rotCos, int* n) {
    const SkScalar kRecipPixelsPerArcSegment = 0.25f;

    SkScalar rCos = v1.dot(v2);
    SkScalar rSin = v1.cross(v2);
    SkScalar theta = SkScalarATan2(rSin, rCos);

    int steps = SkScalarFloorToInt(SkScalarAbs(r*theta*kRecipPixelsPerArcSegment));

    SkScalar dTheta = theta / steps;
    *rotSin = SkScalarSinCos(dTheta, rotCos);
    *n = steps;
}

static inline bool relative_eq(SkScalar a, SkScalar b, SkScalar tolerance = SK_ScalarNearlyZero) {
    return SkScalarAbs(a - b) <= (SkScalarAbs(a) + SkScalarAbs(b) + 1)*tolerance;
}

static inline bool relative_lt(SkScalar a, SkScalar b, SkScalar tolerance = SK_ScalarNearlyZero) {
    return a < b - (SkScalarAbs(a) + SkScalarAbs(b) + 1)*tolerance;
}

static bool left(const SkPoint& p0, const SkPoint& p1) {
    return relative_lt(p0.fX, p1.fX) || (relative_eq(p0.fX, p1.fX) && relative_lt(p0.fY, p1.fY));
}

// packed to fit into 16 bytes (one cache line)
struct Vertex {
    SkPoint  fPosition;
    uint16_t fPrevIndex;   // indices for previous and next vertex
    uint16_t fNextIndex;
    uint32_t fFlags;
};

enum VertexFlags {
    kPrevRight_VertexFlag = 0x1,
    kNextRight_VertexFlag = 0x2,

    kLeftLeftMask = 0x0,
    kRightLeftMask = 0x1,
    kLeftRightMask = 0x2,
    kRightRightMask = 0x3,
};

struct QueueVertex {
    static bool Left(const QueueVertex& qv0, const QueueVertex& qv1) {
        return ::left(qv0.fPosition, qv1.fPosition);
    }
    SkPoint  fPosition;
    int32_t fUnsortedIndex;  // index of vertex in unsorted list
};

struct Edge {
    static bool Above(const Edge& e0, const Edge& e1) {  // true if e0 above e1
        SkASSERT(relative_lt(e0.fSegment.fP0.fX, e1.fSegment.fP0.fX) ||
                 relative_eq(e0.fSegment.fP0.fX, e1.fSegment.fP0.fX));
        SkScalar dx = e0.fSegment.fP1.fX - e0.fSegment.fP0.fX;
        SkScalar dy = e0.fSegment.fP1.fY - e0.fSegment.fP0.fY;
        SkScalar sweepIntercept = e0.fSegment.fP0.fY +
                                  (e1.fSegment.fP0.fX - e0.fSegment.fP0.fX)*dy / dx;
        return sweepIntercept > e1.fSegment.fP0.fY;
    }

    bool intersection(const Edge& e0, const Edge& e1, SkPoint* intersection) {
        // don't detect intersections between neighboring edges
        if (e0.fIndex0 == e1.fIndex1 || e0.fIndex1 == e1.fIndex0) {
            return false;
        }

        SkScalar s, t;
        return compute_intersection(e0.fSegment, e1.fSegment, intersection, &s, &t);
    }

    bool operator==(const Edge& that) {
        return (this->fIndex0 == that.fIndex0 && this->fIndex1 == that.fIndex1);
    }

    bool operator!=(const Edge& that) {
        return !operator==(that);
    }


    OffsetSegment fSegment;
    int32_t fIndex0;   // indices for previous and next vertex
    int32_t fIndex1;
};

class EdgeList {
public:
    void reserve(int count) { fEdges.reserve(count); }

    void insert(const Edge& newEdge) {

        //**** linear search for now
        int insertIndex = 0;
        fEdges.emplace_back();
        while (insertIndex < fEdges.count()-1 && Edge::Above(fEdges[insertIndex], newEdge)) {
            ++insertIndex;
        }
        //**** hm, lots of copying here
        for (int i = insertIndex + 1; i < fEdges.count(); ++i) {
            fEdges[i] = fEdges[i - 1];
        }
        fEdges[insertIndex] = newEdge;
    }

    void remove(const Edge& edge) {
        SkASSERT(fEdges.count() > 0);

        //**** linear search for now
        int removeIndex = 0;
        while (removeIndex < fEdges.count() && fEdges[removeIndex] != edge) {
            ++removeIndex;
        }
        //**** hm, lots of copying here
        for (int i = removeIndex; i < fEdges.count()-1; ++i) {
            fEdges[i] = fEdges[i+1];
        }

        fEdges.pop_back();
    }

private:
    //*** why not an SkTDArray? On stack?
    SkSTArray<1, Edge> fEdges;
};

/*
static void handle_intersection() {
}

static bool simplify_polygon(SkTDArray<SkPoint>* offsetPolygon) {
    SkTDArray<Vertex> unsortedVertices;
    SkTDPQueue <QueueVertex, QueueVertex::Left> vertexQueue;
    EdgeList sweepLine;
    SkTDArray<int> contours;

    // should generate this as part of SkOffsetPolygon rather than run through polygon twice?
    int vertexCount = offsetPolygon->count();
    int expectedIntersections = (vertexCount - 3)*(vertexCount - 3) / 2;
    unsortedVertices.setReserve(vertexCount + expectedIntersections);
    unsortedVertices.setCount(vertexCount);
    sweepLine.reserve(vertexCount + expectedIntersections);
    contours.setReserve(1 + expectedIntersections);
    contours.setCount(1);
    for (int i = 0; i < vertexCount; ++i) {
        unsortedVertices[i].fPosition = (*offsetPolygon)[i];
        unsortedVertices[i].fPrevIndex = (i - 1 + vertexCount) % vertexCount;
        unsortedVertices[i].fNextIndex = (i + 1) % vertexCount;
        unsortedVertices[i].fFlags = 0;
        if (i > 0) {
            if (left(unsortedVertices[i - 1].fPosition, unsortedVertices[i].fPosition)) {
                unsortedVertices[i - 1].fFlags |= kNextRight_VertexFlag;
            } else {
                unsortedVertices[i].fFlags |= kPrevRight_VertexFlag;
            }
        }
        vertexQueue.insert(QueueVertex{ (*offsetPolygon)[i], i });
    }
    if (left(unsortedVertices[vertexCount - 1].fPosition, unsortedVertices[0].fPosition)) {
        unsortedVertices[vertexCount - 1].fFlags |= kNextRight_VertexFlag;
    } else {
        unsortedVertices[0].fFlags |= kPrevRight_VertexFlag;
    }

    // initialize contour list
    contours[0] = 0;

    // iterate through
    while (vertexQueue.count() > 0) {
        const QueueVertex& qv = vertexQueue.peek();
        const Vertex& v = unsortedVertices[qv.fUnsortedIndex];

        // check edge to previous vertex
        if (v.fFlags & kPrevRight_VertexFlag) {
            Edge edge{ { v.fPosition, unsortedVertices[v.fPrevIndex].fPosition },
                       qv.fUnsortedIndex, v.fPrevIndex };
            sweepLine.insert(edge);

            // handle intersections

        } else {
            Edge edge{ { unsortedVertices[v.fPrevIndex].fPosition, v.fPosition },
                        v.fPrevIndex, qv.fUnsortedIndex };
            sweepLine.remove(edge);
        }

        // check edge to next vertex
        if (v.fFlags & kNextRight_VertexFlag) {
            Edge edge{ { v.fPosition, unsortedVertices[v.fNextIndex].fPosition },
                qv.fUnsortedIndex, v.fNextIndex };
            sweepLine.insert(edge);

            // handle intersections

        } else {
            Edge edge{ { unsortedVertices[v.fNextIndex].fPosition, v.fPosition },
                v.fNextIndex, qv.fUnsortedIndex };
            sweepLine.remove(edge);
        }

        vertexQueue.pop();
    }

    // remove any contours that have negative area

    if (contours.count() != 1) {
        return false;
    }

    // stuff the result back into the polygon

    return true;
}
*/
// TODO: assuming a constant offset here -- do we want to support variable offset?
bool SkOffsetPolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                     SkScalar offset, SkTDArray<SkPoint>* offsetPolygon) {
    if (inputPolygonSize < 3) {
        return false;
    }

    SkAutoSTMalloc<64, SkVector> normals(inputPolygonSize);
    SkScalar quadArea = 0;
    for (int curr = 0; curr < inputPolygonSize; ++curr) {
        int next = (curr + 1) % inputPolygonSize;
        SkVector tangent = inputPolygonVerts[next] - inputPolygonVerts[curr];
        SkVector normal = SkVector::Make(tangent.fY, -tangent.fX);
        normal.setLength(offset);
        normals[curr] = normal;
        quadArea = inputPolygonVerts[curr].cross(inputPolygonVerts[next]);
    }
    // 1 == ccw, -1 == cw
    int winding = (quadArea > 0) ? 1 : -1;
    if (0 == winding) {
        return false;
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

        int localWinding = (offset > 0) ? -winding : winding;

        // if reflex point, fill in curve
        if (side*winding*offset > 0) {
            localWinding = -localWinding;
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
                edge.fWinding = localWinding;
                edge.init();
                prevNormal = currNormal;
            }
            EdgeData& edge = edgeData.push_back();
            edge.fInset.fP0 = inputPolygonVerts[currIndex] + prevNormal;
            edge.fInset.fP1 = inputPolygonVerts[currIndex] + normals[currIndex];
            edge.fWinding = localWinding;
            edge.init();
        }

        EdgeData& edge = edgeData.push_back();
        edge.fInset.fP0 = inputPolygonVerts[currIndex] + normals[currIndex];
        edge.fInset.fP1 = inputPolygonVerts[nextIndex] + normals[currIndex];
        edge.fWinding = localWinding;
        edge.init();

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

                // go to next segment
                prevIndex = currIndex;
                currIndex = (currIndex + 1) % edgeDataSize;
            }
        } else {
            // store vector instead of p1?
            SkVector vPrev = edgeData[prevIndex].fInset.fP1 - edgeData[prevIndex].fInset.fP0;
            SkVector vCurr = edgeData[currIndex].fInset.fP1 - edgeData[currIndex].fInset.fP0;
            // if pointing in opposite directions, dealing with a collapse
            if (vPrev.dot(vCurr) < 0) {
                SkScalar prevPrevIndex = (prevIndex + edgeDataSize - 1) % edgeDataSize;
                SkScalar currNextIndex = (currIndex + 1) % edgeDataSize;
                SkScalar dist0 = SkPoint::Distance(edgeData[prevIndex].fInset.fP0,
                                                   edgeData[currNextIndex].fInset.fP1);
                SkScalar dist1 = SkPoint::Distance(edgeData[prevPrevIndex].fInset.fP0,
                                                   edgeData[currIndex].fInset.fP1);
                if (dist0 > dist1) {
                    edgeData[prevIndex].fValid = false;
                    prevIndex = prevPrevIndex;
                } else {
                    edgeData[currIndex].fValid = false;
                    currIndex = currNextIndex;
                }
                --insetVertexCount;
            } else {
                // if prev to right side of curr
                int nextIndex = (prevIndex + 1) % edgeDataSize;
                int prevWinding = edgeData[nextIndex].fWinding;
                int currWinding = edgeData[currIndex].fWinding;
                int localWinding = (prevWinding == 1 || currWinding == 1) ? 1 : -1;
                int side = localWinding * compute_side(edgeData[currIndex].fInset.fP0,
                                                       edgeData[currIndex].fInset.fP1,
                                                       edgeData[prevIndex].fInset.fP1);
                if (side < 0 && side == localWinding * compute_side(edgeData[currIndex].fInset.fP0,
                                                                    edgeData[currIndex].fInset.fP1,
                                                                    edgeData[prevIndex].fInset.fP0)) {
                    // no point in considering this one again
                    edgeData[prevIndex].fValid = false;
                    --insetVertexCount;
                    // go back one segment
                    prevIndex = (prevIndex + edgeDataSize - 1) % edgeDataSize;
                } else {
                    // move to next segment
                    edgeData[currIndex].fValid = false;
                    --insetVertexCount;
                    currIndex = (currIndex + 1) % edgeDataSize;
                }
            }
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
            currIndex++;
        }
        //*offsetPolygon->push() = edgeData[i].fInset.fP0;
        //*offsetPolygon->push() = edgeData[i].fInset.fP1;
    }
    // make sure the first and last points aren't coincident
    if (currIndex >= 1 &&
        SkPointPriv::EqualsWithinTolerance((*offsetPolygon)[0], (*offsetPolygon)[currIndex],
                                           kCleanupTolerance)) {
        offsetPolygon->pop();
    }

    return (offsetPolygon->count() >= 3 && is_convex(*offsetPolygon));
}

