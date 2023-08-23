/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/render/AtlasShapeRenderStep.h"

#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PathAtlas.h"
#include "src/gpu/graphite/geom/AtlasShape.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

namespace skgpu::graphite {

AtlasShapeRenderStep::AtlasShapeRenderStep()
        : RenderStep("AtlasShapeRenderStep",
                     "",
                     Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage,
                     /*uniforms=*/{{"atlasSizeInv", SkSLType::kFloat2},
                                   {"isInverted", SkSLType::kInt}},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGEqualPass,
                     /*vertexAttrs=*/{},
                     /*instanceAttrs=*/
                     {{"drawBounds"  , VertexAttribType::kFloat4 , SkSLType::kFloat4},  // ltrb
                      {"deviceOrigin", VertexAttribType::kInt2, SkSLType::kInt2},
                      {"uvOrigin"    , VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"maskSize"    , VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"depth"     , VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndex" , VertexAttribType::kInt  , SkSLType::kInt},
                      {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3}},
                     /*varyings=*/
                     {// `maskBounds` are the atlas-relative bounds of the coverage mask.
                      // `textureCoords` are the atlas-relative UV coordinates of the draw, which
                      // can spill beyond `maskBounds` for inverse fills.
                      // TODO: maskBounds is constant for all fragments for a given instance,
                      // could we store them in the draw's SSBO?
                      {"maskBounds"   , SkSLType::kFloat4},
                      {"textureCoords", SkSLType::kFloat2}}) {}

std::string AtlasShapeRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "float4 devPosition = atlas_shape_vertex_fn("
                    "float2(sk_VertexID >> 1, sk_VertexID & 1), atlasSizeInv, "
                    "drawBounds, float2(deviceOrigin), float2(uvOrigin), "
                    "float2(maskSize), depth, float3x3(mat0, mat1, mat2), "
                    "maskBounds, textureCoords, stepLocalCoords);\n";
}

std::string AtlasShapeRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    return EmitSamplerLayout(bindingReqs, nextBindingIndex) + " sampler2D pathAtlas;";
}

const char* AtlasShapeRenderStep::fragmentCoverageSkSL() const {
    return R"(
        half c = sample(pathAtlas, clamp(textureCoords, maskBounds.LT, maskBounds.RB)).r;
        outputCoverage = half4(isInverted == 1 ? (1 - c) : c);
    )";
}

float AtlasShapeRenderStep::boundsOutset(const Transform& localToDevice, const Rect&) const {
    // Always incorporate a 1-pixel wide border to the (device space) mask for AA. AtlasShapes are
    // expected to be in device space but only after the clip stack has been applied to the
    // AtlasShape's originating geometry. Hence `localToDevice` is not guaranteed to be identity
    // `boundsOutset` needs to return a local coordinate outset for the shape which will be applied
    // to its local-coordinate bounds before it gets transformed to device space.
    //
    // TODO(b/238770428): This won't produce an accurate result if the transform has perspective as
    // the scale is not uniform across the shape. Reconsider what to do here when this RenderStep
    // supports perspective.
    return 1.0 / localToDevice.maxScaleFactor();
}

void AtlasShapeRenderStep::writeVertices(DrawWriter* dw,
                                         const DrawParams& params,
                                         int ssboIndex) const {
    const AtlasShape& atlasShape = params.geometry().atlasShape();

    // A quad is a 4-vertex instance. The coordinates are derived from the vertex IDs.
    DrawWriter::Instances instances(*dw, {}, {}, 4);

    skvx::half2 maskSize, uvOrigin;
    skvx::int2 deviceOrigin;
    if (params.clip().transformedShapeBounds().isEmptyNegativeOrNaN()) {
        // If the mask shape is clipped out then this must be an inverse fill. There is no mask to
        // sample but we still need to paint the fill region that excludes the mask shape. Signal
        // this by setting the mask size to 0.
        SkASSERT(atlasShape.inverted());
        maskSize = uvOrigin = 0;
        deviceOrigin = 0;
    } else {
        // `maskSize` and `deviceOrigin` correspond to the mask bounds (transformed shape bounds
        // expanded by a 1-pixel border for AA). `uvOrigin` is positioned to include the additional
        // 1-pixel border between atlas entries (which corresponds to their clip bounds and should
        // contain 0).
        maskSize     = atlasShape.maskSize();
        deviceOrigin = atlasShape.deviceOrigin();
        uvOrigin     = atlasShape.atlasOrigin();
    }

    const SkM44& m = atlasShape.deviceToLocal();
    instances.append(1) << params.clip().drawBounds().ltrb()     // drawBounds
                        << deviceOrigin << uvOrigin << maskSize
                        << params.order().depthAsFloat() << ssboIndex
                        << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)   // mat0
                        << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)   // mat1
                        << m.rc(0,3) << m.rc(1,3) << m.rc(3,3);  // mat2
}

void AtlasShapeRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                    PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    const AtlasShape& atlasShape = params.geometry().atlasShape();
    const TextureProxy* proxy = atlasShape.atlas()->texture();
    SkASSERT(proxy);

    // write uniforms
    SkV2 atlasSizeInv = {1.f / proxy->dimensions().width(), 1.f / proxy->dimensions().height()};
    gatherer->write(atlasSizeInv);
    gatherer->write(int(atlasShape.inverted()));

    // write textures and samplers
    const SkSamplingOptions kSamplingOptions(SkFilterMode::kNearest);
    constexpr SkTileMode kTileModes[2] = {SkTileMode::kClamp, SkTileMode::kClamp};
    gatherer->add(kSamplingOptions, kTileModes, sk_ref_sp(proxy));
}

}  // namespace skgpu::graphite
