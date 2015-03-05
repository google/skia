/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTessellatingPathRenderer.h"

#include "GrBatch.h"
#include "GrBatchTarget.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrPathUtils.h"
#include "SkChunkAlloc.h"
#include "SkGeometry.h"

#include <stdio.h>

/*
 * This path renderer tessellates the path into triangles, uploads the triangles to a
 * vertex buffer, and renders them with a single draw call. It does not currently do
 * antialiasing, so it must be used in conjunction with multisampling.
 *
 * There are six stages to the algorithm:
 *
 * 1) Linearize the path contours into piecewise linear segments (path_to_contours()).
 * 2) Build a mesh of edges connecting the vertices (build_edges()).
 * 3) Sort the vertices in Y (and secondarily in X) (merge_sort()).
 * 4) Simplify the mesh by inserting new vertices at intersecting edges (simplify()).
 * 5) Tessellate the simplified mesh into monotone polygons (tessellate()).
 * 6) Triangulate the monotone polygons directly into a vertex buffer (polys_to_triangles()).
 *
 * The vertex sorting in step (3) is a merge sort, since it plays well with the linked list
 * of vertices (and the necessity of inserting new vertices on intersection).
 *
 * Stages (4) and (5) use an active edge list, which a list of all edges for which the
 * sweep line has crossed the top vertex, but not the bottom vertex.  It's sorted
 * left-to-right based on the point where both edges are active (when both top vertices
 * have been seen, so the "lower" top vertex of the two). If the top vertices are equal
 * (shared), it's sorted based on the last point where both edges are active, so the
 * "upper" bottom vertex.
 *
 * The most complex step is the simplification (4). It's based on the Bentley-Ottman
 * line-sweep algorithm, but due to floating point inaccuracy, the intersection points are
 * not exact and may violate the mesh topology or active edge list ordering. We
 * accommodate this by adjusting the topology of the mesh and AEL to match the intersection
 * points. This occurs in three ways:
 *
 * A) Intersections may cause a shortened edge to no longer be ordered with respect to its
 *    neighbouring edges at the top or bottom vertex. This is handled by merging the
 *    edges (merge_collinear_edges()).
 * B) Intersections may cause an edge to violate the left-to-right ordering of the
 *    active edge list. This is handled by splitting the neighbour edge on the
 *    intersected vertex (cleanup_active_edges()).
 * C) Shortening an edge may cause an active edge to become inactive or an inactive edge
 *    to become active. This is handled by removing or inserting the edge in the active
 *    edge list (fix_active_state()).
 *
 * The tessellation steps (5) and (6) are based on "Triangulating Simple Polygons and
 * Equivalent Problems" (Fournier and Montuno); also a line-sweep algorithm. Note that it
 * currently uses a linked list for the active edge list, rather than a 2-3 tree as the
 * paper describes. The 2-3 tree gives O(lg N) lookups, but insertion and removal also
 * become O(lg N). In all the test cases, it was found that the cost of frequent O(lg N)
 * insertions and removals was greater than the cost of infrequent O(N) lookups with the
 * linked list implementation. With the latter, all removals are O(1), and most insertions
 * are O(1), since we know the adjacent edge in the active edge list based on the topology.
 * Only type 2 vertices (see paper) require the O(N) lookups, and these are much less
 * frequent. There may be other data structures worth investigating, however.
 *
 * Note that there is a compile-time flag (SWEEP_IN_X) which changes the orientation of the
 * line sweep algorithms. When SWEEP_IN_X is unset, we sort vertices based on increasing
 * Y coordinate, and secondarily by increasing X coordinate. When SWEEP_IN_X is set, we sort by
 * increasing X coordinate, but secondarily by *decreasing* Y coordinate. This is so that the
 * "left" and "right" orientation in the code remains correct (edges to the left are increasing
 * in Y; edges to the right are decreasing in Y). That is, the setting rotates 90 degrees
 * counterclockwise, rather that transposing.
 *
 * The choice is arbitrary, but most test cases are wider than they are tall, so the
 * default is to sweep in X. In the future, we may want to make this a runtime parameter
 * and base it on the aspect ratio of the clip bounds.
 */
#define LOGGING_ENABLED 0
#define WIREFRAME 0
#define SWEEP_IN_X 1

#if LOGGING_ENABLED
#define LOG printf
#else
#define LOG(...)
#endif

#define ALLOC_NEW(Type, args, alloc) \
    SkNEW_PLACEMENT_ARGS(alloc.allocThrow(sizeof(Type)), Type, args)

