/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_PathCurveTessellator_DEFINED
#define tessellate_PathCurveTessellator_DEFINED

#include "src/gpu/tessellate/PathTessellator.h"

namespace skgpu {

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

    int patchPreallocCount(int totalCombinedPathVerbCnt) const final;

    void writePatches(PatchWriter& patchWriter,
                      int maxTessellationSegments,
                      const SkMatrix& shaderMatrix,
                      const PathDrawList& pathDrawList) final;

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
    void prepareFixedCountBuffers(GrMeshDrawTarget*) final;

    void drawTessellated(GrOpFlushState*) const final;
    void drawFixedCount(GrOpFlushState*) const final;

    // Draws a 4-point instance for each patch. This method is used for drawing convex hulls over
    // each cubic with GrFillCubicHullShader. The caller is responsible for binding its desired
    // pipeline ahead of time.
    void drawHullInstances(GrOpFlushState*, sk_sp<const GrGpuBuffer> vertexBufferIfNeeded) const;
#endif
};

}  // namespace skgpu

#endif  // tessellate_PathCurveTessellator_DEFINED
