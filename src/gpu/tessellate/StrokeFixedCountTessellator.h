/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_StrokeFixedCountTessellator_DEFINED
#define tessellate_StrokeFixedCountTessellator_DEFINED

#include "src/gpu/GrVertexChunkArray.h"
#include "src/gpu/tessellate/StrokeTessellator.h"

namespace skgpu {

// Renders strokes as fixed-count triangle strip instances. Any extra triangles not needed by the
// instance are emitted as degenerate triangles.
class StrokeFixedCountTessellator : public StrokeTessellator {
public:
    StrokeFixedCountTessellator(const GrShaderCaps&,
                                ShaderFlags,
                                const SkMatrix&,
                                PathStrokeList*,
                                std::array<float, 2> matrixMinMaxScales,
                                const SkRect& strokeCullBounds);

    void prepare(GrMeshDrawTarget*, int totalCombinedVerbCnt) override;
#if SK_GPU_V1
    void draw(GrOpFlushState*) const override;
#endif

private:
    GrVertexChunkArray fInstanceChunks;
    int fFixedVertexCount = 0;

    // Only used if sk_VertexID is not supported.
    sk_sp<const GrGpuBuffer> fVertexBufferIfNoIDSupport;
};

}  // namespace skgpu

#endif  // tessellate_StrokeFixedCountTessellator_DEFINED
