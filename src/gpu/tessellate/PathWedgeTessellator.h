/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_PathWedgeTessellator_DEFINED
#define tessellate_PathWedgeTessellator_DEFINED

#include "src/gpu/tessellate/PathTessellator.h"

namespace skgpu {

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

    int patchPreallocCount(int totalCombinedPathVerbCnt) const final;

    void writePatches(PatchWriter&,
                      int maxTessellationSegments,
                      const SkMatrix& shaderMatrix,
                      const PathDrawList&) final;

    // Size of the vertex buffer to use when rendering with a fixed count shader.
    constexpr static int FixedVertexBufferSize(int maxFixedResolveLevel) {
        return (((1 << maxFixedResolveLevel) + 1) + 1/*fan vertex*/) * sizeof(SkPoint);
    }

    // Writes the vertex buffer to use when rendering with a fixed count shader.
    static void WriteFixedVertexBuffer(VertexWriter, size_t bufferSize);

    // Size of the index buffer to use when rendering with a fixed count shader.
    constexpr static int FixedIndexBufferSize(int maxFixedResolveLevel) {
        return (NumCurveTrianglesAtResolveLevel(maxFixedResolveLevel) + 1/*fan triangle*/) *
               3 * sizeof(uint16_t);
    }

    // Writes the index buffer to use when rendering with a fixed count shader.
    static void WriteFixedIndexBuffer(VertexWriter vertexWriter, size_t bufferSize);

#if SK_GPU_V1
    void prepareFixedCountBuffers(GrMeshDrawTarget*) final;

    void drawTessellated(GrOpFlushState*) const final;
    void drawFixedCount(GrOpFlushState*) const final;
#endif
};

}  // namespace skgpu

#endif  // tessellate_PathWedgeTessellator_DEFINED
