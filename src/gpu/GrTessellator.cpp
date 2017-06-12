/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTessellator.h"

#include "GrDefaultGeoProcFactory.h"
#include "GrPathUtils.h"

#include "SkArenaAlloc.h"
#include "SkGeometry.h"
#include "SkPath.h"

#include <stdio.h>

/*
 * There are six stages to the basic algorithm:
 *
 * 1) Linearize the path contours into piecewise linear segments (path_to_contours()).
 * 2) Build a mesh of edges connecting the vertices (build_edges()).
 * 3) Sort the vertices in Y (and secondarily in X) (merge_sort()).
 * 4) Simplify the mesh by inserting new vertices at intersecting edges (simplify()).
 * 5) Tessellate the simplified mesh into monotone polygons (tessellate()).
 * 6) Triangulate the monotone polygons directly into a vertex buffer (polys_to_triangles()).
 *
 * For screenspace antialiasing, the algorithm is modified as follows:
 *
 * Run steps 1-5 above to produce polygons.
 * 5b) Apply fill rules to extract boundary contours from the polygons (extract_boundaries()).
 * 5c) Simplify boundaries to remove "pointy" vertices that cause inversions (simplify_boundary()).
 * 5d) Displace edges by half a pixel inward and outward along their normals. Intersect to find
 *     new vertices, and set zero alpha on the exterior and one alpha on the interior. Build a new
 *     antialiased mesh from those vertices (stroke_boundary()).
 * Run steps 3-6 above on the new mesh, and produce antialiased triangles.
 *
 * The vertex sorting in step (3) is a merge sort, since it plays well with the linked list
 * of vertices (and the necessity of inserting new vertices on intersection).
 *
 * Stages (4) and (5) use an active edge list -- a list of all edges for which the
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
 * points. This occurs in two ways:
 *
 * A) Intersections may cause a shortened edge to no longer be ordered with respect to its
 *    neighbouring edges at the top or bottom vertex. This is handled by merging the
 *    edges (merge_collinear_edges()).
 * B) Intersections may cause an edge to violate the left-to-right ordering of the
 *    active edge list. This is handled by detecting potential violations and rewinding
 *    the active edge list to the vertex before they occur (rewind() during merging,
 *    rewind_if_necessary() during splitting).
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
 * Note that the orientation of the line sweep algorithms is determined by the aspect ratio of the
 * path bounds. When the path is taller than it is wide, we sort vertices based on increasing Y
 * coordinate, and secondarily by increasing X coordinate. When the path is wider than it is tall,
 * we sort by increasing X coordinate, but secondarily by *decreasing* Y coordinate. This is so
 * that the "left" and "right" orientation in the code remains correct (edges to the left are
 * increasing in Y; edges to the right are decreasing in Y). That is, the setting rotates 90
 * degrees counterclockwise, rather that transposing.
 */

#define LOGGING_ENABLED 0

#if LOGGING_ENABLED
#define LOG printf
#else
#define LOG(...)
#endif