namespace {

struct Vertex;
struct Edge;
struct Poly;

template <class T, T* T::*Prev, T* T::*Next>
void insert(T* t, T* prev, T* next, T** head, T** tail) {
    t->*Prev = prev;
    t->*Next = next;
    if (prev) {
        prev->*Next = t;
    } else if (head) {
        *head = t;
    }
    if (next) {
        next->*Prev = t;
    } else if (tail) {
        *tail = t;
    }
}

template <class T, T* T::*Prev, T* T::*Next>
void remove(T* t, T** head, T** tail) {
    if (t->*Prev) {
        t->*Prev->*Next = t->*Next;
    } else if (head) {
        *head = t->*Next;
    }
    if (t->*Next) {
        t->*Next->*Prev = t->*Prev;
    } else if (tail) {
        *tail = t->*Prev;
    }
    t->*Prev = t->*Next = NULL;
}

/**
 * Vertices are used in three ways: first, the path contours are converted into a
 * circularly-linked list of Vertices for each contour. After edge construction, the same Vertices
 * are re-ordered by the merge sort according to the sweep_lt comparator (usually, increasing
 * in Y) using the same fPrev/fNext pointers that were used for the contours, to avoid
 * reallocation. Finally, MonotonePolys are built containing a circularly-linked list of
 * Vertices. (Currently, those Vertices are newly-allocated for the MonotonePolys, since
 * an individual Vertex from the path mesh may belong to multiple
 * MonotonePolys, so the original Vertices cannot be re-used.
 */

struct Vertex {
  Vertex(const SkPoint& point)
    : fPoint(point), fPrev(NULL), fNext(NULL)
    , fFirstEdgeAbove(NULL), fLastEdgeAbove(NULL)
    , fFirstEdgeBelow(NULL), fLastEdgeBelow(NULL)
    , fProcessed(false)
#if LOGGING_ENABLED
    , fID (-1.0f)
#endif
    {}
    SkPoint fPoint;           // Vertex position
    Vertex* fPrev;            // Linked list of contours, then Y-sorted vertices.
    Vertex* fNext;            // "
    Edge*   fFirstEdgeAbove;  // Linked list of edges above this vertex.
    Edge*   fLastEdgeAbove;   // "
    Edge*   fFirstEdgeBelow;  // Linked list of edges below this vertex.
    Edge*   fLastEdgeBelow;   // "
    bool    fProcessed;       // Has this vertex been seen in simplify()?
#if LOGGING_ENABLED
    float   fID;              // Identifier used for logging.
#endif
};

/***************************************************************************************/

bool sweep_lt(const SkPoint& a, const SkPoint& b) {
#if SWEEP_IN_X
    return a.fX == b.fX ? a.fY > b.fY : a.fX < b.fX;
#else
    return a.fY == b.fY ? a.fX < b.fX : a.fY < b.fY;
#endif
}

bool sweep_gt(const SkPoint& a, const SkPoint& b) {
#if SWEEP_IN_X
    return a.fX == b.fX ? a.fY < b.fY : a.fX > b.fX;
#else
    return a.fY == b.fY ? a.fX > b.fX : a.fY > b.fY;
#endif
}

inline void* emit_vertex(Vertex* v, void* data) {
    SkPoint* d = static_cast<SkPoint*>(data);
    *d++ = v->fPoint;
    return d;
}

void* emit_triangle(Vertex* v0, Vertex* v1, Vertex* v2, void* data) {
#if WIREFRAME
    data = emit_vertex(v0, data);
    data = emit_vertex(v1, data);
    data = emit_vertex(v1, data);
    data = emit_vertex(v2, data);
    data = emit_vertex(v2, data);
    data = emit_vertex(v0, data);
#else
    data = emit_vertex(v0, data);
    data = emit_vertex(v1, data);
    data = emit_vertex(v2, data);
#endif
    return data;
}

/**
 * An Edge joins a top Vertex to a bottom Vertex. Edge ordering for the list of "edges above" and
 * "edge below" a vertex as well as for the active edge list is handled by isLeftOf()/isRightOf().
 * Note that an Edge will give occasionally dist() != 0 for its own endpoints (because floating
 * point). For speed, that case is only tested by the callers which require it (e.g.,
 * cleanup_active_edges()). Edges also handle checking for intersection with other edges.
 * Currently, this converts the edges to the parametric form, in order to avoid doing a division
 * until an intersection has been confirmed. This is slightly slower in the "found" case, but
 * a lot faster in the "not found" case.
 *
 * The coefficients of the line equation stored in double precision to avoid catastrphic
 * cancellation in the isLeftOf() and isRightOf() checks. Using doubles ensures that the result is
 * correct in float, since it's a polynomial of degree 2. The intersect() function, being
 * degree 5, is still subject to catastrophic cancellation. We deal with that by assuming its
 * output may be incorrect, and adjusting the mesh topology to match (see comment at the top of
 * this file).
 */

struct Edge {
    Edge(Vertex* top, Vertex* bottom, int winding)
        : fWinding(winding)
        , fTop(top)
        , fBottom(bottom)
        , fLeft(NULL)
        , fRight(NULL)
        , fPrevEdgeAbove(NULL)
        , fNextEdgeAbove(NULL)
        , fPrevEdgeBelow(NULL)
        , fNextEdgeBelow(NULL)
        , fLeftPoly(NULL)
        , fRightPoly(NULL) {
            recompute();
        }
    int      fWinding;          // 1 == edge goes downward; -1 = edge goes upward.
    Vertex*  fTop;              // The top vertex in vertex-sort-order (sweep_lt).
    Vertex*  fBottom;           // The bottom vertex in vertex-sort-order.
    Edge*    fLeft;             // The linked list of edges in the active edge list.
    Edge*    fRight;            // "
    Edge*    fPrevEdgeAbove;    // The linked list of edges in the bottom Vertex's "edges above".
    Edge*    fNextEdgeAbove;    // "
    Edge*    fPrevEdgeBelow;    // The linked list of edges in the top Vertex's "edges below".
    Edge*    fNextEdgeBelow;    // "
    Poly*    fLeftPoly;         // The Poly to the left of this edge, if any.
    Poly*    fRightPoly;        // The Poly to the right of this edge, if any.
    double   fDX;               // The line equation for this edge, in implicit form.
    double   fDY;               // fDY * x + fDX * y + fC = 0, for point (x, y) on the line.
    double   fC;
    double dist(const SkPoint& p) const {
        return fDY * p.fX - fDX * p.fY + fC;
    }
    bool isRightOf(Vertex* v) const {
        return dist(v->fPoint) < 0.0;
    }
    bool isLeftOf(Vertex* v) const {
        return dist(v->fPoint) > 0.0;
    }
    void recompute() {
        fDX = static_cast<double>(fBottom->fPoint.fX) - fTop->fPoint.fX;
        fDY = static_cast<double>(fBottom->fPoint.fY) - fTop->fPoint.fY;
        fC = static_cast<double>(fTop->fPoint.fY) * fBottom->fPoint.fX -
             static_cast<double>(fTop->fPoint.fX) * fBottom->fPoint.fY;
    }
    bool intersect(const Edge& other, SkPoint* p) {
        LOG("intersecting %g -> %g with %g -> %g\n",
               fTop->fID, fBottom->fID,
               other.fTop->fID, other.fBottom->fID);
        if (fTop == other.fTop || fBottom == other.fBottom) {
            return false;
        }
        double denom = fDX * other.fDY - fDY * other.fDX;
        if (denom == 0.0) {
            return false;
        }
        double dx = static_cast<double>(fTop->fPoint.fX) - other.fTop->fPoint.fX;
        double dy = static_cast<double>(fTop->fPoint.fY) - other.fTop->fPoint.fY;
        double sNumer = dy * other.fDX - dx * other.fDY;
        double tNumer = dy * fDX - dx * fDY;
        // If (sNumer / denom) or (tNumer / denom) is not in [0..1], exit early.
        // This saves us doing the divide below unless absolutely necessary.
        if (denom > 0.0 ? (sNumer < 0.0 || sNumer > denom || tNumer < 0.0 || tNumer > denom)
                        : (sNumer > 0.0 || sNumer < denom || tNumer > 0.0 || tNumer < denom)) {
            return false;
        }
        double s = sNumer / denom;
        SkASSERT(s >= 0.0 && s <= 1.0);
        p->fX = SkDoubleToScalar(fTop->fPoint.fX + s * fDX);
        p->fY = SkDoubleToScalar(fTop->fPoint.fY + s * fDY);
        return true;
    }
    bool isActive(Edge** activeEdges) const {
        return activeEdges && (fLeft || fRight || *activeEdges == this);
    }
};

/***************************************************************************************/

struct Poly {
    Poly(int winding)
        : fWinding(winding)
        , fHead(NULL)
        , fTail(NULL)
        , fActive(NULL)
        , fNext(NULL)
        , fPartner(NULL)
        , fCount(0)
    {
#if LOGGING_ENABLED
        static int gID = 0;
        fID = gID++;
        LOG("*** created Poly %d\n", fID);
#endif
    }
    typedef enum { kNeither_Side, kLeft_Side, kRight_Side } Side;
    struct MonotonePoly {
        MonotonePoly()
            : fSide(kNeither_Side)
            , fHead(NULL)
            , fTail(NULL)
            , fPrev(NULL)
            , fNext(NULL) {}
        Side          fSide;
        Vertex*       fHead;
        Vertex*       fTail;
        MonotonePoly* fPrev;
        MonotonePoly* fNext;
        bool addVertex(Vertex* v, Side side, SkChunkAlloc& alloc) {
            Vertex* newV = ALLOC_NEW(Vertex, (v->fPoint), alloc);
            bool done = false;
            if (fSide == kNeither_Side) {
                fSide = side;
            } else {
                done = side != fSide;
            }
            if (fHead == NULL) {
                fHead = fTail = newV;
            } else if (fSide == kRight_Side) {
                newV->fPrev = fTail;
                fTail->fNext = newV;
                fTail = newV;
            } else {
                newV->fNext = fHead;
                fHead->fPrev = newV;
                fHead = newV;
            }
            return done;
        }

