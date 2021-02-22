/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeHardwareTessellator_DEFINED
#define GrStrokeHardwareTessellator_DEFINED

#include "include/core/SkStrokeRec.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/tessellate/GrStrokeTessellateOp.h"
#include "src/gpu/tessellate/GrStrokeTessellateShader.h"

// Renders opaque, constant-color strokes by decomposing them into standalone tessellation patches.
// Each patch is either a "cubic" (single stroked bezier curve with butt caps) or a "join". Requires
// MSAA if antialiasing is desired.
class GrStrokeHardwareTessellator : public GrStrokeTessellator {
public:
    // We generate and store patch buffers in chunks. Normally there will only be one chunk, but in
    // rare cases the first can run out of space if too many cubics needed to be subdivided.
    struct PatchChunk {
        sk_sp<const GrBuffer> fPatchBuffer;
        int fPatchCount = 0;
        int fBasePatch;
    };

    GrStrokeHardwareTessellator(ShaderFlags shaderFlags, PathStrokeList* pathStrokeList,
                                int totalCombinedVerbCnt, const GrShaderCaps& shaderCaps)
            : GrStrokeTessellator(shaderFlags, std::move(pathStrokeList))
            , fTotalCombinedVerbCnt(totalCombinedVerbCnt) {
    }

    void prepare(GrMeshDrawOp::Target*, const SkMatrix&) override;
    void draw(GrOpFlushState*) const override;

private:
    // The combined number of path verbs from all paths in fPathStrokeList.
    const int fTotalCombinedVerbCnt;

    SkSTArray<1, PatchChunk> fPatchChunks;

    friend class GrOp;  // For ctor.

public:
    // This class is used to benchmark prepareBuffers().
    class TestingOnly_Benchmark;
};

#endif
