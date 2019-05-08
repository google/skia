/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPolyUtils.h"

#include <limits>

#include "SkNx.h"
#include "SkPointPriv.h"
#include "SkTArray.h"
#include "SkTemplates.h"
#include "SkTDPQueue.h"
#include "SkTInternalLList.h"

//////////////////////////////////////////////////////////////////////////////////
// Helper data structures and functions

struct OffsetSegment {
    SkPoint fP0;
    SkVector fV;
};

constexpr SkScalar kCrossTolerance = SK_ScalarNearlyZero * SK_ScalarNearlyZero;

// Computes perpDot for point p compared to segment defined by origin p0 and vector v.
// A positive value means the point is to the left of the segment,
// negative is to the right, 0 is collinear.
static int compute_side(const SkPoint& p0, const SkVector& v, const SkPoint& p) {
    SkVector w = p - p0;
    SkScalar perpDot = v.cross(w);
    if (!SkScalarNearlyZero(perpDot, kCrossTolerance)) {
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
    for (int curr = 2; curr < polygonSize; ++curr) {
        SkVector v1 = polygonVerts[curr] - polygonVerts[0];
        quadArea += v0.cross(v1);
        v0 = v1;
    }
    if (SkScalarNearlyZero(quadArea, kCrossTolerance)) {
        return 0;
    }
    // 1 == ccw, -1 == cw
    return (quadArea > 0) ? 1 : -1;
}

// Compute difference vector to offset p0-p1 'offset' units in direction specified by 'side'
bool compute_offset_vector(const SkPoint& p0, const SkPoint& p1, SkScalar offset, int side,
                           SkPoint* vector) {
    SkASSERT(side == -1 || side == 1);
    // if distances are equal, can just outset by the perpendicular
    SkVector perp = SkVector::Make(p0.fY - p1.fY, p1.fX - p0.fX);
    if (!perp.setLength(offset*side)) {
        return false;
    }
    *vector = perp;
    return true;
}

// check interval to see if intersection is in segment
static inline bool outside_interval(SkScalar numer, SkScalar denom, bool denomPositive) {
    return (denomPositive && (numer < 0 || numer > denom)) ||
           (!denomPositive && (numer > 0 || numer < denom));
}

// Compute the intersection 'p' between segments s0 and s1, if any.
// 's' is the parametric value for the intersection along 's0' & 't' is the same for 's1'.
// Returns false if there is no intersection.
static bool compute_intersection(const OffsetSegment& s0, const OffsetSegment& s1,
                                 SkPoint* p, SkScalar* s, SkScalar* t) {
    const SkVector& v0 = s0.fV;
    const SkVector& v1 = s1.fV;
    SkVector w = s1.fP0 - s0.fP0;
    SkScalar denom = v0.cross(v1);
    bool denomPositive = (denom > 0);
    SkScalar sNumer, tNumer;
    if (SkScalarNearlyZero(denom, kCrossTolerance)) {
        // segments are parallel, but not collinear
        if (!SkScalarNearlyZero(w.cross(v0), kCrossTolerance) ||
            !SkScalarNearlyZero(w.cross(v1), kCrossTolerance)) {
            return false;
        }

        // Check for zero-length segments
        if (!SkPointPriv::CanNormalize(v0.fX, v0.fY)) {
            // Both are zero-length
            if (!SkPointPriv::CanNormalize(v1.fX, v1.fY)) {
                // Check if they're the same point
                if (!SkPointPriv::CanNormalize(w.fX, w.fY)) {
                    *p = s0.fP0;
                    *s = 0;
                    *t = 0;
                    return true;
                } else {
                    return false;
                }
            }
            // Otherwise project segment0's origin onto segment1
            tNumer = v1.dot(-w);
            denom = v1.dot(v1);
            if (outside_interval(tNumer, denom, true)) {
                return false;
            }
            sNumer = 0;
        } else {
            // Project segment1's endpoints onto segment0
            sNumer = v0.dot(w);
            denom = v0.dot(v0);
            tNumer = 0;
            if (outside_interval(sNumer, denom, true)) {
                // The first endpoint doesn't lie on segment0
                // If segment1 is degenerate, then there's no collision
                if (!SkPointPriv::CanNormalize(v1.fX, v1.fY)) {
                    return false;
                }

                // Otherwise try the other one
                SkScalar oldSNumer = sNumer;
                sNumer = v0.dot(w + v1);
                tNumer = denom;
                if (outside_interval(sNumer, denom, true)) {
                    // it's possible that segment1's interval surrounds segment0
                    // this is false if params have the same signs, and in that case no collision
                    if (sNumer*oldSNumer > 0) {
                        return false;
                    }
                    // otherwise project segment0's endpoint onto segment1 instead
                    sNumer = 0;
                    tNumer = v1.dot(-w);
                    denom = v1.dot(v1);
                }
            }
        }
    } else {
        sNumer = w.cross(v1);
        if (outside_interval(sNumer, denom, denomPositive)) {
            return false;
        }
        tNumer = w.cross(v0);
        if (outside_interval(tNumer, denom, denomPositive)) {
            return false;
        }
    }

    SkScalar localS = sNumer/denom;
    SkScalar localT = tNumer/denom;

    *p = s0.fP0 + v0*localS;
    *s = localS;
    *t = localT;

    return true;
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

struct OffsetEdge {
    OffsetEdge*   fPrev;
    OffsetEdge*   fNext;
    OffsetSegment fOffset;
    SkPoint       fIntersection;
    SkScalar      fTValue;
    uint16_t      fIndex;
    uint16_t      fEnd;

    void init(uint16_t start = 0, uint16_t end = 0) {
        fIntersection = fOffset.fP0;
        fTValue = SK_ScalarMin;
        fIndex = start;
        fEnd = end;
    }

    // special intersection check that looks for endpoint intersection
    bool checkIntersection(const OffsetEdge* that,
                           SkPoint* p, SkScalar* s, SkScalar* t) {
        if (this->fEnd == that->fIndex) {
            SkPoint p1 = this->fOffset.fP0 + this->fOffset.fV;
            if (SkPointPriv::EqualsWithinTolerance(p1, that->fOffset.fP0)) {
                *p = p1;
                *s = SK_Scalar1;
                *t = 0;
                return true;
            }
        }

        return compute_intersection(this->fOffset, that->fOffset, p, s, t);
    }

    // computes the line intersection and then the "distance" from that to this
    // this is really a signed squared distance, where negative means that
    // the intersection lies inside this->fOffset
    SkScalar computeCrossingDistance(const OffsetEdge* that) {
        const OffsetSegment& s0 = this->fOffset;
        const OffsetSegment& s1 = that->fOffset;
        const SkVector& v0 = s0.fV;
        const SkVector& v1 = s1.fV;

        SkScalar denom = v0.cross(v1);
        if (SkScalarNearlyZero(denom, kCrossTolerance)) {
            // segments are parallel
            return SK_ScalarMax;
        }

        SkVector w = s1.fP0 - s0.fP0;
        SkScalar localS = w.cross(v1) / denom;
        if (localS < 0) {
            localS = -localS;
        } else {
            localS -= SK_Scalar1;
        }

        localS *= SkScalarAbs(localS);
        localS *= v0.dot(v0);

        return localS;
    }

};

static void remove_node(const OffsetEdge* node, OffsetEdge** head) {
    // remove from linked list
    node->fPrev->fNext = node->fNext;
    node->fNext->fPrev = node->fPrev;
    if (node == *head) {
        *head = (node->fNext == node) ? nullptr : node->fNext;
    }
}

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
                          SkScalar inset, SkTDArray<SkPoint>* insetPolygon) {
    if (inputPolygonSize < 3) {
        return false;
    }

    // restrict this to match other routines
    // practically we don't want anything bigger than this anyway
    if (inputPolygonSize > std::numeric_limits<uint16_t>::max()) {
        return false;
    }

    // can't inset by a negative or non-finite amount
    if (inset < -SK_ScalarNearlyZero || !SkScalarIsFinite(inset)) {
        return false;
    }

    // insetting close to zero just returns the original poly
    if (inset <= SK_ScalarNearlyZero) {
        for (int i = 0; i < inputPolygonSize; ++i) {
            *insetPolygon->push() = inputPolygonVerts[i];
        }
        return true;
    }

    // get winding direction
    int winding = SkGetPolygonWinding(inputPolygonVerts, inputPolygonSize);
    if (0 == winding) {
        return false;
    }

    // set up
    SkAutoSTMalloc<64, OffsetEdge> edgeData(inputPolygonSize);
    int prev = inputPolygonSize - 1;
    for (int curr = 0; curr < inputPolygonSize; prev = curr, ++curr) {
        int next = (curr + 1) % inputPolygonSize;
        if (!inputPolygonVerts[curr].isFinite()) {
            return false;
        }
        // check for convexity just to be sure
        if (compute_side(inputPolygonVerts[prev], inputPolygonVerts[curr] - inputPolygonVerts[prev],
                         inputPolygonVerts[next])*winding < 0) {
            return false;
        }
        SkVector v = inputPolygonVerts[next] - inputPolygonVerts[curr];
        SkVector perp = SkVector::Make(-v.fY, v.fX);
        perp.setLength(inset*winding);
        edgeData[curr].fPrev = &edgeData[prev];
        edgeData[curr].fNext = &edgeData[next];
        edgeData[curr].fOffset.fP0 = inputPolygonVerts[curr] + perp;
        edgeData[curr].fOffset.fV = v;
        edgeData[curr].init();
    }

    OffsetEdge* head = &edgeData[0];
    OffsetEdge* currEdge = head;
    OffsetEdge* prevEdge = currEdge->fPrev;
    int insetVertexCount = inputPolygonSize;
    unsigned int iterations = 0;
    unsigned int maxIterations = inputPolygonSize * inputPolygonSize;
    while (head && prevEdge != currEdge) {
        ++iterations;
        // we should check each edge against each other edge at most once
        if (iterations > maxIterations) {
            return false;
        }

        SkScalar s, t;
        SkPoint intersection;
        if (compute_intersection(prevEdge->fOffset, currEdge->fOffset,
                                 &intersection, &s, &t)) {
            // if new intersection is further back on previous inset from the prior intersection
            if (s < prevEdge->fTValue) {
                // no point in considering this one again
                remove_node(prevEdge, &head);
                --insetVertexCount;
                // go back one segment
                prevEdge = prevEdge->fPrev;
            // we've already considered this intersection, we're done
            } else if (currEdge->fTValue > SK_ScalarMin &&
                       SkPointPriv::EqualsWithinTolerance(intersection,
                                                          currEdge->fIntersection,
                                                          1.0e-6f)) {
                break;
            } else {
                // add intersection
                currEdge->fIntersection = intersection;
                currEdge->fTValue = t;

                // go to next segment
                prevEdge = currEdge;
                currEdge = currEdge->fNext;
            }
        } else {
            // if prev to right side of curr
            int side = winding*compute_side(currEdge->fOffset.fP0,
                                            currEdge->fOffset.fV,
                                            prevEdge->fOffset.fP0);
            if (side < 0 &&
                side == winding*compute_side(currEdge->fOffset.fP0,
                                             currEdge->fOffset.fV,
                                             prevEdge->fOffset.fP0 + prevEdge->fOffset.fV)) {
                // no point in considering this one again
                remove_node(prevEdge, &head);
                --insetVertexCount;
                // go back one segment
                prevEdge = prevEdge->fPrev;
            } else {
                // move to next segment
                remove_node(currEdge, &head);
                --insetVertexCount;
                currEdge = currEdge->fNext;
            }
        }
    }

    // store all the valid intersections that aren't nearly coincident
    // TODO: look at the main algorithm and see if we can detect these better
    insetPolygon->reset();
    if (!head) {
        return false;
    }

    static constexpr SkScalar kCleanupTolerance = 0.01f;
    if (insetVertexCount >= 0) {
        insetPolygon->setReserve(insetVertexCount);
    }
    int currIndex = 0;
    *insetPolygon->push() = head->fIntersection;
    currEdge = head->fNext;
    while (currEdge != head) {
        if (!SkPointPriv::EqualsWithinTolerance(currEdge->fIntersection,
                                                (*insetPolygon)[currIndex],
                                                kCleanupTolerance)) {
            *insetPolygon->push() = currEdge->fIntersection;
            currIndex++;
        }
        currEdge = currEdge->fNext;
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
bool SkComputeRadialSteps(const SkVector& v1, const SkVector& v2, SkScalar offset,
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

    SkScalar floatSteps = SkScalarAbs(offset*theta*kRecipPixelsPerArcSegment);
    // limit the number of steps to at most max uint16_t (that's all we can index)
    // knock one value off the top to account for rounding
    if (floatSteps >= std::numeric_limits<uint16_t>::max()) {
        return false;
    }
    int steps = SkScalarRoundToInt(floatSteps);

    SkScalar dTheta = steps > 0 ? theta / steps : 0;
    *rotSin = SkScalarSin(dTheta);
    *rotCos = SkScalarCos(dTheta);
    *n = steps;
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

// a point is "left" to another if its x-coord is less, or if equal, its y-coord is greater
static bool left(const SkPoint& p0, const SkPoint& p1) {
    return p0.fX < p1.fX || (!(p0.fX > p1.fX) && p0.fY > p1.fY);
}

// a point is "right" to another if its x-coord is greater, or if equal, its y-coord is less
static bool right(const SkPoint& p0, const SkPoint& p1) {
    return p0.fX > p1.fX || (!(p0.fX < p1.fX) && p0.fY < p1.fY);
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

struct ActiveEdge {
    ActiveEdge() : fChild{ nullptr, nullptr }, fAbove(nullptr), fBelow(nullptr), fRed(false) {}
    ActiveEdge(const SkPoint& p0, const SkVector& v, uint16_t index0, uint16_t index1)
        : fSegment({ p0, v })
        , fIndex0(index0)
        , fIndex1(index1)
        , fAbove(nullptr)
        , fBelow(nullptr)
        , fRed(true) {
        fChild[0] = nullptr;
        fChild[1] = nullptr;
    }

    // Returns true if "this" is above "that", assuming this->p0 is to the left of that->p0
    // This is only used to verify the edgelist -- the actual test for insertion/deletion is much
    // simpler because we can make certain assumptions then.
    bool aboveIfLeft(const ActiveEdge* that) const {
        const SkPoint& p0 = this->fSegment.fP0;
        const SkPoint& q0 = that->fSegment.fP0;
        SkASSERT(p0.fX <= q0.fX);
        SkVector d = q0 - p0;
        const SkVector& v = this->fSegment.fV;
        const SkVector& w = that->fSegment.fV;
        // The idea here is that if the vector between the origins of the two segments (d)
        // rotates counterclockwise up to the vector representing the "this" segment (v),
        // then we know that "this" is above "that". If the result is clockwise we say it's below.
        if (this->fIndex0 != that->fIndex0) {
            SkScalar cross = d.cross(v);
            if (cross > kCrossTolerance) {
                return true;
            } else if (cross < -kCrossTolerance) {
                return false;
            }
        } else if (this->fIndex1 == that->fIndex1) {
            return false;
        }
        // At this point either the two origins are nearly equal or the origin of "that"
        // lies on dv. So then we try the same for the vector from the tail of "this"
        // to the head of "that". Again, ccw means "this" is above "that".
        // d = that.P1 - this.P0
        //   = that.fP0 + that.fV - this.fP0
        //   = that.fP0 - this.fP0 + that.fV
        //   = old_d + that.fV
        d += w;
        SkScalar cross = d.cross(v);
        if (cross > kCrossTolerance) {
            return true;
        } else if (cross < -kCrossTolerance) {
            return false;
        }
        // If the previous check fails, the two segments are nearly collinear
        // First check y-coord of first endpoints
        if (p0.fX < q0.fX) {
            return (p0.fY >= q0.fY);
        } else if (p0.fY > q0.fY) {
            return true;
        } else if (p0.fY < q0.fY) {
            return false;
        }
        // The first endpoints are the same, so check the other endpoint
        SkPoint p1 = p0 + v;
        SkPoint q1 = q0 + w;
        if (p1.fX < q1.fX) {
            return (p1.fY >= q1.fY);
        } else {
            return (p1.fY > q1.fY);
        }
    }

    // same as leftAndAbove(), but generalized
    bool above(const ActiveEdge* that) const {
        const SkPoint& p0 = this->fSegment.fP0;
        const SkPoint& q0 = that->fSegment.fP0;
        if (right(p0, q0)) {
            return !that->aboveIfLeft(this);
        } else {
            return this->aboveIfLeft(that);
        }
    }

    bool intersect(const SkPoint& q0, const SkVector& w, uint16_t index0, uint16_t index1) const {
        // check first to see if these edges are neighbors in the polygon
        if (this->fIndex0 == index0 || this->fIndex1 == index0 ||
            this->fIndex0 == index1 || this->fIndex1 == index1) {
            return false;
        }

        // We don't need the exact intersection point so we can do a simpler test here.
        const SkPoint& p0 = this->fSegment.fP0;
        const SkVector& v = this->fSegment.fV;
        SkPoint p1 = p0 + v;
        SkPoint q1 = q0 + w;

        // We assume some x-overlap due to how the edgelist works
        // This allows us to simplify our test
        // We need some slop here because storing the vector and recomputing the second endpoint
        // doesn't necessary give us the original result in floating point.
        // TODO: Store vector as double? Store endpoint as well?
        SkASSERT(q0.fX <= p1.fX + SK_ScalarNearlyZero);

        // if each segment straddles the other (i.e., the endpoints have different sides)
        // then they intersect
        bool result;
        if (p0.fX < q0.fX) {
            if (q1.fX < p1.fX) {
                result = (compute_side(p0, v, q0)*compute_side(p0, v, q1) < 0);
            } else {
                result = (compute_side(p0, v, q0)*compute_side(q0, w, p1) > 0);
            }
        } else {
            if (p1.fX < q1.fX) {
                result = (compute_side(q0, w, p0)*compute_side(q0, w, p1) < 0);
            } else {
                result = (compute_side(q0, w, p0)*compute_side(p0, v, q1) > 0);
            }
        }
        return result;
    }

    bool intersect(const ActiveEdge* edge) {
        return this->intersect(edge->fSegment.fP0, edge->fSegment.fV, edge->fIndex0, edge->fIndex1);
    }

    bool lessThan(const ActiveEdge* that) const {
        SkASSERT(!this->above(this));
        SkASSERT(!that->above(that));
        SkASSERT(!(this->above(that) && that->above(this)));
        return this->above(that);
    }

    bool equals(uint16_t index0, uint16_t index1) const {
        return (this->fIndex0 == index0 && this->fIndex1 == index1);
    }

    OffsetSegment fSegment;
    uint16_t fIndex0;   // indices for previous and next vertex in polygon
    uint16_t fIndex1;
    ActiveEdge* fChild[2];
    ActiveEdge* fAbove;
    ActiveEdge* fBelow;
    int32_t  fRed;
};

class ActiveEdgeList {
public:
    ActiveEdgeList(int maxEdges) {
        fAllocation = (char*) sk_malloc_throw(sizeof(ActiveEdge)*maxEdges);
        fCurrFree = 0;
        fMaxFree = maxEdges;
    }
    ~ActiveEdgeList() {
        fTreeHead.fChild[1] = nullptr;
        sk_free(fAllocation);
    }

    bool insert(const SkPoint& p0, const SkPoint& p1, uint16_t index0, uint16_t index1) {
        SkVector v = p1 - p0;
        if (!v.isFinite()) {
            return false;
        }
        // empty tree case -- easy
        if (!fTreeHead.fChild[1]) {
            ActiveEdge* root = fTreeHead.fChild[1] = this->allocate(p0, v, index0, index1);
            SkASSERT(root);
            if (!root) {
                return false;
            }
            root->fRed = false;
            return true;
        }

        // set up helpers
        ActiveEdge* top = &fTreeHead;
        ActiveEdge *grandparent = nullptr;
        ActiveEdge *parent = nullptr;
        ActiveEdge *curr = top->fChild[1];
        int dir = 0;
        int last = 0; // ?
        // predecessor and successor, for intersection check
        ActiveEdge* pred = nullptr;
        ActiveEdge* succ = nullptr;

        // search down the tree
        while (true) {
            if (!curr) {
                // check for intersection with predecessor and successor
                if ((pred && pred->intersect(p0, v, index0, index1)) ||
                    (succ && succ->intersect(p0, v, index0, index1))) {
                    return false;
                }
                // insert new node at bottom
                parent->fChild[dir] = curr = this->allocate(p0, v, index0, index1);
                SkASSERT(curr);
                if (!curr) {
                    return false;
                }
                curr->fAbove = pred;
                curr->fBelow = succ;
                if (pred) {
                    pred->fBelow = curr;
                }
                if (succ) {
                    succ->fAbove = curr;
                }
                if (IsRed(parent)) {
                    int dir2 = (top->fChild[1] == grandparent);
                    if (curr == parent->fChild[last]) {
                        top->fChild[dir2] = SingleRotation(grandparent, !last);
                    } else {
                        top->fChild[dir2] = DoubleRotation(grandparent, !last);
                    }
                }
                break;
            } else if (IsRed(curr->fChild[0]) && IsRed(curr->fChild[1])) {
                // color flip
                curr->fRed = true;
                curr->fChild[0]->fRed = false;
                curr->fChild[1]->fRed = false;
                if (IsRed(parent)) {
                    int dir2 = (top->fChild[1] == grandparent);
                    if (curr == parent->fChild[last]) {
                        top->fChild[dir2] = SingleRotation(grandparent, !last);
                    } else {
                        top->fChild[dir2] = DoubleRotation(grandparent, !last);
                    }
                }
            }

            last = dir;
            int side;
            // check to see if segment is above or below
            if (curr->fIndex0 == index0) {
                side = compute_side(curr->fSegment.fP0, curr->fSegment.fV, p1);
            } else {
                side = compute_side(curr->fSegment.fP0, curr->fSegment.fV, p0);
            }
            if (0 == side) {
                return false;
            }
            dir = (side < 0);

            if (0 == dir) {
                succ = curr;
            } else {
                pred = curr;
            }

            // update helpers
            if (grandparent) {
                top = grandparent;
            }
            grandparent = parent;
            parent = curr;
            curr = curr->fChild[dir];
        }

        // update root and make it black
        fTreeHead.fChild[1]->fRed = false;

        SkDEBUGCODE(VerifyTree(fTreeHead.fChild[1]));

        return true;
    }

    // replaces edge p0p1 with p1p2
    bool replace(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                 uint16_t index0, uint16_t index1, uint16_t index2) {
        if (!fTreeHead.fChild[1]) {
            return false;
        }

        SkVector v = p2 - p1;
        ActiveEdge* curr = &fTreeHead;
        ActiveEdge* found = nullptr;
        int dir = 1;

        // search
        while (curr->fChild[dir] != nullptr) {
            // update helpers
            curr = curr->fChild[dir];
            // save found node
            if (curr->equals(index0, index1)) {
                found = curr;
                break;
            } else {
                // check to see if segment is above or below
                int side;
                if (curr->fIndex1 == index1) {
                    side = compute_side(curr->fSegment.fP0, curr->fSegment.fV, p0);
                } else {
                    side = compute_side(curr->fSegment.fP0, curr->fSegment.fV, p1);
                }
                if (0 == side) {
                    return false;
                }
                dir = (side < 0);
            }
        }

        if (!found) {
            return false;
        }

        // replace if found
        ActiveEdge* pred = found->fAbove;
        ActiveEdge* succ = found->fBelow;
        // check deletion and insert intersection cases
        if (pred && (pred->intersect(found) || pred->intersect(p1, v, index1, index2))) {
            return false;
        }
        if (succ && (succ->intersect(found) || succ->intersect(p1, v, index1, index2))) {
            return false;
        }
        found->fSegment.fP0 = p1;
        found->fSegment.fV = v;
        found->fIndex0 = index1;
        found->fIndex1 = index2;
        // above and below should stay the same

        SkDEBUGCODE(VerifyTree(fTreeHead.fChild[1]));

        return true;
    }

    bool remove(const SkPoint& p0, const SkPoint& p1, uint16_t index0, uint16_t index1) {
        if (!fTreeHead.fChild[1]) {
            return false;
        }

        ActiveEdge* curr = &fTreeHead;
        ActiveEdge* parent = nullptr;
        ActiveEdge* grandparent = nullptr;
        ActiveEdge* found = nullptr;
        int dir = 1;

        // search and push a red node down
        while (curr->fChild[dir] != nullptr) {
            int last = dir;

            // update helpers
            grandparent = parent;
            parent = curr;
            curr = curr->fChild[dir];
            // save found node
            if (curr->equals(index0, index1)) {
                found = curr;
                dir = 0;
            } else {
                // check to see if segment is above or below
                int side;
                if (curr->fIndex1 == index1) {
                    side = compute_side(curr->fSegment.fP0, curr->fSegment.fV, p0);
                } else {
                    side = compute_side(curr->fSegment.fP0, curr->fSegment.fV, p1);
                }
                if (0 == side) {
                    return false;
                }
                dir = (side < 0);
            }

            // push the red node down
            if (!IsRed(curr) && !IsRed(curr->fChild[dir])) {
                if (IsRed(curr->fChild[!dir])) {
                    parent = parent->fChild[last] = SingleRotation(curr, dir);
                } else {
                    ActiveEdge *s = parent->fChild[!last];

                    if (s != NULL) {
                        if (!IsRed(s->fChild[!last]) && !IsRed(s->fChild[last])) {
                            // color flip
                            parent->fRed = false;
                            s->fRed = true;
                            curr->fRed = true;
                        } else {
                            int dir2 = (grandparent->fChild[1] == parent);

                            if (IsRed(s->fChild[last])) {
                                grandparent->fChild[dir2] = DoubleRotation(parent, last);
                            } else if (IsRed(s->fChild[!last])) {
                                grandparent->fChild[dir2] = SingleRotation(parent, last);
                            }

                            // ensure correct coloring
                            curr->fRed = grandparent->fChild[dir2]->fRed = true;
                            grandparent->fChild[dir2]->fChild[0]->fRed = false;
                            grandparent->fChild[dir2]->fChild[1]->fRed = false;
                        }
                    }
                }
            }
        }

        // replace and remove if found
        if (found) {
            ActiveEdge* pred = found->fAbove;
            ActiveEdge* succ = found->fBelow;
            if ((pred && pred->intersect(found)) || (succ && succ->intersect(found))) {
                return false;
            }
            if (found != curr) {
                found->fSegment = curr->fSegment;
                found->fIndex0 = curr->fIndex0;
                found->fIndex1 = curr->fIndex1;
                found->fAbove = curr->fAbove;
                pred = found->fAbove;
                // we don't need to set found->fBelow here
            } else {
                if (succ) {
                    succ->fAbove = pred;
                }
            }
            if (pred) {
                pred->fBelow = curr->fBelow;
            }
            parent->fChild[parent->fChild[1] == curr] = curr->fChild[!curr->fChild[0]];

            // no need to delete
            curr->fAbove = reinterpret_cast<ActiveEdge*>(0xdeadbeefll);
            curr->fBelow = reinterpret_cast<ActiveEdge*>(0xdeadbeefll);
            if (fTreeHead.fChild[1]) {
                fTreeHead.fChild[1]->fRed = false;
            }
        }

        // update root and make it black
        if (fTreeHead.fChild[1]) {
            fTreeHead.fChild[1]->fRed = false;
        }

        SkDEBUGCODE(VerifyTree(fTreeHead.fChild[1]));

        return true;
    }

private:
    // allocator
    ActiveEdge * allocate(const SkPoint& p0, const SkPoint& p1, uint16_t index0, uint16_t index1) {
        if (fCurrFree >= fMaxFree) {
            return nullptr;
        }
        char* bytes = fAllocation + sizeof(ActiveEdge)*fCurrFree;
        ++fCurrFree;
        return new(bytes) ActiveEdge(p0, p1, index0, index1);
    }

    ///////////////////////////////////////////////////////////////////////////////////
    // Red-black tree methods
    ///////////////////////////////////////////////////////////////////////////////////
    static bool IsRed(const ActiveEdge* node) {
        return node && node->fRed;
    }

    static ActiveEdge* SingleRotation(ActiveEdge* node, int dir) {
        ActiveEdge* tmp = node->fChild[!dir];

        node->fChild[!dir] = tmp->fChild[dir];
        tmp->fChild[dir] = node;

        node->fRed = true;
        tmp->fRed = false;

        return tmp;
    }

    static ActiveEdge* DoubleRotation(ActiveEdge* node, int dir) {
        node->fChild[!dir] = SingleRotation(node->fChild[!dir], !dir);

        return SingleRotation(node, dir);
    }

    // returns black link count
    static int VerifyTree(const ActiveEdge* tree) {
        if (!tree) {
            return 1;
        }

        const ActiveEdge* left = tree->fChild[0];
        const ActiveEdge* right = tree->fChild[1];

        // no consecutive red links
        if (IsRed(tree) && (IsRed(left) || IsRed(right))) {
            SkASSERT(false);
            return 0;
        }

        // check secondary links
        if (tree->fAbove) {
            SkASSERT(tree->fAbove->fBelow == tree);
            SkASSERT(tree->fAbove->lessThan(tree));
        }
        if (tree->fBelow) {
            SkASSERT(tree->fBelow->fAbove == tree);
            SkASSERT(tree->lessThan(tree->fBelow));
        }

        // violates binary tree order
        if ((left && tree->lessThan(left)) || (right && right->lessThan(tree))) {
            SkASSERT(false);
            return 0;
        }

        int leftCount = VerifyTree(left);
        int rightCount = VerifyTree(right);

        // return black link count
        if (leftCount != 0 && rightCount != 0) {
            // black height mismatch
            if (leftCount != rightCount) {
                SkASSERT(false);
                return 0;
            }
            return IsRed(tree) ? leftCount : leftCount + 1;
        } else {
            return 0;
        }
    }

    ActiveEdge fTreeHead;
    char*      fAllocation;
    int        fCurrFree;
    int        fMaxFree;
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

    // If it's convex, it's simple
    if (SkIsConvexPolygon(polygon, polygonSize)) {
        return true;
    }

    // practically speaking, it takes too long to process large polygons
    if (polygonSize > 2048) {
        return false;
    }

    SkTDPQueue <Vertex, Vertex::Left> vertexQueue(polygonSize);
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
    ActiveEdgeList sweepLine(polygonSize);
    while (vertexQueue.count() > 0) {
        const Vertex& v = vertexQueue.peek();

        // both to the right -- insert both
        if (v.fFlags == 0) {
            if (!sweepLine.insert(v.fPosition, polygon[v.fPrevIndex], v.fIndex, v.fPrevIndex)) {
                break;
            }
            if (!sweepLine.insert(v.fPosition, polygon[v.fNextIndex], v.fIndex, v.fNextIndex)) {
                break;
            }
        // both to the left -- remove both
        } else if (v.fFlags == (kPrevLeft_VertexFlag | kNextLeft_VertexFlag)) {
            if (!sweepLine.remove(polygon[v.fPrevIndex], v.fPosition, v.fPrevIndex, v.fIndex)) {
                break;
            }
            if (!sweepLine.remove(polygon[v.fNextIndex], v.fPosition, v.fNextIndex, v.fIndex)) {
                break;
            }
        // one to left and right -- replace one with another
        } else {
            if (v.fFlags & kPrevLeft_VertexFlag) {
                if (!sweepLine.replace(polygon[v.fPrevIndex], v.fPosition, polygon[v.fNextIndex],
                                       v.fPrevIndex, v.fIndex, v.fNextIndex)) {
                    break;
                }
            } else {
                SkASSERT(v.fFlags & kNextLeft_VertexFlag);
                if (!sweepLine.replace(polygon[v.fNextIndex], v.fPosition, polygon[v.fPrevIndex],
                                       v.fNextIndex, v.fIndex, v.fPrevIndex)) {
                    break;
                }
            }
        }

        vertexQueue.pop();
    }

    return (vertexQueue.count() == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////

// helper function for SkOffsetSimplePolygon
static void setup_offset_edge(OffsetEdge* currEdge,
                              const SkPoint& endpoint0, const SkPoint& endpoint1,
                              uint16_t startIndex, uint16_t endIndex) {
    currEdge->fOffset.fP0 = endpoint0;
    currEdge->fOffset.fV = endpoint1 - endpoint0;
    currEdge->init(startIndex, endIndex);
}

static bool is_reflex_vertex(const SkPoint* inputPolygonVerts, int winding, SkScalar offset,
                             uint16_t prevIndex, uint16_t currIndex, uint16_t nextIndex) {
    int side = compute_side(inputPolygonVerts[prevIndex],
                            inputPolygonVerts[currIndex] - inputPolygonVerts[prevIndex],
                            inputPolygonVerts[nextIndex]);
    // if reflex point, we need to add extra edges
    return (side*winding*offset < 0);
}

bool SkOffsetSimplePolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                           const SkRect& bounds, SkScalar offset,
                           SkTDArray<SkPoint>* offsetPolygon, SkTDArray<int>* polygonIndices) {
    if (inputPolygonSize < 3) {
        return false;
    }

    // need to be able to represent all the vertices in the 16-bit indices
    if (inputPolygonSize >= std::numeric_limits<uint16_t>::max()) {
        return false;
    }

    if (!SkScalarIsFinite(offset)) {
        return false;
    }

    // can't inset more than the half bounds of the polygon
    if (offset > SkTMin(SkTAbs(SK_ScalarHalf*bounds.width()),
                        SkTAbs(SK_ScalarHalf*bounds.height()))) {
        return false;
    }

    // offsetting close to zero just returns the original poly
    if (SkScalarNearlyZero(offset)) {
        for (int i = 0; i < inputPolygonSize; ++i) {
            *offsetPolygon->push() = inputPolygonVerts[i];
            if (polygonIndices) {
                *polygonIndices->push() = i;
            }
        }
        return true;
    }

    // get winding direction
    int winding = SkGetPolygonWinding(inputPolygonVerts, inputPolygonSize);
    if (0 == winding) {
        return false;
    }

    // build normals
    SkAutoSTMalloc<64, SkVector> normals(inputPolygonSize);
    unsigned int numEdges = 0;
    for (int currIndex = 0, prevIndex = inputPolygonSize - 1;
         currIndex < inputPolygonSize;
         prevIndex = currIndex, ++currIndex) {
        if (!inputPolygonVerts[currIndex].isFinite()) {
            return false;
        }
        int nextIndex = (currIndex + 1) % inputPolygonSize;
        if (!compute_offset_vector(inputPolygonVerts[currIndex], inputPolygonVerts[nextIndex],
                                   offset, winding, &normals[currIndex])) {
            return false;
        }
        if (currIndex > 0) {
            // if reflex point, we need to add extra edges
            if (is_reflex_vertex(inputPolygonVerts, winding, offset,
                                 prevIndex, currIndex, nextIndex)) {
                SkScalar rotSin, rotCos;
                int numSteps;
                if (!SkComputeRadialSteps(normals[prevIndex], normals[currIndex], offset,
                                          &rotSin, &rotCos, &numSteps)) {
                    return false;
                }
                numEdges += SkTMax(numSteps, 1);
            }
        }
        numEdges++;
    }
    // finish up the edge counting
    if (is_reflex_vertex(inputPolygonVerts, winding, offset, inputPolygonSize-1, 0, 1)) {
        SkScalar rotSin, rotCos;
        int numSteps;
        if (!SkComputeRadialSteps(normals[inputPolygonSize-1], normals[0], offset,
                                  &rotSin, &rotCos, &numSteps)) {
            return false;
        }
        numEdges += SkTMax(numSteps, 1);
    }

    // Make sure we don't overflow the max array count.
    // We shouldn't overflow numEdges, as SkComputeRadialSteps returns a max of 2^16-1,
    // and we have a max of 2^16-1 original vertices.
    if (numEdges > (unsigned int)std::numeric_limits<int32_t>::max()) {
        return false;
    }

    // build initial offset edge list
    SkSTArray<64, OffsetEdge> edgeData(numEdges);
    OffsetEdge* prevEdge = nullptr;
    for (int currIndex = 0, prevIndex = inputPolygonSize - 1;
         currIndex < inputPolygonSize;
         prevIndex = currIndex, ++currIndex) {
        int nextIndex = (currIndex + 1) % inputPolygonSize;
        // if reflex point, fill in curve
        if (is_reflex_vertex(inputPolygonVerts, winding, offset,
                             prevIndex, currIndex, nextIndex)) {
            SkScalar rotSin, rotCos;
            int numSteps;
            SkVector prevNormal = normals[prevIndex];
            if (!SkComputeRadialSteps(prevNormal, normals[currIndex], offset,
                                      &rotSin, &rotCos, &numSteps)) {
                return false;
            }
            auto currEdge = edgeData.push_back_n(SkTMax(numSteps, 1));
            for (int i = 0; i < numSteps - 1; ++i) {
                SkVector currNormal = SkVector::Make(prevNormal.fX*rotCos - prevNormal.fY*rotSin,
                                                     prevNormal.fY*rotCos + prevNormal.fX*rotSin);
                setup_offset_edge(currEdge,
                                  inputPolygonVerts[currIndex] + prevNormal,
                                  inputPolygonVerts[currIndex] + currNormal,
                                  currIndex, currIndex);
                prevNormal = currNormal;
                currEdge->fPrev = prevEdge;
                if (prevEdge) {
                    prevEdge->fNext = currEdge;
                }
                prevEdge = currEdge;
                ++currEdge;
            }
            setup_offset_edge(currEdge,
                              inputPolygonVerts[currIndex] + prevNormal,
                              inputPolygonVerts[currIndex] + normals[currIndex],
                              currIndex, currIndex);
            currEdge->fPrev = prevEdge;
            if (prevEdge) {
                prevEdge->fNext = currEdge;
            }
            prevEdge = currEdge;
        }

        // Add the edge
        auto currEdge = edgeData.push_back_n(1);
        setup_offset_edge(currEdge,
                          inputPolygonVerts[currIndex] + normals[currIndex],
                          inputPolygonVerts[nextIndex] + normals[currIndex],
                          currIndex, nextIndex);
        currEdge->fPrev = prevEdge;
        if (prevEdge) {
            prevEdge->fNext = currEdge;
        }
        prevEdge = currEdge;
    }
    // close up the linked list
    SkASSERT(prevEdge);
    prevEdge->fNext = &edgeData[0];
    edgeData[0].fPrev = prevEdge;

    // now clip edges
    SkASSERT(edgeData.count() == (int)numEdges);
    auto head = &edgeData[0];
    auto currEdge = head;
    unsigned int offsetVertexCount = numEdges;
    unsigned long long iterations = 0;
    unsigned long long maxIterations = (unsigned long long)(numEdges) * numEdges;
    while (head && prevEdge != currEdge && offsetVertexCount > 0) {
        ++iterations;
        // we should check each edge against each other edge at most once
        if (iterations > maxIterations) {
            return false;
        }

        SkScalar s, t;
        SkPoint intersection;
        if (prevEdge->checkIntersection(currEdge, &intersection, &s, &t)) {
            // if new intersection is further back on previous inset from the prior intersection
            if (s < prevEdge->fTValue) {
                // no point in considering this one again
                remove_node(prevEdge, &head);
                --offsetVertexCount;
                // go back one segment
                prevEdge = prevEdge->fPrev;
                // we've already considered this intersection, we're done
            } else if (currEdge->fTValue > SK_ScalarMin &&
                       SkPointPriv::EqualsWithinTolerance(intersection,
                                                          currEdge->fIntersection,
                                                          1.0e-6f)) {
                break;
            } else {
                // add intersection
                currEdge->fIntersection = intersection;
                currEdge->fTValue = t;
                currEdge->fIndex = prevEdge->fEnd;

                // go to next segment
                prevEdge = currEdge;
                currEdge = currEdge->fNext;
            }
        } else {
            // If there is no intersection, we want to minimize the distance between
            // the point where the segment lines cross and the segments themselves.
            OffsetEdge* prevPrevEdge = prevEdge->fPrev;
            OffsetEdge* currNextEdge = currEdge->fNext;
            SkScalar dist0 = currEdge->computeCrossingDistance(prevPrevEdge);
            SkScalar dist1 = prevEdge->computeCrossingDistance(currNextEdge);
            // if both lead to direct collision
            if (dist0 < 0 && dist1 < 0) {
                // check first to see if either represent parts of one contour
                SkPoint p1 = prevPrevEdge->fOffset.fP0 + prevPrevEdge->fOffset.fV;
                bool prevSameContour = SkPointPriv::EqualsWithinTolerance(p1,
                                                                          prevEdge->fOffset.fP0);
                p1 = currEdge->fOffset.fP0 + currEdge->fOffset.fV;
                bool currSameContour = SkPointPriv::EqualsWithinTolerance(p1,
                                                                         currNextEdge->fOffset.fP0);

                // want to step along contour to find intersections rather than jump to new one
                if (currSameContour && !prevSameContour) {
                    remove_node(currEdge, &head);
                    currEdge = currNextEdge;
                    --offsetVertexCount;
                    continue;
                } else if (prevSameContour && !currSameContour) {
                    remove_node(prevEdge, &head);
                    prevEdge = prevPrevEdge;
                    --offsetVertexCount;
                    continue;
                }
            }

            // otherwise minimize collision distance along segment
            if (dist0 < dist1) {
                remove_node(prevEdge, &head);
                prevEdge = prevPrevEdge;
            } else {
                remove_node(currEdge, &head);
                currEdge = currNextEdge;
            }
            --offsetVertexCount;
        }
    }

    // store all the valid intersections that aren't nearly coincident
    // TODO: look at the main algorithm and see if we can detect these better
    offsetPolygon->reset();
    if (!head || offsetVertexCount == 0 ||
        offsetVertexCount >= std::numeric_limits<uint16_t>::max()) {
        return false;
    }

    static constexpr SkScalar kCleanupTolerance = 0.01f;
    offsetPolygon->setReserve(offsetVertexCount);
    int currIndex = 0;
    *offsetPolygon->push() = head->fIntersection;
    if (polygonIndices) {
        *polygonIndices->push() = head->fIndex;
    }
    currEdge = head->fNext;
    while (currEdge != head) {
        if (!SkPointPriv::EqualsWithinTolerance(currEdge->fIntersection,
                                                (*offsetPolygon)[currIndex],
                                                kCleanupTolerance)) {
            *offsetPolygon->push() = currEdge->fIntersection;
            if (polygonIndices) {
                *polygonIndices->push() = currEdge->fIndex;
            }
            currIndex++;
        }
        currEdge = currEdge->fNext;
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

static void compute_triangle_bounds(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                                    SkRect* bounds) {
    Sk4s min, max;
    min = max = Sk4s(p0.fX, p0.fY, p0.fX, p0.fY);
    Sk4s xy(p1.fX, p1.fY, p2.fX, p2.fY);
    min = Sk4s::Min(min, xy);
    max = Sk4s::Max(max, xy);
    bounds->set(SkTMin(min[0], min[2]), SkTMin(min[1], min[3]),
                SkTMax(max[0], max[2]), SkTMax(max[1], max[3]));
}

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
    bool init(const SkRect& bounds, int vertexCount) {
        fBounds = bounds;
        fNumVerts = 0;
        SkScalar width = bounds.width();
        SkScalar height = bounds.height();
        if (!SkScalarIsFinite(width) || !SkScalarIsFinite(height)) {
            return false;
        }

        // We want vertexCount grid cells, roughly distributed to match the bounds ratio
        SkScalar hCount = SkScalarSqrt(sk_ieee_float_divide(vertexCount*width, height));
        if (!SkScalarIsFinite(hCount)) {
            return false;
        }
        fHCount = SkTMax(SkTMin(SkScalarRoundToInt(hCount), vertexCount), 1);
        fVCount = vertexCount/fHCount;
        fGridConversion.set(sk_ieee_float_divide(fHCount - 0.001f, width),
                            sk_ieee_float_divide(fVCount - 0.001f, height));
        if (!fGridConversion.isFinite()) {
            return false;
        }

        fGrid.setCount(fHCount*fVCount);
        for (int i = 0; i < fGrid.count(); ++i) {
            fGrid[i].reset();
        }

        return true;
    }

    void add(TriangulationVertex* v) {
        int index = hash(v);
        fGrid[index].addToTail(v);
        ++fNumVerts;
    }

    void remove(TriangulationVertex* v) {
        int index = hash(v);
        fGrid[index].remove(v);
        --fNumVerts;
    }

    bool checkTriangle(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                       uint16_t ignoreIndex0, uint16_t ignoreIndex1) const {
        if (!fNumVerts) {
            return false;
        }

        SkRect triBounds;
        compute_triangle_bounds(p0, p1, p2, &triBounds);
        int h0 = (triBounds.fLeft - fBounds.fLeft)*fGridConversion.fX;
        int h1 = (triBounds.fRight - fBounds.fLeft)*fGridConversion.fX;
        int v0 = (triBounds.fTop - fBounds.fTop)*fGridConversion.fY;
        int v1 = (triBounds.fBottom - fBounds.fTop)*fGridConversion.fY;

        for (int v = v0; v <= v1; ++v) {
            for (int h = h0; h <= h1; ++h) {
                int i = v * fHCount + h;
                for (SkTInternalLList<TriangulationVertex>::Iter reflexIter = fGrid[i].begin();
                     reflexIter != fGrid[i].end(); ++reflexIter) {
                    TriangulationVertex* reflexVertex = *reflexIter;
                    if (reflexVertex->fIndex != ignoreIndex0 &&
                        reflexVertex->fIndex != ignoreIndex1 &&
                        point_in_triangle(p0, p1, p2, reflexVertex->fPosition)) {
                        return true;
                    }
                }

            }
        }

        return false;
    }

private:
    int hash(TriangulationVertex* vert) const {
        int h = (vert->fPosition.fX - fBounds.fLeft)*fGridConversion.fX;
        int v = (vert->fPosition.fY - fBounds.fTop)*fGridConversion.fY;
        SkASSERT(v*fHCount + h >= 0);
        return v*fHCount + h;
    }

    SkRect fBounds;
    int fHCount;
    int fVCount;
    int fNumVerts;
    // converts distance from the origin to a grid location (when cast to int)
    SkVector fGridConversion;
    SkTDArray<SkTInternalLList<TriangulationVertex>> fGrid;
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
    if (polygonSize >= std::numeric_limits<uint16_t>::max()) {
        return false;
    }

    // get bounds
    SkRect bounds;
    if (!bounds.setBoundsCheck(polygonVerts, polygonSize)) {
        return false;
    }
    // get winding direction
    // TODO: we do this for all the polygon routines -- might be better to have the client
    // compute it and pass it in
    int winding = SkGetPolygonWinding(polygonVerts, polygonSize);
    if (0 == winding) {
        return false;
    }

    // Set up vertices
    SkAutoSTMalloc<64, TriangulationVertex> triangulationVertices(polygonSize);
    int prevIndex = polygonSize - 1;
    SkVector v0 = polygonVerts[0] - polygonVerts[prevIndex];
    for (int currIndex = 0; currIndex < polygonSize; ++currIndex) {
        int nextIndex = (currIndex + 1) % polygonSize;

        SkDEBUGCODE(memset(&triangulationVertices[currIndex], 0, sizeof(TriangulationVertex)));
        triangulationVertices[currIndex].fPosition = polygonVerts[currIndex];
        triangulationVertices[currIndex].fIndex = currIndex;
        triangulationVertices[currIndex].fPrevIndex = prevIndex;
        triangulationVertices[currIndex].fNextIndex = nextIndex;
        SkVector v1 = polygonVerts[nextIndex] - polygonVerts[currIndex];
        if (winding*v0.cross(v1) > SK_ScalarNearlyZero*SK_ScalarNearlyZero) {
            triangulationVertices[currIndex].fVertexType = TriangulationVertex::VertexType::kConvex;
        } else {
            triangulationVertices[currIndex].fVertexType = TriangulationVertex::VertexType::kReflex;
        }

        prevIndex = currIndex;
        v0 = v1;
    }

    // Classify initial vertices into a list of convex vertices and a hash of reflex vertices
    // TODO: possibly sort the convexList in some way to get better triangles
    SkTInternalLList<TriangulationVertex> convexList;
    ReflexHash reflexHash;
    if (!reflexHash.init(bounds, polygonSize)) {
        return false;
    }
    prevIndex = polygonSize - 1;
    for (int currIndex = 0; currIndex < polygonSize; prevIndex = currIndex, ++currIndex) {
        TriangulationVertex::VertexType currType = triangulationVertices[currIndex].fVertexType;
        if (TriangulationVertex::VertexType::kConvex == currType) {
            int nextIndex = (currIndex + 1) % polygonSize;
            TriangulationVertex::VertexType prevType = triangulationVertices[prevIndex].fVertexType;
            TriangulationVertex::VertexType nextType = triangulationVertices[nextIndex].fVertexType;
            // We prioritize clipping vertices with neighboring reflex vertices.
            // The intent here is that it will cull reflex vertices more quickly.
            if (TriangulationVertex::VertexType::kReflex == prevType ||
                TriangulationVertex::VertexType::kReflex == nextType) {
                convexList.addToHead(&triangulationVertices[currIndex]);
            } else {
                convexList.addToTail(&triangulationVertices[currIndex]);
            }
        } else {
            // We treat near collinear vertices as reflex
            reflexHash.add(&triangulationVertices[currIndex]);
        }
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

///////////

static double crs(SkVector a, SkVector b) {
    return a.fX * b.fY - a.fY * b.fX;
}

static int sign(SkScalar v) {
    return v < 0 ? -1 : (v > 0);
}

struct SignTracker {
    int fSign;
    int fSignChanges;

    void reset() {
        fSign = 0;
        fSignChanges = 0;
    }

    void init(int s) {
        SkASSERT(fSignChanges == 0);
        SkASSERT(s == 1 || s == -1 || s == 0);
        fSign = s;
        fSignChanges = 1;
    }

    void update(int s) {
        if (s) {
            if (fSign != s) {
                fSignChanges += 1;
                fSign = s;
            }
        }
    }
};

struct ConvexTracker {
    SkVector    fFirst, fPrev;
    SignTracker fDSign, fCSign;
    int         fVecCounter;
    bool        fIsConcave;

    ConvexTracker() { this->reset(); }

    void reset() {
        fPrev = {0, 0};
        fDSign.reset();
        fCSign.reset();
        fVecCounter = 0;
        fIsConcave = false;
    }

    void addVec(SkPoint p1, SkPoint p0) {
        this->addVec(p1 - p0);
    }
    void addVec(SkVector v) {
        if (v.fX == 0 && v.fY == 0) {
            return;
        }

        fVecCounter += 1;
        if (fVecCounter == 1) {
            fFirst = fPrev = v;
            fDSign.update(sign(v.fX));
            return;
        }

        SkScalar d = v.fX;
        SkScalar c = crs(fPrev, v);
        int sign_c;
        if (c) {
            sign_c = sign(c);
        } else {
            if (d >= 0) {
                sign_c = fCSign.fSign;
            } else {
                sign_c = -fCSign.fSign;
            }
        }

        fDSign.update(sign(d));
        fCSign.update(sign_c);
        fPrev = v;

        if (fDSign.fSignChanges > 3 || fCSign.fSignChanges > 1) {
            fIsConcave = true;
        }
    }

    void finalCross() {
        this->addVec(fFirst);
    }
};

bool SkIsPolyConvex_experimental(const SkPoint pts[], int count) {
    if (count <= 3) {
        return true;
    }

    ConvexTracker tracker;

    for (int i = 0; i < count - 1; ++i) {
        tracker.addVec(pts[i + 1], pts[i]);
        if (tracker.fIsConcave) {
            return false;
        }
    }
    tracker.addVec(pts[0], pts[count - 1]);
    tracker.finalCross();
    return !tracker.fIsConcave;
}