        void* emit(void* data) {
            Vertex* first = fHead;
            Vertex* v = first->fNext;
            while (v != fTail) {
                SkASSERT(v && v->fPrev && v->fNext);
#ifdef SK_DEBUG
                validate();
#endif
                Vertex* prev = v->fPrev;
                Vertex* curr = v;
                Vertex* next = v->fNext;
                double ax = static_cast<double>(curr->fPoint.fX) - prev->fPoint.fX;
                double ay = static_cast<double>(curr->fPoint.fY) - prev->fPoint.fY;
                double bx = static_cast<double>(next->fPoint.fX) - curr->fPoint.fX;
                double by = static_cast<double>(next->fPoint.fY) - curr->fPoint.fY;
                if (ax * by - ay * bx >= 0.0) {
                    data = emit_triangle(prev, curr, next, data);
                    v->fPrev->fNext = v->fNext;
                    v->fNext->fPrev = v->fPrev;
                    if (v->fPrev == first) {
                        v = v->fNext;
                    } else {
                        v = v->fPrev;
                    }
                } else {
                    v = v->fNext;
                    SkASSERT(v != fTail);
                }
            }
            return data;
        }

#ifdef SK_DEBUG
        void validate() {
            int winding = sweep_lt(fHead->fPoint, fTail->fPoint) ? 1 : -1;
            Vertex* top = winding < 0 ? fTail : fHead;
            Vertex* bottom = winding < 0 ? fHead : fTail;
            Edge e(top, bottom, winding);
            for (Vertex* v = fHead->fNext; v != fTail; v = v->fNext) {
                if (fSide == kRight_Side) {
                    SkASSERT(!e.isRightOf(v));
                } else if (fSide == Poly::kLeft_Side) {
                    SkASSERT(!e.isLeftOf(v));
                }
            }
        }
#endif
    };
    Poly* addVertex(Vertex* v, Side side, SkChunkAlloc& alloc) {
        LOG("addVertex() to %d at %g (%g, %g), %s side\n", fID, v->fID, v->fPoint.fX, v->fPoint.fY,
               side == kLeft_Side ? "left" : side == kRight_Side ? "right" : "neither");
        Poly* partner = fPartner;
        Poly* poly = this;
        if (partner) {
            fPartner = partner->fPartner = NULL;
        }
        if (!fActive) {
            fActive = ALLOC_NEW(MonotonePoly, (), alloc);
        }
        if (fActive->addVertex(v, side, alloc)) {
#ifdef SK_DEBUG
            fActive->validate();
#endif
            if (fTail) {
                fActive->fPrev = fTail;
                fTail->fNext = fActive;
                fTail = fActive;
            } else {
                fHead = fTail = fActive;
            }
            if (partner) {
                partner->addVertex(v, side, alloc);
                poly = partner;
            } else {
                Vertex* prev = fActive->fSide == Poly::kLeft_Side ?
                               fActive->fHead->fNext : fActive->fTail->fPrev;
                fActive = ALLOC_NEW(MonotonePoly, , alloc);
                fActive->addVertex(prev, Poly::kNeither_Side, alloc);
                fActive->addVertex(v, side, alloc);
            }
        }
        fCount++;
        return poly;
    }
    void end(Vertex* v, SkChunkAlloc& alloc) {
        LOG("end() %d at %g, %g\n", fID, v->fPoint.fX, v->fPoint.fY);
        if (fPartner) {
            fPartner = fPartner->fPartner = NULL;
        }
        addVertex(v, fActive->fSide == kLeft_Side ? kRight_Side : kLeft_Side, alloc);
    }
    void* emit(void *data) {
        if (fCount < 3) {
            return data;
        }
        LOG("emit() %d, size %d\n", fID, fCount);
        for (MonotonePoly* m = fHead; m != NULL; m = m->fNext) {
            data = m->emit(data);
        }
        return data;
    }
    int fWinding;
    MonotonePoly* fHead;
    MonotonePoly* fTail;
    MonotonePoly* fActive;
    Poly* fNext;
    Poly* fPartner;
    int fCount;
#if LOGGING_ENABLED
    int fID;
#endif
};

/***************************************************************************************/

bool coincident(const SkPoint& a, const SkPoint& b) {
    return a == b;
}

Poly* new_poly(Poly** head, Vertex* v, int winding, SkChunkAlloc& alloc) {
    Poly* poly = ALLOC_NEW(Poly, (winding), alloc);
    poly->addVertex(v, Poly::kNeither_Side, alloc);
    poly->fNext = *head;
    *head = poly;
    return poly;
}

#ifdef SK_DEBUG
void validate_edges(Edge* head) {
    for (Edge* e = head; e != NULL; e = e->fRight) {
        SkASSERT(e->fTop != e->fBottom);
        if (e->fLeft) {
            SkASSERT(e->fLeft->fRight == e);
            if (sweep_gt(e->fTop->fPoint, e->fLeft->fTop->fPoint)) {
                SkASSERT(e->fLeft->isLeftOf(e->fTop));
            }
            if (sweep_lt(e->fBottom->fPoint, e->fLeft->fBottom->fPoint)) {
                SkASSERT(e->fLeft->isLeftOf(e->fBottom));
            }
        } else {
            SkASSERT(e == head);
        }
        if (e->fRight) {
            SkASSERT(e->fRight->fLeft == e);
            if (sweep_gt(e->fTop->fPoint, e->fRight->fTop->fPoint)) {
                SkASSERT(e->fRight->isRightOf(e->fTop));
            }
            if (sweep_lt(e->fBottom->fPoint, e->fRight->fBottom->fPoint)) {
                SkASSERT(e->fRight->isRightOf(e->fBottom));
            }
        }
    }
}

void validate_connectivity(Vertex* v) {
    for (Edge* e = v->fFirstEdgeAbove; e != NULL; e = e->fNextEdgeAbove) {
        SkASSERT(e->fBottom == v);
        if (e->fPrevEdgeAbove) {
            SkASSERT(e->fPrevEdgeAbove->fNextEdgeAbove == e);
            SkASSERT(e->fPrevEdgeAbove->isLeftOf(e->fTop));
        } else {
            SkASSERT(e == v->fFirstEdgeAbove);
        }
        if (e->fNextEdgeAbove) {
            SkASSERT(e->fNextEdgeAbove->fPrevEdgeAbove == e);
            SkASSERT(e->fNextEdgeAbove->isRightOf(e->fTop));
        } else {
            SkASSERT(e == v->fLastEdgeAbove);
        }
    }
    for (Edge* e = v->fFirstEdgeBelow; e != NULL; e = e->fNextEdgeBelow) {
        SkASSERT(e->fTop == v);
        if (e->fPrevEdgeBelow) {
            SkASSERT(e->fPrevEdgeBelow->fNextEdgeBelow == e);
            SkASSERT(e->fPrevEdgeBelow->isLeftOf(e->fBottom));
        } else {
            SkASSERT(e == v->fFirstEdgeBelow);
        }
        if (e->fNextEdgeBelow) {
            SkASSERT(e->fNextEdgeBelow->fPrevEdgeBelow == e);
            SkASSERT(e->fNextEdgeBelow->isRightOf(e->fBottom));
        } else {
            SkASSERT(e == v->fLastEdgeBelow);
        }
    }
}
#endif

Vertex* append_point_to_contour(const SkPoint& p, Vertex* prev, Vertex** head,
                                SkChunkAlloc& alloc) {
    Vertex* v = ALLOC_NEW(Vertex, (p), alloc);
#if LOGGING_ENABLED
    static float gID = 0.0f;
    v->fID = gID++;
#endif
    if (prev) {
        prev->fNext = v;
        v->fPrev = prev;
    } else {
        *head = v;
    }
    return v;
}

Vertex* generate_quadratic_points(const SkPoint& p0,
                                  const SkPoint& p1,
                                  const SkPoint& p2,
                                  SkScalar tolSqd,
                                  Vertex* prev,
                                  Vertex** head,
                                  int pointsLeft,
                                  SkChunkAlloc& alloc) {
    SkScalar d = p1.distanceToLineSegmentBetweenSqd(p0, p2);
    if (pointsLeft < 2 || d < tolSqd || !SkScalarIsFinite(d)) {
        return append_point_to_contour(p2, prev, head, alloc);
    }

    const SkPoint q[] = {
        { SkScalarAve(p0.fX, p1.fX), SkScalarAve(p0.fY, p1.fY) },
        { SkScalarAve(p1.fX, p2.fX), SkScalarAve(p1.fY, p2.fY) },
    };
    const SkPoint r = { SkScalarAve(q[0].fX, q[1].fX), SkScalarAve(q[0].fY, q[1].fY) };

    pointsLeft >>= 1;
    prev = generate_quadratic_points(p0, q[0], r, tolSqd, prev, head, pointsLeft, alloc);
    prev = generate_quadratic_points(r, q[1], p2, tolSqd, prev, head, pointsLeft, alloc);
    return prev;
}

Vertex* generate_cubic_points(const SkPoint& p0,
                              const SkPoint& p1,
                              const SkPoint& p2,
                              const SkPoint& p3,
                              SkScalar tolSqd,
                              Vertex* prev,
                              Vertex** head,
                              int pointsLeft,
                              SkChunkAlloc& alloc) {
    SkScalar d1 = p1.distanceToLineSegmentBetweenSqd(p0, p3);
    SkScalar d2 = p2.distanceToLineSegmentBetweenSqd(p0, p3);
    if (pointsLeft < 2 || (d1 < tolSqd && d2 < tolSqd) ||
        !SkScalarIsFinite(d1) || !SkScalarIsFinite(d2)) {
        return append_point_to_contour(p3, prev, head, alloc);
    }
    const SkPoint q[] = {
        { SkScalarAve(p0.fX, p1.fX), SkScalarAve(p0.fY, p1.fY) },
        { SkScalarAve(p1.fX, p2.fX), SkScalarAve(p1.fY, p2.fY) },
        { SkScalarAve(p2.fX, p3.fX), SkScalarAve(p2.fY, p3.fY) }
    };
    const SkPoint r[] = {
        { SkScalarAve(q[0].fX, q[1].fX), SkScalarAve(q[0].fY, q[1].fY) },
        { SkScalarAve(q[1].fX, q[2].fX), SkScalarAve(q[1].fY, q[2].fY) }
    };
    const SkPoint s = { SkScalarAve(r[0].fX, r[1].fX), SkScalarAve(r[0].fY, r[1].fY) };
    pointsLeft >>= 1;
    prev = generate_cubic_points(p0, q[0], r[0], s, tolSqd, prev, head, pointsLeft, alloc);
    prev = generate_cubic_points(s, r[1], q[2], p3, tolSqd, prev, head, pointsLeft, alloc);
    return prev;
}

// Stage 1: convert the input path to a set of linear contours (linked list of Vertices).

void path_to_contours(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
                      Vertex** contours, SkChunkAlloc& alloc) {

    SkScalar toleranceSqd = tolerance * tolerance;

    SkPoint pts[4];
    bool done = false;
    SkPath::Iter iter(path, false);
    Vertex* prev = NULL;
    Vertex* head = NULL;
    if (path.isInverseFillType()) {
        SkPoint quad[4];
        clipBounds.toQuad(quad);
        for (int i = 3; i >= 0; i--) {
            prev = append_point_to_contour(quad[i], prev, &head, alloc);
        }
        head->fPrev = prev;
        prev->fNext = head;
        *contours++ = head;
        head = prev = NULL;
    }
    SkAutoConicToQuads converter;
    while (!done) {
        SkPath::Verb verb = iter.next(pts);
        switch (verb) {
            case SkPath::kConic_Verb: {
                SkScalar weight = iter.conicWeight();
                const SkPoint* quadPts = converter.computeQuads(pts, weight, toleranceSqd);
                for (int i = 0; i < converter.countQuads(); ++i) {
                    int pointsLeft = GrPathUtils::quadraticPointCount(quadPts, toleranceSqd);
                    prev = generate_quadratic_points(quadPts[0], quadPts[1], quadPts[2],
                                                     toleranceSqd, prev, &head, pointsLeft, alloc);
                    quadPts += 2;
                }
                break;
            }
            case SkPath::kMove_Verb:
                if (head) {
                    head->fPrev = prev;
                    prev->fNext = head;
                    *contours++ = head;
                }
                head = prev = NULL;
                prev = append_point_to_contour(pts[0], prev, &head, alloc);
                break;
            case SkPath::kLine_Verb: {
                prev = append_point_to_contour(pts[1], prev, &head, alloc);
                break;
            }
            case SkPath::kQuad_Verb: {
                int pointsLeft = GrPathUtils::quadraticPointCount(pts, toleranceSqd);
                prev = generate_quadratic_points(pts[0], pts[1], pts[2], toleranceSqd, prev,
                                                 &head, pointsLeft, alloc);
                break;
            }
            case SkPath::kCubic_Verb: {
                int pointsLeft = GrPathUtils::cubicPointCount(pts, toleranceSqd);
                prev = generate_cubic_points(pts[0], pts[1], pts[2], pts[3],
                                toleranceSqd, prev, &head, pointsLeft, alloc);
                break;
            }
            case SkPath::kClose_Verb:
                if (head) {
                    head->fPrev = prev;
                    prev->fNext = head;
                    *contours++ = head;
                }
                head = prev = NULL;
                break;
            case SkPath::kDone_Verb:
                if (head) {
                    head->fPrev = prev;
                    prev->fNext = head;
                    *contours++ = head;
                }
                done = true;
                break;
        }
    }
}

inline bool apply_fill_type(SkPath::FillType fillType, int winding) {
    switch (fillType) {
        case SkPath::kWinding_FillType:
            return winding != 0;
        case SkPath::kEvenOdd_FillType:
            return (winding & 1) != 0;
        case SkPath::kInverseWinding_FillType:
            return winding == 1;
        case SkPath::kInverseEvenOdd_FillType:
            return (winding & 1) == 1;
        default:
            SkASSERT(false);
            return false;
    }
}

Edge* new_edge(Vertex* prev, Vertex* next, SkChunkAlloc& alloc) {
    int winding = sweep_lt(prev->fPoint, next->fPoint) ? 1 : -1;
    Vertex* top = winding < 0 ? next : prev;
    Vertex* bottom = winding < 0 ? prev : next;
    return ALLOC_NEW(Edge, (top, bottom, winding), alloc);
}

void remove_edge(Edge* edge, Edge** head) {
    LOG("removing edge %g -> %g\n", edge->fTop->fID, edge->fBottom->fID);
    SkASSERT(edge->isActive(head));
    remove<Edge, &Edge::fLeft, &Edge::fRight>(edge, head, NULL);
}

void insert_edge(Edge* edge, Edge* prev, Edge** head) {
    LOG("inserting edge %g -> %g\n", edge->fTop->fID, edge->fBottom->fID);
    SkASSERT(!edge->isActive(head));
    Edge* next = prev ? prev->fRight : *head;
    insert<Edge, &Edge::fLeft, &Edge::fRight>(edge, prev, next, head, NULL);
}

void find_enclosing_edges(Vertex* v, Edge* head, Edge** left, Edge** right) {
    if (v->fFirstEdgeAbove) {
        *left = v->fFirstEdgeAbove->fLeft;
        *right = v->fLastEdgeAbove->fRight;
        return;
    }
    Edge* prev = NULL;
    Edge* next;
    for (next = head; next != NULL; next = next->fRight) {
        if (next->isRightOf(v)) {
            break;
        }
        prev = next;
    }
    *left = prev;
    *right = next;
    return;
}

void find_enclosing_edges(Edge* edge, Edge* head, Edge** left, Edge** right) {
    Edge* prev = NULL;
    Edge* next;
    for (next = head; next != NULL; next = next->fRight) {
        if ((sweep_gt(edge->fTop->fPoint, next->fTop->fPoint) && next->isRightOf(edge->fTop)) ||
            (sweep_gt(next->fTop->fPoint, edge->fTop->fPoint) && edge->isLeftOf(next->fTop)) ||
            (sweep_lt(edge->fBottom->fPoint, next->fBottom->fPoint) &&
             next->isRightOf(edge->fBottom)) ||
            (sweep_lt(next->fBottom->fPoint, edge->fBottom->fPoint) &&
             edge->isLeftOf(next->fBottom))) {
            break;
        }
        prev = next;
    }
    *left = prev;
    *right = next;
    return;
}

void fix_active_state(Edge* edge, Edge** activeEdges) {
    if (edge->isActive(activeEdges)) {
        if (edge->fBottom->fProcessed || !edge->fTop->fProcessed) {
            remove_edge(edge, activeEdges);
        }
    } else if (edge->fTop->fProcessed && !edge->fBottom->fProcessed) {
        Edge* left;
        Edge* right;
        find_enclosing_edges(edge, *activeEdges, &left, &right);
        insert_edge(edge, left, activeEdges);
    }
}

void insert_edge_above(Edge* edge, Vertex* v) {
    if (edge->fTop->fPoint == edge->fBottom->fPoint ||
        sweep_gt(edge->fTop->fPoint, edge->fBottom->fPoint)) {
        SkASSERT(false);
        return;
    }
    LOG("insert edge (%g -> %g) above vertex %g\n", edge->fTop->fID, edge->fBottom->fID, v->fID);
    Edge* prev = NULL;
    Edge* next;
    for (next = v->fFirstEdgeAbove; next; next = next->fNextEdgeAbove) {
        if (next->isRightOf(edge->fTop)) {
            break;
        }
        prev = next;
    }
    insert<Edge, &Edge::fPrevEdgeAbove, &Edge::fNextEdgeAbove>(
        edge, prev, next, &v->fFirstEdgeAbove, &v->fLastEdgeAbove);
}

void insert_edge_below(Edge* edge, Vertex* v) {
    if (edge->fTop->fPoint == edge->fBottom->fPoint ||
        sweep_gt(edge->fTop->fPoint, edge->fBottom->fPoint)) {
        SkASSERT(false);
        return;
    }
    LOG("insert edge (%g -> %g) below vertex %g\n", edge->fTop->fID, edge->fBottom->fID, v->fID);
    Edge* prev = NULL;
    Edge* next;
    for (next = v->fFirstEdgeBelow; next; next = next->fNextEdgeBelow) {
        if (next->isRightOf(edge->fBottom)) {
            break;
        }
        prev = next;
    }
    insert<Edge, &Edge::fPrevEdgeBelow, &Edge::fNextEdgeBelow>(
        edge, prev, next, &v->fFirstEdgeBelow, &v->fLastEdgeBelow);
}

void remove_edge_above(Edge* edge) {
    LOG("removing edge (%g -> %g) above vertex %g\n", edge->fTop->fID, edge->fBottom->fID,
        edge->fBottom->fID);
    remove<Edge, &Edge::fPrevEdgeAbove, &Edge::fNextEdgeAbove>(
        edge, &edge->fBottom->fFirstEdgeAbove, &edge->fBottom->fLastEdgeAbove);
}

void remove_edge_below(Edge* edge) {
    LOG("removing edge (%g -> %g) below vertex %g\n", edge->fTop->fID, edge->fBottom->fID,
        edge->fTop->fID);
    remove<Edge, &Edge::fPrevEdgeBelow, &Edge::fNextEdgeBelow>(
        edge, &edge->fTop->fFirstEdgeBelow, &edge->fTop->fLastEdgeBelow);
}

void erase_edge_if_zero_winding(Edge* edge, Edge** head) {
    if (edge->fWinding != 0) {
        return;
    }
    LOG("erasing edge (%g -> %g)\n", edge->fTop->fID, edge->fBottom->fID);
    remove_edge_above(edge);
    remove_edge_below(edge);
    if (edge->isActive(head)) {
        remove_edge(edge, head);
    }
}

void merge_collinear_edges(Edge* edge, Edge** activeEdges);

void set_top(Edge* edge, Vertex* v, Edge** activeEdges) {
    remove_edge_below(edge);
    edge->fTop = v;
    edge->recompute();
    insert_edge_below(edge, v);
    fix_active_state(edge, activeEdges);
    merge_collinear_edges(edge, activeEdges);
}

void set_bottom(Edge* edge, Vertex* v, Edge** activeEdges) {
    remove_edge_above(edge);
    edge->fBottom = v;
    edge->recompute();
    insert_edge_above(edge, v);
    fix_active_state(edge, activeEdges);
    merge_collinear_edges(edge, activeEdges);
}

void merge_edges_above(Edge* edge, Edge* other, Edge** activeEdges) {
    if (coincident(edge->fTop->fPoint, other->fTop->fPoint)) {
        LOG("merging coincident above edges (%g, %g) -> (%g, %g)\n",
            edge->fTop->fPoint.fX, edge->fTop->fPoint.fY,
            edge->fBottom->fPoint.fX, edge->fBottom->fPoint.fY);
        other->fWinding += edge->fWinding;
        erase_edge_if_zero_winding(other, activeEdges);
        edge->fWinding = 0;
        erase_edge_if_zero_winding(edge, activeEdges);
    } else if (sweep_lt(edge->fTop->fPoint, other->fTop->fPoint)) {
        other->fWinding += edge->fWinding;
        erase_edge_if_zero_winding(other, activeEdges);
        set_bottom(edge, other->fTop, activeEdges);
    } else {
        edge->fWinding += other->fWinding;
        erase_edge_if_zero_winding(edge, activeEdges);
        set_bottom(other, edge->fTop, activeEdges);
    }
}

void merge_edges_below(Edge* edge, Edge* other, Edge** activeEdges) {
    if (coincident(edge->fBottom->fPoint, other->fBottom->fPoint)) {
        LOG("merging coincident below edges (%g, %g) -> (%g, %g)\n",
            edge->fTop->fPoint.fX, edge->fTop->fPoint.fY,
            edge->fBottom->fPoint.fX, edge->fBottom->fPoint.fY);
        other->fWinding += edge->fWinding;
        erase_edge_if_zero_winding(other, activeEdges);
        edge->fWinding = 0;
        erase_edge_if_zero_winding(edge, activeEdges);
    } else if (sweep_lt(edge->fBottom->fPoint, other->fBottom->fPoint)) {
        edge->fWinding += other->fWinding;
        erase_edge_if_zero_winding(edge, activeEdges);
        set_top(other, edge->fBottom, activeEdges);
    } else {
        other->fWinding += edge->fWinding;
        erase_edge_if_zero_winding(other, activeEdges);
        set_top(edge, other->fBottom, activeEdges);
    }
}

void merge_collinear_edges(Edge* edge, Edge** activeEdges) {
    if (edge->fPrevEdgeAbove && (edge->fTop == edge->fPrevEdgeAbove->fTop ||
                                 !edge->fPrevEdgeAbove->isLeftOf(edge->fTop))) {
        merge_edges_above(edge, edge->fPrevEdgeAbove, activeEdges);
    } else if (edge->fNextEdgeAbove && (edge->fTop == edge->fNextEdgeAbove->fTop ||
                                        !edge->isLeftOf(edge->fNextEdgeAbove->fTop))) {
        merge_edges_above(edge, edge->fNextEdgeAbove, activeEdges);
    }
    if (edge->fPrevEdgeBelow && (edge->fBottom == edge->fPrevEdgeBelow->fBottom ||
                                 !edge->fPrevEdgeBelow->isLeftOf(edge->fBottom))) {
        merge_edges_below(edge, edge->fPrevEdgeBelow, activeEdges);
    } else if (edge->fNextEdgeBelow && (edge->fBottom == edge->fNextEdgeBelow->fBottom ||
                                        !edge->isLeftOf(edge->fNextEdgeBelow->fBottom))) {
        merge_edges_below(edge, edge->fNextEdgeBelow, activeEdges);
    }
}

void split_edge(Edge* edge, Vertex* v, Edge** activeEdges, SkChunkAlloc& alloc);

void cleanup_active_edges(Edge* edge, Edge** activeEdges, SkChunkAlloc& alloc) {
    Vertex* top = edge->fTop;
    Vertex* bottom = edge->fBottom;
    if (edge->fLeft) {
        Vertex* leftTop = edge->fLeft->fTop;
        Vertex* leftBottom = edge->fLeft->fBottom;
        if (sweep_gt(top->fPoint, leftTop->fPoint) && !edge->fLeft->isLeftOf(top)) {
            split_edge(edge->fLeft, edge->fTop, activeEdges, alloc);
        } else if (sweep_gt(leftTop->fPoint, top->fPoint) && !edge->isRightOf(leftTop)) {
            split_edge(edge, leftTop, activeEdges, alloc);
        } else if (sweep_lt(bottom->fPoint, leftBottom->fPoint) && !edge->fLeft->isLeftOf(bottom)) {
            split_edge(edge->fLeft, bottom, activeEdges, alloc);
        } else if (sweep_lt(leftBottom->fPoint, bottom->fPoint) && !edge->isRightOf(leftBottom)) {
            split_edge(edge, leftBottom, activeEdges, alloc);
        }
    }
    if (edge->fRight) {
        Vertex* rightTop = edge->fRight->fTop;
        Vertex* rightBottom = edge->fRight->fBottom;
        if (sweep_gt(top->fPoint, rightTop->fPoint) && !edge->fRight->isRightOf(top)) {
            split_edge(edge->fRight, top, activeEdges, alloc);
        } else if (sweep_gt(rightTop->fPoint, top->fPoint) && !edge->isLeftOf(rightTop)) {
            split_edge(edge, rightTop, activeEdges, alloc);
        } else if (sweep_lt(bottom->fPoint, rightBottom->fPoint) &&
                   !edge->fRight->isRightOf(bottom)) {
            split_edge(edge->fRight, bottom, activeEdges, alloc);
        } else if (sweep_lt(rightBottom->fPoint, bottom->fPoint) &&
                   !edge->isLeftOf(rightBottom)) {
            split_edge(edge, rightBottom, activeEdges, alloc);
        }
    }
}

void split_edge(Edge* edge, Vertex* v, Edge** activeEdges, SkChunkAlloc& alloc) {
    LOG("splitting edge (%g -> %g) at vertex %g (%g, %g)\n",
        edge->fTop->fID, edge->fBottom->fID,
        v->fID, v->fPoint.fX, v->fPoint.fY);
    if (sweep_lt(v->fPoint, edge->fTop->fPoint)) {
        set_top(edge, v, activeEdges);
    } else if (sweep_gt(v->fPoint, edge->fBottom->fPoint)) {
        set_bottom(edge, v, activeEdges);
    } else {
        Edge* newEdge = ALLOC_NEW(Edge, (v, edge->fBottom, edge->fWinding), alloc);
        insert_edge_below(newEdge, v);
        insert_edge_above(newEdge, edge->fBottom);
        set_bottom(edge, v, activeEdges);
        cleanup_active_edges(edge, activeEdges, alloc);
        fix_active_state(newEdge, activeEdges);
        merge_collinear_edges(newEdge, activeEdges);
    }
}

void merge_vertices(Vertex* src, Vertex* dst, Vertex** head, SkChunkAlloc& alloc) {
    LOG("found coincident verts at %g, %g; merging %g into %g\n", src->fPoint.fX, src->fPoint.fY,
        src->fID, dst->fID);
    for (Edge* edge = src->fFirstEdgeAbove; edge;) {
        Edge* next = edge->fNextEdgeAbove;
        set_bottom(edge, dst, NULL);
        edge = next;
    }
    for (Edge* edge = src->fFirstEdgeBelow; edge;) {
        Edge* next = edge->fNextEdgeBelow;
        set_top(edge, dst, NULL);
        edge = next;
    }
    remove<Vertex, &Vertex::fPrev, &Vertex::fNext>(src, head, NULL);
}

Vertex* check_for_intersection(Edge* edge, Edge* other, Edge** activeEdges, SkChunkAlloc& alloc) {
    SkPoint p;
    if (!edge || !other) {
        return NULL;
    }
    if (edge->intersect(*other, &p)) {
        Vertex* v;
        LOG("found intersection, pt is %g, %g\n", p.fX, p.fY);
        if (p == edge->fTop->fPoint || sweep_lt(p, edge->fTop->fPoint)) {
            split_edge(other, edge->fTop, activeEdges, alloc);
            v = edge->fTop;
        } else if (p == edge->fBottom->fPoint || sweep_gt(p, edge->fBottom->fPoint)) {
            split_edge(other, edge->fBottom, activeEdges, alloc);
            v = edge->fBottom;
        } else if (p == other->fTop->fPoint || sweep_lt(p, other->fTop->fPoint)) {
            split_edge(edge, other->fTop, activeEdges, alloc);
            v = other->fTop;
        } else if (p == other->fBottom->fPoint || sweep_gt(p, other->fBottom->fPoint)) {
            split_edge(edge, other->fBottom, activeEdges, alloc);
            v = other->fBottom;
        } else {
            Vertex* nextV = edge->fTop;
            while (sweep_lt(p, nextV->fPoint)) {
                nextV = nextV->fPrev;
            }
            while (sweep_lt(nextV->fPoint, p)) {
                nextV = nextV->fNext;
            }
            Vertex* prevV = nextV->fPrev;
            if (coincident(prevV->fPoint, p)) {
                v = prevV;
            } else if (coincident(nextV->fPoint, p)) {
                v = nextV;
            } else {
                v = ALLOC_NEW(Vertex, (p), alloc);
                LOG("inserting between %g (%g, %g) and %g (%g, %g)\n",
                    prevV->fID, prevV->fPoint.fX, prevV->fPoint.fY,
                    nextV->fID, nextV->fPoint.fX, nextV->fPoint.fY);
#if LOGGING_ENABLED
                v->fID = (nextV->fID + prevV->fID) * 0.5f;
#endif
                v->fPrev = prevV;
                v->fNext = nextV;
                prevV->fNext = v;
                nextV->fPrev = v;
            }
            split_edge(edge, v, activeEdges, alloc);
            split_edge(other, v, activeEdges, alloc);
        }
#ifdef SK_DEBUG
        validate_connectivity(v);
#endif
        return v;
    }
    return NULL;
}

void sanitize_contours(Vertex** contours, int contourCnt) {
    for (int i = 0; i < contourCnt; ++i) {
        SkASSERT(contours[i]);
        for (Vertex* v = contours[i];;) {
            if (coincident(v->fPrev->fPoint, v->fPoint)) {
                LOG("vertex %g,%g coincident; removing\n", v->fPoint.fX, v->fPoint.fY);
                if (v->fPrev == v) {
                    contours[i] = NULL;
                    break;
                }
                v->fPrev->fNext = v->fNext;
                v->fNext->fPrev = v->fPrev;
                if (contours[i] == v) {
                    contours[i] = v->fNext;
                }
                v = v->fPrev;
            } else {
                v = v->fNext;
                if (v == contours[i]) break;
            }
        }
    }
}

void merge_coincident_vertices(Vertex** vertices, SkChunkAlloc& alloc) {
    for (Vertex* v = (*vertices)->fNext; v != NULL; v = v->fNext) {
        if (sweep_lt(v->fPoint, v->fPrev->fPoint)) {
            v->fPoint = v->fPrev->fPoint;
        }
        if (coincident(v->fPrev->fPoint, v->fPoint)) {
            merge_vertices(v->fPrev, v, vertices, alloc);
        }
    }
}

// Stage 2: convert the contours to a mesh of edges connecting the vertices.

Vertex* build_edges(Vertex** contours, int contourCnt, SkChunkAlloc& alloc) {
    Vertex* vertices = NULL;
    Vertex* prev = NULL;
    for (int i = 0; i < contourCnt; ++i) {
        for (Vertex* v = contours[i]; v != NULL;) {
            Vertex* vNext = v->fNext;
            Edge* edge = new_edge(v->fPrev, v, alloc);
            if (edge->fWinding > 0) {
                insert_edge_below(edge, v->fPrev);
                insert_edge_above(edge, v);
            } else {
                insert_edge_below(edge, v);
                insert_edge_above(edge, v->fPrev);
            }
            merge_collinear_edges(edge, NULL);
            if (prev) {
                prev->fNext = v;
                v->fPrev = prev;
            } else {
                vertices = v;
            }
            prev = v;
            v = vNext;
            if (v == contours[i]) break;
        }
    }
    if (prev) {
        prev->fNext = vertices->fPrev = NULL;
    }
    return vertices;
}

// Stage 3: sort the vertices by increasing Y (or X if SWEEP_IN_X is on).

Vertex* sorted_merge(Vertex* a, Vertex* b);

void front_back_split(Vertex* v, Vertex** pFront, Vertex** pBack) {
    Vertex* fast;
    Vertex* slow;
    if (!v || !v->fNext) {
        *pFront = v;
        *pBack = NULL;
    } else {
        slow = v;
        fast = v->fNext;

        while (fast != NULL) {
            fast = fast->fNext;
            if (fast != NULL) {
                slow = slow->fNext;
                fast = fast->fNext;
            }
        }

        *pFront = v;
        *pBack = slow->fNext;
        slow->fNext->fPrev = NULL;
        slow->fNext = NULL;
    }
}

void merge_sort(Vertex** head) {
    if (!*head || !(*head)->fNext) {
        return;
    }

    Vertex* a;
    Vertex* b;
    front_back_split(*head, &a, &b);

    merge_sort(&a);
    merge_sort(&b);

    *head = sorted_merge(a, b);
}

Vertex* sorted_merge(Vertex* a, Vertex* b) {
    if (!a) {
        return b;
    } else if (!b) {
        return a;
    }

    Vertex* result = NULL;

    if (sweep_lt(a->fPoint, b->fPoint)) {
        result = a;
        result->fNext = sorted_merge(a->fNext, b);
    } else {
        result = b;
        result->fNext = sorted_merge(a, b->fNext);
    }
    result->fNext->fPrev = result;
    return result;
}

// Stage 4: Simplify the mesh by inserting new vertices at intersecting edges.

void simplify(Vertex* vertices, SkChunkAlloc& alloc) {
    LOG("simplifying complex polygons\n");
    Edge* activeEdges = NULL;
    for (Vertex* v = vertices; v != NULL; v = v->fNext) {
        if (!v->fFirstEdgeAbove && !v->fFirstEdgeBelow) {
            continue;
        }
#if LOGGING_ENABLED
        LOG("\nvertex %g: (%g,%g)\n", v->fID, v->fPoint.fX, v->fPoint.fY);
#endif
#ifdef SK_DEBUG
        validate_connectivity(v);
#endif
        Edge* leftEnclosingEdge = NULL;
        Edge* rightEnclosingEdge = NULL;
        bool restartChecks;
        do {
            restartChecks = false;
            find_enclosing_edges(v, activeEdges, &leftEnclosingEdge, &rightEnclosingEdge);
            if (v->fFirstEdgeBelow) {
                for (Edge* edge = v->fFirstEdgeBelow; edge != NULL; edge = edge->fNextEdgeBelow) {
                    if (check_for_intersection(edge, leftEnclosingEdge, &activeEdges, alloc)) {
                        restartChecks = true;
                        break;
                    }
                    if (check_for_intersection(edge, rightEnclosingEdge, &activeEdges, alloc)) {
                        restartChecks = true;
                        break;
                    }
                }
            } else {
                if (Vertex* pv = check_for_intersection(leftEnclosingEdge, rightEnclosingEdge,
                                                        &activeEdges, alloc)) {
                    if (sweep_lt(pv->fPoint, v->fPoint)) {
                        v = pv;
                    }
                    restartChecks = true;
                }

            }
        } while (restartChecks);
        SkASSERT(!leftEnclosingEdge || leftEnclosingEdge->isLeftOf(v));
        SkASSERT(!rightEnclosingEdge || rightEnclosingEdge->isRightOf(v));
#ifdef SK_DEBUG
        validate_edges(activeEdges);
#endif
        for (Edge* e = v->fFirstEdgeAbove; e; e = e->fNextEdgeAbove) {
            remove_edge(e, &activeEdges);
        }
        Edge* leftEdge = leftEnclosingEdge;
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            insert_edge(e, leftEdge, &activeEdges);
            leftEdge = e;
        }
        v->fProcessed = true;
    }
}

