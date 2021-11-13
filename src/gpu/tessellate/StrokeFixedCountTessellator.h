/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_StrokeFixedCountTessellator_DEFINED
#define tessellate_StrokeFixedCountTessellator_DEFINED

#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrVertexChunkArray.h"
#include "src/gpu/tessellate/StrokeTessellator.h"

namespace skgpu {

// Renders strokes as fixed-count triangle strip instances. Any extra triangles not needed by the
// instance are emitted as degenerate triangles.
class StrokeFixedCountTessellator : public StrokeTessellator {
public:
    constexpr static float kMaxParametricSegments_pow4 = 32*32*32*32;  // 32^4
    constexpr static int8_t kMaxParametricSegments_log2 = 5;  // log2(32)

    StrokeFixedCountTessellator(PatchAttribs attribs) : StrokeTessellator(attribs) {}

    int prepare(GrMeshDrawTarget*,
                const SkMatrix& shaderMatrix,
                std::array<float,2> matrixMinMaxScales,
                PathStrokeList*,
                int totalCombinedVerbCnt) override;

#if SK_GPU_V1
    void draw(GrOpFlushState*) const override;
#endif

    // Initializes the fallback vertex buffer that should be bound when sk_VertexID is not
    // supported. Each vertex is a single float and each edge is composed of two vertices, so the
    // desired edge count in the buffer is presumed to be "bufferSize / (sizeof(float) * 2)". The
    // caller cannot draw more vertices than edgeCount * 2.
    static void InitializeVertexIDFallbackBuffer(VertexWriter vertexWriter, size_t bufferSize);

    // Returns the fixed number of edges that are always emitted with the given join type. If the
    // join is round, the caller needs to account for the additional radial edges on their own.
    // Specifically, each join always emits:
    //
    //   * Two colocated edges at the beginning (a full-width edge to seam with the preceding stroke
    //     and a half-width edge to begin the join).
    //
    //   * An extra edge in the middle for miter joins, or else a variable number of radial edges
    //     for round joins (the caller is responsible for counting radial edges from round joins).
    //
    //   * A half-width edge at the end of the join that will be colocated with the first
    //     (full-width) edge of the stroke.
    //
    constexpr static int NumFixedEdgesInJoin(SkPaint::Join joinType) {
        switch (joinType) {
            case SkPaint::kMiter_Join:
                return 4;
            case SkPaint::kRound_Join:
                // The caller is responsible for counting the variable number of middle, radial
                // segments on round joins.
                [[fallthrough]];
            case SkPaint::kBevel_Join:
                return 3;
        }
        SkUNREACHABLE;
    }

private:
    GrVertexChunkArray fInstanceChunks;
    int fFixedEdgeCount = 0;

    // Only used if sk_VertexID is not supported.
    sk_sp<const GrGpuBuffer> fVertexBufferIfNoIDSupport;
};

}  // namespace skgpu

#endif  // tessellate_StrokeFixedCountTessellator_DEFINED
