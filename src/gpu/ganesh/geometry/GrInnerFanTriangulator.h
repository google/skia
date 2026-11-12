/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrInnerFanTriangulator_DEFINED
#define GrInnerFanTriangulator_DEFINED

#include "include/core/SkRect.h"
#include "src/gpu/ganesh/geometry/GrTriangulator.h"

#include <utility>

class GrEagerVertexAllocator;
class SkArenaAlloc;
class SkPath;

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

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

    [[nodiscard]] bool pathToTriangles(GrEagerVertexAllocator* vertexAlloc,
                                       BreadcrumbTriangleList* breadcrumbList,
                                       bool* isLinear, int* vertexCount) {
        auto [polys, success] = this->pathToPolys(breadcrumbList, isLinear);
        if (!success) {
            *vertexCount = 0;
            return false;
        }
        return this->polysToTriangles(polys, vertexAlloc, breadcrumbList, vertexCount);
    }

    std::tuple<Poly*, bool> pathToPolys(BreadcrumbTriangleList* breadcrumbList, bool* isLinear) {
        auto [ polys, success ] = this->GrTriangulator::pathToPolys(0, SkRect::MakeEmpty(),
                                                                    isLinear);
        if (!success) {
            return {nullptr, false};
        }
        breadcrumbList->concat(std::move(fBreadcrumbList));
        return {polys, success};
    }

    [[nodiscard]] bool polysToTriangles(Poly* polys,
                                        GrEagerVertexAllocator* vertexAlloc,
                                        BreadcrumbTriangleList* breadcrumbList,
                                        int* vertexCount) const {
        bool result = this->GrTriangulator::polysToTriangles(polys, vertexAlloc, vertexCount);
        breadcrumbList->concat(std::move(fBreadcrumbList));
        return result;
    }
};

#else

// Stub out GrInnerFanTriangulator::BreadcrumbTriangleList for function declarations.
namespace GrInnerFanTriangulator {
    struct BreadcrumbTriangleList {
        BreadcrumbTriangleList() = delete;
    };
}

#endif // SK_ENABLE_OPTIMIZE_SIZE

#endif // GrInnerFanTriangulator_DEFINED