namespace {

const int kArenaChunkSize = 16 * 1024;

struct Vertex;
struct Edge;
struct Poly;

template <class T, T* T::*Prev, T* T::*Next>
void list_insert(T* t, T* prev, T* next, T** head, T** tail) {
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
void list_remove(T* t, T** head, T** tail) {
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
    t->*Prev = t->*Next = nullptr;
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
  Vertex(const SkPoint& point, uint8_t alpha)
    : fPoint(point), fPrev(nullptr), fNext(nullptr)
    , fFirstEdgeAbove(nullptr), fLastEdgeAbove(nullptr)
    , fFirstEdgeBelow(nullptr), fLastEdgeBelow(nullptr)
    , fLeftEnclosingEdge(nullptr), fRightEnclosingEdge(nullptr)
    , fPartner(nullptr)
    , fAlpha(alpha)
#if LOGGING_ENABLED
    , fID (-1.0f)
#endif
    {}
    SkPoint fPoint;               // Vertex position
    Vertex* fPrev;                // Linked list of contours, then Y-sorted vertices.
    Vertex* fNext;                // "
    Edge*   fFirstEdgeAbove;      // Linked list of edges above this vertex.
    Edge*   fLastEdgeAbove;       // "
    Edge*   fFirstEdgeBelow;      // Linked list of edges below this vertex.
    Edge*   fLastEdgeBelow;       // "
    Edge*   fLeftEnclosingEdge;   // Nearest edge in the AEL left of this vertex.
    Edge*   fRightEnclosingEdge;  // Nearest edge in the AEL right of this vertex.
    Vertex* fPartner;             // Corresponding inner or outer vertex (for AA).
    uint8_t fAlpha;
#if LOGGING_ENABLED
    float   fID;                  // Identifier used for logging.
#endif
};

/***************************************************************************************/

struct AAParams {
    bool fTweakAlpha;
    GrColor fColor;
};

typedef bool (*CompareFunc)(const SkPoint& a, const SkPoint& b);

bool sweep_lt_horiz(const SkPoint& a, const SkPoint& b) {
    return a.fX < b.fX || (a.fX == b.fX && a.fY > b.fY);
}

bool sweep_lt_vert(const SkPoint& a, const SkPoint& b) {
    return a.fY < b.fY || (a.fY == b.fY && a.fX < b.fX);
}

struct Comparator {
    enum class Direction { kVertical, kHorizontal };
    Comparator(Direction direction) : fDirection(direction) {}
    bool sweep_lt(const SkPoint& a, const SkPoint& b) const {
        return fDirection == Direction::kHorizontal ? sweep_lt_horiz(a, b) : sweep_lt_vert(a, b);
    }
    Direction fDirection;
};

inline void* emit_vertex(Vertex* v, const AAParams* aaParams, void* data) {
    if (!aaParams) {
        SkPoint* d = static_cast<SkPoint*>(data);
        *d++ = v->fPoint;
        return d;
    }
    if (aaParams->fTweakAlpha) {
        auto d = static_cast<GrDefaultGeoProcFactory::PositionColorAttr*>(data);
        d->fPosition = v->fPoint;
        d->fColor = SkAlphaMulQ(aaParams->fColor, SkAlpha255To256(v->fAlpha));
        d++;
        return d;
    }
    auto d = static_cast<GrDefaultGeoProcFactory::PositionColorCoverageAttr*>(data);
    d->fPosition = v->fPoint;
    d->fColor = aaParams->fColor;
    d->fCoverage = GrNormalizeByteToFloat(v->fAlpha);
    d++;
    return d;
}

void* emit_triangle(Vertex* v0, Vertex* v1, Vertex* v2, const AAParams* aaParams, void* data) {
    LOG("emit_triangle (%g, %g) %d\n", v0->fPoint.fX, v0->fPoint.fY, v0->fAlpha);
    LOG("              (%g, %g) %d\n", v1->fPoint.fX, v1->fPoint.fY, v1->fAlpha);
    LOG("              (%g, %g) %d\n", v2->fPoint.fX, v2->fPoint.fY, v2->fAlpha);
#if TESSELLATOR_WIREFRAME
    data = emit_vertex(v0, aaParams, data);
    data = emit_vertex(v1, aaParams, data);
    data = emit_vertex(v1, aaParams, data);
    data = emit_vertex(v2, aaParams, data);
    data = emit_vertex(v2, aaParams, data);
    data = emit_vertex(v0, aaParams, data);
#else
    data = emit_vertex(v0, aaParams, data);
    data = emit_vertex(v1, aaParams, data);
    data = emit_vertex(v2, aaParams, data);
#endif
    return data;
}

struct VertexList {
    VertexList() : fHead(nullptr), fTail(nullptr) {}
    VertexList(Vertex* head, Vertex* tail) : fHead(head), fTail(tail) {}
    Vertex* fHead;
    Vertex* fTail;
    void insert(Vertex* v, Vertex* prev, Vertex* next) {
        list_insert<Vertex, &Vertex::fPrev, &Vertex::fNext>(v, prev, next, &fHead, &fTail);
    }
    void append(Vertex* v) {
        insert(v, fTail, nullptr);
    }
    void append(const VertexList& list) {
        if (!list.fHead) {
            return;
        }
        if (fTail) {
            fTail->fNext = list.fHead;
            list.fHead->fPrev = fTail;
        } else {
            fHead = list.fHead;
        }
        fTail = list.fTail;
    }
    void prepend(Vertex* v) {
        insert(v, nullptr, fHead);
    }
    void remove(Vertex* v) {
        list_remove<Vertex, &Vertex::fPrev, &Vertex::fNext>(v, &fHead, &fTail);
    }
    void close() {
        if (fHead && fTail) {
            fTail->fNext = fHead;
            fHead->fPrev = fTail;
        }
    }
};

// Round to nearest quarter-pixel. This is used for screenspace tessellation.

inline void round(SkPoint* p) {
    p->fX = SkScalarRoundToScalar(p->fX * SkFloatToScalar(4.0f)) * SkFloatToScalar(0.25f);
    p->fY = SkScalarRoundToScalar(p->fY * SkFloatToScalar(4.0f)) * SkFloatToScalar(0.25f);
}

// A line equation in implicit form. fA * x + fB * y + fC = 0, for all points (x, y) on the line.
struct Line {
    Line(Vertex* p, Vertex* q) : Line(p->fPoint, q->fPoint) {}
    Line(const SkPoint& p, const SkPoint& q)
        : fA(static_cast<double>(q.fY) - p.fY)      // a = dY
        , fB(static_cast<double>(p.fX) - q.fX)      // b = -dX
        , fC(static_cast<double>(p.fY) * q.fX -     // c = cross(q, p)
             static_cast<double>(p.fX) * q.fY) {}
    double dist(const SkPoint& p) const {
        return fA * p.fX + fB * p.fY + fC;
    }
    double magSq() const {
        return fA * fA + fB * fB;
    }

    // Compute the intersection of two (infinite) Lines.
    bool intersect(const Line& other, SkPoint* point) {
        double denom = fA * other.fB - fB * other.fA;
        if (denom == 0.0) {
            return false;
        }
        double scale = 1.0f / denom;
        point->fX = SkDoubleToScalar((fB * other.fC - other.fB * fC) * scale);
        point->fY = SkDoubleToScalar((other.fA * fC - fA * other.fC) * scale);
        round(point);
        return true;
    }
    double fA, fB, fC;
};

/**
 * An Edge joins a top Vertex to a bottom Vertex. Edge ordering for the list of "edges above" and
 * "edge below" a vertex as well as for the active edge list is handled by isLeftOf()/isRightOf().
 * Note that an Edge will give occasionally dist() != 0 for its own endpoints (because floating
 * point). For speed, that case is only tested by the callers that require it (e.g.,
 * rewind_if_necessary()). Edges also handle checking for intersection with other edges.
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
    enum class Type { kInner, kOuter, kConnector };
    Edge(Vertex* top, Vertex* bottom, int winding, Type type)
        : fWinding(winding)
        , fTop(top)
        , fBottom(bottom)
        , fType(type)
        , fLeft(nullptr)
        , fRight(nullptr)
        , fPrevEdgeAbove(nullptr)
        , fNextEdgeAbove(nullptr)
        , fPrevEdgeBelow(nullptr)
        , fNextEdgeBelow(nullptr)
        , fLeftPoly(nullptr)
        , fRightPoly(nullptr)
        , fLeftPolyPrev(nullptr)
        , fLeftPolyNext(nullptr)
        , fRightPolyPrev(nullptr)
        , fRightPolyNext(nullptr)
        , fUsedInLeftPoly(false)
        , fUsedInRightPoly(false)
        , fLine(top, bottom) {
        }
    int      fWinding;          // 1 == edge goes downward; -1 = edge goes upward.
    Vertex*  fTop;              // The top vertex in vertex-sort-order (sweep_lt).
    Vertex*  fBottom;           // The bottom vertex in vertex-sort-order.
    Type     fType;
    Edge*    fLeft;             // The linked list of edges in the active edge list.
    Edge*    fRight;            // "
    Edge*    fPrevEdgeAbove;    // The linked list of edges in the bottom Vertex's "edges above".
    Edge*    fNextEdgeAbove;    // "
    Edge*    fPrevEdgeBelow;    // The linked list of edges in the top Vertex's "edges below".
    Edge*    fNextEdgeBelow;    // "
    Poly*    fLeftPoly;         // The Poly to the left of this edge, if any.
    Poly*    fRightPoly;        // The Poly to the right of this edge, if any.
    Edge*    fLeftPolyPrev;
    Edge*    fLeftPolyNext;
    Edge*    fRightPolyPrev;
    Edge*    fRightPolyNext;
    bool     fUsedInLeftPoly;
    bool     fUsedInRightPoly;
    Line     fLine;
    double dist(const SkPoint& p) const {
        return fLine.dist(p);
    }
    bool isRightOf(Vertex* v) const {
        return fLine.dist(v->fPoint) < 0.0;
    }
    bool isLeftOf(Vertex* v) const {
        return fLine.dist(v->fPoint) > 0.0;
    }
    void recompute() {
        fLine = Line(fTop, fBottom);
    }
    bool intersect(const Edge& other, SkPoint* p, uint8_t* alpha = nullptr) {
        LOG("intersecting %g -> %g with %g -> %g\n",
               fTop->fID, fBottom->fID,
               other.fTop->fID, other.fBottom->fID);
        if (fTop == other.fTop || fBottom == other.fBottom) {
            return false;
        }
        double denom = fLine.fA * other.fLine.fB - fLine.fB * other.fLine.fA;
        if (denom == 0.0) {
            return false;
        }
        double dx = static_cast<double>(other.fTop->fPoint.fX) - fTop->fPoint.fX;
        double dy = static_cast<double>(other.fTop->fPoint.fY) - fTop->fPoint.fY;
        double sNumer = dy * other.fLine.fB + dx * other.fLine.fA;
        double tNumer = dy * fLine.fB + dx * fLine.fA;
        // If (sNumer / denom) or (tNumer / denom) is not in [0..1], exit early.
        // This saves us doing the divide below unless absolutely necessary.
        if (denom > 0.0 ? (sNumer < 0.0 || sNumer > denom || tNumer < 0.0 || tNumer > denom)
                        : (sNumer > 0.0 || sNumer < denom || tNumer > 0.0 || tNumer < denom)) {
            return false;
        }
        double s = sNumer / denom;
        SkASSERT(s >= 0.0 && s <= 1.0);
        p->fX = SkDoubleToScalar(fTop->fPoint.fX - s * fLine.fB);
        p->fY = SkDoubleToScalar(fTop->fPoint.fY + s * fLine.fA);
        if (alpha) {
            if (fType == Type::kConnector) {
                *alpha = (1.0 - s) * fTop->fAlpha + s * fBottom->fAlpha;
            } else if (other.fType == Type::kConnector) {
                double t = tNumer / denom;
                *alpha = (1.0 - t) * other.fTop->fAlpha + t * other.fBottom->fAlpha;
            } else if (fType == Type::kOuter && other.fType == Type::kOuter) {
                *alpha = 0;
            } else {
                *alpha = 255;
            }
        }
        return true;
    }
};

struct EdgeList {
    EdgeList() : fHead(nullptr), fTail(nullptr) {}
    Edge* fHead;
    Edge* fTail;
    void insert(Edge* edge, Edge* prev, Edge* next) {
        list_insert<Edge, &Edge::fLeft, &Edge::fRight>(edge, prev, next, &fHead, &fTail);
    }
    void append(Edge* e) {
        insert(e, fTail, nullptr);
    }
    void remove(Edge* edge) {
        list_remove<Edge, &Edge::fLeft, &Edge::fRight>(edge, &fHead, &fTail);
    }
    void removeAll() {
        while (fHead) {
            this->remove(fHead);
        }
    }
    void close() {
        if (fHead && fTail) {
            fTail->fRight = fHead;
            fHead->fLeft = fTail;
        }
    }
    bool contains(Edge* edge) const {
        return edge->fLeft || edge->fRight || fHead == edge;
    }
};

/***************************************************************************************/

struct Poly {
    Poly(Vertex* v, int winding)
        : fFirstVertex(v)
        , fWinding(winding)
        , fHead(nullptr)
        , fTail(nullptr)
        , fNext(nullptr)
        , fPartner(nullptr)
        , fCount(0)
    {
#if LOGGING_ENABLED
        static int gID = 0;
        fID = gID++;
        LOG("*** created Poly %d\n", fID);
#endif
    }
    typedef enum { kLeft_Side, kRight_Side } Side;
    struct MonotonePoly {
        MonotonePoly(Edge* edge, Side side)
            : fSide(side)
            , fFirstEdge(nullptr)
            , fLastEdge(nullptr)
            , fPrev(nullptr)
            , fNext(nullptr) {
            this->addEdge(edge);
        }
        Side          fSide;
        Edge*         fFirstEdge;
        Edge*         fLastEdge;
        MonotonePoly* fPrev;
        MonotonePoly* fNext;
        void addEdge(Edge* edge) {
            if (fSide == kRight_Side) {
                SkASSERT(!edge->fUsedInRightPoly);
                list_insert<Edge, &Edge::fRightPolyPrev, &Edge::fRightPolyNext>(
                    edge, fLastEdge, nullptr, &fFirstEdge, &fLastEdge);
                edge->fUsedInRightPoly = true;
            } else {
                SkASSERT(!edge->fUsedInLeftPoly);
                list_insert<Edge, &Edge::fLeftPolyPrev, &Edge::fLeftPolyNext>(
                    edge, fLastEdge, nullptr, &fFirstEdge, &fLastEdge);
                edge->fUsedInLeftPoly = true;
            }
        }

        void* emit(const AAParams* aaParams, void* data) {
            Edge* e = fFirstEdge;
            VertexList vertices;
            vertices.append(e->fTop);
            int count = 1;
            while (e != nullptr) {
                if (kRight_Side == fSide) {
                    vertices.append(e->fBottom);
                    e = e->fRightPolyNext;
                } else {
                    vertices.prepend(e->fBottom);
                    e = e->fLeftPolyNext;
                }
                count++;
            }
            Vertex* first = vertices.fHead;
            Vertex* v = first->fNext;
            while (v != vertices.fTail) {
                SkASSERT(v && v->fPrev && v->fNext);
                Vertex* prev = v->fPrev;
                Vertex* curr = v;
                Vertex* next = v->fNext;
                if (count == 3) {
                    return emit_triangle(prev, curr, next, aaParams, data);
                }
                double ax = static_cast<double>(curr->fPoint.fX) - prev->fPoint.fX;
                double ay = static_cast<double>(curr->fPoint.fY) - prev->fPoint.fY;
                double bx = static_cast<double>(next->fPoint.fX) - curr->fPoint.fX;
                double by = static_cast<double>(next->fPoint.fY) - curr->fPoint.fY;
                if (ax * by - ay * bx >= 0.0) {
                    data = emit_triangle(prev, curr, next, aaParams, data);
                    v->fPrev->fNext = v->fNext;
                    v->fNext->fPrev = v->fPrev;
                    count--;
                    if (v->fPrev == first) {
                        v = v->fNext;
                    } else {
                        v = v->fPrev;
                    }
                } else {
                    v = v->fNext;
                }
            }
            return data;
        }
    };
    Poly* addEdge(Edge* e, Side side, SkArenaAlloc& alloc) {
        LOG("addEdge (%g -> %g) to poly %d, %s side\n",
               e->fTop->fID, e->fBottom->fID, fID, side == kLeft_Side ? "left" : "right");
        Poly* partner = fPartner;
        Poly* poly = this;
        if (side == kRight_Side) {
            if (e->fUsedInRightPoly) {
                return this;
            }
        } else {
            if (e->fUsedInLeftPoly) {
                return this;
            }
        }
        if (partner) {
            fPartner = partner->fPartner = nullptr;
        }
        if (!fTail) {
            fHead = fTail = alloc.make<MonotonePoly>(e, side);
            fCount += 2;
        } else if (e->fBottom == fTail->fLastEdge->fBottom) {
            return poly;
        } else if (side == fTail->fSide) {
            fTail->addEdge(e);
            fCount++;
        } else {
            e = alloc.make<Edge>(fTail->fLastEdge->fBottom, e->fBottom, 1, Edge::Type::kInner);
            fTail->addEdge(e);
            fCount++;
            if (partner) {
                partner->addEdge(e, side, alloc);
                poly = partner;
            } else {
                MonotonePoly* m = alloc.make<MonotonePoly>(e, side);
                m->fPrev = fTail;
                fTail->fNext = m;
                fTail = m;
            }
        }
        return poly;
    }
    void* emit(const AAParams* aaParams, void *data) {
        if (fCount < 3) {
            return data;
        }
        LOG("emit() %d, size %d\n", fID, fCount);
        for (MonotonePoly* m = fHead; m != nullptr; m = m->fNext) {
            data = m->emit(aaParams, data);
        }
        return data;
    }
    Vertex* lastVertex() const { return fTail ? fTail->fLastEdge->fBottom : fFirstVertex; }
    Vertex* fFirstVertex;
    int fWinding;
    MonotonePoly* fHead;
    MonotonePoly* fTail;
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

Poly* new_poly(Poly** head, Vertex* v, int winding, SkArenaAlloc& alloc) {
    Poly* poly = alloc.make<Poly>(v, winding);
    poly->fNext = *head;
    *head = poly;
    return poly;
}

void append_point_to_contour(const SkPoint& p, VertexList* contour, SkArenaAlloc& alloc) {
    Vertex* v = alloc.make<Vertex>(p, 255);
#if LOGGING_ENABLED
    static float gID = 0.0f;
    v->fID = gID++;
#endif
    contour->append(v);
}

SkScalar quad_error_at(const SkPoint pts[3], SkScalar t, SkScalar u) {
    SkQuadCoeff quad(pts);
    SkPoint p0 = to_point(quad.eval(t - 0.5f * u));
    SkPoint mid = to_point(quad.eval(t));
    SkPoint p1 = to_point(quad.eval(t + 0.5f * u));
    if (!p0.isFinite() || !mid.isFinite() || !p1.isFinite()) {
        return 0;
    }
    return mid.distanceToLineSegmentBetweenSqd(p0, p1);
}

void append_quadratic_to_contour(const SkPoint pts[3], SkScalar toleranceSqd, VertexList* contour,
                                 SkArenaAlloc& alloc) {
    SkQuadCoeff quad(pts);
    Sk2s aa = quad.fA * quad.fA;
    SkScalar denom = 2.0f * (aa[0] + aa[1]);
    Sk2s ab = quad.fA * quad.fB;
    SkScalar t = denom ? (-ab[0] - ab[1]) / denom : 0.0f;
    int nPoints = 1;
    SkScalar u;
    // Test possible subdivision values only at the point of maximum curvature.
    // If it passes the flatness metric there, it'll pass everywhere.
    for (;;) {
        u = 1.0f / nPoints;
        if (quad_error_at(pts, t, u) < toleranceSqd) {
            break;
        }
        nPoints++;
    }
    for (int j = 1; j <= nPoints; j++) {
        append_point_to_contour(to_point(quad.eval(j * u)), contour, alloc);
    }
}

void generate_cubic_points(const SkPoint& p0,
                           const SkPoint& p1,
                           const SkPoint& p2,
                           const SkPoint& p3,
                           SkScalar tolSqd,
                           VertexList* contour,
                           int pointsLeft,
                           SkArenaAlloc& alloc) {
    SkScalar d1 = p1.distanceToLineSegmentBetweenSqd(p0, p3);
    SkScalar d2 = p2.distanceToLineSegmentBetweenSqd(p0, p3);
    if (pointsLeft < 2 || (d1 < tolSqd && d2 < tolSqd) ||
        !SkScalarIsFinite(d1) || !SkScalarIsFinite(d2)) {
        append_point_to_contour(p3, contour, alloc);
        return;
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
    generate_cubic_points(p0, q[0], r[0], s, tolSqd, contour, pointsLeft, alloc);
    generate_cubic_points(s, r[1], q[2], p3, tolSqd, contour, pointsLeft, alloc);
}

// Stage 1: convert the input path to a set of linear contours (linked list of Vertices).

void path_to_contours(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
                      VertexList* contours, SkArenaAlloc& alloc, bool *isLinear) {
    SkScalar toleranceSqd = tolerance * tolerance;

    SkPoint pts[4];
    *isLinear = true;
    VertexList* contour = contours;
    SkPath::Iter iter(path, false);
    if (path.isInverseFillType()) {
        SkPoint quad[4];
        clipBounds.toQuad(quad);
        for (int i = 3; i >= 0; i--) {
            append_point_to_contour(quad[i], contours, alloc);
        }
        contour++;
    }
    SkAutoConicToQuads converter;
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kConic_Verb: {
                SkScalar weight = iter.conicWeight();
                const SkPoint* quadPts = converter.computeQuads(pts, weight, toleranceSqd);
                for (int i = 0; i < converter.countQuads(); ++i) {
                    append_quadratic_to_contour(quadPts, toleranceSqd, contour, alloc);
                    quadPts += 2;
                }
                *isLinear = false;
                break;
            }
            case SkPath::kMove_Verb:
                if (contour->fHead) {
                    contour++;
                }
                append_point_to_contour(pts[0], contour, alloc);
                break;
            case SkPath::kLine_Verb: {
                append_point_to_contour(pts[1], contour, alloc);
                break;
            }
            case SkPath::kQuad_Verb: {
                append_quadratic_to_contour(pts, toleranceSqd, contour, alloc);
                *isLinear = false;
                break;
            }
            case SkPath::kCubic_Verb: {
                int pointsLeft = GrPathUtils::cubicPointCount(pts, tolerance);
                generate_cubic_points(pts[0], pts[1], pts[2], pts[3], toleranceSqd, contour,
                                      pointsLeft, alloc);
                *isLinear = false;
                break;
            }
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
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

inline bool apply_fill_type(SkPath::FillType fillType, Poly* poly) {
    return poly && apply_fill_type(fillType, poly->fWinding);
}

Edge* new_edge(Vertex* prev, Vertex* next, Edge::Type type, Comparator& c, SkArenaAlloc& alloc) {
    int winding = c.sweep_lt(prev->fPoint, next->fPoint) ? 1 : -1;
    Vertex* top = winding < 0 ? next : prev;
    Vertex* bottom = winding < 0 ? prev : next;
    return alloc.make<Edge>(top, bottom, winding, type);
}

void remove_edge(Edge* edge, EdgeList* edges) {
    LOG("removing edge %g -> %g\n", edge->fTop->fID, edge->fBottom->fID);
    SkASSERT(edges->contains(edge));
    edges->remove(edge);
}

void insert_edge(Edge* edge, Edge* prev, EdgeList* edges) {
    LOG("inserting edge %g -> %g\n", edge->fTop->fID, edge->fBottom->fID);
    SkASSERT(!edges->contains(edge));
    Edge* next = prev ? prev->fRight : edges->fHead;
    edges->insert(edge, prev, next);
}

void find_enclosing_edges(Vertex* v, EdgeList* edges, Edge** left, Edge** right) {
    if (v->fFirstEdgeAbove && v->fLastEdgeAbove) {
        *left = v->fFirstEdgeAbove->fLeft;
        *right = v->fLastEdgeAbove->fRight;
        return;
    }
    Edge* next = nullptr;
    Edge* prev;
    for (prev = edges->fTail; prev != nullptr; prev = prev->fLeft) {
        if (prev->isLeftOf(v)) {
            break;
        }
        next = prev;
    }
    *left = prev;
    *right = next;
}

void insert_edge_above(Edge* edge, Vertex* v, Comparator& c) {
    if (edge->fTop->fPoint == edge->fBottom->fPoint ||
        c.sweep_lt(edge->fBottom->fPoint, edge->fTop->fPoint)) {
        return;
    }
    LOG("insert edge (%g -> %g) above vertex %g\n", edge->fTop->fID, edge->fBottom->fID, v->fID);
    Edge* prev = nullptr;
    Edge* next;
    for (next = v->fFirstEdgeAbove; next; next = next->fNextEdgeAbove) {
        if (next->isRightOf(edge->fTop)) {
            break;
        }
        prev = next;
    }
    list_insert<Edge, &Edge::fPrevEdgeAbove, &Edge::fNextEdgeAbove>(
        edge, prev, next, &v->fFirstEdgeAbove, &v->fLastEdgeAbove);
}

void insert_edge_below(Edge* edge, Vertex* v, Comparator& c) {
    if (edge->fTop->fPoint == edge->fBottom->fPoint ||
        c.sweep_lt(edge->fBottom->fPoint, edge->fTop->fPoint)) {
        return;
    }
    LOG("insert edge (%g -> %g) below vertex %g\n", edge->fTop->fID, edge->fBottom->fID, v->fID);
    Edge* prev = nullptr;
    Edge* next;
    for (next = v->fFirstEdgeBelow; next; next = next->fNextEdgeBelow) {
        if (next->isRightOf(edge->fBottom)) {
            break;
        }
        prev = next;
    }
    list_insert<Edge, &Edge::fPrevEdgeBelow, &Edge::fNextEdgeBelow>(
        edge, prev, next, &v->fFirstEdgeBelow, &v->fLastEdgeBelow);
}

void remove_edge_above(Edge* edge) {
    LOG("removing edge (%g -> %g) above vertex %g\n", edge->fTop->fID, edge->fBottom->fID,
        edge->fBottom->fID);
    list_remove<Edge, &Edge::fPrevEdgeAbove, &Edge::fNextEdgeAbove>(
        edge, &edge->fBottom->fFirstEdgeAbove, &edge->fBottom->fLastEdgeAbove);
}

void remove_edge_below(Edge* edge) {
    LOG("removing edge (%g -> %g) below vertex %g\n", edge->fTop->fID, edge->fBottom->fID,
        edge->fTop->fID);
    list_remove<Edge, &Edge::fPrevEdgeBelow, &Edge::fNextEdgeBelow>(
        edge, &edge->fTop->fFirstEdgeBelow, &edge->fTop->fLastEdgeBelow);
}

void disconnect(Edge* edge)
{
    remove_edge_above(edge);
    remove_edge_below(edge);
}

void merge_collinear_edges(Edge* edge, EdgeList* activeEdges, Vertex** current, Comparator& c);

void rewind(EdgeList* activeEdges, Vertex** current, Vertex* dst, Comparator& c) {
    if (!current || *current == dst || c.sweep_lt((*current)->fPoint, dst->fPoint)) {
        return;
    }
    Vertex* v = *current;
    LOG("rewinding active edges from vertex %g to vertex %g\n", v->fID, dst->fID);
    while (v != dst) {
        v = v->fPrev;
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            remove_edge(e, activeEdges);
        }
        Edge* leftEdge = v->fLeftEnclosingEdge;
        for (Edge* e = v->fFirstEdgeAbove; e; e = e->fNextEdgeAbove) {
            insert_edge(e, leftEdge, activeEdges);
            leftEdge = e;
        }
    }
    *current = v;
}

void rewind_if_necessary(Edge* edge, EdgeList* activeEdges, Vertex** current, Comparator& c) {
    if (!activeEdges || !current) {
        return;
    }
    Vertex* top = edge->fTop;
    Vertex* bottom = edge->fBottom;
    if (edge->fLeft) {
        Vertex* leftTop = edge->fLeft->fTop;
        Vertex* leftBottom = edge->fLeft->fBottom;
        if (c.sweep_lt(leftTop->fPoint, top->fPoint) && !edge->fLeft->isLeftOf(top)) {
            rewind(activeEdges, current, leftTop, c);
        } else if (c.sweep_lt(top->fPoint, leftTop->fPoint) && !edge->isRightOf(leftTop)) {
            rewind(activeEdges, current, top, c);
        } else if (c.sweep_lt(bottom->fPoint, leftBottom->fPoint) &&
                   !edge->fLeft->isLeftOf(bottom)) {
            rewind(activeEdges, current, leftTop, c);
        } else if (c.sweep_lt(leftBottom->fPoint, bottom->fPoint) && !edge->isRightOf(leftBottom)) {
            rewind(activeEdges, current, top, c);
        }
    }
    if (edge->fRight) {
        Vertex* rightTop = edge->fRight->fTop;
        Vertex* rightBottom = edge->fRight->fBottom;
        if (c.sweep_lt(rightTop->fPoint, top->fPoint) && !edge->fRight->isRightOf(top)) {
            rewind(activeEdges, current, rightTop, c);
        } else if (c.sweep_lt(top->fPoint, rightTop->fPoint) && !edge->isLeftOf(rightTop)) {
            rewind(activeEdges, current, top, c);
        } else if (c.sweep_lt(bottom->fPoint, rightBottom->fPoint) &&
                   !edge->fRight->isRightOf(bottom)) {
            rewind(activeEdges, current, rightTop, c);
        } else if (c.sweep_lt(rightBottom->fPoint, bottom->fPoint) &&
                   !edge->isLeftOf(rightBottom)) {
            rewind(activeEdges, current, top, c);
        }
    }
}

void set_top(Edge* edge, Vertex* v, EdgeList* activeEdges, Vertex** current, Comparator& c) {
    remove_edge_below(edge);
    edge->fTop = v;
    edge->recompute();
    insert_edge_below(edge, v, c);
    rewind_if_necessary(edge, activeEdges, current, c);
    merge_collinear_edges(edge, activeEdges, current, c);
}

void set_bottom(Edge* edge, Vertex* v, EdgeList* activeEdges, Vertex** current, Comparator& c) {
    remove_edge_above(edge);
    edge->fBottom = v;
    edge->recompute();
    insert_edge_above(edge, v, c);
    rewind_if_necessary(edge, activeEdges, current, c);
    merge_collinear_edges(edge, activeEdges, current, c);
}

void merge_edges_above(Edge* edge, Edge* other, EdgeList* activeEdges, Vertex** current,
                       Comparator& c) {
    if (coincident(edge->fTop->fPoint, other->fTop->fPoint)) {
        LOG("merging coincident above edges (%g, %g) -> (%g, %g)\n",
            edge->fTop->fPoint.fX, edge->fTop->fPoint.fY,
            edge->fBottom->fPoint.fX, edge->fBottom->fPoint.fY);
        rewind(activeEdges, current, edge->fTop, c);
        other->fWinding += edge->fWinding;
        disconnect(edge);
    } else if (c.sweep_lt(edge->fTop->fPoint, other->fTop->fPoint)) {
        rewind(activeEdges, current, edge->fTop, c);
        other->fWinding += edge->fWinding;
        set_bottom(edge, other->fTop, activeEdges, current, c);
    } else {
        rewind(activeEdges, current, other->fTop, c);
        edge->fWinding += other->fWinding;
        set_bottom(other, edge->fTop, activeEdges, current, c);
    }
}

void merge_edges_below(Edge* edge, Edge* other, EdgeList* activeEdges, Vertex** current,
                       Comparator& c) {
    if (coincident(edge->fBottom->fPoint, other->fBottom->fPoint)) {
        LOG("merging coincident below edges (%g, %g) -> (%g, %g)\n",
            edge->fTop->fPoint.fX, edge->fTop->fPoint.fY,
            edge->fBottom->fPoint.fX, edge->fBottom->fPoint.fY);
        rewind(activeEdges, current, edge->fTop, c);
        other->fWinding += edge->fWinding;
        disconnect(edge);
    } else if (c.sweep_lt(edge->fBottom->fPoint, other->fBottom->fPoint)) {
        rewind(activeEdges, current, other->fTop, c);
        edge->fWinding += other->fWinding;
        set_top(other, edge->fBottom, activeEdges, current, c);
    } else {
        rewind(activeEdges, current, edge->fTop, c);
        other->fWinding += edge->fWinding;
        set_top(edge, other->fBottom, activeEdges, current, c);
    }
}

void merge_collinear_edges(Edge* edge, EdgeList* activeEdges, Vertex** current, Comparator& c) {
    for (;;) {
        if (edge->fPrevEdgeAbove && (edge->fTop == edge->fPrevEdgeAbove->fTop ||
                                     !edge->fPrevEdgeAbove->isLeftOf(edge->fTop))) {
            merge_edges_above(edge, edge->fPrevEdgeAbove, activeEdges, current, c);
        } else if (edge->fNextEdgeAbove && (edge->fTop == edge->fNextEdgeAbove->fTop ||
                                            !edge->isLeftOf(edge->fNextEdgeAbove->fTop))) {
            merge_edges_above(edge, edge->fNextEdgeAbove, activeEdges, current, c);
        } else if (edge->fPrevEdgeBelow && (edge->fBottom == edge->fPrevEdgeBelow->fBottom ||
                                     !edge->fPrevEdgeBelow->isLeftOf(edge->fBottom))) {
            merge_edges_below(edge, edge->fPrevEdgeBelow, activeEdges, current, c);
        } else if (edge->fNextEdgeBelow && (edge->fBottom == edge->fNextEdgeBelow->fBottom ||
                                            !edge->isLeftOf(edge->fNextEdgeBelow->fBottom))) {
            merge_edges_below(edge, edge->fNextEdgeBelow, activeEdges, current, c);
        } else {
            break;
        }
    }
    SkASSERT(!edge->fPrevEdgeAbove || edge->fPrevEdgeAbove->isLeftOf(edge->fTop));
    SkASSERT(!edge->fPrevEdgeBelow || edge->fPrevEdgeBelow->isLeftOf(edge->fBottom));
    SkASSERT(!edge->fNextEdgeAbove || edge->fNextEdgeAbove->isRightOf(edge->fTop));
    SkASSERT(!edge->fNextEdgeBelow || edge->fNextEdgeBelow->isRightOf(edge->fBottom));
}

void split_edge(Edge* edge, Vertex* v, EdgeList* activeEdges, Vertex** current, Comparator& c,
                SkArenaAlloc& alloc) {
    if (v == edge->fTop || v == edge->fBottom) {
        return;
    }
    LOG("splitting edge (%g -> %g) at vertex %g (%g, %g)\n",
        edge->fTop->fID, edge->fBottom->fID,
        v->fID, v->fPoint.fX, v->fPoint.fY);
    Vertex* top;
    Vertex* bottom;
    if (c.sweep_lt(v->fPoint, edge->fTop->fPoint)) {
        top = v;
        bottom = edge->fTop;
        set_top(edge, v, activeEdges, current, c);
    } else if (c.sweep_lt(edge->fBottom->fPoint, v->fPoint)) {
        top = edge->fBottom;
        bottom = v;
        set_bottom(edge, v, activeEdges, current, c);
    } else {
        top = v;
        bottom = edge->fBottom;
        set_bottom(edge, v, activeEdges, current, c);
    }
    Edge* newEdge = alloc.make<Edge>(top, bottom, edge->fWinding, edge->fType);
    insert_edge_below(newEdge, top, c);
    insert_edge_above(newEdge, bottom, c);
    merge_collinear_edges(newEdge, activeEdges, current, c);
}

Edge* connect(Vertex* prev, Vertex* next, Edge::Type type, Comparator& c, SkArenaAlloc& alloc,
              int winding_scale = 1) {
    Edge* edge = new_edge(prev, next, type, c, alloc);
    insert_edge_below(edge, edge->fTop, c);
    insert_edge_above(edge, edge->fBottom, c);
    edge->fWinding *= winding_scale;
    merge_collinear_edges(edge, nullptr, nullptr, c);
    return edge;
}

void merge_vertices(Vertex* src, Vertex* dst, VertexList* mesh, Comparator& c,
                    SkArenaAlloc& alloc) {
    LOG("found coincident verts at %g, %g; merging %g into %g\n", src->fPoint.fX, src->fPoint.fY,
        src->fID, dst->fID);
    dst->fAlpha = SkTMax(src->fAlpha, dst->fAlpha);
    if (src->fPartner) {
        src->fPartner->fPartner = dst;
    }
    for (Edge* edge = src->fFirstEdgeAbove; edge;) {
        Edge* next = edge->fNextEdgeAbove;
        set_bottom(edge, dst, nullptr, nullptr, c);
        edge = next;
    }
    for (Edge* edge = src->fFirstEdgeBelow; edge;) {
        Edge* next = edge->fNextEdgeBelow;
        set_top(edge, dst, nullptr, nullptr, c);
        edge = next;
    }
    mesh->remove(src);
}

uint8_t max_edge_alpha(Edge* a, Edge* b) {
    if (a->fType == Edge::Type::kInner || b->fType == Edge::Type::kInner) {
        return 255;
    } else if (a->fType == Edge::Type::kOuter && b->fType == Edge::Type::kOuter) {
        return 0;
    } else {
        return SkTMax(SkTMax(a->fTop->fAlpha, a->fBottom->fAlpha),
                      SkTMax(b->fTop->fAlpha, b->fBottom->fAlpha));
    }
}

bool check_for_intersection(Edge* edge, Edge* other, EdgeList* activeEdges, Vertex** current,
                            VertexList* mesh, Comparator& c, SkArenaAlloc& alloc) {
    if (!edge || !other) {
        return false;
    }
    SkPoint p;
    uint8_t alpha;
    if (edge->intersect(*other, &p, &alpha) && p.isFinite()) {
        Vertex* v;
        LOG("found intersection, pt is %g, %g\n", p.fX, p.fY);
        Vertex* top = *current;
        // If the intersection point is above the current vertex, rewind to the vertex above the
        // intersection.
        while (top && c.sweep_lt(p, top->fPoint)) {
            top = top->fPrev;
        }
        if (p == edge->fTop->fPoint) {
            v = edge->fTop;
        } else if (p == edge->fBottom->fPoint) {
            v = edge->fBottom;
        } else if (p == other->fTop->fPoint) {
            v = other->fTop;
        } else if (p == other->fBottom->fPoint) {
            v = other->fBottom;
        } else {
            Vertex* prevV = top;
            Vertex* nextV = top ? top->fNext : mesh->fHead;
            while (nextV && c.sweep_lt(nextV->fPoint, p)) {
                prevV = nextV;
                nextV = nextV->fNext;
            }
            if (prevV && coincident(prevV->fPoint, p)) {
                v = prevV;
            } else if (nextV && coincident(nextV->fPoint, p)) {
                v = nextV;
            } else {
                v = alloc.make<Vertex>(p, alpha);
#if LOGGING_ENABLED
                if (!prevV) {
                    v->fID = mesh->fHead->fID - 1.0f;
                } else if (!nextV) {
                    v->fID = mesh->fTail->fID + 1.0f;
                } else {
                    v->fID = (prevV->fID + nextV->fID) * 0.5f;
                }
#endif
                mesh->insert(v, prevV, nextV);
            }
        }
        rewind(activeEdges, current, top ? top : v, c);
        split_edge(edge, v, activeEdges, current, c, alloc);
        split_edge(other, v, activeEdges, current, c, alloc);
        v->fAlpha = SkTMax(v->fAlpha, alpha);
        return true;
    }
    return false;
}

void sanitize_contours(VertexList* contours, int contourCnt, bool approximate) {
    for (VertexList* contour = contours; contourCnt > 0; --contourCnt, ++contour) {
        SkASSERT(contour->fHead);
        Vertex* prev = contour->fTail;
        if (approximate) {
            round(&prev->fPoint);
        }
        for (Vertex* v = contour->fHead; v;) {
            if (approximate) {
                round(&v->fPoint);
            }
            Vertex* next = v->fNext;
            if (coincident(prev->fPoint, v->fPoint)) {
                LOG("vertex %g,%g coincident; removing\n", v->fPoint.fX, v->fPoint.fY);
                contour->remove(v);
            }
            prev = v;
            v = next;
        }
    }
}

void merge_coincident_vertices(VertexList* mesh, Comparator& c, SkArenaAlloc& alloc) {
    if (!mesh->fHead) {
        return;
    }
    for (Vertex* v = mesh->fHead->fNext; v != nullptr; v = v->fNext) {
        if (c.sweep_lt(v->fPoint, v->fPrev->fPoint)) {
            v->fPoint = v->fPrev->fPoint;
        }
        if (coincident(v->fPrev->fPoint, v->fPoint)) {
            merge_vertices(v->fPrev, v, mesh, c, alloc);
        }
    }
}

// Stage 2: convert the contours to a mesh of edges connecting the vertices.

void build_edges(VertexList* contours, int contourCnt, VertexList* mesh, Comparator& c,
                 SkArenaAlloc& alloc) {
    for (VertexList* contour = contours; contourCnt > 0; --contourCnt, ++contour) {
        Vertex* prev = contour->fTail;
        for (Vertex* v = contour->fHead; v;) {
            Vertex* next = v->fNext;
            connect(prev, v, Edge::Type::kInner, c, alloc);
            mesh->append(v);
            prev = v;
            v = next;
        }
    }
}

void connect_partners(VertexList* outerVertices, Comparator& c, SkArenaAlloc& alloc) {
    for (Vertex* outer = outerVertices->fHead; outer; outer = outer->fNext) {
        if (Vertex* inner = outer->fPartner) {
            // Connector edges get zero winding, since they're only structural (i.e., to ensure
            // no 0-0-0 alpha triangles are produced), and shouldn't affect the poly winding number.
            connect(outer, inner, Edge::Type::kConnector, c, alloc, 0);
            inner->fPartner = outer->fPartner = nullptr;
        }
    }
}

template <CompareFunc sweep_lt>
void sorted_merge(VertexList* front, VertexList* back, VertexList* result) {
    Vertex* a = front->fHead;
    Vertex* b = back->fHead;
    while (a && b) {
        if (sweep_lt(a->fPoint, b->fPoint)) {
            front->remove(a);
            result->append(a);
            a = front->fHead;
        } else {
            back->remove(b);
            result->append(b);
            b = back->fHead;
        }
    }
    result->append(*front);
    result->append(*back);
}

void sorted_merge(VertexList* front, VertexList* back, VertexList* result, Comparator& c) {
    if (c.fDirection == Comparator::Direction::kHorizontal) {
        sorted_merge<sweep_lt_horiz>(front, back, result);
    } else {
        sorted_merge<sweep_lt_vert>(front, back, result);
    }
#if LOGGING_ENABLED
    float id = 0.0f;
    for (Vertex* v = result->fHead; v; v = v->fNext) {
        v->fID = id++;
    }
#endif
}

// Stage 3: sort the vertices by increasing sweep direction.

template <CompareFunc sweep_lt>
void merge_sort(VertexList* vertices) {
    Vertex* slow = vertices->fHead;
    if (!slow) {
        return;
    }
    Vertex* fast = slow->fNext;
    if (!fast) {
        return;
    }
    do {
        fast = fast->fNext;
        if (fast) {
            fast = fast->fNext;
            slow = slow->fNext;
        }
    } while (fast);
    VertexList front(vertices->fHead, slow);
    VertexList back(slow->fNext, vertices->fTail);
    front.fTail->fNext = back.fHead->fPrev = nullptr;

    merge_sort<sweep_lt>(&front);
    merge_sort<sweep_lt>(&back);

    vertices->fHead = vertices->fTail = nullptr;
    sorted_merge<sweep_lt>(&front, &back, vertices);
}

// Stage 4: Simplify the mesh by inserting new vertices at intersecting edges.

void simplify(VertexList* mesh, Comparator& c, SkArenaAlloc& alloc) {
    LOG("simplifying complex polygons\n");
    EdgeList activeEdges;
    for (Vertex* v = mesh->fHead; v != nullptr; v = v->fNext) {
        if (!v->fFirstEdgeAbove && !v->fFirstEdgeBelow) {
            continue;
        }
        Edge* leftEnclosingEdge;
        Edge* rightEnclosingEdge;
        bool restartChecks;
        do {
            LOG("\nvertex %g: (%g,%g), alpha %d\n", v->fID, v->fPoint.fX, v->fPoint.fY, v->fAlpha);
            restartChecks = false;
            find_enclosing_edges(v, &activeEdges, &leftEnclosingEdge, &rightEnclosingEdge);
            if (rightEnclosingEdge && !rightEnclosingEdge->isRightOf(v)) {
                split_edge(rightEnclosingEdge, v, &activeEdges, &v, c, alloc);
                restartChecks = true;
                continue;
            }
            SkASSERT(!rightEnclosingEdge || rightEnclosingEdge->isRightOf(v));
            v->fLeftEnclosingEdge = leftEnclosingEdge;
            v->fRightEnclosingEdge = rightEnclosingEdge;
            if (v->fFirstEdgeBelow) {
                for (Edge* edge = v->fFirstEdgeBelow; edge; edge = edge->fNextEdgeBelow) {
                    if (check_for_intersection(edge, leftEnclosingEdge, &activeEdges, &v, mesh, c,
                                               alloc)) {
                        restartChecks = true;
                        break;
                    }
                    if (check_for_intersection(edge, rightEnclosingEdge, &activeEdges, &v, mesh, c,
                                               alloc)) {
                        restartChecks = true;
                        break;
                    }
                }
            } else {
                if (check_for_intersection(leftEnclosingEdge, rightEnclosingEdge,
                                           &activeEdges, &v, mesh, c, alloc)) {
                    restartChecks = true;
                }

            }
        } while (restartChecks);
        if (v->fAlpha == 0) {
            if ((leftEnclosingEdge && leftEnclosingEdge->fWinding < 0) &&
                (rightEnclosingEdge && rightEnclosingEdge->fWinding > 0)) {
                v->fAlpha = max_edge_alpha(leftEnclosingEdge, rightEnclosingEdge);
            }
        }
        for (Edge* e = v->fFirstEdgeAbove; e; e = e->fNextEdgeAbove) {
            remove_edge(e, &activeEdges);
        }
        Edge* leftEdge = leftEnclosingEdge;
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            insert_edge(e, leftEdge, &activeEdges);
            leftEdge = e;
        }
    }
}

// This is a stripped-down version of simplify() (the Bentley-Ottmann algorithm) that
// early-returns true on the first found intersection, false if none.
bool is_complex(const VertexList& vertices) {
    LOG("testing polygon complexity\n");
    EdgeList activeEdges;
    for (Vertex* v = vertices.fHead; v != nullptr; v = v->fNext) {
        if (!v->fFirstEdgeAbove && !v->fFirstEdgeBelow) {
            continue;
        }
        Edge* leftEnclosingEdge;
        Edge* rightEnclosingEdge;
        find_enclosing_edges(v, &activeEdges, &leftEnclosingEdge, &rightEnclosingEdge);
        SkPoint dummy;
        if (v->fFirstEdgeBelow) {
            for (Edge* edge = v->fFirstEdgeBelow; edge; edge = edge->fNextEdgeBelow) {
                if (edge && leftEnclosingEdge && edge->intersect(*leftEnclosingEdge, &dummy)) {
                    activeEdges.removeAll();
                    return true;
                }
                if (edge && rightEnclosingEdge && edge->intersect(*rightEnclosingEdge, &dummy)) {
                    activeEdges.removeAll();
                    return true;
                }
            }
        } else if (leftEnclosingEdge && rightEnclosingEdge &&
                   leftEnclosingEdge->intersect(*rightEnclosingEdge, &dummy)) {
            activeEdges.removeAll();
            return true;
        }
        for (Edge* e = v->fFirstEdgeAbove; e; e = e->fNextEdgeAbove) {
            remove_edge(e, &activeEdges);
        }
        Edge* leftEdge = leftEnclosingEdge;
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            insert_edge(e, leftEdge, &activeEdges);
            leftEdge = e;
        }
    }
    activeEdges.removeAll();
    return false;
}

// Stage 5: Tessellate the simplified mesh into monotone polygons.

Poly* tessellate(const VertexList& vertices, SkArenaAlloc& alloc) {
    LOG("tessellating simple polygons\n");
    EdgeList activeEdges;
    Poly* polys = nullptr;
    for (Vertex* v = vertices.fHead; v != nullptr; v = v->fNext) {
        if (!v->fFirstEdgeAbove && !v->fFirstEdgeBelow) {
            continue;
        }
#if LOGGING_ENABLED
        LOG("\nvertex %g: (%g,%g), alpha %d\n", v->fID, v->fPoint.fX, v->fPoint.fY, v->fAlpha);
#endif
        Edge* leftEnclosingEdge;
        Edge* rightEnclosingEdge;
        find_enclosing_edges(v, &activeEdges, &leftEnclosingEdge, &rightEnclosingEdge);
        Poly* leftPoly;
        Poly* rightPoly;
        if (v->fFirstEdgeAbove) {
            leftPoly = v->fFirstEdgeAbove->fLeftPoly;
            rightPoly = v->fLastEdgeAbove->fRightPoly;
        } else {
            leftPoly = leftEnclosingEdge ? leftEnclosingEdge->fRightPoly : nullptr;
            rightPoly = rightEnclosingEdge ? rightEnclosingEdge->fLeftPoly : nullptr;
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
                leftPoly = leftPoly->addEdge(v->fFirstEdgeAbove, Poly::kRight_Side, alloc);
            }
            if (rightPoly) {
                rightPoly = rightPoly->addEdge(v->fLastEdgeAbove, Poly::kLeft_Side, alloc);
            }
            for (Edge* e = v->fFirstEdgeAbove; e != v->fLastEdgeAbove; e = e->fNextEdgeAbove) {
                Edge* rightEdge = e->fNextEdgeAbove;
                remove_edge(e, &activeEdges);
                if (e->fRightPoly) {
                    e->fRightPoly->addEdge(e, Poly::kLeft_Side, alloc);
                }
                if (rightEdge->fLeftPoly && rightEdge->fLeftPoly != e->fRightPoly) {
                    rightEdge->fLeftPoly->addEdge(e, Poly::kRight_Side, alloc);
                }
            }
            remove_edge(v->fLastEdgeAbove, &activeEdges);
            if (!v->fFirstEdgeBelow) {
                if (leftPoly && rightPoly && leftPoly != rightPoly) {
                    SkASSERT(leftPoly->fPartner == nullptr && rightPoly->fPartner == nullptr);
                    rightPoly->fPartner = leftPoly;
                    leftPoly->fPartner = rightPoly;
                }
            }
        }
        if (v->fFirstEdgeBelow) {
            if (!v->fFirstEdgeAbove) {
                if (leftPoly && rightPoly) {
                    if (leftPoly == rightPoly) {
                        if (leftPoly->fTail && leftPoly->fTail->fSide == Poly::kLeft_Side) {
                            leftPoly = new_poly(&polys, leftPoly->lastVertex(),
                                                 leftPoly->fWinding, alloc);
                            leftEnclosingEdge->fRightPoly = leftPoly;
                        } else {
                            rightPoly = new_poly(&polys, rightPoly->lastVertex(),
                                                 rightPoly->fWinding, alloc);
                            rightEnclosingEdge->fLeftPoly = rightPoly;
                        }
                    }
                    Edge* join = alloc.make<Edge>(leftPoly->lastVertex(), v, 1, Edge::Type::kInner);
                    leftPoly = leftPoly->addEdge(join, Poly::kRight_Side, alloc);
                    rightPoly = rightPoly->addEdge(join, Poly::kLeft_Side, alloc);
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
#if LOGGING_ENABLED
        LOG("\nactive edges:\n");
        for (Edge* e = activeEdges.fHead; e != nullptr; e = e->fRight) {
            LOG("%g -> %g, lpoly %d, rpoly %d\n", e->fTop->fID, e->fBottom->fID,
                e->fLeftPoly ? e->fLeftPoly->fID : -1, e->fRightPoly ? e->fRightPoly->fID : -1);
        }
#endif
    }
    return polys;
}

void remove_non_boundary_edges(const VertexList& mesh, SkPath::FillType fillType,
                               SkArenaAlloc& alloc) {
    LOG("removing non-boundary edges\n");
    EdgeList activeEdges;
    for (Vertex* v = mesh.fHead; v != nullptr; v = v->fNext) {
        if (!v->fFirstEdgeAbove && !v->fFirstEdgeBelow) {
            continue;
        }
        Edge* leftEnclosingEdge;
        Edge* rightEnclosingEdge;
        find_enclosing_edges(v, &activeEdges, &leftEnclosingEdge, &rightEnclosingEdge);
        bool prevFilled = leftEnclosingEdge &&
                          apply_fill_type(fillType, leftEnclosingEdge->fWinding);
        for (Edge* e = v->fFirstEdgeAbove; e;) {
            Edge* next = e->fNextEdgeAbove;
            remove_edge(e, &activeEdges);
            bool filled = apply_fill_type(fillType, e->fWinding);
            if (filled == prevFilled) {
                disconnect(e);
            }
            prevFilled = filled;
            e = next;
        }
        Edge* prev = leftEnclosingEdge;
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            if (prev) {
                e->fWinding += prev->fWinding;
            }
            insert_edge(e, prev, &activeEdges);
            prev = e;
        }
    }
}

// Note: this is the normal to the edge, but not necessarily unit length.
void get_edge_normal(const Edge* e, SkVector* normal) {
    normal->set(SkDoubleToScalar(e->fLine.fA) * e->fWinding,
                SkDoubleToScalar(e->fLine.fB) * e->fWinding);
}

// Stage 5c: detect and remove "pointy" vertices whose edge normals point in opposite directions
// and whose adjacent vertices are less than a quarter pixel from an edge. These are guaranteed to
// invert on stroking.

void simplify_boundary(EdgeList* boundary, Comparator& c, SkArenaAlloc& alloc) {
    Edge* prevEdge = boundary->fTail;
    SkVector prevNormal;
    get_edge_normal(prevEdge, &prevNormal);
    for (Edge* e = boundary->fHead; e != nullptr;) {
        Vertex* prev = prevEdge->fWinding == 1 ? prevEdge->fTop : prevEdge->fBottom;
        Vertex* next = e->fWinding == 1 ? e->fBottom : e->fTop;
        double dist = e->dist(prev->fPoint);
        SkVector normal;
        get_edge_normal(e, &normal);
        double denom = 0.0625f * e->fLine.magSq();
        if (prevNormal.dot(normal) < 0.0 && (dist * dist) <= denom) {
            Edge* join = new_edge(prev, next, Edge::Type::kInner, c, alloc);
            insert_edge(join, e, boundary);
            remove_edge(prevEdge, boundary);
            remove_edge(e, boundary);
            if (join->fLeft && join->fRight) {
                prevEdge = join->fLeft;
                e = join;
            } else {
                prevEdge = boundary->fTail;
                e = boundary->fHead; // join->fLeft ? join->fLeft : join;
            }
            get_edge_normal(prevEdge, &prevNormal);
        } else {
            prevEdge = e;
            prevNormal = normal;
            e = e->fRight;
        }
    }
}

void fix_inversions(Vertex* prev, Vertex* next, Edge* prevBisector, Edge* nextBisector,
                    Edge* prevEdge, Comparator& c) {
    if (!prev || !next) {
        return;
    }
    int winding = c.sweep_lt(prev->fPoint, next->fPoint) ? 1 : -1;
    SkPoint p;
    uint8_t alpha;
    if (winding != prevEdge->fWinding && prevBisector->intersect(*nextBisector, &p, &alpha)) {
        prev->fPoint = next->fPoint = p;
        prev->fAlpha = next->fAlpha = alpha;
    }
}

// Stage 5d: Displace edges by half a pixel inward and outward along their normals. Intersect to
// find new vertices, and set zero alpha on the exterior and one alpha on the interior. Build a
// new antialiased mesh from those vertices.

void stroke_boundary(EdgeList* boundary, VertexList* innerMesh, VertexList* outerMesh,
                     Comparator& c, SkArenaAlloc& alloc) {
    // A boundary with fewer than 3 edges is degenerate.
    if (!boundary->fHead || !boundary->fHead->fRight || !boundary->fHead->fRight->fRight) {
        return;
    }
    Edge* prevEdge = boundary->fTail;
    float radius = 0.5f;
    double offset = radius * sqrt(prevEdge->fLine.magSq()) * prevEdge->fWinding;
    Line prevInner(prevEdge->fLine);
    prevInner.fC -= offset;
    Line prevOuter(prevEdge->fLine);
    prevOuter.fC += offset;
    VertexList innerVertices;
    VertexList outerVertices;
    Edge* prevBisector = nullptr;
    for (Edge* e = boundary->fHead; e != nullptr; e = e->fRight) {
        double offset = radius * sqrt(e->fLine.magSq()) * e->fWinding;
        Line inner(e->fLine);
        inner.fC -= offset;
        Line outer(e->fLine);
        outer.fC += offset;
        SkPoint innerPoint, outerPoint;
        if (prevInner.intersect(inner, &innerPoint) &&
            prevOuter.intersect(outer, &outerPoint)) {
            Vertex* innerVertex = alloc.make<Vertex>(innerPoint, 255);
            Vertex* outerVertex = alloc.make<Vertex>(outerPoint, 0);
            Edge* bisector = new_edge(outerVertex, innerVertex, Edge::Type::kConnector, c, alloc);
            fix_inversions(innerVertices.fTail, innerVertex, prevBisector, bisector, prevEdge, c);
            fix_inversions(outerVertices.fTail, outerVertex, prevBisector, bisector, prevEdge, c);
            innerVertex->fPartner = outerVertex;
            outerVertex->fPartner = innerVertex;
            innerVertices.append(innerVertex);
            outerVertices.append(outerVertex);
            prevBisector = bisector;
        }
        prevInner = inner;
        prevOuter = outer;
        prevEdge = e;
    }

    Vertex* innerVertex = innerVertices.fHead;
    Vertex* outerVertex = outerVertices.fHead;
    if (!innerVertex || !outerVertex) {
        return;
    }
    Edge* bisector = new_edge(outerVertices.fHead, innerVertices.fHead, Edge::Type::kConnector, c,
                              alloc);
    fix_inversions(innerVertices.fTail, innerVertices.fHead, prevBisector, bisector, prevEdge, c);
    fix_inversions(outerVertices.fTail, outerVertices.fHead, prevBisector, bisector, prevEdge, c);
    Vertex* prevInnerVertex = innerVertices.fTail;
    Vertex* prevOuterVertex = outerVertices.fTail;
    while (innerVertex && outerVertex) {
        // Connect vertices into a quad mesh. Outer edges get default (1) winding.
        // Inner edges get -2 winding. This ensures that the interior is always filled
        // (-1 winding number for normal cases, 3 for thin features where the interior inverts).
        connect(prevOuterVertex, outerVertex, Edge::Type::kOuter, c, alloc);
        connect(prevInnerVertex, innerVertex, Edge::Type::kInner, c, alloc, -2);
        prevInnerVertex = innerVertex;
        prevOuterVertex = outerVertex;
        innerVertex = innerVertex->fNext;
        outerVertex = outerVertex->fNext;
    }
    innerMesh->append(innerVertices);
    outerMesh->append(outerVertices);
}

void extract_boundary(EdgeList* boundary, Edge* e, SkPath::FillType fillType, SkArenaAlloc& alloc) {
    bool down = apply_fill_type(fillType, e->fWinding);
    while (e) {
        e->fWinding = down ? 1 : -1;
        Edge* next;
        boundary->append(e);
        if (down) {
            // Find outgoing edge, in clockwise order.
            if ((next = e->fNextEdgeAbove)) {
                down = false;
            } else if ((next = e->fBottom->fLastEdgeBelow)) {
                down = true;
            } else if ((next = e->fPrevEdgeAbove)) {
                down = false;
            }
        } else {
            // Find outgoing edge, in counter-clockwise order.
            if ((next = e->fPrevEdgeBelow)) {
                down = true;
            } else if ((next = e->fTop->fFirstEdgeAbove)) {
                down = false;
            } else if ((next = e->fNextEdgeBelow)) {
                down = true;
            }
        }
        disconnect(e);
        e = next;
    }
}

// Stage 5b: Extract boundaries from mesh, simplify and stroke them into a new mesh.

void extract_boundaries(const VertexList& inMesh, VertexList* innerVertices,
                        VertexList* outerVertices, SkPath::FillType fillType,
                        Comparator& c, SkArenaAlloc& alloc) {
    remove_non_boundary_edges(inMesh, fillType, alloc);
    for (Vertex* v = inMesh.fHead; v; v = v->fNext) {
        while (v->fFirstEdgeBelow) {
            EdgeList boundary;
            extract_boundary(&boundary, v->fFirstEdgeBelow, fillType, alloc);
            simplify_boundary(&boundary, c, alloc);
            stroke_boundary(&boundary, innerVertices, outerVertices, c, alloc);
        }
    }
}

// This is a driver function that calls stages 2-5 in turn.

void contours_to_mesh(VertexList* contours, int contourCnt, bool antialias,
                      VertexList* mesh, Comparator& c, SkArenaAlloc& alloc) {
#if LOGGING_ENABLED
    for (int i = 0; i < contourCnt; ++i) {
        Vertex* v = contours[i].fHead;
        SkASSERT(v);
        LOG("path.moveTo(%20.20g, %20.20g);\n", v->fPoint.fX, v->fPoint.fY);
        for (v = v->fNext; v; v = v->fNext) {
            LOG("path.lineTo(%20.20g, %20.20g);\n", v->fPoint.fX, v->fPoint.fY);
        }
    }
#endif
    sanitize_contours(contours, contourCnt, antialias);
    build_edges(contours, contourCnt, mesh, c, alloc);
}

void sort_mesh(VertexList* vertices, Comparator& c, SkArenaAlloc& alloc) {
    if (!vertices || !vertices->fHead) {
        return;
    }

    // Sort vertices in Y (secondarily in X).
    if (c.fDirection == Comparator::Direction::kHorizontal) {
        merge_sort<sweep_lt_horiz>(vertices);
    } else {
        merge_sort<sweep_lt_vert>(vertices);
    }
#if LOGGING_ENABLED
    for (Vertex* v = vertices->fHead; v != nullptr; v = v->fNext) {
        static float gID = 0.0f;
        v->fID = gID++;
    }
#endif
}

Poly* contours_to_polys(VertexList* contours, int contourCnt, SkPath::FillType fillType,
                        const SkRect& pathBounds, bool antialias, VertexList* outerMesh,
                        SkArenaAlloc& alloc) {
    Comparator c(pathBounds.width() > pathBounds.height() ? Comparator::Direction::kHorizontal
                                                          : Comparator::Direction::kVertical);
    VertexList mesh;
    contours_to_mesh(contours, contourCnt, antialias, &mesh, c, alloc);
    sort_mesh(&mesh, c, alloc);
    merge_coincident_vertices(&mesh, c, alloc);
    simplify(&mesh, c, alloc);
    if (antialias) {
        VertexList innerMesh;
        extract_boundaries(mesh, &innerMesh, outerMesh, fillType, c, alloc);
        sort_mesh(&innerMesh, c, alloc);
        sort_mesh(outerMesh, c, alloc);
        if (is_complex(innerMesh) || is_complex(*outerMesh)) {
            LOG("found complex mesh; taking slow path\n");
            VertexList aaMesh;
            connect_partners(outerMesh, c, alloc);
            sorted_merge(&innerMesh, outerMesh, &aaMesh, c);
            merge_coincident_vertices(&aaMesh, c, alloc);
            simplify(&aaMesh, c, alloc);
            outerMesh->fHead = outerMesh->fTail = nullptr;
            return tessellate(aaMesh, alloc);
        } else {
            LOG("no complex polygons; taking fast path\n");
            merge_coincident_vertices(&innerMesh, c, alloc);
            return tessellate(innerMesh, alloc);
        }
    } else {
        return tessellate(mesh, alloc);
    }
}

// Stage 6: Triangulate the monotone polygons into a vertex buffer.
void* polys_to_triangles(Poly* polys, SkPath::FillType fillType, const AAParams* aaParams,
                         void* data) {
    for (Poly* poly = polys; poly; poly = poly->fNext) {
        if (apply_fill_type(fillType, poly)) {
            data = poly->emit(aaParams, data);
        }
    }
    return data;
}

Poly* path_to_polys(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
                    int contourCnt, SkArenaAlloc& alloc, bool antialias, bool* isLinear,
                    VertexList* outerMesh) {
    SkPath::FillType fillType = path.getFillType();
    if (SkPath::IsInverseFillType(fillType)) {
        contourCnt++;
    }
    std::unique_ptr<VertexList[]> contours(new VertexList[contourCnt]);

    path_to_contours(path, tolerance, clipBounds, contours.get(), alloc, isLinear);
    return contours_to_polys(contours.get(), contourCnt, path.getFillType(), path.getBounds(),
                             antialias, outerMesh, alloc);
}

int get_contour_count(const SkPath& path, SkScalar tolerance) {
    int contourCnt;
    int maxPts = GrPathUtils::worstCasePointCount(path, &contourCnt, tolerance);
    if (maxPts <= 0) {
        return 0;
    }
    if (maxPts > ((int)SK_MaxU16 + 1)) {
        SkDebugf("Path not rendered, too many verts (%d)\n", maxPts);
        return 0;
    }
    return contourCnt;
}

int count_points(Poly* polys, SkPath::FillType fillType) {
    int count = 0;
    for (Poly* poly = polys; poly; poly = poly->fNext) {
        if (apply_fill_type(fillType, poly) && poly->fCount >= 3) {
            count += (poly->fCount - 2) * (TESSELLATOR_WIREFRAME ? 6 : 3);
        }
    }
    return count;
}

int count_outer_mesh_points(const VertexList& outerMesh) {
    int count = 0;
    for (Vertex* v = outerMesh.fHead; v; v = v->fNext) {
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            count += TESSELLATOR_WIREFRAME ? 12 : 6;
        }
    }
    return count;
}

void* outer_mesh_to_triangles(const VertexList& outerMesh, const AAParams* aaParams, void* data) {
    for (Vertex* v = outerMesh.fHead; v; v = v->fNext) {
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            Vertex* v0 = e->fTop;
            Vertex* v1 = e->fBottom;
            Vertex* v2 = e->fBottom->fPartner;
            Vertex* v3 = e->fTop->fPartner;
            data = emit_triangle(v0, v1, v2, aaParams, data);
            data = emit_triangle(v0, v2, v3, aaParams, data);
        }
    }
    return data;
}

} // namespace

