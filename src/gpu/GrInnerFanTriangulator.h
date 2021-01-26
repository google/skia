/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrInnerFanTriangulator_DEFINED
#define GrInnerFanTriangulator_DEFINED

#include "src/gpu/GrTriangulator.h"

// Triangulates the inner polygon(s) of a path (i.e., the triangle fan for a Redbook rendering
// method). When combined with the outer curves and breadcrumb triangles, these produce a complete
// path. If a breadcrumbCollector is not provided, pathToPolys fails upon self intersection.
class GrInnerFanTriangulator : private GrTriangulator {
public:
    using GrTriangulator::BreadcrumbTriangleList;

    GrInnerFanTriangulator(const SkPath& path, SkArenaAlloc* alloc)
            : GrTriangulator(path, alloc) {
        fCullCollinearVertices = false;
        fBreadcrumbTriangleList = fAlloc->make<BreadcrumbTriangleList>();
    }

    int pathToTriangles(GrEagerVertexAllocator* vertexAlloc,
                        const BreadcrumbTriangleList** breadcrumbTriangleList, bool* isLinear) {
        return this->polysToTriangles(this->pathToPolys(isLinear), vertexAlloc,
                                      breadcrumbTriangleList);
    }

    Poly* pathToPolys(bool* isLinear) {
        return this->GrTriangulator::pathToPolys(0, SkRect::MakeEmpty(), isLinear);
    }

    int polysToTriangles(Poly* polys, GrEagerVertexAllocator* vertexAlloc,
                         const BreadcrumbTriangleList** breadcrumbTriangleList) {
        int vertexCount = this->GrTriangulator::polysToTriangles(polys, vertexAlloc);
        *breadcrumbTriangleList = fBreadcrumbTriangleList;
        return vertexCount;
    }
};

#endif