// Stage 5: Tessellate the simplified mesh into monotone polygons.

Poly* tessellate(Vertex* vertices, SkChunkAlloc& alloc) {
    LOG("tessellating simple polygons\n");
    Edge* activeEdges = NULL;
    Poly* polys = NULL;
    for (Vertex* v = vertices; v != NULL; v = v->fNext) {
        if (!v->fFirstEdgeAbove && !v->fFirstEdgeBelow) {
            continue;
        }
#if LOGGING_ENABLED
        LOG("\nvertex %g: (%g,%g)\n", v->fID, v->fPoint.fX, v->fPoint.fY);
#endif
#ifdef SK_DEBUG
        validate_connectivity(v);
#endif
        Edge* leftEnclosingEdge = NULL;
        Edge* rightEnclosingEdge = NULL;
        find_enclosing_edges(v, activeEdges, &leftEnclosingEdge, &rightEnclosingEdge);
        SkASSERT(!leftEnclosingEdge || leftEnclosingEdge->isLeftOf(v));
        SkASSERT(!rightEnclosingEdge || rightEnclosingEdge->isRightOf(v));
#ifdef SK_DEBUG
        validate_edges(activeEdges);
#endif
        Poly* leftPoly = NULL;
        Poly* rightPoly = NULL;
        if (v->fFirstEdgeAbove) {
            leftPoly = v->fFirstEdgeAbove->fLeftPoly;
            rightPoly = v->fLastEdgeAbove->fRightPoly;
        } else {
            leftPoly = leftEnclosingEdge ? leftEnclosingEdge->fRightPoly : NULL;
            rightPoly = rightEnclosingEdge ? rightEnclosingEdge->fLeftPoly : NULL;
        }
#if LOGGING_ENABLED
        LOG("edges above:\n");
        for (Edge* e = v->fFirstEdgeAbove; e; e = e->fNextEdgeAbove) {
            LOG("%g -> %g, lpoly %d, rpoly %d\n", e->fTop->fID, e->fBottom->fID,
                e->fLeftPoly ? e->fLeftPoly->fID : -1, e->fRightPoly ? e->fRightPoly->fID : -1);
        }
        LOG("edges below:\n");
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            LOG("%g -> %g, lpoly %d, rpoly %d\n", e->fTop->fID, e->fBottom->fID,
                e->fLeftPoly ? e->fLeftPoly->fID : -1, e->fRightPoly ? e->fRightPoly->fID : -1);
        }
#endif
        if (v->fFirstEdgeAbove) {
            if (leftPoly) {
                leftPoly = leftPoly->addVertex(v, Poly::kRight_Side, alloc);
            }
            if (rightPoly) {
                rightPoly = rightPoly->addVertex(v, Poly::kLeft_Side, alloc);
            }
            for (Edge* e = v->fFirstEdgeAbove; e != v->fLastEdgeAbove; e = e->fNextEdgeAbove) {
                Edge* leftEdge = e;
                Edge* rightEdge = e->fNextEdgeAbove;
                SkASSERT(rightEdge->isRightOf(leftEdge->fTop));
                remove_edge(leftEdge, &activeEdges);
                if (leftEdge->fRightPoly) {
                    leftEdge->fRightPoly->end(v, alloc);
                }
                if (rightEdge->fLeftPoly && rightEdge->fLeftPoly != leftEdge->fRightPoly) {
                    rightEdge->fLeftPoly->end(v, alloc);
                }
            }
            remove_edge(v->fLastEdgeAbove, &activeEdges);
            if (!v->fFirstEdgeBelow) {
                if (leftPoly && rightPoly && leftPoly != rightPoly) {
                    SkASSERT(leftPoly->fPartner == NULL && rightPoly->fPartner == NULL);
                    rightPoly->fPartner = leftPoly;
                    leftPoly->fPartner = rightPoly;
                }
            }
        }
        if (v->fFirstEdgeBelow) {
            if (!v->fFirstEdgeAbove) {
                if (leftPoly && leftPoly == rightPoly) {
                    // Split the poly.
                    if (leftPoly->fActive->fSide == Poly::kLeft_Side) {
                        leftPoly = new_poly(&polys, leftEnclosingEdge->fTop, leftPoly->fWinding,
                                            alloc);
                        leftPoly->addVertex(v, Poly::kRight_Side, alloc);
                        rightPoly->addVertex(v, Poly::kLeft_Side, alloc);
                        leftEnclosingEdge->fRightPoly = leftPoly;
                    } else {
                        rightPoly = new_poly(&polys, rightEnclosingEdge->fTop, rightPoly->fWinding,
                                             alloc);
                        rightPoly->addVertex(v, Poly::kLeft_Side, alloc);
                        leftPoly->addVertex(v, Poly::kRight_Side, alloc);
                        rightEnclosingEdge->fLeftPoly = rightPoly;
                    }
                } else {
                    if (leftPoly) {
                        leftPoly = leftPoly->addVertex(v, Poly::kRight_Side, alloc);
                    }
                    if (rightPoly) {
                        rightPoly = rightPoly->addVertex(v, Poly::kLeft_Side, alloc);
                    }
                }
            }
            Edge* leftEdge = v->fFirstEdgeBelow;
            leftEdge->fLeftPoly = leftPoly;
            insert_edge(leftEdge, leftEnclosingEdge, &activeEdges);
            for (Edge* rightEdge = leftEdge->fNextEdgeBelow; rightEdge;
                 rightEdge = rightEdge->fNextEdgeBelow) {
                insert_edge(rightEdge, leftEdge, &activeEdges);
                int winding = leftEdge->fLeftPoly ? leftEdge->fLeftPoly->fWinding : 0;
                winding += leftEdge->fWinding;
                if (winding != 0) {
                    Poly* poly = new_poly(&polys, v, winding, alloc);
                    leftEdge->fRightPoly = rightEdge->fLeftPoly = poly;
                }
                leftEdge = rightEdge;
            }
            v->fLastEdgeBelow->fRightPoly = rightPoly;
        }
#ifdef SK_DEBUG
        validate_edges(activeEdges);
#endif
#if LOGGING_ENABLED
        LOG("\nactive edges:\n");
        for (Edge* e = activeEdges; e != NULL; e = e->fRight) {
            LOG("%g -> %g, lpoly %d, rpoly %d\n", e->fTop->fID, e->fBottom->fID,
                e->fLeftPoly ? e->fLeftPoly->fID : -1, e->fRightPoly ? e->fRightPoly->fID : -1);
        }
#endif
    }
    return polys;
}

