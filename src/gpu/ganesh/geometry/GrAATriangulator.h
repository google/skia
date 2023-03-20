/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAATriangulator_DEFINED
#define GrAATriangulator_DEFINED

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

#include "src/gpu/ganesh/geometry/GrTriangulator.h"

// Triangulates the given path in device space with a mesh of alpha ramps for antialiasing.
class GrAATriangulator : private GrTriangulator {
public:
    static int PathToAATriangles(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
                                 GrEagerVertexAllocator* vertexAllocator) {
        SkArenaAlloc alloc(kArenaDefaultChunkSize);
        GrAATriangulator aaTriangulator(path, &alloc);
        aaTriangulator.fRoundVerticesToQuarterPixel = true;
        aaTriangulator.fEmitCoverage = true;
        bool isLinear;
        auto [ polys, success ] = aaTriangulator.pathToPolys(tolerance, clipBounds, &isLinear);
        if (!success) {
            return 0;
        }
        return aaTriangulator.polysToAATriangles(polys, vertexAllocator);
    }

    // Structs used by GrAATriangulator internals.
    struct SSEdge;
    struct EventList;
    struct Event {
        Event(SSEdge* edge, const SkPoint& point, uint8_t alpha)
                : fEdge(edge), fPoint(point), fAlpha(alpha) {}
        SSEdge* fEdge;
        SkPoint fPoint;
        uint8_t fAlpha;
        void apply(VertexList* mesh, const Comparator&, EventList* events, GrAATriangulator*);
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

private:
    GrAATriangulator(const SkPath& path, SkArenaAlloc* alloc) : GrTriangulator(path, alloc) {}

    // For screenspace antialiasing, the algorithm is modified as follows:
    //
    // Run steps 1-5 above to produce polygons.
    // 5b) Apply fill rules to extract boundary contours from the polygons:
    void extractBoundary(EdgeList* boundary, Edge* e) const;
    void extractBoundaries(const VertexList& inMesh, VertexList* innerVertices,
                           const Comparator&);

    // 5c) Simplify boundaries to remove "pointy" vertices that cause inversions:
    void simplifyBoundary(EdgeList* boundary, const Comparator&);

    // 5d) Displace edges by half a pixel inward and outward along their normals. Intersect to find
    //     new vertices, and set zero alpha on the exterior and one alpha on the interior. Build a
    //     new antialiased mesh from those vertices:
    void strokeBoundary(EdgeList* boundary, VertexList* innerMesh, const Comparator&);

    // Run steps 3-6 above on the new mesh, and produce antialiased triangles.
    std::tuple<Poly*, bool> tessellate(const VertexList& mesh, const Comparator&) override;
    int polysToAATriangles(Poly*, GrEagerVertexAllocator*) const;

    // Additional helpers and driver functions.
    void makeEvent(SSEdge*, EventList* events) const;
    void makeEvent(SSEdge*, Vertex* v, SSEdge* other, Vertex* dest, EventList* events,
                   const Comparator&) const;
    void connectPartners(VertexList* mesh, const Comparator&);
    void removeNonBoundaryEdges(const VertexList& mesh) const;
    void connectSSEdge(Vertex* v, Vertex* dest, const Comparator&);
    bool collapseOverlapRegions(VertexList* mesh, const Comparator&, EventComparator comp);

    // FIXME: fOuterMesh should be plumbed through function parameters instead.
    mutable VertexList fOuterMesh;
};

#endif // SK_ENABLE_OPTIMIZE_SIZE

#endif // GrAATriangulator_DEFINED
