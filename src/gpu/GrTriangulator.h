/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTriangulator_DEFINED
#define GrTriangulator_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/private/SkColorData.h"
#include "src/core/SkArenaAlloc.h"
#include "src/gpu/GrColor.h"

class GrEagerVertexAllocator;
struct SkRect;

#define TRIANGULATOR_LOGGING 0
#define TRIANGULATOR_WIREFRAME 0

/**
 * Provides utility functions for converting paths to a collection of triangles.
 */
class GrTriangulator {
public:
    static int PathToTriangles(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
                               GrEagerVertexAllocator* vertexAllocator, bool* isLinear) {
        GrTriangulator triangulator(path);
        int count = triangulator.pathToTriangles(tolerance, clipBounds, vertexAllocator);
        *isLinear = triangulator.fIsLinear;
        return count;
    }

    // The breadcrumb triangles serve as a glue that erases T-junctions between a path's outer
    // curves and its inner polygon triangulation. Drawing a path's outer curves, breadcrumb
    // triangles, and inner polygon triangulation all together into the stencil buffer has the same
    // identical rasterized effect as stenciling a classic Redbook fan.
    //
    // The breadcrumb triangles track all the edge splits that led from the original inner polygon
    // edges to the final triangulation. Every time an edge splits, we emit a razor-thin breadcrumb
    // triangle consisting of the edge's original endpoints and the split point. (We also add
    // supplemental breadcrumb triangles to areas where abs(winding) > 1.)
    //
    //                a
    //               /
    //              /
    //             /
    //            x  <- Edge splits at x. New breadcrumb triangle is: [a, b, x].
    //           /
    //          /
    //         b
    //
    // The opposite-direction shared edges between the triangulation and breadcrumb triangles should
    // all cancel out, leaving just the set of edges from the original polygon.
    class BreadcrumbTriangleCollector { public:
        void push(SkPoint a, SkPoint b, SkPoint c, int winding) {
            if (a != b && a != c && b != c) {
                if (winding > 0) {
                    this->onPush(a, b, c, winding);
                } else if (winding < 0) {
                    this->onPush(b, a, c, -winding);
                }
            }
        }
        virtual ~BreadcrumbTriangleCollector() {}
    private:
        virtual void onPush(SkPoint a, SkPoint b, SkPoint c, int winding) = 0;
    };

    static int TriangulateInnerPolygons(const SkPath& path, GrEagerVertexAllocator* vertexAllocator,
                                        BreadcrumbTriangleCollector* breadcrumbTriangles, bool
                                        *isLinear) {
        GrTriangulator triangulator(path);
        triangulator.fCullCollinearVertices = false;
        triangulator.fBreadcrumbTriangles = breadcrumbTriangles;
        int count = triangulator.pathToTriangles(0, SkRect::MakeEmpty(), vertexAllocator);
        *isLinear = triangulator.fIsLinear;
        return count;
    }

    static int TriangulateSimpleInnerPolygons(const SkPath& path,
                                              GrEagerVertexAllocator* vertexAllocator,
                                              bool *isLinear) {
        GrTriangulator triangulator(path);
        triangulator.fCullCollinearVertices = false;
        triangulator.fDisallowSelfIntersection = true;
        int count = triangulator.pathToTriangles(0, SkRect::MakeEmpty(), vertexAllocator);
        *isLinear = triangulator.fIsLinear;
        return count;
    }

    struct WindingVertex {
        SkPoint fPos;
        int fWinding;
    };

    // *DEPRECATED*: Once CCPR is removed this method will go away.
    //
    // Triangulates a path to an array of vertices. Each triangle is represented as a set of three
    // WindingVertex entries, each of which contains the position and winding count (which is the
    // same for all three vertices of a triangle). The 'verts' out parameter is set to point to the
    // resultant vertex array. CALLER IS RESPONSIBLE for deleting this buffer to avoid a memory
    // leak!
    static int PathToVertices(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
                              WindingVertex** verts);

    // Enums used by GrTriangulator internals.
    typedef enum { kLeft_Side, kRight_Side } Side;
    enum class EdgeType { kInner, kOuter, kConnector };