// This is a driver function which calls stages 2-5 in turn.

Poly* contours_to_polys(Vertex** contours, int contourCnt, SkChunkAlloc& alloc) {
#if LOGGING_ENABLED
    for (int i = 0; i < contourCnt; ++i) {
        Vertex* v = contours[i];
        SkASSERT(v);
        LOG("path.moveTo(%20.20g, %20.20g);\n", v->fPoint.fX, v->fPoint.fY);
        for (v = v->fNext; v != contours[i]; v = v->fNext) {
            LOG("path.lineTo(%20.20g, %20.20g);\n", v->fPoint.fX, v->fPoint.fY);
        }
    }
#endif
    sanitize_contours(contours, contourCnt);
    Vertex* vertices = build_edges(contours, contourCnt, alloc);
    if (!vertices) {
        return NULL;
    }

    // Sort vertices in Y (secondarily in X).
    merge_sort(&vertices);
    merge_coincident_vertices(&vertices, alloc);
#if LOGGING_ENABLED
    for (Vertex* v = vertices; v != NULL; v = v->fNext) {
        static float gID = 0.0f;
        v->fID = gID++;
    }
#endif
    simplify(vertices, alloc);
    return tessellate(vertices, alloc);
}

// Stage 6: Triangulate the monotone polygons into a vertex buffer.

