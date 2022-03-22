/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef StrokeTessellator_DEFINED
#define StrokeTessellator_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/SkColorData.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrVertexChunkArray.h"
#include "src/gpu/tessellate/FixedCountBufferUtils.h"
#include "src/gpu/tessellate/Tessellation.h"

class GrGpuBuffer;
class GrMeshDrawTarget;
class GrOpFlushState;

namespace skgpu::v1 {

// Prepares GPU data for, and then draws a stroke's tessellated geometry.
class StrokeTessellator {
public:
    struct PathStrokeList {
        PathStrokeList(const SkPath& path, const SkStrokeRec& stroke, const SkPMColor4f& color)
                : fPath(path), fStroke(stroke), fColor(color) {}
        SkPath fPath;
        SkStrokeRec fStroke;
        SkPMColor4f fColor;
        PathStrokeList* fNext = nullptr;
    };

    StrokeTessellator(PatchAttribs attribs) : fAttribs(attribs | PatchAttribs::kJoinControlPoint) {}

    // Called before draw(). Prepares GPU buffers containing the geometry to tessellate.
    //
    // Returns the fixed number of edges the tessellator will draw per patch, if using fixed-count
    // rendering, otherwise 0.
    virtual int prepare(GrMeshDrawTarget*,
                        const SkMatrix& shaderMatrix,
                        std::array<float,2> matrixMinMaxScales,
                        PathStrokeList*,
                        int totalCombinedStrokeVerbCnt) = 0;

    // Issues draw calls for the tessellated stroke. The caller is responsible for creating and
    // binding a pipeline that uses this class's shader() before calling draw().
    virtual void draw(GrOpFlushState*) const = 0;

    virtual ~StrokeTessellator() {}

protected:
    const PatchAttribs fAttribs;

    GrVertexChunkArray fVertexChunkArray;
};

// Renders strokes as fixed-count triangle strip instances. Any extra triangles not needed by the
// instance are emitted as degenerate triangles.
class StrokeFixedCountTessellator final : public StrokeTessellator {
public:
    constexpr static int8_t kMaxParametricSegments_log2 =
            SkNextLog2_portable(kMaxParametricSegments);

    StrokeFixedCountTessellator(PatchAttribs attribs) : StrokeTessellator(attribs) {}

    int prepare(GrMeshDrawTarget*,
                const SkMatrix& shaderMatrix,
                std::array<float,2> matrixMinMaxScales,
                PathStrokeList*,
                int totalCombinedStrokeVerbCnt) final;

    void draw(GrOpFlushState*) const final;

private:
    int fFixedEdgeCount = 0;

    // Only used if sk_VertexID is not supported.
    sk_sp<const GrGpuBuffer> fVertexBufferIfNoIDSupport;
};

// Renders opaque, constant-color strokes by decomposing them into standalone tessellation patches.
// Each patch is either a "cubic" (single stroked bezier curve with butt caps) or a "join". Requires
// MSAA if antialiasing is desired.
class StrokeHardwareTessellator final : public StrokeTessellator {
public:
    StrokeHardwareTessellator(PatchAttribs attribs, int maxTessellationSegments)
            : StrokeTessellator(attribs), fMaxTessellationSegments(maxTessellationSegments) {}

    int prepare(GrMeshDrawTarget*,
                const SkMatrix& shaderMatrix,
                std::array<float,2> matrixMinMaxScales,
                PathStrokeList*,
                int totalCombinedStrokeVerbCnt) final;

    void draw(GrOpFlushState*) const final;

private:
    const int fMaxTessellationSegments;
};

}  // namespace skgpu::v1

#endif  // StrokeTessellator_DEFINED
