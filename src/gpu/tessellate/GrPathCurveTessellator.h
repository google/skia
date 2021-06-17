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

class GrCaps;
class GrPipeline;

// Draws an array of "outer curve" patches and, optionally, inner fan triangles for
// GrCubicTessellateShader. Each patch is an independent 4-point curve, representing either a cubic
// or a conic. Quadratics are converted to cubics and triangles are converted to conics with w=Inf.
class GrPathCurveTessellator : public GrPathTessellator {
public:
    // If DrawInnerFan is kNo, this class only emits the path's outer curves. In that case the
    // caller is responsible to handle the path's inner fan.
    enum class DrawInnerFan : bool {
        kNo = false,
        kYes
    };

    // Creates a curve tessellator with the shader type best suited for the given path description.
    static GrPathTessellator* Make(SkArenaAlloc*, const SkMatrix& viewMatrix, const SkPMColor4f&,
                                   DrawInnerFan, int numPathVerbs, const GrPipeline&,
                                   const GrCaps&);

    void prepare(GrMeshDrawTarget*, const SkRect& cullBounds, const SkPath&,
                 const BreadcrumbTriangleList*) override;
    void draw(GrOpFlushState*) const override;
    void drawHullInstances(GrOpFlushState*) const override;

private:
    GrPathCurveTessellator(GrPathTessellationShader* shader, DrawInnerFan drawInnerFan)
            : GrPathTessellator(shader)
            , fDrawInnerFan(drawInnerFan == DrawInnerFan::kYes) {}

    const bool fDrawInnerFan;
    GrVertexChunkArray fVertexChunkArray;

    // If using fixed count, this is the number of vertices we need to emit per instance.
    int fFixedVertexCount;
};

#endif
