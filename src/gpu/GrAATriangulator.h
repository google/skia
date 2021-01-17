/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAATriangulator_DEFINED
#define GrAATriangulator_DEFINED

#include "src/gpu/GrTriangulator.h"

// Triangulates the given path in device space with a mesh of alpha ramps for antialiasing.
class GrAATriangulator : public GrTriangulator {
public:
    static int PathToTriangles(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds,
                               GrEagerVertexAllocator* vertexAllocator) {
        GrAATriangulator aaTriangulator(path);
        aaTriangulator.fRoundVerticesToQuarterPixel = true;
        aaTriangulator.fEmitCoverage = true;
        return  aaTriangulator.pathToTriangles(tolerance, clipBounds, vertexAllocator);
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
    GrAATriangulator(const SkPath& path) : GrTriangulator(path) {}

    // For screenspace antialiasing, the algorithm is modified as follows:
    //
    // Run steps 1-5 above to produce polygons.
    // 5b) Apply fill rules to extract boundary contours from the polygons:
    void extractBoundary(EdgeList* boundary, Edge* e);
    void extractBoundaries(const VertexList& inMesh, VertexList* innerVertices, const Comparator&);

    // 5c) Simplify boundaries to remove "pointy" vertices that cause inversions:
    void simplifyBoundary(EdgeList* boundary, const Comparator&);

    // 5d) Displace edges by half a pixel inward and outward along their normals. Intersect to find
    //     new vertices, and set zero alpha on the exterior and one alpha on the interior. Build a
    //     new antialiased mesh from those vertices:
    void strokeBoundary(EdgeList* boundary, VertexList* innerMesh, const Comparator&);

    // Run steps 3-6 above on the new mesh, and produce antialiased triangles.
    Poly* tessellate(const VertexList& mesh, const Comparator&) override;
    int64_t countPoints(Poly* polys) const override;
    void* polysToTriangles(Poly* polys, void* data) override;

    // Additional helpers and driver functions.
    void makeEvent(SSEdge*, EventList* events);
    void makeEvent(SSEdge*, Vertex* v, SSEdge* other, Vertex* dest, EventList* events,
                   const Comparator&);
    void connectPartners(VertexList* mesh, const Comparator&);
    void removeNonBoundaryEdges(const VertexList& mesh);
    void connectSSEdge(Vertex* v, Vertex* dest, const Comparator&);
    bool collapseOverlapRegions(VertexList* mesh, const Comparator&, EventComparator comp);

    VertexList fOuterMesh;
};

#endif
