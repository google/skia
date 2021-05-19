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
    GrStrokeFixedCountTessellator(ShaderFlags shaderFlags, const SkMatrix& viewMatrix,
                                  PathStrokeList* pathStrokeList,
                                  std::array<float, 2> matrixMinMaxScales,
                                  const SkRect& strokeCullBounds)
            : GrStrokeTessellator(GrStrokeShader::Mode::kFixedCount, shaderFlags,
                                  viewMatrix, pathStrokeList, matrixMinMaxScales,
                                  strokeCullBounds) {
    }

    void prepare(GrMeshDrawOp::Target*, int totalCombinedVerbCnt) override;
    void draw(GrOpFlushState*) const override;

private:
    GrVertexChunkArray fInstanceChunks;
    int fFixedVertexCount = 0;
};

#endif
