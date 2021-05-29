/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathIndirectTessellator_DEFINED
#define GrPathIndirectTessellator_DEFINED

#include "src/gpu/tessellate/GrPathTessellator.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

// Prepares patches of the path's outer curves and, optionally, inner fan triangles using indirect
// draw commands. Quadratics are converted to cubics and triangles are converted to conics with
// w=Inf. An outer curve is an independent, 4-point closed contour that represents either a cubic or
// a conic.
class GrPathIndirectTessellator : public GrPathTessellator {
public:
    static GrPathTessellator* Make(SkArenaAlloc*, const SkPath&, const SkMatrix&,
                                   const SkPMColor4f&, DrawInnerFan);

    void prepare(GrMeshDrawOp::Target*, const SkRect& cullBounds, const SkPath&,
                 const BreadcrumbTriangleList*) override;
    void draw(GrOpFlushState*) const override;
    void drawHullInstances(GrOpFlushState*) const override;

private:
    constexpr static int kMaxResolveLevel = GrTessellationPathRenderer::kMaxResolveLevel;

    GrPathIndirectTessellator(GrPathTessellationShader*, const SkPath&, DrawInnerFan);

    const bool fDrawInnerFan;
    int fResolveLevelCounts[kMaxResolveLevel + 1] = {0};
    int fOuterCurveInstanceCount = 0;

    sk_sp<const GrBuffer> fInstanceBuffer;
    int fBaseInstance = 0;
    int fTotalInstanceCount = 0;

    sk_sp<const GrBuffer> fIndirectDrawBuffer;
    size_t fIndirectDrawOffset = 0;
    int fIndirectDrawCount = 0;

    friend class SkArenaAlloc;  // For constructor.
};

#endif
