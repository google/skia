/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrInnerFanTriangulator_DEFINED
#define GrInnerFanTriangulator_DEFINED

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

#include "src/gpu/ganesh/geometry/GrTriangulator.h"

// Triangulates the inner polygon(s) of a path (i.e., the triangle fan for a Redbook rendering
// method). When combined with the outer curves and breadcrumb triangles, these produce a complete
// path. If a breadcrumbCollector is not provided, pathToPolys fails upon self intersection.
class GrInnerFanTriangulator : private GrTriangulator {
public:
    using GrTriangulator::BreadcrumbTriangleList;

    GrInnerFanTriangulator(const SkPath& path, SkArenaAlloc* alloc)
            : GrTriangulator(path, alloc) {
        fPreserveCollinearVertices = true;
        fCollectBreadcrumbTriangles = true;
    }

    int pathToTriangles(GrEagerVertexAllocator* vertexAlloc, BreadcrumbTriangleList* breadcrumbList,
                        bool* isLinear) {
        Poly* polys = this->pathToPolys(breadcrumbList, isLinear);
        return this->polysToTriangles(polys, vertexAlloc, breadcrumbList);
    }

    Poly* pathToPolys(BreadcrumbTriangleList* breadcrumbList, bool* isLinear) {
        auto [ polys, success ] = this->GrTriangulator::pathToPolys(0, SkRect::MakeEmpty(),
                                                                    isLinear);
        if (!success) {
            return nullptr;
        }
        breadcrumbList->concat(std::move(fBreadcrumbList));
        return polys;
    }

    int polysToTriangles(Poly* polys, GrEagerVertexAllocator* vertexAlloc,
                         BreadcrumbTriangleList* breadcrumbList) const {
        int vertexCount = this->GrTriangulator::polysToTriangles(polys, vertexAlloc);
        breadcrumbList->concat(std::move(fBreadcrumbList));
        return vertexCount;
    }
};

#else

// Stub out GrInnerFanTriangulator::BreadcrumbTriangleList for function declarations.
namespace GrInnerFanTriangulator {
    struct BreadcrumbTriangleList {
        BreadcrumbTriangleList() = delete;
    };
};

#endif // SK_ENABLE_OPTIMIZE_SIZE

#endif // GrInnerFanTriangulator_DEFINED