    // Structs used by GrTriangulator internals.
    struct Vertex;
    struct VertexList;
    struct Line;
    struct Edge;
    struct EdgeList;
    struct MonotonePoly;
    struct Poly;
    struct Comparator;

protected:
    GrTriangulator(const SkPath& path) : fPath(path) {}
    virtual ~GrTriangulator() {}

    // There are six stages to the basic algorithm:
    //
    // 1) Linearize the path contours into piecewise linear segments:
    void pathToContours(float tolerance, const SkRect& clipBounds, VertexList* contours);

    // 2) Build a mesh of edges connecting the vertices:
    void contoursToMesh(VertexList* contours, int contourCnt, VertexList* mesh, const Comparator&);

    // 3) Sort the vertices in Y (and secondarily in X):
    static void SortedMerge(VertexList* front, VertexList* back, VertexList* result,
                            const Comparator&);
    static void SortMesh(VertexList* vertices, const Comparator&);

    // 4) Simplify the mesh by inserting new vertices at intersecting edges:
    enum class SimplifyResult {
        kAlreadySimple,
        kFoundSelfIntersection,
        kAbort
    };

    SimplifyResult simplify(VertexList* mesh, const Comparator&);

    // 5) Tessellate the simplified mesh into monotone polygons:
    virtual Poly* tessellate(const VertexList& vertices, const Comparator&);

    // 6) Triangulate the monotone polygons directly into a vertex buffer:
    virtual int64_t countPoints(Poly* polys) const {
        return this->countPointsImpl(polys, fPath.getFillType());
    }
    virtual void* polysToTriangles(Poly* polys, void* data) {
        return this->polysToTrianglesImpl(polys, data, fPath.getFillType());
    }

    // The vertex sorting in step (3) is a merge sort, since it plays well with the linked list
    // of vertices (and the necessity of inserting new vertices on intersection).
    //
    // Stages (4) and (5) use an active edge list -- a list of all edges for which the
    // sweep line has crossed the top vertex, but not the bottom vertex.  It's sorted
    // left-to-right based on the point where both edges are active (when both top vertices
    // have been seen, so the "lower" top vertex of the two). If the top vertices are equal
    // (shared), it's sorted based on the last point where both edges are active, so the
    // "upper" bottom vertex.
    //
    // The most complex step is the simplification (4). It's based on the Bentley-Ottman
    // line-sweep algorithm, but due to floating point inaccuracy, the intersection points are
    // not exact and may violate the mesh topology or active edge list ordering. We
    // accommodate this by adjusting the topology of the mesh and AEL to match the intersection
    // points. This occurs in two ways:
    //
    // A) Intersections may cause a shortened edge to no longer be ordered with respect to its
    //    neighbouring edges at the top or bottom vertex. This is handled by merging the
    //    edges (mergeCollinearVertices()).
    // B) Intersections may cause an edge to violate the left-to-right ordering of the
    //    active edge list. This is handled by detecting potential violations and rewinding
    //    the active edge list to the vertex before they occur (rewind() during merging,
    //    rewind_if_necessary() during splitting).
    //
    // The tessellation steps (5) and (6) are based on "Triangulating Simple Polygons and
    // Equivalent Problems" (Fournier and Montuno); also a line-sweep algorithm. Note that it
    // currently uses a linked list for the active edge list, rather than a 2-3 tree as the
    // paper describes. The 2-3 tree gives O(lg N) lookups, but insertion and removal also
    // become O(lg N). In all the test cases, it was found that the cost of frequent O(lg N)
    // insertions and removals was greater than the cost of infrequent O(N) lookups with the
    // linked list implementation. With the latter, all removals are O(1), and most insertions
    // are O(1), since we know the adjacent edge in the active edge list based on the topology.
    // Only type 2 vertices (see paper) require the O(N) lookups, and these are much less
    // frequent. There may be other data structures worth investigating, however.
    //
    // Note that the orientation of the line sweep algorithms is determined by the aspect ratio of
    // the path bounds. When the path is taller than it is wide, we sort vertices based on
    // increasing Y coordinate, and secondarily by increasing X coordinate. When the path is wider
    // than it is tall, we sort by increasing X coordinate, but secondarily by *decreasing* Y
    // coordinate. This is so that the "left" and "right" orientation in the code remains correct
    // (edges to the left are increasing in Y; edges to the right are decreasing in Y). That is, the
    // setting rotates 90 degrees counterclockwise, rather that transposing.

