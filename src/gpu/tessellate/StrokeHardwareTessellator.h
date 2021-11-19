/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_StrokeHardwareTessellator_DEFINED
#define tessellate_StrokeHardwareTessellator_DEFINED

#include "src/gpu/tessellate/StrokeTessellator.h"

namespace skgpu {

// Renders opaque, constant-color strokes by decomposing them into standalone tessellation patches.
// Each patch is either a "cubic" (single stroked bezier curve with butt caps) or a "join". Requires
// MSAA if antialiasing is desired.
class StrokeHardwareTessellator final : public StrokeTessellator {
public:
    StrokeHardwareTessellator(PatchAttribs attribs, int maxTessellationSegments)
            : StrokeTessellator(attribs), fMaxTessellationSegments(maxTessellationSegments) {}

    int patchPreallocCount(int totalCombinedStrokeVerbCnt) const final;

    int writePatches(PatchWriter&,
                     const SkMatrix& shaderMatrix,
                     std::array<float,2> matrixMinMaxScales,
                     PathStrokeList*) final;

#if SK_GPU_V1
    int prepare(GrMeshDrawTarget*,
                const SkMatrix& shaderMatrix,
                std::array<float,2> matrixMinMaxScales,
                PathStrokeList*,
                int totalCombinedStrokeVerbCnt) final;

    void draw(GrOpFlushState*) const final;
#endif

private:
    const int fMaxTessellationSegments;
};

}  // namespace skgpu

#endif  // tessellate_StrokeHardwareTessellator_DEFINED
