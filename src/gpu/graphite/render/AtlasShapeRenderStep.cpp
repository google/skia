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
                      {"deviceOrigin", VertexAttribType::kFloat2 , SkSLType::kFloat2},
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
    // An atlas shape is an axis-aligned rectangle tessellated as a triangle strip.
    //
    // The bounds coordinates that we use here have already been transformed to device space and
    // match the desired vertex coordinates of the draw (taking clipping into account), so a
    // localToDevice transform is always the identity matrix.
    //
    // AtlasShape is always defined based on a regular Shape geometry and we can derive the local
    // coordinates from the bounds by simply applying the inverse of the shape's localToDevice
    // transform.
    return R"(
        float3x3 deviceToLocal = float3x3(mat0, mat1, mat2);
        float2 quadCoords = float2(float(sk_VertexID >> 1), float(sk_VertexID & 1));

        // Vertex coordinates.
        float2 maskDims = float2(maskSize);
        float2 drawCoords =
                drawBounds.xy + quadCoords * max(drawBounds.zw - drawBounds.xy, maskDims);

        // Local coordinates used for shading.
        float3 localCoords = deviceToLocal * drawCoords.xy1;
        stepLocalCoords = localCoords.xy / localCoords.z;

        // Adjust the `maskBounds` to span the full atlas entry with a 2-pixel outset (-1 since the
        // clamp we apply in the fragment shader is inclusive). `textureCoords` get set with a 1
        // pixel inset and its dimensions should exactly match the draw coords.
        //
        // For an inverse fill, `textureCoords` will get clamped to `maskBounds` and the edge pixels
        // will always land on a 0-coverage border pixel.
        float2 uvPos  = float2(uvOrigin);
        if (maskDims.x > 0 && maskDims.y > 0) {
            maskBounds    = float4(uvPos, uvPos + maskDims + float2(1)) * atlasSizeInv.xyxy;
            textureCoords = (uvPos + float2(1) + drawCoords - deviceOrigin) * atlasSizeInv;
        } else {
            // The mask is clipped out so send the texture coordinates to 0. This pixel should
            // always be empty.
            maskBounds = float4(0);
            textureCoords = float2(0);
        }

        float4 devPosition = float4(drawCoords.xy, depth, 1);
    )";
}

std::string AtlasShapeRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    return EmitSamplerLayout(bindingReqs, nextBindingIndex) + " uniform sampler2D pathAtlas;";
}

const char* AtlasShapeRenderStep::fragmentCoverageSkSL() const {
    return R"(
        half c = sample(pathAtlas, clamp(textureCoords, maskBounds.xy, maskBounds.zw)).r;
        outputCoverage = half4(isInverted == 1 ? (1 - c) : c);
    )";
}

void AtlasShapeRenderStep::writeVertices(DrawWriter* dw,
                                         const DrawParams& params,
                                         int ssboIndex) const {
    const AtlasShape& atlasShape = params.geometry().atlasShape();

    // A quad is a 4-vertex instance. The coordinates are derived from the vertex IDs.
    DrawWriter::Instances instances(*dw, {}, {}, 4);

    skvx::float2 maskSize, deviceOrigin, uvOrigin;
    if (params.clip().transformedShapeBounds().isEmptyNegativeOrNaN()) {
        // If the mask shape is clipped out then this must be an inverse fill. There is no mask to
        // sample but we still need to paint the fill region that excludes the mask shape. Signal
        // this by setting the mask size to 0.
        SkASSERT(atlasShape.inverted());
        maskSize = deviceOrigin = uvOrigin = 0;
    } else {
        // Adjust the mask size and device origin for the 1-pixel atlas border for AA. `uvOrigin` is
        // positioned to include the additional 1-pixel border between atlas entries (which
        // corresponds to their clip bounds and should contain 0).
        maskSize     = atlasShape.maskSize() + 2;
        deviceOrigin = atlasShape.deviceOrigin() - 1;
        uvOrigin     = atlasShape.atlasOrigin();
    }

    const SkM44& m = atlasShape.deviceToLocal();
    instances.append(1) << params.clip().drawBounds().ltrb()                 // drawBounds
                        << deviceOrigin                                      // deviceOrigin
                        << uint16_t(uvOrigin.x()) << uint16_t(uvOrigin.y())  // uvOrigin
                        << uint16_t(maskSize.x()) << uint16_t(maskSize.y())  // maskSize
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
