/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/CoverBoundsRenderStep.h"

#include "src/base/SkVx.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

namespace skgpu::graphite {

CoverBoundsRenderStep::CoverBoundsRenderStep(bool inverseFill)
        : RenderStep("CoverBoundsRenderStep",
                     inverseFill ? "inverse" : "regular",
                     Flags::kPerformsShading,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangleStrip,
                     inverseFill ? kInverseCoverPass : kRegularCoverPass,
                     /*vertexAttrs=*/{{"position",
                                       VertexAttribType::kFloat4,
                                       SkSLType::kFloat4}},
                     /*instanceAttrs=*/{{"bounds", VertexAttribType::kFloat4, SkSLType::kFloat4},
                                        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                                        {"ssboIndices", VertexAttribType::kUShort2, SkSLType::kUShort2},
                                        {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                                        {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                                        {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3}})
        , fInverseFill(inverseFill) {}

CoverBoundsRenderStep::~CoverBoundsRenderStep() {}

std::string CoverBoundsRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "float4 devPosition = cover_bounds_vertex_fn("
                                         "float2(sk_VertexID / 2, sk_VertexID % 2), "
                                         "bounds, depth, float3x3(mat0, mat1, mat2), "
                                         "stepLocalCoords);\n";
}

void CoverBoundsRenderStep::writeVertices(DrawWriter* writer,
                                          const DrawParams& params,
                                          skvx::ushort2 ssboIndices) const {
    // Each instance is 4 vertices, forming 2 triangles from a single triangle strip, so no indices
    // are needed. sk_VertexID is used to place vertex positions, so no vertex buffer is needed.
    DrawWriter::Instances instances{*writer, {}, {}, 4};

    skvx::float4 bounds;
    const SkM44* m;
    if (fInverseFill) {
        // Normally all bounding boxes are sorted such that l<r and t<b. We upload an inverted
        // rectangle [r,b,l,t] when it's an inverse fill to encode that the bounds are already in
        // device space and then use the inverse of the transform to compute local coordinates.
        bounds = skvx::shuffle</*R*/2, /*B*/3, /*L*/0, /*T*/1>(
                skvx::cast<float>(skvx::int4::Load(&params.clip().scissor())));
        m = &params.transform().inverse();
    } else {
        bounds = params.geometry().bounds().ltrb();
        m = &params.transform().matrix();
    }

    // Since the local coords always have Z=0, we can discard the 3rd row and column of the matrix.
    instances.append(1) << bounds << params.order().depthAsFloat() << ssboIndices
                        << m->rc(0,0) << m->rc(1,0) << m->rc(3,0)
                        << m->rc(0,1) << m->rc(1,1) << m->rc(3,1)
                        << m->rc(0,3) << m->rc(1,3) << m->rc(3,3);
}

void CoverBoundsRenderStep::writeUniformsAndTextures(const DrawParams&,
                                                     PipelineDataGatherer*) const {
    // All data is uploaded as instance attributes, so no uniforms are needed.
}

}  // namespace skgpu::graphite
