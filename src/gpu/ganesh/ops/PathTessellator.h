/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PathTessellator_DEFINED
#define PathTessellator_DEFINED

#include "src/core/SkArenaAlloc.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrVertexChunkArray.h"
#include "src/gpu/ganesh/geometry/GrInnerFanTriangulator.h"
#include "src/gpu/tessellate/FixedCountBufferUtils.h"
#include "src/gpu/tessellate/Tessellation.h"

class GrMeshDrawTarget;
class GrOpFlushState;
class SkPath;

namespace skgpu::v1 {

// Prepares GPU data for, and then draws a path's tessellated geometry. Depending on the subclass,
// the caller may or may not be required to draw the path's inner fan separately.
class PathTessellator {
public:

    struct PathDrawList {
        PathDrawList(const SkMatrix& pathMatrix,
                     const SkPath& path,
                     const SkPMColor4f& color,
                     PathDrawList* next = nullptr)
                : fPathMatrix(pathMatrix), fPath(path), fColor(color), fNext(next) {}

        SkMatrix fPathMatrix;
        SkPath fPath;
        SkPMColor4f fColor;
        PathDrawList* fNext;

        struct Iter {
            void operator++() { fHead = fHead->fNext; }
            bool operator!=(const Iter& b) const { return fHead != b.fHead; }
            std::tuple<const SkMatrix&, const SkPath&, const SkPMColor4f&> operator*() const {
                return {fHead->fPathMatrix, fHead->fPath, fHead->fColor};
            }
            const PathDrawList* fHead;
        };
        Iter begin() const { return {this}; }
        Iter end() const { return {nullptr}; }
    };

    virtual ~PathTessellator() {}

    PatchAttribs patchAttribs() const { return fAttribs; }

    // Initializes the internal vertex and index buffers required for drawFixedCount().
    virtual void prepareFixedCountBuffers(GrMeshDrawTarget*) = 0;

    // Called before draw(). Prepares GPU buffers containing the geometry to tessellate.
    virtual void prepare(GrMeshDrawTarget* target,
                         int maxTessellationSegments,
                         const SkMatrix& shaderMatrix,
                         const PathDrawList& pathDrawList,
                         int totalCombinedPathVerbCnt,
                         bool willUseTessellationShaders) = 0;

    // Issues hardware tessellation draw calls over the patches. The caller is responsible for
    // binding its desired pipeline ahead of time.
    virtual void drawTessellated(GrOpFlushState*) const = 0;

    // Issues fixed-count instanced draw calls over the patches. The caller is responsible for
    // binding its desired pipeline ahead of time.
    virtual void drawFixedCount(GrOpFlushState*) const = 0;

    void draw(GrOpFlushState* flushState, bool willUseTessellationShaders) {
        if (willUseTessellationShaders) {
            this->drawTessellated(flushState);
        } else {
            this->drawFixedCount(flushState);
        }
    }

protected:
    PathTessellator(bool infinitySupport, PatchAttribs attribs) : fAttribs(attribs) {
        if (!infinitySupport) {
            fAttribs |= PatchAttribs::kExplicitCurveType;
        }
    }

    void updateResolveLevel(int resolveLevel) {
        // We should already chopped curves to make sure none needed a higher resolveLevel than
        // kMaxFixedResolveLevel.
        fFixedResolveLevel = SkTPin(resolveLevel, fFixedResolveLevel, skgpu::kMaxFixedResolveLevel);
    }

    PatchAttribs fAttribs;

    // Calculated during prepare(). If using fixed count, this is the resolveLevel to use on our
    // instanced draws. 2^resolveLevel == numSegments.
    int fFixedResolveLevel = 0;

    GrVertexChunkArray fVertexChunkArray;

    // If using fixed-count rendering, these are the vertex and index buffers.
    sk_sp<const GrGpuBuffer> fFixedVertexBuffer;
    sk_sp<const GrGpuBuffer> fFixedIndexBuffer;
};

// Draws an array of "outer curve" patches. Each patch is an independent 4-point curve, representing
// either a cubic or a conic. Quadratics are converted to cubics and triangles are converted to
// conics with w=Inf.
class PathCurveTessellator final : public PathTessellator {
public:
    static PathCurveTessellator* Make(SkArenaAlloc* arena,
                                      bool infinitySupport,
                                      PatchAttribs attribs = PatchAttribs::kNone) {
        return arena->make<PathCurveTessellator>(infinitySupport, attribs);
    }

    PathCurveTessellator(bool infinitySupport,
                         PatchAttribs attribs = PatchAttribs::kNone)
            : PathTessellator(infinitySupport, attribs) {}

    void prepareWithTriangles(GrMeshDrawTarget* target,
                              int maxTessellationSegments,
                              const SkMatrix& shaderMatrix,
                              GrInnerFanTriangulator::BreadcrumbTriangleList* extraTriangles,
                              const PathDrawList& pathDrawList,
                              int totalCombinedPathVerbCnt,
                              bool willUseTessellationShaders);

    void prepare(GrMeshDrawTarget* target,
                 int maxTessellationSegments,
                 const SkMatrix& shaderMatrix,
                 const PathDrawList& pathDrawList,
                 int totalCombinedPathVerbCnt,
                 bool willUseTessellationShaders) final {
        this->prepareWithTriangles(target,
                                   maxTessellationSegments,
                                   shaderMatrix,
                                   nullptr, // no extra triangles by default
                                   pathDrawList,
                                   totalCombinedPathVerbCnt,
                                   willUseTessellationShaders);
    }

    void prepareFixedCountBuffers(GrMeshDrawTarget*) final;

    void drawTessellated(GrOpFlushState*) const final;
    void drawFixedCount(GrOpFlushState*) const final;

    // Draws a 4-point instance for each patch. This method is used for drawing convex hulls over
    // each cubic with GrFillCubicHullShader. The caller is responsible for binding its desired
    // pipeline ahead of time.
    void drawHullInstances(GrOpFlushState*, sk_sp<const GrGpuBuffer> vertexBufferIfNeeded) const;
};

// Prepares an array of "wedge" patches. A wedge is an independent, 5-point closed contour
// consisting of 4 control points plus an anchor point fanning from the center of the curve's
// resident contour. A wedge can be either a cubic or a conic. Quadratics and lines are converted to
// cubics. Once stencilled, these wedges alone define the complete path.
class PathWedgeTessellator final : public PathTessellator {
public:
    static PathWedgeTessellator* Make(SkArenaAlloc* arena,
                                      bool infinitySupport,
                                      PatchAttribs attribs = PatchAttribs::kNone) {
        return arena->make<PathWedgeTessellator>(infinitySupport, attribs);
    }

    PathWedgeTessellator(bool infinitySupport, PatchAttribs attribs = PatchAttribs::kNone)
            : PathTessellator(infinitySupport, attribs) {
        fAttribs |= PatchAttribs::kFanPoint;
    }

    void prepare(GrMeshDrawTarget* target,
                 int maxTessellationSegments,
                 const SkMatrix& shaderMatrix,
                 const PathDrawList& pathDrawList,
                 int totalCombinedPathVerbCnt,
                 bool willUseTessellationShaders) final;

    void prepareFixedCountBuffers(GrMeshDrawTarget*) final;

    void drawTessellated(GrOpFlushState*) const final;
    void drawFixedCount(GrOpFlushState*) const final;
};

}  // namespace skgpu::v1

#endif  // PathTessellator_DEFINED
