/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef StrokeTessellator_DEFINED
#define StrokeTessellator_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStrokeRec.h"
#include "src/core/SkColorData.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrVertexChunkArray.h"
#include "src/gpu/tessellate/Tessellation.h"

class GrMeshDrawTarget;
class GrOpFlushState;
class SkMatrix;

namespace skgpu::ganesh {

// Prepares GPU data for, and then draws a stroke's tessellated geometry. Renders strokes as
// fixed-count triangle strip instances. Any extra triangles not needed by the instance are emitted
// as degenerate triangles.
class StrokeTessellator {
public:
    using PatchAttribs = tess::PatchAttribs;

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
    // Returns the fixed number of edges the tessellator will draw per patch.
    void prepare(GrMeshDrawTarget*,
                 const SkMatrix& shaderMatrix,
                 PathStrokeList*,
                 int totalCombinedStrokeVerbCnt);

    // Issues draw calls for the tessellated stroke. The caller is responsible for creating and
    // binding a pipeline that uses this class's shader() before calling draw().
    void draw(GrOpFlushState*) const;

protected:
    const PatchAttribs fAttribs;

    GrVertexChunkArray fVertexChunkArray;

    int fVertexCount = 0;

    // Only used if sk_VertexID is not supported.
    sk_sp<const GrGpuBuffer> fVertexBufferIfNoIDSupport;
};

}  // namespace skgpu::ganesh

#endif  // StrokeTessellator_DEFINED
