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

#define TRIANGULATOR_WIREFRAME 0

/**
 * Provides utility functions for converting paths to a collection of triangles.
 */
class GrTriangulator {
public:
    static int PathToTriangles(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
                               GrEagerVertexAllocator* vertexAllocator, bool* isLinear) {
        GrTriangulator triangulator(path);
        int count = triangulator.pathToTriangles(tolerance, clipBounds, vertexAllocator,
                                                 path.getFillType());
        *isLinear = triangulator.fIsLinear;
        return count;
    }

    static int PathToAATriangles(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
                                 GrEagerVertexAllocator* vertexAllocator, bool* isLinear) {
        GrTriangulator triangulator(path);
        triangulator.fRoundVerticesToQuarterPixel = true;
        triangulator.fEmitCoverage = true;
        int count = triangulator.pathToTriangles(tolerance, clipBounds, vertexAllocator,
                                                 SkPathFillType::kWinding);
        *isLinear = triangulator.fIsLinear;
        return count;
    }

    static int TriangulateSimpleInnerPolygons(const SkPath& path,
                                              GrEagerVertexAllocator* vertexAllocator,
                                              bool *isLinear) {
        GrTriangulator triangulator(path);
        triangulator.fCullCollinearVertices = false;
        triangulator.fSimpleInnerPolygons = true;
        int count = triangulator.pathToTriangles(0, SkRect::MakeEmpty(), vertexAllocator,
                                                 path.getFillType());
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

private:
    GrTriangulator(const SkPath& path) : fPath(path) {}

    // There are six stages to the basic algorithm:
    //
    // 1) Linearize the path contours into piecewise linear segments:
    void pathToContours(float tolerance, const SkRect& clipBounds, VertexList* contours);

    // 2) Build a mesh of edges connecting the vertices:
    void contoursToMesh(VertexList* contours, int contourCnt, VertexList* mesh, const Comparator&);

    // 3) Sort the vertices in Y (and secondarily in X) (merge_sort()).
    static void SortMesh(VertexList* vertices, const Comparator&);

    // 4) Simplify the mesh by inserting new vertices at intersecting edges:
    enum class SimplifyResult {
        kAlreadySimple,
        kFoundSelfIntersection,
        kAbort
    };

    SimplifyResult simplify(VertexList* mesh, const Comparator&);

    // 5) Tessellate the simplified mesh into monotone polygons:
    Poly* tessellate(const VertexList& vertices);

    // 6) Triangulate the monotone polygons directly into a vertex buffer:
    void* polysToTriangles(Poly* polys, void* data, SkPathFillType overrideFillType);

    // For screenspace antialiasing, the algorithm is modified as follows:
    //
    // Run steps 1-5 above to produce polygons.
    // 5b) Apply fill rules to extract boundary contours from the polygons (extract_boundaries()).
    // 5c) Simplify boundaries to remove "pointy" vertices that cause inversions
    //     (simplify_boundary()).
    // 5d) Displace edges by half a pixel inward and outward along their normals. Intersect to find
    //     new vertices, and set zero alpha on the exterior and one alpha on the interior. Build a
    //     new antialiased mesh from those vertices (stroke_boundary()).
    // Run steps 3-6 above on the new mesh, and produce antialiased triangles.
    //
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
    //    edges (merge_collinear_edges()).
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
    void appendPointToContour(const SkPoint& p, VertexList* contour);
    void appendQuadraticToContour(const SkPoint[3], SkScalar toleranceSqd, VertexList* contour);
    void generateCubicPoints(const SkPoint&, const SkPoint&, const SkPoint&, const SkPoint&,
                             SkScalar tolSqd, VertexList* contour, int pointsLeft);
    bool splitEdge(Edge* edge, Vertex* v, EdgeList* activeEdges, Vertex** current,
                   const Comparator&);
    bool intersectEdgePair(Edge* left, Edge* right, EdgeList* activeEdges, Vertex** current,
                           const Comparator&);
    bool checkForIntersection(Edge* left, Edge* right, EdgeList* activeEdges, Vertex** current,
                              VertexList* mesh, const Comparator&);
    void sanitizeContours(VertexList* contours, int contourCnt);
    bool mergeCoincidentVertices(VertexList* mesh, const Comparator&);
    void buildEdges(VertexList* contours, int contourCnt, VertexList* mesh, const Comparator&);
    Poly* contoursToPolys(VertexList* contours, int contourCnt, VertexList* outerMesh);
    Poly* pathToPolys(float tolerance, const SkRect& clipBounds, int contourCnt,
                      VertexList* outerMesh);
    int pathToTriangles(float tolerance, const SkRect& clipBounds, GrEagerVertexAllocator*,
                        SkPathFillType overrideFillType);

    constexpr static int kArenaChunkSize = 16 * 1024;
    SkArenaAlloc fAlloc{kArenaChunkSize};
    const SkPath fPath;
    bool fIsLinear = false;

    // Flags.
    bool fRoundVerticesToQuarterPixel = false;
    bool fEmitCoverage = false;
    bool fCullCollinearVertices = true;
    bool fSimpleInnerPolygons = false;
};

#endif
