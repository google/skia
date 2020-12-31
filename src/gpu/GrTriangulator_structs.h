/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTriangulator_structs_DEFINED
#define GrTriangulator_structs_DEFINED

#include "src/gpu/GrTriangulator.h"

enum class GrTriangulator::Side : bool {
    kLeft,
    kRight
};

enum class GrTriangulator::EdgeType : int {
    kInner,
    kOuter,
    kConnector
};

// Vertices are used in three ways: first, the path contours are converted into a
// circularly-linked list of Vertices for each contour. After edge construction, the same Vertices
// are re-ordered by the merge sort according to the sweep_lt comparator (usually, increasing
// in Y) using the same fPrev/fNext pointers that were used for the contours, to avoid
// reallocation. Finally, MonotonePolys are built containing a circularly-linked list of
// Vertices. (Currently, those Vertices are newly-allocated for the MonotonePolys, since
// an individual Vertex from the path mesh may belong to multiple
// MonotonePolys, so the original Vertices cannot be re-used.
struct GrTriangulator::Vertex {
    Vertex(const SkPoint& point, uint8_t alpha)
            : fPoint(point)
            , fAlpha(alpha)
#if TRIANGULATOR_LOGGING
            , fID (-1.0f)
#endif
    {}
    SkPoint fPoint;                 // Vertex position
    Vertex* fPrev{};                // Linked list of contours, then Y-sorted vertices.
    Vertex* fNext{};                // "
    Edge*   fFirstEdgeAbove{};      // Linked list of edges above this vertex.
    Edge*   fLastEdgeAbove{};       // "
    Edge*   fFirstEdgeBelow{};      // Linked list of edges below this vertex.
    Edge*   fLastEdgeBelow{};       // "
    Edge*   fLeftEnclosingEdge{};   // Nearest edge in the AEL left of this vertex.
    Edge*   fRightEnclosingEdge{};  // Nearest edge in the AEL right of this vertex.
    Vertex* fPartner{};             // Corresponding inner or outer vertex (for AA).
    uint8_t fAlpha;
    bool    fSynthetic{};           // Is this a synthetic vertex?
#if TRIANGULATOR_LOGGING
    float   fID;                    // Identifier used for logging.
#endif
    bool isConnected() const { return fFirstEdgeAbove || fFirstEdgeBelow; }
    void insertEdgeAbove(Edge*, const Comparator& c);
    void insertEdgeBelow(Edge*, const Comparator& c);
};

struct GrTriangulator::VertexList {
    VertexList() = default;
    VertexList(Vertex* head, Vertex* tail) : fHead(head), fTail(tail) {}
    Vertex* fHead{};
    Vertex* fTail{};
    void insert(Vertex* v, Vertex* prev, Vertex* next);
    void append(Vertex* v) { this->insert(v, fTail, nullptr); }
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
    void prepend(Vertex* v) { this->insert(v, nullptr, fHead); }
    void remove(Vertex* v);
    void close() {
        if (fHead && fTail) {
            fTail->fNext = fHead;
            fHead->fPrev = fTail;
        }
    }
#if TRIANGULATOR_LOGGING
    void dump();
#endif
};

// A line equation in implicit form. fA * x + fB * y + fC = 0, for all points (x, y) on the line.
struct GrTriangulator::Line {
    Line(double a, double b, double c) : fA(a), fB(b), fC(c) {}
    Line(Vertex* p, Vertex* q) : Line(p->fPoint, q->fPoint) {}
    Line(const SkPoint& p, const SkPoint& q)
            : fA(static_cast<double>(q.fY) - p.fY)      // a = dY
            , fB(static_cast<double>(p.fX) - q.fX)      // b = -dX
            , fC(static_cast<double>(p.fY) * q.fX -     // c = cross(q, p)
                 static_cast<double>(p.fX) * q.fY) {
    }
    double dist(const SkPoint& p) const { return fA * p.fX + fB * p.fY + fC; }
    Line operator*(double v) const { return Line(fA * v, fB * v, fC * v); }
    double magSq() const { return fA * fA + fB * fB; }
    void normalize();
    bool nearParallel(const Line& o) const {
        return fabs(o.fA - fA) < 0.00001 && fabs(o.fB - fB) < 0.00001;
    }
    // Compute the intersection of two (infinite) Lines.
    bool intersect(const Line& other, SkPoint* point) const;
    double fA, fB, fC;
};