namespace GrTessellator {

// Stage 6: Triangulate the monotone polygons into a vertex buffer.

int PathToTriangles(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
                    VertexAllocator* vertexAllocator, bool antialias, const GrColor& color,
                    bool canTweakAlphaForCoverage, bool* isLinear) {
    int contourCnt = get_contour_count(path, tolerance);
    if (contourCnt <= 0) {
        *isLinear = true;
        return 0;
    }
    SkArenaAlloc alloc(kArenaChunkSize);
    VertexList outerMesh;
    Poly* polys = path_to_polys(path, tolerance, clipBounds, contourCnt, alloc, antialias,
                                isLinear, &outerMesh);
    SkPath::FillType fillType = antialias ? SkPath::kWinding_FillType : path.getFillType();
    int count = count_points(polys, fillType);
    if (antialias) {
        count += count_outer_mesh_points(outerMesh);
    }
    if (0 == count) {
        return 0;
    }

    void* verts = vertexAllocator->lock(count);
    if (!verts) {
        SkDebugf("Could not allocate vertices\n");
        return 0;
    }

    LOG("emitting %d verts\n", count);
    AAParams aaParams;
    aaParams.fTweakAlpha = canTweakAlphaForCoverage;
    aaParams.fColor = color;

    void* end = polys_to_triangles(polys, fillType, antialias ? &aaParams : nullptr, verts);
    end = outer_mesh_to_triangles(outerMesh, &aaParams, end);
    int actualCount = static_cast<int>((static_cast<uint8_t*>(end) - static_cast<uint8_t*>(verts))
                                       / vertexAllocator->stride());
    SkASSERT(actualCount <= count);
    vertexAllocator->unlock(actualCount);
    return actualCount;
}

int PathToVertices(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
                   GrTessellator::WindingVertex** verts) {
    int contourCnt = get_contour_count(path, tolerance);
    if (contourCnt <= 0) {
        return 0;
    }
    SkArenaAlloc alloc(kArenaChunkSize);
    bool isLinear;
    Poly* polys = path_to_polys(path, tolerance, clipBounds, contourCnt, alloc, false, &isLinear,
                                nullptr);
    SkPath::FillType fillType = path.getFillType();
    int count = count_points(polys, fillType);
    if (0 == count) {
        *verts = nullptr;
        return 0;
    }

    *verts = new GrTessellator::WindingVertex[count];
    GrTessellator::WindingVertex* vertsEnd = *verts;
    SkPoint* points = new SkPoint[count];
    SkPoint* pointsEnd = points;
    for (Poly* poly = polys; poly; poly = poly->fNext) {
        if (apply_fill_type(fillType, poly)) {
            SkPoint* start = pointsEnd;
            pointsEnd = static_cast<SkPoint*>(poly->emit(nullptr, pointsEnd));
            while (start != pointsEnd) {
                vertsEnd->fPos = *start;
                vertsEnd->fWinding = poly->fWinding;
                ++start;
                ++vertsEnd;
            }
        }
    }
    int actualCount = static_cast<int>(vertsEnd - *verts);
    SkASSERT(actualCount <= count);
    SkASSERT(pointsEnd - points == actualCount);
    delete[] points;
    return actualCount;
}

} // namespace
