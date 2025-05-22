/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/BitmapTextRenderStep.h"

#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/SubRunData.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"
#include "src/sksl/SkSLString.h"
#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/VertexFiller.h"

using AtlasSubRun = sktext::gpu::AtlasSubRun;

namespace skgpu::graphite {

namespace {

// We are expecting to sample from up to 4 textures
constexpr int kNumTextAtlasTextures = 4;

RenderStep::RenderStepID variant_id(skgpu::MaskFormat variant) {
    switch (variant) {
        case skgpu::MaskFormat::kA8:   return RenderStep::RenderStepID::kBitmapText_Mask;
        case skgpu::MaskFormat::kA565: return RenderStep::RenderStepID::kBitmapText_LCD;
        case skgpu::MaskFormat::kARGB: return RenderStep::RenderStepID::kBitmapText_Color;
    }

    SkUNREACHABLE;
}

}  // namespace

BitmapTextRenderStep::BitmapTextRenderStep(skgpu::MaskFormat variant)
        : RenderStep(variant_id(variant),
                     Flags(variant) | Flags::kAppendInstances,
                     /*uniforms=*/{{"subRunDeviceMatrix", SkSLType::kFloat4x4},
                                   {"deviceToLocal"     , SkSLType::kFloat4x4},
                                   {"atlasSizeInv"      , SkSLType::kFloat2}},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGEqualPass,
                     /*staticAttrs=*/ {},
                     /*appendAttrs=*/
                     {{"size", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"uvPos", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"xyPos", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"indexAndFlags", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"strikeToSourceScale", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndices", VertexAttribType::kUInt2, SkSLType::kUInt2}},
                     /*varyings=*/
                     {{"textureCoords", SkSLType::kFloat2},
                      {"texIndex", SkSLType::kHalf},
                      {"maskFormat", SkSLType::kHalf}}) {}

BitmapTextRenderStep::~BitmapTextRenderStep() {}

SkEnumBitMask<RenderStep::Flags> BitmapTextRenderStep::Flags(skgpu::MaskFormat variant) {
    switch (variant) {
        case skgpu::MaskFormat::kA8:
            return Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage;
        case skgpu::MaskFormat::kA565:
            return Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage |
                   Flags::kLCDCoverage;
        case skgpu::MaskFormat::kARGB:
            return Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsPrimitiveColor;
        default:
            SkUNREACHABLE;
    }
}

std::string BitmapTextRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "texIndex = half(indexAndFlags.x);"
           "maskFormat = half(indexAndFlags.y);"
           "float2 unormTexCoords;"
           "float4 devPosition = text_vertex_fn(float2(sk_VertexID >> 1, sk_VertexID & 1), "
                                               "subRunDeviceMatrix, "
                                               "deviceToLocal, "
                                               "atlasSizeInv, "
                                               "float2(size), "
                                               "float2(uvPos), "
                                               "xyPos, "
                                               "strikeToSourceScale, "
                                               "depth, "
                                               "textureCoords, "
                                               "unormTexCoords, "
                                               "stepLocalCoords);";
}

std::string BitmapTextRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    std::string result;

    for (unsigned int i = 0; i < kNumTextAtlasTextures; ++i) {
        result += EmitSamplerLayout(bindingReqs, nextBindingIndex);
        SkSL::String::appendf(&result, " sampler2D text_atlas_%u;\n", i);
    }

    return result;
}


const char* BitmapTextRenderStep::fragmentColorSkSL() const {
    // The returned SkSL must write its color into a 'half4 primitiveColor' variable
    // (defined in the calling code).
    static_assert(kNumTextAtlasTextures == 4);
    return "primitiveColor = sample_indexed_atlas(textureCoords, "
                                                 "int(texIndex), "
                                                 "text_atlas_0, "
                                                 "text_atlas_1, "
                                                 "text_atlas_2, "
                                                 "text_atlas_3);";
}

const char* BitmapTextRenderStep::fragmentCoverageSkSL() const {
    // The returned SkSL must write its coverage into a 'half4 outputCoverage' variable (defined in
    // the calling code) with the actual coverage splatted out into all four channels.
    static_assert(kNumTextAtlasTextures == 4);
    return "outputCoverage = bitmap_text_coverage_fn(sample_indexed_atlas(textureCoords, "
                                                                         "int(texIndex), "
                                                                         "text_atlas_0, "
                                                                         "text_atlas_1, "
                                                                         "text_atlas_2, "
                                                                         "text_atlas_3), "
                                                    "int(maskFormat));";
}

bool BitmapTextRenderStep::usesUniformsInFragmentSkSL() const { return false; }

void BitmapTextRenderStep::writeVertices(DrawWriter* dw,
                                         const DrawParams& params,
                                         skvx::uint2 ssboIndices) const {
    const SubRunData& subRunData = params.geometry().subRunData();

    subRunData.subRun()->vertexFiller().fillInstanceData(dw,
                                                         subRunData.startGlyphIndex(),
                                                         subRunData.glyphCount(),
                                                         subRunData.subRun()->instanceFlags(),
                                                         ssboIndices,
                                                         subRunData.subRun()->glyphs(),
                                                         params.order().depthAsFloat());
}

void BitmapTextRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                    PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    const SubRunData& subRunData = params.geometry().subRunData();
    unsigned int numProxies;
    Recorder* recorder = subRunData.recorder();
    const sk_sp<TextureProxy>* proxies =
            recorder->priv().atlasProvider()->textAtlasManager()->getProxies(
                    subRunData.subRun()->maskFormat(), &numProxies);
    SkASSERT(proxies && numProxies > 0);

    // write uniforms
    gatherer->write(params.transform().matrix());  // subRunDeviceMatrix
    gatherer->write(subRunData.deviceToLocal());
    SkV2 atlasDimensionsInverse = {1.f/proxies[0]->dimensions().width(),
                                   1.f/proxies[0]->dimensions().height()};
    gatherer->write(atlasDimensionsInverse);

    // write textures and samplers
    for (unsigned int i = 0; i < numProxies; ++i) {
        gatherer->add(proxies[i], {SkFilterMode::kNearest, SkTileMode::kClamp});
    }
    // If the atlas has less than 4 active proxies we still need to set up samplers for the shader.
    for (unsigned int i = numProxies; i < kNumTextAtlasTextures; ++i) {
        gatherer->add(proxies[0], {SkFilterMode::kNearest, SkTileMode::kClamp});
    }
}

}  // namespace skgpu::graphite
