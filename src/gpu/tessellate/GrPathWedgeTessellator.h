/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathWedgeTessellator_DEFINED
#define GrPathWedgeTessellator_DEFINED

#include "src/gpu/GrVertexChunkArray.h"
#include "src/gpu/tessellate/GrPathTessellator.h"

// Prepares an array of "wedge" patches for GrWedgeTessellateShader. A wedge is an independent,
// 5-point closed contour consisting of 4 control points plus an anchor point fanning from the
// center of the curve's resident contour. A wedge can be either a cubic or a conic. Quadratics and
// lines are converted to cubics. Once stencilled, these wedges alone define the complete path.
class GrPathWedgeTessellator : public GrPathTessellator {
public:
    static GrPathTessellator* Make(SkArenaAlloc*, const SkMatrix&, const SkPMColor4f&);

    void prepare(GrMeshDrawOp::Target*, const SkRect& cullBounds, const SkPath&,
                 const BreadcrumbTriangleList*) override;
    void draw(GrOpFlushState*) const override;

private:
    GrPathWedgeTessellator(GrPathTessellationShader* shader)
            : GrPathTessellator(shader) {}

    GrVertexChunkArray fVertexChunkArray;

    friend class SkArenaAlloc;  // For constructor.
};

#endif