// An Edge joins a top Vertex to a bottom Vertex. Edge ordering for the list of "edges above" and
// "edge below" a vertex as well as for the active edge list is handled by isLeftOf()/isRightOf().
// Note that an Edge will give occasionally dist() != 0 for its own endpoints (because floating
// point). For speed, that case is only tested by the callers that require it (e.g.,
// rewind_if_necessary()). Edges also handle checking for intersection with other edges.
// Currently, this converts the edges to the parametric form, in order to avoid doing a division
// until an intersection has been confirmed. This is slightly slower in the "found" case, but
// a lot faster in the "not found" case.
//
// The coefficients of the line equation stored in double precision to avoid catastrphic
// cancellation in the isLeftOf() and isRightOf() checks. Using doubles ensures that the result is
// correct in float, since it's a polynomial of degree 2. The intersect() function, being
// degree 5, is still subject to catastrophic cancellation. We deal with that by assuming its
// output may be incorrect, and adjusting the mesh topology to match (see comment at the top of
// this file).
struct GrTriangulator::Edge {
    Edge(Vertex* top, Vertex* bottom, int winding, EdgeType type)
            : fWinding(winding), fTop(top), fBottom(bottom), fType(type), fLine(top, bottom) {}
    int      fWinding;          // 1 == edge goes downward; -1 = edge goes upward.
    Vertex*  fTop;              // The top vertex in vertex-sort-order (sweep_lt).
    Vertex*  fBottom;           // The bottom vertex in vertex-sort-order.
    EdgeType fType;
    Edge*    fLeft{};             // The linked list of edges in the active edge list.
    Edge*    fRight{};            // "
    Edge*    fPrevEdgeAbove{};    // The linked list of edges in the bottom Vertex's "edges above".
    Edge*    fNextEdgeAbove{};    // "
    Edge*    fPrevEdgeBelow{};    // The linked list of edges in the top Vertex's "edges below".
    Edge*    fNextEdgeBelow{};    // "
    Poly*    fLeftPoly{};         // The Poly to the left of this edge, if any.
    Poly*    fRightPoly{};        // The Poly to the right of this edge, if any.
    Edge*    fLeftPolyPrev{};
    Edge*    fLeftPolyNext{};
    Edge*    fRightPolyPrev{};
    Edge*    fRightPolyNext{};
    bool     fUsedInLeftPoly{};
    bool     fUsedInRightPoly{};
    Line     fLine;
    void disconnect();
    double dist(const SkPoint& p) const { return fLine.dist(p); }
    bool isRightOf(Vertex* v) const { return fLine.dist(v->fPoint) < 0.0; }
    bool isLeftOf(Vertex* v) const { return fLine.dist(v->fPoint) > 0.0; }
    void recompute() { fLine = Line(fTop, fBottom); }
    bool intersect(const Edge& other, SkPoint* p, uint8_t* alpha = nullptr) const;
};

struct GrTriangulator::EdgeList {
    Edge* fHead{};
    Edge* fTail{};
    void insert(Edge* edge, Edge* prev);
    void insert(Edge* edge, Edge* prev, Edge* next);
    void append(Edge* e) {
        this->insert(e, fTail, nullptr);
    }
    void remove(Edge* edge);
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

struct GrTriangulator::Poly {
    Poly(Vertex* v, int winding) : fFirstVertex(v), fWinding(winding) {
#if TRIANGULATOR_LOGGING
        static int gID = 0;
        fID = gID++;
        TESS_LOG("*** created Poly %d\n", fID);
#endif
    }
    Vertex* lastVertex() const;
    Vertex* fFirstVertex;
    int fWinding;
    MonotonePoly* fHead{};
    MonotonePoly* fTail{};
    Poly* fNext{};
    Poly* fPartner{};
    int fCount{};
#if TRIANGULATOR_LOGGING
    int fID;
#endif
};

struct GrTriangulator::MonotonePoly {
    MonotonePoly(Edge* edge, Side side, int winding) : fSide(side), fWinding(winding) {
        this->addEdge(edge);
    }
    Side          fSide;
    Edge*         fFirstEdge{};
    Edge*         fLastEdge{};
    MonotonePoly* fPrev{};
    MonotonePoly* fNext{};
    int fWinding;
    void addEdge(Edge* edge);
    void* emitTriangle(Vertex* prev, Vertex* curr, Vertex* next, bool emitCoverage,
                       void* data) const;
};

struct GrTriangulator::Comparator {
    enum class Direction { kVertical, kHorizontal };
    Comparator(Direction direction) : fDirection(direction) {}
    bool sweep_lt(const SkPoint& a, const SkPoint& b) const;
    Direction fDirection;
};

#endif