    // Additional helpers and driver functions.
    void* emitMonotonePoly(const MonotonePoly*, void* data);
    void* emitTriangle(Vertex* prev, Vertex* curr, Vertex* next, int winding, void* data) const;
    void* emitPoly(const Poly*, void *data);
    Poly* makePoly(Poly** head, Vertex* v, int winding);
    void appendPointToContour(const SkPoint& p, VertexList* contour);
    void appendQuadraticToContour(const SkPoint[3], SkScalar toleranceSqd, VertexList* contour);
    void generateCubicPoints(const SkPoint&, const SkPoint&, const SkPoint&, const SkPoint&,
                             SkScalar tolSqd, VertexList* contour, int pointsLeft);
    bool applyFillType(int winding);
    Edge* makeEdge(Vertex* prev, Vertex* next, EdgeType type, const Comparator&);
    void setTop(Edge* edge, Vertex* v, EdgeList* activeEdges, Vertex** current, const Comparator&);
    void setBottom(Edge* edge, Vertex* v, EdgeList* activeEdges, Vertex** current,
                   const Comparator&);
    void mergeEdgesAbove(Edge* edge, Edge* other, EdgeList* activeEdges, Vertex** current,
                         const Comparator&);
    void mergeEdgesBelow(Edge* edge, Edge* other, EdgeList* activeEdges, Vertex** current,
                         const Comparator&);
    Edge* makeConnectingEdge(Vertex* prev, Vertex* next, EdgeType, const Comparator&,
                             int windingScale = 1);
    void mergeVertices(Vertex* src, Vertex* dst, VertexList* mesh, const Comparator&);
    static void FindEnclosingEdges(Vertex* v, EdgeList* edges, Edge** left, Edge** right);
    void mergeCollinearEdges(Edge* edge, EdgeList* activeEdges, Vertex** current,
                             const Comparator&);
    bool splitEdge(Edge* edge, Vertex* v, EdgeList* activeEdges, Vertex** current,
                   const Comparator&);
    bool intersectEdgePair(Edge* left, Edge* right, EdgeList* activeEdges, Vertex** current,
                           const Comparator&);
    Vertex* makeSortedVertex(const SkPoint&, uint8_t alpha, VertexList* mesh, Vertex* reference,
                             const Comparator&);
    void computeBisector(Edge* edge1, Edge* edge2, Vertex*);
    bool checkForIntersection(Edge* left, Edge* right, EdgeList* activeEdges, Vertex** current,
                              VertexList* mesh, const Comparator&);
    void sanitizeContours(VertexList* contours, int contourCnt);
    bool mergeCoincidentVertices(VertexList* mesh, const Comparator&);
    void buildEdges(VertexList* contours, int contourCnt, VertexList* mesh, const Comparator&);
    Poly* contoursToPolys(VertexList* contours, int contourCnt);
    Poly* pathToPolys(float tolerance, const SkRect& clipBounds, int contourCnt);
    int64_t countPointsImpl(Poly* polys, SkPathFillType overrideFillType) const;
    void* polysToTrianglesImpl(Poly* polys, void* data, SkPathFillType overrideFillType);
    int pathToTriangles(float tolerance, const SkRect& clipBounds, GrEagerVertexAllocator*);

    constexpr static int kArenaChunkSize = 16 * 1024;
    SkArenaAlloc fAlloc{kArenaChunkSize};
    const SkPath fPath;
    bool fIsLinear = false;

    // Internal control knobs.
    bool fRoundVerticesToQuarterPixel = false;
    bool fEmitCoverage = false;
    bool fCullCollinearVertices = true;
    bool fDisallowSelfIntersection = false;
    BreadcrumbTriangleCollector* fBreadcrumbTriangles = nullptr;
};

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
#if TRIANGULATOR_LOGGING
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
#if TRIANGULATOR_LOGGING
    float   fID;                  // Identifier used for logging.
