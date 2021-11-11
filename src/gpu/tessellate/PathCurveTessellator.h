/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_PathCurveTessellator_DEFINED
#define tessellate_PathCurveTessellator_DEFINED

#include "src/gpu/GrVertexChunkArray.h"
#include "src/gpu/tessellate/PathTessellator.h"

class GrCaps;
class GrPipeline;

namespace skgpu {

// Draws an array of "outer curve" patches and, optionally, inner fan triangles for
// GrCubicTessellateShader. Each patch is an independent 4-point curve, representing either a cubic
// or a conic. Quadratics are converted to cubics and triangles are converted to conics with w=Inf.
class PathCurveTessellator final : public PathTessellator {
public:
    // If DrawInnerFan is kNo, this class only emits the path's outer curves. In that case the
    // caller is responsible to handle the path's inner fan.
    enum class DrawInnerFan : bool {
        kNo = false,
        kYes
    };

    static PathCurveTessellator* Make(SkArenaAlloc* arena,
                                      DrawInnerFan drawInnerFan,
                                      bool infinitySupport,
                                      PatchAttribs attribs = PatchAttribs::kNone) {
        return arena->make<PathCurveTessellator>(drawInnerFan, infinitySupport, attribs);
    }

    PathCurveTessellator(DrawInnerFan drawInnerFan,
                         bool infinitySupport,
                         PatchAttribs attribs = PatchAttribs::kNone)
            : PathTessellator(infinitySupport, attribs)
            , fDrawInnerFan(drawInnerFan != DrawInnerFan::kNo) {}

    void prepare(GrMeshDrawTarget* target,
                 int maxTessellationSegments,
                 const SkMatrix& shaderMatrix,
                 const PathDrawList& pathDrawList,
                 int totalCombinedPathVerbCnt) final {
        this->prepare(target, maxTessellationSegments, shaderMatrix, pathDrawList,
                      totalCombinedPathVerbCnt, nullptr);
    }

    // Implements PathTessellator::prepare(), also sending an additional list of breadcrumb
    // triangles to the GPU. The breadcrumb triangles are implemented as conics with w=Infinity.
    //
    // ALSO NOTE: The breadcrumb triangles do not have a matrix. These need to be pre-transformed by
    // the caller if a CPU-side transformation is desired.
    void prepare(GrMeshDrawTarget*,
                 int maxTessellationSegments,
                 const SkMatrix& shaderMatrix,
                 const PathDrawList&,
                 int totalCombinedPathVerbCnt,
                 const BreadcrumbTriangleList*);

    // Size of the vertex buffer to use when rendering with a fixed count shader.
    constexpr static int FixedVertexBufferSize(int maxFixedResolveLevel) {
        return ((1 << maxFixedResolveLevel) + 1) * sizeof(SkPoint);
    }

    // Writes the vertex buffer to use when rendering with a fixed count shader.
    static void WriteFixedVertexBuffer(VertexWriter, size_t bufferSize);

    // Size of the index buffer to use when rendering with a fixed count shader.
    constexpr static int FixedIndexBufferSize(int maxFixedResolveLevel) {
        return NumCurveTrianglesAtResolveLevel(maxFixedResolveLevel) * 3 * sizeof(uint16_t);
    }

    // Writes the index buffer to use when rendering with a fixed count shader.
    static void WriteFixedIndexBuffer(VertexWriter vertexWriter, size_t bufferSize) {
        WriteFixedIndexBufferBaseIndex(std::move(vertexWriter), bufferSize, 0);
    }

    static void WriteFixedIndexBufferBaseIndex(VertexWriter, size_t bufferSize, uint16_t baseIndex);

#if SK_GPU_V1
    void prepareFixedCountBuffers(GrResourceProvider*) final;

    void drawTessellated(GrOpFlushState*) const final;
    void drawFixedCount(GrOpFlushState*) const final;

    // Draws a 4-point instance for each patch. This method is used for drawing convex hulls over
    // each cubic with GrFillCubicHullShader. The caller is responsible for binding its desired
    // pipeline ahead of time.
    void drawHullInstances(GrOpFlushState*, sk_sp<const GrGpuBuffer> vertexBufferIfNeeded) const;
#endif

private:
    const bool fDrawInnerFan;
    GrVertexChunkArray fVertexChunkArray;
};

}  // namespace skgpu

#endif  // tessellate_PathCurveTessellator_DEFINED
