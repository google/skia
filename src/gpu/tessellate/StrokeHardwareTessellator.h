/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_StrokeHardwareTessellator_DEFINED
#define tessellate_StrokeHardwareTessellator_DEFINED

#include "src/gpu/GrVertexChunkArray.h"
#include "src/gpu/tessellate/StrokeTessellator.h"

namespace skgpu {

// Renders opaque, constant-color strokes by decomposing them into standalone tessellation patches.
// Each patch is either a "cubic" (single stroked bezier curve with butt caps) or a "join". Requires
// MSAA if antialiasing is desired.
class StrokeHardwareTessellator : public StrokeTessellator {
public:
    StrokeHardwareTessellator(PatchAttribs attribs) : StrokeTessellator(attribs) {}

    int prepare(GrMeshDrawTarget*,
                const SkMatrix& shaderMatrix,
                std::array<float,2> matrixMinMaxScales,
                PathStrokeList*,
                int totalCombinedVerbCnt) override;
#if SK_GPU_V1
    void draw(GrOpFlushState*) const override;
#endif

private:
    GrVertexChunkArray fPatchChunks;
};

}  // namespace skgpu

#endif  // tessellate_StrokeHardwareTessellator_DEFINED