#endif
    bool isConnected() const { return this->fFirstEdgeAbove || this->fFirstEdgeBelow; }
};

struct GrTriangulator::VertexList {
    VertexList() : fHead(nullptr), fTail(nullptr) {}
    VertexList(Vertex* head, Vertex* tail) : fHead(head), fTail(tail) {}
    Vertex* fHead;
    Vertex* fTail;
    void insert(Vertex* v, Vertex* prev, Vertex* next);
    void append(Vertex* v) { insert(v, fTail, nullptr); }
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
    void prepend(Vertex* v) { insert(v, nullptr, fHead); }
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
             static_cast<double>(p.fX) * q.fY) {}
    double dist(const SkPoint& p) const { return fA * p.fX + fB * p.fY + fC; }
    Line operator*(double v) const { return Line(fA * v, fB * v, fC * v); }
    double magSq() const { return fA * fA + fB * fB; }
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
    bool intersect(const Line& other, SkPoint* point) const;
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
 * The coefficients of the line equation stored in double precision to avoid catastrophic
 * cancellation in the isLeftOf() and isRightOf() checks. Using doubles ensures that the result is
 * correct in float, since it's a polynomial of degree 2. The intersect() function, being
 * degree 5, is still subject to catastrophic cancellation. We deal with that by assuming its
 * output may be incorrect, and adjusting the mesh topology to match (see comment at the top of
 * this file).
 */

struct GrTriangulator::Edge {
    Edge(Vertex* top, Vertex* bottom, int winding, EdgeType type)
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
    EdgeType fType;
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
    double dist(const SkPoint& p) const { return fLine.dist(p); }
    bool isRightOf(Vertex* v) const { return fLine.dist(v->fPoint) < 0.0; }
    bool isLeftOf(Vertex* v) const { return fLine.dist(v->fPoint) > 0.0; }
    void recompute() { fLine = Line(fTop, fBottom); }
    void insertAbove(Vertex*, const Comparator&);
    void insertBelow(Vertex*, const Comparator&);
    void disconnect();
    bool intersect(const Edge& other, SkPoint* p, uint8_t* alpha = nullptr) const;
};

struct GrTriangulator::EdgeList {
    EdgeList() : fHead(nullptr), fTail(nullptr) {}
    Edge* fHead;
    Edge* fTail;
    void insert(Edge* edge, Edge* prev, Edge* next);
    void insert(Edge* edge, Edge* prev);
    void append(Edge* e) { insert(e, fTail, nullptr); }
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
    bool contains(Edge* edge) const { return edge->fLeft || edge->fRight || fHead == edge; }
};

struct GrTriangulator::MonotonePoly {
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
    void addEdge(Edge*);
    void* emit(bool emitCoverage, void* data);
    void* emitTriangle(Vertex* prev, Vertex* curr, Vertex* next, bool emitCoverage,
                       void* data) const;
};

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
#if TRIANGULATOR_LOGGING
        static int gID = 0;
        fID = gID++;
        TESS_LOG("*** created Poly %d\n", fID);
#endif
    }
    Poly* addEdge(Edge* e, Side side, SkArenaAlloc& alloc);
    void* emit(bool emitCoverage, void *data);
    Vertex* lastVertex() const { return fTail ? fTail->fLastEdge->fBottom : fFirstVertex; }
    Vertex* fFirstVertex;
    int fWinding;
    MonotonePoly* fHead;
    MonotonePoly* fTail;
    Poly* fNext;
    Poly* fPartner;
    int fCount;
#if TRIANGULATOR_LOGGING
    int fID;
#endif
};

struct GrTriangulator::Comparator {
    enum class Direction { kVertical, kHorizontal };
    Comparator(Direction direction) : fDirection(direction) {}
    bool sweep_lt(const SkPoint& a, const SkPoint& b) const;
    Direction fDirection;
};

#endif
