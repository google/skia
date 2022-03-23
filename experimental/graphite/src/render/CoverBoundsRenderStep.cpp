/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/render/CoverBoundsRenderStep.h"

#include "experimental/graphite/src/DrawGeometry.h"
#include "experimental/graphite/src/DrawWriter.h"
#include "experimental/graphite/src/render/StencilAndCoverDSS.h"

namespace skgpu {

CoverBoundsRenderStep::CoverBoundsRenderStep(bool inverseFill)
        : RenderStep("CoverBoundsRenderStep",
                     inverseFill ? "inverse" : "regular",
                     Flags::kPerformsShading,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     inverseFill ? kInverseCoverPass : kRegularCoverPass,
                     /*vertexAttrs=*/{{"position",
                                       VertexAttribType::kFloat4,
                                       SkSLType::kFloat4}},
                     /*instanceAttrs=*/{})
        , fInverseFill(inverseFill) {}

CoverBoundsRenderStep::~CoverBoundsRenderStep() {}

const char* CoverBoundsRenderStep::vertexSkSL() const {
    return "     float4 devPosition = position;\n";
}

void CoverBoundsRenderStep::writeVertices(DrawWriter* writer, const DrawGeometry& geom) const {
    SkV4 devPoints[4]; // ordered TL, TR, BR, BL

    if (fInverseFill) {
        // TODO: When we handle local coords, we'd need to map these corners by the inverse.
        const SkIRect& bounds = geom.clip().scissor();
        devPoints[0] = {(float) bounds.fLeft,  (float) bounds.fTop,    0.f, 1.f};
        devPoints[1] = {(float) bounds.fRight, (float) bounds.fTop,    0.f, 1.f};
        devPoints[2] = {(float) bounds.fRight, (float) bounds.fBottom, 0.f, 1.f};
        devPoints[3] = {(float) bounds.fLeft,  (float) bounds.fBottom, 0.f, 1.f};
    } else {
        geom.transform().mapPoints(geom.shape().bounds(), devPoints);
    }

    float depth = geom.order().depthAsFloat();
    DrawWriter::Vertices verts{*writer};
    verts.append(6) << devPoints[0].x << devPoints[0].y << depth << devPoints[0].w // TL
                    << devPoints[3].x << devPoints[3].y << depth << devPoints[3].w // BL
                    << devPoints[1].x << devPoints[1].y << depth << devPoints[1].w // TR
                    << devPoints[1].x << devPoints[1].y << depth << devPoints[1].w // TR
                    << devPoints[3].x << devPoints[3].y << depth << devPoints[3].w // BL
                    << devPoints[2].x << devPoints[2].y << depth << devPoints[2].w;// BR
}

sk_sp<SkUniformData> CoverBoundsRenderStep::writeUniforms(Layout, const DrawGeometry&) const {
    // Control points are pre-transformed to device space on the CPU, so no uniforms needed.
    return nullptr;
}

}  // namespace skgpu
