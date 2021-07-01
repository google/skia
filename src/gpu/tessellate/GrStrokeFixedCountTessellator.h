/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeFixedCountTessellator_DEFINED
#define GrStrokeFixedCountTessellator_DEFINED

#include "src/gpu/GrVertexChunkArray.h"
#include "src/gpu/tessellate/GrStrokeTessellator.h"

// Renders strokes as fixed-count triangle strip instances. Any extra triangles not needed by the
// instance are emitted as degenerate triangles.
class GrStrokeFixedCountTessellator : public GrStrokeTessellator {
public:
    GrStrokeFixedCountTessellator(const GrShaderCaps&, ShaderFlags, const SkMatrix&,
                                  PathStrokeList*, std::array<float, 2> matrixMinMaxScales,
                                  const SkRect& strokeCullBounds);

    void prepare(GrMeshDrawTarget*, int totalCombinedVerbCnt) override;
    void draw(GrOpFlushState*) const override;

private:
    GrVertexChunkArray fInstanceChunks;
    int fFixedVertexCount = 0;

    // Only used if sk_VertexID is not supported.
    sk_sp<const GrGpuBuffer> fVertexBufferIfNoIDSupport;
};

#endif
