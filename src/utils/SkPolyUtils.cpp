/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPolyUtils.h"

#include "SkPointPriv.h"
#include "SkTArray.h"
#include "SkTemplates.h"
#include "SkTDPQueue.h"
#include "SkTInternalLList.h"

//////////////////////////////////////////////////////////////////////////////////
// Helper data structures and functions

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

// Returns 1 for cw, -1 for ccw and 0 if zero signed area (either degenerate or self-intersecting)
int SkGetPolygonWinding(const SkPoint* polygonVerts, int polygonSize) {
    if (polygonSize < 3) {
        return 0;
    }

    // compute area and use sign to determine winding
    SkScalar quadArea = 0;
    SkVector v0 = polygonVerts[1] - polygonVerts[0];
    for (int curr = 1; curr < polygonSize - 1; ++curr) {
        int next = (curr + 1) % polygonSize;
        SkVector v1 = polygonVerts[next] - polygonVerts[0];
        quadArea += v0.cross(v1);
        v0 = v1;
    }
    if (SkScalarNearlyZero(quadArea)) {
        return 0;
    }
    // 1 == ccw, -1 == cw
    return (quadArea > 0) ? 1 : -1;
}

// Helper function to compute the individual vector for non-equal offsets
inline void compute_offset(SkScalar d, const SkPoint& polyPoint, int side,
                           const SkPoint& outerTangentIntersect, SkVector* v) {
    SkScalar dsq = d * d;
    SkVector dP = outerTangentIntersect - polyPoint;
    SkScalar dPlenSq = SkPointPriv::LengthSqd(dP);
    if (SkScalarNearlyZero(dPlenSq)) {
        v->set(0, 0);
    } else {
        SkScalar discrim = SkScalarSqrt(dPlenSq - dsq);
        v->fX = (dsq*dP.fX - side * d*dP.fY*discrim) / dPlenSq;
        v->fY = (dsq*dP.fY + side * d*dP.fX*discrim) / dPlenSq;
    }
}

// Compute difference vector to offset p0-p1 'd0' and 'd1' units in direction specified by 'side'
bool compute_offset_vectors(const SkPoint& p0, const SkPoint& p1, SkScalar d0, SkScalar d1,
                            int side, SkPoint* vector0, SkPoint* vector1) {
    SkASSERT(side == -1 || side == 1);
    if (SkScalarNearlyEqual(d0, d1)) {
        // if distances are equal, can just outset by the perpendicular
        SkVector perp = SkVector::Make(p0.fY - p1.fY, p1.fX - p0.fX);
        perp.setLength(d0*side);
        *vector0 = perp;
        *vector1 = perp;
    } else {
        SkScalar d0abs = SkTAbs(d0);
        SkScalar d1abs = SkTAbs(d1);
        // Otherwise we need to compute the outer tangent.
        // See: http://www.ambrsoft.com/TrigoCalc/Circles2/Circles2Tangent_.htm
        if (d0abs < d1abs) {
            side = -side;
        }
        SkScalar dD = d0abs - d1abs;
        // if one circle is inside another, we can't compute an offset
        if (dD*dD >= SkPointPriv::DistanceToSqd(p0, p1)) {
            return false;
        }
        SkPoint outerTangentIntersect = SkPoint::Make((p1.fX*d0abs - p0.fX*d1abs) / dD,
                                                      (p1.fY*d0abs - p0.fY*d1abs) / dD);

        compute_offset(d0, p0, side, outerTangentIntersect, vector0);
        compute_offset(d1, p1, side, outerTangentIntersect, vector1);
    }

    return true;
}

// Offset line segment p0-p1 'd0' and 'd1' units in the direction specified by 'side'
bool SkOffsetSegment(const SkPoint& p0, const SkPoint& p1, SkScalar d0, SkScalar d1,
                     int side, SkPoint* offset0, SkPoint* offset1) {
    SkVector v0, v1;
    if (!compute_offset_vectors(p0, p1, d0, d1, side, &v0, &v1)) {
        return false;
    }
    *offset0 = p0 + v0;
    *offset1 = p1 + v1;

    return true;
}