void* polys_to_triangles(Poly* polys, SkPath::FillType fillType, void* data) {
    void* d = data;
    for (Poly* poly = polys; poly; poly = poly->fNext) {
        if (apply_fill_type(fillType, poly->fWinding)) {
            d = poly->emit(d);
        }
    }
    return d;
}

};

GrTessellatingPathRenderer::GrTessellatingPathRenderer() {
}

GrPathRenderer::StencilSupport GrTessellatingPathRenderer::onGetStencilSupport(
                                                            const GrDrawTarget*,
                                                            const GrPipelineBuilder*,
                                                            const SkPath&,
                                                            const SkStrokeRec&) const {
    return GrPathRenderer::kNoSupport_StencilSupport;
}

bool GrTessellatingPathRenderer::canDrawPath(const GrDrawTarget* target,
                                             const GrPipelineBuilder* pipelineBuilder,
                                             const SkMatrix& viewMatrix,
                                             const SkPath& path,
                                             const SkStrokeRec& stroke,
                                             bool antiAlias) const {
    // This path renderer can draw all fill styles, but does not do antialiasing. It can do convex
    // and concave paths, but we'll leave the convex ones to simpler algorithms.
    return stroke.isFillStyle() && !antiAlias && !path.isConvex();
}

class TessellatingPathBatch : public GrBatch {
public:

    static GrBatch* Create(const GrColor& color,
                           const SkPath& path,
                           const SkMatrix& viewMatrix,
                           SkRect clipBounds) {
        return SkNEW_ARGS(TessellatingPathBatch, (color, path, viewMatrix, clipBounds));
    }

    const char* name() const SK_OVERRIDE { return "TessellatingPathBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setKnownFourComponents(fColor);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrPipelineInfo& init) SK_OVERRIDE {
        // Handle any color overrides
        if (init.fColorIgnored) {
            fColor = GrColor_ILLEGAL;
        } else if (GrColor_ILLEGAL != init.fOverrideColor) {
            fColor = init.fOverrideColor;
        }
        fPipelineInfo = init;
    }

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) SK_OVERRIDE {
        SkScalar tol = GrPathUtils::scaleToleranceToSrc(SK_Scalar1, fViewMatrix, fPath.getBounds());
        int contourCnt;
        int maxPts = GrPathUtils::worstCasePointCount(fPath, &contourCnt, tol);
        if (maxPts <= 0) {
            return;
        }
        if (maxPts > ((int)SK_MaxU16 + 1)) {
            SkDebugf("Path not rendered, too many verts (%d)\n", maxPts);
            return;
        }
        SkPath::FillType fillType = fPath.getFillType();
        if (SkPath::IsInverseFillType(fillType)) {
            contourCnt++;
        }

        LOG("got %d pts, %d contours\n", maxPts, contourCnt);
        uint32_t flags = GrDefaultGeoProcFactory::kPosition_GPType;
        SkAutoTUnref<const GrGeometryProcessor> gp(
            GrDefaultGeoProcFactory::Create(flags, fColor, fViewMatrix, SkMatrix::I()));
        batchTarget->initDraw(gp, pipeline);
        gp->initBatchTracker(batchTarget->currentBatchTracker(), fPipelineInfo);

        SkAutoTDeleteArray<Vertex*> contours(SkNEW_ARRAY(Vertex *, contourCnt));

        // For the initial size of the chunk allocator, estimate based on the point count:
        // one vertex per point for the initial passes, plus two for the vertices in the
        // resulting Polys, since the same point may end up in two Polys.  Assume minimal
        // connectivity of one Edge per Vertex (will grow for intersections).
        SkChunkAlloc alloc(maxPts * (3 * sizeof(Vertex) + sizeof(Edge)));
        path_to_contours(fPath, tol, fClipBounds, contours.get(), alloc);
        Poly* polys;
        polys = contours_to_polys(contours.get(), contourCnt, alloc);
        int count = 0;
        for (Poly* poly = polys; poly; poly = poly->fNext) {
            if (apply_fill_type(fillType, poly->fWinding) && poly->fCount >= 3) {
                count += (poly->fCount - 2) * (WIREFRAME ? 6 : 3);
            }
        }

        size_t stride = gp->getVertexStride();
        const GrVertexBuffer* vertexBuffer;
        int firstVertex;
        void* vertices = batchTarget->vertexPool()->makeSpace(stride,
                                                              count,
                                                              &vertexBuffer,
                                                              &firstVertex);

        if (!vertices) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        LOG("emitting %d verts\n", count);
        void* end = polys_to_triangles(polys, fillType, vertices);
        int actualCount = static_cast<int>(
            (static_cast<char*>(end) - static_cast<char*>(vertices)) / stride);
        LOG("actual count: %d\n", actualCount);
        SkASSERT(actualCount <= count);

        GrPrimitiveType primitiveType = WIREFRAME ? kLines_GrPrimitiveType
                                                  : kTriangles_GrPrimitiveType;
        GrDrawTarget::DrawInfo drawInfo;
        drawInfo.setPrimitiveType(primitiveType);
        drawInfo.setVertexBuffer(vertexBuffer);
        drawInfo.setStartVertex(firstVertex);
        drawInfo.setVertexCount(actualCount);
        drawInfo.setStartIndex(0);
        drawInfo.setIndexCount(0);
        batchTarget->draw(drawInfo);

        batchTarget->putBackVertices((size_t)(count - actualCount), stride);
        return;
    }

    bool onCombineIfPossible(GrBatch*) SK_OVERRIDE {
        return false;
    }

