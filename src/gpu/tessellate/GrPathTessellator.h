/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathTessellator_DEFINED
#define GrPathTessellator_DEFINED

#include "src/core/SkPathPriv.h"
#include "src/gpu/GrInnerFanTriangulator.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/GrVx.h"

class SkPath;
class GrMeshDrawTarget;
class GrGpuBuffer;
class GrOpFlushState;
class GrPathTessellationShader;

// Prepares GPU data for, and then draws a path's tessellated geometry. Depending on the subclass,
// the caller may or may not be required to draw the path's inner fan separately.
class GrPathTessellator {
public:
    using BreadcrumbTriangleList = GrInnerFanTriangulator::BreadcrumbTriangleList;

    struct PathDrawList {
        PathDrawList(const SkMatrix& pathMatrix, const SkPath& path, PathDrawList* next = nullptr)
                : fPathMatrix(pathMatrix), fPath(path), fNext(next) {}

        SkMatrix fPathMatrix;
        SkPath fPath;
        PathDrawList* fNext;

        struct Iter {
            void operator++() { fHead = fHead->fNext; }
            bool operator!=(const Iter& b) const { return fHead != b.fHead; }
            std::tuple<const SkMatrix&, const SkPath&> operator*() const {
                return {fHead->fPathMatrix, fHead->fPath};
            }
            const PathDrawList* fHead;
        };
        Iter begin() const { return {this}; }
        Iter end() const { return {nullptr}; }
    };

    virtual ~GrPathTessellator() {}

    const GrPathTessellationShader* shader() const { return fShader; }

    // Called before draw(). Prepares GPU buffers containing the geometry to tessellate.
    //
    // Each path's fPathMatrix in the list is applied on the CPU while the geometry is being written
    // out. This is a tool for batching, and is applied in addition to the shader's on-GPU matrix.
    virtual void prepare(GrMeshDrawTarget*,
                         const SkRect& cullBounds,
                         const PathDrawList&,
                         int totalCombinedPathVerbCnt) = 0;

    // Issues draw calls for the tessellated geometry. The caller is responsible for binding its
    // desired pipeline ahead of time.
    virtual void draw(GrOpFlushState*) const = 0;

    // Returns an upper bound on the number of combined edges there might be from all inner fans in
    // a PathDrawList.
    static int MaxCombinedFanEdgesInPathDrawList(int totalCombinedPathVerbCnt) {
        // Path fans might have an extra edge from an implicit kClose at the end, but they also
        // always begin with kMove. So the max possible number of edges in a single path is equal to
        // the number of verbs. Therefore, the max number of combined fan edges in a PathDrawList is
        // the number of combined path verbs in that PathDrawList.
        return totalCombinedPathVerbCnt;
    }

protected:
    GrPathTessellator(GrPathTessellationShader* shader) : fShader(shader) {}

    GrPathTessellationShader* fShader;
};

#endif