// compute fraction of d along v
static inline SkScalar compute_param(const SkVector& v, const SkVector& d) {
    if (SkScalarNearlyZero(v.fX)) {
        return d.fY / v.fY;
    } else {
        return d.fX / v.fX;
    }
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
    SkVector d = s1.fP0 - s0.fP0;
    SkScalar perpDot = v0.cross(v1);
    SkScalar localS, localT;
    if (SkScalarNearlyZero(perpDot)) {
        // segments are parallel, but not collinear
        if (!SkScalarNearlyZero(d.cross(v0)) || !SkScalarNearlyZero(d.cross(v1))) {
            return false;
        }

        // Check for degenerate segments
        if (!SkPointPriv::CanNormalize(v0.fX, v0.fY)) {
            // Both are degenerate
            if (!SkPointPriv::CanNormalize(v1.fX, v1.fY)) {
                // Check if they're the same point
                if (!SkPointPriv::CanNormalize(d.fX, d.fY)) {
                    *p = s0.fP0;
                    *s = 0;
                    *t = 0;
                    return true;
                } else {
                    return false;
                }
            }
            // Otherwise project onto segment1
            localT = compute_param(v1, -d);
            if (localT < 0 || localT > SK_Scalar1) {
                return false;
            }
            localS = 0;
        } else {
            // Project segment1's endpoints onto segment0
            localS = compute_param(v0, d);
            localT = 0;
            if (localS < 0 || localS > SK_Scalar1) {
                // The first endpoint doesn't lie on segment0
                // If segment1 is degenerate, then there's no collision
                if (!SkPointPriv::CanNormalize(v1.fX, v1.fY)) {
                    return false;
                }

                // Otherwise try the other one
                SkScalar oldLocalS = localS;
                localS = compute_param(v0, s1.fP1 - s0.fP0);
                localT = SK_Scalar1;
                if (localS < 0 || localS > SK_Scalar1) {
                    // it's possible that segment1's interval surrounds segment0
                    // this is false if params have the same signs, and in that case no collision
                    if (localS*oldLocalS > 0) {
                        return false;
                    }
                    // otherwise project segment0's endpoint onto segment1 instead
                    localS = 0;
                    localT = compute_param(v1, -d);
                }
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

    *p = s0.fP0 + v0*localS;
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

bool SkIsConvexPolygon(const SkPoint* polygonVerts, int polygonSize) {
    if (polygonSize < 3) {
        return false;
    }

    SkScalar lastArea = 0;
    SkScalar lastPerpDot = 0;

    int prevIndex = polygonSize - 1;
    int currIndex = 0;
    int nextIndex = 1;
    SkPoint origin = polygonVerts[0];
    SkVector v0 = polygonVerts[currIndex] - polygonVerts[prevIndex];
    SkVector v1 = polygonVerts[nextIndex] - polygonVerts[currIndex];
    SkVector w0 = polygonVerts[currIndex] - origin;
    SkVector w1 = polygonVerts[nextIndex] - origin;
    for (int i = 0; i < polygonSize; ++i) {
        if (!polygonVerts[i].isFinite()) {
            return false;
        }

        // Check that winding direction is always the same (otherwise we have a reflex vertex)
        SkScalar perpDot = v0.cross(v1);
        if (lastPerpDot*perpDot < 0) {
            return false;
        }
        if (0 != perpDot) {
            lastPerpDot = perpDot;
        }

        // If the signed area ever flips it's concave
        // TODO: see if we can verify convexity only with signed area
        SkScalar quadArea = w0.cross(w1);
        if (quadArea*lastArea < 0) {
            return false;
        }
        if (0 != quadArea) {
            lastArea = quadArea;
        }

        prevIndex = currIndex;
        currIndex = nextIndex;
        nextIndex = (currIndex + 1) % polygonSize;
        v0 = v1;
        v1 = polygonVerts[nextIndex] - polygonVerts[currIndex];
        w0 = w1;
        w1 = polygonVerts[nextIndex] - origin;
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

//////////////////////////////////////////////////////////////////////////////////

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
                          std::function<SkScalar(const SkPoint&)> insetDistanceFunc,
                          SkTDArray<SkPoint>* insetPolygon) {
    if (inputPolygonSize < 3) {
        return false;
    }

    // get winding direction
    int winding = SkGetPolygonWinding(inputPolygonVerts, inputPolygonSize);
    if (0 == winding) {
        return false;
    }

    // set up
    SkAutoSTMalloc<64, EdgeData> edgeData(inputPolygonSize);
    for (int i = 0; i < inputPolygonSize; ++i) {
        int j = (i + 1) % inputPolygonSize;
        int k = (i + 2) % inputPolygonSize;
        if (!inputPolygonVerts[i].isFinite()) {
            return false;
        }
        // check for convexity just to be sure
        if (compute_side(inputPolygonVerts[i], inputPolygonVerts[j],
                         inputPolygonVerts[k])*winding < 0) {
            return false;
        }
        if (!SkOffsetSegment(inputPolygonVerts[i], inputPolygonVerts[j],
                             insetDistanceFunc(inputPolygonVerts[i]),
                             insetDistanceFunc(inputPolygonVerts[j]),
                             winding,
                             &edgeData[i].fInset.fP0, &edgeData[i].fInset.fP1)) {
            return false;
        }
        edgeData[i].init();
    }

    int prevIndex = inputPolygonSize - 1;
    int currIndex = 0;
    int insetVertexCount = inputPolygonSize;
    int iterations = 0;
    while (prevIndex != currIndex) {
        ++iterations;
        // we should check each edge against each other edge at most once
        if (iterations > inputPolygonSize*inputPolygonSize) {
            return false;
        }

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

    return SkIsConvexPolygon(insetPolygon->begin(), insetPolygon->count());
}

///////////////////////////////////////////////////////////////////////////////////////////

// compute the number of points needed for a circular join when offsetting a reflex vertex
bool SkComputeRadialSteps(const SkVector& v1, const SkVector& v2, SkScalar r,
                          SkScalar* rotSin, SkScalar* rotCos, int* n) {
    const SkScalar kRecipPixelsPerArcSegment = 0.25f;

    SkScalar rCos = v1.dot(v2);
    if (!SkScalarIsFinite(rCos)) {
        return false;
    }
    SkScalar rSin = v1.cross(v2);
    if (!SkScalarIsFinite(rSin)) {
        return false;
    }
    SkScalar theta = SkScalarATan2(rSin, rCos);

    int steps = SkScalarRoundToInt(SkScalarAbs(r*theta*kRecipPixelsPerArcSegment));

    SkScalar dTheta = steps > 0 ? theta / steps : 0;
    *rotSin = SkScalarSinCos(dTheta, rotCos);
    *n = steps;
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

// a point is "left" to another if its x coordinate is less, or if equal, its y coordinate
static bool left(const SkPoint& p0, const SkPoint& p1) {
    return p0.fX < p1.fX || (!(p0.fX > p1.fX) && p0.fY < p1.fY);
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
        SkASSERT(this->fSegment.fP0.fX < that.fSegment.fP0.fX ||
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
        if (removeIndex > 0 && fEdges[removeIndex].intersect(fEdges[removeIndex - 1])) {
            return false;
        }
        if (removeIndex < fEdges.count() - 1) {
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
bool SkIsSimplePolygon(const SkPoint* polygon, int polygonSize) {
    if (polygonSize < 3) {
        return false;
    }

    SkTDPQueue <Vertex, Vertex::Left> vertexQueue;
    EdgeList sweepLine;

    sweepLine.reserve(polygonSize);
    for (int i = 0; i < polygonSize; ++i) {
        Vertex newVertex;
        if (!polygon[i].isFinite()) {
            return false;
        }
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

///////////////////////////////////////////////////////////////////////////////////////////

bool SkOffsetSimplePolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                           std::function<SkScalar(const SkPoint&)> offsetDistanceFunc,
                           SkTDArray<SkPoint>* offsetPolygon, SkTDArray<int>* polygonIndices) {
    if (inputPolygonSize < 3) {
        return false;
    }

    // get winding direction
    int winding = SkGetPolygonWinding(inputPolygonVerts, inputPolygonSize);
    if (0 == winding) {
        return false;
    }

    // build normals
    SkAutoSTMalloc<64, SkVector> normal0(inputPolygonSize);
    SkAutoSTMalloc<64, SkVector> normal1(inputPolygonSize);
    SkScalar currOffset = offsetDistanceFunc(inputPolygonVerts[0]);
    if (!SkScalarIsFinite(currOffset)) {
        return false;
    }
    for (int curr = 0; curr < inputPolygonSize; ++curr) {
        if (!inputPolygonVerts[curr].isFinite()) {
            return false;
        }
        int next = (curr + 1) % inputPolygonSize;
        SkScalar nextOffset = offsetDistanceFunc(inputPolygonVerts[next]);
        if (!SkScalarIsFinite(nextOffset)) {
            return false;
        }
        if (!compute_offset_vectors(inputPolygonVerts[curr], inputPolygonVerts[next],
                                    currOffset, nextOffset, winding,
                                    &normal0[curr], &normal1[next])) {
            return false;
        }
        currOffset = nextOffset;
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
        SkScalar offset = offsetDistanceFunc(inputPolygonVerts[currIndex]);
        // if reflex point, fill in curve
        if (side*winding*offset < 0) {
            SkScalar rotSin, rotCos;
            int numSteps;
            SkVector prevNormal = normal1[currIndex];
            if (!SkComputeRadialSteps(prevNormal, normal0[currIndex], SkScalarAbs(offset),
                                      &rotSin, &rotCos, &numSteps)) {
                return false;
            }
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
            edge.fInset.fP1 = inputPolygonVerts[currIndex] + normal0[currIndex];
            edge.init(currIndex, currIndex);
        }

        // Add the edge
        EdgeData& edge = edgeData.push_back();
        edge.fInset.fP0 = inputPolygonVerts[currIndex] + normal0[currIndex];
        edge.fInset.fP1 = inputPolygonVerts[nextIndex] + normal1[nextIndex];
        edge.init(currIndex, nextIndex);

        prevIndex = currIndex;
        currIndex++;
        nextIndex = (nextIndex + 1) % inputPolygonSize;
    }

    int edgeDataSize = edgeData.count();
    prevIndex = edgeDataSize - 1;
    currIndex = 0;
    int insetVertexCount = edgeDataSize;
    int iterations = 0;
    while (prevIndex != currIndex) {
        ++iterations;
        // we should check each edge against each other edge at most once
        if (iterations > edgeDataSize*edgeDataSize) {
            return false;
        }

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

    // check winding of offset polygon (it should be same as the original polygon)
    SkScalar offsetWinding = SkGetPolygonWinding(offsetPolygon->begin(), offsetPolygon->count());

    return (winding*offsetWinding > 0 &&
            SkIsSimplePolygon(offsetPolygon->begin(), offsetPolygon->count()));
}

//////////////////////////////////////////////////////////////////////////////////////////

struct TriangulationVertex {
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(TriangulationVertex);

    enum class VertexType { kConvex, kReflex };

    SkPoint    fPosition;
    VertexType fVertexType;
    uint16_t   fIndex;
    uint16_t   fPrevIndex;
    uint16_t   fNextIndex;
};

// test to see if point p is in triangle p0p1p2.
// for now assuming strictly inside -- if on the edge it's outside
static bool point_in_triangle(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                              const SkPoint& p) {
    SkVector v0 = p1 - p0;
    SkVector v1 = p2 - p1;
    SkScalar n = v0.cross(v1);

    SkVector w0 = p - p0;
    if (n*v0.cross(w0) < SK_ScalarNearlyZero) {
        return false;
    }

    SkVector w1 = p - p1;
    if (n*v1.cross(w1) < SK_ScalarNearlyZero) {
        return false;
    }

    SkVector v2 = p0 - p2;
    SkVector w2 = p - p2;
    if (n*v2.cross(w2) < SK_ScalarNearlyZero) {
        return false;
    }

    return true;
}

// Data structure to track reflex vertices and check whether any are inside a given triangle
class ReflexHash {
public:
    void add(TriangulationVertex* v) {
        fReflexList.addToTail(v);
    }

    void remove(TriangulationVertex* v) {
        fReflexList.remove(v);
    }

    bool checkTriangle(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                       uint16_t ignoreIndex0, uint16_t ignoreIndex1) {
        for (SkTInternalLList<TriangulationVertex>::Iter reflexIter = fReflexList.begin();
             reflexIter != fReflexList.end(); ++reflexIter) {
            TriangulationVertex* reflexVertex = *reflexIter;
            if (reflexVertex->fIndex != ignoreIndex0 && reflexVertex->fIndex != ignoreIndex1 &&
                point_in_triangle(p0, p1, p2, reflexVertex->fPosition)) {
                return true;
            }
        }

        return false;
    }

private:
    // TODO: switch to an actual spatial hash
    SkTInternalLList<TriangulationVertex> fReflexList;
};

// Check to see if a reflex vertex has become a convex vertex after clipping an ear
static void reclassify_vertex(TriangulationVertex* p, const SkPoint* polygonVerts,
                              int winding, ReflexHash* reflexHash,
                              SkTInternalLList<TriangulationVertex>* convexList) {
    if (TriangulationVertex::VertexType::kReflex == p->fVertexType) {
        SkVector v0 = p->fPosition - polygonVerts[p->fPrevIndex];
        SkVector v1 = polygonVerts[p->fNextIndex] - p->fPosition;
        if (winding*v0.cross(v1) > SK_ScalarNearlyZero*SK_ScalarNearlyZero) {
            p->fVertexType = TriangulationVertex::VertexType::kConvex;
            reflexHash->remove(p);
            p->fPrev = p->fNext = nullptr;
            convexList->addToTail(p);
        }
    }
}

bool SkTriangulateSimplePolygon(const SkPoint* polygonVerts, uint16_t* indexMap, int polygonSize,
                                SkTDArray<uint16_t>* triangleIndices) {
    if (polygonSize < 3) {
        return false;
    }
    // need to be able to represent all the vertices in the 16-bit indices
    if (polygonSize >= (1 << 16)) {
        return false;
    }

    // get winding direction
    // TODO: we do this for all the polygon routines -- might be better to have the client
    // compute it and pass it in
    int winding = SkGetPolygonWinding(polygonVerts, polygonSize);
    if (0 == winding) {
        return false;
    }

    // Classify initial vertices into a list of convex vertices and a hash of reflex vertices
    // TODO: possibly sort the convexList in some way to get better triangles
    SkTInternalLList<TriangulationVertex> convexList;
    ReflexHash reflexHash;
    SkAutoSTMalloc<64, TriangulationVertex> triangulationVertices(polygonSize);
    int prevIndex = polygonSize - 1;
    int currIndex = 0;
    int nextIndex = 1;
    SkVector v0 = polygonVerts[currIndex] - polygonVerts[prevIndex];
    SkVector v1 = polygonVerts[nextIndex] - polygonVerts[currIndex];
    for (int i = 0; i < polygonSize; ++i) {
        SkDEBUGCODE(memset(&triangulationVertices[currIndex], 0, sizeof(TriangulationVertex)));
        triangulationVertices[currIndex].fPosition = polygonVerts[currIndex];
        triangulationVertices[currIndex].fIndex = currIndex;
        triangulationVertices[currIndex].fPrevIndex = prevIndex;
        triangulationVertices[currIndex].fNextIndex = nextIndex;
        if (winding*v0.cross(v1) > SK_ScalarNearlyZero*SK_ScalarNearlyZero) {
            triangulationVertices[currIndex].fVertexType = TriangulationVertex::VertexType::kConvex;
            convexList.addToTail(&triangulationVertices[currIndex]);
        } else {
            // We treat near collinear vertices as reflex
            triangulationVertices[currIndex].fVertexType = TriangulationVertex::VertexType::kReflex;
            reflexHash.add(&triangulationVertices[currIndex]);
        }

        prevIndex = currIndex;
        currIndex = nextIndex;
        nextIndex = (currIndex + 1) % polygonSize;
        v0 = v1;
        v1 = polygonVerts[nextIndex] - polygonVerts[currIndex];
    }

    // The general concept: We are trying to find three neighboring vertices where
    // no other vertex lies inside the triangle (an "ear"). If we find one, we clip
    // that ear off, and then repeat on the new polygon. Once we get down to three vertices
    // we have triangulated the entire polygon.
    // In the worst case this is an n^2 algorithm. We can cut down the search space somewhat by
    // noting that only convex vertices can be potential ears, and we only need to check whether
    // any reflex vertices lie inside the ear.
    triangleIndices->setReserve(triangleIndices->count() + 3 * (polygonSize - 2));
    int vertexCount = polygonSize;
    while (vertexCount > 3) {
        bool success = false;
        TriangulationVertex* earVertex = nullptr;
        TriangulationVertex* p0 = nullptr;
        TriangulationVertex* p2 = nullptr;
        // find a convex vertex to clip
        for (SkTInternalLList<TriangulationVertex>::Iter convexIter = convexList.begin();
             convexIter != convexList.end(); ++convexIter) {
            earVertex = *convexIter;
            SkASSERT(TriangulationVertex::VertexType::kReflex != earVertex->fVertexType);

            p0 = &triangulationVertices[earVertex->fPrevIndex];
            p2 = &triangulationVertices[earVertex->fNextIndex];

            // see if any reflex vertices are inside the ear
            bool failed = reflexHash.checkTriangle(p0->fPosition, earVertex->fPosition,
                                                   p2->fPosition, p0->fIndex, p2->fIndex);
            if (failed) {
                continue;
            }

            // found one we can clip
            success = true;
            break;
        }
        // If we can't find any ears to clip, this probably isn't a simple polygon
        if (!success) {
            return false;
        }

        // add indices
        auto indices = triangleIndices->append(3);
        indices[0] = indexMap[p0->fIndex];
        indices[1] = indexMap[earVertex->fIndex];
        indices[2] = indexMap[p2->fIndex];

        // clip the ear
        convexList.remove(earVertex);
        --vertexCount;

        // reclassify reflex verts
        p0->fNextIndex = earVertex->fNextIndex;
        reclassify_vertex(p0, polygonVerts, winding, &reflexHash, &convexList);

        p2->fPrevIndex = earVertex->fPrevIndex;
        reclassify_vertex(p2, polygonVerts, winding, &reflexHash, &convexList);
    }

    // output indices
    for (SkTInternalLList<TriangulationVertex>::Iter vertexIter = convexList.begin();
         vertexIter != convexList.end(); ++vertexIter) {
        TriangulationVertex* vertex = *vertexIter;
        *triangleIndices->push() = indexMap[vertex->fIndex];
    }

    return true;
}