private:
    TessellatingPathBatch(const GrColor& color,
                          const SkPath& path,
                          const SkMatrix& viewMatrix,
                          const SkRect& clipBounds)
      : fColor(color)
      , fPath(path)
      , fViewMatrix(viewMatrix)
      , fClipBounds(clipBounds) {
        this->initClassID<TessellatingPathBatch>();
    }

    GrColor        fColor;
    SkPath         fPath;
    SkMatrix       fViewMatrix;
    SkRect         fClipBounds; // in source space
    GrPipelineInfo fPipelineInfo;
};

bool GrTessellatingPathRenderer::onDrawPath(GrDrawTarget* target,
                                            GrPipelineBuilder* pipelineBuilder,
                                            GrColor color,
                                            const SkMatrix& viewM,
                                            const SkPath& path,
                                            const SkStrokeRec& stroke,
                                            bool antiAlias) {
    SkASSERT(!antiAlias);
    const GrRenderTarget* rt = pipelineBuilder->getRenderTarget();
    if (NULL == rt) {
        return false;
    }

    SkIRect clipBoundsI;
    pipelineBuilder->clip().getConservativeBounds(rt, &clipBoundsI);
    SkRect clipBounds = SkRect::Make(clipBoundsI);
    SkMatrix vmi;
    if (!viewM.invert(&vmi)) {
        return false;
    }
    vmi.mapRect(&clipBounds);
    SkAutoTUnref<GrBatch> batch(TessellatingPathBatch::Create(color, path, viewM, clipBounds));
    target->drawBatch(pipelineBuilder, batch);

    return true;
}
