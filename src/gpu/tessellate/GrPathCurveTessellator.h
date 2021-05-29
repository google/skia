/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathCurveTessellator_DEFINED
#define GrPathCurveTessellator_DEFINED

#include "src/gpu/GrVertexChunkArray.h"
#include "src/gpu/tessellate/GrPathTessellator.h"

// Draws an array of "outer curve" patches and, optionally, inner fan triangles for
// GrCubicTessellateShader. Each patch is an independent 4-point curve, representing either a cubic
// or a conic. Quadratics are converted to cubics and triangles are converted to conics with w=Inf.
class GrPathCurveTessellator : public GrPathTessellator {
public:
    static GrPathTessellator* Make(SkArenaAlloc*, const SkMatrix&, const SkPMColor4f&,
                                   DrawInnerFan);

    void prepare(GrMeshDrawOp::Target*, const SkRect& cullBounds, const SkPath&,
                 const BreadcrumbTriangleList*) override;
    void draw(GrOpFlushState*) const override;
    void drawHullInstances(GrOpFlushState*) const override;

private:
    GrPathCurveTessellator(GrPathTessellationShader* shader, DrawInnerFan drawInnerFan)
            : GrPathTessellator(shader)
            , fDrawInnerFan(drawInnerFan == DrawInnerFan::kYes) {}

    const bool fDrawInnerFan;
    GrVertexChunkArray fVertexChunkArray;

    friend class SkArenaAlloc;  // For constructor.
};

#endif
