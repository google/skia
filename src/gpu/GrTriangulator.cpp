/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrTriangulator.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/geometry/GrPathUtils.h"

#include "src/core/SkGeometry.h"
#include "src/core/SkPointPriv.h"

#include <algorithm>
#include <cstdio>
#include <queue>
#include <unordered_map>
#include <utility>


#define LOGGING_ENABLED 0

#if LOGGING_ENABLED
#define TESS_LOG printf
#else
#define TESS_LOG(...)
#endif

constexpr static float kCosMiterAngle = 0.97f; // Corresponds to an angle of ~14 degrees.

struct Event;

using Mode = GrTriangulator::Mode;
using Vertex = GrTriangulator::Vertex;
using VertexList = GrTriangulator::VertexList;
using Edge = GrTriangulator::Edge;
using EdgeList = GrTriangulator::EdgeList;
using Poly = GrTriangulator::Poly;
using Comparator = GrTriangulator::Comparator;

template <class T, T* T::*Prev, T* T::*Next>
static void list_insert(T* t, T* prev, T* next, T** head, T** tail) {
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
static void list_remove(T* t, T** head, T** tail) {
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

struct GrTriangulator::Vertex {
  Vertex(const SkPoint& point, uint8_t alpha)
    : fPoint(point), fPrev(nullptr), fNext(nullptr)
    , fFirstEdgeAbove(nullptr), fLastEdgeAbove(nullptr)
    , fFirstEdgeBelow(nullptr), fLastEdgeBelow(nullptr)
    , fLeftEnclosingEdge(nullptr), fRightEnclosingEdge(nullptr)
    , fPartner(nullptr)
    , fAlpha(alpha)
    , fSynthetic(false)
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
    bool    fSynthetic;           // Is this a synthetic vertex?
#if LOGGING_ENABLED
    float   fID;                  // Identifier used for logging.
#endif
};

/***************************************************************************************/

typedef bool (*CompareFunc)(const SkPoint& a, const SkPoint& b);

static bool sweep_lt_horiz(const SkPoint& a, const SkPoint& b) {
    return a.fX < b.fX || (a.fX == b.fX && a.fY > b.fY);
}

static bool sweep_lt_vert(const SkPoint& a, const SkPoint& b) {
    return a.fY < b.fY || (a.fY == b.fY && a.fX < b.fX);
}

struct GrTriangulator::Comparator {
    enum class Direction { kVertical, kHorizontal };
    Comparator(Direction direction) : fDirection(direction) {}
    bool sweep_lt(const SkPoint& a, const SkPoint& b) const {
        return fDirection == Direction::kHorizontal ? sweep_lt_horiz(a, b) : sweep_lt_vert(a, b);
    }
    Direction fDirection;
};

static inline void* emit_vertex(Vertex* v, bool emitCoverage, void* data) {
    GrVertexWriter verts{data};
    verts.write(v->fPoint);

    if (emitCoverage) {
        verts.write(GrNormalizeByteToFloat(v->fAlpha));
    }

    return verts.fPtr;
}

static void* emit_triangle(Vertex* v0, Vertex* v1, Vertex* v2, bool emitCoverage, void* data) {
    TESS_LOG("emit_triangle %g (%g, %g) %d\n", v0->fID, v0->fPoint.fX, v0->fPoint.fY, v0->fAlpha);
    TESS_LOG("              %g (%g, %g) %d\n", v1->fID, v1->fPoint.fX, v1->fPoint.fY, v1->fAlpha);
    TESS_LOG("              %g (%g, %g) %d\n", v2->fID, v2->fPoint.fX, v2->fPoint.fY, v2->fAlpha);
#if TESSELLATOR_WIREFRAME
    data = emit_vertex(v0, emitCoverage, data);
    data = emit_vertex(v1, emitCoverage, data);
    data = emit_vertex(v1, emitCoverage, data);
    data = emit_vertex(v2, emitCoverage, data);
    data = emit_vertex(v2, emitCoverage, data);
    data = emit_vertex(v0, emitCoverage, data);
#else
    data = emit_vertex(v0, emitCoverage, data);
    data = emit_vertex(v1, emitCoverage, data);
    data = emit_vertex(v2, emitCoverage, data);
#endif
    return data;
}

struct GrTriangulator::VertexList {
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

static inline void round(SkPoint* p) {
    p->fX = SkScalarRoundToScalar(p->fX * SkFloatToScalar(4.0f)) * SkFloatToScalar(0.25f);
    p->fY = SkScalarRoundToScalar(p->fY * SkFloatToScalar(4.0f)) * SkFloatToScalar(0.25f);
}

static inline SkScalar double_to_clamped_scalar(double d) {
    return SkDoubleToScalar(std::min((double) SK_ScalarMax, std::max(d, (double) -SK_ScalarMax)));
}

// A line equation in implicit form. fA * x + fB * y + fC = 0, for all points (x, y) on the line.
struct Line {
    Line(double a, double b, double c) : fA(a), fB(b), fC(c) {}
    Line(Vertex* p, Vertex* q) : Line(p->fPoint, q->fPoint) {}
    Line(const SkPoint& p, const SkPoint& q)
        : fA(static_cast<double>(q.fY) - p.fY)      // a = dY
        , fB(static_cast<double>(p.fX) - q.fX)      // b = -dX
        , fC(static_cast<double>(p.fY) * q.fX -     // c = cross(q, p)
             static_cast<double>(p.fX) * q.fY) {}
    double dist(const SkPoint& p) const {
        return fA * p.fX + fB * p.fY + fC;
    }
    Line operator*(double v) const {
        return Line(fA * v, fB * v, fC * v);
    }
    double magSq() const {
        return fA * fA + fB * fB;
    }
    void normalize() {
        double len = sqrt(this->magSq());
        if (len == 0.0) {
            return;
        }
        double scale = 1.0f / len;
        fA *= scale;
        fB *= scale;
        fC *= scale;
    }
    bool nearParallel(const Line& o) const {
        return fabs(o.fA - fA) < 0.00001 && fabs(o.fB - fB) < 0.00001;
    }

    // Compute the intersection of two (infinite) Lines.
    bool intersect(const Line& other, SkPoint* point) const {
        double denom = fA * other.fB - fB * other.fA;
        if (denom == 0.0) {
            return false;
        }
        double scale = 1.0 / denom;
        point->fX = double_to_clamped_scalar((fB * other.fC - other.fB * fC) * scale);
        point->fY = double_to_clamped_scalar((other.fA * fC - fA * other.fC) * scale);
        round(point);
        return point->isFinite();
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

struct GrTriangulator::Edge {
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
    bool intersect(const Edge& other, SkPoint* p, uint8_t* alpha = nullptr) const {
        TESS_LOG("intersecting %g -> %g with %g -> %g\n",
                 fTop->fID, fBottom->fID, other.fTop->fID, other.fBottom->fID);
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

struct SSEdge;

struct SSVertex {
    SSVertex(Vertex* v) : fVertex(v), fPrev(nullptr), fNext(nullptr) {}
    Vertex* fVertex;
    SSEdge* fPrev;
    SSEdge* fNext;
};

struct SSEdge {
    SSEdge(Edge* edge, SSVertex* prev, SSVertex* next)
      : fEdge(edge), fEvent(nullptr), fPrev(prev), fNext(next) {
    }
    Edge*     fEdge;
    Event*    fEvent;
    SSVertex* fPrev;
    SSVertex* fNext;
};

typedef std::unordered_map<Vertex*, SSVertex*> SSVertexMap;
typedef std::vector<SSEdge*> SSEdgeList;

struct GrTriangulator::EdgeList {
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

struct EventList;

struct Event {
    Event(SSEdge* edge, const SkPoint& point, uint8_t alpha)
      : fEdge(edge), fPoint(point), fAlpha(alpha) {
    }
    SSEdge* fEdge;
    SkPoint fPoint;
    uint8_t fAlpha;
    void apply(VertexList* mesh, Comparator& c, EventList* events, SkArenaAlloc& alloc);
};

struct EventComparator {
    enum class Op { kLessThan, kGreaterThan };
    EventComparator(Op op) : fOp(op) {}
    bool operator() (Event* const &e1, Event* const &e2) {
        return fOp == Op::kLessThan ? e1->fAlpha < e2->fAlpha
                                    : e1->fAlpha > e2->fAlpha;
    }
    Op fOp;
};

typedef  std::priority_queue<Event*, std::vector<Event*>, EventComparator> EventPQ;

struct EventList : EventPQ {
    EventList(EventComparator comparison) : EventPQ(comparison) {
    }
};

static void create_event(SSEdge* e, EventList* events, SkArenaAlloc& alloc) {
    Vertex* prev = e->fPrev->fVertex;
    Vertex* next = e->fNext->fVertex;
    if (prev == next || !prev->fPartner || !next->fPartner) {
        return;
    }
    Edge bisector1(prev, prev->fPartner, 1, Edge::Type::kConnector);
    Edge bisector2(next, next->fPartner, 1, Edge::Type::kConnector);
    SkPoint p;
    uint8_t alpha;
    if (bisector1.intersect(bisector2, &p, &alpha)) {
        TESS_LOG("found edge event for %g, %g (original %g -> %g), "
                 "will collapse to %g,%g alpha %d\n",
                  prev->fID, next->fID, e->fEdge->fTop->fID, e->fEdge->fBottom->fID, p.fX, p.fY,
                  alpha);
        e->fEvent = alloc.make<Event>(e, p, alpha);
        events->push(e->fEvent);
    }
}

static void create_event(SSEdge* edge, Vertex* v, SSEdge* other, Vertex* dest, EventList* events,
                         Comparator& c, SkArenaAlloc& alloc) {
    if (!v->fPartner) {
        return;
    }
    Vertex* top = edge->fEdge->fTop;
    Vertex* bottom = edge->fEdge->fBottom;
    if (!top || !bottom ) {
        return;
    }
    Line line = edge->fEdge->fLine;
    line.fC = -(dest->fPoint.fX * line.fA  + dest->fPoint.fY * line.fB);
    Edge bisector(v, v->fPartner, 1, Edge::Type::kConnector);
    SkPoint p;
    uint8_t alpha = dest->fAlpha;
    if (line.intersect(bisector.fLine, &p) && !c.sweep_lt(p, top->fPoint) &&
                                               c.sweep_lt(p, bottom->fPoint)) {
        TESS_LOG("found p edge event for %g, %g (original %g -> %g), "
                 "will collapse to %g,%g alpha %d\n",
                 dest->fID, v->fID, top->fID, bottom->fID, p.fX, p.fY, alpha);
        edge->fEvent = alloc.make<Event>(edge, p, alpha);
        events->push(edge->fEvent);
    }
}

/***************************************************************************************/

struct GrTriangulator::Poly {
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
        TESS_LOG("*** created Poly %d\n", fID);
#endif
    }
    typedef enum { kLeft_Side, kRight_Side } Side;
    struct MonotonePoly {
        MonotonePoly(Edge* edge, Side side, int winding)
            : fSide(side)
            , fFirstEdge(nullptr)
            , fLastEdge(nullptr)
            , fPrev(nullptr)
            , fNext(nullptr)
            , fWinding(winding) {
            this->addEdge(edge);
        }
        Side          fSide;
        Edge*         fFirstEdge;
        Edge*         fLastEdge;
        MonotonePoly* fPrev;
        MonotonePoly* fNext;
        int fWinding;
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

        void* emit(bool emitCoverage, void* data) {
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
                    return this->emitTriangle(prev, curr, next, emitCoverage, data);
                }
                double ax = static_cast<double>(curr->fPoint.fX) - prev->fPoint.fX;
                double ay = static_cast<double>(curr->fPoint.fY) - prev->fPoint.fY;
                double bx = static_cast<double>(next->fPoint.fX) - curr->fPoint.fX;
                double by = static_cast<double>(next->fPoint.fY) - curr->fPoint.fY;
                if (ax * by - ay * bx >= 0.0) {
                    data = this->emitTriangle(prev, curr, next, emitCoverage, data);
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
        void* emitTriangle(Vertex* prev, Vertex* curr, Vertex* next, bool emitCoverage,
                           void* data) const {
            if (fWinding < 0) {
                // Ensure our triangles always wind in the same direction as if the path had been
                // triangulated as a simple fan (a la red book).
                std::swap(prev, next);
            }
            return emit_triangle(next, curr, prev, emitCoverage, data);
        }
    };
    Poly* addEdge(Edge* e, Side side, SkArenaAlloc& alloc) {
        TESS_LOG("addEdge (%g -> %g) to poly %d, %s side\n",
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
            fHead = fTail = alloc.make<MonotonePoly>(e, side, fWinding);
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
                MonotonePoly* m = alloc.make<MonotonePoly>(e, side, fWinding);
                m->fPrev = fTail;
                fTail->fNext = m;
                fTail = m;
            }
        }
        return poly;
    }
    void* emit(bool emitCoverage, void *data) {
        if (fCount < 3) {
            return data;
        }
        TESS_LOG("emit() %d, size %d\n", fID, fCount);
        for (MonotonePoly* m = fHead; m != nullptr; m = m->fNext) {
            data = m->emit(emitCoverage, data);
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

static bool coincident(const SkPoint& a, const SkPoint& b) {
    return a == b;
}

static Poly* new_poly(Poly** head, Vertex* v, int winding, SkArenaAlloc& alloc) {
    Poly* poly = alloc.make<Poly>(v, winding);
    poly->fNext = *head;
    *head = poly;
    return poly;
}

void GrTriangulator::appendPointToContour(const SkPoint& p, VertexList* contour) {
    Vertex* v = fAlloc.make<Vertex>(p, 255);
#if LOGGING_ENABLED
    static float gID = 0.0f;
    v->fID = gID++;
#endif
    contour->append(v);
}

static SkScalar quad_error_at(const SkPoint pts[3], SkScalar t, SkScalar u) {
    SkQuadCoeff quad(pts);
    SkPoint p0 = to_point(quad.eval(t - 0.5f * u));
    SkPoint mid = to_point(quad.eval(t));
    SkPoint p1 = to_point(quad.eval(t + 0.5f * u));
    if (!p0.isFinite() || !mid.isFinite() || !p1.isFinite()) {
        return 0;
    }
    return SkPointPriv::DistanceToLineSegmentBetweenSqd(mid, p0, p1);
}

void GrTriangulator::appendQuadraticToContour(const SkPoint pts[3], SkScalar toleranceSqd,
                                              VertexList* contour) {
    SkQuadCoeff quad(pts);
    Sk2s aa = quad.fA * quad.fA;
    SkScalar denom = 2.0f * (aa[0] + aa[1]);
    Sk2s ab = quad.fA * quad.fB;
    SkScalar t = denom ? (-ab[0] - ab[1]) / denom : 0.0f;
    int nPoints = 1;
    SkScalar u = 1.0f;
    // Test possible subdivision values only at the point of maximum curvature.
    // If it passes the flatness metric there, it'll pass everywhere.
    while (nPoints < GrPathUtils::kMaxPointsPerCurve) {
        u = 1.0f / nPoints;
        if (quad_error_at(pts, t, u) < toleranceSqd) {
            break;
        }
        nPoints++;
    }
    for (int j = 1; j <= nPoints; j++) {
        this->appendPointToContour(to_point(quad.eval(j * u)), contour);
    }
}

void GrTriangulator::generateCubicPoints(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                                         const SkPoint& p3, SkScalar tolSqd, VertexList* contour,
                                         int pointsLeft) {
    SkScalar d1 = SkPointPriv::DistanceToLineSegmentBetweenSqd(p1, p0, p3);
    SkScalar d2 = SkPointPriv::DistanceToLineSegmentBetweenSqd(p2, p0, p3);
    if (pointsLeft < 2 || (d1 < tolSqd && d2 < tolSqd) ||
        !SkScalarIsFinite(d1) || !SkScalarIsFinite(d2)) {
        this->appendPointToContour(p3, contour);
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
    this->generateCubicPoints(p0, q[0], r[0], s, tolSqd, contour, pointsLeft);
    this->generateCubicPoints(s, r[1], q[2], p3, tolSqd, contour, pointsLeft);
}

// Stage 1: convert the input path to a set of linear contours (linked list of Vertices).

void GrTriangulator::pathToContours(float tolerance, const SkRect& clipBounds,
                                    VertexList* contours) {
    SkScalar toleranceSqd = tolerance * tolerance;
    bool innerPolygons = (Mode::kSimpleInnerPolygons == fMode);

    SkPoint pts[4];
    fIsLinear = true;
    VertexList* contour = contours;
    SkPath::Iter iter(fPath, false);
    if (fPath.isInverseFillType()) {
        SkPoint quad[4];
        clipBounds.toQuad(quad);
        for (int i = 3; i >= 0; i--) {
            this->appendPointToContour(quad[i], contours);
        }
        contour++;
    }
    SkAutoConicToQuads converter;
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kConic_Verb: {
                fIsLinear = false;
                if (innerPolygons) {
                    this->appendPointToContour(pts[2], contour);
                    break;
                }
                SkScalar weight = iter.conicWeight();
                const SkPoint* quadPts = converter.computeQuads(pts, weight, toleranceSqd);
                for (int i = 0; i < converter.countQuads(); ++i) {
                    this->appendQuadraticToContour(quadPts, toleranceSqd, contour);
                    quadPts += 2;
                }
                break;
            }
            case SkPath::kMove_Verb:
                if (contour->fHead) {
                    contour++;
                }
                this->appendPointToContour(pts[0], contour);
                break;
            case SkPath::kLine_Verb: {
                this->appendPointToContour(pts[1], contour);
                break;
            }
            case SkPath::kQuad_Verb: {
                fIsLinear = false;
                if (innerPolygons) {
                    this->appendPointToContour(pts[2], contour);
                    break;
                }
                this->appendQuadraticToContour(pts, toleranceSqd, contour);
                break;
            }
            case SkPath::kCubic_Verb: {
                fIsLinear = false;
                if (innerPolygons) {
                    this->appendPointToContour(pts[3], contour);
                    break;
                }
                int pointsLeft = GrPathUtils::cubicPointCount(pts, tolerance);
                this->generateCubicPoints(pts[0], pts[1], pts[2], pts[3], toleranceSqd, contour,
                                          pointsLeft);
                break;
            }
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
                break;
        }
    }
}

static inline bool apply_fill_type(SkPathFillType fillType, int winding) {
    switch (fillType) {
        case SkPathFillType::kWinding:
            return winding != 0;
        case SkPathFillType::kEvenOdd:
            return (winding & 1) != 0;
        case SkPathFillType::kInverseWinding:
            return winding == 1;
        case SkPathFillType::kInverseEvenOdd:
            return (winding & 1) == 1;
        default:
            SkASSERT(false);
            return false;
    }
}

static inline bool apply_fill_type(SkPathFillType fillType, Poly* poly) {
    return poly && apply_fill_type(fillType, poly->fWinding);
}

static Edge* new_edge(Vertex* prev, Vertex* next, Edge::Type type, Comparator& c,
                      SkArenaAlloc& alloc) {
    int winding = c.sweep_lt(prev->fPoint, next->fPoint) ? 1 : -1;
    Vertex* top = winding < 0 ? next : prev;
    Vertex* bottom = winding < 0 ? prev : next;
    return alloc.make<Edge>(top, bottom, winding, type);
}

static void remove_edge(Edge* edge, EdgeList* edges) {
    TESS_LOG("removing edge %g -> %g\n", edge->fTop->fID, edge->fBottom->fID);
    SkASSERT(edges->contains(edge));
    edges->remove(edge);
}

static void insert_edge(Edge* edge, Edge* prev, EdgeList* edges) {
    TESS_LOG("inserting edge %g -> %g\n", edge->fTop->fID, edge->fBottom->fID);
    SkASSERT(!edges->contains(edge));
    Edge* next = prev ? prev->fRight : edges->fHead;
    edges->insert(edge, prev, next);
}

static void find_enclosing_edges(Vertex* v, EdgeList* edges, Edge** left, Edge** right) {
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

static void insert_edge_above(Edge* edge, Vertex* v, Comparator& c) {
    if (edge->fTop->fPoint == edge->fBottom->fPoint ||
        c.sweep_lt(edge->fBottom->fPoint, edge->fTop->fPoint)) {
        return;
    }
    TESS_LOG("insert edge (%g -> %g) above vertex %g\n",
             edge->fTop->fID, edge->fBottom->fID, v->fID);
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

static void insert_edge_below(Edge* edge, Vertex* v, Comparator& c) {
    if (edge->fTop->fPoint == edge->fBottom->fPoint ||
        c.sweep_lt(edge->fBottom->fPoint, edge->fTop->fPoint)) {
        return;
    }
    TESS_LOG("insert edge (%g -> %g) below vertex %g\n",
             edge->fTop->fID, edge->fBottom->fID, v->fID);
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

static void remove_edge_above(Edge* edge) {
    SkASSERT(edge->fTop && edge->fBottom);
    TESS_LOG("removing edge (%g -> %g) above vertex %g\n", edge->fTop->fID, edge->fBottom->fID,
             edge->fBottom->fID);
    list_remove<Edge, &Edge::fPrevEdgeAbove, &Edge::fNextEdgeAbove>(
        edge, &edge->fBottom->fFirstEdgeAbove, &edge->fBottom->fLastEdgeAbove);
}

static void remove_edge_below(Edge* edge) {
    SkASSERT(edge->fTop && edge->fBottom);
    TESS_LOG("removing edge (%g -> %g) below vertex %g\n",
             edge->fTop->fID, edge->fBottom->fID, edge->fTop->fID);
    list_remove<Edge, &Edge::fPrevEdgeBelow, &Edge::fNextEdgeBelow>(
        edge, &edge->fTop->fFirstEdgeBelow, &edge->fTop->fLastEdgeBelow);
}

static void disconnect(Edge* edge)
{
    remove_edge_above(edge);
    remove_edge_below(edge);
}

static void merge_collinear_edges(Edge* edge, EdgeList* activeEdges, Vertex** current,
                                  Comparator& c);

static void rewind(EdgeList* activeEdges, Vertex** current, Vertex* dst, Comparator& c) {
    if (!current || *current == dst || c.sweep_lt((*current)->fPoint, dst->fPoint)) {
        return;
    }
    Vertex* v = *current;
    TESS_LOG("rewinding active edges from vertex %g to vertex %g\n", v->fID, dst->fID);
    while (v != dst) {
        v = v->fPrev;
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            remove_edge(e, activeEdges);
        }
        Edge* leftEdge = v->fLeftEnclosingEdge;
        for (Edge* e = v->fFirstEdgeAbove; e; e = e->fNextEdgeAbove) {
            insert_edge(e, leftEdge, activeEdges);
            leftEdge = e;
            Vertex* top = e->fTop;
            if (c.sweep_lt(top->fPoint, dst->fPoint) &&
                ((top->fLeftEnclosingEdge && !top->fLeftEnclosingEdge->isLeftOf(e->fTop)) ||
                 (top->fRightEnclosingEdge && !top->fRightEnclosingEdge->isRightOf(e->fTop)))) {
                dst = top;
            }
        }
    }
    *current = v;
}

static void rewind_if_necessary(Edge* edge, EdgeList* activeEdges, Vertex** current,
                                Comparator& c) {
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

static void set_top(Edge* edge, Vertex* v, EdgeList* activeEdges, Vertex** current, Comparator& c) {
    remove_edge_below(edge);
    edge->fTop = v;
    edge->recompute();
    insert_edge_below(edge, v, c);
    rewind_if_necessary(edge, activeEdges, current, c);
    merge_collinear_edges(edge, activeEdges, current, c);
}

static void set_bottom(Edge* edge, Vertex* v, EdgeList* activeEdges, Vertex** current,
                       Comparator& c) {
    remove_edge_above(edge);
    edge->fBottom = v;
    edge->recompute();
    insert_edge_above(edge, v, c);
    rewind_if_necessary(edge, activeEdges, current, c);
    merge_collinear_edges(edge, activeEdges, current, c);
}

static void merge_edges_above(Edge* edge, Edge* other, EdgeList* activeEdges, Vertex** current,
                              Comparator& c) {
    if (coincident(edge->fTop->fPoint, other->fTop->fPoint)) {
        TESS_LOG("merging coincident above edges (%g, %g) -> (%g, %g)\n",
                 edge->fTop->fPoint.fX, edge->fTop->fPoint.fY,
                 edge->fBottom->fPoint.fX, edge->fBottom->fPoint.fY);
        rewind(activeEdges, current, edge->fTop, c);
        other->fWinding += edge->fWinding;
        disconnect(edge);
        edge->fTop = edge->fBottom = nullptr;
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

static void merge_edges_below(Edge* edge, Edge* other, EdgeList* activeEdges, Vertex** current,
                              Comparator& c) {
    if (coincident(edge->fBottom->fPoint, other->fBottom->fPoint)) {
        TESS_LOG("merging coincident below edges (%g, %g) -> (%g, %g)\n",
                 edge->fTop->fPoint.fX, edge->fTop->fPoint.fY,
                 edge->fBottom->fPoint.fX, edge->fBottom->fPoint.fY);
        rewind(activeEdges, current, edge->fTop, c);
        other->fWinding += edge->fWinding;
        disconnect(edge);
        edge->fTop = edge->fBottom = nullptr;
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

static bool top_collinear(Edge* left, Edge* right) {
    if (!left || !right) {
        return false;
    }
    return left->fTop->fPoint == right->fTop->fPoint ||
           !left->isLeftOf(right->fTop) || !right->isRightOf(left->fTop);
}

static bool bottom_collinear(Edge* left, Edge* right) {
    if (!left || !right) {
        return false;
    }
    return left->fBottom->fPoint == right->fBottom->fPoint ||
           !left->isLeftOf(right->fBottom) || !right->isRightOf(left->fBottom);
}

static void merge_collinear_edges(Edge* edge, EdgeList* activeEdges, Vertex** current,
                                  Comparator& c) {
    for (;;) {
        if (top_collinear(edge->fPrevEdgeAbove, edge)) {
            merge_edges_above(edge->fPrevEdgeAbove, edge, activeEdges, current, c);
        } else if (top_collinear(edge, edge->fNextEdgeAbove)) {
            merge_edges_above(edge->fNextEdgeAbove, edge, activeEdges, current, c);
        } else if (bottom_collinear(edge->fPrevEdgeBelow, edge)) {
            merge_edges_below(edge->fPrevEdgeBelow, edge, activeEdges, current, c);
        } else if (bottom_collinear(edge, edge->fNextEdgeBelow)) {
            merge_edges_below(edge->fNextEdgeBelow, edge, activeEdges, current, c);
        } else {
            break;
        }
    }
    SkASSERT(!top_collinear(edge->fPrevEdgeAbove, edge));
    SkASSERT(!top_collinear(edge, edge->fNextEdgeAbove));
    SkASSERT(!bottom_collinear(edge->fPrevEdgeBelow, edge));
    SkASSERT(!bottom_collinear(edge, edge->fNextEdgeBelow));
}

bool GrTriangulator::splitEdge(Edge* edge, Vertex* v, EdgeList* activeEdges, Vertex** current,
                               Comparator& c) {
    if (!edge->fTop || !edge->fBottom || v == edge->fTop || v == edge->fBottom) {
        return false;
    }
    TESS_LOG("splitting edge (%g -> %g) at vertex %g (%g, %g)\n",
             edge->fTop->fID, edge->fBottom->fID, v->fID, v->fPoint.fX, v->fPoint.fY);
    Vertex* top;
    Vertex* bottom;
    int winding = edge->fWinding;
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
    Edge* newEdge = fAlloc.make<Edge>(top, bottom, winding, edge->fType);
    insert_edge_below(newEdge, top, c);
    insert_edge_above(newEdge, bottom, c);
    merge_collinear_edges(newEdge, activeEdges, current, c);
    return true;
}

bool GrTriangulator::intersectEdgePair(Edge* left, Edge* right, EdgeList* activeEdges,
                                       Vertex** current, Comparator& c) {
    if (!left->fTop || !left->fBottom || !right->fTop || !right->fBottom) {
        return false;
    }
    if (left->fTop == right->fTop || left->fBottom == right->fBottom) {
        return false;
    }
    if (c.sweep_lt(left->fTop->fPoint, right->fTop->fPoint)) {
        if (!left->isLeftOf(right->fTop)) {
            rewind(activeEdges, current, right->fTop, c);
            return this->splitEdge(left, right->fTop, activeEdges, current, c);
        }
    } else {
        if (!right->isRightOf(left->fTop)) {
            rewind(activeEdges, current, left->fTop, c);
            return this->splitEdge(right, left->fTop, activeEdges, current, c);
        }
    }
    if (c.sweep_lt(right->fBottom->fPoint, left->fBottom->fPoint)) {
        if (!left->isLeftOf(right->fBottom)) {
            rewind(activeEdges, current, right->fBottom, c);
            return this->splitEdge(left, right->fBottom, activeEdges, current, c);
        }
    } else {
        if (!right->isRightOf(left->fBottom)) {
            rewind(activeEdges, current, left->fBottom, c);
            return this->splitEdge(right, left->fBottom, activeEdges, current, c);
        }
    }
    return false;
}

static Edge* connect(Vertex* prev, Vertex* next, Edge::Type type, Comparator& c,
                     SkArenaAlloc& alloc, int winding_scale = 1) {
    if (!prev || !next || prev->fPoint == next->fPoint) {
        return nullptr;
    }
    Edge* edge = new_edge(prev, next, type, c, alloc);
    insert_edge_below(edge, edge->fTop, c);
    insert_edge_above(edge, edge->fBottom, c);
    edge->fWinding *= winding_scale;
    merge_collinear_edges(edge, nullptr, nullptr, c);
    return edge;
}

static void merge_vertices(Vertex* src, Vertex* dst, VertexList* mesh, Comparator& c) {
    TESS_LOG("found coincident verts at %g, %g; merging %g into %g\n",
             src->fPoint.fX, src->fPoint.fY, src->fID, dst->fID);
    dst->fAlpha = std::max(src->fAlpha, dst->fAlpha);
    if (src->fPartner) {
        src->fPartner->fPartner = dst;
    }
    while (Edge* edge = src->fFirstEdgeAbove) {
        set_bottom(edge, dst, nullptr, nullptr, c);
    }
    while (Edge* edge = src->fFirstEdgeBelow) {
        set_top(edge, dst, nullptr, nullptr, c);
    }
    mesh->remove(src);
    dst->fSynthetic = true;
}

static Vertex* create_sorted_vertex(const SkPoint& p, uint8_t alpha, VertexList* mesh,
                                    Vertex* reference, Comparator& c, SkArenaAlloc& alloc) {
    Vertex* prevV = reference;
    while (prevV && c.sweep_lt(p, prevV->fPoint)) {
        prevV = prevV->fPrev;
    }
    Vertex* nextV = prevV ? prevV->fNext : mesh->fHead;
    while (nextV && c.sweep_lt(nextV->fPoint, p)) {
        prevV = nextV;
        nextV = nextV->fNext;
    }
    Vertex* v;
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
    return v;
}

// If an edge's top and bottom points differ only by 1/2 machine epsilon in the primary
// sort criterion, it may not be possible to split correctly, since there is no point which is
// below the top and above the bottom. This function detects that case.
static bool nearly_flat(Comparator& c, Edge* edge) {
    SkPoint diff = edge->fBottom->fPoint - edge->fTop->fPoint;
    float primaryDiff = c.fDirection == Comparator::Direction::kHorizontal ? diff.fX : diff.fY;
    return fabs(primaryDiff) < std::numeric_limits<float>::epsilon() && primaryDiff != 0.0f;
}

static SkPoint clamp(SkPoint p, SkPoint min, SkPoint max, Comparator& c) {
    if (c.sweep_lt(p, min)) {
        return min;
    } else if (c.sweep_lt(max, p)) {
        return max;
    } else {
        return p;
    }
}

static void compute_bisector(Edge* edge1, Edge* edge2, Vertex* v, SkArenaAlloc& alloc) {
    Line line1 = edge1->fLine;
    Line line2 = edge2->fLine;
    line1.normalize();
    line2.normalize();
    double cosAngle = line1.fA * line2.fA + line1.fB * line2.fB;
    if (cosAngle > 0.999) {
        return;
    }
    line1.fC += edge1->fWinding > 0 ? -1 : 1;
    line2.fC += edge2->fWinding > 0 ? -1 : 1;
    SkPoint p;
    if (line1.intersect(line2, &p)) {
        uint8_t alpha = edge1->fType == Edge::Type::kOuter ? 255 : 0;
        v->fPartner = alloc.make<Vertex>(p, alpha);
        TESS_LOG("computed bisector (%g,%g) alpha %d for vertex %g\n", p.fX, p.fY, alpha, v->fID);
    }
}

bool GrTriangulator::checkForIntersection(Edge* left, Edge* right, EdgeList* activeEdges,
                                          Vertex** current, VertexList* mesh, Comparator& c) {
    if (!left || !right) {
        return false;
    }
    SkPoint p;
    uint8_t alpha;
    if (left->intersect(*right, &p, &alpha) && p.isFinite()) {
        Vertex* v;
        TESS_LOG("found intersection, pt is %g, %g\n", p.fX, p.fY);
        Vertex* top = *current;
        // If the intersection point is above the current vertex, rewind to the vertex above the
        // intersection.
        while (top && c.sweep_lt(p, top->fPoint)) {
            top = top->fPrev;
        }
        if (!nearly_flat(c, left)) {
            p = clamp(p, left->fTop->fPoint, left->fBottom->fPoint, c);
        }
        if (!nearly_flat(c, right)) {
            p = clamp(p, right->fTop->fPoint, right->fBottom->fPoint, c);
        }
        if (p == left->fTop->fPoint) {
            v = left->fTop;
        } else if (p == left->fBottom->fPoint) {
            v = left->fBottom;
        } else if (p == right->fTop->fPoint) {
            v = right->fTop;
        } else if (p == right->fBottom->fPoint) {
            v = right->fBottom;
        } else {
            v = create_sorted_vertex(p, alpha, mesh, top, c, fAlloc);
            if (left->fTop->fPartner) {
                v->fSynthetic = true;
                compute_bisector(left, right, v, fAlloc);
            }
        }
        rewind(activeEdges, current, top ? top : v, c);
        this->splitEdge(left, v, activeEdges, current, c);
        this->splitEdge(right, v, activeEdges, current, c);
        v->fAlpha = std::max(v->fAlpha, alpha);
        return true;
    }
    return this->intersectEdgePair(left, right, activeEdges, current, c);
}

void GrTriangulator::sanitizeContours(VertexList* contours, int contourCnt) {
    bool approximate = (Mode::kEdgeAntialias == fMode);
    bool removeCollinearVertices = (Mode::kSimpleInnerPolygons != fMode);
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
            Vertex* nextWrap = next ? next : contour->fHead;
            if (coincident(prev->fPoint, v->fPoint)) {
                TESS_LOG("vertex %g,%g coincident; removing\n", v->fPoint.fX, v->fPoint.fY);
                contour->remove(v);
            } else if (!v->fPoint.isFinite()) {
                TESS_LOG("vertex %g,%g non-finite; removing\n", v->fPoint.fX, v->fPoint.fY);
                contour->remove(v);
            } else if (removeCollinearVertices &&
                       Line(prev->fPoint, nextWrap->fPoint).dist(v->fPoint) == 0.0) {
                TESS_LOG("vertex %g,%g collinear; removing\n", v->fPoint.fX, v->fPoint.fY);
                contour->remove(v);
            } else {
                prev = v;
            }
            v = next;
        }
    }
}

bool GrTriangulator::mergeCoincidentVertices(VertexList* mesh, Comparator& c) {
    if (!mesh->fHead) {
        return false;
    }
    bool merged = false;
    for (Vertex* v = mesh->fHead->fNext; v;) {
        Vertex* next = v->fNext;
        if (c.sweep_lt(v->fPoint, v->fPrev->fPoint)) {
            v->fPoint = v->fPrev->fPoint;
        }
        if (coincident(v->fPrev->fPoint, v->fPoint)) {
            merge_vertices(v, v->fPrev, mesh, c);
            merged = true;
        }
        v = next;
    }
    return merged;
}

// Stage 2: convert the contours to a mesh of edges connecting the vertices.

void GrTriangulator::buildEdges(VertexList* contours, int contourCnt, VertexList* mesh,
                                Comparator& c) {
    for (VertexList* contour = contours; contourCnt > 0; --contourCnt, ++contour) {
        Vertex* prev = contour->fTail;
        for (Vertex* v = contour->fHead; v;) {
            Vertex* next = v->fNext;
            connect(prev, v, Edge::Type::kInner, c, fAlloc);
            mesh->append(v);
            prev = v;
            v = next;
        }
    }
}

static void connect_partners(VertexList* mesh, Comparator& c, SkArenaAlloc& alloc) {
    for (Vertex* outer = mesh->fHead; outer; outer = outer->fNext) {
        if (Vertex* inner = outer->fPartner) {
            if ((inner->fPrev || inner->fNext) && (outer->fPrev || outer->fNext)) {
                // Connector edges get zero winding, since they're only structural (i.e., to ensure
                // no 0-0-0 alpha triangles are produced), and shouldn't affect the poly winding
                // number.
                connect(outer, inner, Edge::Type::kConnector, c, alloc, 0);
                inner->fPartner = outer->fPartner = nullptr;
            }
        }
    }
}

template <CompareFunc sweep_lt>
static void sorted_merge(VertexList* front, VertexList* back, VertexList* result) {
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

static void sorted_merge(VertexList* front, VertexList* back, VertexList* result, Comparator& c) {
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
static void merge_sort(VertexList* vertices) {
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

static void dump_mesh(const VertexList& mesh) {
#if LOGGING_ENABLED
    for (Vertex* v = mesh.fHead; v; v = v->fNext) {
        TESS_LOG("vertex %g (%g, %g) alpha %d", v->fID, v->fPoint.fX, v->fPoint.fY, v->fAlpha);
        if (Vertex* p = v->fPartner) {
            TESS_LOG(", partner %g (%g, %g) alpha %d\n",
                    p->fID, p->fPoint.fX, p->fPoint.fY, p->fAlpha);
        } else {
            TESS_LOG(", null partner\n");
        }
        for (Edge* e = v->fFirstEdgeAbove; e; e = e->fNextEdgeAbove) {
            TESS_LOG("  edge %g -> %g, winding %d\n", e->fTop->fID, e->fBottom->fID, e->fWinding);
        }
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            TESS_LOG("  edge %g -> %g, winding %d\n", e->fTop->fID, e->fBottom->fID, e->fWinding);
        }
    }
#endif
}

static void dump_skel(const SSEdgeList& ssEdges) {
#if LOGGING_ENABLED
    for (SSEdge* edge : ssEdges) {
        if (edge->fEdge) {
            TESS_LOG("skel edge %g -> %g",
                edge->fPrev->fVertex->fID,
                edge->fNext->fVertex->fID);
            if (edge->fEdge->fTop && edge->fEdge->fBottom) {
                TESS_LOG(" (original %g -> %g)\n",
                         edge->fEdge->fTop->fID,
                         edge->fEdge->fBottom->fID);
            } else {
                TESS_LOG("\n");
            }
        }
    }
#endif
}

#ifdef SK_DEBUG
static void validate_edge_pair(Edge* left, Edge* right, Comparator& c) {
    if (!left || !right) {
        return;
    }
    if (left->fTop == right->fTop) {
        SkASSERT(left->isLeftOf(right->fBottom));
        SkASSERT(right->isRightOf(left->fBottom));
    } else if (c.sweep_lt(left->fTop->fPoint, right->fTop->fPoint)) {
        SkASSERT(left->isLeftOf(right->fTop));
    } else {
        SkASSERT(right->isRightOf(left->fTop));
    }
    if (left->fBottom == right->fBottom) {
        SkASSERT(left->isLeftOf(right->fTop));
        SkASSERT(right->isRightOf(left->fTop));
    } else if (c.sweep_lt(right->fBottom->fPoint, left->fBottom->fPoint)) {
        SkASSERT(left->isLeftOf(right->fBottom));
    } else {
        SkASSERT(right->isRightOf(left->fBottom));
    }
}

static void validate_edge_list(EdgeList* edges, Comparator& c) {
    Edge* left = edges->fHead;
    if (!left) {
        return;
    }
    for (Edge* right = left->fRight; right; right = right->fRight) {
        validate_edge_pair(left, right, c);
        left = right;
    }
}
#endif

// Stage 4: Simplify the mesh by inserting new vertices at intersecting edges.

static bool connected(Vertex* v) {
    return v->fFirstEdgeAbove || v->fFirstEdgeBelow;
}

GrTriangulator::SimplifyResult GrTriangulator::simplify(VertexList* mesh, Comparator& c) {
    TESS_LOG("simplifying complex polygons\n");
    EdgeList activeEdges;
    auto result = SimplifyResult::kAlreadySimple;
    for (Vertex* v = mesh->fHead; v != nullptr; v = v->fNext) {
        if (!connected(v)) {
            continue;
        }
        Edge* leftEnclosingEdge;
        Edge* rightEnclosingEdge;
        bool restartChecks;
        do {
            TESS_LOG("\nvertex %g: (%g,%g), alpha %d\n",
                     v->fID, v->fPoint.fX, v->fPoint.fY, v->fAlpha);
            restartChecks = false;
            find_enclosing_edges(v, &activeEdges, &leftEnclosingEdge, &rightEnclosingEdge);
            v->fLeftEnclosingEdge = leftEnclosingEdge;
            v->fRightEnclosingEdge = rightEnclosingEdge;
            if (v->fFirstEdgeBelow) {
                for (Edge* edge = v->fFirstEdgeBelow; edge; edge = edge->fNextEdgeBelow) {
                    if (this->checkForIntersection(
                            leftEnclosingEdge, edge, &activeEdges, &v, mesh, c) ||
                        this->checkForIntersection(
                            edge, rightEnclosingEdge, &activeEdges, &v, mesh, c)) {
                        if (Mode::kSimpleInnerPolygons == fMode) {
                            return SimplifyResult::kAbort;
                        }
                        result = SimplifyResult::kFoundSelfIntersection;
                        restartChecks = true;
                        break;
                    }
                }
            } else {
                if (this->checkForIntersection(leftEnclosingEdge, rightEnclosingEdge, &activeEdges,
                                               &v, mesh, c)) {
                    if (Mode::kSimpleInnerPolygons == fMode) {
                        return SimplifyResult::kAbort;
                    }
                    result = SimplifyResult::kFoundSelfIntersection;
                    restartChecks = true;
                }

            }
        } while (restartChecks);
#ifdef SK_DEBUG
        validate_edge_list(&activeEdges, c);
#endif
        for (Edge* e = v->fFirstEdgeAbove; e; e = e->fNextEdgeAbove) {
            remove_edge(e, &activeEdges);
        }
        Edge* leftEdge = leftEnclosingEdge;
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            insert_edge(e, leftEdge, &activeEdges);
            leftEdge = e;
        }
    }
    SkASSERT(!activeEdges.fHead && !activeEdges.fTail);
    return result;
}

// Stage 5: Tessellate the simplified mesh into monotone polygons.

Poly* GrTriangulator::tessellate(const VertexList& vertices) {
    TESS_LOG("\ntessellating simple polygons\n");
    int maxWindMagnitude = std::numeric_limits<int>::max();
    if (Mode::kSimpleInnerPolygons == fMode && !SkPathFillType_IsEvenOdd(fPath.getFillType())) {
        maxWindMagnitude = 1;
    }
    EdgeList activeEdges;
    Poly* polys = nullptr;
    for (Vertex* v = vertices.fHead; v != nullptr; v = v->fNext) {
        if (!connected(v)) {
            continue;
        }
#if LOGGING_ENABLED
        TESS_LOG("\nvertex %g: (%g,%g), alpha %d\n", v->fID, v->fPoint.fX, v->fPoint.fY, v->fAlpha);
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
        TESS_LOG("edges above:\n");
        for (Edge* e = v->fFirstEdgeAbove; e; e = e->fNextEdgeAbove) {
            TESS_LOG("%g -> %g, lpoly %d, rpoly %d\n",
                     e->fTop->fID, e->fBottom->fID,
                     e->fLeftPoly ? e->fLeftPoly->fID : -1,
                     e->fRightPoly ? e->fRightPoly->fID : -1);
        }
        TESS_LOG("edges below:\n");
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            TESS_LOG("%g -> %g, lpoly %d, rpoly %d\n",
                     e->fTop->fID, e->fBottom->fID,
                     e->fLeftPoly ? e->fLeftPoly->fID : -1,
                     e->fRightPoly ? e->fRightPoly->fID : -1);
        }
#endif
        if (v->fFirstEdgeAbove) {
            if (leftPoly) {
                leftPoly = leftPoly->addEdge(v->fFirstEdgeAbove, Poly::kRight_Side, fAlloc);
            }
            if (rightPoly) {
                rightPoly = rightPoly->addEdge(v->fLastEdgeAbove, Poly::kLeft_Side, fAlloc);
            }
            for (Edge* e = v->fFirstEdgeAbove; e != v->fLastEdgeAbove; e = e->fNextEdgeAbove) {
                Edge* rightEdge = e->fNextEdgeAbove;
                remove_edge(e, &activeEdges);
                if (e->fRightPoly) {
                    e->fRightPoly->addEdge(e, Poly::kLeft_Side, fAlloc);
                }
                if (rightEdge->fLeftPoly && rightEdge->fLeftPoly != e->fRightPoly) {
                    rightEdge->fLeftPoly->addEdge(e, Poly::kRight_Side, fAlloc);
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
                                                 leftPoly->fWinding, fAlloc);
                            leftEnclosingEdge->fRightPoly = leftPoly;
                        } else {
                            rightPoly = new_poly(&polys, rightPoly->lastVertex(),
                                                 rightPoly->fWinding, fAlloc);
                            rightEnclosingEdge->fLeftPoly = rightPoly;
                        }
                    }
                    Edge* join = fAlloc.make<Edge>(leftPoly->lastVertex(), v, 1,
                                                   Edge::Type::kInner);
                    leftPoly = leftPoly->addEdge(join, Poly::kRight_Side, fAlloc);
                    rightPoly = rightPoly->addEdge(join, Poly::kLeft_Side, fAlloc);
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
                    if (abs(winding) > maxWindMagnitude) {
                        return nullptr;  // We can't have weighted wind in kSimpleInnerPolygons mode
                    }
                    Poly* poly = new_poly(&polys, v, winding, fAlloc);
                    leftEdge->fRightPoly = rightEdge->fLeftPoly = poly;
                }
                leftEdge = rightEdge;
            }
            v->fLastEdgeBelow->fRightPoly = rightPoly;
        }
#if LOGGING_ENABLED
        TESS_LOG("\nactive edges:\n");
        for (Edge* e = activeEdges.fHead; e != nullptr; e = e->fRight) {
            TESS_LOG("%g -> %g, lpoly %d, rpoly %d\n",
                     e->fTop->fID, e->fBottom->fID,
                     e->fLeftPoly ? e->fLeftPoly->fID : -1,
                     e->fRightPoly ? e->fRightPoly->fID : -1);
        }
#endif
    }
    return polys;
}

static void remove_non_boundary_edges(const VertexList& mesh, SkPathFillType fillType,
                                      SkArenaAlloc& alloc) {
    TESS_LOG("removing non-boundary edges\n");
    EdgeList activeEdges;
    for (Vertex* v = mesh.fHead; v != nullptr; v = v->fNext) {
        if (!connected(v)) {
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
static void get_edge_normal(const Edge* e, SkVector* normal) {
    normal->set(SkDoubleToScalar(e->fLine.fA),
                SkDoubleToScalar(e->fLine.fB));
}

// Stage 5c: detect and remove "pointy" vertices whose edge normals point in opposite directions
// and whose adjacent vertices are less than a quarter pixel from an edge. These are guaranteed to
// invert on stroking.

static void simplify_boundary(EdgeList* boundary, Comparator& c, SkArenaAlloc& alloc) {
    Edge* prevEdge = boundary->fTail;
    SkVector prevNormal;
    get_edge_normal(prevEdge, &prevNormal);
    for (Edge* e = boundary->fHead; e != nullptr;) {
        Vertex* prev = prevEdge->fWinding == 1 ? prevEdge->fTop : prevEdge->fBottom;
        Vertex* next = e->fWinding == 1 ? e->fBottom : e->fTop;
        double distPrev = e->dist(prev->fPoint);
        double distNext = prevEdge->dist(next->fPoint);
        SkVector normal;
        get_edge_normal(e, &normal);
        constexpr double kQuarterPixelSq = 0.25f * 0.25f;
        if (prev == next) {
            remove_edge(prevEdge, boundary);
            remove_edge(e, boundary);
            prevEdge = boundary->fTail;
            e = boundary->fHead;
            if (prevEdge) {
                get_edge_normal(prevEdge, &prevNormal);
            }
        } else if (prevNormal.dot(normal) < 0.0 &&
            (distPrev * distPrev <= kQuarterPixelSq || distNext * distNext <= kQuarterPixelSq)) {
            Edge* join = new_edge(prev, next, Edge::Type::kInner, c, alloc);
            if (prev->fPoint != next->fPoint) {
                join->fLine.normalize();
                join->fLine = join->fLine * join->fWinding;
            }
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

static void ss_connect(Vertex* v, Vertex* dest, Comparator& c, SkArenaAlloc& alloc) {
    if (v == dest) {
        return;
    }
    TESS_LOG("ss_connecting vertex %g to vertex %g\n", v->fID, dest->fID);
    if (v->fSynthetic) {
        connect(v, dest, Edge::Type::kConnector, c, alloc, 0);
    } else if (v->fPartner) {
        TESS_LOG("setting %g's partner to %g ", v->fPartner->fID, dest->fID);
        TESS_LOG("and %g's partner to null\n", v->fID);
        v->fPartner->fPartner = dest;
        v->fPartner = nullptr;
    }
}

void Event::apply(VertexList* mesh, Comparator& c, EventList* events, SkArenaAlloc& alloc) {
    if (!fEdge) {
        return;
    }
    Vertex* prev = fEdge->fPrev->fVertex;
    Vertex* next = fEdge->fNext->fVertex;
    SSEdge* prevEdge = fEdge->fPrev->fPrev;
    SSEdge* nextEdge = fEdge->fNext->fNext;
    if (!prevEdge || !nextEdge || !prevEdge->fEdge || !nextEdge->fEdge) {
        return;
    }
    Vertex* dest = create_sorted_vertex(fPoint, fAlpha, mesh, prev, c, alloc);
    dest->fSynthetic = true;
    SSVertex* ssv = alloc.make<SSVertex>(dest);
    TESS_LOG("collapsing %g, %g (original edge %g -> %g) to %g (%g, %g) alpha %d\n",
             prev->fID, next->fID, fEdge->fEdge->fTop->fID, fEdge->fEdge->fBottom->fID, dest->fID,
             fPoint.fX, fPoint.fY, fAlpha);
    fEdge->fEdge = nullptr;

    ss_connect(prev, dest, c, alloc);
    ss_connect(next, dest, c, alloc);

    prevEdge->fNext = nextEdge->fPrev = ssv;
    ssv->fPrev = prevEdge;
    ssv->fNext = nextEdge;
    if (!prevEdge->fEdge || !nextEdge->fEdge) {
        return;
    }
    if (prevEdge->fEvent) {
        prevEdge->fEvent->fEdge = nullptr;
    }
    if (nextEdge->fEvent) {
        nextEdge->fEvent->fEdge = nullptr;
    }
    if (prevEdge->fPrev == nextEdge->fNext) {
        ss_connect(prevEdge->fPrev->fVertex, dest, c, alloc);
        prevEdge->fEdge = nextEdge->fEdge = nullptr;
    } else {
        compute_bisector(prevEdge->fEdge, nextEdge->fEdge, dest, alloc);
        SkASSERT(prevEdge != fEdge && nextEdge != fEdge);
        if (dest->fPartner) {
            create_event(prevEdge, events, alloc);
            create_event(nextEdge, events, alloc);
        } else {
            create_event(prevEdge, prevEdge->fPrev->fVertex, nextEdge, dest, events, c, alloc);
            create_event(nextEdge, nextEdge->fNext->fVertex, prevEdge, dest, events, c, alloc);
        }
    }
}

static bool is_overlap_edge(Edge* e) {
    if (e->fType == Edge::Type::kOuter) {
        return e->fWinding != 0 && e->fWinding != 1;
    } else if (e->fType == Edge::Type::kInner) {
        return e->fWinding != 0 && e->fWinding != -2;
    } else {
        return false;
    }
}

// This is a stripped-down version of tessellate() which computes edges which
// join two filled regions, which represent overlap regions, and collapses them.
static bool collapse_overlap_regions(VertexList* mesh, Comparator& c, SkArenaAlloc& alloc,
                                     EventComparator comp) {
    TESS_LOG("\nfinding overlap regions\n");
    EdgeList activeEdges;
    EventList events(comp);
    SSVertexMap ssVertices;
    SSEdgeList ssEdges;
    for (Vertex* v = mesh->fHead; v != nullptr; v = v->fNext) {
        if (!connected(v)) {
            continue;
        }
        Edge* leftEnclosingEdge;
        Edge* rightEnclosingEdge;
        find_enclosing_edges(v, &activeEdges, &leftEnclosingEdge, &rightEnclosingEdge);
        for (Edge* e = v->fLastEdgeAbove; e && e != leftEnclosingEdge;) {
            Edge* prev = e->fPrevEdgeAbove ? e->fPrevEdgeAbove : leftEnclosingEdge;
            remove_edge(e, &activeEdges);
            bool leftOverlap = prev && is_overlap_edge(prev);
            bool rightOverlap = is_overlap_edge(e);
            bool isOuterBoundary = e->fType == Edge::Type::kOuter &&
                                   (!prev || prev->fWinding == 0 || e->fWinding == 0);
            if (prev) {
                e->fWinding -= prev->fWinding;
            }
            if (leftOverlap && rightOverlap) {
                TESS_LOG("found interior overlap edge %g -> %g, disconnecting\n",
                         e->fTop->fID, e->fBottom->fID);
                disconnect(e);
            } else if (leftOverlap || rightOverlap) {
                TESS_LOG("found overlap edge %g -> %g%s\n",
                         e->fTop->fID, e->fBottom->fID,
                         isOuterBoundary ? ", is outer boundary" : "");
                Vertex* prevVertex = e->fWinding < 0 ? e->fBottom : e->fTop;
                Vertex* nextVertex = e->fWinding < 0 ? e->fTop : e->fBottom;
                SSVertex* ssPrev = ssVertices[prevVertex];
                if (!ssPrev) {
                    ssPrev = ssVertices[prevVertex] = alloc.make<SSVertex>(prevVertex);
                }
                SSVertex* ssNext = ssVertices[nextVertex];
                if (!ssNext) {
                    ssNext = ssVertices[nextVertex] = alloc.make<SSVertex>(nextVertex);
                }
                SSEdge* ssEdge = alloc.make<SSEdge>(e, ssPrev, ssNext);
                ssEdges.push_back(ssEdge);
//                SkASSERT(!ssPrev->fNext && !ssNext->fPrev);
                ssPrev->fNext = ssNext->fPrev = ssEdge;
                create_event(ssEdge, &events, alloc);
                if (!isOuterBoundary) {
                    disconnect(e);
                }
            }
            e = prev;
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
    bool complex = events.size() > 0;

    TESS_LOG("\ncollapsing overlap regions\n");
    TESS_LOG("skeleton before:\n");
    dump_skel(ssEdges);
    while (events.size() > 0) {
        Event* event = events.top();
        events.pop();
        event->apply(mesh, c, &events, alloc);
    }
    TESS_LOG("skeleton after:\n");
    dump_skel(ssEdges);
    for (SSEdge* edge : ssEdges) {
        if (Edge* e = edge->fEdge) {
            connect(edge->fPrev->fVertex, edge->fNext->fVertex, e->fType, c, alloc, 0);
        }
    }
    return complex;
}

static bool inversion(Vertex* prev, Vertex* next, Edge* origEdge, Comparator& c) {
    if (!prev || !next) {
        return true;
    }
    int winding = c.sweep_lt(prev->fPoint, next->fPoint) ? 1 : -1;
    return winding != origEdge->fWinding;
}

// Stage 5d: Displace edges by half a pixel inward and outward along their normals. Intersect to
// find new vertices, and set zero alpha on the exterior and one alpha on the interior. Build a
// new antialiased mesh from those vertices.

static void stroke_boundary(EdgeList* boundary, VertexList* innerMesh, VertexList* outerMesh,
                            Comparator& c, SkArenaAlloc& alloc) {
    TESS_LOG("\nstroking boundary\n");
    // A boundary with fewer than 3 edges is degenerate.
    if (!boundary->fHead || !boundary->fHead->fRight || !boundary->fHead->fRight->fRight) {
        return;
    }
    Edge* prevEdge = boundary->fTail;
    Vertex* prevV = prevEdge->fWinding > 0 ? prevEdge->fTop : prevEdge->fBottom;
    SkVector prevNormal;
    get_edge_normal(prevEdge, &prevNormal);
    double radius = 0.5;
    Line prevInner(prevEdge->fLine);
    prevInner.fC -= radius;
    Line prevOuter(prevEdge->fLine);
    prevOuter.fC += radius;
    VertexList innerVertices;
    VertexList outerVertices;
    bool innerInversion = true;
    bool outerInversion = true;
    for (Edge* e = boundary->fHead; e != nullptr; e = e->fRight) {
        Vertex* v = e->fWinding > 0 ? e->fTop : e->fBottom;
        SkVector normal;
        get_edge_normal(e, &normal);
        Line inner(e->fLine);
        inner.fC -= radius;
        Line outer(e->fLine);
        outer.fC += radius;
        SkPoint innerPoint, outerPoint;
        TESS_LOG("stroking vertex %g (%g, %g)\n", v->fID, v->fPoint.fX, v->fPoint.fY);
        if (!prevEdge->fLine.nearParallel(e->fLine) && prevInner.intersect(inner, &innerPoint) &&
            prevOuter.intersect(outer, &outerPoint)) {
            float cosAngle = normal.dot(prevNormal);
            if (cosAngle < -kCosMiterAngle) {
                Vertex* nextV = e->fWinding > 0 ? e->fBottom : e->fTop;

                // This is a pointy vertex whose angle is smaller than the threshold; miter it.
                Line bisector(innerPoint, outerPoint);
                Line tangent(v->fPoint, v->fPoint + SkPoint::Make(bisector.fA, bisector.fB));
                if (tangent.fA == 0 && tangent.fB == 0) {
                    continue;
                }
                tangent.normalize();
                Line innerTangent(tangent);
                Line outerTangent(tangent);
                innerTangent.fC -= 0.5;
                outerTangent.fC += 0.5;
                SkPoint innerPoint1, innerPoint2, outerPoint1, outerPoint2;
                if (prevNormal.cross(normal) > 0) {
                    // Miter inner points
                    if (!innerTangent.intersect(prevInner, &innerPoint1) ||
                        !innerTangent.intersect(inner, &innerPoint2) ||
                        !outerTangent.intersect(bisector, &outerPoint)) {
                        continue;
                    }
                    Line prevTangent(prevV->fPoint,
                                     prevV->fPoint + SkVector::Make(prevOuter.fA, prevOuter.fB));
                    Line nextTangent(nextV->fPoint,
                                     nextV->fPoint + SkVector::Make(outer.fA, outer.fB));
                    if (prevTangent.dist(outerPoint) > 0) {
                        bisector.intersect(prevTangent, &outerPoint);
                    }
                    if (nextTangent.dist(outerPoint) < 0) {
                        bisector.intersect(nextTangent, &outerPoint);
                    }
                    outerPoint1 = outerPoint2 = outerPoint;
                } else {
                    // Miter outer points
                    if (!outerTangent.intersect(prevOuter, &outerPoint1) ||
                        !outerTangent.intersect(outer, &outerPoint2)) {
                        continue;
                    }
                    Line prevTangent(prevV->fPoint,
                                     prevV->fPoint + SkVector::Make(prevInner.fA, prevInner.fB));
                    Line nextTangent(nextV->fPoint,
                                     nextV->fPoint + SkVector::Make(inner.fA, inner.fB));
                    if (prevTangent.dist(innerPoint) > 0) {
                        bisector.intersect(prevTangent, &innerPoint);
                    }
                    if (nextTangent.dist(innerPoint) < 0) {
                        bisector.intersect(nextTangent, &innerPoint);
                    }
                    innerPoint1 = innerPoint2 = innerPoint;
                }
                if (!innerPoint1.isFinite() || !innerPoint2.isFinite() ||
                    !outerPoint1.isFinite() || !outerPoint2.isFinite()) {
                    continue;
                }
                TESS_LOG("inner (%g, %g), (%g, %g), ",
                         innerPoint1.fX, innerPoint1.fY, innerPoint2.fX, innerPoint2.fY);
                TESS_LOG("outer (%g, %g), (%g, %g)\n",
                         outerPoint1.fX, outerPoint1.fY, outerPoint2.fX, outerPoint2.fY);
                Vertex* innerVertex1 = alloc.make<Vertex>(innerPoint1, 255);
                Vertex* innerVertex2 = alloc.make<Vertex>(innerPoint2, 255);
                Vertex* outerVertex1 = alloc.make<Vertex>(outerPoint1, 0);
                Vertex* outerVertex2 = alloc.make<Vertex>(outerPoint2, 0);
                innerVertex1->fPartner = outerVertex1;
                innerVertex2->fPartner = outerVertex2;
                outerVertex1->fPartner = innerVertex1;
                outerVertex2->fPartner = innerVertex2;
                if (!inversion(innerVertices.fTail, innerVertex1, prevEdge, c)) {
                    innerInversion = false;
                }
                if (!inversion(outerVertices.fTail, outerVertex1, prevEdge, c)) {
                    outerInversion = false;
                }
                innerVertices.append(innerVertex1);
                innerVertices.append(innerVertex2);
                outerVertices.append(outerVertex1);
                outerVertices.append(outerVertex2);
            } else {
                TESS_LOG("inner (%g, %g), ", innerPoint.fX, innerPoint.fY);
                TESS_LOG("outer (%g, %g)\n", outerPoint.fX, outerPoint.fY);
                Vertex* innerVertex = alloc.make<Vertex>(innerPoint, 255);
                Vertex* outerVertex = alloc.make<Vertex>(outerPoint, 0);
                innerVertex->fPartner = outerVertex;
                outerVertex->fPartner = innerVertex;
                if (!inversion(innerVertices.fTail, innerVertex, prevEdge, c)) {
                    innerInversion = false;
                }
                if (!inversion(outerVertices.fTail, outerVertex, prevEdge, c)) {
                    outerInversion = false;
                }
                innerVertices.append(innerVertex);
                outerVertices.append(outerVertex);
            }
        }
        prevInner = inner;
        prevOuter = outer;
        prevV = v;
        prevEdge = e;
        prevNormal = normal;
    }
    if (!inversion(innerVertices.fTail, innerVertices.fHead, prevEdge, c)) {
        innerInversion = false;
    }
    if (!inversion(outerVertices.fTail, outerVertices.fHead, prevEdge, c)) {
        outerInversion = false;
    }
    // Outer edges get 1 winding, and inner edges get -2 winding. This ensures that the interior
    // is always filled (1 + -2 = -1 for normal cases, 1 + 2 = 3 for thin features where the
    // interior inverts).
    // For total inversion cases, the shape has now reversed handedness, so invert the winding
    // so it will be detected during collapse_overlap_regions().
    int innerWinding = innerInversion ? 2 : -2;
    int outerWinding = outerInversion ? -1 : 1;
    for (Vertex* v = innerVertices.fHead; v && v->fNext; v = v->fNext) {
        connect(v, v->fNext, Edge::Type::kInner, c, alloc, innerWinding);
    }
    connect(innerVertices.fTail, innerVertices.fHead, Edge::Type::kInner, c, alloc, innerWinding);
    for (Vertex* v = outerVertices.fHead; v && v->fNext; v = v->fNext) {
        connect(v, v->fNext, Edge::Type::kOuter, c, alloc, outerWinding);
    }
    connect(outerVertices.fTail, outerVertices.fHead, Edge::Type::kOuter, c, alloc, outerWinding);
    innerMesh->append(innerVertices);
    outerMesh->append(outerVertices);
}

static void extract_boundary(EdgeList* boundary, Edge* e, SkPathFillType fillType,
                             SkArenaAlloc& alloc) {
    TESS_LOG("\nextracting boundary\n");
    bool down = apply_fill_type(fillType, e->fWinding);
    Vertex* start = down ? e->fTop : e->fBottom;
    do {
        e->fWinding = down ? 1 : -1;
        Edge* next;
        e->fLine.normalize();
        e->fLine = e->fLine * e->fWinding;
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
    } while (e && (down ? e->fTop : e->fBottom) != start);
}

// Stage 5b: Extract boundaries from mesh, simplify and stroke them into a new mesh.

static void extract_boundaries(const VertexList& inMesh, VertexList* innerVertices,
                               VertexList* outerVertices, SkPathFillType fillType, Comparator& c,
                               SkArenaAlloc& alloc) {
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

void GrTriangulator::contoursToMesh(VertexList* contours, int contourCnt, VertexList* mesh,
                                    Comparator& c) {
#if LOGGING_ENABLED
    for (int i = 0; i < contourCnt; ++i) {
        Vertex* v = contours[i].fHead;
        SkASSERT(v);
        TESS_LOG("path.moveTo(%20.20g, %20.20g);\n", v->fPoint.fX, v->fPoint.fY);
        for (v = v->fNext; v; v = v->fNext) {
            TESS_LOG("path.lineTo(%20.20g, %20.20g);\n", v->fPoint.fX, v->fPoint.fY);
        }
    }
#endif
    this->sanitizeContours(contours, contourCnt);
    this->buildEdges(contours, contourCnt, mesh, c);
}

void GrTriangulator::SortMesh(VertexList* vertices, const Comparator& c) {
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

Poly* GrTriangulator::contoursToPolys(VertexList* contours, int contourCnt, VertexList* outerMesh) {
    const SkRect& pathBounds = fPath.getBounds();
    Comparator c(pathBounds.width() > pathBounds.height() ? Comparator::Direction::kHorizontal
                                                          : Comparator::Direction::kVertical);
    VertexList mesh;
    this->contoursToMesh(contours, contourCnt, &mesh, c);
    SortMesh(&mesh, c);
    this->mergeCoincidentVertices(&mesh, c);
    if (SimplifyResult::kAbort == this->simplify(&mesh, c)) {
        return nullptr;
    }
    TESS_LOG("\nsimplified mesh:\n");
    dump_mesh(mesh);
    if (Mode::kEdgeAntialias == fMode) {
        VertexList innerMesh;
        extract_boundaries(mesh, &innerMesh, outerMesh, fPath.getFillType(), c, fAlloc);
        SortMesh(&innerMesh, c);
        SortMesh(outerMesh, c);
        this->mergeCoincidentVertices(&innerMesh, c);
        bool was_complex = this->mergeCoincidentVertices(outerMesh, c);
        auto result = this->simplify(&innerMesh, c);
        SkASSERT(SimplifyResult::kAbort != result);
        was_complex = (SimplifyResult::kFoundSelfIntersection == result) || was_complex;
        result = this->simplify(outerMesh, c);
        SkASSERT(SimplifyResult::kAbort != result);
        was_complex = (SimplifyResult::kFoundSelfIntersection == result) || was_complex;
        TESS_LOG("\ninner mesh before:\n");
        dump_mesh(innerMesh);
        TESS_LOG("\nouter mesh before:\n");
        dump_mesh(*outerMesh);
        EventComparator eventLT(EventComparator::Op::kLessThan);
        EventComparator eventGT(EventComparator::Op::kGreaterThan);
        was_complex = collapse_overlap_regions(&innerMesh, c, fAlloc, eventLT) || was_complex;
        was_complex = collapse_overlap_regions(outerMesh, c, fAlloc, eventGT) || was_complex;
        if (was_complex) {
            TESS_LOG("found complex mesh; taking slow path\n");
            VertexList aaMesh;
            TESS_LOG("\ninner mesh after:\n");
            dump_mesh(innerMesh);
            TESS_LOG("\nouter mesh after:\n");
            dump_mesh(*outerMesh);
            connect_partners(outerMesh, c, fAlloc);
            connect_partners(&innerMesh, c, fAlloc);
            sorted_merge(&innerMesh, outerMesh, &aaMesh, c);
            this->mergeCoincidentVertices(&aaMesh, c);
            result = this->simplify(&aaMesh, c);
            SkASSERT(SimplifyResult::kAbort != result);
            TESS_LOG("combined and simplified mesh:\n");
            dump_mesh(aaMesh);
            outerMesh->fHead = outerMesh->fTail = nullptr;
            return this->tessellate(aaMesh);
        } else {
            TESS_LOG("no complex polygons; taking fast path\n");
            return this->tessellate(innerMesh);
        }
    } else {
        return this->tessellate(mesh);
    }
}

// Stage 6: Triangulate the monotone polygons into a vertex buffer.
void* GrTriangulator::polysToTriangles(Poly* polys, void* data, SkPathFillType overrideFillType) {
    bool emitCoverage = (Mode::kEdgeAntialias == fMode);
    for (Poly* poly = polys; poly; poly = poly->fNext) {
        if (apply_fill_type(overrideFillType, poly)) {
            data = poly->emit(emitCoverage, data);
        }
    }
    return data;
}

Poly* GrTriangulator::pathToPolys(float tolerance, const SkRect& clipBounds, int contourCnt,
                                  VertexList* outerMesh) {
    if (SkPathFillType_IsInverse(fPath.getFillType())) {
        contourCnt++;
    }
    std::unique_ptr<VertexList[]> contours(new VertexList[contourCnt]);

    this->pathToContours(tolerance, clipBounds, contours.get());
    return this->contoursToPolys(contours.get(), contourCnt, outerMesh);
}

static int get_contour_count(const SkPath& path, SkScalar tolerance) {
    // We could theoretically be more aggressive about not counting empty contours, but we need to
    // actually match the exact number of contour linked lists the tessellator will create later on.
    int contourCnt = 1;
    bool hasPoints = false;

    SkPath::Iter iter(path, false);
    SkPath::Verb verb;
    SkPoint pts[4];
    bool first = true;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                if (!first) {
                    ++contourCnt;
                }
                [[fallthrough]];
            case SkPath::kLine_Verb:
            case SkPath::kConic_Verb:
            case SkPath::kQuad_Verb:
            case SkPath::kCubic_Verb:
                hasPoints = true;
                break;
            default:
                break;
        }
        first = false;
    }
    if (!hasPoints) {
        return 0;
    }
    return contourCnt;
}

static int64_t count_points(Poly* polys, SkPathFillType fillType) {
    int64_t count = 0;
    for (Poly* poly = polys; poly; poly = poly->fNext) {
        if (apply_fill_type(fillType, poly) && poly->fCount >= 3) {
            count += (poly->fCount - 2) * (TRIANGULATOR_WIREFRAME ? 6 : 3);
        }
    }
    return count;
}

static int64_t count_outer_mesh_points(const VertexList& outerMesh) {
    int64_t count = 0;
    for (Vertex* v = outerMesh.fHead; v; v = v->fNext) {
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            count += TRIANGULATOR_WIREFRAME ? 12 : 6;
        }
    }
    return count;
}

static void* outer_mesh_to_triangles(const VertexList& outerMesh, bool emitCoverage, void* data) {
    for (Vertex* v = outerMesh.fHead; v; v = v->fNext) {
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            Vertex* v0 = e->fTop;
            Vertex* v1 = e->fBottom;
            Vertex* v2 = e->fBottom->fPartner;
            Vertex* v3 = e->fTop->fPartner;
            data = emit_triangle(v0, v1, v2, emitCoverage, data);
            data = emit_triangle(v0, v2, v3, emitCoverage, data);
        }
    }
    return data;
}

// Stage 6: Triangulate the monotone polygons into a vertex buffer.

int GrTriangulator::PathToTriangles(const SkPath& path, SkScalar tolerance,
                                    const SkRect& clipBounds,
                                    GrEagerVertexAllocator* vertexAllocator, Mode mode,
                                    bool* isLinear) {
    GrTriangulator triangulator(path, mode);
    SkPathFillType overrideFillType = (GrTriangulator::Mode::kEdgeAntialias == mode)
            ? SkPathFillType::kWinding
            : path.getFillType();
    int count = triangulator.pathToTriangles(tolerance, clipBounds, vertexAllocator,
                                             overrideFillType);
    *isLinear = triangulator.fIsLinear;
    return count;
}

int GrTriangulator::pathToTriangles(float tolerance, const SkRect& clipBounds,
                                    GrEagerVertexAllocator* vertexAllocator,
                                    SkPathFillType overrideFillType) {
    int contourCnt = get_contour_count(fPath, tolerance);
    if (contourCnt <= 0) {
        fIsLinear = true;
        return 0;
    }
    VertexList outerMesh;
    Poly* polys = this->pathToPolys(tolerance, clipBounds, contourCnt, &outerMesh);
    int64_t count64 = count_points(polys, overrideFillType);
    if (GrTriangulator::Mode::kEdgeAntialias == fMode) {
        count64 += count_outer_mesh_points(outerMesh);
    }
    if (0 == count64 || count64 > SK_MaxS32) {
        return 0;
    }
    int count = count64;

    size_t vertexStride = GetVertexStride(fMode);
    void* verts = vertexAllocator->lock(vertexStride, count);
    if (!verts) {
        SkDebugf("Could not allocate vertices\n");
        return 0;
    }

    TESS_LOG("emitting %d verts\n", count);
    void* end = this->polysToTriangles(polys, verts, overrideFillType);
    end = outer_mesh_to_triangles(outerMesh, true, end);

    int actualCount = static_cast<int>((static_cast<uint8_t*>(end) - static_cast<uint8_t*>(verts))
                                       / vertexStride);
    SkASSERT(actualCount <= count);
    vertexAllocator->unlock(actualCount);
    return actualCount;
}

int GrTriangulator::PathToVertices(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
                                   WindingVertex** verts) {
    int contourCnt = get_contour_count(path, tolerance);
    if (contourCnt <= 0) {
        *verts = nullptr;
        return 0;
    }
    GrTriangulator triangulator(path, Mode::kNormal);
    Poly* polys = triangulator.pathToPolys(tolerance, clipBounds, contourCnt, nullptr);
    SkPathFillType fillType = path.getFillType();
    int64_t count64 = count_points(polys, fillType);
    if (0 == count64 || count64 > SK_MaxS32) {
        *verts = nullptr;
        return 0;
    }
    int count = count64;

    *verts = new WindingVertex[count];
    WindingVertex* vertsEnd = *verts;
    SkPoint* points = new SkPoint[count];
    SkPoint* pointsEnd = points;
    for (Poly* poly = polys; poly; poly = poly->fNext) {
        if (apply_fill_type(fillType, poly)) {
            SkPoint* start = pointsEnd;
            pointsEnd = static_cast<SkPoint*>(poly->emit(false, pointsEnd));
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
